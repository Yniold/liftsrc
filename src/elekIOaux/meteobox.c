// ============================================
// meteobox.c
// MeteoBox Control Thread
// ============================================
//
// $RCSfile: meteobox.c,v $ last changed on $Date: 2007-07-12 17:18:44 $ by $Author: rudolf $
//
// History:
//
// $Log: meteobox.c,v $
// Revision 1.7  2007-07-12 17:18:44  rudolf
// corrected include file order
//
// Revision 1.6  2007-03-08 14:01:22  rudolf
// cleaned up unused ports
//
// Revision 1.5  2007-03-07 21:13:54  rudolf
// startet work on ncurses based GUI
//
// Revision 1.4  2007-03-07 18:11:28  rudolf
// fixed nasty locking bug
//
// Revision 1.3  2007-03-05 20:48:09  rudolf
// added thread for collecting ship's data, more work on parser
//
// Revision 1.2  2007-03-04 19:28:41  rudolf
// added parsing for data into the right structure elements
//
// Revision 1.1  2007-03-04 13:41:59  rudolf
// created new server for auxilliary data like weather data, ships GPS etc
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

#include <stdint.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <errno.h>
#include <signal.h>
#include <ncurses.h>


#include "../include/elekGeneral.h"
#include "../include/elekIO.h"
#include "../include/elekIOPorts.h"
#include "../commTools/udptools.h"

#include "meteobox.h"

extern struct MessagePortType MessageOutPortList[];
extern bool bEnableGUI;
extern WINDOW* pStatusWin;

enum OutPortListEnum
{
   // this list has to be coherent with MessageOutPortList
   ELEK_DEBUG_OUT,                 // port for outgoing messages to debug
     ELEK_MANUAL_OUT,                // reverse port for answers to eCmd
     ELEK_ELEKIO_AUX_MASTER_OUT,     // port for outgoing data packets from elekAux to master
     MAX_MESSAGE_OUTPORTS
};

// Variables
//
typedef void Sigfunc (int);
volatile int iTryCounts = 0;
char aUDPBuffer[1024];
bool bGiveUp = false;

// empty handler for connect() with timeout
static void connect_timeout_handler(int signo)
{
   extern struct MessagePortType MessageOutPortList[];
   SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],"elekIOaux : can't connect to MeteoBox for 2 seconds, still trying...");
   if(bEnableGUI)
     {
	wprintw(pStatusWin,"Can't connect to MeteoBox for 2000ms, still trying...\n");
	wrefresh(pStatusWin);
     }
   else
     printf("elekIOaux : can't connect to MeteoBox for 2000ms, still trying...\n\r");
   return;
};

// empty handler for readl() with timeout
static void read_timeout_handler(int signo)
{
   extern struct MessagePortType MessageOutPortList[];

   if(++iTryCounts < 11)
     {

	sprintf(aUDPBuffer,"elekIOaux : did not get any data from MeteoBox for %02d seconds, try # %02d...",iTryCounts*2, iTryCounts);
	SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],aUDPBuffer);

	if(bEnableGUI)
	  {
	     wprintw(pStatusWin,"Did not get any data from MeteoBox for %02d seconds, try # %02d...\n",iTryCounts*2, iTryCounts);
	     wrefresh(pStatusWin);
	  }
	else
	  printf("elekIOaux : did not get any data from MeteoBox for %02d seconds, try # %02d...\n\r",iTryCounts*2, iTryCounts);
     }
   else
     {
	bGiveUp = true;
     };

   return;
};

// the thread works on this structure
//
struct sMeteoBoxType sMeteoBoxThread =
{
   .iFD = -1,                      /* socket FD */
     .Valid = 0,                /* signal if data is valid or not to main thread */

     .dWindSpeed = 0,                /* Windspeed in m/s */
     .uiWindDirection = 0,           /* 45° resolution */
     .dRelHum = 0.0f,                /* 000.0 - 100.0 % */
     .dAirTemp = 0.0f,               /* Temperature in degree celsius */
     .dGasSensorVoltage = 0.0f       /* dirt sensor */
};

pthread_mutex_t mMeteoBoxMutex;

// bind to socket and create the parser thread
//
int MeteoBoxInit(void)
{
   int iRetCode;
   pthread_t ptMeteoBoxThread;

   // init mutex before creating thread
   pthread_mutex_init(&mMeteoBoxMutex,NULL);

   iRetCode = pthread_create(&ptMeteoBoxThread, NULL, (void*)&MeteoBoxThreadFunc,(void*) &sMeteoBoxThread);
   if(iRetCode > 0)
     {
	extern struct MessagePortType MessageOutPortList[];
	SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],"elekIOaux : init MeteoBox successfull");
	printf("In MeteoBoxInit(): pthread_create failed!\n\r");
	return (1);
     };

   return(0);
};

