#define VERSION 0.10f
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

#include <sys/ipc.h>
#include <sys/shm.h>

#include "../include/elekGeneral.h"
#include "../include/elekIO.h"
#include "../include/elekIOPorts.h"
#include "elekIOGSB.h"
#include "GSB_IO_thread.h"

#define STATUS_INTERVAL  200

#define DEBUGLEVEL 0
// #define DEBUG_SLAVECOM
#undef DEBUG_TIMER

#define CPUCLOCK 200000000UL 	// CPU clock of ARM9

#define STRINGIFY_(x) #x
#define STRINGIFY(x) STRINGIFY_(x)
 
#if defined(__AVR__)
#define CPUTYPE "AVR"

#elif defined (__arm__)
#define CPUTYPE "ARM"

#elif defined (__i386__)
#define CPUTYPE "Intel i386"

#else
#define CPUTYPE ""
#endif
 
#if defined(__GNUC__)
#if defined(__GNUC_PATCHLEVEL__)
#define COMPILER CPUTYPE "-" "GCC" " " STRINGIFY(__GNUC__) "." STRINGIFY(__GNUC_MINOR__) "." STRINGIFY(__GNUC_PATCHLEVEL__)
#else
#define COMPILER CPUTYPE "-" "GCC" " " STRINGIFY(__GNUC__) "." STRINGIFY(__GNUC_MINOR__)
#endif
#endif

struct GSBStatusType* pGSBStatus;

enum InPortListEnum
{
   // this list has to be coherent with MessageInPortList
     ELEK_MANUAL_IN,       // port for incoming commands from  eCmd
     ELEK_ETALON_IN,       // port for incoming commands from  etalon
     ELEK_SCRIPT_IN,       // port for incoming commands from  scripting host (not yet existing, HH, Feb2005
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
     ELEK_ELEKIO_GSB_MASTER_OUT,     // port for outgoing data packets from GSB to master
     MAX_MESSAGE_OUTPORTS
};

static struct MessagePortType MessageInPortList[MAX_MESSAGE_INPORTS]=
{
   // order in list defines sequence of polling
    /* Name   , PortNo                    , ReversePort  , IPAddr, fdSocket, MaxMsg, Direction */
     {"Manual"        , UDP_ELEK_MANUAL_INPORT        , ELEK_MANUAL_OUT, IP_LOCALHOST, -1, 1,  UDP_IN_PORT},
     {"Etalon"        , UDP_ELEK_ETALON_INPORT        , ELEK_ETALON_OUT, IP_LOCALHOST, -1, 10, UDP_IN_PORT},
     {"Script"        , UDP_ELEK_SCRIPT_INPORT        , ELEK_SCRIPT_OUT, IP_LOCALHOST, -1, 5,  UDP_IN_PORT}
};

static struct MessagePortType MessageOutPortList[MAX_MESSAGE_OUTPORTS]=
{
   // order in list defines sequence of polling
    /* Name           ,PortNo                        , ReversePort        , IPAddr, fdSocket, MaxMsg, Direction */
     {"Status"        ,UDP_CALIB_STATUS_STATUS_OUTPORT, -1                   , IP_STATUS_CLIENT, -1, 0,  UDP_OUT_PORT},
     {"ElekIOStatus"  ,UDP_ELEK_SLAVE_DATA_INPORT    , -1                    , IP_ELEKIO_MASTER, -1, 0,  UDP_OUT_PORT},
     {"ElekIOServer"  ,UDP_ELEK_MANUAL_INPORT        , ELEK_ELEKIO_STATUS_OUT, IP_ELEK_SERVER  , -1, 0,  UDP_OUT_PORT},
     {"Manual"        ,UDP_ELEK_MANUAL_OUTPORT       , ELEK_MANUAL_IN        , IP_LOCALHOST    , -1, 0,  UDP_OUT_PORT},
     {"Etalon"        ,UDP_ELEK_ETALON_OUTPORT       , -1                    , IP_ETALON_CLIENT, -1, 0,  UDP_OUT_PORT},
     {"EtalonStatus"  ,UDP_ELEK_ETALON_STATUS_OUTPORT, -1                    , IP_STATUS_CLIENT, -1, 0,  UDP_OUT_PORT},
     {"Script"        ,UDP_ELEK_SCRIPT_OUTPORT       , ELEK_SCRIPT_IN        , IP_SCRIPT_CLIENT, -1, 0,  UDP_OUT_PORT},
     {"DebugPort"     ,UDP_ELEK_DEBUG_OUTPORT        , -1                    , IP_DEBUG_CLIENT , -1, 0,  UDP_OUT_PORT},
     {"ElekIOOut"     ,UDP_ELEK_SLAVE_DATA_INPORT    , -1                    , IP_ELEKIO_MASTER, -1, 0,  UDP_OUT_PORT},
     {"ElekIOcalibOut",UDP_ELEK_CALIB_DATA_INPORT    , -1                    , IP_ELEKIO_MASTER, -1, 0,  UDP_OUT_PORT},
     {"ElekIOGSBOut"  ,UDP_ELEK_GSB_DATA_INPORT      , -1                    , IP_ELEKIO_MASTER, -1, 0,  UDP_OUT_PORT}
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
	perror("kann scheduler nicht wechseln");
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


