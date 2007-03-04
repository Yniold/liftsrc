/*
 * $RCSfile: elekIOaux.c,v $ last changed on $Date: 2007-03-04 13:41:59 $ by $Author: rudolf $
 *
 * $Log: elekIOaux.c,v $
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

#include "../include/elekGeneral.h"
#include "../include/elekIO.h"
#include "../include/elekIOPorts.h"
#include "elekIOaux.h"
#include "meteobox.h"

#define STATUS_INTERVAL  200

#define DEBUGLEVEL 0
#undef DEBUG_TIMER

#define CPUCLOCK 500000000UL 	// CPU clock of Markus' Athlon XP

enum InPortListEnum
{
   // this list has to be coherent with MessageInPortList
     ELEK_MANUAL_IN,       // port for incoming commands from  eCmd
     ELEK_ETALON_IN,       // port for incoming commands from  etalon
     ELEK_SCRIPT_IN,       // port for incoming commands from  scripting host (not yet existing, HH, Feb2005
     //     ELEK_STATUS_IN,       // port to receive status data from slaves
     MAX_MESSAGE_INPORTS
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

static struct MessagePortType MessageInPortList[MAX_MESSAGE_INPORTS]=
{
   // order in list defines sequence of polling
    /* Name   , PortNo                    , ReversePort  , IPAddr, fdSocket, MaxMsg, Direction */
     {"Manual"        , UDP_ELEK_MANUAL_INPORT        , ELEK_MANUAL_OUT, IP_LOCALHOST, -1, 1,  UDP_IN_PORT},
     {"Etalon"        , UDP_ELEK_ETALON_INPORT        , ELEK_ETALON_OUT, IP_LOCALHOST, -1, 10, UDP_IN_PORT},
     {"Script"        , UDP_ELEK_SCRIPT_INPORT        , ELEK_SCRIPT_OUT, IP_LOCALHOST, -1, 5,  UDP_IN_PORT},
   //     {"ElekIOCalibIn" , UDP_ELEK_CALIB_DATA_INPORT    , -1             , IP_LOCALHOST, -1, 1,  UDP_IN_PORT} // status inport from elekIOServ
};

static struct MessagePortType MessageOutPortList[MAX_MESSAGE_OUTPORTS]=
{
   // order in list defines sequence of polling
    /* Name           ,PortNo                        , ReversePort        , IPAddr, fdSocket, MaxMsg, Direction */
//     {"Status"        ,UDP_CALIB_STATUS_STATUS_OUTPORT, -1                   , "10.111.111.188", -1, 0,  UDP_OUT_PORT},
     {"Status"        ,UDP_CALIB_STATUS_STATUS_OUTPORT, -1                   , IP_STATUS_CLIENT, -1, 0,  UDP_OUT_PORT},
     {"ElekIOStatus"  ,UDP_ELEK_SLAVE_DATA_INPORT    , -1                    , IP_ELEKIO_MASTER, -1, 0,  UDP_OUT_PORT},
     {"ElekIOServer"  ,UDP_ELEK_MANUAL_INPORT        , ELEK_ELEKIO_STATUS_OUT, IP_ELEK_SERVER  , -1, 0,  UDP_OUT_PORT},
     {"Manual"        ,UDP_ELEK_MANUAL_OUTPORT       , ELEK_MANUAL_IN        , IP_LOCALHOST    , -1, 0,  UDP_OUT_PORT},
     {"Etalon"        ,UDP_ELEK_ETALON_OUTPORT       , -1                    , IP_ETALON_CLIENT, -1, 0,  UDP_OUT_PORT},
     {"EtalonStatus"  ,UDP_ELEK_ETALON_STATUS_OUTPORT, -1                    , IP_STATUS_CLIENT, -1, 0,  UDP_OUT_PORT},
     {"Script"        ,UDP_ELEK_SCRIPT_OUTPORT       , ELEK_SCRIPT_IN        , IP_SCRIPT_CLIENT, -1, 0,  UDP_OUT_PORT},
     {"DebugPort"     ,UDP_ELEK_DEBUG_OUTPORT        , -1                    , IP_DEBUG_CLIENT , -1, 0,  UDP_OUT_PORT},
     {"ElekIOOut"     ,UDP_ELEK_SLAVE_DATA_INPORT    , -1                    , IP_ELEKIO_MASTER, -1, 0,  UDP_OUT_PORT},
     {"ElekIOcalibOut",UDP_ELEK_CALIB_DATA_INPORT    , -1                    , IP_ELEKIO_MASTER, -1, 0,  UDP_OUT_PORT}
};

