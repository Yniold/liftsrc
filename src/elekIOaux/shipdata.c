// ============================================
// shipdata.c
// ShipData Control Thread
// ============================================
//
// $RCSfile: shipdata.c,v $ last changed on $Date: 2007-03-07 18:11:28 $ by $Author: rudolf $
//
// History:
//
// $Log: shipdata.c,v $
// Revision 1.3  2007-03-07 18:11:28  rudolf
// fixed nasty locking bug
//
// Revision 1.2  2007-03-07 17:14:04  rudolf
// more work on parser
//
// Revision 1.1  2007-03-05 20:48:09  rudolf
// added thread for collecting ship's data, more work on parser
//
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

#include "../include/elekGeneral.h"
#include "../include/elekIO.h"
#include "../include/elekIOPorts.h"
#include "../commTools/udptools.h"

#include "shipdata.h"
#include <stdint.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <errno.h>
#include <signal.h>

extern struct MessagePortType MessageOutPortList[];
typedef void Sigfunc (int);

static void dummy_handler(int signo)
{
   return;
};

enum OutPortListEnum
{
   // this list has to be coherent with MessageOutPortList
   CALIB_STATUS_OUT,                // port for outgoing messages to status
     ELEK_ELEKIO_STATUS_OUT,         // port for outgoing status to elekIO
     ELEK_ELEKIO_SLAVE_OUT,          // port for outgoing messages to slaves
     ELEK_MANUAL_OUT,                // port for outgoing messages to eCmd
     ELEK_ETALON_OUT,                // port for outgoing messages to etalon
     ELEK_ETALON_STATUS_OUT,         // port for outgoing messages to etalon status, so etalon is directly informed of the status
     ELEK_SCRIPT_OUT,                // port for outgoing messages to script
     ELEK_DEBUG_OUT,                 // port for outgoing messages to debug
     ELEK_ELEKIO_SLAVE_MASTER_OUT,   // port for outgoing data packets from slave to master
     ELEK_ELEKIO_CALIB_MASTER_OUT,   // port for outgoing data packets from calib to master
     MAX_MESSAGE_OUTPORTS
};

// Variables
//
//typedef void Sigfunc (int);

// the thread works on this structure
//
struct sShipDataType sShipDataThread =
{
   .iFD = -1,                      /* socket FD */
};

pthread_mutex_t mShipDataMutex;

// bind to socket and create the parser thread
//
int ShipDataInit(void)
{
   int iRetCode;
   pthread_t ptShipDataThread;

   // init mutex before creating thread
   pthread_mutex_init(&mShipDataMutex,NULL);

   iRetCode = pthread_create(&ptShipDataThread, NULL, (void*)&ShipDataThreadFunc,(void*) &sShipDataThread);
   if(iRetCode > 0)
     {
	extern struct MessagePortType MessageOutPortList[];
	SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],"elekIOaux : thread create failed");

	printf("In ShipDataInit(): pthread_create failed!\n\r");
	return (1);
     };
   signal(SIGALRM, dummy_handler);

   return(0);
};

