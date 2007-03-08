// realigning procedure for motorized mirrors

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
#include <asm/msr.h>
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


struct MessagePortType MessageInPortList[MAX_MESSAGE_INPORTS]={   // order in list defines sequence of polling
    /* Name, PortNo, ReversePortNo, fdSocket, MaxMessages, Direction */
    {"ElekIOin"  ,        UDP_ELEK_MIRROR_OUTPORT, ELEK_ELEKIO_OUT, IP_LOCALHOST,-1, 1,  UDP_IN_PORT},
    {"ElekStatus", UDP_ELEK_MIRROR_STATUS_OUTPORT,              -1, IP_LOCALHOST,-1, 1,  UDP_IN_PORT}
};

struct MessagePortType MessageOutPortList[MAX_MESSAGE_OUTPORTS]={ // order in list defines sequence of polling
    /* Name, PortNo, ReversePortNo, fdSocket, MaxMessages, Direction */
    {"StatusReq",   UDP_ELEK_MIRROR_INPORT, ELEK_ELEKIO_IN, IP_ELEK_SERVER,  -1,  0, UDP_OUT_PORT},
    {"DebugPort",   UDP_ELEK_DEBUG_OUTPORT,             -1, IP_DEBUG_CLIENT, -1,  0, UDP_OUT_PORT}
};


static uint64_t MessageNumber=0;


enum MirrorStateEnum {
  MIRROR_HOME,
  MIRROR_RIGHT,
  MIRROR_LEFT,
  MIRROR_UP,
  MIRROR_DOWN,
  MAX_MIRROR_STATE };

static char *strStateDescription[MAX_MIRROR_STATE]=
{
   "Home",
     "Right",
     "Left",
     "up",
     "down"
};


struct AverageDataType {
  unsigned NumDat;
  double   Data;
  double   SumData;
  double   SumSqr;
  double   Avg;
  double   Std;
};

#define CHECK_TIME   5



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


// send given Message to ElekIOServ
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


// reset all content of ptrData to 0
void ResetAverageStruct(struct AverageDataType *ptrData) {
  ptrData->NumDat=0;
  ptrData->Data=0.0;
  ptrData->SumData=0.0;
  ptrData->SumSqr=0.0;
  ptrData->Avg=0.0;
  ptrData->Std=0.0;
} /* ResetAverageStruct */


// copy the contents of ptrDataNew to ptrDataOld
void MakeNewOldStruct(struct AverageDataType *ptrDataOld, struct AverageDataType *ptrDataNew) {

  ptrDataOld->NumDat=ptrDataNew->NumDat;
  ptrDataOld->Data=ptrDataNew->Data;
  ptrDataOld->SumData=ptrDataNew->SumData;
  ptrDataOld->SumSqr=ptrDataNew->SumSqr;
  ptrDataOld->Avg=ptrDataNew->Avg;
  ptrDataOld->Std=ptrDataNew->Std;

} /* MakeNewOldStruct */


// include given Counts to content of ptrData
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


// Send Message to ElekIOServ to move a given mirror axis to a given position
int MirrorGoTo(int Mirror, int Axis, int32_t Position) {

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
    Message.MsgType=MSG_TYPE_MIRROR_MOVE;
    Message.Addr=(Mirror << 8) + (Axis & 0x00FF);
    Message.Value=Position;
	
    SendUDPData(&MessageOutPortList[ELEK_ELEKIO_OUT],sizeof(struct ElekMessageType), &Message);
//    printf("ID: %d send req on %4x wait for data....", Message.MsgID, Addr);
    RecieveUDPData(&MessageInPortList[ELEK_ELEKIO_IN], sizeof(struct ElekMessageType), &Message);    
//    printf("ID: %d %4x %d\n",Message.MsgID,Message.Value, Message.MsgType);

    return(Message.Value);
}




