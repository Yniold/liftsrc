/*
 * $RCSfile: elekIOaux.c,v $ last changed on $Date: 2007-03-11 11:17:36 $ by $Author: rudolf $
 *
 * $Log: elekIOaux.c,v $
 * Revision 1.9  2007-03-11 11:17:36  rudolf
 * put timestamp to structure
 *
 * Revision 1.8  2007-03-10 14:34:49  rudolf
 * broadcasting data on Port 1180
 *
 * Revision 1.7  2007-03-08 18:53:23  rudolf
 * made fields flash green if new data received, cosmetics
 *
 * Revision 1.6  2007-03-08 14:01:22  rudolf
 * cleaned up unused ports
 *
 * Revision 1.5  2007-03-07 21:13:54  rudolf
 * startet work on ncurses based GUI
 *
 * Revision 1.4  2007-03-07 18:11:28  rudolf
 * fixed nasty locking bug
 *
 * Revision 1.3  2007-03-05 20:48:09  rudolf
 * added thread for collecting ship's data, more work on parser
 *
 * Revision 1.2  2007-03-04 19:28:41  rudolf
 * added parsing for data into the right structure elements
 *
 * Revision 1.1  2007-03-04 13:41:59  rudolf
 * created new server for auxilliary data like weather data, ships GPS etc
 *
 *
 *
 */

#define VERSION 0.1
#define POSIX_SOURCE 1

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <ctype.h> /*for NAN*/
#include <stdlib.h>
#include <math.h>

#include <signal.h>
#include <errno.h>
#include <sched.h>
#include <pthread.h>
#include <ncurses.h>                   /* ncurses.h includes stdio.h */

#include "../include/elekGeneral.h"
#include "../include/elekIO.h"
#include "../include/elekIOPorts.h"
#include "elekIOaux.h"
#include "meteobox.h"
#include "shipdata.h"

#define STATUS_INTERVAL  200

#define DEBUGLEVEL 0
#undef DEBUG_TIMER

#define CPUCLOCK 500000000UL 	// CPU clock of Markus' Athlon XP

// #define DEBUG_STRUCTUREPASSING
//
char gPlotData = 0;    // this is not a define, it's meant to be replaced by a commandline arg in the future

enum InPortListEnum
{
   // this list has to be coherent with MessageInPortList
   ELEK_MANUAL_IN,       // port for incoming commands from  eCmd
     MAX_MESSAGE_INPORTS
};

enum OutPortListEnum
{
   // this list has to be coherent with MessageOutPortList
   ELEK_DEBUG_OUT,                 // port for outgoing messages to debug
     ELEK_MANUAL_OUT,                // reverse port for answers to eCmd
     ELEK_ELEKIO_AUX_MASTER_OUT,     // port for outgoing data packets from elekAux to master
     MAX_MESSAGE_OUTPORTS
};

struct MessagePortType MessageInPortList[MAX_MESSAGE_INPORTS]=
{
   // order in list defines sequence of polling
    /* Name           , PortNo                        , ReversePort    , IPAddr      , fdSocket, MaxMsg, Direction */
     {"Manual"        , UDP_ELEK_MANUAL_INPORT        , ELEK_MANUAL_OUT, IP_LOCALHOST, -1, 1,  UDP_IN_PORT}
};

struct MessagePortType MessageOutPortList[MAX_MESSAGE_OUTPORTS]=
{
   // order in list defines sequence of polling
    /* Name           ,PortNo                        , ReversePort        , IPAddr          , fdSocket, MaxMsg, Direction */
     {"DebugPort"     ,UDP_ELEK_DEBUG_OUTPORT        , -1                 , IP_DEBUG_CLIENT , -1      , 0     ,  UDP_OUT_PORT},
     {"Manual"        ,UDP_ELEK_MANUAL_OUTPORT       , ELEK_MANUAL_IN     , IP_LOCALHOST    , -1      , 0     ,  UDP_OUT_PORT},
     {"ElekIOauxOut"  ,UDP_ELEK_AUX_INPORT           , -1                 , "255.255.255.255", -1      , 0     ,  UDP_OUT_PORT}
};

struct TaskListType TasktoWakeList[MAX_TASKS_TO_WAKE]=
{
   // order defines sequence of wake up after timer
    /* TaskName TaskConn TaskWantStatusOnPort */
     {      "",                  -1,                    -1},
     {      "",                  -1,                    -1},
     {      "",                  -1,                    -1}
};

/**********************************************************************************************************/
/* NCURSES STUFF                                                                                          */
/**********************************************************************************************************/

bool bEnableGUI;

WINDOW* pGPSWin;
WINDOW* pMeteoBoxWin;
WINDOW* pWaterWin;
WINDOW* pSonarWin;
WINDOW* pGyroWin;
WINDOW* pAnemoWin;

WINDOW* pStatusBorderWin; // we create a dummy window just containing the border, so we can use wprintw
// without needing to set the x position to 1 each time
WINDOW* pStatusWin;

/**********************************************************************************************************/
/* Signal Handler                                                                                         */
/**********************************************************************************************************/

static int StatusFlag=0;
static enum TimerSignalStateEnum TimerState=TIMER_SIGNAL_STATE_INITIAL;

/* Signalhandler */
void signalstatus(int signo)
{
  /* locale variables for Timer*/
   extern int StatusFlag;
   extern enum TimerSignalStateEnum TimerState;

   char buf[GENERIC_BUF_LEN];

   ++StatusFlag;
   TimerState=(TimerState+1) % TIMER_SIGNAL_STATE_MAX;

}

//
/**********************************************************************************************************/
/* Load Module Config                                                                                     */
/**********************************************************************************************************/

/* function to load the config settings for all modules */