static struct TaskListType TasktoWakeList[MAX_TASKS_TO_WAKE]=
{
   // order defines sequence of wake up after timer
    /* TaskName TaskConn TaskWantStatusOnPort */
     {"Etalon",     ELEK_ETALON_OUT,ELEK_ETALON_STATUS_OUT},    // Etalon Task needs Status info
     {"Script",     ELEK_SCRIPT_OUT,                    -1},
     {      "",                  -1,                    -1}
};


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
	sprintf(debugbuf,"elekIOaux : Can't create LICOR Thread!\n\r");
	SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],debugbuf);

	return (INIT_MODULE_FAILED);
     };

   // success
   sprintf(debugbuf,"elekIOaux: MeteoBox Thread running!\n\r");
   SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],debugbuf);

   return (INIT_MODULE_SUCCESS);
}
/* Init Butterfly */

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
}
/* InitModules */


/**********************************************************************************************************/
/* GetMeteoBoxData                                                                                        */
/**********************************************************************************************************/

void GetMeteoBoxData ( struct auxStatusType *ptrAuxStatus)
{

   extern struct MessagePortType MessageInPortList[];
   extern struct MessagePortType MessageOutPortList[];
   extern pthread_mutex_t mLicorMutex;
   extern struct sLicorType sLicorThread;

   uint16_t       ret;
   uint16_t       Control;
   char           buf[GENERIC_BUF_LEN];

     {
	pthread_mutex_lock(&mMeteoBoxMutex);
//	ptrAuxStatus->MeteoBox.LicorTemperature = sMeteoThread.LicorTemperature;
	
//	ptrAuxStatus->LicorCalib.AmbientPressure = sMeteoThread.AmbientPressure;

//	ptrAuxStatus->LicorCalib.CO2A = sLicorThread.CO2A;
//	ptrAuxStatus->LicorCalib.CO2B = sLicorThread.CO2B; /* CO2 concentration cell B in mymol/mol, coding scheme T.B.D. */
//	ptrAuxStatus->LicorCalib.CO2D = sLicorThread.CO2D; /* CO2 differential concentration in mymol/mol, coding scheme T.B.D. */

//	ptrAuxStatus->LicorCalib.H2OA = sLicorThread.H2OA; /* H2O concentration cell A in mmol/mol, coding scheme T.B.D. */
//	ptrAuxStatus->LicorCalib.H2OB = sLicorThread.H2OB; /* H2O concentration cell B in mmol/mol, coding scheme T.B.D. */
//	ptrAuxStatus->LicorCalib.H2OD = sLicorThread.H2OD; /* H2O differential concentration in mmol/mol, coding scheme T.B.D. */
//
	pthread_mutex_unlock(&mMeteoBoxMutex);

#ifdef DEBUG_STRUCTUREPASSING
	printf("ptrAuxStatus->LicorCalib.LicorTemperature: %05d\n\r",ptrAuxStatus->LicorCalib.LicorTemperature);
	printf("ptrAuxStatus->LicorCalib.AmbientPressure:  %05d\n\r",ptrAuxStatus->LicorCalib.AmbientPressure);

	printf("ptrAuxStatus->LicorCalib.CO2A:             %05d\n\r",ptrAuxStatus->LicorCalib.CO2A);
	printf("ptrAuxStatus->LicorCalib.CO2B:             %05d\n\r",ptrAuxStatus->LicorCalib.CO2B);
	printf("ptrAuxStatus->LicorCalib.CO2D:             %05d\n\r\n\r",ptrAuxStatus->LicorCalib.CO2D);

	printf("ptrAuxStatus->LicorCalib.H2OA:             %05d\n\r",ptrAuxStatus->LicorCalib.H2OA);
	printf("ptrAuxStatus->LicorCalib.H2OB:             %05d\n\r",ptrAuxStatus->LicorCalib.H2OB);
	printf("ptrAuxStatus->LicorCalib.H2OD:             %05d\n\r\n\r",ptrAuxStatus->LicorCalib.H2OD);

#endif
     }
}
/* GetLicorData */

