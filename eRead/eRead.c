/*
* $RCSfile: eRead.c,v $ last changed on $Date: 2005/04/21 13:49:36 $ by $Author: rudolf $
*
* $Log: eRead.c,v $
* Revision 1.2  2005/04/21 13:49:36  rudolf
* more work on conditional compile, added revision history
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
#include <asm/msr.h>
#endif

#include "../include/elekIOPorts.h"
#include "../include/elekIO.h"


#define DEFAULT_SPEED 30


enum InPortListEnum {  // this list has to be coherent with MessageInPortList
    ELEK_ELEKIO_IN,
    MAX_MESSAGE_INPORTS };

enum OutPortListEnum {  // this list has to be coherent with MessageOutPortList
    ELEK_ELEKIO_OUT,
    ELEK_DEBUG_OUT,
    MAX_MESSAGE_OUTPORTS };


static struct MessagePortType MessageInPortList[MAX_MESSAGE_INPORTS]={   // order in list defines sequence of polling
    /* Name, PortNo, ReversePortNo, fdSocket, MaxMessages, Direction */
    {"ElekIOin", UDP_ELEK_MANUAL_OUTPORT  , ELEK_ELEKIO_OUT, -1, 1,  UDP_IN_PORT},
};

static struct MessagePortType MessageOutPortList[MAX_MESSAGE_OUTPORTS]={ // order in list defines sequence of polling
    /* Name, PortNo, ReversePortNo, fdSocket, MaxMessages, Direction */
    {"StatusReq",   UDP_ELEK_MANUAL_INPORT, ELEK_ELEKIO_IN,IP_ELEK_SERVER,  -1,  0, UDP_OUT_PORT},
    {"DebugPort",   UDP_ELEK_DEBUG_OUTPORT,             -1,IP_DEBUG_CLIENT, -1,  0, UDP_OUT_PORT}
};


static uint64_t MessageNumber=0;

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


int WriteCommand(uint16_t Addr, uint16_t Value) {

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
    printf("ID: %d send req on %4x wait for data....", Message.MsgID, Addr);
    RecieveUDPData(&MessageInPortList[ELEK_ELEKIO_IN], sizeof(struct ElekMessageType), &Message);    
    printf("ID: %d %4x %d\n",Message.MsgID,Message.Value, Message.MsgType);

    return(Message.Value);
	
} /* WriteCommand */

int SetStatusCommand(uint16_t Addr, uint16_t Value) {

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
    Message.MsgType=Addr;
    Message.Addr=Addr;
    Message.Value=Value;
	
    SendUDPData(&MessageOutPortList[ELEK_ELEKIO_OUT],sizeof(struct ElekMessageType), &Message);
    printf("ID: %d send %d on %4x wait for data....", Message.MsgID, Value, Addr);
    RecieveUDPData(&MessageInPortList[ELEK_ELEKIO_IN], sizeof(struct ElekMessageType), &Message);    
    printf("ID: %d %4x %d\n",Message.MsgID,Message.Value, Message.MsgType);

    return(Message.Value);
	
} /* SetStatusCommand */	

int main(int argc, char *argv[])
{
    extern int errno;
    extern struct MessagePortType MessageOutPortList[];
    extern struct MessagePortType MessageInPortList[];
    
    fd_set fdsMaster;               // master file descriptor list
    fd_set fdsSelect;               // temp file descriptor list for select()
    int fdMax;                      // max fd for select
    int addr_len;
    int MessagePort;

    uint16_t Addr;
    uint16_t Value;

    int ret;
    long TimeOut;

    struct timespec SleepTime; 

    int ArgCount;

    if (argc<2) {
	printf("Usage :\t%s  addr\n", argv[0]);
	exit(EXIT_FAILURE);
    }
    
//    printf("argc %d\n",argc);

    //   printf("%s %s %s\n",argv[0],argv[1],argv[2]);
    

//    Value=strtod(argv[ArgCount],NULL);
//    ArgCount++;

    // setup wait time between checks
    
    SleepTime.tv_sec=0;
    SleepTime.tv_nsec=(long) 1e8;

     // setup master fd
    FD_ZERO(&fdsMaster);              // clear the master and temp sets
    FD_ZERO(&fdsSelect);
        
    // init inports
    for (MessagePort=0; MessagePort<MAX_MESSAGE_INPORTS;MessagePort++) {
	
//	printf("opening IN Port %s on Port %d\n",
//	       MessageInPortList[MessagePort].PortName,
//	       MessageInPortList[MessagePort].PortNumber);
	
	MessageInPortList[MessagePort].fdSocket=InitUDPInSocket(MessageInPortList[MessagePort].PortNumber);
	
	FD_SET(MessageInPortList[MessagePort].fdSocket, &fdsMaster);     // add the manual port to the master set
	fdMax=MessageInPortList[MessagePort].fdSocket;                   // the last one will give the max number
    } /* for MessageInPort */
    
    // init outports
    for (MessagePort=0; MessagePort<MAX_MESSAGE_OUTPORTS;MessagePort++) {
//	printf("opening OUT Port %s on Port %d\n",
//	       MessageOutPortList[MessagePort].PortName,MessageOutPortList[MessagePort].PortNumber);
	MessageOutPortList[MessagePort].fdSocket=InitUDPOutSocket(MessageOutPortList[MessagePort].PortNumber);
	
    } /* for MessageOutPort */
    
//    printf("done init ports\n");


    ArgCount=1;

    switch(argv[ArgCount++][0]) {
	case 'r':
	    Addr=strtod(argv[ArgCount],NULL);
	    ArgCount++;
	    Value=ReadCommand(Addr);
	    printf("Read %4x(%5d) : %4x(%5d)\n",Addr,Addr, Value, Value);
	    break;

	case 'w':
	    Addr=strtod(argv[ArgCount],NULL);
	    ArgCount++;
//    printf("%s %s %s\n",argv[0],argv[1],argv[2]);
	    Value=strtod(argv[ArgCount],NULL);
	    ArgCount++;
	    ret=WriteCommand(Addr,Value);
	    printf("Wrote %4x=%4x(%5d) : %4x(%5d)\n",ret,Addr,Addr,Value,Value);
	    break;

	case 's':
	    
	    if (strcmp(argv[ArgCount],"stopquery")==0) {
		Addr=MSG_TYPE_CHANGE_FLAG_STATUS_QUERY;
		Value=0;
	    };
		
	    if (strcmp(argv[ArgCount],"startquery")==0) {
		Addr=MSG_TYPE_CHANGE_FLAG_STATUS_QUERY;
		Value=1;
	    };
	    SetStatusCommand(Addr,Value); 
	    break;

	default:
	    printf("unkown type, valid is r,w,s\n");
	    break;
    } /* switch */
		

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