void LoadModulesConfig(struct auxStatusType *ptrAuxStatus)
{

   extern struct MessagePortType MessageInPortList[];
   extern struct MessagePortType MessageOutPortList[];

   int       i;
   int       Channel;
   int       Card;
   char      buf[GENERIC_BUF_LEN];

   // set all data to invalid
   ptrAuxStatus->Status.Status.Word = 0;
};

/**********************************************************************************************************/
/* Init METEOBOX                                                                                          */
/**********************************************************************************************************/

int InitMeteoBox(struct auxStatusType *ptrAuxStatus)
{

   extern struct MessagePortType MessageInPortList[];
   extern struct MessagePortType MessageOutPortList[];
   char debugbuf[GENERIC_BUF_LEN];

   int ret;

   // create butterfly thread
   ret = MeteoBoxInit();

   if(ret == 1)
     {
	sprintf(debugbuf,"elekIOaux : Can't create MeteoBox Thread!\n\r");
	SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],debugbuf);

	return (INIT_MODULE_FAILED);
     };

   // success
   sprintf(debugbuf,"elekIOaux: MeteoBox Thread running!\n\r");
   SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],debugbuf);

   return (INIT_MODULE_SUCCESS);
}
/* Init MeteoBox */

/**********************************************************************************************************/
/* Init ShipData                                                                                         */
/**********************************************************************************************************/

int InitShipData(struct auxStatusType *ptrAuxStatus)
{

   extern struct MessagePortType MessageInPortList[];
   extern struct MessagePortType MessageOutPortList[];
   char debugbuf[GENERIC_BUF_LEN];

   int ret;

   // create ShipData thread
   ret = ShipDataInit();

   if(ret == 1)
     {
	sprintf(debugbuf,"elekIOaux : Can't create ShipData Thread!\n\r");
	SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],debugbuf);

	return (INIT_MODULE_FAILED);
     };

   // success
   sprintf(debugbuf,"elekIOaux : ShipData Thread running!\n\r");
   SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],debugbuf);

   return (INIT_MODULE_SUCCESS);
}
/* InitShipData */

/**********************************************************************************************************/
/* Init Modules                                                                                        */
/**********************************************************************************************************/

/* function to initialize all Modules */

void InitModules(struct auxStatusType *ptrAuxStatus)
{

   extern struct MessagePortType MessageInPortList[];
   extern struct MessagePortType MessageOutPortList[];

   int       ret;
   char      buf[GENERIC_BUF_LEN];

   LoadModulesConfig(ptrAuxStatus);

   if (INIT_MODULE_SUCCESS == (ret=InitMeteoBox(ptrAuxStatus)))
     {
	SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],"elekIOaux : init MeteoBox successfull");
     }
   else
     {
	SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],"elekIOaux : init MeteoBox failed !!");
     }

   if (INIT_MODULE_SUCCESS == (ret=InitShipData(ptrAuxStatus)))
     {
	SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],"elekIOaux : init ShipData successfull");
     }
   else
     {
	SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],"elekIOaux : init ShipData failed !!");
     }

}
/* InitModules */

/**********************************************************************************************************/
/* GetMeteoBoxData                                                                                        */
/**********************************************************************************************************/

void GetMeteoBoxData ( struct auxStatusType *ptrAuxStatus)
{
   extern struct MessagePortType MessageInPortList[];
   extern struct MessagePortType MessageOutPortList[];
   extern pthread_mutex_t mMeteoBoxMutex;
   extern struct sMeteoType sMeteoThread;

   uint16_t       ret;
   uint16_t       Control;
   char           buf[GENERIC_BUF_LEN];

     {
	//write(2,"GetMeteo: before lock\n\r",sizeof("GetMeteo: before lock\n\r"));
	pthread_mutex_lock(&mMeteoBoxMutex);
	//write(2,"GetMeteo: after lock\n\r",sizeof("GetMeteo: after lock\n\r"));
	//	pthread_mutex_lock(&mMeteoBoxMutex);

	ptrAuxStatus->Status.Status.Word         = ptrAuxStatus->Status.Status.Word | sMeteoBoxThread.Valid.Word; /* will copy only bits set to one*/
	sMeteoBoxThread.Valid.Word = 0;
	ptrAuxStatus->MeteoBox.dWindSpeed        = sMeteoBoxThread.dWindSpeed;        /* Windspeed in m/s */
	ptrAuxStatus->MeteoBox.uiWindDirection   = sMeteoBoxThread.uiWindDirection;   /* 45° resolution */
	ptrAuxStatus->MeteoBox.dRelHum           = sMeteoBoxThread.dRelHum;           /* 000.0 - 100.0 % */
	ptrAuxStatus->MeteoBox.dAirTemp          = sMeteoBoxThread.dAirTemp;          /* Temperature in degree celsius */
	ptrAuxStatus->MeteoBox.dGasSensorVoltage = sMeteoBoxThread.dGasSensorVoltage; /* dirt sensor */
	//write(2,"GetMeteo: before unlock\n\r",sizeof("GetMeteo: before unlock\n\r"));
	pthread_mutex_unlock(&mMeteoBoxMutex);
	//write(2,"GetMeteo: after unlock\n\r",sizeof("GetMeteo: after unlock\n\r"));

	//	pthread_mutex_unlock(&mMeteoBoxMutex);

#ifdef DEBUG_STRUCTUREPASSING
	printf("ptrAuxStatus->MeteoBox.dWindSpeed:        %04.2f\n\r",ptrAuxStatus->MeteoBox.dWindSpeed);
	printf("ptrAuxStatus->MeteoBox.uiWindDirection:   %03d\n\r",ptrAuxStatus->MeteoBox.uiWindDirection);

	printf("ptrAuxStatus->MeteoBox.dRelHum:           %04.2f\n\r",ptrAuxStatus->MeteoBox.dRelHum);
	printf("ptrAuxStatus->MeteoBox.dAirTemp:          %+04.2f\n\r",ptrAuxStatus->MeteoBox.dAirTemp);
	printf("ptrAuxStatus->MeteoBox.dGasSensorVoltage: %05.3f\n\r",ptrAuxStatus->MeteoBox.dGasSensorVoltage);
#endif
     }
}
/* GetMeteoBoxData */

