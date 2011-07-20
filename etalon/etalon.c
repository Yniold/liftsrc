/*
* $RCSfile: etalon.c,v $ last changed on $Date: 2008/09/23 16:41:09 $ by $Author: martinez $
*
* $Log: etalon.c,v $
* Revision 1.30  2008/09/23 16:41:09  martinez
* corrected syntax errors
*
* Revision 1.29  2008/08/12 14:59:18  martinez
* equal time on dither left and right
* harder try to exclude spike at startup
* corrected error leading to omitted offline left when online finished on dither left
* corrected error blocking change to dither_online
*
* Revision 1.28  2007/07/24 08:43:21  martinez
* made integer position variables to long, deleted superfluous variables
*
* Revision 1.27  2007-06-11 16:55:57  martinez
* etalon controller changes
*
* Revision 1.26  2006-10-15 12:02:59  harder
* minor bugs and disp changes in ref channel
*
* Revision 1.25  2006/10/15 11:41:51  harder
* *** empty log message ***
*
* Revision 1.24  2006/10/15 11:35:32  harder
* *** empty log message ***
*
* Revision 1.23  2006/10/15 08:55:19  harder
* ref channel can be now assigned to any counter channel
* eCmd: new command 'refchannel'
* elekIOServ : used etalon Status info to store channel info
* elekIO.h modified etalon structure in status
*
* Revision 1.22  2006/08/14 14:15:56  rudolf
* corrected syntax error
*
* Revision 1.21  2006/08/14 13:32:06  rudolf
* use current position, but change position only if etalon has reached set position
*
* Revision 1.20  2006/08/14 11:51:40  rudolf
* use old set position instead of current position to calculate new set position
*
* Revision 1.19  2006/08/08 12:25:19  rudolf
* corrected error for left dither determination
*
* Revision 1.18  2006/08/08 11:59:03  rudolf
* allowed imprecision of half dither step in determining dithering position for SortandAddCounts
*
* Revision 1.17  2006/08/08 11:41:55  rudolf
* changed MAX_AVG_DIFF from 0.03 to 0.01
*
* Revision 1.16  2006/08/08 11:17:22  martinez
* exchanged current for encoder position in SortandAddCounts
*
* Revision 1.15  2006/08/07 15:41:11  rudolf
* allow deviation of 10 in Encoder Position to define on- and offline status, corrected minor errors
*
* Revision 1.14  2006/08/07 13:53:45  rudolf
* corrected syntax error in maxRefSignal
*
* Revision 1.13  2006/08/07 11:50:16  martinez
* corrected syntax errors, removed option "RECAL" from etalon Actions
*
* Revision 1.12  2006/08/07 11:20:24  rudolf
* corrected syntax errors
*
* Revision 1.11  2006/08/04 17:41:04  martinez
* related all etalon positions to encoder position;
* homing etalon sets encoder position to 0 at left end switch in etalon.c;
* homing is done only in horusStart, home etalon in Dyelaser.m only moves etalon to 0 position;
* included online find in etalon.c and eCmd.c, replaced in Dyelaser.m
*
* Revision 1.10  2006/08/03 15:38:51  martinez
* define ETALON_DEFAULT_ACCSPD as 0x2020 in elekIO.h and use it in etalon.c and elekIOServ.c
*
* Revision 1.9  2006/08/02 15:29:49  martinez
* corrected variable assignment in StepperGoTo and changed speed to 0 before setting new position in main
*
* Revision 1.8  2005/12/14 13:53:28  rudolf
* GABRIEL campaign changes
*
* Revision 1.7  2005/09/18 22:44:49  martinez
* switch ARM LED
*
* Revision 1.6  2005/07/23 09:00:15  rudolf
* added etalonditheronline command
*
* Revision 1.5  2005/07/05 08:26:02  rudolf
* change online pos only by half of the dither step size
*
* Revision 1.4  2005/06/24 18:48:27  martinez
* casted double on Numdat when calculating Avg in AddCounts
*
* Revision 1.3  2005/05/22 19:11:37  rudolf
* fixes for new elekStatus structure
*
* Revision 1.2  2005/04/21 13:58:42  rudolf
* more work on conditional compile
*
*
*
*/

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

#ifdef RUNONPC
#define rdtsc(low,high) \
__asm__ __volatile__("rdtsc" : "=a" (low), "=d" (high))

#define rdtscl(low) \
__asm__ __volatile__("rdtsc" : "=a" (low) : : "edx")