// thread code
//
void MeteoBoxThreadFunc(void* pArgument)
{
   // buffer for one line
   char aTCPBuffer[1024];

   extern struct MessagePortType MessageOutPortList[];
   int iRetVal;
   Sigfunc *sigfunc;

   char cTheChar;
   char *pTheBuffer = aTCPBuffer;
   int iNumChars;

   // socket structure
   struct sockaddr_in ServerAddress;

   // shared structure
   struct sMeteoBoxType *sStructure = (struct sMeteoBoxType *) pArgument;

   while(1)
     {
	// try to connect till success
	while(1)
	  {

	     // create SocketFD
	     sStructure->iFD = socket(AF_INET, SOCK_STREAM, 0);
	     memset(&ServerAddress,0, sizeof(ServerAddress));

	     // fill out structure for connection to XPORT
	     ServerAddress.sin_family = AF_INET;
	     ServerAddress.sin_port = htons(XPORT_PORTNUMBER);
	     ServerAddress.sin_addr.s_addr = inet_addr(XPORT_IP_ADDRESS);

	     // save old handler and set new one
	     sigfunc = signal(SIGALRM, connect_timeout_handler);

	     // set timeout to 2 seconds
	     alarm(2);

	     // try connection to XPORT using a timeout
	     if((iRetVal = connect(sStructure->iFD, (const struct sockaddr*)&ServerAddress, sizeof(ServerAddress))) < 0)
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
	// we are connected now
	// Turn off alarm
	alarm(0);

	// set handler for read timeout
	sigfunc = signal(SIGALRM, read_timeout_handler);

	if(iRetVal < 0)
	  printf("%s\r\n",strerror(errno));
	else
	  {
	     SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],"elekIOaux : Connected to XPORT on MeteoBox");
	if(bEnableGUI)
	  {
	     wprintw(pStatusWin,"Connected to XPORT on MeteoBox\n");
	     wrefresh(pStatusWin);
	  }
	else
	     printf("elekIOaux : Connected to XPORT on MeteoBox\r\n");
	  }
	;

	// read with timeout
	while(bGiveUp == false)
	  {
	     sigfunc = signal(SIGALRM, read_timeout_handler);
	     alarm(2);
	     //	     printf("entering read()\r\n");

	     if( (iRetVal = read(sStructure->iFD,&cTheChar,1)) < 0)
	       {

		  if(errno == EINTR)
		    printf("elekIOaux : socket timeout\r\n");
		  else
		    printf("read() error\r\n");
	       }
	     // we got data in time
	     else
	       {
		  alarm(0);
		  iTryCounts = 0;
		  //		  printf("Got a Byte\r\n");

		  // check for start of buffer
		  if(cTheChar == '\n')
		    {
		       pTheBuffer = aTCPBuffer;
		       iNumChars = 0;
		    }
		  else
		    // check for end of buffer
		    if((cTheChar == '\r') && (iNumChars > 0))
		      {
			 aTCPBuffer[iNumChars] = 0;
			 MeteoBoxParseBuffer(aTCPBuffer,iNumChars,sStructure);
			 pTheBuffer = aTCPBuffer;
			 iNumChars = 0;
		      }
		  else
		    // store char
		    if(iNumChars < 1024)
		      {
			 (*pTheBuffer) = cTheChar;
			 pTheBuffer++;
			 iNumChars++;
		      }
		  else
		    {
		       printf("Buffer overflow\n\r");
		       pTheBuffer = aTCPBuffer;
		       iNumChars = 0;
		    };
	       };
	  };
	printf("Trying to reconnect!\r\n");
	close(sStructure->iFD);
     };

}

void MeteoBoxParseBuffer(char* pBuffer, int iBuffLen, struct sMeteoBoxType* sDataStructure)
{
   char* pRetVal;
   int iTokenNumber;
   char* pContext;
   //write(2,"Meteo: before lock\n\r",sizeof("Meteo: before lock\n\r"));
   pthread_mutex_lock(&mMeteoBoxMutex);
   //write(2,"Meteo: after lock\n\r",sizeof("Meteo: after lock\n\r"));

   pRetVal = strtok_r(pBuffer,";",&pContext);

   while(true)
     {
	if(pRetVal != NULL)
	  {
	     iTokenNumber++;
	     //	     printf("Token %02d is '%s'\r\n",iTokenNumber,pRetVal);
	     switch(iTokenNumber)
	       {
		case 3:
		  sDataStructure->dWindSpeed = strtod(pRetVal,NULL);

		case 9:
		  sDataStructure->dAirTemp = strtod(pRetVal,NULL);

		case 12:
		  sDataStructure->dRelHum = strtod(pRetVal,NULL);

		case 15:
		  sDataStructure->uiWindDirection= atoi(pRetVal);

		case 18:
		  sDataStructure->dGasSensorVoltage = strtod(pRetVal,NULL);

		default:
		  break;
	       };
	     pRetVal=strtok_r(NULL,";",&pContext);
	  }
	else
	  break;
     };
   sDataStructure->Valid.Field.MeteoBoxDataValid = 1;
   //write(2,"Meteo: before unlock\n\r",sizeof("Meteo: before unlock\n\r"));
   pthread_mutex_unlock(&mMeteoBoxMutex);
   //write(2,"Meteo: after unlock\n\r",sizeof("Meteo: after unlock\n\r"));

   if(gPlotData)
     {

	printf("Wind Speed: %05.3f m/s\r\n",sDataStructure->dWindSpeed);
	printf("Air Temp:   %05.3f °C\r\n",sDataStructure->dAirTemp);
	printf("Rel. Hum.:  %05.3f %\r\n",sDataStructure->dRelHum);
	printf("Wind Dir.:  %03d °\r\n"   ,sDataStructure->uiWindDirection);
	printf("Gas Sensor: %05.3f V\r\n\r\n",sDataStructure->dGasSensorVoltage);
     }

};