/**********************************************************************************************************/
/* GetShipData                                                                                            */
/**********************************************************************************************************/

void GetShipData ( struct auxStatusType *ptrAuxStatus)
{

   extern struct MessagePortType MessageInPortList[];
   extern struct MessagePortType MessageOutPortList[];
   extern pthread_mutex_t mShipDataMutex;
   extern struct sShipDataType sShipDataThread;

   uint16_t       ret;
   uint16_t       Control;
   char           buf[GENERIC_BUF_LEN];

     {
	//	pthread_mutex_lock(&mShipDataMutex);

	//write(2,"GetShip: before lock\n\r",sizeof("GetShip: before lock\n\r"));
	pthread_mutex_lock(&mShipDataMutex);
	//write(2,"GetShip: after lock\n\r",sizeof("GetShip: after lock\n\r"));

	ptrAuxStatus->Status.Status.Word = ptrAuxStatus->Status.Status.Word | sShipDataThread.Valid.Word; /* will copy only bits set to one*/
	sShipDataThread.Valid.Word = 0;

	ptrAuxStatus->ShipGPS.ucUTCHours = sShipDataThread.ucUTCHours;               /* binary, not BCD coded (!) 0 - 23 decimal*/
	ptrAuxStatus->ShipGPS.ucUTCMins = sShipDataThread.ucUTCMins;                 /* binary, 0-59 decimal */
	ptrAuxStatus->ShipGPS.ucUTCSeconds = sShipDataThread.ucUTCSeconds;           /* binary 0-59 decimal */
	ptrAuxStatus->ShipGPS.ucUTCDay = sShipDataThread.ucUTCDay;                   /* day 1-31 */
	ptrAuxStatus->ShipGPS.ucUTCMonth = sShipDataThread.ucUTCMonth;               /* month 1-12 */
	ptrAuxStatus->ShipGPS.uiUTCYear = sShipDataThread.uiUTCYear;                 /* year 4 digits */
	ptrAuxStatus->ShipGPS.dLongitude = sShipDataThread.dLongitude;               /* "Laengengrad" I always mix it up...
			                                                              * signed notation,
			                                                              * negative values mean "W - west of Greenwich"
			                                                              * positive values mean "E - east of Greenwich" */

	ptrAuxStatus->ShipGPS.dLatitude = sShipDataThread.dLatitude;                 /* "Breitengrad" I always mix it up...
			                                                              * signed notation,
			                                                              * negative values mean "S - south of the equator"
			                                                              * positive values mean "N - north of the equator*/
	ptrAuxStatus->ShipGPS.dGroundSpeed = sShipDataThread.dGroundSpeed;           /* speed in knots above ground */
	ptrAuxStatus->ShipGPS.dCourseOverGround = sShipDataThread.dCourseOverGround; /* heading in degrees */
	ptrAuxStatus->ShipWater.dSalinity = sShipDataThread.dSalinity;               /* gramms per litre */
	ptrAuxStatus->ShipWater.dWaterTemp = sShipDataThread.dWaterTemp;             /* water temp in degrees celsius */

	ptrAuxStatus->ShipMeteo.dWindSpeed = sShipDataThread.dWindSpeed;             /* m/s */
	ptrAuxStatus->ShipMeteo.dWindDirection = sShipDataThread.dWindDirection;     /* in degrees relative to ship ??? */

	ptrAuxStatus->ShipSonar.dFrequency = sShipDataThread.dFrequency;             /* Khz */
	ptrAuxStatus->ShipSonar.dWaterDepth = sShipDataThread.dWaterDepth;           /* m */

	ptrAuxStatus->ShipGyro.dDirection = sShipDataThread.dDirection;              /* degrees */
	
	//write(2,"GetShip: before unlock\n\r",sizeof("GetShip: before unlock\n\r"));
	pthread_mutex_unlock(&mShipDataMutex);
	//write(2,"GetShip: after unlock\n\r",sizeof("GetShip: after unlock\n\r"));

	//	pthread_mutex_unlock(&mShipDataMutex);

#ifdef DEBUG_STRUCTUREPASSING
	printf("ptrAuxStatus->ShipGPS.ucUTCHours:         %02d\n\r",ptrAuxStatus->ShipGPS.ucUTCHours);
	printf("ptrAuxStatus->ShipGPS.ucUTCMins:          %02d\n\r",ptrAuxStatus->ShipGPS.ucUTCMins);
	printf("ptrAuxStatus->ShipGPS.ucUTCSeconds:       %02d\n\r",ptrAuxStatus->ShipGPS.ucUTCSeconds);
	printf("ptrAuxStatus->ShipGPS.ucUTCDay:           %02d\n\r",ptrAuxStatus->ShipGPS.ucUTCDay);
	printf("ptrAuxStatus->ShipGPS.ucUTCMonth:         %02d\n\r",ptrAuxStatus->ShipGPS.ucUTCMonth);
	printf("ptrAuxStatus->ShipGPS.uiUTCYear:          %04d\n\r",ptrAuxStatus->ShipGPS.uiUTCYear);
	printf("ptrAuxStatus->ShipGPS.dLongitude:         %+06.4f\n\r",ptrAuxStatus->ShipGPS.dLongitude);
	printf("ptrAuxStatus->ShipGPS.dLatitude:          %+06.4f\n\r",ptrAuxStatus->ShipGPS.dLatitude);
	printf("ptrAuxStatus->ShipGPS.dGroundSpeed:       %-5.2f\n\r",ptrAuxStatus->ShipGPS.dGroundSpeed);
	printf("ptrAuxStatus->ShipGPS.dCourseOverGround:  %-6.2f\n\r",ptrAuxStatus->ShipGPS.dCourseOverGround);
	printf("ptrAuxStatus->ShipWater.dSalinity:        %04.2f\n\r",ptrAuxStatus->ShipWater.dSalinity);
	printf("ptrAuxStatus->ShipWater.dWaterTemp:       %04.2f\n\r",ptrAuxStatus->ShipWater.dWaterTemp);

	printf("ptrAuxStatus->Status.Status.Word:         %04x\n\r",ptrAuxStatus->Status.Status.Word);
	ptrAuxStatus->Status.Status.Word = 0;
#endif
     }
}
/* GetShipData */

