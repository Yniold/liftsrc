// ============================================
// Mirror.c
// Mirror Valve Control Thread
// ============================================

//
//

//#define DEBUG_MUTEX
#define DEBUG
//#define DEBUG_SETPOS

//#undef DEBUG
#undef DEBUG_SETPOS
#undef DEBUG_MUTEX

#include "mirrorcom.h"
#include "serial.h"
#include <stdint.h>
#include <pthread.h>

long lMirrorBaudrate = 19200;
int iMirrorFile;
unsigned char ucMirrorDeviceName[256] = "/dev/ttyS0";
unsigned char ucMirrorDeviceOpened = false; 
char aMirrorRxBuffer[1024];
struct sMirrorType sMirrorThread = {-1,0,0,0,0,0,0,0,0,0};

pthread_mutex_t mMirrorMutex;

// open tty and create the thread

int MirrorInit(void)
{
	int iRetCode;
	pthread_t ptMirrorThread;
	iMirrorFile = serial_open((char*)ucMirrorDeviceName, lMirrorBaudrate); // will stop with exit()
	sMirrorThread.iFD = iMirrorFile;
#ifdef DEBUG
	printf("In MirrorInit(): FD is %d\n\r", iMirrorFile);
#endif

       	iRetCode = pthread_create(&ptMirrorThread, NULL, (void*)&MirrorThreadFunc,(void*) &sMirrorThread);
	if(iRetCode > 0)
	{
		printf("In MirrorInit: pthread_create failed!\n\r");
		return (1);
	};
	
	return(0);
};

// thread code