	struct timeval tvLocalTime;
	char c;
	int iShmHandle;
	key_t ShmKey; // is an int actually
	char *shm, *s;

	// shared memory unique key
	ShmKey = GSB_SHMKEY;

	// create a shared memory segment
	if ((iShmHandle = shmget(ShmKey, sizeof(struct GSBStatusType), IPC_CREAT | 0666)) < 0) 
	{
		perror("Error calling shmget()");
		exit(1);
	};

	// attach SHM to my process addressspace
	if ((shm = shmat(iShmHandle, NULL, 0)) == (char *) -1) 
	{
		perror("Error calling shmat()");
		exit(1);
	}

	// pointer now is a pointer to struct
	pGSBStatus = (struct GSBStatusType*) shm;

	// setup master fd
	FD_ZERO(&fdsMaster);              // clear the master and temp sets
	FD_ZERO(&fdsSelect);
	InitUDPPorts(&fdsMaster,&fdMax);                  // Setup UDP in and out Ports

	addr_len = sizeof(struct sockaddr);

	// output version info on debugMon and Console
	//
	printf("This is elekIOGSB Version %3.2f for ARM\n",VERSION);
	printf("Compiled with %s\n",COMPILER);
	sprintf(buf, "This is elekIOGSB Version %3.2f for ARM\n",VERSION);
	SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],buf);
	sprintf(buf, "Compiled with %s\n",COMPILER);
	SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],buf);

	// Init I2C IO Thread
	GSBIOThreadInit();

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
	gettimeofday(&LastAction, NULL);
	EndOfSession=FALSE;
	RequestDataFlag=FALSE;

	while (!EndOfSession)
	{
	write(2,"Wait for data..\r",16);

	fdsSelect=fdsMaster;

	pselect_timeout.tv_sec= UDP_SERVER_TIMEOUT;
	pselect_timeout.tv_nsec=0;

	// we are busy waiting here for timeout or an incoming UDP request
	ret=pselect(fdMax+1, &fdsSelect, NULL, NULL, &pselect_timeout, &SignalMask);             // wiat until incoming udp or Signal

	// get GMT Time
	gettimeofday(&tvLocalTime,0);
	memcpy((void*)&pGSBStatus->TimeOfDayGSB,(void*)&tvLocalTime,sizeof(struct timeval));

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
	     if (errno==EINTR)
	       {
		  if ((StatusFlag % 50)==0)
		    {
		       // printf("get Status %6d..\r",StatusFlag);
		       write(2,"get Status.....\r",16);
		    }
		  gettimeofday(&GetStatusStartTime, NULL);
		  // GetGSBStatus(&GSBStatus,IsMaster);
		  gettimeofday(&GetStatusStopTime, NULL);
		  SendUDPData(&MessageOutPortList[CALIB_STATUS_OUT],sizeof(struct GSBStatusType), &GSBStatus);
	       }
	     else
	       {
		  //  if(errno==EINTR)  so was not the Timer, it was a UDP Packet that caused err
		  perror("select");
		  SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],"elekIOGSB: Problem with select");
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
		       switch (MessagePort)
			 {
			  case ELEK_MANUAL_IN:       // port for incoming commands from  eCmd

			    if ((numbytes=recvfrom(MessageInPortList[MessagePort].fdSocket,
						   &Message,sizeof(struct ElekMessageType)  , 0,
						   (struct sockaddr *)&their_addr, &addr_len)) == -1)
			      {
				 perror("recvfrom");
				 SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],"elekIOGSB: Problem with receive");
			      }
#ifdef DEBUG_SLAVECOM
			    sprintf(buf,"recv command from %s on port %d",inet_ntoa(their_addr.sin_addr),
				    ntohs(their_addr.sin_port));
			    SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],buf);