/**********************************************************************************************************/
/* GetAuxStatus                                                                                          */
/**********************************************************************************************************/

/* function to retrieve Statusinformation */
void GetAuxStatus ( struct auxStatusType *ptrAuxStatus, int IsMaster)
{

   gettimeofday(&(ptrAuxStatus->TimeOfDayAux), NULL);

   // get values from MeteoBox
   GetMeteoBoxData (ptrAuxStatus);
   GetShipData (ptrAuxStatus);
}
/* GetAuxStatus */

/**********************************************************************************************************/
/* Init UDP Ports                                                                                         */
/**********************************************************************************************************/

int InitUDPPorts(fd_set *pFDsMaster, int *fdMax)
{

   extern struct MessagePortType MessageInPortList[];
   extern struct MessagePortType MessageOutPortList[];
   int    MessagePort;

   // init inports
   for (MessagePort=0; MessagePort<MAX_MESSAGE_INPORTS;MessagePort++)
     {

	printf("opening IN Port %s on Port %d Socket:",
	       MessageInPortList[MessagePort].PortName,
	       MessageInPortList[MessagePort].PortNumber);

	MessageInPortList[MessagePort].fdSocket=InitUDPInSocket(MessageInPortList[MessagePort].PortNumber);
	FD_SET(MessageInPortList[MessagePort].fdSocket, pFDsMaster);     // add the manual port to the master set
	printf("%08x\n",MessageInPortList[MessagePort].fdSocket);
	(*fdMax)=MessageInPortList[MessagePort].fdSocket;                   // the last one will give the max number
     }
   /* for MessageInPort */

   // init outports
   for (MessagePort=0; MessagePort<MAX_MESSAGE_OUTPORTS;MessagePort++)
     {
	printf("opening OUT Port %s on Port %d, Destination IP: %s\n",
	       MessageOutPortList[MessagePort].PortName,
	       MessageOutPortList[MessagePort].PortNumber,
	       MessageOutPortList[MessagePort].IPAddr);
	MessageOutPortList[MessagePort].fdSocket=InitUDPOutSocket(MessageOutPortList[MessagePort].PortNumber);

     }
   /* for MessageOutPort */

   return(1);

}
/*InitUDPPorts*/

/**********************************************************************************************************/
/* Change Priority                                                                                        */
/**********************************************************************************************************/

int ChangePriority()
{

   struct sched_param param;
   int scheduler;
   int prio;
   struct timespec tp;
   int ret;
   int min,max;

   sched_getparam(0,&param);
   printf("sched prio  %d\n",param.sched_priority);

   printf("min max prio %d %d\n",sched_get_priority_min(SCHED_RR),sched_get_priority_max(SCHED_RR));
   sched_rr_get_interval(0, &tp);
   printf("RR Quantum : %f\n",tp.tv_nsec*1e-9);

   max=sched_get_priority_max(SCHED_RR);
   min=sched_get_priority_max(SCHED_RR);
   param.sched_priority= (int)((max-min)/2);
   if (-1==(ret=sched_setscheduler(0,SCHED_RR, &param)))
     {
	char aBuffer[1024];
	printf("kann scheduler nicht wechseln: %s\r\n",strerror(errno));
     }

   return (ret);

}
/* ChangePriority */

/**********************************************************************************************************/
/* MAIN                                                                                                   */
/**********************************************************************************************************/

