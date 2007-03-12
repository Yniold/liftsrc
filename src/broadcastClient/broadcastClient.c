/*
 *
 $RCSfile: broadcastClient.c,v $ last changed on $Date: 2007-03-12 13:49:25 $ by $Author: rudolf $
 *
 * $Log: broadcastClient.c,v $
 * Revision 1.1  2007-03-12 13:49:25  rudolf
 * created a small client for the broadcast data incl. ETA to durban
 *
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
#include "broadcastClient.h"

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
   ELEK_AUX_IN,
     MAX_MESSAGE_INPORTS
};

enum OutPortListEnum
{
   ELEK_DEBUG_OUT,
     MAX_MESSAGE_OUTPORTS
};

struct MessagePortType MessageInPortList[MAX_MESSAGE_INPORTS]=
{
   // order in list defines sequence of polling
    /* Name           , PortNo                        , ReversePort    , IPAddr      , fdSocket, MaxMsg, Direction */
     {"ElekIOauxIn"   , UDP_ELEK_AUX_INPORT            , -1             , IP_LOCALHOST, -1, 0,  UDP_IN_PORT}
};

struct MessagePortType MessageOutPortList[MAX_MESSAGE_OUTPORTS]=
{
   // order in list defines sequence of polling
    /* Name           ,PortNo                        , ReversePort        , IPAddr          , fdSocket, MaxMsg, Direction */
     {"DebugPort"     ,UDP_ELEK_DEBUG_OUTPORT        , -1                 , IP_DEBUG_CLIENT , -1      , 0     ,  UDP_OUT_PORT}
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
WINDOW* pDistWin;
WINDOW* pETAWin;
WINDOW* pAvgWin;
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
   struct auxStatusType AuxStatus;

   int SlaveNum;
   int Task;
   int Channel;
   int MaskAddr;
   struct SyncFlagType SyncFlag;
   int RequestDataFlag;

   // for ETA calculations
   //

#define NAUTICAL_MILE (1.852f)

   int iSeconds;

   double StartLatitude,StartLongitude,StopLatitude,StopLongitude;

   double AcosArgument;
   double AcosArgument2;
   double AcosResult;

   double DistanceInKm;
   double DistanceInMiles;
   double DistanceInHours;

   double dShipSpeed;
   struct timeval Now;
   struct timeval Arrival;

   // setup master fd
   FD_ZERO(&fdsMaster);              // clear the master and temp sets
   FD_ZERO(&fdsSelect);
   InitUDPPorts(&fdsMaster,&fdMax);                  // Setup UDP in and out Ports

   // change scheduler and set priority
   if (-1==(ret=ChangePriority()))
     {
	SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],"broadcastClient : cannot set Priority");
     }

   InitNcursesWindows();
   bEnableGUI = true;

   addr_len = sizeof(struct sockaddr);

   // output version info on debugMon and Console
   //

#ifdef RUNONPC
   if(bEnableGUI)
     {
	wprintw(pStatusWin,"broadcastClient I386(CVS: $Id: broadcastClient.c,v 1.1 2007-03-12 13:49:25 rudolf Exp $)\n");
	sprintf(buf, "This is broadcastClient Version %3.2f (CVS: $Id: broadcastClient.c,v 1.1 2007-03-12 13:49:25 rudolf Exp $) for I386\n",VERSION);
	wrefresh(pStatusWin);
     }
   else
     {
	printf("This is broadcastClient Version %3.2f (CVS: $Id: broadcastClient.c,v 1.1 2007-03-12 13:49:25 rudolf Exp $) for I386\n",VERSION);
	sprintf(buf, "This is broadcastClient Version %3.2f (CVS: $Id: broadcastClient.c,v 1.1 2007-03-12 13:49:25 rudolf Exp $) for I386\n",VERSION);
     };

#else
   printf("This is broadcastClient Version %3.2f (CVS: $Id: broadcastClient.c,v 1.1 2007-03-12 13:49:25 rudolf Exp $) for ARM\n",VERSION);
   sprintf(buf, "This is broadcastClient Version %3.2f (CVS: $Id: broadcastClient.c,v 1.1 2007-03-12 13:49:25 rudolf Exp $) for ARM\n",VERSION);
