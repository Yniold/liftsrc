// ============================================
// GSB_IO_thread.c
//
// thread for communication with 
// onboard ADC, DAC, AVR etc. via I2C
// 
// Comm is rather slow, so this is done async
// to the main GSB task
// communication is done via a shared structure
// ============================================

#define DEBUG_IOTHREAD
#define DEBUG_ADC_READOUT 1

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <math.h>
#include <stdint.h>
#include <pthread.h>

#include "i2c-dev.h"
#include "gsb.h"
#include "GSB_IO_thread.h"
#include "i2c-commands.h"

// Addresses of I2C devices

#define AVR_ADDRESS 	(0x10)
#define RTC_ADDRESS 	(0x68)
#define DAC_ADDRESS 	(0x4C)
#define ADC_0_ADDRESS	(0x14)
#define ADC_1_ADDRESS	(0x15)
#define EEPROM_ADDRESS	(0x50)

#define DAC_1VOLT	(13107)
#define DAC_2VOLT	(2*DAC_1VOLT)
#define DAC_3VOLT	(3*DAC_1VOLT)
#define DAC_4VOLT	(4*DAC_1VOLT)

int iGSBIOThreadFile;
unsigned char ucGSBIOThreadDeviceName[256] = "/dev/i2c-0";
unsigned char ucGSBIOThreadDeviceOpened = false; 
unsigned char aGSBIOThreadRxBuffer[1024];
unsigned char aGSBIOThreadTxBuffer[1024];

unsigned int uiNumBytesRx;
unsigned char aRxBuffer[35];

// use these two for doing performance measurements
// for reduction of ADC conversion times to the minimum

struct timeval StopWatchStartTime;
struct timeval StopWatchStopTime;

struct sRawADCStruct sMyADCStruct;

unsigned char aWriteCmdBuf[2] = 
{
0xA0,
0x90
};

size_t StackSize;

pthread_attr_t ThreadAttribute;

struct sGSBIOThreadType sGSBIOThread;

// Queue is defined in elekIOGSB.c
extern struct sI2CMessageQueue MyMessageQueue;

// we need this mutex for synchronisation of the structure access
pthread_mutex_t mGSBIOThreadMutex;

// ==============================
// open I2C and create the thread
// ==============================

int GSBIOThreadInit(void)
{
	int iRetCode;
	pthread_t ptGSBIOThread;
	iGSBIOThreadFile = open(ucGSBIOThreadDeviceName, O_RDWR);
    if (iGSBIOThreadFile < 0)
    {
        printf("<GSBTHREAD> ERROR: open(%s) failed\n", ucGSBIOThreadDeviceName);
        printf("<GSBTHREAD> errno = %d, %s\n", errno, strerror(errno));
        return -1;
    };
	#ifdef DEBUG_IOTHREAD
    printf("<GSBTHREAD> SUCCESS: open(%s) passed\n", ucGSBIOThreadDeviceName);
	#endif    
	
	// pass file descriptor and
	// pointer to shm status structure so thread has these informations
	sGSBIOThread.iFD = iGSBIOThreadFile;
	sGSBIOThread.psStatus = pGSBStatus;
	sGSBIOThread.pMessageQueue = &MyMessageQueue;
	
	
	#ifdef DEBUG_IOTHREAD
	printf("<GSBTHREAD> GSBIOThreadInit(): FD is %d\n\r", iGSBIOThreadFile);
	#endif
 
 	// get default stack size
 	// it turned out that even on our 16MB embedded system
 	// a thread gets 8MB(!) as default stack size
 	// which of course fails
 	
	pthread_attr_init(&ThreadAttribute);
	pthread_attr_getstacksize (&ThreadAttribute, &StackSize);
	#ifdef DEBUG_IOTHREAD
	printf("<GSBTHREAD> Default stack size = %li bytes\n\r", StackSize);
	#endif
	
	// set stack size to something reasonable
	StackSize = 128*1024;
	#ifdef DEBUG_IOTHREAD
	printf("<GSBTHREAD> Setting stack size to %li bytes\n\r", StackSize);
	#endif
	pthread_attr_setstacksize (&ThreadAttribute, StackSize);

	iRetCode = pthread_create(&ptGSBIOThread, &ThreadAttribute, (void*)&GSBIOThreadFunc,(void*) &sGSBIOThread);
	if(iRetCode > 0)
	{
        printf("<GSBTHREAD> pthread_create() failed with errno = %d, %s\n", errno, strerror(errno));
		return (1);
	};
	
	return(0);
};