int main(int argc, char *argv[])
{
   extern int errno;
   extern int StatusFlag;
   extern enum TimerSignalStateEnum TimerState;

   int IsMaster = 1;

   extern struct MessagePortType MessageInPortList[];
   extern struct MessagePortType MessageOutPortList[];

   uint64_t ulCpuClock = CPUCLOCK;

   int nLostPackets = 0;				// counter for lost packets from slave

   int fdMax;                      // max fd for select
   int i;                          // loop counter
   int fdNum;                      // fd number in loop
   fd_set fdsMaster;               // master file descriptor list
   fd_set fdsSelect;               // temp file descriptor list for select()
   int ret;
   uint64_t ProcessTick;
   uint64_t TSC,TSCin;
   uint64_t TSCsentPacket;
   uint64_t MinTimeDiff=1e6;
   uint64_t MaxTimeDiff=0;
   struct timeval StartAction;
   struct timeval StopAction;
   struct timeval LastAction;
   struct timeval GetStatusStartTime;
   struct timeval GetStatusStopTime;
   float ProcessTime;
   struct tm tmZeit;

   struct sigaction  SignalAction;
   struct sigevent   SignalEvent;
   sigset_t          SignalMask;

   struct itimerspec StatusTimer;
   timer_t           StatusTimer_id;
   clock_t           clock = CLOCK_REALTIME;
   int               StatusInterval=STATUS_INTERVAL;

   struct timespec pselect_timeout;

   struct timespec RealTime;         // Real time clock
   struct sockaddr_in my_addr;     // my address information
   struct sockaddr_in their_addr;  // connector's address information
   int    numbytes;
   socklen_t addr_len;
   char   buf[GENERIC_BUF_LEN];
   bool   EndOfSession;
   int    MessagePort;
   int    MessageNumber;
   struct ElekMessageType Message;

   int SlaveNum;
   int Task;
   int Channel;
   int MaskAddr;
   struct SyncFlagType SyncFlag;
   int RequestDataFlag;

   if (elkInit())
     {
	printf("Error: failed to grant IO access rights\n");
	exit(EXIT_FAILURE);
     };

   // setup master fd
   FD_ZERO(&fdsMaster);              // clear the master and temp sets
   FD_ZERO(&fdsSelect);
   InitUDPPorts(&fdsMaster,&fdMax);                  // Setup UDP in and out Ports

   // change scheduler and set priority
   if (-1==(ret=ChangePriority()))
     {
	SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],"elekIOaux : cannot set Priority");
     }

   if (argc==2)
     {
	// Check if we should display a summary of received data on screen
	if ((argv[1][0] == 's') || (argv[1][0] == 'S'))
	  {
	     bEnableGUI = true;
	     InitNcursesWindows();
	  }
	else
	  bEnableGUI = false;
     };

   addr_len = sizeof(struct sockaddr);

   // output version info on debugMon and Console
   //

#ifdef RUNONPC
   if(bEnableGUI)
     {
	wprintw(pStatusWin,"elekIOaux I386(CVS: $Id: elekIOaux.c,v 1.9 2007-03-11 11:17:36 rudolf Exp $)\n");
	sprintf(buf, "This is elekIOaux Version %3.2f (CVS: $Id: elekIOaux.c,v 1.9 2007-03-11 11:17:36 rudolf Exp $) for I386\n",VERSION);
	wrefresh(pStatusWin);
     }
   else
     {
	printf("This is elekIOaux Version %3.2f (CVS: $Id: elekIOaux.c,v 1.9 2007-03-11 11:17:36 rudolf Exp $) for I386\n",VERSION);
	sprintf(buf, "This is elekIOaux Version %3.2f (CVS: $Id: elekIOaux.c,v 1.9 2007-03-11 11:17:36 rudolf Exp $) for I386\n",VERSION);
     };

#else
   printf("This is elekIOaux Version %3.2f (CVS: $Id: elekIOaux.c,v 1.9 2007-03-11 11:17:36 rudolf Exp $) for ARM\n",VERSION);
   sprintf(buf, "This is elekIOaux Version %3.2f (CVS: $Id: elekIOaux.c,v 1.9 2007-03-11 11:17:36 rudolf Exp $) for ARM\n",VERSION);