#define rdtscll(val) \
__asm__ __volatile__("rdtsc" : "=A" (val)) 
#endif

#include "../include/elekGeneral.h"
#include "../include/elekIOPorts.h"
#include "../include/elekIO.h"

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
#define DITHER_RIGHT_TIME  2
#define OFFLINE_LEFT_TIME  25
#define OFFLINE_RIGHT_TIME 25
#define ONLINE_TIME        25


#define MAX_AVG_DIFF       0.01


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

static char *strEtalonAction[ETALON_ACTION_MAX+1]={   /* description of Etalon Actions, defined in elekIO.h, for display*/ 

  "Toggle online left",
  "Toggle online right",
  "Toggle online",
  "Toggle offline left",
  "Toggle offline right",
  "No Operation",
  "Dither Online",
  "Scan",
  "Home",
  "Find Online"};

int ReadCommand(uint16_t Addr) {

    extern uint64_t MessageNumber;
    extern struct MessagePortType MessageOutPortList[];
    extern struct MessagePortType MessageInPortList[];

    uint64_t TSC;
    struct ElekMessageType Message;

    #ifdef RUNONPC
    rdtscll(TSC);
    #endif

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


int WriteCommand(uint16_t Addr, uint64_t Value) {

    extern uint64_t MessageNumber;
    extern struct MessagePortType MessageOutPortList[];
    extern struct MessagePortType MessageInPortList[];

    uint64_t TSC;
    struct ElekMessageType Message;    
    

    #ifdef RUNONPC
    rdtscll(TSC);
    #endif

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

int SetStatusCommand(uint16_t MsgType, uint16_t Addr, uint64_t Value) {

    extern uint64_t MessageNumber;
    extern struct MessagePortType MessageOutPortList[];
    extern struct MessagePortType MessageInPortList[];

    uint64_t TSC;
    struct ElekMessageType Message;    
    
    #ifdef RUNONPC
    rdtscll(TSC);
    #endif

    Message.MsgID=MessageNumber++;
    Message.MsgTime=TSC;
    Message.MsgType=MsgType;
    Message.Addr=Addr;
    Message.Value=Value;
	
    SendUDPData(&MessageOutPortList[ELEK_ELEKIO_OUT],sizeof(struct ElekMessageType), &Message);
    //printf("ID: %d send %d on %4x wait for data....", Message.MsgID, Value, Addr);
    RecieveUDPData(&MessageInPortList[ELEK_ELEKIO_IN], sizeof(struct ElekMessageType), &Message);    
    //printf("ID: %d %4x %d\n",Message.MsgID,Message.Value, Message.MsgType);

    return(Message.Value);
	
} /* SetStatusCommand */

int SetAction(enum EtalonActionType Action) {

    extern uint64_t MessageNumber;
    extern struct MessagePortType MessageOutPortList[];
    extern struct MessagePortType MessageInPortList[];

    uint64_t TSC;
    struct ElekMessageType Message;    
    int ret;

    ret=SetStatusCommand(MSG_TYPE_CHANGE_FLAG_ETALON_ACTION,MSG_TYPE_CHANGE_FLAG_ETALON_ACTION,(uint64_t)Action);
    //    Message.MsgID=MessageNumber++;
    //Message.MsgTime=TSC;
    //Message.MsgType=MSG_TYPE_CHANGE_FLAG_ETALON_ACTION;
    //Message.Addr=MSG_TYPE_CHANGE_FLAG_ETALON_ACTION;
    //Message.Value=Action;
	
    //    SendUDPData(&MessageOutPortList[ELEK_ELEKIO_OUT],sizeof(struct ElekMessageType), &Message);
//    printf("ID: %d send req on %4x wait for data....", Message.MsgID, Addr);
//    RecieveUDPData(&MessageInPortList[ELEK_ELEKIO_IN], sizeof(struct ElekMessageType), &Message);    
//    printf("ID: %d %4x %d\n",Message.MsgID,Message.Value, Message.MsgType);

    return(ret);
	
} /* SetAction */	

void ResetAverageStruct(struct AverageDataType *ptrData) {
  ptrData->NumDat=0;
  ptrData->Data=0.0;
  ptrData->SumData=0.0;
  ptrData->SumSqr=0.0;
  ptrData->Avg=0.0;
  ptrData->Std=0.0;
}

void AddCounts(struct AverageDataType *ptrData, uint16_t Counts) {

  //printf("add C %d %d %lf %lf %lf",Counts,ptrData->NumDat,ptrData->SumData,ptrData->SumSqr,ptrData->Avg);
  (ptrData->NumDat)++;
  (ptrData->Data)=Counts;
  (ptrData->SumData)=(ptrData->SumData)+Counts;
  (ptrData->SumSqr)=(ptrData->SumSqr)+Counts*Counts;
  if ((ptrData->NumDat)>0) {
    ptrData->Avg=(ptrData->SumData)/(double)(ptrData->NumDat);
  } else
    ptrData->Avg=0.0;

} /* AddCounts */

int SortAndAddCounts(struct AverageDataType *ptrOnlineLeftCounts, 
		 struct AverageDataType *ptrOnlineRightCounts,
		 struct elekStatusType  *ptrElekStatus) {
		 
  if (abs(ptrElekStatus->EtalonData.Encoder.Position-ptrElekStatus->EtalonData.Online.Position)<
  		(ptrElekStatus->EtalonData.DitherStepWidth)/2) // do we dither on left ?
    AddCounts(ptrOnlineLeftCounts,ptrElekStatus->CounterCardMaster.Channel[ptrElekStatus->EtalonData.Status.StatusField.RefChannel].Counts);

  if (((ptrElekStatus->EtalonData.Encoder.Position- ptrElekStatus->EtalonData.Online.Position) > 
      (ptrElekStatus->EtalonData.DitherStepWidth/2)) &&  
      ((ptrElekStatus->EtalonData.Encoder.Position- ptrElekStatus->EtalonData.Online.Position) < 
      (ptrElekStatus->EtalonData.DitherStepWidth*3/2))) // do we dither on right ?
    AddCounts(ptrOnlineRightCounts,ptrElekStatus->CounterCardMaster.Channel[ptrElekStatus->EtalonData.Status.StatusField.RefChannel].Counts);

} /* SortAndAddCounts */


int StepperGoTo(int32_t Position) {

  union PositionType ElekPosition;
  int ret;

  ElekPosition.Position=Position;
  ret=WriteCommand(ELK_STEP_SETSPD, 0);         //default StepSpeed & Acceleration
  ret=WriteCommand(ELK_STEP_SETPOS,ElekPosition.PositionWord.Low );
  ret=WriteCommand(ELK_STEP_SETPOS+2,ElekPosition.PositionWord.High );
  ret=WriteCommand(ELK_STEP_SETSPD, ETALON_DEFAULT_ACCSPD);         //default StepSpeed & Acceleration

  return(ret);
}

int AdjustOnline(struct AverageDataType *ptrOnlineLeftCounts, 
		 struct AverageDataType *ptrOnlineRightCounts,
		 struct elekStatusType  *ptrElekStatus) {

  double LeftRightBalance;
  int64_t StepPosOnline;

  if ( (ptrOnlineLeftCounts->NumDat>5) && (ptrOnlineRightCounts->NumDat>5) ) { // do we have some data points
    if ( (ptrOnlineLeftCounts->Avg+ptrOnlineRightCounts->Avg)>0 ) { // if the sum of both is zero no need to change
      LeftRightBalance=(ptrOnlineLeftCounts->Avg-ptrOnlineRightCounts->Avg)/
	(ptrOnlineLeftCounts->Avg+ptrOnlineRightCounts->Avg);
      // printf("Left %f Right %f Balance %f",ptrOnlineLeftCounts->Avg,ptrOnlineRightCounts->Avg,LeftRightBalance);
      if ( LeftRightBalance>MAX_AVG_DIFF ) { // if we get a higher left Count
	// step to the left
	StepPosOnline=ptrElekStatus->EtalonData.Online.Position-(ptrElekStatus->EtalonData.DitherStepWidth/2);
	SetStatusCommand(MSG_TYPE_CHANGE_FLAG_SYSTEM_PARAMETER,
			 SYS_PARAMETER_ETALON_ONLINE,
			 StepPosOnline);
	// printf("go left %d->%d\n", ptrElekStatus->EtalonData.Online.Position, StepPosOnline);
      } else if ( -LeftRightBalance>MAX_AVG_DIFF ) { // if we get a higher right Count
	StepPosOnline=ptrElekStatus->EtalonData.Online.Position+(ptrElekStatus->EtalonData.DitherStepWidth/2);
	SetStatusCommand(MSG_TYPE_CHANGE_FLAG_SYSTEM_PARAMETER,
			 SYS_PARAMETER_ETALON_ONLINE,
			 StepPosOnline);
	// printf("go right %d->%d\n", ptrElekStatus->EtalonData.Online.Position, StepPosOnline);
      } else {
	// printf("stay %d\n",ptrElekStatus->EtalonData.Online.Position);
      }
    } /* if sum>0 */
    ResetAverageStruct(ptrOnlineLeftCounts); 
    ResetAverageStruct(ptrOnlineRightCounts); 
    
  } /* if NumData > 0 */
} /* Adjust OnlinePos */



int main(int argc, char *argv[])
{
    extern int errno;
    extern struct MessagePortType MessageOutPortList[];
    extern struct MessagePortType MessageInPortList[];
    
    int ArgCount; 
    enum EtalonActionType RunningCommand;
    enum EtalonActionType LastEtalonAction;

    struct ElekMessageType Message;   // sample message
    fd_set fdsMaster;               // master file descriptor list
    fd_set fdsSelect;               // temp file descriptor list for select()
    int fdMax;                      // max fd for select
    int addr_len;
    int MessagePort;
    int numbytes;
    
    long SetPosition;
    long StepPosOnline=ETALON_STEP_POS_ONLINE;
    long SavePos;
    long EtalonScanPos;
     

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
    int endswitchtouchdown;
    int iLastRefChannel=-1;
     
    double RefSignal;
    uint64_t RefSignalPos;
    double maxRefSignal=0;
    uint64_t maxRefSignalPos=0;



    ArgCount=1;
    if (argc<2) {
      printf("Usage :\t%s  OnlinePos +StepOfflineLeft +StepDither\n", argv[0]);
      printf("\t + indicates a relative position\n");
      printf("\t default  = %d %d %d\n", ETALON_STEP_POS_ONLINE,ETALON_STEP_OFFLINE,ETALON_STEP_DITHER);
      exit(EXIT_FAILURE);
    }
    
    printf("argc %d\n",argc);
    
    // first get OnlinePosition 
    if (ArgCount<argc) 
      ElekStatus.EtalonData.Online.Position=(int64_t)strtod(argv[ArgCount],NULL);  // we have a OnlinePos
    else ElekStatus.EtalonData.Online.Position=ETALON_STEP_POS_ONLINE;
    ArgCount++;

    if (ArgCount<argc) 
      ElekStatus.EtalonData.OfflineStepLeft=(int64_t)strtod(argv[ArgCount],NULL);  // we have a OfflineDelta
    else ElekStatus.EtalonData.OfflineStepLeft=ETALON_STEP_OFFLINE;
    ArgCount++;
    
    if (ArgCount<argc) 
      ElekStatus.EtalonData.DitherStepWidth=(int64_t)strtod(argv[ArgCount],NULL);  // we have a DitherDelta
    else 
      ElekStatus.EtalonData.DitherStepWidth=ETALON_STEP_DITHER;
    ArgCount++;

    

    // zero TimeOfDay to indicate invalid status
    ElekStatus.TimeOfDayMaster.tv_sec=0L;

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
    LastEtalonAction=ETALON_ACTION_MAX;

    // Init Etalonpositions

    ElekStatus.EtalonData.ScanStart.Position =ETALON_SCAN_POS_START;
    ElekStatus.EtalonData.ScanStop.Position  =ETALON_SCAN_POS_STOP;
    ElekStatus.EtalonData.ScanStepWidth      =ETALON_SCAN_STEP_WIDTH;
    
    ElekStatus.EtalonData.Online.Position    =ETALON_STEP_POS_ONLINE;
    ElekStatus.EtalonData.DitherStepWidth    =ETALON_STEP_DITHER;
    ElekStatus.EtalonData.OfflineStepLeft    =ETALON_STEP_OFFLINE;
    ElekStatus.EtalonData.OfflineStepRight   =ETALON_STEP_OFFLINE;


// greetings
    printf("This is Etalon Version (CVS: $Id: etalon.c,v 1.30 2008/09/23 16:41:09 martinez Exp $) for i386\n");
    sprintf(buf,"Etalon : This is Etalon (CVS: $Id: etalon.c,v 1.30 2008/09/23 16:41:09 martinez Exp $) for i386\n");
    SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],buf);   
   

    EndOfSession=FALSE;
    while (!EndOfSession) {
      //        printf("wait for data ....\n");
	
      if ( LastEtalonAction!=ElekStatus.InstrumentFlags.EtalonAction) {
	// printf("Etalonmode : %s->%s\n",
	//       strEtalonAction[LastEtalonAction],
	//       strEtalonAction[ElekStatus.InstrumentFlags.EtalonAction]); 
	LastEtalonAction=ElekStatus.InstrumentFlags.EtalonAction;
      } /* LastEtalonAction */

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

	    //	    sprintf(buf,"Etalon: -----------------------> Port %d ",MessagePort);
	    //	    SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],buf);
	      
	    switch (MessagePort) {
	      
	    case ELEK_STATUS_IN:
	      // we got the current status
	      RecieveUDPData(&MessageInPortList[MessagePort], sizeof(struct elekStatusType), &ElekStatus);

	      //	      sprintf(buf,"Etalon: -----------------------> Status ok... ");
	      //	      SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],buf);

            // lets see if some things have changed
            if (iLastRefChannel!=ElekStatus.EtalonData.Status.StatusField.RefChannel) {
                printf("RefChannel has changed, using now %d before %d\n",
                    ElekStatus.EtalonData.Status.StatusField.RefChannel,iLastRefChannel); 
                iLastRefChannel=ElekStatus.EtalonData.Status.StatusField.RefChannel;
            }

	      break;
	      
	    case ELEK_ELEKIO_IN:
	      RecieveUDPData(&MessageInPortList[MessagePort], sizeof(struct ElekMessageType), &Message);

	      //	      sprintf(buf,"Etalon: -----------------------> ElekIO with MsgType %d ",Message.MsgType);
	      //	      SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],buf);

	      switch (Message.MsgType) {

	      case MSG_TYPE_SIGNAL:   // woke up for proceeding with etalon stuff
		TimeCounter++;

		//check if etalon has reached set position, otherwise do nothing
		if (ElekStatus.EtalonData.Current.Position == ElekStatus.EtalonData.Set.Position 
			|| ElekStatus.EtalonData.Status.StatusField.EndswitchLeft==1 
			|| ElekStatus.EtalonData.Status.StatusField.EndswitchRight==1 ) {
			
		// printf("Endswitch left: %d, right: %d/n", ElekStatus.EtalonData.Status.StatusField.EndswitchLeft, ElekStatus.EtalonData.Status.StatusField.EndswitchRight);	
		
				
		  //		sprintf(buf,"Etalon: -----------------------> ElekIO with EtalonAction %d ",
		  //      		ElekStatus.InstrumentFlags.EtalonAction );
		  //		SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],buf);
		
		  // record size and position of max. ref. signals as a tool to define online position
		  RefSignal=ElekStatus.CounterCardMaster.Channel[ElekStatus.EtalonData.Status.StatusField.RefChannel].Counts;
		  RefSignalPos=ElekStatus.EtalonData.Encoder.Position;
		  if (RefSignal<9000){ // exclude spike at startup
			  if (RefSignal>maxRefSignal) { // if the actual ref. signal is greater than previously recorded
				maxRefSignal=RefSignal;  //set new max. signal value and position
				maxRefSignalPos=RefSignalPos;
	        	  } else {
				maxRefSignal=(1.0-1.0/LIFE_REFSIGNAL)*maxRefSignal; /* reduce max Signal with time to remove obsolete data */
	        	  }
		  }
		
		  switch (ElekStatus.InstrumentFlags.EtalonAction) {
		  
		  case ETALON_ACTION_NOP:
		    RunningCommand=ETALON_ACTION_NOP;
		    break; /* ETALON_ACTION_NOP */
		  
		  case ETALON_ACTION_TOGGLE:
		  case ETALON_ACTION_TOGGLE_ONLINE_LEFT:         /* etalon is on the left ONLINE Position */
		  case ETALON_ACTION_TOGGLE_ONLINE_RIGHT:        /* etalon is on the right ONLINE Position */
		  case ETALON_ACTION_TOGGLE_OFFLINE_LEFT:        /* etalon is on the left OFFLINE Position */
		  case ETALON_ACTION_TOGGLE_OFFLINE_RIGHT:       /* etalon is on the right OFFLINE Position */
		  case ETALON_ACTION_DITHER_ONLINE:              /* etalon is in dither only mode */
		  case ETALON_ACTION_DITHER_ONLINE_LEFT:         /* etalon is in dither only mode */
		  case ETALON_ACTION_DITHER_ONLINE_RIGHT:        /* etalon is in dither only mode */

		  // printf("State %d Action %d\n",State,ElekStatus.InstrumentFlags.EtalonAction);
		    switch (State) {
		    case ETALON_DITHER_LEFT:
		      SetPosition=(ElekStatus.EtalonData.Online.Position+
		    	  ElekStatus.EtalonData.Current.Position*ETALON_STEPRATIO_ENCODER_MOTOR-ElekStatus.EtalonData.Encoder.Position)
		    	  /ETALON_STEPRATIO_ENCODER_MOTOR;
		      //AddCounts(&OnlineLeftCounts,ElekStatus.CounterCard.Channel[CHANNEL_REF_CELL].Counts);
		      //printf("Left (%d/%d): %d %d %lf %lf\n",(int)SetPosition,(int)ElekStatus.EtalonData.Current.Position,
		      //   ElekStatus.CounterCard.Channel[CHANNEL_REF_CELL].Counts,
		      //   OnlineLeftCounts.NumDat,OnlineLeftCounts.SumData,OnlineLeftCounts.Avg);
		    
		      if (DitherLeftTime--<1) { 
		        DitherRightTime=DITHER_RIGHT_TIME; 
		        State=ETALON_DITHER_RIGHT;
		        // we set the dithermode in the action flag only in toggle mode, stupid but easy for now
		        if (ElekStatus.InstrumentFlags.EtalonAction!=ETALON_ACTION_DITHER_ONLINE) 
			  SetAction(ETALON_ACTION_TOGGLE_ONLINE_RIGHT);		      		      
		        //ResetAverageStruct(&OnlineRightCounts); 
		      } /* if DitherleftTime */
		      // see wehter we were long enough online and we are not staying online
		      if (ElekStatus.InstrumentFlags.EtalonAction!=ETALON_ACTION_DITHER_ONLINE && 
			  OnlineTime--<1) {
		        OfflineCounter++;
		        // printf("Counts Left (%ld) : %d %lf %lf\n",SetPosition,OnlineLeftCounts.NumDat,OnlineLeftCounts.SumData,OnlineLeftCounts.Avg);
		        if (OfflineCounter%2) { 
			  OfflineLeftTime=OFFLINE_LEFT_TIME; 
			  State=ETALON_OFFLINE_LEFT; 			
			  SetAction(ETALON_ACTION_TOGGLE_OFFLINE_LEFT);
		        } else { 
			  OfflineRightTime=OFFLINE_RIGHT_TIME; 
			  State=ETALON_OFFLINE_RIGHT; 
			  SetAction(ETALON_ACTION_TOGGLE_OFFLINE_RIGHT);
		        } /* if Offline Counter */
		      }
		      // lets see if we should change the online position
		      AdjustOnline(&OnlineLeftCounts,&OnlineRightCounts,&ElekStatus);
		  
		      break; /* ETALON_DITHER_LEFT */
		    
		    case ETALON_DITHER_RIGHT:
		      SetPosition=(ElekStatus.EtalonData.Online.Position+ElekStatus.EtalonData.DitherStepWidth+
		    	  ElekStatus.EtalonData.Current.Position*ETALON_STEPRATIO_ENCODER_MOTOR-ElekStatus.EtalonData.Encoder.Position)
		    	  /ETALON_STEPRATIO_ENCODER_MOTOR;
		      //AddCounts(&OnlineRightCounts,ElekStatus.CounterCard.Channel[CHANNEL_REF_CELL].Counts);
		      //printf("Right (%d/%d) : %d %d %lf %lf\n",(int)SetPosition,(int)ElekStatus.EtalonData.Current.Position,
		      //   ElekStatus.CounterCard.Channel[CHANNEL_REF_CELL].Counts,
		      //   OnlineRightCounts.NumDat,OnlineRightCounts.SumData,OnlineRightCounts.Avg);    
		    
		      if (DitherRightTime--<1) { // finished with dither right ?
		        DitherLeftTime=DITHER_LEFT_TIME;
		        //ResetAverageStruct(&OnlineLeftCounts);
		        State=ETALON_DITHER_LEFT;
		        // we set the dithermode in the action flag only in toggle mode, stupid but easy for now
		        if (ElekStatus.InstrumentFlags.EtalonAction!=ETALON_ACTION_DITHER_ONLINE) 
			  SetAction(ETALON_ACTION_TOGGLE_ONLINE_LEFT);
		      } /* if DitherRightTime */
		      if (ElekStatus.InstrumentFlags.EtalonAction!=ETALON_ACTION_DITHER_ONLINE &&
			  OnlineTime--<1) {
		        OfflineCounter++;
		        //printf("Counts Right (%ld) : %d %lf %lf\n",SetPosition,OnlineRightCounts.NumDat,
		        //     OnlineRightCounts.SumData,OnlineRightCounts.Avg);		    
		        if (OfflineCounter%2) { 
			  OfflineLeftTime=OFFLINE_LEFT_TIME; 
			  State=ETALON_OFFLINE_LEFT; 
			  SetAction(ETALON_ACTION_TOGGLE_OFFLINE_LEFT);
		        } else { 
			  OfflineRightTime=OFFLINE_RIGHT_TIME; 
			  State=ETALON_OFFLINE_RIGHT;
			  SetAction(ETALON_ACTION_TOGGLE_OFFLINE_RIGHT);
		        }
		      }
		      // lets see if we should change the online position
		      AdjustOnline(&OnlineLeftCounts,&OnlineRightCounts,&ElekStatus);

		      break; /*ETALON_DITHER_RIGHT*/
		    
		    case ETALON_OFFLINE_LEFT:
		      SetPosition=(ElekStatus.EtalonData.Online.Position-ElekStatus.EtalonData.OfflineStepLeft+
		    	  ElekStatus.EtalonData.Current.Position*ETALON_STEPRATIO_ENCODER_MOTOR-ElekStatus.EtalonData.Encoder.Position)
		    	  /ETALON_STEPRATIO_ENCODER_MOTOR;
		    
			  if (ElekStatus.InstrumentFlags.EtalonAction!=ETALON_ACTION_DITHER_ONLINE){
				if (OfflineLeftTime--<1) { 
					OnlineTime=ONLINE_TIME; 
			        DitherLeftTime=DITHER_LEFT_TIME; 
			        State=ETALON_DITHER_LEFT;
			        SetAction(ETALON_ACTION_TOGGLE_ONLINE_LEFT);
				    // lets see if we should change the online position
					//AdjustOnline(&OnlineLeftCounts,&OnlineRightCounts,&ElekStatus);

					//ResetAverageStruct(&OnlineLeftCounts);
				}
			  } else {
		        DitherLeftTime=DITHER_LEFT_TIME; 
				State=ETALON_DITHER_LEFT;
			  }
		      break; /* ETALON_OFFLINE_LEFT */
		    
		    case ETALON_OFFLINE_RIGHT:
		      SetPosition=(ElekStatus.EtalonData.Online.Position+ElekStatus.EtalonData.OfflineStepRight+
		    	  ElekStatus.EtalonData.Current.Position*ETALON_STEPRATIO_ENCODER_MOTOR-ElekStatus.EtalonData.Encoder.Position)
		    	  /ETALON_STEPRATIO_ENCODER_MOTOR;
			  if (ElekStatus.InstrumentFlags.EtalonAction!=ETALON_ACTION_DITHER_ONLINE){
			      if (OfflineRightTime--<1) { 
			        OnlineTime=ONLINE_TIME; 
			        DitherRightTime=DITHER_RIGHT_TIME; 
				    State=ETALON_DITHER_RIGHT;
					SetAction(ETALON_ACTION_TOGGLE_ONLINE_RIGHT);
		      
			        // lets see if we should change the online position
			        //AdjustOnline(&OnlineLeftCounts,&OnlineRightCounts,&ElekStatus);		      
			        // ResetAverageStruct(&OnlineRightCounts);
				}
			  } else {
		        DitherRightTime=DITHER_RIGHT_TIME; 
				State=ETALON_DITHER_RIGHT;
			  }
		      break; /* ETALON_OFFLINE_RIGHT */
		    
		    default :
		      printf("encountered Default mode\n");
		      State=ETALON_OFFLINE_LEFT;
		      SetAction(ETALON_ACTION_TOGGLE_OFFLINE_LEFT);
		      OfflineLeftTime=OFFLINE_LEFT_TIME;
		      break; /* default */		  
		    } /* switch state */
		  
		    if (ElekStatus.TimeOfDayMaster.tv_sec) {   // do we have a valid status ?
		      // SetPosition=((SetPosition-ElekStatus.EtalonData.Current.Position*ETALON_STEPRATIO_ENCODER_MOTOR)+ElekStatus.EtalonData.Encoder.Position)/ETALON_STEPRATIO_ENCODER_MOTOR;
		      //		  printf("Deviation : %d\n",(ElekStatus.EtalonData.Encoder.Position-ElekStatus.EtalonData.Current.Position));		    
		    } /*if ElekStatus.TimeOfDay */
		    // we sort and add the counts
		    SortAndAddCounts(&OnlineLeftCounts,&OnlineRightCounts,&ElekStatus);
		    ret=StepperGoTo(SetPosition);
		  
		    break; /* ETALON_ACTION_TOGGLE */

		  case ETALON_ACTION_SCAN:
		    if (RunningCommand!=ETALON_ACTION_SCAN) {
		      RunningCommand=ETALON_ACTION_SCAN;
		      SavePos=ElekStatus.EtalonData.Encoder.Position;
		      EtalonScanPos=ElekStatus.EtalonData.ScanStart.Position;
		    } else { /* if RunningCommand */
		      if (EtalonScanPos<ElekStatus.EtalonData.ScanStop.Position) {
		        EtalonScanPos+=ElekStatus.EtalonData.ScanStepWidth;     
		      } else { /* are we finished with Scan ? */
		        EtalonScanPos=SavePos;
		        RunningCommand=ETALON_ACTION_NOP;
		        //		      ElekStatus.InstrumentFlags.EtalonAction=ETALON_ACTION_NOP;
		        SetAction(ETALON_ACTION_NOP);
		      } /* if EtalonScanPos */
		    } /* if RunningCommand */
		    SetPosition=(EtalonScanPos+ElekStatus.EtalonData.Current.Position*ETALON_STEPRATIO_ENCODER_MOTOR-ElekStatus.EtalonData.Encoder.Position)
		    	/ETALON_STEPRATIO_ENCODER_MOTOR;
		    ret=StepperGoTo(SetPosition);
		    break; /* ETALON_ACTION_SCAN */
		  case ETALON_ACTION_HOME:
		    if (RunningCommand!=ETALON_ACTION_HOME) { // are we already homeing ?
		      RunningCommand=ETALON_ACTION_HOME;      // not yet so lets do it
		      ret=StepperGoTo(-1000000);               // lets go to a position far far away
		      endswitchtouchdown=1;
		    } else {
		      // already send to home position.. hopefully we will encounter an endswitch
		      if (ElekStatus.EtalonData.Status.StatusField.EndswitchLeft==1 && endswitchtouchdown==1) { // have we just reached the end switch ?
			  ret=WriteCommand(ELK_STEP_MODE,4); // set encoder position to 0
			  endswitchtouchdown=0;
		      } /* if EndswitchLeft */
		    } /* if Running Command */
		    break; /* ETALON_ACTION_HOME */
		  case ETALON_ACTION_FIND_ONLINE:
		    if (RunningCommand!=ETALON_ACTION_FIND_ONLINE) { 
		      RunningCommand=ETALON_ACTION_FIND_ONLINE;      // not yet so lets do it
		      printf("RefSignal: %f \n",RefSignal);
		      printf("maxRefSignal: %f \n",maxRefSignal);
		      StepPosOnline=maxRefSignalPos;		
		      SetStatusCommand(MSG_TYPE_CHANGE_FLAG_SYSTEM_PARAMETER, 
			   SYS_PARAMETER_ETALON_ONLINE,
			   StepPosOnline);
		      SetPosition=(StepPosOnline+ElekStatus.EtalonData.Current.Position*ETALON_STEPRATIO_ENCODER_MOTOR-
		          ElekStatus.EtalonData.Encoder.Position)/ETALON_STEPRATIO_ENCODER_MOTOR;
		      ret=StepperGoTo(SetPosition);               // go to new online position
		      RunningCommand=ETALON_ACTION_DITHER_ONLINE;
		      SetAction(ETALON_ACTION_DITHER_ONLINE);

		    } /* if Running Command */
		    break; /* ETALON_ACTION_FIND_ONLINE */
		  		  
		  default:
		    sprintf(buf,"Etalon: %d unknown action",ElekStatus.InstrumentFlags.EtalonAction);
		    SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],buf);
		    break; /* default */
		  } /* Etalon Action */
		} /* if Set.Position == Current.Position */
		break; /*MSG_TYPE_SIGNAL*/
	        default:
		  sprintf(buf,"Etalon: %d unknown Message Type",Message.MsgType);
		  SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],buf);
		  break; /* default */
	        } /* switch Msg Type*/ 
	        break; /* ELEK_ELEKIO_IN */
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

