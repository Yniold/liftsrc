/************************************************************************/
/*
$RCSfile: eCmd.c,v $ $Revision: 1.15 $
last change on $Date: 2005-05-18 16:48:44 $ by $Author: rudolf $

$Log: eCmd.c,v $
Revision 1.15  2005-05-18 16:48:44  rudolf
fixed buffer overflow in cmdline parsing

Revision 1.14  2005/04/21 13:48:55  rudolf
more work on conditional compile, added revision history

Revision 1.13  2005/02/11 13:44:00  harder
added InstrumentAction to elekStatus, added support in elekIOServ & eCmd

Revision 1.12  2005/01/24 16:32:09  harder
revision information included in file header

Revision 1.3  2005/01/21 14:17:47  harder
update file description

Revision 1.2  2005/01/21 14:13:03  harder
update file description

*/
/************************************************************************/

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

    struct ElekMessageType Message;
    uint64_t TSC;

    #ifdef RUNONPC
    rdtscll(TSC);
    #endif

    Message.MsgID=MessageNumber++;
    Message.MsgTime=TSC; /* will put zero into struct on ARM */
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

int SetStatusCommand(uint16_t MsgType, uint16_t Addr, uint16_t Value) {

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
    printf("ID: %d send %d on %4x wait for data....", Message.MsgID, Value, Addr);
    RecieveUDPData(&MessageInPortList[ELEK_ELEKIO_IN], sizeof(struct ElekMessageType), &Message);    
    printf("ID: %d %4x %d\n",Message.MsgID,Message.Value, Message.MsgType);

    return(Message.Value);
	
} /* SetStatusCommand */

	
/*************************************************************************************/
/* FindInstrumentAction                                                              */
/* Input: String to look for                                                         */
/* Return: Value of enum InstrumentActionType to correspond to input string          */
/*************************************************************************************/