#endif
   SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],buf);

   if(bEnableGUI)
     {
	wprintw(pStatusWin,"Structure size of 'AuxStatus' in bytes is: %d\n",sizeof(AuxStatus));
	wrefresh(pStatusWin);
     }
   else
     printf("Structure size of 'AuxStatus' in bytes is: %05d\r\n", sizeof(AuxStatus));

   sprintf(buf, "elekIOaux : Structure size of 'AuxStatus' in bytes is: %d", sizeof(AuxStatus));
   SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],buf);
   wrefresh(pStatusWin);
   refresh();

   /* init all modules */
   InitModules(&AuxStatus);

    /* set up signal handler */

   sigfillset(&SignalAction.sa_mask);
   SignalAction.sa_flags = 0;
   SignalAction.sa_handler = signalstatus;
   sigaction(SIGNAL_STATUS, &SignalAction, NULL);

    /* Set up timer: */
   memset(&SignalEvent, 0, sizeof(SignalEvent));
   SignalEvent.sigev_notify = SIGEV_SIGNAL;
   SignalEvent.sigev_signo = SIGNAL_STATUS;
   SignalEvent.sigev_value.sival_int = 0;

   ret= timer_create(clock, &SignalEvent, &StatusTimer_id);
   if (ret < 0)
     {
	perror("timer_create");
	return EXIT_FAILURE;
     }

   /* Start timer: */
   StatusTimer.it_interval.tv_sec =   StatusInterval / 1000;
   StatusTimer.it_interval.tv_nsec = (StatusInterval % 1000) * 1000000;
   StatusTimer.it_value = StatusTimer.it_interval;
   ret = timer_settime(StatusTimer_id, 0, &StatusTimer, NULL);
   if (ret < 0)
     {
	perror("timer_settime");
	return EXIT_FAILURE;
     }
   sigemptyset(&SignalMask);
   //    sigsuspend(&SignalMask);
   //
   gettimeofday(&LastAction, NULL);
   EndOfSession=FALSE;
   RequestDataFlag=FALSE;

   while (!EndOfSession)
     {
/*	if(bEnableGUI)
	  {
	     wprintw(pStatusWin,"Wait for data..\n");
	     wrefresh(pStatusWin);
	  }
	else
	  write(2,"Wait for data..\r",16);
*/
	fdsSelect=fdsMaster;

	pselect_timeout.tv_sec= UDP_SERVER_TIMEOUT;
	pselect_timeout.tv_nsec=0;

	ret=pselect(fdMax+1, &fdsSelect, NULL, NULL, &pselect_timeout, &SignalMask);             // wait until incoming udp or Signal

	gettimeofday(&StartAction, NULL);

#ifdef DEBUG_TIMER
	printf("Time:");
	localtime_r(&StartAction.tv_sec,&tmZeit);

	printf("%02d:%02d:%02d.%03d :%d\n", tmZeit.tm_hour, tmZeit.tm_min,
	       tmZeit.tm_sec, StartAction.tv_usec/1000,TimerState);
	printf("ret %d StatusFlag %d\n",ret,StatusFlag);
#endif

	if (ret ==-1 )
	  {
	     // select error
	     //
	     if (errno==EINTR)
	       {
		  gettimeofday(&GetStatusStartTime, NULL);
		  GetAuxStatus(&AuxStatus,IsMaster);
		  gettimeofday(&GetStatusStopTime, NULL);

		  AuxStatus.TimeOfDayAux = GetStatusStartTime;
		  // Send Status to Status process
		  SendUDPData(&MessageOutPortList[ELEK_ELEKIO_AUX_MASTER_OUT],sizeof(struct auxStatusType), &AuxStatus);
		  UpdateWindows((struct auxStatusType*)&AuxStatus);
		  AuxStatus.Status.Status.Word = 0; // mark data invalid
	       }
	     else
	       {
		  //  if(errno==EINTR)  so was not the Timer, it was a UDP Packet that caused err
		  perror("select");
		  SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],"elekIOaux: Problem with select");
	       }
	     // if errno
	  }
	else if (ret>0)
	  {

	     //	     printf("woke up...");
	     write(2,"incoming Call..\r",16);

	     for (MessagePort=0; MessagePort<MAX_MESSAGE_INPORTS;MessagePort++)
	       {

		  if (FD_ISSET(MessageInPortList[MessagePort].fdSocket,&fdsSelect))
		    {
		       // new msg on fdNum. socket ...
		       // printf("fdsSelect: %016lx\n\r",fdsSelect);
		       //		    fdElekManual=MessagePortList[MessagePort].fdSocket;
		       //		    fdElekManual=MessagePortList[0].fdSocket;
		       switch (MessagePort)
			 {
			  case ELEK_MANUAL_IN:       // port for incoming commands from  eCmd

			    if ((numbytes=recvfrom(MessageInPortList[MessagePort].fdSocket,
						   &Message,sizeof(struct ElekMessageType)  , 0,
						   (struct sockaddr *)&their_addr, &addr_len)) == -1)
			      {
				 perror("recvfrom");
				 SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],"elekIOaux: Problem with receive");
			      }

			    switch (Message.MsgType)
			      {

			       case MSG_TYPE_FETCH_DATA:  // Master want data

				 gettimeofday(&GetStatusStartTime, NULL);
				 GetAuxStatus(&AuxStatus,IsMaster);
				 gettimeofday(&GetStatusStopTime, NULL);

				 // send this debugmessage message to debugmon
				 sprintf(buf,"elekIOaux : FETCH_DATA from Port: %05d",
					 MessageInPortList[MessagePort].PortNumber,
					 Message.Addr,Message.Value,Message.Value);
				 SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],buf);

				 // send requested data, don't send any acknowledges
				 SendUDPData(&MessageOutPortList[ELEK_ELEKIO_AUX_MASTER_OUT],
					     sizeof(struct auxStatusType), &AuxStatus); // send data packet
				 break;

			       case MSG_TYPE_READ_DATA:

				 // printf("elekIOaux: manual read from Address %04x\n", Message.Addr);
				 Message.Value=elkReadData(Message.Addr);
				 Message.MsgType=MSG_TYPE_ACK;

				 sprintf(buf,"elekIOaux : ReadCmd from %05d Port %04x Value %lld (%04llx)",
					 MessageInPortList[MessagePort].PortNumber,
					 Message.Addr,Message.Value,Message.Value);
				 SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],buf);

				 sprintf(buf,"%d",MessageInPortList[MessagePort].RevMessagePort);
				 SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],buf);

				 SendUDPDataToIP(&MessageOutPortList[MessageInPortList[MessagePort].RevMessagePort],
						 inet_ntoa(their_addr.sin_addr),
						 sizeof(struct ElekMessageType), &Message);
				 break;

			       case MSG_TYPE_WRITE_DATA:

				 sprintf(buf,"elekIOaux : WriteCmd from %05d Port %04x Value %lld (%04llx)",
					 MessageInPortList[MessagePort].PortNumber,
					 Message.Addr,Message.Value,Message.Value);
				 SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],buf);
				 Message.Status=elkWriteData(Message.Addr,Message.Value);
				 Message.MsgType=MSG_TYPE_ACK;
				 SendUDPDataToIP(&MessageOutPortList[MessageInPortList[MessagePort].RevMessagePort],
						 inet_ntoa(their_addr.sin_addr),
						 sizeof(struct ElekMessageType), &Message);
				 break;

			      }

	     /* switch MsgType */
			    break;
			  default:
			    SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],"elekIOaux: unknown Port Type");
			    break;

			 }
		       // switch MessagePort
		       //		    printf("%lld got Message on Port %d from %s\n",TSC,inet_ntoa(their_addr.sin_addr));
		       //		    printf("packet is %d bytes long\n",numbytes);
		       TSCin=Message.MsgTime;
		       MaxTimeDiff= MaxTimeDiff<TSC-TSCin ? TSC-TSCin : MaxTimeDiff;
		       MinTimeDiff= MinTimeDiff>TSC-TSCin ? TSC-TSCin : MinTimeDiff;

		       //		    printf("diff : %9lld Max: %9lld Min: %9lld\n",TSC-TSCin,MaxTimeDiff,MinTimeDiff);
		       //		    printf("%9lld\n",TSC-TSCin);
		       // check for end signature
		       EndOfSession=(bool)(strstr(buf,"ende")!=NULL);
		       //		    printf("found %u\n",EndOfSession);
		    }
	 /* if fd_isset */
	       }
       /* for MessagePort */
	  }
	else
	  {
	 /* ret==0*/
	     //	    printf("timeout...\n");
	     write(2,"timeout........\r",16);
	     SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],"elekIOaux : TimeOut");
	  }