#endif
   SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],buf);

   if(bEnableGUI)
     {
	wprintw(pStatusWin,"Structure size of 'AuxStatus' in bytes is: %d\n",sizeof(AuxStatus));
	wrefresh(pStatusWin);
     }
   else
     printf("Structure size of 'AuxStatus' in bytes is: %05d\r\n", sizeof(AuxStatus));

   sprintf(buf, "broadcastClient : Structure size of 'AuxStatus' in bytes is: %d", sizeof(AuxStatus));
   SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],buf);
   wrefresh(pStatusWin);
   refresh();

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
	fdsSelect=fdsMaster;

	pselect_timeout.tv_sec= UDP_SERVER_TIMEOUT;
	pselect_timeout.tv_nsec=0;

	ret=pselect(fdMax+1, &fdsSelect, NULL, NULL, &pselect_timeout, &SignalMask);             // wait until incoming udp or Signal

	gettimeofday(&StartAction, NULL);

	if (ret ==-1 )
	  {
	     // select error
	     //
	     if (errno==EINTR)
	       {
	       }
	     else
	       {
		  //  if(errno==EINTR)  so was not the Timer, it was a UDP Packet that caused err
		  perror("select");
		  SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],"broadcastClient: Problem with select");
	       }
	     // if errno
	  }
	else if (ret>0)
	  {
	     for (MessagePort=0; MessagePort<MAX_MESSAGE_INPORTS;MessagePort++)
	       {

		  if (FD_ISSET(MessageInPortList[MessagePort].fdSocket,&fdsSelect))
		    {
		       switch (MessagePort)
			 {
			  case ELEK_AUX_IN:       // port for incoming commands from  eCmd

			    if ((numbytes=recvfrom(MessageInPortList[MessagePort].fdSocket,
						   &AuxStatus,sizeof(struct auxStatusType)  , 0,
						   (struct sockaddr *)&their_addr, &addr_len)) == -1)
			      {
				 perror("recvfrom");
				 SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],"broadcastClient: Problem with receive");
			      }
			    UpdateWindows((struct auxStatusType*)&AuxStatus);

			    break;

			  default:
			    SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],"broadcastClient: unknown Port Type");
			    break;

			 }
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
	     SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],"broadcastClient : TimeOut");
	  }

#ifdef RUNONPC
	if (timer_getoverrun(StatusTimer_id)>0)
	  {
	     printf("OVERRUN\n\r");
	     SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],"broadcastClient : Overrun");
	  }
     /* if overrun */
#endif

	gettimeofday(&StopAction, NULL);