/**********************************************************************************************************/
/* GetAuxStatus                                                                                          */
/**********************************************************************************************************/

/* function to retrieve Statusinformation */
void GetAuxStatus ( struct auxStatusType *ptrAuxStatus, int IsMaster)
{

   gettimeofday(&(ptrAuxStatus->TimeOfDayAux), NULL);

   // get values from MeteoBox
   GetMeteoBoxData (ptrAuxStatus);

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
	printf("kann scheduler nicht wechseln: %s",strerror(errno));
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
	// grant IO access
	printf("Error: failed to grant IO access rights\n");
	exit(EXIT_FAILURE);
     }

   // setup master fd
   FD_ZERO(&fdsMaster);              // clear the master and temp sets
   FD_ZERO(&fdsSelect);
   InitUDPPorts(&fdsMaster,&fdMax);                  // Setup UDP in and out Ports

   addr_len = sizeof(struct sockaddr);

   // output version info on debugMon and Console
   //

#ifdef RUNONPC
   printf("This is elekIOaux Version %3.2f (CVS: $Id: elekIOaux.c,v 1.1 2007-03-04 13:41:59 rudolf Exp $) for I386\n",VERSION);
   sprintf(buf, "This is elekIOaux Version %3.2f (CVS: $Id: elekIOaux.c,v 1.1 2007-03-04 13:41:59 rudolf Exp $) for I386\n",VERSION);
#else
   printf("This is elekIOaux Version %3.2f (CVS: $Id: elekIOaux.c,v 1.1 2007-03-04 13:41:59 rudolf Exp $) for ARM\n",VERSION);
   sprintf(buf, "This is elekIOaux Version %3.2f (CVS: $Id: elekIOaux.c,v 1.1 2007-03-04 13:41:59 rudolf Exp $) for ARM\n",VERSION);
#endif
   SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],buf);

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
   
   // change scheduler and set priority
   if (-1==(ret=ChangePriority()))
     {
       SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],"elekIOaux : cannot set Priority");
     }
   
   sigemptyset(&SignalMask);
   //    sigsuspend(&SignalMask);
   //
   gettimeofday(&LastAction, NULL);
   EndOfSession=FALSE;
   RequestDataFlag=FALSE;
   
   while (!EndOfSession) {
     write(2,"Wait for data..\r",16);
     
     fdsSelect=fdsMaster;
     
     pselect_timeout.tv_sec= UDP_SERVER_TIMEOUT;
     pselect_timeout.tv_nsec=0;
     
     ret=pselect(fdMax+1, &fdsSelect, NULL, NULL, &pselect_timeout, &SignalMask);             // wiat until incoming udp or Signal
     
     gettimeofday(&StartAction, NULL);
     
#ifdef DEBUG_TIMER
     printf("Time:");
     localtime_r(&StartAction.tv_sec,&tmZeit);
     
     printf("%02d:%02d:%02d.%03d :%d\n", tmZeit.tm_hour, tmZeit.tm_min,
	    tmZeit.tm_sec, StartAction.tv_usec/1000,TimerState);
     printf("ret %d StatusFlag %d\n",ret,StatusFlag);
#endif
     
     if (ret ==-1 ) {
       // select error
       //
       if (errno==EINTR) {
	 // got interrupted by timer
	 if ((StatusFlag % 50)==0) {			
	   // printf("get Status %6d..\r",StatusFlag);
	   write(2,"get Status.....\r",16);
	 }			
	 gettimeofday(&GetStatusStartTime, NULL);
	 GetAuxStatus(&AuxStatus,IsMaster);
	 gettimeofday(&GetStatusStopTime, NULL);

	  // Send Status to Status process
     SendUDPData(&MessageOutPortList[CALIB_STATUS_OUT],sizeof(struct auxStatusType), &AuxStatus);
       } else { 
	 //  if(errno==EINTR)  so was not the Timer, it was a UDP Packet that caused err
	 perror("select");
	 SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],"elekIOaux: Problem with select");
       } // if errno
     } else if (ret>0) {
       
       //	     printf("woke up...");
       write(2,"incoming Call..\r",16);
       
       for (MessagePort=0; MessagePort<MAX_MESSAGE_INPORTS;MessagePort++) {
	 
	 if (FD_ISSET(MessageInPortList[MessagePort].fdSocket,&fdsSelect)) {
	   // new msg on fdNum. socket ...
	   // printf("fdsSelect: %016lx\n\r",fdsSelect);
	   //		    fdElekManual=MessagePortList[MessagePort].fdSocket;
	   //		    fdElekManual=MessagePortList[0].fdSocket;
	   switch (MessagePort) {
	   case ELEK_MANUAL_IN:       // port for incoming commands from  eCmd
	     //			  case ELEK_ETALON_IN:       // port for incoming commands from  etalon
	   case ELEK_SCRIPT_IN:       // port for incoming commands from  scripting host (not yet existing)
	     
	     if ((numbytes=recvfrom(MessageInPortList[MessagePort].fdSocket,
				    &Message,sizeof(struct ElekMessageType)  , 0,
				    (struct sockaddr *)&their_addr, &addr_len)) == -1)
	       {
		 perror("recvfrom");
		 SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],"elekIOaux: Problem with receive");
	       }
#ifdef DEBUG_SLAVECOM
	     sprintf(buf,"recv command from %s on port %d",inet_ntoa(their_addr.sin_addr),
		     ntohs(their_addr.sin_port));
	     SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],buf);