#ifdef RUNONPC
	if (timer_getoverrun(StatusTimer_id)>0)
	  {
	     printf("OVERRUN\n\r");
	     SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],"elekIOaux : Overrun");
	  }
     /* if overrun */
#endif

	gettimeofday(&StopAction, NULL);

#ifdef DEBUG_TIME_TASK
	sprintf(buf,"elekIOaux: %ld RT: %ld DT: %ld",StartAction.tv_usec/1000,
		StopAction.tv_usec-StartAction.tv_usec, (StartAction.tv_usec-LastAction.tv_usec)/1000);
	SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],buf);
#endif

	LastAction=StartAction;
     }
   /* while */

#ifdef RUNONPC
   /* delete timer */
   timer_delete(StatusTimer_id);
#endif

   // close all in bound sockets
   for (MessagePort=0; MessagePort<MAX_MESSAGE_INPORTS;MessagePort++)
     {
	close(MessageInPortList[MessagePort].fdSocket);
     }
   /*for MessagePort */

   // close all out bound sockets
   for (MessagePort=0; MessagePort<MAX_MESSAGE_OUTPORTS;MessagePort++)
     {
	close(MessageOutPortList[MessagePort].fdSocket);
     }
   /*for MessagePort */

   if (elkExit())
     {
	// release IO access
	printf("Error: failed to release IO access rights\n");
	exit(EXIT_FAILURE);
     }

   exit(EXIT_SUCCESS);
}

void InitNcursesWindows(void)
{
   initscr();                      /* Start curses mode            */
   cbreak();                       /* Line buffering disabled, Pass on*/
   refresh();
   curs_set(0);
   start_color();                  /* Start color                  */
   init_pair(1, COLOR_YELLOW, COLOR_BLACK);
   init_pair(2, COLOR_WHITE, COLOR_BLACK);
   init_pair(3, COLOR_GREEN, COLOR_BLACK);

   attron(COLOR_PAIR(2));

   pGPSWin = newwin(8, 30, 0, 0);
   wattron(pGPSWin,COLOR_PAIR(1));
   box(pGPSWin,0,0);
   mvwprintw(pGPSWin,0,1,"GPS Ship");
   wrefresh(pGPSWin);

   pMeteoBoxWin = newwin(8, 26, 00, 30);
   wattron(pMeteoBoxWin,COLOR_PAIR(1));
   box(pMeteoBoxWin,0,0);
   mvwprintw(pMeteoBoxWin,0,1,"MeteoBox");
   wrefresh(pMeteoBoxWin);

   pWaterWin = newwin(4, 24, 00, 56);
   wattron(pWaterWin,COLOR_PAIR(1));
   box(pWaterWin,0,0);
   mvwprintw(pWaterWin,0,1,"Water Ship");
   wrefresh(pWaterWin);

   pAnemoWin = newwin(4, 24, 04, 56);
   wattron(pAnemoWin,COLOR_PAIR(1));
   box(pAnemoWin,0,0);
   mvwprintw(pAnemoWin,0,1,"Anemometer Ship");
   wrefresh(pAnemoWin);

   pSonarWin = newwin(4, 30, 8, 0);
   wattron(pSonarWin,COLOR_PAIR(1));
   box(pSonarWin,0,0);
   mvwprintw(pSonarWin,0,1,"Sonar Ship");
   wrefresh(pSonarWin);

   pGyroWin = newwin(4, 26, 8, 30);
   wattron(pGyroWin,COLOR_PAIR(1));
   box(pGyroWin,0,0);
   mvwprintw(pGyroWin,0,1,"Gyro Ship");
   wrefresh(pGyroWin);
   
   pStatusBorderWin = newwin(12, 80, 12, 0);
   wattron(pStatusBorderWin,COLOR_PAIR(1));
   box(pStatusBorderWin,0,0);
   mvwprintw(pStatusBorderWin,0,36,"Messages");
   wrefresh(pStatusBorderWin);

   pStatusWin = newwin(10, 78, 13, 1);
   wrefresh(pStatusWin);
}