int main(int argc, char *argv[])
{
    extern int errno;
    extern struct MessagePortType MessageOutPortList[];
    extern struct MessagePortType MessageInPortList[];
    extern char *strStateDescription[];
    
    int ArgCount; 

    struct ElekMessageType Message;   // sample message
    fd_set fdsMaster;               // master file descriptor list
    fd_set fdsSelect;               // temp file descriptor list for select()
    int fdMax;                      // max fd for select
    int addr_len;
    int MessagePort;
    int numbytes;
    
    
    int ret;

    struct timeval TimeOut;

    char buf[GENERIC_BUF_LEN];
    
    uint64_t TimeCounter=0;
    int MirrorCheckTime=CHECK_TIME;

    struct AverageDataType  OldPosCounts;
    struct AverageDataType  NewPosCounts;

    struct elekStatusType ElekStatus;
    struct tm *ptrTmZeit;
    time_t Seconds;

    int EndOfSession;
     
    double MirrorSignal;
    uint64_t MirrorPosX;
    uint64_t MirrorPosY;
    int State;
    int Mirror, mirrorbitnumber;	    

    // zero TimeOfDay to indicate invalid status
    ElekStatus.TimeOfDayMaster.tv_sec=0L;

    // Init On-Offline Counter
    ResetAverageStruct(&OldPosCounts);
    ResetAverageStruct(&NewPosCounts);


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
    
    //initialise values
    ElekStatus.MirrorData.RealignMinutes=REALIGN_MINUTES;
    ElekStatus.MirrorData.MinUVDiffCts=MIN_UV_DIFF_CTS;

// greetings
    printf("This is Mirror Version (CVS: $Id: mirrors.c,v 1.7 2007-03-08 12:52:35 harder Exp $) for i386\n");
    sprintf(buf,"Mirror : This is Mirror Version (CVS: $Id: mirrors.c,v 1.7 2007-03-08 12:52:35 harder Exp $) for i386\n");
    SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],buf);   

// reset any realigning procedure
    SetStatusCommand(MSG_TYPE_MIRROR_FLAG_REALIGN,0,0);     
	ElekStatus.MirrorData.MovingFlag.Field.Realigning = 0;

   
