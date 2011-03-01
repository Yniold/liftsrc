#ifndef GSB_H
#define GSB_H

#define REJECTION_50HZ_60HZ 	(0)
#define REJECTION_50HZ 			(1)
#define REJECTION_60HZ  		(2)
#define MAX_I2C_QUEUE_ENTRIES	(10)

extern int DAC_SetChannel(int iFD, int iChannel, unsigned short usDACValue);
extern int ADC_GetChannel(int iFD, int iADC, int iChannel, int *piReading);
extern double Ohm2Temperature(double dResistance);

#pragma pack(push)
#pragma pack(1)

struct sLTCConfigField
{
	// LSB come first
	unsigned char LTDontUse0:3;
	unsigned char LTSpeedBit:1;
	unsigned char LTMainsRejection:2;
	unsigned char LTInternalSensorBit:1;
	unsigned char LTNewConfigBit:1;
	

	// MSB follows
	unsigned char LTChannel:3;
	unsigned char LTOdd:1;
	unsigned char LTSingleEndedBit:1;
	unsigned char LTEnableBit:1;
	unsigned char LTPreambleBits:2;
};

union sLTCConfig
{
	struct sLTCConfigField LCField;
	unsigned char LCArray[2];
};

struct sLTCDataField
{
	unsigned char LTDontUse0:6;
	signed int LTReading:25;
	unsigned char LTSignedBit:1; 		
};

union sLTCData
{
	struct sLTCDataField LDField;
	unsigned char LCArray[4];
};

// We need this for internal communication
// from the main elekIOGSB thread to the
// background polling I2C thread

// When the polling thread is idle,
// we can send commands to control valves,
// set flows etc.

struct sI2CMessageQueueEntry
{
	unsigned char ucMsgLength;		// length of the MSG in bytes
	unsigned char ucI2CAddress;		// I2C device we are talking to
	unsigned char aMsgBuffer[32];	// the message we should send out	
};

struct sI2CMessageQueue
{
	unsigned char ucHead;
	unsigned char ucTail;
	struct sI2CMessageQueueEntry sI2CMessage[MAX_I2C_QUEUE_ENTRIES];
};

#pragma pack(pop)
#endif