#ifdef DEBUG_TIME_TASK
	sprintf(buf,"broadcastClient: %ld RT: %ld DT: %ld",StartAction.tv_usec/1000,
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

   pAvgWin = newwin(4, 24, 8, 56);
   wattron(pAvgWin,COLOR_PAIR(1));
   box(pAvgWin,0,0);
   mvwprintw(pAvgWin,0,1,"Average (last 5 min)");
   wrefresh(pAvgWin);

   pDistWin = newwin(4, 30, 12, 0);
   wattron(pDistWin,COLOR_PAIR(1));
   box(pDistWin,0,0);
   mvwprintw(pDistWin,0,1,"Isses noch weit? [TM]");
   wrefresh(pDistWin);

   pETAWin = newwin(4, 50, 12, 30);
   wattron(pETAWin,COLOR_PAIR(1));
   box(pETAWin,0,0);
   mvwprintw(pETAWin,0,1,"Dauerts noch lang? [TM]");
   wrefresh(pETAWin);

   pStatusBorderWin = newwin(8, 80, 16, 0);
   wattron(pStatusBorderWin,COLOR_PAIR(1));
   box(pStatusBorderWin,0,0);
   mvwprintw(pStatusBorderWin,0,36,"Messages");
   wrefresh(pStatusBorderWin);

   pStatusWin = newwin(6, 78, 17, 1);
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
	     CalculateETA(ptrAuxStatus);
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
#define NAUTICAL_MILE (1.852f)

double DegToRad(double dDegrees)
{
   return (dDegrees*3.1415926535897932384626433832795028841971693993751058209749445923078164062862089986280348253421170679f/180);
};

int CalculateETA(struct auxStatusType *ptrAuxStatus)
{
   // for ETA calculations
   //

   static double aSpeed[300];
#define NAUTICAL_MILE (1.852f)

   int iSeconds;

   double StartLatitude,StartLongitude,StopLatitude,StopLongitude;

   double AcosArgument;
   double AcosArgument2;
   double AcosResult;

   double DistanceInKmDurban;
   double DistanceInMilesDurban;
   double DistanceInHoursDurban;

   double DistanceInKmReunion;
   double DistanceInMilesReunion;
   double DistanceInHoursReunion;

   double dShipSpeed;
   struct timeval Durban;
   struct timeval Reunion;
   double dStartLat, dStartLon, LocStartLatitude,LocStartLongitude,LocStopLatitude,LocStopLongitude;

   dStartLon = ptrAuxStatus->ShipGPS.dLongitude;
   dStartLat = ptrAuxStatus->ShipGPS.dLatitude;

   // START POSITION
   // // convert in absolute degree value
   int iStartLatNonFrac = dStartLat/100;
   int iStartLonNonFrac = dStartLon/100;

   double dStartLatFrac  = (dStartLat) -((double)(iStartLatNonFrac*100));
   double dStartLonFrac  = (dStartLon)-((double)(iStartLonNonFrac*100));
   dStartLatFrac = dStartLatFrac / 60;
   dStartLonFrac = dStartLonFrac / 60;
   //
   // check sign to do proper math
   if(dStartLat>=0)
     {
	LocStartLatitude  = ((double)iStartLatNonFrac)+dStartLatFrac;
     }
   else
     {
	LocStartLatitude  = ((double)-iStartLatNonFrac)-dStartLatFrac;
	LocStartLatitude  = -LocStartLatitude;
     };

   if(dStartLon>=0)
     {
	LocStartLongitude = ((double)iStartLonNonFrac)+dStartLonFrac;
     }
   else
     {
	LocStartLongitude = ((double)-iStartLonNonFrac)-dStartLonFrac;
	LocStartLongitude = -LocStartLongitude;
     };
   //
   StartLatitude 	= LocStartLatitude;	// Breite Start
   StartLongitude 	= LocStartLongitude;

   StopLatitude	        = -29.86467240084049;	// Durban
   StopLongitude	= 31.0209422124315f;

   AcosArgument =  sin(DegToRad(StartLatitude)) * sin(DegToRad(StopLatitude));
   AcosArgument += cos(DegToRad(StartLatitude)) * cos(DegToRad(StopLatitude)) * cos(DegToRad(fabs(StopLongitude-StartLongitude)));
   DistanceInKmDurban = acos(AcosArgument) * 6370;

   StartLatitude 	= LocStartLatitude;	// Breite Start
   StartLongitude 	= LocStartLongitude;

   StopLatitude	        = -20.9291344241915;	// Le Port
   StopLongitude	= 55.31561751544788;

   AcosArgument =  sin(DegToRad(StartLatitude)) * sin(DegToRad(StopLatitude));
   AcosArgument += cos(DegToRad(StartLatitude)) * cos(DegToRad(StopLatitude)) * cos(DegToRad(fabs(StopLongitude-StartLongitude)));
   DistanceInKmReunion = acos(AcosArgument) * 6370;

   gettimeofday(&Durban,NULL);
   gettimeofday(&Reunion,NULL);

   // printf("Current time is %s\r\n",ctime(&Now.tv_sec));
   //
   wattroff(pDistWin,COLOR_PAIR(1));
   wattroff(pDistWin,COLOR_PAIR(3));
   wattron(pDistWin,COLOR_PAIR(2));

   mvwprintw(pDistWin,1,2,"Durban:  %6.2f km",DistanceInKmDurban);
   mvwprintw(pDistWin,2,2,"Reunion: %6.2f km",DistanceInKmReunion);
   wrefresh(pDistWin);

   DistanceInMilesDurban = (DistanceInKmDurban / NAUTICAL_MILE);
   DistanceInMilesReunion = (DistanceInKmReunion / NAUTICAL_MILE);
   // printf("That's %6.2f nautical miles\r\n",DistanceInMiles);
   //
   int iLoop = 0;
   double dRunningAverage = 0;

   for(iLoop = 1; iLoop < 300;iLoop++)
     aSpeed[iLoop-1]=aSpeed[iLoop];

   aSpeed[299]=ptrAuxStatus->ShipGPS.dGroundSpeed;

   for(iLoop = 0;iLoop < 300; iLoop++)
     dRunningAverage += aSpeed[iLoop];

   dRunningAverage = dRunningAverage / 300.0f;
   wattroff(pAvgWin,COLOR_PAIR(1));
   wattroff(pAvgWin,COLOR_PAIR(3));
   wattron(pAvgWin,COLOR_PAIR(2));
   mvwprintw(pAvgWin,1,2,"Speed: %05.2f knots",dRunningAverage);
   wrefresh(pAvgWin);
   if(dRunningAverage > 0)
     {

	char aBuffer[256];

	DistanceInHoursDurban = DistanceInMilesDurban / dRunningAverage; //ptrAuxStatus->ShipGPS.dGroundSpeed;
	iSeconds = (int)(DistanceInHoursDurban*3600);
        Durban.tv_sec += iSeconds;

	DistanceInHoursReunion = DistanceInMilesReunion / dRunningAverage; //ptrAuxStatus->ShipGPS.dGroundSpeed;
	iSeconds = (int)(DistanceInHoursReunion*3600);
        Reunion.tv_sec += iSeconds;
	wattroff(pETAWin,COLOR_PAIR(1));
	wattroff(pETAWin,COLOR_PAIR(3));
	wattron(pETAWin,COLOR_PAIR(2));
	snprintf(aBuffer,20,"%s",ctime(&Durban.tv_sec));
        mvwprintw(pETAWin,1,2,"ETA Durban  (UTC): %s",aBuffer);
	snprintf(aBuffer,20,"%s",ctime(&Reunion.tv_sec));
        mvwprintw(pETAWin,2,2,"ETA Reunion (UTC): %s",aBuffer);
	wrefresh(pETAWin);
     };
   return 0;
}