void MirrorThreadFunc(void* pArgument)
{
	int iBytesRead;
	int iIndex;
	char aStatusLine[256];
	char aBuffer[256];
	char *pCurrentChar;
	int iStatusLineIndex = 0;
//	extern pthread_mutex_t mMirrorMutex;
	struct sMirrorType *pStructure = (struct sMirrorType *) pArgument;
	int32_t SetPos, OldPos, StartPos;
	int mirror, axis, driver, channel, mirrorbitnumber;
	int iSize;	
	
#ifdef DEBUG
	printf("MirrorThreadFunc started, argument = %08x\n\r", (unsigned int)pArgument);
	printf("FD = %08x\n\r",pStructure->iFD);
#endif
	// init mutex before creating thread
 //	mMirrorMutex = PTHREAD_MUTEX_INITIALIZER;	
	pthread_mutex_init(&mMirrorMutex,NULL);
	memset(aStatusLine,0,250);

	
	// thread will run endless till exit();
	while(true)
	{
	
		//read only if command has been sent and response is therefore expected
		if (pStructure->CommandSent == 1) 
		{
#ifdef DEBUG
		    printf("MirrorCom : CommandStatus : %d \n\r",pStructure->CommandSent);
#endif
			memset(aMirrorRxBuffer,0,1000);
			iBytesRead = read(pStructure->iFD, aMirrorRxBuffer, 1024); // read non blocking
#ifdef DEBUG
			printf("MirrorCom : passed read() %d #%s#\n\r",iBytesRead,aMirrorRxBuffer);
#endif
			if(iBytesRead > 0)
			{
			pCurrentChar = aMirrorRxBuffer;
#ifdef DEBUG
				printf("MirrorCom : bytesread = %d, char = %c, Index = %d\n\r", iBytesRead, *pCurrentChar, iStatusLineIndex);
#endif
				
				for(iIndex = 0; iIndex < iBytesRead; iIndex++)
				{
					if(iStatusLineIndex < 256)
					{
						aStatusLine[iStatusLineIndex++] = *pCurrentChar;
					};
	
					// check if the response is complete
					if((*pCurrentChar == '>') || (*pCurrentChar == '?'))
					{
						MirrorParseLine(aStatusLine, iStatusLineIndex, pStructure);
						iStatusLineIndex = 0;
						memset(aStatusLine,0,250);
					};
					
					pCurrentChar++;
				}
			}
		}

		pthread_mutex_lock(&mMirrorMutex);
		SetPos = pStructure->RelPositionSet;
		pthread_mutex_unlock(&mMirrorMutex);
//#ifdef DEBUG
//		printf("MirrorCom : SetPos = %d\n\r",SetPos);
//#endif

		// check for emergency stop command
		if (pStructure->StopFlag ==1)
		{
			iSize = sprintf(aBuffer,"hal\r");			
			if(write(pStructure->iFD, aBuffer, iSize) < 0)
			{
				printf("Write failed!\n\r");
			} else {
				pthread_mutex_lock(&mMirrorMutex);
				pStructure->StopFlag = 0;
				pStructure->CommandSent = 1;
				pStructure->CurrentRelPos   = 0;
				OldPos=-1;
				if(SetPos != 0)
				{
					pStructure->PosCommandStatus = POS_MOVING;
				}
				pthread_mutex_unlock(&mMirrorMutex);
			}
		}

		// move motor giving all necessary commands and reading out the buffer sequentially
		if(SetPos != 0)
		{
#ifdef DEBUG
		  printf("MirrorCom: Setpos %d CommandStatus %d\n", 
			 SetPos, pStructure->PosCommandStatus);
#endif
			if (pStructure->CommandSent ==0)
			{
				switch (pStructure->PosCommandStatus) {
				case POS_IDLE:
  					pthread_mutex_lock(&mMirrorMutex);
					mirror = pStructure->Mirror;
					axis = pStructure->Axis;
					pthread_mutex_unlock(&mMirrorMutex);
					
					// select driver and channel according to mirror and axis given
					mirrorbitnumber = 2*mirror+axis;
					driver = (int)(mirrorbitnumber/3)+1;
					channel = mirrorbitnumber%3;

					// write vel command and set flag to read buffer
					iSize = sprintf(aBuffer,"vel a%d %d=100\r", driver, channel);		
#ifdef DEBUG
		  			printf("MirrorCom: send Command :#%s# \n", aBuffer); 
#endif	
					if(write(pStructure->iFD, aBuffer, iSize) < 0)
					{
						printf("Write failed!\n\r");
					} else {
						pthread_mutex_lock(&mMirrorMutex);
						pStructure->CurrentRelPos = 0;;
						pStructure->PosCommandStatus   = POS_PREP1;
						pStructure->CommandSent   = 1;
						pthread_mutex_unlock(&mMirrorMutex);
					}
					break;
				case POS_PREP1:			
					iSize = sprintf(aBuffer,"chl a%d=%d\r", driver, channel);			
					if(write(pStructure->iFD, aBuffer, iSize) < 0)
					{
						printf("MirrorCom : Write failed!\n\r");
					} else {
						pthread_mutex_lock(&mMirrorMutex);
						pStructure->PosCommandStatus   = POS_PREP2;
						pStructure->CommandSent   = 1;
						pthread_mutex_unlock(&mMirrorMutex);
					}			
					break;
				case POS_PREP2:
					iSize = sprintf(aBuffer,"rel a%d %d\r", driver, SetPos);			
					if(write(pStructure->iFD, aBuffer, iSize) < 0)
					{
						printf("MirrorCom : Write failed!\n\r");
					} else {
						pthread_mutex_lock(&mMirrorMutex);
						pStructure->PosCommandStatus   = POS_PREP3;
						pStructure->CommandSent   = 1;
						StartPos = pStructure->CurrentAbsPos;
						pthread_mutex_unlock(&mMirrorMutex);
					}			
					break;
				case POS_PREP3:
					iSize = sprintf(aBuffer,"go a%d\r", driver);			
					if(write(pStructure->iFD, aBuffer, iSize) < 0)
					{
						printf("MirrorCom : Write failed!\n\r");
					} else {
						pthread_mutex_lock(&mMirrorMutex);
						pStructure->PosCommandStatus   = POS_MOVING;
						pStructure->CommandSent   = 1;
						pStructure->CurrentRelPos   = 0;
						OldPos=-1;
						pthread_mutex_unlock(&mMirrorMutex);
					}			
					break;
				case POS_MOVING: //recheck position until it doesn´t change anymore
					if (pStructure->CurrentRelPos != OldPos)
					{ 					
						iSize = sprintf(aBuffer,"pos a%d\r", driver);			
						if(write(pStructure->iFD, aBuffer, iSize) < 0)
						{
							printf("MirrorCom : Write failed!\n\r");
						} else {
							pthread_mutex_lock(&mMirrorMutex);
							pStructure->PosCommandStatus   = POS_MOVING;
							pStructure->CommandSent   = 1;
							OldPos=pStructure->CurrentRelPos;
							pStructure->ReadPosCommand   = 1;
							pStructure->CurrentAbsPos = StartPos + pStructure->CurrentRelPos;
							pthread_mutex_unlock(&mMirrorMutex);
						}			
					} else { // movement ended
						pthread_mutex_lock(&mMirrorMutex);
						pStructure->PosCommandStatus   = 0;
						pStructure->CurrentAbsPos = StartPos + pStructure->CurrentRelPos;
						pStructure->CurrentRelPos = 0;;
						pStructure->RelPositionSet = 0;
						pthread_mutex_unlock(&mMirrorMutex);
					}						
				} /* switch PosCommandStatus */
			} /* if CommandSent */
		} /* if SetPos */
			
	usleep(100*1000); // sleep for 100ms to free CPU
	};
};

// parse the input line

void MirrorParseLine(char* aBuffer, int iLength, struct sMirrorType* pStructure)
{
#ifdef DEBUG
	printf("MirrorCom : Buffer consistes of %d chars\n\r",iLength);
	printf("MirrorCom : Buffer contains: %s\n\r",aBuffer);
#endif
	char *pEqualsign;
	int32_t Position;

	if (pStructure->ReadPosCommand == 1) // read position
	{
		pEqualsign=strchr(aBuffer,'=');
	
		if (pEqualsign != NULL)
		{ 
			Position=strtol(pEqualsign+1,NULL,0);

			// copy data into shared structure, so make sure the main thread 
			// is not reading meanwhile
#ifdef DEBUG
			printf("MirrorCom : position read: %ld\n\r",Position);
#endif
			pthread_mutex_lock(&mMirrorMutex);
			pStructure->CurrentRelPos   = (int32_t)Position;
			pStructure->CommandSent   = 0;
			pStructure->ReadPosCommand   = 0;
			pthread_mutex_unlock(&mMirrorMutex);
		}
	} else {
		// copy data into shared structure, so make sure the main thread 
		// is not reading meanwhile

		pthread_mutex_lock(&mMirrorMutex);
		pStructure->CommandSent   = 0;
		pthread_mutex_unlock(&mMirrorMutex);
	}		

};