// ==============================
// thread code
// ==============================

void GSBIOThreadFunc(void* pArgument)
{

	struct sGSBIOThreadType *sStructure = (struct sGSBIOThreadType *) pArgument;
	int iLoop;

	#ifdef DEBUG_IOTHREAD
	printf("<GSBTHREAD> GSBIOThreadFunc started, argument = %08x\n\r", (unsigned int)pArgument);
	printf("<GSBTHREAD> FD = %d\n\r",sStructure->iFD);
	#endif

	pthread_mutex_init(&mGSBIOThreadMutex,NULL);
    
    // talk to AVR, CMD_GET_FIRMWARE_INFO
    i2c_recv_from_avr(sStructure->iFD,CMD_GET_FIRMWARE_INFO, (char*)&aRxBuffer, (unsigned int*)&uiNumBytesRx);
    
    if(uiNumBytesRx)
    {
    	printf("<GSBTHREAD> AVR Firmware Identify String: '");
    	for(iLoop = 0; iLoop < uiNumBytesRx;iLoop++)
    	{
    		printf("%c",aRxBuffer[iLoop]);
    	};
		printf("'\n\r");    
	};

	// init setpoint voltages on DAC

	DAC_SetChannel(sStructure->iFD, 0, 0);
	DAC_SetChannel(sStructure->iFD, 1, 0);
	DAC_SetChannel(sStructure->iFD, 2, 0);
	DAC_SetChannel(sStructure->iFD, 3, 0);

	// thread will run endless till exit();
	while(true)
	{
		gettimeofday(&StopWatchStartTime, NULL);

	    printf("<GSBTHREAD> Starting ADC#0 conversion\n\r");
   		ADC_FetchData(sStructure->iFD, 0, &sMyADCStruct);

		gettimeofday(&StopWatchStopTime, NULL);
		
		unsigned long ulTimeDiffMillisecs;
		if(((StopWatchStopTime.tv_usec-StopWatchStartTime.tv_usec)/1000) < 0) // check negative
		{
			ulTimeDiffMillisecs = 1000*((StopWatchStopTime.tv_sec-StopWatchStartTime.tv_sec)+1);
			ulTimeDiffMillisecs -= (1000+(StopWatchStopTime.tv_usec-StopWatchStartTime.tv_usec)/1000);
		}
		else
		{
			ulTimeDiffMillisecs = 1000*(StopWatchStopTime.tv_sec-StopWatchStartTime.tv_sec);
			ulTimeDiffMillisecs += ((StopWatchStopTime.tv_usec-StopWatchStartTime.tv_usec)/1000);
		};
		printf("<GSBTHREAD> Time for 9 channels: %06ldms\n\r",ulTimeDiffMillisecs);
		printf("<GSBTHREAD> Average time for 1 channel: %06ldms\n\r",ulTimeDiffMillisecs / 9);
		printf("<GSBTHREAD> if we do it right[TM] -> 1 channel: %06ldms\n\r",ulTimeDiffMillisecs / 18);
    	printf("<GSBTHREAD> Starting ADC#1 conversion\n\r");
   		ADC_FetchData(sStructure->iFD, 1, &sMyADCStruct);

		// data exchange with main thread
		// lock access to structure against main thread
		pthread_mutex_lock(&mGSBIOThreadMutex);
		
		// first process 
		// data from ADC#0
		
   		sStructure->psStatus->iRawFlowMFC1 		= sMyADCStruct.ADC0.iChannel[0];
   		sStructure->psStatus->iRawFlowMFC2 		= sMyADCStruct.ADC0.iChannel[1];
   		sStructure->psStatus->iRawFlowMFC3 		= sMyADCStruct.ADC0.iChannel[2];
   		sStructure->psStatus->iRawPressureNO1 	= sMyADCStruct.ADC0.iChannel[3];
   		sStructure->psStatus->iRawPressureNO2 	= sMyADCStruct.ADC0.iChannel[4];
   		sStructure->psStatus->iRawPressureNO3 	= sMyADCStruct.ADC0.iChannel[5];
   		sStructure->psStatus->iRawPT100NO1 		= sMyADCStruct.ADC0.iChannel[6];
   		sStructure->psStatus->iRawPT100NO2 		= sMyADCStruct.ADC0.iChannel[7];
   		sStructure->psStatus->iTempADC0 		= sMyADCStruct.ADC0.iTemperature;

		// data from ADC#1
		
   		sStructure->psStatus->iRawPressureCO1 	= sMyADCStruct.ADC1.iChannel[0];
   		sStructure->psStatus->iRawPressureCO2 	= sMyADCStruct.ADC1.iChannel[1];
   		sStructure->psStatus->iRawPressureCO3	= sMyADCStruct.ADC1.iChannel[2];
   		sStructure->psStatus->iRawPT100CO1 		= sMyADCStruct.ADC1.iChannel[3];

		// NB: channels 4-7 are not used right now
   		sStructure->psStatus->iTempADC0 		= sMyADCStruct.ADC1.iTemperature;
   		
   		// copy calculated data, ADC#0 first
		sStructure->psStatus->dFlowMFC1			= sMyADCStruct.ADC0.dMFC0Flow;
		sStructure->psStatus->dFlowMFC2			= sMyADCStruct.ADC0.dMFC1Flow;
		sStructure->psStatus->dFlowMFC3			= sMyADCStruct.ADC0.dMFC2Flow;

		// pressure readings (NO variant)
		sStructure->psStatus->dPressureNO1		= sMyADCStruct.ADC0.dPressSens0;
		sStructure->psStatus->dPressureNO2		= sMyADCStruct.ADC0.dPressSens1;
		sStructure->psStatus->dPressureNO3		= sMyADCStruct.ADC0.dPressSens2;
  
		// PT100 temp (NO variant)
		sStructure->psStatus->dPT100NO1			= sMyADCStruct.ADC0.dPT100Temp0;
		sStructure->psStatus->dPT100NO2			= sMyADCStruct.ADC0.dPT100Temp1;
		sStructure->psStatus->dTempADC0			= sMyADCStruct.ADC0.dADCTemp;   		

   		// TODO: read in config file and set GSB type properly
   		sStructure->psStatus->eTypeOfGSB = GSBTYPE_GSB_M;
		
		// DEBUG
		if(1)
		{
			CheckAndPopMessageQueue(sStructure);		
		}		
		// unlock access
		pthread_mutex_unlock(&mGSBIOThreadMutex);
   	};
};
/*

// lock mutex for reading current set position
#ifdef DEBUG_MUTEX
		printf("before mutex_lock()\n\r");
#endif
		pthread_mutex_lock(&mGSBIOThreadMutex);
#ifdef DEBUG_MUTEX
		printf("after mutex_lock()\n\r");
#endif
		volatile uint16_t sTempSetPos = sStructure->sTargetPositionSet;

#ifdef DEBUG_MUTEX
		printf("before mutex_unlock()\n\r");
#endif
		pthread_mutex_unlock(&mGSBIOThreadMutex);

#ifdef DEBUG_MUTEX
		printf("after mutex_unlock()\n\r");
#endif
*/

