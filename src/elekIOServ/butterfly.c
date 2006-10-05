// ============================================
// butterfly.c
// Butterfly Valve Control Thread
// ============================================

// $RCSfile: butterfly.c,v $ last changed on $Date: 2006-10-05 14:34:59 $ by $Author: rudolf $

// History:
//
// $Log: butterfly.c,v $
// Revision 1.1  2006-10-05 14:34:59  rudolf
// preperations for butterfly integration
//
//
//

#define DEBUG
#define DEBUG_SETPOS

#undef DEBUG
#undef DEBUG_SETPOS

#include "butterfly.h"
#include "serial.h"
#include <stdint.h>
#include <pthread.h>

long lButterflyBaudrate = 19200;
int iButterflyFile;
unsigned char ucButterflyDeviceName[256] = "/dev/ttyS0";
unsigned char ucButterflyDeviceOpened = false; 
unsigned char aButterflyRxBuffer[1024];
unsigned char aButterflyTxBuffer[1024];
struct sButterflyType sButterflyThread = {-1,-1,0,0,MAGIC_POSITION_NUMBER,0,0,0};
uint16_t sOldTargetPosition = MAGIC_POSITION_NUMBER;




pthread_mutex_t mButterflyMutex = PTHREAD_MUTEX_INITIALIZER;

// open tty and create the thread

int ButterflyInit(void)
{
	int iRetCode;
	pthread_t ptButterflyThread;

	iButterflyFile = serial_open((char*)ucButterflyDeviceName, lButterflyBaudrate); // will stop with exit()
	sButterflyThread.iFD = iButterflyFile;
		// init mutex before creating thread	
	
	iRetCode = pthread_create(&ptButterflyThread, NULL, (void*)&ButterflyThreadFunc,(void*) &sButterflyThread);
	if(iRetCode > 0)
	{
		printf("In ButterflyInit: pthread_create failed!\n\r");
		return (1);
	};
	
	return(0);
};

// thread code

void ButterflyThreadFunc(void* pArgument)
{
	static unsigned char ucExclamationSeen = 0;
	int iBytesRead;
	int iIndex;
	unsigned char aStatusLine[256];
	char aBuffer[256];
	unsigned char *pCurrentChar;
	int iStatusLineIndex = 0;
	struct sButterflyType *sStructure = (struct sButterflyType *) pArgument;
#ifdef DEBUG
	printf("ButterflyThreadFunc started, argument = %08x\n\r", (unsigned int)pArgument);
	printf("FD = %08x\n\r",sStructure->iFD);
#endif
	// thread will run endless till exit();
	while(true)
	{
		iBytesRead = read(sStructure->iFD, aButterflyRxBuffer, 1024); // read non blocking
		if(iBytesRead > 0)
		{
			pCurrentChar = aButterflyRxBuffer;
			for(iIndex = 0; iIndex < iBytesRead; iIndex++)
			{
#ifdef DEBUG
				printf("Index = %d, bytesread = %d, seen = %d, char = %c, Index = %d\n\r",iIndex, iBytesRead, ucExclamationSeen,*pCurrentChar, iStatusLineIndex);
#endif
				if(*pCurrentChar == '!')
				{
					ucExclamationSeen = true;
#ifdef DEBUG
					printf("Start of Line detected!\n\r");
#endif
				};
				
				if((*pCurrentChar != '!') && (ucExclamationSeen == true) && (iStatusLineIndex < 256))
				{
					aStatusLine[iStatusLineIndex++] = *pCurrentChar;
				};
				
				if((*pCurrentChar == 0x0D) && (ucExclamationSeen == true))
				{
					aStatusLine[iStatusLineIndex] = 0;
					ButterflyParseLine(aStatusLine, iStatusLineIndex, sStructure);
					iStatusLineIndex = 0;
					ucExclamationSeen = false;
				};
				pCurrentChar++;
			};
			pCurrentChar = aButterflyRxBuffer; // prepare for next package	
		};

		// lock mutex for reading current set position
		pthread_mutex_lock(&mButterflyMutex);
		volatile uint16_t sTempSetPos = sStructure->sTargetPositionSet;
#ifdef DEBUG_SETPOS
		printf("Set Position %d\n\r",sTempSetPos);
#endif
		pthread_mutex_unlock(&mButterflyMutex);
		
		// check if we just bootet and don't have a valid set position up to now
		if(sTempSetPos != MAGIC_POSITION_NUMBER)
		{
			if(sTempSetPos != sOldTargetPosition)
			{
				int iSize = sprintf(aBuffer,"!goto %d\r",(int)sTempSetPos);
#ifdef DEBUG_SETPOS
				printf(aBuffer);
#endif
				if(write(sStructure->iFD, aBuffer, iSize) < 0)
					printf("Write failed!\n\r");
				sOldTargetPosition = sTempSetPos;
			};
		};
	usleep(100*1000); // sleep for 100ms to free CPU
	};
};

// parse the input line

void ButterflyParseLine(unsigned char* aBuffer, int iLength, struct sButterflyType* sTheStructure)
{
#ifdef DEBUG
	printf("Buffer consistes of %d chars\n\r",iLength);
	printf("Buffer contains: %s\n\r",aBuffer);
#endif
	char cTempChar; 
	int iTempArg[5];
#ifdef DEBUG
	int iLoopCount;
#endif

	if(iLength == 38)
	{
		sscanf((char*)aBuffer, " %c%01d %c %05d %c %05d %c 0x%04x %c 0x%02x",
		&cTempChar,		
		&iTempArg[0],
		&cTempChar,		
		&iTempArg[1],
		&cTempChar,		
		&iTempArg[2],
		&cTempChar,		
		&iTempArg[3],
		&cTempChar,		
		&iTempArg[4]);

		// copy data into shared structure, so make shure the main thread 
		// is not reading meanwhile

		pthread_mutex_lock(&mButterflyMutex);
		sTheStructure->ucPositionValid   = (unsigned char)iTempArg[0];
		sTheStructure->sCurrentPosition  = (uint16_t)iTempArg[1];
		sTheStructure->sTargetPositionRead   = (uint16_t)iTempArg[2];
		sTheStructure->sMotorControlWord = (uint16_t)iTempArg[3];
		pthread_mutex_unlock(&mButterflyMutex);

#ifdef DEBUG
		for(iLoopCount=0; iLoopCount < 5; iLoopCount++)
	 	{
			printf("value number#%d:%08x\n\r",iLoopCount,iTempArg[iLoopCount]);
		};
#endif
	};
};
