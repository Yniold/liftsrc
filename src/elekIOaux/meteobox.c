// ============================================
// meteobox.c
// MeteoBox Control Thread
// ============================================
//
// $RCSfile: meteobox.c,v $ last changed on $Date: 2007-03-04 13:41:59 $ by $Author: rudolf $
//
// History:
//
// $Log: meteobox.c,v $
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

#include "../include/elekGeneral.h"
#include "../include/elekIO.h"
#include "../include/elekIOPorts.h"
#include "../commTools/udptools.h"

#include "meteobox.h"
#include <stdint.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <errno.h>
#include <signal.h>

extern struct MessagePortType MessageOutPortList[];

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

typedef void Sigfunc (int);
char aUDPMessage[1024];

// empty handler for connect() with timeout
static void connect_timeout_handler(int signo)
{
   extern struct MessagePortType MessageOutPortList[];
   SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],"elekIOaux : can't connect to MeteoBox for 2000ms, still trying...");
   printf("elekIOaux : can't connect to MeteoBox for 2000ms, still trying...\n\r");
   return;
};

// the thread works on this structure
//
struct sMeteoBoxType sMeteoBoxThread =
{
   .iFD = -1,
     .iCommand = -1,
};

pthread_mutex_t mMeteoBoxMutex;

// bind to socket and create the parser thread
//
int MeteoBoxInit(void)
{
   int iRetCode;
   pthread_t ptMeteoBoxThread;

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
   extern struct MessagePortType MessageOutPortList[];
   int iRetVal;
   Sigfunc *sigfunc;

   char cTheChar;

   // socket structure
   struct sockaddr_in ServerAddress;

   // shared structure
   struct sMeteoBoxType *sStructure = (struct sMeteoBoxType *) pArgument;

   // init mutex before creating thread
   pthread_mutex_init(&mMeteoBoxMutex,NULL);

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
	if(alarm(2) != 0)
	  {
	     printf("Alarm handler already set!\r\n");
	     exit(1);
	  };

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
   //   alarm(0);
   //
   // restore old handler if any
   //   signal(SIGALRM, sigfunc);
   //
   if(iRetVal < 0)
     printf("%s\r\n",strerror(errno));
   else
     {
	SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],"elekIOaux : Connected to XPORT on MeteoBox");
	printf("Connected to XPORT on MeteoBox\r\n");
     }
   ;
   // thread will run endless till exit();
   while(true)
     {
	iRetVal = read(sStructure->iFD,&cTheChar,1);
	if(iRetVal>0)
	  printf("%c",cTheChar);
	else
	  printf("%s\r\n",strerror(errno));
     }

}