void UpdateWindows(struct auxStatusType *ptrAuxStatus)
{
   if(bEnableGUI)
     {
	// GPS
	//
	if(ptrAuxStatus->Status.Status.Field.ShipGPSDataValid == 1)
	  {
	     wattroff(pGPSWin,COLOR_PAIR(1));
	     wattroff(pGPSWin,COLOR_PAIR(2));
	     wattron(pGPSWin,COLOR_PAIR(3));
	  }
	else
	  {
	     wattroff(pGPSWin,COLOR_PAIR(1));
	     wattroff(pGPSWin,COLOR_PAIR(3));
	     wattron(pGPSWin,COLOR_PAIR(2));
	  };

	mvwprintw(pGPSWin,1,2,"Time (UTC):   %02d:%02d:%02d",ptrAuxStatus->ShipGPS.ucUTCHours\
             ,ptrAuxStatus->ShipGPS.ucUTCMins\
             ,ptrAuxStatus->ShipGPS.ucUTCSeconds);

	mvwprintw(pGPSWin,2,2,"Date:         %02d.%02d.%04d",ptrAuxStatus->ShipGPS.ucUTCDay\
             ,ptrAuxStatus->ShipGPS.ucUTCMonth\
             ,ptrAuxStatus->ShipGPS.uiUTCYear);

	mvwprintw(pGPSWin,3,2,"Longitude:    %+06.4f °",ptrAuxStatus->ShipGPS.dLongitude);
	mvwprintw(pGPSWin,4,2,"Latitude:     %+06.4f °",ptrAuxStatus->ShipGPS.dLatitude);

	mvwprintw(pGPSWin,5,2,"COG:          %-6.2f °",ptrAuxStatus->ShipGPS.dCourseOverGround);
	mvwprintw(pGPSWin,6,2,"Ground Speed: %-5.2f knots",ptrAuxStatus->ShipGPS.dGroundSpeed);
	wrefresh(pGPSWin);

	// Water Ship
	if(ptrAuxStatus->Status.Status.Field.ShipWaterDataValid == 1)
	  {
	     wattroff(pWaterWin,COLOR_PAIR(1));
	     wattroff(pWaterWin,COLOR_PAIR(2));
	     wattron(pWaterWin,COLOR_PAIR(3));
	  }
	else
	  {
	     wattroff(pWaterWin,COLOR_PAIR(1));
	     wattroff(pWaterWin,COLOR_PAIR(3));
	     wattron(pWaterWin,COLOR_PAIR(2));
	  };
	mvwprintw(pWaterWin,1,2,"Temp.:    %+5.2f °C",ptrAuxStatus->ShipWater.dWaterTemp);
	mvwprintw(pWaterWin,2,2,"Salinity: %-6.2f g/l",ptrAuxStatus->ShipWater.dSalinity);
	wrefresh(pWaterWin);

	// Anemo Ship
      	if(ptrAuxStatus->Status.Status.Field.ShipMeteoDataValid == 1)
	  {
	     wattroff(pAnemoWin,COLOR_PAIR(1));
	     wattroff(pAnemoWin,COLOR_PAIR(2));
	     wattron(pAnemoWin,COLOR_PAIR(3));
	  }
	else
	  {
	     wattroff(pAnemoWin,COLOR_PAIR(1));
	     wattroff(pAnemoWin,COLOR_PAIR(3));
	     wattron(pAnemoWin,COLOR_PAIR(2));
	  };
	mvwprintw(pAnemoWin,1,2,"Direction: %-6.2f °",ptrAuxStatus->ShipMeteo.dWindDirection);
	mvwprintw(pAnemoWin,2,2,"Speed:     %-2.0f m/s",ptrAuxStatus->ShipMeteo.dWindSpeed);
	wrefresh(pAnemoWin);

	// Sonar Ship
     	if(ptrAuxStatus->Status.Status.Field.ShipSonarDataValid == 1)
	  {
	     wattroff(pSonarWin,COLOR_PAIR(1));
	     wattroff(pSonarWin,COLOR_PAIR(2));
	     wattron(pSonarWin,COLOR_PAIR(3));
	  }
	else
	  {
	     wattroff(pSonarWin,COLOR_PAIR(1));
	     wattroff(pSonarWin,COLOR_PAIR(3));
	     wattron(pSonarWin,COLOR_PAIR(2));
	  };

	mvwprintw(pSonarWin,1,2,"Frequency:  %-6.4f Khz",ptrAuxStatus->ShipSonar.dFrequency);
	mvwprintw(pSonarWin,2,2,"Waterdepth: %-8.2f m",ptrAuxStatus->ShipSonar.dWaterDepth);
	wrefresh(pSonarWin);

	// Gyro Ship
     	if(ptrAuxStatus->Status.Status.Field.ShipGyroDataValid == 1)
	  {
	     wattroff(pGyroWin,COLOR_PAIR(1));
	     wattroff(pGyroWin,COLOR_PAIR(2));
	     wattron(pGyroWin,COLOR_PAIR(3));
	  }
	else
	  {
	     wattroff(pGyroWin,COLOR_PAIR(1));
	     wattroff(pGyroWin,COLOR_PAIR(3));
	     wattron(pGyroWin,COLOR_PAIR(2));
	  };

	mvwprintw(pGyroWin,1,2,"Heading:  %03.0f °",ptrAuxStatus->ShipGyro.dDirection);
	wrefresh(pGyroWin);

	// MeteoBox
	if(ptrAuxStatus->Status.Status.Field.MeteoBoxDataValid == 1)
	  {
	     wattroff(pMeteoBoxWin,COLOR_PAIR(1));
	     wattroff(pMeteoBoxWin,COLOR_PAIR(2));
	     wattron(pMeteoBoxWin,COLOR_PAIR(3));
	  }
	else
	  {
	     wattroff(pMeteoBoxWin,COLOR_PAIR(1));
	     wattroff(pMeteoBoxWin,COLOR_PAIR(3));
	     wattron(pMeteoBoxWin,COLOR_PAIR(2));
	  };
	mvwprintw(pMeteoBoxWin,1,2,"Windspeed:  %-6.2f m/s",ptrAuxStatus->MeteoBox.dWindSpeed);
	mvwprintw(pMeteoBoxWin,2,2,"Windspeed:  %-6.2f km/h",ptrAuxStatus->MeteoBox.dWindSpeed*3.6f);
	mvwprintw(pMeteoBoxWin,3,2,"Wind Dir:   %-3d °",ptrAuxStatus->MeteoBox.uiWindDirection);
	mvwprintw(pMeteoBoxWin,4,2,"Rel. Humid: %-5.2f %",ptrAuxStatus->MeteoBox.dRelHum);
	mvwprintw(pMeteoBoxWin,5,2,"Air Temp:   %+05.2f °C",ptrAuxStatus->MeteoBox.dAirTemp);
	mvwprintw(pMeteoBoxWin,6,2,"Gas Sensor: %-5.3f V",ptrAuxStatus->MeteoBox.dGasSensorVoltage);
	wrefresh(pMeteoBoxWin);
     };
};