int FindInstrumentAction ( char *InstrumentAction)
{

    if (strcasecmp(InstrumentAction,"nop")==0) return (INSTRUMENT_ACTION_NOP);
    if (strcasecmp(InstrumentAction,"measure")==0) return (INSTRUMENT_ACTION_MEASURE);
    if (strcasecmp(InstrumentAction,"cal")==0) return (INSTRUMENT_ACTION_CAL);
    if (strcasecmp(InstrumentAction,"diag")==0) return (INSTRUMENT_ACTION_DIAG);
    if (strcasecmp(InstrumentAction,"powerup")==0) return (INSTRUMENT_ACTION_POWERUP);
    if (strcasecmp(InstrumentAction,"powerdown")==0) return (INSTRUMENT_ACTION_POWERDOWN);
    if (strcasecmp(InstrumentAction,"lasermirrortune")==0) return (INSTRUMENT_ACTION_LASERMIRRORTUNE);
    return(-1);

} /* FindInstrumentAction */



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
    uint16_t MsgType;

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
	    printf("%d\n",Value);
	    break;

	case 'w':
	    if(argc>3) // better check here before messing around with argv[], hartwig :)
	    {
		Addr=strtod(argv[ArgCount],NULL);
		ArgCount++;
	//    printf("%s %s %s\n",argv[0],argv[1],argv[2]);
		Value=strtod(argv[ArgCount],NULL);
		ArgCount++;
		ret=WriteCommand(Addr,Value);
		printf("Wrote %4x=%4x(%5d) : %4x(%5d)\n",ret,Addr,Addr,Value,Value);
	    }
	    else
		printf("Usage :\t%s w addr data\n", argv[0]);
	    
	    break;

	case 's':
	  MsgType=MAX_MSG_TYPE;

	    if (strcasecmp(argv[ArgCount],"stopquery")==0) {
		MsgType=MSG_TYPE_CHANGE_FLAG_STATUS_QUERY;
		Addr=MSG_TYPE_CHANGE_FLAG_STATUS_QUERY;
		Value=0;
	    };
		
	    if (strcasecmp(argv[ArgCount],"startquery")==0) {
		MsgType=MSG_TYPE_CHANGE_FLAG_STATUS_QUERY;
		Addr=MSG_TYPE_CHANGE_FLAG_STATUS_QUERY;
		Value=1;
	    };

	    if (strcasecmp(argv[ArgCount],"instrumentaction")==0) {
		if (argc>ArgCount+1) { // do we still have a given parameter ?
		    if ( (ret=FindInstrumentAction(argv[ArgCount+1]))>-1) {
			MsgType=MSG_TYPE_CHANGE_FLAG_INSTRUMENT_ACTION;
			Addr=MSG_TYPE_CHANGE_FLAG_INSTRUMENT_ACTION;
			Value=(uint16_t)ret;
		    } else { // no valid paramter
			printf("Error: invalid parameter %s for %s \n",argv[ArgCount], argv[ArgCount+1]);
		    } /* if FindInstrumentAction */
		} else { // we don't have enough parameter 
		    printf("Error please supply parameter for %s\n",argv[ArgCount]);
		} /* if ArgC> */
	    } 

	    /* Etalon commands */	    
	    if (strcasecmp(argv[ArgCount],"etalonnop")==0) {
	      MsgType=MSG_TYPE_CHANGE_FLAG_ETALON_ACTION;
	      Addr=MSG_TYPE_CHANGE_FLAG_ETALON_ACTION;
	      Value=ETALON_ACTION_NOP;
	    };

	    if (strcasecmp(argv[ArgCount],"etalontoggle")==0) {
	      MsgType=MSG_TYPE_CHANGE_FLAG_ETALON_ACTION;
	      Addr=MSG_TYPE_CHANGE_FLAG_ETALON_ACTION;
	      Value=ETALON_ACTION_TOGGLE;
	    };

	    if (strcasecmp(argv[ArgCount],"etalonscan")==0) {
	      MsgType=MSG_TYPE_CHANGE_FLAG_ETALON_ACTION;
	      Addr=MSG_TYPE_CHANGE_FLAG_ETALON_ACTION;
	      Value=ETALON_ACTION_SCAN;
	    };

	    if (strcasecmp(argv[ArgCount],"etalonhome")==0) {
	      MsgType=MSG_TYPE_CHANGE_FLAG_ETALON_ACTION;
	      Addr=MSG_TYPE_CHANGE_FLAG_ETALON_ACTION;
	      Value=ETALON_ACTION_HOME;
	    };

	    if (strcasecmp(argv[ArgCount],"etalonrecal")==0) {
	      MsgType=MSG_TYPE_CHANGE_FLAG_ETALON_ACTION;
	      Addr=MSG_TYPE_CHANGE_FLAG_ETALON_ACTION;
	      Value=ETALON_ACTION_RECAL;
	    };
	    

	    if (strcasecmp(argv[ArgCount],"etalonrecal")==0) {
	      MsgType=MSG_TYPE_CHANGE_FLAG_ETALON_ACTION;
	      Addr=MSG_TYPE_CHANGE_FLAG_ETALON_ACTION;
	      Value=ETALON_ACTION_RECAL;
	    };	    	    


	    if (strcasecmp(argv[ArgCount],"etalonscanstart")==0) {
	      if (argc>ArgCount+1) { // do we still have a given parameter ?
		Value=strtod(argv[ArgCount+1],NULL);
		MsgType=MSG_TYPE_CHANGE_FLAG_SYSTEM_PARAMETER;
		Addr=SYS_PARAMETER_ETALON_SCAN_START;
	      } else { // we don't have enough parameter
		printf("Error please supply parameter for %s %d %d %s\n",argv[ArgCount],argc,ArgCount,argv[ArgCount+1]);
	      }
	    };	    	    
	    
	    if (strcasecmp(argv[ArgCount],"etalonscanstop")==0) {
	      if (argc>ArgCount+1) { // do we still have a given parameter ?
		Value=strtod(argv[ArgCount+1],NULL);
		MsgType=MSG_TYPE_CHANGE_FLAG_SYSTEM_PARAMETER;
		Addr=SYS_PARAMETER_ETALON_SCAN_STOP;
	      } else { // we don't have enough parameter
		printf("Error please supply parameter for %s\n",argv[ArgCount]);
	      }
	    };	    	    
	    
	    if (strcasecmp(argv[ArgCount],"etalonscanstep")==0) {
	      if (argc>ArgCount+1) { // do we still have a given parameter ?
		Value=strtod(argv[ArgCount+1],NULL);
		MsgType=MSG_TYPE_CHANGE_FLAG_SYSTEM_PARAMETER;
		Addr=SYS_PARAMETER_ETALON_SCAN_STEP;
	      } else { // we don't have enough parameter
		printf("Error please supply parameter for %s\n",argv[ArgCount]);
	      }
	    };	    	    
	    
	    if (strcasecmp(argv[ArgCount],"etalononline")==0) {
	      if (argc>ArgCount+1) { // do we still have a given parameter ?
		Value=strtod(argv[ArgCount+1],NULL);
		MsgType=MSG_TYPE_CHANGE_FLAG_SYSTEM_PARAMETER;
		Addr=SYS_PARAMETER_ETALON_ONLINE;
	      } else { // we don't have enough parameter
		printf("Error please supply parameter for %s\n",argv[ArgCount]);
	      }
	    };	  
  	    
	    if (strcasecmp(argv[ArgCount],"etalondither")==0) {
	      if (argc>ArgCount+1) { // do we still have a given parameter ?
		Value=strtod(argv[ArgCount+1],NULL);
		MsgType=MSG_TYPE_CHANGE_FLAG_SYSTEM_PARAMETER;
		Addr=SYS_PARAMETER_ETALON_DITHER;
	      } else { // we don't have enough parameter
		printf("Error please supply parameter for %s\n",argv[ArgCount]);
	      }
	    };	    	    
	    if (strcasecmp(argv[ArgCount],"etalonofflineleft")==0) {
	      if (argc>ArgCount+1) { // do we still have a given parameter ?
		Value=strtod(argv[ArgCount+1],NULL);
		MsgType=MSG_TYPE_CHANGE_FLAG_SYSTEM_PARAMETER;
		Addr=SYS_PARAMETER_ETALON_OFFLINE_LEFT;
	      } else { // we don't have enough parameter
		printf("Error please supply parameter for %s\n",argv[ArgCount]);
	      }
	    };	    	    

	    if (strcasecmp(argv[ArgCount],"etalonofflineright")==0) {
	      if (argc>ArgCount+1) { // do we still have a given parameter ?
		Value=strtod(argv[ArgCount+1],NULL);
		MsgType=MSG_TYPE_CHANGE_FLAG_SYSTEM_PARAMETER;
		Addr=SYS_PARAMETER_ETALON_OFFLINE_RIGHT;
	      } else { // we don't have enough parameter
		printf("Error please supply parameter for %s\n",argv[ArgCount]);
	      }
	    };	    	    

	    if (strcasecmp(argv[ArgCount],"setmask")==0) {
	      if (argc>ArgCount+2) { // do we still have a given parameter ?
		Value=strtod(argv[ArgCount+2],NULL);
		MsgType=MSG_TYPE_CHANGE_MASK;
		// Addr 0-9 for word 0-9 of Channel 0, 10-19 for Channel 1, 20-29 for Channel 2
		Addr=strtod(argv[ArgCount+1],NULL); 
	      } else { // we don't have enough parameter
		printf("Error please supply parameters for %s\n",argv[ArgCount]);
	      }
	    };	    	    
	    
	    // if we got a valid Msg send it
	    if (MsgType<MAX_MSG_TYPE) {
	      SetStatusCommand(MsgType,Addr,Value); 
	    } else {
	      printf("I don't send anything, there was a problem before\n");
	    } /* if MsgType */
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
    
    
    return(EXIT_SUCCESS);
    
}