/* read the data structure from the connected avr
 * return 0 on success, 1 on failure
 */

int i2c_recv_from_avr(int fd, int command, char *rxdata, unsigned int *recvlen)
{
	unsigned long funcs;
	int status;
	
	if(ioctl(fd, I2C_FUNCS, &funcs)) 
	{
		perror("<GSBTHREAD> ioctl() I2C_FUNCS failed.\n");
		return 1;
	}

	// talk to AVR
	status = ioctl(fd, I2C_SLAVE, 0x10);
	if (status < 0)
	{
		printf("<GSBTHREAD> ERROR: ioctl(fd, I2C_SLAVE, 0x%02X) failed\n", 0x10);
		printf("<GSBTHREAD> errno = %d, %s\n", errno, strerror(errno));
		close(fd);
		return -1;
	}

	// check if we have I2C FUNCS (not all drivers provide this)
	if((funcs & I2C_FUNC_I2C) != 0)
	{
		char wbuf[10];
		char rbuf[35];

		wbuf[0] = command;
		
		// send command
		status = write(fd, &wbuf[0], 1);
		
		// as the bloody write() seems to be derived from some driver internas,
		// it doesn't return number of bytes written, instead
		// it returns zero on success...
		
		if(status)
		{
			printf("<GSBTHREAD> write() failed return code was %d\n\r",status);
		  	return 1;
		};
		
		// read reply from AVR
		unsigned char *pBuffer = (unsigned char*)&rbuf;
		status = read(fd,pBuffer,32);
		*recvlen = (int) rbuf[0];
		memcpy(rxdata, &rbuf[1], 32);
		
		return 0;
	}
	printf("<GSBTHREAD> ERROR: I2C_FUNC_I2C unavailable!\n\r");
	return -1;
};

