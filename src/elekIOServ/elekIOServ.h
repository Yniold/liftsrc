// ============================================
// elekIOServ.h
// Headerfile
// ============================================

// $RCSfile: elekIOServ.h,v $ last changed on $Date: 2006-09-04 10:47:22 $ by $Author: rudolf $

// History:
//
// $Log: elekIOServ.h,v $
// Revision 1.14  2006-09-04 10:47:22  rudolf
// fixed compiler warning for GCC 4.03
//
// Revision 1.13  2005/12/14 13:53:27  rudolf
// GABRIEL campaign changes
//
// Revision 1.12  2005/09/21 21:02:48  rudolf
// fixed Latitude and Longitude parsing, changed tty to S1
//
// Revision 1.11  2005/06/27 10:12:42  rudolf
// changed TimerStates numbering (HH), fixed bug
//
// Revision 1.10  2005/06/26 13:02:17  rudolf
// added time limit for answer from slave
//
// Revision 1.9  2005/06/08 17:31:51  rudolf
// prepared sockets for sending the data structure between master and slave
//
// Revision 1.8  2005/05/22 15:02:43  rudolf
// changed BaudRate and ttyX, changed debug output if commands are sent via eCmd
//
// Revision 1.7  2005/04/21 15:59:21  rudolf
// fixed stupid typo
//
// Revision 1.6  2005/04/21 15:51:52  rudolf
// fix for ARM version
//
// Revision 1.5  2005/01/31 09:49:31  rudolf
// more work on GPS
//

#ifdef RUNONPC
#define SIGNAL_STATUS SIGRTMAX
#else
#define SIGNAL_STATUS SIGALRM
#endif


#define MAX_AGE_SLAVE_STATUS 0.06  // time that a status package is valid after init req.
#define MAX_TASKS_TO_WAKE 10

enum TimerSignalStateEnum {   // states of signal timer
  
  TIMER_SIGNAL_STATE_INITIAL=-1,    // wait for signal to occur
  TIMER_SIGNAL_STATE_GATHER,     // wake up and gather data
  TIMER_SIGNAL_STATE_REQ,        // request data from slave and instruments

  TIMER_SIGNAL_STATE_MAX
  
};


typedef unsigned char BOOL;
struct TaskListType {
  char TaskName[MAX_PORT_NAME_LEN];
  int TaskConn;
  int TaskWantStatusOnPort;         
}; /* TaskListType */

struct SyncFlagType {
	BOOL MaskChange;
};  /*SyncFlagType */



#define MAXSLAVES 3
/* struct to keep a list of slaves */
struct SlaveListType {
  char *SlaveName;                /* name for debug messages */  
  char *SlaveIP;                  /* IP Address */
};



// ========================
// Globals for GPS
// ========================

char port[256] = "/dev/ttyS1";   // serial device used for the incoming GPS data
unsigned char pDataBuffer[1024];

char aProgramName[] = "GPS";              // needed for serial.c 's debug output
char *progname = (char *) aProgramName;   // export pointer to debug string

int verbose = 255;                        // be very verbose
long baud = 4800;                         // serial baudrate
long baudmaster= 38400;                   // baudrate master
volatile char ucDataReadyFlag = 0;        // Data ready flag
int fdGPS = -1;                           // file descriptor for serial communication
char ucPortOpened = 0;                    // flag for the timer routine wether port is available or not