// thread code
//
void ShipDataThreadFunc(void* pArgument)
{
   // buffer for one line
   char aUDPBuffer[1024];

   extern struct MessagePortType MessageOutPortList[];
   int iRetVal;

   char cTheChar;
   char *pTheBuffer = aUDPBuffer;
   int iNumChars;

   // socket structure
   struct sockaddr_in ServerAddress;

   // shared structure
   struct sShipDataType *sStructure = (struct sShipDataType *) pArgument;

   while(1)
     {
	// try to connect till success
	while(1)
	  {

	     // create SocketFD
	     sStructure->iFD = socket(AF_INET, SOCK_DGRAM, 0);
	     memset(&ServerAddress,0, sizeof(ServerAddress));

	     // fill out structure for connection to XPORT
	     ServerAddress.sin_family = AF_INET;
	     ServerAddress.sin_port = htons(3040);
	     ServerAddress.sin_addr.s_addr = htonl(INADDR_ANY);

	     // try connection to XPORT using a timeout
	     if((iRetVal = bind(sStructure->iFD, (const struct sockaddr*)&ServerAddress, sizeof(ServerAddress))) < 0)
	       {
		  close(sStructure->iFD);
		  if(errno == EINTR)
		    errno = ETIMEDOUT;
	       }
	     else
	       {
		  break;
	       };
	  };

	if(iRetVal < 0)
	  printf("%s\r\n",strerror(errno));
	else
	  {
	     SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],"elekIOaux : bound to ship's broadcast data");
	     printf("elekIOaux : bound to ship's broadcast data\r\n");
	  };

	while(true)
	  {
	     // clear buffer
             memset(aUDPBuffer,0,1023);
	     if( (iRetVal = recvfrom(sStructure->iFD,aUDPBuffer,1023,0,NULL,NULL)) < 0)
	       {
		  if(errno == EINTR)
		    printf("elekIOaux : socket timeout\r\n");
		  else
		    printf("recvfrom() error\r\n");
	       }
	     // we got data in time
	     else
	       {
		  // strncmp may fail with shorter strings
		  if(iRetVal > 5)
		    {
		       if(strncmp(aUDPBuffer,"@GPS1",5) == 0)
			 {
			    ShipDataParseGPSBuffer(aUDPBuffer,iRetVal,sStructure);
			 };
		       if(strncmp(aUDPBuffer,"@MTS1",5) == 0)
			 {
			    ShipDataParseWaterBuffer(aUDPBuffer,iRetVal,sStructure);
			 };
		       if(strncmp(aUDPBuffer,"@ESFB",5) == 0)
			 {
			    ShipDataParseSonarBuffer(aUDPBuffer,iRetVal,sStructure);
			 };
		       if(strncmp(aUDPBuffer,"@GYR1",5) == 0)
			 {
			    ShipDataParseGyroBuffer(aUDPBuffer,iRetVal,sStructure);
			 };
		       if(strncmp(aUDPBuffer,"@GIL1",5) == 0)
			 {
			    ShipDataParseAnemoBuffer(aUDPBuffer,iRetVal,sStructure);
			 };
		    };
		  //		  		  printf("%s\r\n",aUDPBuffer);
	       };
	  };
     };
   printf("Trying to reconnect!\r\n");
   close(sStructure->iFD);
};

void ShipDataParseGPSBuffer(char* pBuffer, int iBuffLen, struct sShipDataType* sDataStructure)
{
   char* pRetVal;
   int iTokenNumber=0;
   char* pContext;

   // use thread safe version here...
   pRetVal = strtok_r(pBuffer,",",&pContext);

   //write(2,"ShipDataParseGPSBuffer: before lock\n\r",sizeof("ShipDataParseGPSBuffer: before lock\n\r"));
   pthread_mutex_lock(&mShipDataMutex);
   //write(2,"ShipDataParseGPSBuffer: after lock\n\r",sizeof("ShipDataParseGPSBuffer: after lock\n\r"));

   while(true)
     {
	if(pRetVal != NULL)
	  {
	     iTokenNumber++;
	     switch(iTokenNumber)
	       {
		case 3:
		    {
		       unsigned int uiTime = atoi(pRetVal);
		       sDataStructure->ucUTCHours = uiTime / 10000;
		       sDataStructure->ucUTCMins = (uiTime - (sDataStructure->ucUTCHours * 10000)) / 100;
		       sDataStructure->ucUTCSeconds = (uiTime -(sDataStructure->ucUTCHours * 10000)-(sDataStructure->ucUTCMins*100));
		    }

		case 4:
		  sDataStructure->ucUTCDay = atoi(pRetVal);

		case 5:
		  sDataStructure->ucUTCMonth = atoi(pRetVal);

		case 6:
		  sDataStructure->uiUTCYear = atoi(pRetVal);

		case 7:
		  sDataStructure->dLongitude= strtod(pRetVal,NULL);

		case 8:
		  if(pRetVal[0] == 'S')
		    sDataStructure->dLongitude = -sDataStructure->dLongitude;

		case 9:
		  sDataStructure->dLatitude= strtod(pRetVal,NULL);

		case 10:
		  if(pRetVal[0] == 'W')
		    sDataStructure->dLatitude = -sDataStructure->dLatitude;

		case 11:
		  sDataStructure->dCourseOverGround= strtod(pRetVal,NULL);

		case 12:
		  sDataStructure->dGroundSpeed= strtod(pRetVal,NULL);

		default:
		  break;
	       };
	     pRetVal=strtok_r(NULL,",",&pContext);
	  }
	else
	  break;

     };

   sDataStructure->Valid.Field.ShipGPSDataValid = 1;
   //write(2,"ShipDataParseGPSBuffer: before unlock\n\r",sizeof("ShipDataParseGPSBuffer: before unlock\n\r"));
   pthread_mutex_unlock(&mShipDataMutex);
   //write(2,"ShipDataParseGPSBuffer: after unlock\n\r",sizeof("ShipDataParseGPSBuffer: after unlock\n\r"));

   if(gPlotData)
     {
	printf("GPS Time:(UTC) %02d:%02d:%02d\r\n",sDataStructure->ucUTCHours,sDataStructure->ucUTCMins,sDataStructure->ucUTCSeconds);
	printf("GPS Date:      %02d.%02d.%04d\r\n",sDataStructure->ucUTCDay,sDataStructure->ucUTCMonth,sDataStructure->uiUTCYear);
	printf("GPS Longitude: %09.6f\r\n",sDataStructure->dLongitude);
	printf("GPS Latitude:  %09.6f\r\n",sDataStructure->dLatitude);
	printf("GPS Course:    %04.2f\r\n",sDataStructure->dCourseOverGround);
	printf("GPS Speed:     %04.2f\r\n\r\n",sDataStructure->dGroundSpeed);
     };
};