#endif
	     
	     switch (Message.MsgType)
	       {
		 
	       case MSG_TYPE_FETCH_DATA:  // Master want data
#ifdef DEBUG_SLAVECOM
		 printf("elekIOaux: FETCH_DATA received, TIME_T  was: %016lx\n\r", Message.MsgTime);
#endif
		 
#ifdef DEBUG_SLAVECOM
		 printf("elekIOaux: gathering Data...\n\r");
#endif
		 gettimeofday(&GetStatusStartTime, NULL);
		 GetAuxStatus(&AuxStatus,IsMaster);
		 gettimeofday(&GetStatusStopTime, NULL);
#ifdef DEBUG_SLAVECOM
		 printf("elekIOaux: Data aquisition took: %02d.%03ds\n\r",
			GetStatusStopTime.tv_sec-GetStatusStartTime.tv_sec,
			(GetStatusStopTime.tv_usec-GetStatusStartTime.tv_usec)/1000);
#endif
		 // send this debugmessage message to debugmon
		 sprintf(buf,"elekIOaux : FETCH_DATA from Port: %05d",
			 MessageInPortList[MessagePort].PortNumber,
			 Message.Addr,Message.Value,Message.Value);
		 SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],buf);
		 
		 // send requested data, don't send any acknowledges
		 SendUDPData(&MessageOutPortList[ELEK_ELEKIO_CALIB_MASTER_OUT],
			     sizeof(struct auxStatusType), &AuxStatus); // send data packet
		 break;
		 
	       case MSG_TYPE_READ_DATA:
		 // printf("elekIOaux: manual read from Address %04x\n", Message.Addr);
		 Message.Value=elkReadData(Message.Addr);
		 Message.MsgType=MSG_TYPE_ACK;
		 
		 if (MessagePort!=ELEK_ETALON_IN)
		   {
		     sprintf(buf,"elekIOaux : ReadCmd from %05d Port %04x Value %d (%04x)",
			     MessageInPortList[MessagePort].PortNumber,
			     Message.Addr,Message.Value,Message.Value);
		     SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],buf);
		     
		     sprintf(buf,"%d",MessageInPortList[MessagePort].RevMessagePort);
		     SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],buf);
		   }
		 
		 SendUDPDataToIP(&MessageOutPortList[MessageInPortList[MessagePort].RevMessagePort],
				 inet_ntoa(their_addr.sin_addr),
				 sizeof(struct ElekMessageType), &Message);
		 break;
	       case MSG_TYPE_WRITE_DATA:
		 if (MessagePort!=ELEK_ETALON_IN)
		   {
		     sprintf(buf,"elekIOaux : WriteCmd from %05d Port %04x Value %d (%04x)",
			     MessageInPortList[MessagePort].PortNumber,
			     Message.Addr,Message.Value,Message.Value);
		     SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],buf);
		   }
		 
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

