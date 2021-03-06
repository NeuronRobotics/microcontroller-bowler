#include "UserApp_avr.h"

#include <util/delay.h>
#include <avr/io.h>
#include <string.h>
#include <avr/interrupt.h>

#if defined(WPIRBE)
boolean setMode(uint8_t pin,uint8_t mode);
/**
 * Private functions
 */
void SPISlaveTx(BowlerPacket * Packet);
void TXadd(uint8_t b);
void RXadd(uint8_t b);
uint8_t TXget();
uint8_t getTXcount();

boolean gotPacket = false;

BowlerPacket Packet;

uint8_t privateRX[BOWLER_PacketSize];
uint8_t*_FIFO_STORAGE storeRX;
uint8_t privateTX[BOWLER_PacketSize];
uint8_t*_FIFO_STORAGE storeTX;
/**
 * Initialize the SPI port to work as an SPI channel
 */
void SPISlaveInit(){
	//println_I("\t\tStarting the SPI slave mode");
	DDRBbits._P4=INPUT;//Set the SS to an input
	SPIDDRbits._MOSI=INPUT;// Master out
	SPIDDRbits._SCK=INPUT; // Clock
	SPIDDRbits._MISO=OUTPUT; // Master in

	PRRbits._PRSPI=0;  	//Disable powersave features.
	SPCRbits._SPIE=1;   //Enable Interupt
	SPCRbits._SPE=1;	//Enable SPI
	SPCRbits._DORD=0;	//Most signifigant bit first
	SPCRbits._MSTR=0;	//Set as slave
	SPCRbits._CPOL=0;   //Idle clock at 0, rising edge on leading edge
	SPCRbits._CPHA=0;   //Clock phase leading edge sample
	SPDR=0xff;

	InitByteFifo(&storeTX,privateTX,sizeof(privateTX));
	InitByteFifo(&storeRX,privateRX,sizeof(privateRX));
	SetPowerState(TRUE,FALSE,false) ;
	uint8_t i;
	for(i=8;i<16;i++){
		if(GetChannelMode(i)==IS_DI||GetChannelMode(i)==IS_DO)
			setMode(i,IS_SERVO);
	}
	//setMode(8,IS_DO);
}

/**
 * Run the SPI slave server
 */
void SPISlaveServer(){
	if(GetBowlerPacket(&Packet,&storeRX)){
		gotPacket=true; 
		Process_Self_Packet(&Packet);
		SPISlaveTx(&Packet);
		gotPacket=false; 
	}
	//println_I("Slave Server");
}

void SPISlaveTx(BowlerPacket * Packet){
	int end = BowlerHeaderSize + Packet->use.head.DataLegnth;
	int i;
	extern MAC_ADDR MyMAC;
	for(i=0;i<6;i++){
		Packet->use.head.MAC.v[i]=MyMAC.v[i];
	}
	for(i=0;i<end;i++){
		TXadd(Packet->stream[i]);
	}
}

void RXadd(uint8_t b){
	FifoAddByte(&storeRX,b);

}
void TXadd(uint8_t b){
	FifoAddByte(&storeTX,b);
}

uint8_t TXget(){
	uint8_t b=0xff;
	if(getTXcount()>0)
		FifoGetByteStream(&storeTX,&b,1);
	return b;
}
uint8_t getTXcount(){
	return storeTX.byteCount;
}

ISR(SPI_STC_vect){
	RXadd(SPDR);
	SPDR=TXget();
}


#endif
