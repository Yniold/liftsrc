#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <time.h>
#include <asm/msr.h>

#include "../include/elekGeneral.h"
#include "../include/elekIOPorts.h"
#include "../include/elekIO.h"

#define ETALON_DEFAULT_ACCSPD 0x1010

enum InPortListEnum {  // this list has to be coherent with MessageInPortList
    ELEK_ELEKIO_IN,
    ELEK_STATUS_IN,          // Port for incoming Status information from ELEKIOServ
    MAX_MESSAGE_INPORTS };

enum OutPortListEnum {  // this list has to be coherent with MessageOutPortList
    ELEK_ELEKIO_OUT,
    ELEK_DEBUG_OUT,
    MAX_MESSAGE_OUTPORTS };

enum EtalonStateEnum {
  ETALON_DITHER_LEFT,
  ETALON_DITHER_RIGHT,
  ETALON_OFFLINE_LEFT,
  ETALON_OFFLINE_RIGHT,
  MAX_ETALON_STATE };

struct AverageDataType {
  unsigned NumDat;
  double   Data;
  double   SumData;
  double   SumSqr;
  double   Avg;
  double   Std;
};

#define DITHER_LEFT_TIME   2
#define DITHER_RIGHT_TIME  3
#define OFFLINE_LEFT_TIME  25
#define OFFLINE_RIGHT_TIME 25
#define ONLINE_TIME        25

#define STEP_POS_ONLINE     500
#define STEP_DITHER         16
#define STEP_OFFLINE        64

#define MAX_AVG_DIFF       0.1


static struct MessagePortType MessageInPortList[MAX_MESSAGE_INPORTS]={   // order in list defines sequence of polling
    /* Name, PortNo, ReversePortNo, fdSocket, MaxMessages, Direction */
    {"ElekIOin"  ,        UDP_ELEK_ETALON_OUTPORT, ELEK_ELEKIO_OUT, IP_LOCALHOST,-1, 1,  UDP_IN_PORT},
    {"ElekStatus", UDP_ELEK_ETALON_STATUS_OUTPORT,              -1, IP_LOCALHOST,-1, 1,  UDP_IN_PORT}
};

static struct MessagePortType MessageOutPortList[MAX_MESSAGE_OUTPORTS]={ // order in list defines sequence of polling
    /* Name, PortNo, ReversePortNo, fdSocket, MaxMessages, Direction */
    {"StatusReq",   UDP_ELEK_ETALON_INPORT, ELEK_ELEKIO_IN, IP_ELEK_SERVER,  -1,  0, UDP_OUT_PORT},
    {"DebugPort",   UDP_ELEK_DEBUG_OUTPORT,             -1, IP_DEBUG_CLIENT, -1,  0, UDP_OUT_PORT}
};


static uint64_t MessageNumber=0;

int ReadCommand(uint16_t Addr) {

    extern uint64_t MessageNumber;
    extern struct MessagePortType MessageOutPortList[];
    extern struct MessagePortType MessageInPortList[];

    uint64_t TSC;
    struct ElekMessageType Message;    
    

    rdtscll(TSC);

    Message.MsgID=MessageNumber++;
    Message.MsgTime=TSC;
    Message.MsgType=MSG_TYPE_READ_DATA;
    Message.Addr=Addr;
    Message.Value=0;
	
    SendUDPData(&MessageOutPortList[ELEK_ELEKIO_OUT],sizeof(struct ElekMessageType), &Message);
//    printf("send req on %4x wait for data....", Addr);
    RecieveUDPData(&MessageInPortList[ELEK_ELEKIO_IN], sizeof(struct ElekMessageType), &Message);    
//    printf("got %4x\n",Message.Value);

    return(Message.Value);
	
} /* ReadCommand */