// =================================================================
// get voltage from ADC
// =================================================================
//
// expects a pointer where the ADC reading is put to

int ADC_FetchData(int iFD, int iADC, struct sRawADCStruct *sADCStruct)
{
	static int iAddress;
	static int iChannel=0;
	static int status;
	static unsigned char aBuffer[20];
	static unsigned char aTempVal[4];

	static union sLTCData strLTCData;

	// check ADC number
	if((iADC<0) || (iADC>1))
		return -1;

	// talk to desired LTC2499
	if(iADC == 0)
	{
		#ifdef DEBUG_ADC_READOUT
		printf("<GSBTHREAD> Using ADC#0 @I2C Address 0x%02X\n\r",ADC_0_ADDRESS);
		#endif
		iAddress = ADC_0_ADDRESS; // address of first LTC2499 (w/o R/W bit)
	}
	else
	if(iADC ==1)
	{
		#ifdef DEBUG_ADC_READOUT
		printf("<GSBTHREAD> Using ADC#1 @I2C Address 0x%02X\n\r",ADC_1_ADDRESS);
		#endif
		iAddress = ADC_1_ADDRESS; // address of first LTC2499 (w/o R/W bit)
	}
	else
		return -2;

	#ifdef DEBUG_ADC_READOUT
	printf("FD: %d I2C Address: %02X\n\r",iFD ,iAddress);
	#endif
	
	// try to change slave address to new value
    status = ioctl(iFD, I2C_SLAVE_FORCE, iAddress);
    if (status < 0)
    {
        printf("<GSBTHREAD> ERROR: ioctl(fd, I2C_SLAVE, 0x%02X) failed\n", iAddress);
        printf("<GSBTHREAD> errno = %d, %s\n", errno, strerror(errno));
        close(iFD);
        return -3;
    }
	
	#ifdef DEBUG_ADC_READOUT
	printf("<GSBTHREAD> trying SOC...\n\r");
	#endif

	// set channel & start conversion
	do
	{
		// start of conversion in normal speed mode
		// read internal temperature sensor
		aWriteCmdBuf[0] = 0xA0;
		aWriteCmdBuf[1] = 0xD0;
		
		status = write(iFD,aWriteCmdBuf,2);
		
		if (status < 0)
		{
			//printf("<GSBTHREAD> errno = %d, %s\n", errno, strerror(errno));
			//printf("<GSBTHREAD> status = %d \n\r",status);				
		
			usleep(1000);
		}
	}
	while(status < 0);
	
	// get reading
	do
	{
		status = read(iFD,aBuffer,4);
		if (status < 0)
		{
			usleep(1000);
		}
	}
	while(status < 0);

	#ifdef DEBUG_ADC_READOUT
	printf("%02X:%02x:%02X:%02X ->",aBuffer[0],aBuffer[1],aBuffer[2],aBuffer[3]);
	#endif

	// endianess is swapped, so swap back
	strLTCData.LCArray[0] = aBuffer[3];
	strLTCData.LCArray[1] = aBuffer[2];
	strLTCData.LCArray[2] = aBuffer[1];
	strLTCData.LCArray[3] = aBuffer[0];

	// use the bitfield & union to get the signed 24bit reading
	int iReading = (signed int)strLTCData.iReading32 - 2147483648;
	iReading = iReading / 128;
	
	// save temp data
	if(iADC == 0)
		sADCStruct->ADC0.iTemperature = iReading;
	if(iADC == 1)
		sADCStruct->ADC1.iTemperature = iReading;
			
	
	printf("RAW Counts: %d Temp: %+08.3f\n\r", iReading, (double)iReading/(314.0f)-273.15f);
	
	// LTC2499 has 16 channels from 0-15
	// scan 8 differential channels, IN+@ even channel numbers, IN-@ odd channel numbers
	// see datasheet on page 18 as reference

	for(iChannel=0; iChannel<8; iChannel++)
	{
		// set channel & start conversion
		do
		{
			aWriteCmdBuf[0] = 0xA0 + iChannel;
			aWriteCmdBuf[1] = 0x90;
			
			status = write(iFD,aWriteCmdBuf,2);
			if (status < 0)
			{
				//printf("<GSBTHREAD> write() SOC failed, errno = %d, %s\n", errno, strerror(errno));
				//printf("<GSBTHREAD> status = %d \n\r",status);				
		
				usleep(1000);
			}
		}
		while(status < 0);

		// poll channel
		do
		{
			status = read(iFD,aBuffer,4);
			if (status < 0)
			{
				usleep(1000);
				//printf("<GSBTHREAD> read() conversion result failed, errno = %d, %s\n", errno, strerror(errno));
				//printf("<GSBTHREAD> status = %d \n\r",status);				
			}
		}
		while(status < 0);

		// endianess is swapped, so swap back
		strLTCData.LCArray[0] = aBuffer[3];
		strLTCData.LCArray[1] = aBuffer[2];
		strLTCData.LCArray[2] = aBuffer[1];
		strLTCData.LCArray[3] = aBuffer[0];

		// use the bitfield & union to get the signed 24bit reading
//		int iReading = strLTCData.LDField.LTReading;
		int iReading = (signed int)strLTCData.iReading32 - 2147483648;

//		double dVoltage = (double)iReading * (double)2.5f / (double)16777216.0f;
		double dVoltage = (double)iReading * (double)5.0f / (double)2147483648;
		printf("Channel %02d: %+09d CTS %+9.7f V (%+5.3fV Diff) ", iChannel, iReading, dVoltage, dVoltage * 3);

		// store in array
		if(iADC == 0)
			sADCStruct->ADC0.iChannel[iChannel] = iReading;
		if(iADC == 1)
			sADCStruct->ADC1.iChannel[iChannel] = iReading;


		if(iADC == 0)
		{
			if(iChannel == 0)
			{
				dVoltage = dVoltage * 3;
				double dFlow = 0.8f / 5.0f * dVoltage;
				printf("Flow:  %+5.3f sml (0.8 sml FSC)\n\r",dFlow);
				sADCStruct->ADC0.dMFC0Flow = dFlow;
			};

			if(iChannel == 1)
			{
				dVoltage = dVoltage * 3;
				double dFlow = 10.0f / 5.0f * dVoltage;
				printf("Flow:  %+5.3f sml (10 sml FSC)\n\r",dFlow);
				sADCStruct->ADC0.dMFC1Flow = dFlow;
			};

			if(iChannel == 2)
			{
				dVoltage = dVoltage * 3;
				double dFlow = 10.0f / 5.0f * dVoltage;
				printf("Flow:  %+5.3f sml (10 sml FSC)\n\r",dFlow);
				sADCStruct->ADC0.dMFC2Flow = dFlow;
			};
		
			if(iChannel == 3)
			{
				dVoltage = dVoltage * 3;
				double dPressure = 3.5f / 5.0f * dVoltage;
				printf("Press: %+5.3f bar (3.5 bar FSC)\n\r",dPressure);
				sADCStruct->ADC0.dPressSens0 = dPressure;
			};

			if(iChannel == 4)
			{
				dVoltage = dVoltage * 3;
				double dPressure = 3.5f / 5.0f * dVoltage;
				printf("Press: %+5.3f bar (3.5 bar FSC)\n\r",dPressure);
				sADCStruct->ADC0.dPressSens1 = dPressure;
			};
		
			if(iChannel == 5)
			{
				dVoltage = dVoltage * 3;
				double dPressure = 60.0f / 5.0f * dVoltage;
				printf("Press: %+5.3f bar (60 bar FSC)\n\r",dPressure);
				sADCStruct->ADC0.dPressSens2 = dPressure;
			};
		
			if(iChannel == 6)
			{
				double dOhms = dVoltage / 200e-6f;				
				double dTemp = Ohm2Temperature(dOhms);
				printf("Ohm: %+5.3f Temp:  %+5.3f degrees celsius\n\r",dOhms, dTemp);
				sADCStruct->ADC0.dPT100Temp0 = dTemp;
			};

			if(iChannel == 7)
			{
				double dOhms = dVoltage / 200e-6f;				
				double dTemp = Ohm2Temperature(dOhms);
				printf("Ohm: %+5.3f Temp:  %+5.3f degrees celsius\n\r",dOhms, dTemp);
				sADCStruct->ADC0.dPT100Temp1 = dTemp;
			};
		};
		if(iADC == 1)
		{
			if(iChannel == 0)
			{
				dVoltage = dVoltage * 3;						// voltage divider is made of 3 equal 10K resistors
				double dCurrent = dVoltage / 100.0f;			// 100R measurement resistor in current loop
				double dSlope = 200.00f / 16e-3f; 				// 16mA span for 200bar FSC
				double dPressure = (dCurrent - 4e-3f) * dSlope;	// 4mA is zero offset (4mA - 20mA current loop sensor)
				
				printf("Curr:  %+9.7fA Press: %+5.3f bar (200 bar FSC)\n\r",dCurrent,dPressure);
			};

			if(iChannel == 1)
			{
				dVoltage = dVoltage * 3;						// voltage divider is made of 3 equal 10K resistors
				double dCurrent = dVoltage / 100.0f;			// 100R measurement resistor in current loop
				double dSlope = 3.00f / 16e-3f; 				// 16mA span for 3bar FSC
				double dPressure = (dCurrent - 4e-3f) * dSlope;	// 4mA is zero offset (4mA - 20mA current loop sensor)
				
				printf("Curr:  %+9.7fA Press: %+5.3f bar (3 bar FSC)\n\r",dCurrent,dPressure);
			};

			if(iChannel == 2)
			{
				dVoltage = dVoltage * 3;						// voltage divider is made of 3 equal 10K resistors
				double dCurrent = dVoltage / 100.0f;			// 100R measurement resistor in current loop
				double dSlope = 3.00f / 16e-3f; 				// 16mA span for 3bar FSC
				double dPressure = (dCurrent - 4e-3f) * dSlope;	// 4mA is zero offset (4mA - 20mA current loop sensor)
				
				printf("Curr:  %+9.7fA Press: %+5.3f bar (3 bar FSC)\n\r",dCurrent,dPressure);
			};
		
		
			if(iChannel == 3)
			{
				double dOhms = dVoltage / 200e-6f;				
				double dTemp = Ohm2Temperature(dOhms);
				printf("Temp:  %+5.3f degrees celsius\n\r",dTemp);
			};
			
			if((iChannel > 3) && (iChannel < 8))
			{
				printf("Not used!\n\r");
			};
		};
	};
	return 0;
};

