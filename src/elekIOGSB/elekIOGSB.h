#ifndef ELEKIOGSB_H
#define ELEKIOGSB_H

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

struct calibStatusType CalibStatus;

// ========================
// External UDP Tools
// ========================

extern int SendUDPData(struct MessagePortType *ptrMessagePort, unsigned nByte, void *msg);
extern int SendUDPMsg(struct MessagePortType *ptrMessagePort, void *msg);
extern int InitUDPOutSocket();
extern int InitUDPInSocket();
extern int SendUDPDataToIP(struct MessagePortType *ptrMessagePort, char *IPAddr, unsigned nByte, void *msg);

#endif