#endif

			    switch (Message.MsgType)
			      {

			       case MSG_TYPE_FETCH_DATA:  // Master wants data
#ifdef DEBUG_SLAVECOM
					 printf("elekIOGSB: MSG_TYPE_FETCH_DATA received, TIME_T  was: %016lx\n\r", Message.MsgTime);
#endif

#ifdef DEBUG_SLAVECOM
					 printf("elekIOGSB: gathering Data...\n\r");
#endif
					 gettimeofday(&GetStatusStartTime, NULL);
					 //GetGSBStatus(&GSBStatus,IsMaster);
					 gettimeofday(&GetStatusStopTime, NULL);
#ifdef DEBUG_SLAVECOM
					 printf("elekIOGSB: Data aquisition took: %02d.%03ds\n\r",
					GetStatusStopTime.tv_sec-GetStatusStartTime.tv_sec,
					(GetStatusStopTime.tv_usec-GetStatusStartTime.tv_usec)/1000);
#endif
					 // send this debugmessage message to debugmon
					 sprintf(buf,"elekIOGSB : MSG_TYPE_FETCH_DATA from Port: %05d",
					 MessageInPortList[MessagePort].PortNumber,
					 Message.Addr,Message.Value,Message.Value);
					 SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],buf);

					 // send requested data, don't send any acknowledges
					 SendUDPData(&MessageOutPortList[ELEK_ELEKIO_GSB_MASTER_OUT],
					     sizeof(struct GSBStatusType), &GSBStatus); // send data packet
				 break;

					// received either from eCmd or cgigateway.cgi: MSG_TYPE_GSB_SETFLOW for one certain flowcontroller
					case MSG_TYPE_GSB_SETFLOW:
				 		// send this debugmessage message to debugmon
				 		sprintf(buf,"elekIOGSB : got MSG_TYPE_GSB_SETFLOW from %s Port: %05d, MFC#:%02d, SetFlow: %05lld",\
					 	inet_ntoa(their_addr.sin_addr),\
					 	MessageInPortList[MessagePort].PortNumber,\
					 	Message.Addr,Message.Value);
				 		SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],buf);
				 		
		 				// save in structure for now
				 		switch(Message.Addr)
				 		{
				 			case 1:
				 				pGSBStatus->uiSetPointMFC0 = Message.Value & 0xFFFF;
				 				break;
				 				
				 			case 2:
				 				pGSBStatus->uiSetPointMFC1 = Message.Value & 0xFFFF;
				 				break;
				 				
				 			case 3:
				 				pGSBStatus->uiSetPointMFC2 = Message.Value & 0xFFFF;
				 				break;
				 				
				 			default:
				 				break;	
				 		}
				 		
						// reply with an ACK	
						Message.Status = Message.Value;
						Message.MsgType = MSG_TYPE_ACK;
						SendUDPDataToIP(&MessageOutPortList[MessageInPortList[MessagePort].RevMessagePort],
						inet_ntoa(their_addr.sin_addr),
						sizeof(struct ElekMessageType), &Message);
					break;

					// received either from eCmd or cgigateway.cgi: MSG_TYPE_GSB_SETVALVE for certain valve
					case MSG_TYPE_GSB_SETVALVE:
				 		// send this debugmessage message to debugmon
				 		sprintf(buf,"elekIOGSB : got MSG_TYPE_GSB_SETVALVE from %s Port: %05d Valves: %s %s %s %s %s",\
					 	inet_ntoa(their_addr.sin_addr),\
					 	MessageInPortList[MessagePort].PortNumber,\
					 	Message.Addr & 0x01?"V1:opn":"V1:cls",\
					 	Message.Addr & 0x02?"V2:opn":"V2:cls",\
					 	Message.Addr & 0x04?"V3:opn":"V3:cls",\
					 	Message.Addr & 0x08?"V4:opn":"V4:cls",\
					 	Message.Addr & 0x10?"V5:opn":"V5:cls"\
					 	);
				 		SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],buf);
				 		
				 		// save in structure for now
				 		pGSBStatus->uiValveControlWord = Message.Addr & 0x1F;

						// reply with an ACK	
						Message.Status = Message.Value;
						Message.MsgType = MSG_TYPE_ACK;
						SendUDPDataToIP(&MessageOutPortList[MessageInPortList[MessagePort].RevMessagePort],
						inet_ntoa(their_addr.sin_addr),
						sizeof(struct ElekMessageType), &Message);
					break;

					// received either from eCmd or cgigateway.cgi: MSG_TYPE_GSB_SETLIGHT for interior LED light
					case MSG_TYPE_GSB_SETLIGHT:
				 		// send this debugmessage message to debugmon
				 		sprintf(buf,"elekIOGSB : got MSG_TYPE_GSB_SETLIGHT from %s Port: %05d",\
					 	inet_ntoa(their_addr.sin_addr),\
					 	MessageInPortList[MessagePort].PortNumber,\
					 	Message.Addr,Message.Value,Message.Value);
				 		SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],buf);

						// reply with an ACK	
						Message.Status = Message.Value;
						Message.MsgType = MSG_TYPE_ACK;
						SendUDPDataToIP(&MessageOutPortList[MessageInPortList[MessagePort].RevMessagePort],
						inet_ntoa(their_addr.sin_addr),
						sizeof(struct ElekMessageType), &Message);
					break;