// =================================================================
// check if we got an I2C message in the queue to be sent out
// and do so if any message is available
// =================================================================

void CheckAndPopMessageQueue(struct sGSBIOThreadType *sThreadStructure)
{
	// check if we have a message from the main GSB thread to send via I2C
	printf("Message Queue Head: %03d\n\r",sThreadStructure->pMessageQueue->ucHead);
	printf("Head Message is:");

	// print HEAD message in HEX for debug
	int iPrintLoop = 0;
	for(iPrintLoop = 0; iPrintLoop < 10; iPrintLoop++)
	{
		printf("0x%02X ",sThreadStructure->pMessageQueue->sI2CMessage[sThreadStructure->pMessageQueue->ucHead].aMsgBuffer[iPrintLoop]);
	};
	printf("Length: %02d byte\n\r",sThreadStructure->pMessageQueue->sI2CMessage[sThreadStructure->pMessageQueue->ucHead].ucMsgLength);
	printf("I2Cadr: 0x%02x\n\r",sThreadStructure->pMessageQueue->sI2CMessage[sThreadStructure->pMessageQueue->ucHead].ucI2CAddress);
	printf("Message Queue Tail: %03d\n\r",sThreadStructure->pMessageQueue->ucTail);


	int status;

	if(sThreadStructure->pMessageQueue->ucHead != sThreadStructure->pMessageQueue->ucTail)
	{
		// try to change slave address to new value
		status = ioctl(sThreadStructure->iFD, I2C_SLAVE_FORCE, \
						sThreadStructure->pMessageQueue->sI2CMessage[sThreadStructure->pMessageQueue->ucHead].ucI2CAddress);
		if (status < 0)
		{
			printf("<GSBTHREAD> MSG_POP: ioctl(fd, I2C_SLAVE) failed\n");
			printf("<GSBTHREAD> errno = %d, %s\n", errno, strerror(errno));
		}

		// write I2C cmd from queue
		do
		{		
			status = write(sThreadStructure->iFD,sThreadStructure->pMessageQueue->sI2CMessage[sThreadStructure->pMessageQueue->ucHead].aMsgBuffer,\
							sThreadStructure->pMessageQueue->sI2CMessage[sThreadStructure->pMessageQueue->ucHead].ucMsgLength);

			if (status < 0)
			{
				usleep(10000);
			}
		}
		while(status < 0);
		sThreadStructure->pMessageQueue->ucTail = (sThreadStructure->pMessageQueue->ucTail+1) % MAX_I2C_QUEUE_ENTRIES;
	};
};