// loop to be executed continuously
    EndOfSession=FALSE;
    ElekStatus.TimeOfDayMaster.tv_sec=1;
    while (!EndOfSession) {
      //        printf("wait for data ....\n");
	
      fdsSelect=fdsMaster;        
      TimeOut.tv_sec= UDP_SERVER_TIMEOUT;
      TimeOut.tv_usec=0;
      ret=select(fdMax+1, &fdsSelect, NULL, NULL, &TimeOut);
        
      //	printf("ret %d\n",ret);
      if (ret ==-1 ) { // select error
	perror("select");
	SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],"Mirror : Problem with select");
		
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

	      //	      sprintf(buf,"Mirrors: -----------------------> Status ok... ");
	      //	      SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],buf);


	      break;
	      
	    case ELEK_ELEKIO_IN:
	      // we got message
	      RecieveUDPData(&MessageInPortList[MessagePort], sizeof(struct ElekMessageType), &Message);

	      //	      sprintf(buf,"Mirrors: -----------------------> ElekIO with MsgType %d ",Message.MsgType);
	      //	      SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],buf);

	      switch (Message.MsgType) {

	      case MSG_TYPE_SIGNAL:   // woke up for proceeding with mirror stuff
		TimeCounter++;
		Seconds=ElekStatus.TimeOfDayMaster.tv_sec;
    		ptrTmZeit=gmtime(&Seconds);
    		// Start automatic realigning process every RealignMinutes minutes
    		if (((ptrTmZeit->tm_min % ElekStatus.MirrorData.RealignMinutes) ==0) && (ptrTmZeit->tm_sec  ==0) 
    				&& (ElekStatus.MirrorData.MovingFlag.Field.Realigning ==0))
    		{
    		 	SetStatusCommand(MSG_TYPE_MIRROR_FLAG_REALIGN,0,1);
    		 	ElekStatus.MirrorData.MovingFlag.Field.Realigning = 1;
    			Mirror=MIRROR_GREEN_2;
    		}
    		break;
    		
	      case MSG_TYPE_MIRROR_CMD_REALIGN: // manual command from eCmd to realign a given mirror, both axes
	        if (ElekStatus.MirrorData.MovingFlag.Field.Realigning ==0)
	        {
    		 	SetStatusCommand(MSG_TYPE_MIRROR_FLAG_REALIGN,0,1);
    			ElekStatus.MirrorData.MovingFlag.Field.Realigning = 1;
			Mirror=(Message.Addr >> 8) & 0x00FF;
			State=MIRROR_HOME;
		}
		break;
	      
	      default:
		  sprintf(buf,"Mirror: %d unknown Message Type",Message.MsgType);
		  SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],buf);
		  break; /* default */
	      } /* switch Msg Type*/ 

   		
  	      if (ElekStatus.MirrorData.MovingFlag.Field.Realigning == 1)
  	      {
		  //check if Mirror has reached set position and if Etalon is Online, otherwise do nothing
		  if ( 	(ElekStatus.MirrorData.MovingFlag.Field.MovingFlagByte == 0) && 
			(abs(ElekStatus.EtalonData.Encoder.Position-                  /* not further than Dithersteps of OnlinePos */
	        	 ElekStatus.EtalonData.Online.Position)<=(ElekStatus.EtalonData.DitherStepWidth+10)) &&
			(ElekStatus.InstrumentFlags.EtalonAction==ETALON_ACTION_TOGGLE_ONLINE_LEFT ||             /* we intend to be online */
			 ElekStatus.InstrumentFlags.EtalonAction==ETALON_ACTION_TOGGLE_ONLINE_RIGHT)
	          ) {             /* so we are online */ 
			
	
		    // select signal to be optimized depending on which mirror is to be realigned	  
		    switch (Mirror) {
		    case MIRROR_GREEN_1:
		  	MirrorSignal=ElekStatus.ADCCardMaster[DIODE_UV_ADCCARDMASTER_NUMBER].ADCChannelData[DIODE_UV_ADCCARDMASTER_CHANNEL].ADCData; //DiodeUV
		  	break;
		    case MIRROR_GREEN_2:
		  	MirrorSignal=ElekStatus.ADCCardMaster[DIODE_UV_ADCCARDMASTER_NUMBER].ADCChannelData[DIODE_UV_ADCCARDMASTER_CHANNEL].ADCData; //DiodeUV
		  	break;
		    case MIRROR_UV_1:
		  	MirrorSignal=ElekStatus.ADCCardSlave[DIODE_WZ2IN_ADCCARDSLAVE_NUMBER].ADCChannelData[DIODE_WZ2IN_ADCCARDSLAVE_CHANNEL].ADCData; //DiodeWZ2in
		  	break;
		    case MIRROR_UV_2:
		  	MirrorSignal=ElekStatus.ADCCardSlave[DIODE_WZ1IN_ADCCARDSLAVE_NUMBER].ADCChannelData[DIODE_WZ1IN_ADCCARDSLAVE_CHANNEL].ADCData; //DiodeWZ1in
		  	break;
  		    } /* Mirror */
		    MirrorPosX=ElekStatus.MirrorData.Mirror[Message.Addr].Axis[XAXIS].Position;
		    MirrorPosY=ElekStatus.MirrorData.Mirror[Message.Addr].Axis[YAXIS].Position;
		  	
		
		    printf("Mirror : %d State : %s NewPosCounts : %5f OldPosCounts : %5f \n",
		        Mirror,strStateDescription[State],NewPosCounts.Avg,OldPosCounts.Avg);
		    switch (State) {
		  
		    case MIRROR_HOME:
	  
		      AddCounts(&OldPosCounts,MirrorSignal);
			    
 		      if (MirrorCheckTime--<1) { 
		        MirrorCheckTime=CHECK_TIME; 
		        MirrorGoTo(Mirror,XAXIS,MirrorPosX+DELTA_XPOSITION);
		        State=MIRROR_RIGHT;
			} /* if MirrorCheckTime */
		      break;

		    case MIRROR_RIGHT:
		  
		      AddCounts(&NewPosCounts,MirrorSignal);

		      if (MirrorCheckTime--<1) { 
		        MirrorCheckTime=CHECK_TIME; 
		        if (NewPosCounts.Avg-OldPosCounts.Avg>ElekStatus.MirrorData.MinUVDiffCts){
			        MirrorGoTo(Mirror,XAXIS,MirrorPosX+DELTA_XPOSITION);
			        State=MIRROR_RIGHT;
	    		      	MakeNewOldStruct(&OldPosCounts,&NewPosCounts);
				ResetAverageStruct(&NewPosCounts);
    			} else {
			        MirrorGoTo(Mirror,XAXIS,MirrorPosX-2*DELTA_XPOSITION);
				ResetAverageStruct(&NewPosCounts);
			        State=MIRROR_LEFT;
			} /* if NewPosCounts */
		      } /* if MirrorCheckTime */		  
		      break;
		  
		    case MIRROR_LEFT:

		      AddCounts(&NewPosCounts,MirrorSignal);

		      if (MirrorCheckTime--<1) { 
		        MirrorCheckTime=CHECK_TIME; 
		        if (NewPosCounts.Avg-OldPosCounts.Avg>ElekStatus.MirrorData.MinUVDiffCts){
			        MirrorGoTo(Mirror,XAXIS,MirrorPosX-DELTA_XPOSITION);
			        State=MIRROR_LEFT;
	    		      	MakeNewOldStruct(&OldPosCounts,&NewPosCounts);
				ResetAverageStruct(&NewPosCounts);
    			} else {
			        MirrorGoTo(Mirror,XAXIS,MirrorPosX+DELTA_XPOSITION);
			        MirrorGoTo(Mirror,YAXIS,MirrorPosY+DELTA_YPOSITION);
				ResetAverageStruct(&NewPosCounts);
			        State=MIRROR_UP;
			} /* if NewPosCounts */
		      } /* if MirrorCheckTime */		  
		      break;

		    case MIRROR_UP:

		      AddCounts(&NewPosCounts,MirrorSignal);

		      if (MirrorCheckTime--<1) { 
		        MirrorCheckTime=CHECK_TIME; 
		        if (NewPosCounts.Avg-OldPosCounts.Avg>ElekStatus.MirrorData.MinUVDiffCts){
			        MirrorGoTo(Mirror,YAXIS,MirrorPosY+DELTA_YPOSITION);
			        State=MIRROR_UP;
	    		      	MakeNewOldStruct(&OldPosCounts,&NewPosCounts);
				ResetAverageStruct(&NewPosCounts);
    			} else {
			        MirrorGoTo(Mirror,YAXIS,MirrorPosY-2*DELTA_YPOSITION);
				ResetAverageStruct(&NewPosCounts);
			        State=MIRROR_DOWN;
			} /* if NewPosCounts */
		      } /* if MirrorCheckTime */		  
		      break;

		    case MIRROR_DOWN:
		      AddCounts(&NewPosCounts,MirrorSignal);

		      if (MirrorCheckTime--<1) { 
		        MirrorCheckTime=CHECK_TIME; 
		        if (NewPosCounts.Avg-OldPosCounts.Avg>ElekStatus.MirrorData.MinUVDiffCts){
			        MirrorGoTo(Mirror,YAXIS,MirrorPosY-DELTA_YPOSITION);
			        State=	MIRROR_DOWN;
	    		      	MakeNewOldStruct(&OldPosCounts,&NewPosCounts);
				ResetAverageStruct(&NewPosCounts);
    			} else {
			        MirrorGoTo(Mirror,YAXIS,MirrorPosY+DELTA_YPOSITION);
				    ResetAverageStruct(&OldPosCounts);
				    ResetAverageStruct(&NewPosCounts);
			        State=MIRROR_HOME;
			        SetStatusCommand(MSG_TYPE_MIRROR_FLAG_REALIGN,0,0);     
		    		ElekStatus.MirrorData.MovingFlag.Field.Realigning = 0;
			} /* if NewPosCounts */
		      } /* if MirrorCheckTime */		  
		      break;

		    default:
		      sprintf(buf,"Mirrors: %d unknown state",State);
		      SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],buf);
		      break; /* default */
		    } /* switch State */
		  } /* if MovingFlag */
		} /* if Realigning */
	        break; /* ELEK_ELEKIO_IN */
		
	      default:
	        sprintf(buf,"Mirror: %d unknown Message Port",MessagePort);
	        SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],buf);
	        break; /* default */
	      } /* switch Port*/
	    
	  } /* if fd_isset */		
	} /* for MessagePort */
	
      } else { /* ret==0*/
	printf("timeout...\n");
	SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],"Mirror : TimeOut");	    
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
