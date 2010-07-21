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


#define DEFAULT_SPEED 30
#define DELTA_STEP 8


enum InPortListEnum {  // this list has to be coherent with MessageInPortList
    ELEK_ELEKIO_IN,
    MAX_MESSAGE_INPORTS };

enum OutPortListEnum {  // this list has to be coherent with MessageOutPortList
    ELEK_ELEKIO_OUT,
    ELEK_DEBUG_OUT,
    MAX_MESSAGE_OUTPORTS };


static struct MessagePortType MessageInPortList[MAX_MESSAGE_INPORTS]={   // order in list defines sequence of polling
    /* Name, PortNo, ReversePortNo, fdSocket, MaxMessages, Direction */
    {"ElekIOin", UDP_ELEK_ETALON_OUTPORT  , ELEK_ELEKIO_OUT, IP_LOCALHOST,-1, 1,  UDP_IN_PORT},
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

int main(int argc, char *argv[])
{
    extern int errno;
    extern struct MessagePortType MessageOutPortList[];
    extern struct MessagePortType MessageInPortList[];
    
    struct ElekMessageType Message;   // sample message
    fd_set fdsMaster;               // master file descriptor list
    fd_set fdsSelect;               // temp file descriptor list for select()
    int fdMax;                      // max fd for select
    int addr_len;
    int MessagePort;
    
    unsigned DeltaStep=FALSE;
    unsigned Scan=FALSE;
    uint16_t SetSpd;
    uint16_t ActSpd;
    uint16_t ActPos;
    uint16_t SetPos;
    uint16_t ScanPos;
    int      DiffPos,NewPos;
    int Delta;

    int ret;
    long TimeOut,ScanTimeOut;

    struct timespec SleepTime; 
    struct timespec ScanTime; 
    char buf[GENERIC_BUF_LEN];

    int ArgCount;

    if (argc<2) {
	printf("Usage :\t%s  [r] position [speed]\n", argv[0]);
	printf("\t r indicates a relative position\n");
	printf("\t default speed = %d\n", DEFAULT_SPEED);
	exit(EXIT_FAILURE);
    }
    
    printf("argc %d\n",argc);
    
    /* first might be a d to inidicate differential stepping */
    ArgCount=1;

    if (*argv[ArgCount]=='r') {
	DeltaStep=TRUE;
	ArgCount++;
    } else if (*argv[ArgCount]=='s') {
	Scan=TRUE;
	ArgCount++;
    }

    SetPos=strtod(argv[ArgCount],NULL);
    ArgCount++;
//    printf("%s %s %s\n",argv[0],argv[1],argv[2]);
    
    if (ArgCount<argc) SetSpd=atoi(argv[ArgCount]);  // we have a speed
    else SetSpd=DEFAULT_SPEED;

    ScanTime.tv_sec=0;
    ArgCount++;
    if (ArgCount<argc) ScanTime.tv_nsec=1e6*atol(argv[ArgCount]);  // we have a dwell time
    else ScanTime.tv_nsec=1e7;
    
    printf("Dwell Time %f\n",ScanTime.tv_nsec/1e9);
    
    // setup wait time between checks
    
    SleepTime.tv_sec=0;
    SleepTime.tv_nsec=(long) 1e6;

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
    
    /* get act. pos & speed */
    ActSpd=ReadCommand(ELK_STEP_GETSPD);
    ActPos=ReadCommand(ELK_STEP_GETPOS);
    printf("Speed %3d/%3d Position %5d/%5d\n",ActSpd,SetSpd,ActPos,SetPos);
    
    /* in case of scan we are going to step slowly */
    if (DeltaStep) { 
	ScanPos=ActPos+SetPos;
	SetPos=ScanPos;
	Delta=0;
    } else {
	ScanPos=SetPos;
    }

    if (Scan) { 
       DiffPos=SetPos-ActPos;
    	Delta=(int)(DELTA_STEP*DiffPos/abs(DiffPos));
	SetSpd=63;
        NewPos=SetPos + (DiffPos % Delta);
        ScanPos=NewPos;
	printf("ScanMode : %d %d %d\n",DiffPos,Delta,ScanPos);
    }	

    ScanTimeOut=0;
    do {
	/*set new pos & speed*/
	if (Scan) SetPos=Delta+ActPos;

	sprintf(buf,"etalon: step to %d spd %d ",SetPos,SetSpd);
	SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],buf);
	
	ret=WriteCommand(ELK_STEP_SETPOS, SetPos);
	ret=WriteCommand(ELK_STEP_SETSPD, SetSpd + 0x1000);         // add 0x1000 for acceleration
	
	
	TimeOut=0;
	while (ActPos!=SetPos & TimeOut++<10000) {
	  /* get act. pos & speed */
	  nanosleep(&SleepTime,NULL);
	  ActSpd=(signed)ReadCommand(ELK_STEP_GETSPD);
	  ActPos=(signed)ReadCommand(ELK_STEP_GETPOS);
	} /*endwhile*/
        printf("%4ld Speed %3d/%3d Position %5d/%5d\n",TimeOut,ActSpd,SetSpd,ActPos,ScanPos);
	
	nanosleep(&ScanTime,NULL);
    } while (ActPos!=ScanPos & ScanTimeOut++<0xfffff); 

    /* get act. pos & speed */
    ActSpd=(signed)ReadCommand(ELK_STEP_GETSPD);
    ActPos=(signed)ReadCommand(ELK_STEP_GETPOS);
    printf("Speed %3hhd Position %5d\n",ActSpd,ActPos);


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
