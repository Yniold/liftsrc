// ============================================
// elekIOServ.h
// Headerfile
// ============================================

// $RCSfile: elekIOServ.h,v $ last changed on $Date: 2005-01-31 09:49:31 $ by $Author: rudolf $

// History:
//
// $Log: elekIOServ.h,v $
// Revision 1.5  2005-01-31 09:49:31  rudolf
// more work on GPS
//

#define SIGNAL_STATUS SIGRTMAX


#define MAX_TASKS_TO_WAKE 10

typedef unsigned char BOOL;
struct TaskListType {
  char TaskName[MAX_PORT_NAME_LEN];
  int TaskConn;
  int TaskWantStatusOnPort;         
}; /* TaskListType */

struct SyncFlagType {
	BOOL MaskChange;
};  /*SyncFlagType */

// ========================
// Globals for GPS
// ========================

unsigned char port[256] = "/dev/ttyS0";   // serial device used for the incoming GPS data
unsigned char pDataBuffer[1024];

char aProgramName[] = "GPS";              // needed for serial.c 's debug output
char *progname = (char *) aProgramName;   // export pointer to debug string

int verbose = 255;                        // be very verbose
long baud = 9600;                         // serial baudrate
volatile char ucDataReadyFlag = 0;        // Data ready flag
int fdGPS = -1;                           // file descriptor for serial communication
char ucPortOpened = 0;                    // flag for the timer routine wether port is available or not