// =================================================================
// set voltage on DAC Channel (MFC) 0-3, Range 0-5V equals 0 - 65535
// =================================================================
int DAC_SetChannel(int iFD, int iChannel, unsigned short usDACValue)
{
	int iAddress;
	int status;
	unsigned short usDACEndianess;

	// talk to DAC8574
	iAddress = DAC_ADDRESS;	// address of DAC8574 (w/o R/W bit)
    status = ioctl(iFD, I2C_SLAVE, iAddress);
    if (status < 0)
    {
        printf("ERROR: ioctl(fd, I2C_SLAVE, 0x%02X) failed\n", iAddress);
        printf("errno = %d, %s\n", errno, strerror(errno));
        close(iFD);
        return -1;
    }
	
	// check parameter
	if((iChannel > 3) || (iChannel <0))
		return -2;

	printf("Set Channel #%01d : ",iChannel);

	printf("HEX #%04X / Dez.: #%05d",usDACValue,usDACValue);

	// swap MSB & LSB, DAC8574 wants MSB first
	usDACEndianess  = (unsigned short)(usDACValue >> 8);
	usDACEndianess |= (unsigned short)(usDACValue << 8);

	printf("  after swap HEX #%04X / Dez.: #%05d\n\r",usDACEndianess,usDACEndianess);

	// write to DAC Channel with update
    status = i2c_smbus_write_word_data(iFD, (iChannel << 1) | 0x20, usDACEndianess); 
	return status;
};

double Ohm2Temperature(double dResistance)
{
	double dTemperature = 0.0f;
	
	if(dResistance >= 100.0f)
	{
		double dFirstTerm = (3.90802e-1f/(2*5.802e-5f));
		double dSecondTerm = ((3.90802e-1f)*(3.90802e-1f))/(4*(5.802e-5f*5.802e-5f));
		double dThirdTerm = (dResistance - 100.0f) / 5.802e-5f;
		
		dSecondTerm = sqrt(dSecondTerm-dThirdTerm);
		dTemperature = dFirstTerm - dSecondTerm;
	}
	else
	{
		dTemperature = (1.597e-10f * pow(dResistance,5)) - (2.951e-8f * pow(dResistance,4)) - (4.784e-6 * pow(dResistance,3)) + \
		(2.613e-3 * pow(dResistance,2)) + (2.219f * dResistance) - 241.9f; 
	};
	return dTemperature;
};


