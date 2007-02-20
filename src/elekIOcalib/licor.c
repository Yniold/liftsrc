// ============================================
// licor.c
// Licor Control Thread
// ============================================
//
// $RCSfile: licor.c,v $ last changed on $Date: 2007-02-20 19:59:09 $ by $Author: rudolf $
//
// History:
//
// $Log: licor.c,v $
// Revision 1.3  2007-02-20 19:59:09  rudolf
// removed debug
//
// Revision 1.2  2007-02-20 13:10:05  rudolf
// fixed warning
//
// Revision 1.1  2007-02-12 18:32:18  rudolf
// added missing files for calibrator
//
//
//
//

//#define DEBUG_MUTEX
//#define DEBUG
//#define DEBUG_SETPOS

#undef DEBUG
#undef DEBUG_SETPOS
#undef DEBUG_MUTEX

#include "licor.h"
#include "serial.h"
#include <stdint.h>
#include <pthread.h>

long lLicorBaudrate = 9600;
int iLicorFile;
unsigned char ucLicorDeviceName[256] = "/dev/ttyS0";
unsigned char ucLicorDeviceOpened = false;
unsigned char aLicorRxBuffer[1024];
unsigned char aLicorTxBuffer[1024];
struct sLicorType sLicorThread = {-1,-1,0,0,9999,0,0,0};

pthread_mutex_t mLicorMutex;

// open tty and create the thread
//
int LicorInit(void)
{
   int iRetCode;
   pthread_t ptLicorThread;
   iLicorFile = serial_open((char*)ucLicorDeviceName, lLicorBaudrate); // will stop with exit()
   sLicorThread.iFD = iLicorFile;
#ifdef DEBUG
   printf("In LicorInit(): FD is %d\n\r", iLicorFile);
#endif

   iRetCode = pthread_create(&ptLicorThread, NULL, (void*)&LicorThreadFunc,(void*) &sLicorThread);
   if(iRetCode > 0)
     {
	printf("In LicorInit: pthread_create failed!\n\r");
	return (1);
     };

   return(0);
};

// thread code
//
void LicorThreadFunc(void* pArgument)
{
#define MAX_PARSE_BUFLEN 1024
   struct sLicorType *sStructure = (struct sLicorType *) pArgument;
   int iBytesRead;
   int iBytesToWrite;

   // used for parsing
   enum LicorDecodeStateEnum DecodeState = LIC_STATE_WAIT_FOR_CR;
   int iBytesInBuffer = 0;
   char cTheChar = 0;

   // init mutex before creating thread
   pthread_mutex_init(&mLicorMutex,NULL);

   iBytesToWrite = snprintf(aLicorTxBuffer,sizeof(aLicorTxBuffer),\
"(RS232 (Rate 1s)(Sources (\"H2OA mm/m\" \"H2OB mm/m\" \"H2OD mm/m\" \"CO2A um/m\" \"CO2B um/m\" \"CO2D um/m\" \"P kPa\" \"T C\" \"Diag\"))(Timestamp Long))\n");
   write(sStructure->iFD,aLicorTxBuffer,iBytesToWrite);

   // thread will run endless till exit();
   while(true)
     {
	iBytesRead = read(sStructure->iFD, &cTheChar, 1); // read non blocking

	if(iBytesRead > 0)
	  {
	     // check if we have not seen a LINEFEED right now
	     if(DecodeState == LIC_STATE_WAIT_FOR_CR)
	       {
		  if(cTheChar == '\n')
		    {
		       DecodeState = LIC_STATE_PARSE_LINE;
		       aLicorRxBuffer[iBytesInBuffer] = 0;
		    };
	       }

	     if(DecodeState == LIC_STATE_PARSE_LINE)
	       {
		  if((cTheChar == '\n') && (iBytesInBuffer > 0))
		    {
		       aLicorRxBuffer[iBytesInBuffer++] = cTheChar;
		       aLicorRxBuffer[iBytesInBuffer] = 0;
		       
		       LicorParseLine(aLicorRxBuffer,iBytesInBuffer,(struct sLicorType*)&sStructure);
		       
		       iBytesInBuffer = 0;
		    }
		  else
		    {
		       if(iBytesInBuffer > (MAX_PARSE_BUFLEN-1))
			 {
			    printf("Buffer overflow in licor parse thread\n");
			    iBytesInBuffer = 0; // throw away string
			 }
		       else
			 {
			    aLicorRxBuffer[iBytesInBuffer++] = cTheChar;
			 };
		    };
	       };
	  };
     };
}

// parse the input line
//
void LicorParseLine(unsigned char* aBuffer, int iLength, struct sLicorType* sTheStructure)
{
#ifdef DEBUG
   printf("Buffer consistes of %d chars\n\r",iLength);
   printf("Buffer contains: %s\n\r",aBuffer);
#endif
   char cTempChar;
   char aTempBuffer[256];
   char aTempBuffer2[256];
   double dFloatArg[9];

#ifdef DEBUG
   int iLoopCount;
   HexDump(aBuffer,iLength);
#endif

   if(iLength > 6)
     {
	if(strncmp("DATAD",aBuffer,5) == 0)
	  {
	     // Data Block detected
	     // so parse the line and fill it into structure
	     sscanf((char*)aBuffer, "DATAD\t%s %s %lf %lf %lf %lf %lf %lf %lf %lf %lf",
		    &aTempBuffer,
		    &aTempBuffer2,
		    &dFloatArg[0],
		    &dFloatArg[1],
		    &dFloatArg[2],
		    &dFloatArg[3],
		    &dFloatArg[4],
		    &dFloatArg[5],
		    &dFloatArg[6],
		    &dFloatArg[7],
		    &dFloatArg[8]
		    );

	  }
	if(strncmp("DATAH",aBuffer,5) == 0)
	  {
	     // Header Block detected
	     //
	  }

     }

/*
	// copy data into shared structure, so make shure the main thread
	// is not reading meanwhile
	//
	pthread_mutex_lock(&mLicorMutex);
	sTheStructure->ucPositionValid   = (unsigned char)iTempArg[0];
	sTheStructure->sCurrentPosition  = (uint16_t)iTempArg[1];
	sTheStructure->sTargetPositionRead   = (uint16_t)iTempArg[2];
	sTheStructure->sMotorControlWord = (uint16_t)iTempArg[3];
	sTheStructure->ucCPUFlags = (uint8_t)iTempArg[4];
	pthread_mutex_unlock(&mLicorMutex);
*/
#ifdef DEBUG
   printf("String1: %s\n\r", aTempBuffer);
   printf("String2: %s\n\r", aTempBuffer2);
   
   for(iLoopCount=0; iLoopCount < 9; iLoopCount++)
     {
	printf("value number#%d:%05.4f\n\r",iLoopCount,dFloatArg[iLoopCount]);
     };
#endif
};

void HexDump(unsigned char *aBuffer, int iBytes)
{
   int iLoop;
   for(iLoop = 0; iLoop < iBytes; iLoop++)
     {
	if((iLoop % 8)== 0)
	  {

	     printf("0x%04x:",iLoop);
	     printf("%02x ",aBuffer[iLoop]);
	  }
	else
	if((iLoop % 8) == 7)
	  {

	     printf("%02x\n\r",aBuffer[iLoop]);
	  }
	else
	  {
	     printf("%02x ",aBuffer[iLoop]);
	  };
     };
   printf("\n\r");
};
