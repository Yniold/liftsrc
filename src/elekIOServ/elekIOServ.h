/* private header file for ElekIOServ */

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