// THESE FUNCTIONS ARE NOT SUPPORTED ON ARM GSB AS WE DON'T HAVE A BACKPLANE SYSTEM
// AND WE THEIREFORE CAN'T WRITE OR READ TO OR FROM THERE
// MAYBE WE IMPLEMENT DIRECT STRUCTURE ACCESS FOR THE STRUCTURES EXCHANGED ON I2C BUS
// IN THE GSB ELECTRONICS LATER

// FOR NOW, JUST COMMENTED IT OUT

/*
			       case MSG_TYPE_READ_DATA:
				 // printf("elekIOGSB: manual read from Address %04x\n", Message.Addr);
				 Message.Value=elkReadData(Message.Addr);
				 Message.MsgType=MSG_TYPE_ACK;

				 if (MessagePort!=ELEK_ETALON_IN)
				   {
				      sprintf(buf,"elekIOGSB : ReadCmd from %05d Port %04x Value %lld (%04llx)",
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
				      sprintf(buf,"elekIOGSB : WriteCmd from %05d Port %04x Value %lld (%04llx)",
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

			       case MSG_TYPE_CALIB_SETTEMP:
				 if (MessagePort!=ELEK_ETALON_IN)
				   {
				      sprintf(buf,"elekIOGSB : SET_TEMP from %05d Port %04x Value %lld (%04llx)",
					      MessageInPortList[MessagePort].PortNumber,
					      Message.Addr,Message.Value,Message.Value);
				      SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],buf);
				   }
				 GSBStatus.PIDRegulator.Setpoint = Message.Value;
				 Message.Status = Message.Value;
				 Message.MsgType = MSG_TYPE_ACK;
				 SendUDPDataToIP(&MessageOutPortList[MessageInPortList[MessagePort].RevMessagePort],
						 inet_ntoa(their_addr.sin_addr),
						 sizeof(struct ElekMessageType), &Message);
				 break;

			       case MSG_TYPE_CALIB_SETFLOW:
				 if (MessagePort!=ELEK_ETALON_IN)
				   {
				      sprintf(buf,"elekIOGSB : SET_FLOW from %05d Port %04x Value %lld (%04llx)",
					      MessageInPortList[MessagePort].PortNumber,
					      Message.Addr,Message.Value,Message.Value);
				      SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],buf);
				   }
				 //Message.Status = SetCalibFlow( &GSBStatus, Message.Addr, Message.Value);
				 Message.MsgType = MSG_TYPE_ACK;
				 SendUDPDataToIP(&MessageOutPortList[MessageInPortList[MessagePort].RevMessagePort],
						 inet_ntoa(their_addr.sin_addr),
						 sizeof(struct ElekMessageType), &Message);
				 break;

			       case MSG_TYPE_CALIB_SETHUMID:
				 if (MessagePort!=ELEK_ETALON_IN)
				   {
				      sprintf(buf,"elekIOGSB : SET_HUM from %05d Port %04x Value %lld (%04llx)",
					      MessageInPortList[MessagePort].PortNumber,
					      Message.Addr,Message.Value,Message.Value);
				      SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],buf);
				   }

				 //Message.Status = SetCalibFlow( &GSBStatus, Message.Addr, Message.Value);
				 Message.MsgType = MSG_TYPE_ACK;
				 SendUDPDataToIP(&MessageOutPortList[MessageInPortList[MessagePort].RevMessagePort],
						 inet_ntoa(their_addr.sin_addr),
						 sizeof(struct ElekMessageType), &Message);
				 break;
*/
				default:
					SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],"elekIOGSB : unsupported or unknown command code");
	
			      }
		
	     /* switch MsgType */
			    break;
			  default:
			    SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],"elekIOGSB : unknown Port Type");
			    break;

			 }
		       // switch MessagePort
		       TSCin=Message.MsgTime;
		       MaxTimeDiff= MaxTimeDiff<TSC-TSCin ? TSC-TSCin : MaxTimeDiff;
		       MinTimeDiff= MinTimeDiff>TSC-TSCin ? TSC-TSCin : MinTimeDiff;

		       EndOfSession=(bool)(strstr(buf,"ende")!=NULL);
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
	     SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],"elekIOGSB : TimeOut");
	  }

#ifdef RUNONPC
	if (timer_getoverrun(StatusTimer_id)>0)
	  {
	     printf("OVERRUN\n\r");
	     SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],"elekIOGSB : Overrun");
	  }
     /* if overrun */
#endif

	gettimeofday(&StopAction, NULL);

#ifdef DEBUG_TIME_TASK
	sprintf(buf,"elekIOGSB: %ld RT: %ld DT: %ld",StartAction.tv_usec/1000,
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