int WriteCommand(uint16_t Addr, uint16_t Value) {

    extern uint64_t MessageNumber;
    extern struct MessagePortType MessageOutPortList[];
    extern struct MessagePortType MessageInPortList[];

    uint64_t TSC;
    struct ElekMessageType Message;    
    

    rdtscll(TSC);

    Message.MsgID=MessageNumber++;
    Message.MsgTime=TSC;
    Message.MsgType=MSG_TYPE_WRITE_DATA;
    Message.Addr=Addr;
    Message.Value=Value;
	
    SendUDPData(&MessageOutPortList[ELEK_ELEKIO_OUT],sizeof(struct ElekMessageType), &Message);
//    printf("ID: %d send req on %4x wait for data....", Message.MsgID, Addr);
    RecieveUDPData(&MessageInPortList[ELEK_ELEKIO_IN], sizeof(struct ElekMessageType), &Message);    
//    printf("ID: %d %4x %d\n",Message.MsgID,Message.Value, Message.MsgType);

    return(Message.Value);
	
} /* WriteCommand */	

void ResetAverageStruct(struct AverageDataType *ptrData) {
  ptrData->NumDat=0;
  ptrData->Data=0.0;
  ptrData->SumData=0.0;
  ptrData->SumSqr=0.0;
  ptrData->Avg=0.0;
  ptrData->Std=0.0;
}

void AddCounts(struct AverageDataType *ptrData, uint16_t Counts) {
  (ptrData->NumDat)++;
  ptrData->Data=Counts;
  (ptrData->SumData)+=Counts;
  (ptrData->SumSqr)+=Counts*Counts;
  if ((ptrData->NumDat)>0) {
    ptrData->Avg=(ptrData->SumData)/(ptrData->NumDat);
  } else
    ptrData->Avg=0;
}

int StepperGoTo(int64_t Position) {

  union PositionType ElekPosition;
  int ret;

  ElekPosition.Position=Position;
  ret=WriteCommand(ELK_STEP_SETPOS,ElekPosition.PositionWord.Low );
  ret=WriteCommand(ELK_STEP_SETPOS+2,ElekPosition.PositionWord.High );
  ret=WriteCommand(ELK_STEP_SETSPD, ETALON_DEFAULT_ACCSPD);         //default StepSpeed & Acceleration

  return(ret);
}

