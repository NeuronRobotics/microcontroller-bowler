#include "UserApp_avr.h"

#define NUM_BLOCKS (NUM_PINS/BLOCK_SIZE)

//#define NUM_SERVO (NUM_PINS/NUM_BLOCKS)

#define dataTableSize (BLOCK_SIZE)

#define MIN_SERVO 6
uint8_t blockIndex = 0;
ServoState servoStateMachineCurrentState;
typedef struct _InteruptServoData{
	uint8_t positionTempA;
	uint8_t positionTempB;
	uint16_t toBON;
	uint16_t toAOFF;
	uint16_t toFINISH;
} InteruptServoData;
InteruptServoData blockData [12];
static uint32_t currentTimer=0;
#define LOOPPERIOD (OFFSET+255+(SPACING/2))
#define SPACING (36)
#define LOOPSPACING (SPACING)
#define OFFSET (255+SPACING)


void startServoLoops(){
	currentTimer = TCNT1;// store the state
	servoStateMachineCurrentState = STARTLOOP;
	setServoTimer( LOOPSPACING);
	blockIndex = 0;
	TIMSK1bits._TOIE1=0;// Lets not use the overflow timer when the servo engine is running
}


uint32_t calcTimer(uint32_t value){
	if(value<=1)
		value=1;
	value = value * 8;
    if(value>0x0000ffff){
		println_E("Maxed timer to: ");prHEX32(value,ERROR_PRINT);
        value = 0x0000ffff;
    }
    uint32_t target = value +currentTimer;
    if(target>0x0000ffff){
    	target -=(0x0000ffff);
    }
    if(target < 5 ){
    	//println_E("Edge: ");prHEX32(target,ERROR_PRINT);
    	return 5;
    }
    return target & 0x0000ffff;
}


void setServoTimer( uint32_t value){
	OCR1A = calcTimer( value);
	TIMSK1bits._OCIE1A=1;// Pin timer
}

void stopServoTimer(){

	TIMSK1bits._OCIE1A=0;// Pin timer


}

ISR(TIMER1_COMPA_vect){//timer 1A compare interrupt
	currentTimer = TCNT1;// store the state
	servoTimerEvent();
	//fixTimers(currentTimer);
}

void updateServoValues(){

	int32_t ip;

	// Interpolate position
	ip =getInterpolatedPin(blockIndex);
	if(GetChannelMode(blockIndex) == IS_SERVO)
		blockData[blockIndex].positionTempA=ip;
	else
		blockData[blockIndex].positionTempA = 0;

	// Interpolate position
	ip =getInterpolatedPin(blockIndex +12);
	if(GetChannelMode(blockIndex+12) == IS_SERVO)
		blockData[blockIndex].positionTempB=ip;
	else
		blockData[blockIndex].positionTempB = 0;



}



void servoTimerEvent()
{
	//int flag = FlagBusy_IO;
	FlagBusy_IO=1;
	switch(servoStateMachineCurrentState){
		case STARTLOOP:
			pinOn( blockIndex );
			servoStateMachineCurrentState = BON;
			OCR1A = blockData[blockIndex].toBON;

			break;

		case BON:
			pinOn( blockIndex + 12 );
			servoStateMachineCurrentState =AOFF;
			OCR1A = blockData[blockIndex].toAOFF;

			break;
		case AOFF:
			pinOff(blockIndex );
			servoStateMachineCurrentState = FINISH;
			OCR1A = blockData[blockIndex].toFINISH;

			break;
		case FINISH:
			pinOff(blockIndex + 12 );

			TCCR1Bbits._CS=0;// stop the clock
			//If block is now done, reset the block index and sort
			blockIndex++;
			if(blockIndex == 12){
				// this resets the block Index
				blockIndex=0;
			}
			updateTimer(TCNT1);
			TCNT1=0;// Start from zero for each pulse section
			currentTimer = 0;

			updateServoValues();
			servoStateMachineCurrentState = STARTLOOP;
			blockData[blockIndex].toBON = 	calcTimer (	LOOPSPACING+LOOPPERIOD - (	OFFSET+blockData[blockIndex].positionTempB)	);
			blockData[blockIndex].toAOFF=	calcTimer (	LOOPSPACING+				OFFSET+blockData[blockIndex].positionTempA	);
			blockData[blockIndex].toFINISH=	calcTimer (	LOOPSPACING+LOOPPERIOD );

			setServoTimer( LOOPSPACING);

			TCCR1Bbits._CS = 2;//  value CLslk I/O/8 (From prescaler)


			break;
	}

	//FlagBusy_IO=flag;
	FlagBusy_IO=0;
}