void ShipDataParseWaterBuffer(char* pBuffer, int iBuffLen, struct sShipDataType* sDataStructure)
{
   char* pRetVal;
   int iTokenNumber=0;
   char* pContext;

   // use thread safe version here...
   pRetVal = strtok_r(pBuffer,",",&pContext);
   //write(2,"ShipDataParseWaterBuffer: before lock\n\r",sizeof("ShipDataParseWaterBuffer: before lock\n\r"));
   pthread_mutex_lock(&mShipDataMutex);
   //write(2,"ShipDataParseWaterBuffer: after lock\n\r",sizeof("ShipDataParseWaterBuffer: after lock\n\r"));

   while(true)
     {
	if(pRetVal != NULL)
	  {
	     iTokenNumber++;
	     switch(iTokenNumber)
	       {
		case 2:
		  if(pRetVal[0] == 'P')
		    {
		       //write(2,"ShipDataParseWaterBuffer: before unlock (return)\n\r",sizeof("ShipDataParseWaterBuffer: before unlock (return)\n\r"));
		       pthread_mutex_unlock(&mShipDataMutex);
		       //write(2,"ShipDataParseWaterBuffer: after unlock (return)\n\r",sizeof("ShipDataParseWaterBuffer: after unlock (return)\n\r"));
		       return;
		    };
		  
		case 3:
		  sDataStructure->dWaterTemp = strtod(pRetVal,NULL);

		case 5:
		  sDataStructure->dSalinity = strtod(pRetVal,NULL);

		default:
		  break;
	       };
	     pRetVal=strtok_r(NULL,",",&pContext);
	  }
	else
	  break;

     };
   sDataStructure->Valid.Field.ShipWaterDataValid = 1;

   //write(2,"ShipDataParseWaterBuffer: before unlock\n\r",sizeof("ShipDataParseWaterBuffer: before unlock\n\r"));
   pthread_mutex_unlock(&mShipDataMutex);
   //write(2,"ShipDataParseWaterBuffer: after unlock\n\r",sizeof("ShipDataParseWaterBuffer: after unlock\n\r"));

   if(gPlotData)
     {
	printf("Water Temperature:  %04.2f\r\n",sDataStructure->dWaterTemp);
	printf("Water Salinity:     %04.2f\r\n\r\n",sDataStructure->dSalinity);
     };
};

void ShipDataParseSonarBuffer(char* pBuffer, int iBuffLen, struct sShipDataType* sDataStructure)
{
   char* pRetVal;
   int iTokenNumber=0;
   char* pContext;

   // use thread safe version here...
   pRetVal = strtok_r(pBuffer,",",&pContext);

   //write(2,"ShipDataParseSonarBuffer: before lock\n\r",sizeof("ShipDataParseSonarBuffer: before lock\n\r"));
   pthread_mutex_lock(&mShipDataMutex);
   //write(2,"ShipDataParseSonarBuffer: after lock\n\r",sizeof("ShipDataParseSonarBuffer: after lock\n\r"));

   while(true)
     {
	if(pRetVal != NULL)
	  {
	     iTokenNumber++;
	     switch(iTokenNumber)
	       {
		case 3:
		  if(strncmp(pRetVal, "#SF11SBP",8) != 0)
		    {
		       //write(2,"ShipDataParseSonarBuffer: before unlock (return)\n\r",sizeof("ShipDataParseSonarBuffer: before unlock (return)\n\r"));
		       pthread_mutex_unlock(&mShipDataMutex);
		       //write(2,"ShipDataParseSonarBuffer: after unlock (return)\n\r",sizeof("ShipDataParseSonarBuffer: after unlock (return)\n\r"));

		       return;
		    };

		case 10:
		  sDataStructure->dWaterDepth = strtod(pRetVal,NULL);

		default:
		  break;
	       };
	     pRetVal=strtok_r(NULL,",",&pContext);
	  }
	else
	  break;

     };
   sDataStructure->Valid.Field.ShipSonarDataValid = 1;
   //write(2,"ShipDataParseSonarBuffer: before unlock\n\r",sizeof("ShipDataParseSonarBuffer: before unlock\n\r"));
   pthread_mutex_unlock(&mShipDataMutex);
   //write(2,"ShipDataParseSonarBuffer: after unlock\n\r",sizeof("ShipDataParseSonarBuffer: after unlock\n\r"));

   if(gPlotData)
     {
	printf("Water Depth:     %06.2f\r\n\r\n",sDataStructure->dWaterDepth);
     };
};