int main(int argc, char *argv[])
{
    extern int errno;
    extern struct MessagePortType MessageOutPortList[];
    extern struct MessagePortType MessageInPortList[];
    
    int ArgCount; 
    enum EtalonActionType RunningCommand;

    struct ElekMessageType Message;   // sample message
    fd_set fdsMaster;               // master file descriptor list
    fd_set fdsSelect;               // temp file descriptor list for select()
    int fdMax;                      // max fd for select
    int addr_len;
    int MessagePort;
    int numbytes;
    
    int ActSpeed;
    int SetSpeed;
    int64_t ActPosition;
    int64_t SetPosition;
    int64_t StepPosOnline=STEP_POS_ONLINE;
    int64_t StepDither=STEP_DITHER;
    int64_t StepPosOffline=STEP_OFFLINE;
     

    unsigned DeltaStep=FALSE;
    unsigned Scan=FALSE;
    
    int ret;
    long ScanTimeOut;

    struct timeval TimeOut;

    struct timespec SleepTime; 
    struct timespec ScanTime; 
    char buf[GENERIC_BUF_LEN];
    
    uint64_t TimeCounter=0;
    uint64_t OfflineCounter=0;
    int DitherLeftTime=0;
    int DitherRightTime=0;
    int OnlineTime=0;
    int OfflineLeftTime=0;
    int OfflineRightTime=0;

    struct AverageDataType  OnlineLeftCounts;
    struct AverageDataType  OnlineRightCounts;
    double LeftRightBalance;

    struct elekStatusType ElekStatus;

    int State;
    int EndOfSession;


    ArgCount=1;
    if (argc<2) {
      printf("Usage :\t%s  OnlinePos +StepOffline +StepDither\n", argv[0]);
      printf("\t + indicates a relative position\n");
      printf("\t default  = %d %d %d\n", STEP_POS_ONLINE,STEP_OFFLINE,STEP_DITHER);
      exit(EXIT_FAILURE);
    }
    
    printf("argc %d\n",argc);
    
    // first get OnlinePosition 
    if (ArgCount<argc) StepPosOnline=(int64_t)strtod(argv[ArgCount],NULL);  // we have a OnlinePos
    else StepPosOnline=STEP_POS_ONLINE;
    ArgCount++;

    if (ArgCount<argc) StepPosOffline=(int64_t)strtod(argv[ArgCount],NULL);  // we have a OfflineDelta
    else StepPosOffline=STEP_OFFLINE;
    ArgCount++;

    if (ArgCount<argc) StepDither=(int64_t)strtod(argv[ArgCount],NULL);  // we have a DitherDelta
    else StepDither=STEP_DITHER;
    ArgCount++;

    

    // zero TimeOfDay to indicate invalid status
    ElekStatus.TimeOfDay.tv_sec=0L;

    // Init On-Offline Counter

    ResetAverageStruct(&OnlineLeftCounts);
    ResetAverageStruct(&OnlineRightCounts);

     // setup master fd
    FD_ZERO(&fdsMaster);              // clear the master and temp sets
    FD_ZERO(&fdsSelect);
        
    // init inports
    for (MessagePort=0; MessagePort<MAX_MESSAGE_INPORTS;MessagePort++) {
	
	printf("opening IN Port %s on Port %d\n",
	       MessageInPortList[MessagePort].PortName,
	       MessageInPortList[MessagePort].PortNumber);
	
	MessageInPortList[MessagePort].fdSocket=InitUDPInSocket(MessageInPortList[MessagePort].PortNumber);
	
	FD_SET(MessageInPortList[MessagePort].fdSocket, &fdsMaster);     // add the manual port to the master set
	fdMax=MessageInPortList[MessagePort].fdSocket;                   // the last one will give the max number
    } /* for MessageInPort */
    
    // init outports
    for (MessagePort=0; MessagePort<MAX_MESSAGE_OUTPORTS;MessagePort++) {
	printf("opening OUT Port %s on Port %d\n",
	       MessageOutPortList[MessagePort].PortName,MessageOutPortList[MessagePort].PortNumber);
	MessageOutPortList[MessagePort].fdSocket=InitUDPOutSocket(MessageOutPortList[MessagePort].PortNumber);
	
    } /* for MessageOutPort */
    
    addr_len = sizeof(struct sockaddr);
    
    State=ETALON_OFFLINE_LEFT;
    OfflineLeftTime=OFFLINE_LEFT_TIME;
    ElekStatus.InstrumentFlags.EtalonAction=ETALON_ACTION_NOP;


    EndOfSession=FALSE;
    while (!EndOfSession) {
      //        printf("wait for data ....\n");
	
      fdsSelect=fdsMaster;        
      TimeOut.tv_sec= UDP_SERVER_TIMEOUT;
      TimeOut.tv_usec=0;
      ret=select(fdMax+1, &fdsSelect, NULL, NULL, &TimeOut);
        
      //	printf("ret %d\n",ret);
      if (ret ==-1 ) { // select error
	perror("select");
	SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],"Etalon : Problem with select");
		
      } else if (ret>0) {
	//	    printf("woke up...");
	    
	for (MessagePort=0; MessagePort<MAX_MESSAGE_INPORTS;MessagePort++) { 
	  
	  if (FD_ISSET(MessageInPortList[MessagePort].fdSocket,&fdsSelect)) {   // new msg on fdNum. socket ... 
	    
	    switch (MessagePort) {
	      
	    case ELEK_STATUS_IN:
	      // we got the current status
	      RecieveUDPData(&MessageInPortList[ELEK_STATUS_IN], sizeof(struct elekStatusType), &ElekStatus);
	      break;
	      
	    case ELEK_ELEKIO_IN:
	      RecieveUDPData(&MessageInPortList[MessagePort], sizeof(struct ElekMessageType), &Message);

	      switch (Message.MsgType) {
	      case MSG_TYPE_SIGNAL:   // woke up for proceeding with etalon stuff
		TimeCounter++;
		
		switch (ElekStatus.InstrumentFlags.EtalonAction) {
		  
		case ETALON_ACTION_NOP:
		  
		  break; /* ETALON_ACTION_NOP */
		  
		case  ETALON_ACTION_TOGGLE:
		  switch (State) {
		  case ETALON_DITHER_LEFT:
		    SetPosition=StepPosOnline+StepDither;
		    AddCounts(&OnlineLeftCounts,ElekStatus.CounterCard.Channel[CHANNEL_REF_CELL].Counts);
		    
		    if (DitherLeftTime--<1) { DitherRightTime=DITHER_RIGHT_TIME; State=ETALON_DITHER_RIGHT; }
		    if (OnlineTime--<1) {
		      OfflineCounter++;
		      printf("Counts Left : %d %d\n",OnlineLeftCounts.SumData,OnlineLeftCounts.Avg);
		      if (OfflineCounter%2) { OfflineLeftTime=OFFLINE_LEFT_TIME; State=ETALON_OFFLINE_LEFT; }
		      else { OfflineLeftTime=OFFLINE_LEFT_TIME; State=ETALON_OFFLINE_RIGHT; }
		    }
		    break; /* ETALON_DITHER_LEFT */
		    
		  case ETALON_DITHER_RIGHT:
		    SetPosition=StepPosOnline;
		    AddCounts(&OnlineRightCounts,ElekStatus.CounterCard.Channel[CHANNEL_REF_CELL].Counts);
		    
		    if (DitherRightTime--<1) { DitherLeftTime=DITHER_LEFT_TIME; State=ETALON_DITHER_LEFT; }
		    if (OnlineTime--<1) {
		      OfflineCounter++;
		      printf("Counts Right : %d %d\n",OnlineRightCounts.SumData,OnlineRightCounts.Avg);		    
		      if (OfflineCounter%2) { OfflineLeftTime=OFFLINE_LEFT_TIME; State=ETALON_OFFLINE_LEFT; }
		      else { OfflineRightTime=OFFLINE_RIGHT_TIME; State=ETALON_OFFLINE_RIGHT; }
		    }
		    break; /*ETALON_DITHER_RIGHT*/
		    
		  case ETALON_OFFLINE_LEFT:
		    SetPosition=StepPosOnline+StepPosOffline;
		    
		    if (OfflineLeftTime--<1) { 
		      OnlineTime=ONLINE_TIME; 
		      DitherLeftTime=DITHER_LEFT_TIME; 
		      State=ETALON_DITHER_LEFT;
		      // lets see if we should change the online position
		      if ( (OnlineLeftCounts.Avg+OnlineRightCounts.Avg)>0 ) { // if the sum of both is zero no need to change
			LeftRightBalance=(OnlineLeftCounts.Avg-OnlineRightCounts.Avg)/
			  (OnlineLeftCounts.Avg+OnlineRightCounts.Avg);
			printf("Balance : %f ",LeftRightBalance);
			if ( LeftRightBalance>MAX_AVG_DIFF ) { // if we get a higher left Count
			  StepPosOnline=StepPosOnline+StepDither;
			  printf("go left %d->%d\n",StepPosOnline-StepDither,StepPosOnline);
			} else if ( -LeftRightBalance>MAX_AVG_DIFF ) { // if we get a higher right Count
			  StepPosOnline=StepPosOnline-StepDither;
			  printf("go right %d->%d\n",StepPosOnline+StepDither,StepPosOnline);
			} else {
			  printf("stay %d\n",StepPosOnline);
			}
		      } /* if sum>0 */
		      ResetAverageStruct(&OnlineLeftCounts);
		    }
		    break; /* ETALON_OFFLINE_LEFT */
		    
		  case ETALON_OFFLINE_RIGHT:
		    SetPosition=StepPosOnline-StepPosOffline;
		    if (OfflineRightTime--<1) { 
		      OnlineTime=ONLINE_TIME; 
		      DitherRightTime=DITHER_RIGHT_TIME; 
		      State=ETALON_DITHER_RIGHT;
		      
		      // lets see if we should change the online position
		      if ( (OnlineLeftCounts.Avg+OnlineRightCounts.Avg)>0 ) { // if the sum of both is zero no need to change
			LeftRightBalance=(OnlineLeftCounts.Avg-OnlineRightCounts.Avg)/
			  (OnlineLeftCounts.Avg+OnlineRightCounts.Avg);
			printf("Balance : %f ",LeftRightBalance);
			if ( LeftRightBalance>MAX_AVG_DIFF ) { // if we get a higher left Count
			  StepPosOnline=StepPosOnline+StepDither;
			  printf("go left %d->%d\n",StepPosOnline-StepDither,StepPosOnline);
			} else if ( -LeftRightBalance>MAX_AVG_DIFF ) { // if we get a higher right Count
			  StepPosOnline=StepPosOnline-StepDither;
			  printf("go right %d->%d\n",StepPosOnline+StepDither,StepPosOnline);
			} else {
			  printf("stay %d\n",StepPosOnline);
			}
		      } /* if sum>0 */
		      
		      ResetAverageStruct(&OnlineRightCounts);
		    }
		    break; /* ETALON_OFFLINE_RIGHT */
		    
		  default :
		    State=ETALON_OFFLINE_LEFT;
		    OfflineLeftTime=OFFLINE_LEFT_TIME;
		    break; /* default */		  
		  } /* switch state */
		  
		  if (ElekStatus.TimeOfDay.tv_sec) {   // do we have a valid status ?
		    // SetPosition=(SetPosition-ElekStatus.EtalonData.Current.Position)+ElekStatus.EtalonData.Encoder.Position;
		    //		  printf("Deviation : %d\n",(ElekStatus.EtalonData.Encoder.Position-ElekStatus.EtalonData.Current.Position));		    
		  } /*if ElekStatus.TimeOfDay */
		  
		  ret=StepperGoTo(SetPosition);
		  
		  break; /* ETALON_ACTION_TOGGLE */

		case ETALON_ACTION_SCAN:
		  break; /* ETALON_ACTION_SCAN */
		case ETALON_ACTION_HOME:
		  if (RunningCommand!=ETALON_ACTION_HOME) { // are we already homeing ?
		    RunningCommand=ETALON_ACTION_HOME;      // not yet so lets do it
		    ret=StepperGoTo(1000000);               // lets go to a position far far away
		  } else {
		    // already send to home position.. hopefully we will encounter an endswitch
		    
		    
		  } /* Running Command */
		  break; /* ETALON_ACTION_HOME */
		case ETALON_ACTION_RECAL:
		  break; /* ETALON_ACTION_RECAL */
		  
		default:
		  sprintf(buf,"Etalon: %d unknown action",ElekStatus.InstrumentFlags.EtalonAction);
		  SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],buf);
		  break; /* default */
		} /* Etalon Action */
	      default:
		sprintf(buf,"Etalon: %d unknown Message Type",Message.MsgType);
		SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],buf);
		break; /* default */
	      } /* switch Msg Type*/ 
	      
	    default:
	      sprintf(buf,"Etalon: %d unknown Message Port",MessagePort);
	      SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],buf);
	      break; /* default */
	    } /* switch Port*/
	    
	  } /* if fd_isset */		
	} /* for MessagePort */
	
      } else { /* ret==0*/
	printf("timeout...\n");
	SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],"Etalon : TimeOut");	    
      }	
    } /* while */
    
    // close all in bound sockets
    for (MessagePort=0; MessagePort<MAX_MESSAGE_INPORTS;MessagePort++) {	
      close(MessageInPortList[MessagePort].fdSocket);
    } /*for MessagePort */ 
    
    // close all out bound sockets
    for (MessagePort=0; MessagePort<MAX_MESSAGE_OUTPORTS;MessagePort++) {	
      close(MessageOutPortList[MessagePort].fdSocket);
    } /*for MessagePort */
    
    exit(EXIT_SUCCESS);

}
