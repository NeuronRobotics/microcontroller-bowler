/*
 * Abstract_bcs_io_setmode.c
 *
 *  Created on: Mar 24, 2013
 *      Author: hephaestus
 */
#include "Bowler/Bowler.h"
#include "Namespace/Namespace_bcs_io_setmode.h"


boolean (*setChanelModeHWPtr)(BYTE,BYTE)=NULL;
/*
 * Initialize SetMode hardware interface functions
 */

void InitilizeBcsIoSetmode(boolean (*setChanelModeHWPtrLocal)(BYTE,BYTE)){
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
		println_E("Set mode pointer not set!")
		return false; 
	}
#if !defined(__AVR_ATmega644P__) && !defined(__AVR_ATmega644PA__) && !defined(__AVR_ATmega324P__)
	println_I("Abstract_bcs_io_setmode Setting Mode: ");printMode(mode,INFO_PRINT);print_I(" on: ");p_int_I(pin);
#endif

	boolean ok = setChanelModeHWPtr(pin,mode);
#if !defined(__AVR_ATmega644P__) && !defined(__AVR_ATmega644PA__) && !defined(__AVR_ATmega324P__)
	print_I(" Hardware ok");
#endif
	getBcsIoDataTable(pin)->PIN.currentChannelMode = mode;
	if(IsAsync(pin)){
#if !defined(__AVR_ATmega644P__) && !defined(__AVR_ATmega644PA__) && !defined(__AVR_ATmega324P__)
		print_I(" Restarting async");
#endif
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
	uint8_t pin =Packet->use.data[0];

	uint8_t mode=Packet->use.data[1];

	if(Packet->use.head.DataLegnth == 7){
		setAsync(pin,Packet->use.data[2]?TRUE:false) ;
	}

	if(SetChannelMode(pin,mode)){
		//READY(Packet,4,33);
		return true; 
	}else{
		println_E("Mode Invalid!");
		return false; 
	}

}
/*
 * Set All channel modes
 * Iterates down the array of channel modes setting all of them
 * Returns true if all modes set OK
 */
boolean AbstractSetAllChannelMode(BowlerPacket * Packet){
	int i=0;
	for(i=0;i<GetNumberOfIOChannels();i++){
		SetChannelMode(i,Packet->use.data[i+1]);
	}
	return true; 
}