void ShipDataParseGyroBuffer(char* pBuffer, int iBuffLen, struct sShipDataType* sDataStructure)
{
   char* pRetVal;
   int iTokenNumber=0;
   char* pContext;

   // use thread safe version here...
   pRetVal = strtok_r(pBuffer,",",&pContext);

   //write(2,"ShipDataParseGyroBuffer: before lock\n\r",sizeof("ShipDataParseGyroBuffer: before lock\n\r"));
   pthread_mutex_lock(&mShipDataMutex);
   //write(2,"ShipDataParseGyroBuffer: after lock\n\r",sizeof("ShipDataParseGyroBuffer: after lock\n\r"));

   while(true)
     {
	if(pRetVal != NULL)
	  {
	     iTokenNumber++;
	     //printf("Token %02d is '%s'\r\n",iTokenNumber,pRetVal);
	     switch(iTokenNumber)
	       {
		case 3:
		  sDataStructure->dDirection = strtod(pRetVal,NULL);

		default:
		  break;
	       };
	     pRetVal=strtok_r(NULL,",",&pContext);
	  }
	else
	  break;

     };
   sDataStructure->Valid.Field.ShipGyroDataValid = 1;

   //write(2,"ShipDataParseGyroBuffer: before unlock\n\r",sizeof("ShipDataParseGyroBuffer: before unlock\n\r"));
   pthread_mutex_unlock(&mShipDataMutex);
   //write(2,"ShipDataParseGyroBuffer: after unlock\n\r",sizeof("ShipDataParseGyroBuffer: after unlock\n\r"));

   if(gPlotData)
     {
	printf("Gyro Heading:     %06.2f\r\n\r\n",sDataStructure->dDirection);
     };
};

void ShipDataParseAnemoBuffer(char* pBuffer, int iBuffLen, struct sShipDataType* sDataStructure)
{
   char* pRetVal;
   int iTokenNumber=0;
   char* pContext;

   // use thread safe version here...
   pRetVal = strtok_r(pBuffer,",",&pContext);

   //write(2,"ShipDataParseAnemoBuffer: before lock\n\r",sizeof("ShipDataParseAnemoBuffer: before lock\n\r"));
   pthread_mutex_lock(&mShipDataMutex);
   //write(2,"ShipDataParseAnemoBuffer: after lock\n\r",sizeof("ShipDataParseAnemoBuffer: after lock\n\r"));

   while(true)
     {
	if(pRetVal != NULL)
	  {
	     iTokenNumber++;
	     switch(iTokenNumber)
	       {
		case 3:
		  sDataStructure->dWindDirection = strtod(pRetVal,NULL);

		case 4:
		  sDataStructure->dWindSpeed = strtod(pRetVal,NULL);

		default:
		  break;
	       };
	     pRetVal=strtok_r(NULL,",",&pContext);
	  }
	else
	  break;

     };
   sDataStructure->Valid.Field.ShipMeteoDataValid = 1;

   //write(2,"ShipDataParseAnemoBuffer: before unlock\n\r",sizeof("ShipDataParseAnemoBuffer: before unlock\n\r"));
   pthread_mutex_unlock(&mShipDataMutex);
   //write(2,"ShipDataParseAnemoBuffer: after unlock\n\r",sizeof("ShipDataParseAnemoBuffer: after unlock\n\r"));

   if(gPlotData)
     {
	printf("Wind Direction:     %06.2f\r\n",sDataStructure->dWindDirection);
	printf("Wind Speed:         %06.2f\r\n\r\n",sDataStructure->dWindSpeed);
     };
};
