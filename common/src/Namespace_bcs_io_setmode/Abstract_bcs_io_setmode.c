/*
 * Abstract_bcs_io_setmode.c
 *
 *  Created on: Mar 24, 2013
 *      Author: hephaestus
 */
#include "Bowler/Bowler.h"
#include "Namespace/Namespace_bcs_io_setmode.h"


boolean (*setChanelModeHWPtr)(uint8_t  ,uint8_t)=NULL;
/*
 * Initialize SetMode hardware interface functions
 */

void InitilizeBcsIoSetmode(boolean (*setChanelModeHWPtrLocal)(uint8_t  ,uint8_t)){
	if(setChanelModeHWPtrLocal == NULL){
		setPrintLevelErrorPrint();
		//println_E("Failed IO.SETMODE sanity check: initialization");
		while(1);
	}
	setChanelModeHWPtr = setChanelModeHWPtrLocal;
}

/*
 * Set Channel Mode
 * Sets the given channel to the given mode
 * Returns true if successful
 */
boolean SetChannelMode(uint8_t pin,uint8_t mode){
	if(setChanelModeHWPtr == NULL){
		//println_E("Set mode pointer not set!")
		return false; 
	}
	//println_I("Abstract_bcs_io_setmode Setting Mode: ");printMode(mode,INFO_PRINT);print_I(" on: ");p_int_I(pin);

	boolean ok = setChanelModeHWPtr(pin,mode);
	//print_I(" Hardware ok");

	//getBcsIoDataTable(pin)->PIN.currentChannelMode = mode;
	SetChannelModeDataTable(pin,mode);
	if(IsAsync(pin)){

		//print_I(" Restarting async");

		startAdvancedAsyncDefault(pin);
	}
	return ok;
}

/*
 * Set All channel modes
 * Iterates down the array of channel modes setting all of them
 * Returns true if all modes set OK
 */
boolean SetAllChannelModes(uint8_t * modeArray){
	int i=0;
	for(i=0;i<GetNumberOfIOChannels();i++){
		if(!SetChannelMode(i,modeArray[i])){
			return false; 
		}
	}
	return true; 
}
/*
 * Set Channel Mode
 * Sets the given channel to the given mode
 * Returns true if successful
 */
boolean AbstractSetChannelMode(BowlerPacket * Packet){
	//printBowlerPacketDEBUG(Packet,WARN_PRINT);
	uint8_t pin =Packet->use.data[0];
	uint8_t mode=Packet->use.data[1];
	//printBowlerPacketDEBUG(Packet,WARN_PRINT);

	if(SetChannelMode(pin,mode)){
		GetAllChannelModeFromPacket(Packet);
		//printBowlerPacketDEBUG(Packet,WARN_PRINT);
		return true; 
	}else{
		GetAllChannelModeFromPacket(Packet);
		return false; 
	}

}
/*
 * Set All channel modes
 * Iterates down the array of channel modes setting all of them
 * Returns true if all modes set OK
 */
boolean AbstractSetAllChannelMode(BowlerPacket * Packet){
        // First byte is the number of channels
	//printBowlerPacketDEBUG(Packet,WARN_PRINT);
	int i;
	for(i=0;i<GetNumberOfIOChannels();i++){
		SetChannelMode(i,Packet->use.data[i+1]);
	}
	GetAllChannelModeFromPacket(Packet);

	return true; 
}
