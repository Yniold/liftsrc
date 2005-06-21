/*
* $RCSfile: elekIOServ.c,v $ last changed on $Date: 2005-06-21 17:03:45 $ by $Author: rudolf $
*
* $Log: elekIOServ.c,v $
* Revision 1.23  2005-06-21 17:03:45  rudolf
* fixed transmitting UDP Debugmessages in an endless loop which led to a locale DOS attack (system was unusable)
*
* Revision 1.22  2005/06/08 22:43:53  rudolf
* added preliminary master slave support, HH
*
* Revision 1.21  2005/06/08 17:31:51  rudolf
* prepared sockets for sending the data structure between master and slave
*
* Revision 1.19  2005/05/23 15:05:48  rudolf
* changed initialisation of cards to distinguish between Master and Slave mode
*
* Revision 1.18  2005/05/22 19:09:22  rudolf
* fixes for new elekStatus structure
*
* Revision 1.17  2005/05/22 15:02:43  rudolf
* changed BaudRate and ttyX, changed debug output if commands are sent via eCmd
*
* Revision 1.16  2005/05/18 18:26:45  rudolf
* added debug output
*
* Revision 1.15  2005/04/21 16:17:18  rudolf
* fixed bug which led to a udp timeout in ARM version. timer is now initialized properly
*
* Revision 1.14  2005/04/21 13:51:17  rudolf
* more work on conditional compile
*
* Revision 1.13  2005/02/17 21:36:00  martinez
* corrected Mask Address calculation by editing device number
*
* Revision 1.12  2005/02/15 18:09:59  harder
* the counter mask for all channels is set to  but the lsb which counts the number of shots, added some comments, removed bug in shift operation in channel extraction of ccADC
*
* Revision 1.11  2005/02/15 17:28:00  harder
* added comments
*
* Revision 1.10  2005/02/14 17:36:31  martinez
* corrected numering for gate settings
*
* Revision 1.9  2005/02/11 13:44:00  harder
* added InstrumentAction to elekStatus, added support in elekIOServ & eCmd
*
* Revision 1.8  2005/02/11 12:36:12  martinez
* started including gatings in CounterCards, started including instrument action structure in elekIO.h and elekIOServ.c
*
* Revision 1.7  2005/02/02 18:06:53  martinez
* debugged ADC for MCP1 and MCP2 plots
*
* Revision 1.6  2005/02/02 14:34:09  martinez
* using only one mask init routine in elekIOServ
*
* Revision 1.5  2005/01/31 12:16:28  rudolf
* added evalution of true heading and groundspeed in kmh
*
* Revision 1.4  2005/01/31 09:49:31  rudolf
* more work on GPS
*
* Revision 1.3  2005/01/28 17:41:44  rudolf
* Added GetGPSData() to fill the structure with (dummy) values, but at leat it still compiles :-)
*
* Revision 1.2  2005/01/27 18:17:21  rudolf
* added InitGPSReceiver(), modifications for GPS
*
*
*/

#define VERSION 0.8
#define POSIX_SOURCE 1

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#ifdef RUNONPC
#include <asm/msr.h>
#endif

#include <signal.h>
#include <errno.h>
#include <sched.h>

#include "../include/elekGeneral.h"
#include "../include/elekIO.h"
#include "../include/elekIOPorts.h"
#include "elekIOServ.h"
#include "NMEAParser.h"
#include "serial.h"

#define STATUS_INTERVAL  200

#define DEBUGLEVEL 0

//#undefine DEBUG_NOTIMER 0

//#define DEBUG_TIME_TASK 1                         // report time needed for processing tasks

#define DEBUG_SLAVECOM                              // debug communication between master and slave

enum InPortListEnum {  // this list has to be coherent with MessageInPortList
    ELEK_MANUAL_IN,       // port for incoming commands from  eCmd
    ELEK_ETALON_IN,       // port for incoming commands from  etalon
    ELEK_SCRIPT_IN,       // port for incoming commands from  scripting host (not yet existing, HH, Feb2005
    ELEK_STATUS_IN,       // port to receive status data from slaves
    MAX_MESSAGE_INPORTS }; 

enum OutPortListEnum {  // this list has to be coherent with MessageOutPortList
  ELEK_STATUS_OUT,         // port for outgoing messages to status
  ELEK_ELEKIO_STATUS_OUT,  // port for outgoing status to elekIO
  ELEK_ELEKIO_SLAVE_OUT,   // port for outgoing messages to slaves
  ELEK_MANUAL_OUT,         // port for outgoing messages to eCmd
  ELEK_ETALON_OUT,         // port for outgoing messages to etalon
  ELEK_ETALON_STATUS_OUT,  // port for outgoing messages to etalon status, so etalon is directly informed of the status
  ELEK_SCRIPT_OUT,         // port for outgoing messages to script 
  ELEK_DEBUG_OUT,          // port for outgoing messages to debug
  MAX_MESSAGE_OUTPORTS }; 

static struct MessagePortType MessageInPortList[MAX_MESSAGE_INPORTS]={   // order in list defines sequence of polling 
  /* Name, PortNo, ReversePort, IPAddr, fdSocket, MaxMessages, Direction */
  {"Manual",   UDP_ELEK_MANUAL_INPORT,        ELEK_MANUAL_OUT, IP_LOCALHOST, -1, 1,  UDP_IN_PORT},
  {"Etalon",   UDP_ELEK_ETALON_INPORT,        ELEK_ETALON_OUT, IP_LOCALHOST, -1, 10, UDP_IN_PORT},
  {"Script",   UDP_ELEK_SCRIPT_INPORT,        ELEK_SCRIPT_OUT, IP_LOCALHOST, -1, 5,  UDP_IN_PORT},
  {"ElekIOIn", UDP_ELEK_ELEKIO_STATUS_OUTPORT,      -1, IP_LOCALHOST, -1, 1,  UDP_IN_PORT} // status inport from elekIOServ
};

static struct MessagePortType MessageOutPortList[MAX_MESSAGE_OUTPORTS]={    // order in list defines sequence of polling 
  /* Name, PortNo, ReversePort, IPAddr, fdSocket, MaxMessages, Direction */
  {"Status",         UDP_ELEK_STATUS_STATUS_OUTPORT,             -1, IP_STATUS_CLIENT, -1, 0,  UDP_OUT_PORT}, 
  {"ElekIOStatus",   UDP_ELEK_ELEKIO_STATUS_OUTPORT,             -1, IP_ELEKIO_MASTER, -1, 0,  UDP_OUT_PORT},
  {"ElekIOServer",           UDP_ELEK_MANUAL_INPORT, ELEK_ELEKIO_STATUS_OUT, IP_ELEK_SERVER,   -1, 0,  UDP_OUT_PORT}, 
  {"Manual",                UDP_ELEK_MANUAL_OUTPORT, ELEK_MANUAL_IN, IP_LOCALHOST    , -1, 0,  UDP_OUT_PORT}, 
  {"Etalon",                UDP_ELEK_ETALON_OUTPORT, ELEK_ETALON_IN, IP_ETALON_CLIENT, -1, 0,  UDP_OUT_PORT},
  {"EtalonStatus",   UDP_ELEK_ETALON_STATUS_OUTPORT,             -1, IP_STATUS_CLIENT, -1, 0,  UDP_OUT_PORT}, 
  {"Script",                UDP_ELEK_SCRIPT_OUTPORT, ELEK_SCRIPT_IN, IP_SCRIPT_CLIENT, -1, 0,  UDP_OUT_PORT},
  {"DebugPort",              UDP_ELEK_DEBUG_OUTPORT,             -1, IP_DEBUG_CLIENT , -1, 0,  UDP_OUT_PORT}
};

static struct TaskListType TasktoWakeList[MAX_TASKS_TO_WAKE]={     // order defines sequence of wake up after timer
  /* TaskName TaskConn TaskWantStatusOnPort */
  {"Etalon",     ELEK_ETALON_OUT,ELEK_ETALON_STATUS_OUT},    // Etalon Task needs Status info
  {"Script",     ELEK_SCRIPT_OUT,                    -1},
  {      "",                  -1,                    -1}
};

static struct SlaveListType SlaveList[MAXSLAVES]={               // list of slave elekIO units like wingpod or cal box
  /* SlaveName  SlaveIP */
  {"ArmAxis",   IP_ARMAXIS},
  {NULL,NULL},
  {NULL,NULL}
};


static char *strSysParameterDescription[MAX_SYS_PARAMETER]={
  "Etalon online position",
  "Etalon offline step size left",
  "Etalon offline step size right",
  "Etalon online dither step size",
  "Etalon scan start position",
  "Etalon scan stop position",
  "Etalon scan step size"
};


/**********************************************************************************************************/
/* Signal Handler                                                                                         */
/**********************************************************************************************************/

static int StatusFlag=0;
static enum TimerSignalStateEnum TimerState=TIMER_SIGNAL_STATE_INITIAL;

/* Signalhandler */
void signalstatus(int signo)
{
   int iBytesRead = 0;
   extern int StatusFlag;
   char buf[GENERIC_BUF_LEN];

   ++StatusFlag;
   TimerState=TIMER_SIGNAL_STATE_GATHER;

   if(ucPortOpened)	// check if main() has opened the port already
   {
      iBytesRead = read(fdGPS, pDataBuffer, 1024);	// nonblocking (!)

      #if DEBUGLEVEL > 0
      // check if the read failed for any reason
      if(iBytesRead < 0)
      {
         char* pErrorMessage = strerror(errno);
         sprintf(buf,"elekIOServ: GPS read() returned with error %d: ",iBytesRead);
         strcat(buf,pErrorMessage);
         SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],buf);
      }
      #endif
		
      // if bytes received, process them
      if(iBytesRead>0)
      {
         #if DEBUGLEVEL > 0
         sprintf(buf,"Got Data!\n");
         SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],buf);
         #endif
			   
         ParseBuffer(pDataBuffer,iBytesRead);	// feed some characters to the parser
      }
   };

}

/**********************************************************************************************************/
/* Load Module Config                                                                                     */
/**********************************************************************************************************/

/* function to load the config settings for all modules */

void LoadModulesConfig(struct elekStatusType *ptrElekStatus, int IsMaster) {
   
  extern struct MessagePortType MessageInPortList[];
  extern struct MessagePortType MessageOutPortList[];
    
  int       i;
  int       Channel;
  int       Card;
  char      buf[GENERIC_BUF_LEN];
    
  if(IsMaster)
  {
	// Etalon Card
	
	ptrElekStatus->EtalonData.ScanStart.Position =ETALON_SCAN_POS_START;
	ptrElekStatus->EtalonData.ScanStop.Position  =ETALON_SCAN_POS_STOP;
	ptrElekStatus->EtalonData.ScanStepWidth      =ETALON_SCAN_STEP_WIDTH;
	
	ptrElekStatus->EtalonData.Online.Position    =ETALON_STEP_POS_ONLINE;
	ptrElekStatus->EtalonData.DitherStepWidth    =ETALON_STEP_DITHER;
	ptrElekStatus->EtalonData.OfflineStepLeft    =ETALON_STEP_OFFLINE;
	ptrElekStatus->EtalonData.OfflineStepRight   =ETALON_STEP_OFFLINE;
	
	// ADC Channels
	for (Card=0; Card<MAX_ADC_CARD_LIFT; Card ++) {
		ptrElekStatus->ADCCardMaster[Card].NumSamples=0;
		
		for (Channel=0;Channel<MAX_ADC_CHANNEL_PER_CARD; Channel++) {
			ptrElekStatus->ADCCardMaster[Card].ADCChannelData[Channel].ADCData                         =0x00;	    
			ptrElekStatus->ADCCardMaster[Card].ADCChannelData[Channel].SumDat                          =0x00;	    
			ptrElekStatus->ADCCardMaster[Card].ADCChannelData[Channel].SumSqr                          =0x00;	    
			
			ptrElekStatus->ADCCardMaster[Card].ADCChannelConfig[Channel].ADCChannelConfigBit.Unused    =0x00;	    
			ptrElekStatus->ADCCardMaster[Card].ADCChannelConfig[Channel].ADCChannelConfigBit.Offset    =0x00;
			ptrElekStatus->ADCCardMaster[Card].ADCChannelConfig[Channel].ADCChannelConfigBit.Gain      =0x00;
			ptrElekStatus->ADCCardMaster[Card].ADCChannelConfig[Channel].ADCChannelConfigBit.Bridge    =0x00;
			ptrElekStatus->ADCCardMaster[Card].ADCChannelConfig[Channel].ADCChannelConfigBit.MuxChannel=Channel;
			//	    printf("Channel %d : Config %04x\n",Channel,
			//		   ptrElekStatus->ADCCardMaster[Card].ADCChannelConfig[Channel].ADCChannelConfig);
		} /* for Channel */
	} /* for Card */
	
	// for bridge pressure sensors we need two adc channels, one for the signal, the other for the bridge power 
	// those pairs are set here
	Card=0; Channel=0;
	ptrElekStatus->ADCCardMaster[Card].ADCChannelConfig[Channel].ADCChannelConfigBit.Unused    =0x00;	    
	ptrElekStatus->ADCCardMaster[Card].ADCChannelConfig[Channel].ADCChannelConfigBit.Offset    =0x01;
	ptrElekStatus->ADCCardMaster[Card].ADCChannelConfig[Channel].ADCChannelConfigBit.Gain      =0x2;
	ptrElekStatus->ADCCardMaster[Card].ADCChannelConfig[Channel].ADCChannelConfigBit.Bridge    =0x1;
	ptrElekStatus->ADCCardMaster[Card].ADCChannelConfig[Channel].ADCChannelConfigBit.MuxChannel=Channel;
	
	Card=0; Channel=1;
	ptrElekStatus->ADCCardMaster[Card].ADCChannelConfig[Channel].ADCChannelConfigBit.Unused    =0x00;	    
	ptrElekStatus->ADCCardMaster[Card].ADCChannelConfig[Channel].ADCChannelConfigBit.Offset    =0x01;
	ptrElekStatus->ADCCardMaster[Card].ADCChannelConfig[Channel].ADCChannelConfigBit.Gain      =0x2;
	ptrElekStatus->ADCCardMaster[Card].ADCChannelConfig[Channel].ADCChannelConfigBit.Bridge    =0x1;
	ptrElekStatus->ADCCardMaster[Card].ADCChannelConfig[Channel].ADCChannelConfigBit.MuxChannel=Channel;
	
	Card=0; Channel=6;  
	ptrElekStatus->ADCCardMaster[Card].ADCChannelConfig[Channel].ADCChannelConfigBit.Unused    =0x00;	    
	ptrElekStatus->ADCCardMaster[Card].ADCChannelConfig[Channel].ADCChannelConfigBit.Offset    =0x01;
	ptrElekStatus->ADCCardMaster[Card].ADCChannelConfig[Channel].ADCChannelConfigBit.Gain      =0x2;
	ptrElekStatus->ADCCardMaster[Card].ADCChannelConfig[Channel].ADCChannelConfigBit.Bridge    =0x1;
	ptrElekStatus->ADCCardMaster[Card].ADCChannelConfig[Channel].ADCChannelConfigBit.MuxChannel=Channel;
	
	Card=0; Channel=7; 
	ptrElekStatus->ADCCardMaster[Card].ADCChannelConfig[Channel].ADCChannelConfigBit.Unused    =0x00;	    
	ptrElekStatus->ADCCardMaster[Card].ADCChannelConfig[Channel].ADCChannelConfigBit.Offset    =0x01;
	ptrElekStatus->ADCCardMaster[Card].ADCChannelConfig[Channel].ADCChannelConfigBit.Gain      =0x2;
	ptrElekStatus->ADCCardMaster[Card].ADCChannelConfig[Channel].ADCChannelConfigBit.Bridge    =0x1;
	ptrElekStatus->ADCCardMaster[Card].ADCChannelConfig[Channel].ADCChannelConfigBit.MuxChannel=Channel;
	
	
	
	// MFC Channels
	for (Card=0; Card<MAX_MFC_CARD_LIFT; Card ++) {
		ptrElekStatus->MFCCardMaster[Card].NumSamples=0;
		for (Channel=0;Channel<MAX_MFC_CHANNEL_PER_CARD; Channel++) {
			ptrElekStatus->MFCCardMaster[Card].MFCChannelData[Channel].SetFlow                         =0x00;	    
			ptrElekStatus->MFCCardMaster[Card].MFCChannelData[Channel].Flow                            =0x00;	    
			ptrElekStatus->MFCCardMaster[Card].MFCChannelData[Channel].SumDat                          =0x00;	    
			ptrElekStatus->MFCCardMaster[Card].MFCChannelData[Channel].SumSqr                          =0x00;	    
	
			ptrElekStatus->MFCCardMaster[Card].MFCChannelConfig[Channel].MFCChannelConfigBit.Unused    =0x00;	    
			ptrElekStatus->MFCCardMaster[Card].MFCChannelConfig[Channel].MFCChannelConfigBit.Ch0       =0x00;   // all valve forcings off
			ptrElekStatus->MFCCardMaster[Card].MFCChannelConfig[Channel].MFCChannelConfigBit.Ch1       =0x00;
			ptrElekStatus->MFCCardMaster[Card].MFCChannelConfig[Channel].MFCChannelConfigBit.Ch2       =0x00;
			ptrElekStatus->MFCCardMaster[Card].MFCChannelConfig[Channel].MFCChannelConfigBit.Ch3       =0x00;
			ptrElekStatus->MFCCardMaster[Card].MFCChannelConfig[Channel].MFCChannelConfigBit.MuxChannel=Channel;
		} /* for Channel */
	} /* for Card */
	
	// Valve Channels
	for (Card=0; Card<MAX_VALVE_CARD_LIFT; Card ++) {
		for (Channel=0;Channel<MAX_VALVE_CHANNEL_PER_CARD; Channel++) {
			ptrElekStatus->ValveCardMaster[Card].Valve=bitset(ptrElekStatus->ValveCardMaster[Card].Valve,Channel,0); 	    
		} /* for Channel */
	} /* for Card */
	
	// DCDC4 Channels
	for (Card=0; Card<MAX_DCDC4_CARD_LIFT; Card ++) {
		for (Channel=0;Channel<MAX_DCDC4_CHANNEL_PER_CARD; Channel++) {
			ptrElekStatus->DCDC4CardMaster[Card].Channel[Channel]=0x00;	    
		} /* for Channel */
	} /* for Card */
	
	
	
	// Counter Card
	
	// init Mask for now all 1
	for (Channel=0; Channel<MAX_COUNTER_CHANNEL; Channel++) 
	{
		// setup Channel Masks
		for (i=0;i<COUNTER_MASK_WIDTH; i++) 
		{
			ptrElekStatus->CounterCardMaster.Channel[Channel].Mask[i]=0x0ffff;   
		} // for i
		// we do not want to count the first data word in the sumcounts which are the number of lasershots
		ptrElekStatus->CounterCardMaster.Channel[Channel].Mask[0]=0x0fffe;
		for (i=0;i<COUNTER_MASK_WIDTH;i++) 
		{
			sprintf(buf,"ElekIOServ(M) : Mask #%d %x",i,ptrElekStatus->CounterCardMaster.Channel[Channel].Mask[i]);
			SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],buf);
		} /* for i */	
	} // for Channel
		
	ptrElekStatus->CounterCardMaster.MasterDelay=0x50;                                                   // master delay
		
		// set CounterCard ShiftDelays for now 1
	//  for(i=0; i<MAX_COUNTER_CHANNEL;i++) ptrElekStatus->CounterCard.Channel[i].ShiftDelay=1;     // channel delay
	
	ptrElekStatus->CounterCardMaster.Channel[0].ShiftDelay=0x21;  //PMT shift
	ptrElekStatus->CounterCardMaster.Channel[1].ShiftDelay=0x32;  //MCP1 shift
	ptrElekStatus->CounterCardMaster.Channel[2].ShiftDelay=0x32;  //MCP2 shift
	
	// set CounterCard GateDelays 
	// set to zero for channel 0, which is the PMT and has no gate register 
	ptrElekStatus->CounterCardMaster.Channel[0].GateDelay=0; 
	ptrElekStatus->CounterCardMaster.Channel[0].GateWidth=0;	
	for(i=1; i<MAX_COUNTER_CHANNEL;i++) {// skip channel 0 which is the PMT and has no gate register 
		ptrElekStatus->CounterCardMaster.Channel[i].GateDelay=0x10;
		ptrElekStatus->CounterCardMaster.Channel[i].GateWidth=1000;	
	}
	
	// init InstrumentFlags
	
	ptrElekStatus->InstrumentFlags.StatusSave=1;                   // we do want to store data
	ptrElekStatus->InstrumentFlags.StatusQuery=1;                  // we want to query the Status
	ptrElekStatus->InstrumentFlags.EtalonAction=ETALON_ACTION_NOP; // do nothing with the etalon for now
  }
	else
	// ----------
	// SLAVE Mode 
	// ----------
 {
	// ADC Channels
	for (Card=0; Card<MAX_ADC_CARD_WP; Card ++) {
		ptrElekStatus->ADCCardSlave[Card].NumSamples=0;
		
		for (Channel=0;Channel<MAX_ADC_CHANNEL_PER_CARD; Channel++) {
			ptrElekStatus->ADCCardSlave[Card].ADCChannelData[Channel].ADCData                         =0x00;	    
			ptrElekStatus->ADCCardSlave[Card].ADCChannelData[Channel].SumDat                          =0x00;	    
			ptrElekStatus->ADCCardSlave[Card].ADCChannelData[Channel].SumSqr                          =0x00;	    
			
			ptrElekStatus->ADCCardSlave[Card].ADCChannelConfig[Channel].ADCChannelConfigBit.Unused    =0x00;	    
			ptrElekStatus->ADCCardSlave[Card].ADCChannelConfig[Channel].ADCChannelConfigBit.Offset    =0x00;
			ptrElekStatus->ADCCardSlave[Card].ADCChannelConfig[Channel].ADCChannelConfigBit.Gain      =0x00;
			ptrElekStatus->ADCCardSlave[Card].ADCChannelConfig[Channel].ADCChannelConfigBit.Bridge    =0x00;
			ptrElekStatus->ADCCardSlave[Card].ADCChannelConfig[Channel].ADCChannelConfigBit.MuxChannel=Channel;
		} /* for Channel */
	} /* for Card */
	
	// for bridge pressure sensors we need two adc channels, one for the signal, the other for the bridge power 
	// those pairs are set here
	Card=0; Channel=0;
	ptrElekStatus->ADCCardSlave[Card].ADCChannelConfig[Channel].ADCChannelConfigBit.Unused    =0x00;	    
	ptrElekStatus->ADCCardSlave[Card].ADCChannelConfig[Channel].ADCChannelConfigBit.Offset    =0x01;
	ptrElekStatus->ADCCardSlave[Card].ADCChannelConfig[Channel].ADCChannelConfigBit.Gain      =0x2;
	ptrElekStatus->ADCCardSlave[Card].ADCChannelConfig[Channel].ADCChannelConfigBit.Bridge    =0x1;
	ptrElekStatus->ADCCardSlave[Card].ADCChannelConfig[Channel].ADCChannelConfigBit.MuxChannel=Channel;
	
	Card=0; Channel=1;
	ptrElekStatus->ADCCardSlave[Card].ADCChannelConfig[Channel].ADCChannelConfigBit.Unused    =0x00;	    
	ptrElekStatus->ADCCardSlave[Card].ADCChannelConfig[Channel].ADCChannelConfigBit.Offset    =0x01;
	ptrElekStatus->ADCCardSlave[Card].ADCChannelConfig[Channel].ADCChannelConfigBit.Gain      =0x2;
	ptrElekStatus->ADCCardSlave[Card].ADCChannelConfig[Channel].ADCChannelConfigBit.Bridge    =0x1;
	ptrElekStatus->ADCCardSlave[Card].ADCChannelConfig[Channel].ADCChannelConfigBit.MuxChannel=Channel;
	
	Card=0; Channel=6;  
	ptrElekStatus->ADCCardSlave[Card].ADCChannelConfig[Channel].ADCChannelConfigBit.Unused    =0x00;	    
	ptrElekStatus->ADCCardSlave[Card].ADCChannelConfig[Channel].ADCChannelConfigBit.Offset    =0x01;
	ptrElekStatus->ADCCardSlave[Card].ADCChannelConfig[Channel].ADCChannelConfigBit.Gain      =0x2;
	ptrElekStatus->ADCCardSlave[Card].ADCChannelConfig[Channel].ADCChannelConfigBit.Bridge    =0x1;
	ptrElekStatus->ADCCardSlave[Card].ADCChannelConfig[Channel].ADCChannelConfigBit.MuxChannel=Channel;
	
	Card=0; Channel=7; 
	ptrElekStatus->ADCCardSlave[Card].ADCChannelConfig[Channel].ADCChannelConfigBit.Unused    =0x00;	    
	ptrElekStatus->ADCCardSlave[Card].ADCChannelConfig[Channel].ADCChannelConfigBit.Offset    =0x01;
	ptrElekStatus->ADCCardSlave[Card].ADCChannelConfig[Channel].ADCChannelConfigBit.Gain      =0x2;
	ptrElekStatus->ADCCardSlave[Card].ADCChannelConfig[Channel].ADCChannelConfigBit.Bridge    =0x1;
	ptrElekStatus->ADCCardSlave[Card].ADCChannelConfig[Channel].ADCChannelConfigBit.MuxChannel=Channel;
	
	
	// ADC 24 bit
	for (Card=0; Card<MAX_24BIT_ADC_CARDS_WP; Card ++) 
	{
		ptrElekStatus->ADC24CardsSlave[Card].NumSamples=0;
		
		for (Channel=0;Channel<MAX_ADC_CHANNEL_PER_CARD; Channel++) 
		{
			ptrElekStatus->ADC24CardsSlave[Card].ADCChannelData[Channel].ADCChannelData = 0x00;	    
		} /* for Channel */
	} /* for Card */
	
	// MFC Channels
	for (Card=0; Card<MAX_MFC_CARD_WP; Card ++) {
		ptrElekStatus->MFCCardSlave[Card].NumSamples=0;
		for (Channel=0;Channel<MAX_MFC_CHANNEL_PER_CARD; Channel++) {
			ptrElekStatus->MFCCardSlave[Card].MFCChannelData[Channel].SetFlow                         =0x00;	    
			ptrElekStatus->MFCCardSlave[Card].MFCChannelData[Channel].Flow                            =0x00;	    
			ptrElekStatus->MFCCardSlave[Card].MFCChannelData[Channel].SumDat                          =0x00;	    
			ptrElekStatus->MFCCardSlave[Card].MFCChannelData[Channel].SumSqr                          =0x00;	    
	
			ptrElekStatus->MFCCardSlave[Card].MFCChannelConfig[Channel].MFCChannelConfigBit.Unused    =0x00;	    
			ptrElekStatus->MFCCardSlave[Card].MFCChannelConfig[Channel].MFCChannelConfigBit.Ch0       =0x00;   // all valve forcings off
			ptrElekStatus->MFCCardSlave[Card].MFCChannelConfig[Channel].MFCChannelConfigBit.Ch1       =0x00;
			ptrElekStatus->MFCCardSlave[Card].MFCChannelConfig[Channel].MFCChannelConfigBit.Ch2       =0x00;
			ptrElekStatus->MFCCardSlave[Card].MFCChannelConfig[Channel].MFCChannelConfigBit.Ch3       =0x00;
			ptrElekStatus->MFCCardSlave[Card].MFCChannelConfig[Channel].MFCChannelConfigBit.MuxChannel=Channel;
		} /* for Channel */
	} /* for Card */
	
	// Valve Channels
	for (Card=0; Card<MAX_VALVE_CARD_WP; Card ++) {
		for (Channel=0;Channel<MAX_VALVE_CHANNEL_PER_CARD; Channel++) {
			ptrElekStatus->ValveCardSlave[Card].Valve=bitset(ptrElekStatus->ValveCardSlave[Card].Valve,Channel,0); 	    
		} /* for Channel */
	} /* for Card */
	
// Counter Card
	
	// init Mask for now all 1
	for (Channel=0; Channel<MAX_COUNTER_CHANNEL; Channel++) 
	{
		// setup Channel Masks
		for (i=0;i<COUNTER_MASK_WIDTH; i++) 
		{
			ptrElekStatus->CounterCardSlave.Channel[Channel].Mask[i]=0x0ffff;   
		} // for i
		// we do not want to count the first data word in the sumcounts which are the number of lasershots
		
		ptrElekStatus->CounterCardSlave.Channel[Channel].Mask[0]=0x0fffe;
		for (i=0;i<COUNTER_MASK_WIDTH;i++) 
		{
			sprintf(buf,"ElekIOServ(S) : Mask #%d %x",i,ptrElekStatus->CounterCardSlave.Channel[Channel].Mask[i]);
			SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],buf);
		} /* for i */
	} // for Channel
	
	
	ptrElekStatus->CounterCardSlave.MasterDelay=0x50;                                                   // master delay
		
		// set CounterCard ShiftDelays for now 1
	//  for(i=0; i<MAX_COUNTER_CHANNEL;i++) ptrElekStatus->CounterCard.Channel[i].ShiftDelay=1;     // channel delay
	
	ptrElekStatus->CounterCardSlave.Channel[0].ShiftDelay=0x21;  //PMT shift
	ptrElekStatus->CounterCardSlave.Channel[1].ShiftDelay=0x32;  //MCP1 shift
	ptrElekStatus->CounterCardSlave.Channel[2].ShiftDelay=0x32;  //MCP2 shift
	
	// set CounterCard GateDelays 
	// set to zero for channel 0, which is the PMT and has no gate register 
	ptrElekStatus->CounterCardSlave.Channel[0].GateDelay=0; 
	ptrElekStatus->CounterCardSlave.Channel[0].GateWidth=0;	
	for(i=1; i<MAX_COUNTER_CHANNEL;i++) {// skip channel 0 which is the PMT and has no gate register 
		ptrElekStatus->CounterCardSlave.Channel[i].GateDelay=0x10;
		ptrElekStatus->CounterCardSlave.Channel[i].GateWidth=1000;	
	}	
	
	// init InstrumentFlags
	
	ptrElekStatus->InstrumentFlags.StatusSave=1;                   // we do want to store data
	ptrElekStatus->InstrumentFlags.StatusQuery=1;                  // we want to query the Status
	ptrElekStatus->InstrumentFlags.EtalonAction=ETALON_ACTION_NOP; // do nothing with the etalon for now
	
 };
} /* LoadModulesConfig */  


/**********************************************************************************************************/
/* SetCounterCardMask                                                                                     */
/**********************************************************************************************************/
                                                                                    
void SetCounterCardMask(struct elekStatusType *ptrElekStatus, int IsMaster)
{

  int Channel; 
  uint16_t CounterStatus;
  int i;
  
  // MASTER
  
  if(IsMaster)
  {
	for (Channel=0; Channel<MAX_COUNTER_CHANNEL; Channel++) 
	{
		CounterStatus=(Channel <<4);
		elkWriteData(ELK_COUNTER_STATUS, CounterStatus);
		
		// setup Channel Masks, but only ten words because of depth of shift register
		for (i=0;i<10; i++) 
		{
			elkWriteData(ELK_COUNTER_MASK,ptrElekStatus->CounterCardMaster.Channel[Channel].Mask[i]);
		} // for i
	} // for Channel
  }
  
  // SLAVE
  else
  {
	for (Channel=0; Channel<MAX_COUNTER_CHANNEL; Channel++) 
	{
		
		CounterStatus=(Channel <<4);
		elkWriteData(ELK_COUNTER_STATUS, CounterStatus);
		
		// setup Channel Masks, but only ten words because of depth of shift register
		for (i=0;i<10; i++) {
			elkWriteData(ELK_COUNTER_MASK,ptrElekStatus->CounterCardSlave.Channel[Channel].Mask[i]);
		} // for i
	} // for Channel
  }
  
} /*SetCounterCardMask*/
 


/**********************************************************************************************************/
/* Init CounterCard                                                                                       */
/**********************************************************************************************************/

int InitCounterCard (struct elekStatusType *ptrElekStatus,int IsMaster) {

  int Channel;
  int i;
  int Page;
  int TimeSlotinPage;
  int InitError=0;
  int ret;
  int       TimeSlot;
  uint16_t  CounterStatus;


  if(IsMaster)
  {
	// Now program
	
	// CounterCard ShiftDelays
	// MasterDelay
	
	ret=elkWriteData(ELK_COUNTER_DELAY_SHIFT+2*MAX_COUNTER_CHANNEL, 
				ptrElekStatus->CounterCardMaster.MasterDelay);
	// proofread
	ret=elkReadData(ELK_COUNTER_DELAY_SHIFT+2*MAX_COUNTER_CHANNEL);
	if (ret!=ptrElekStatus->CounterCardMaster.MasterDelay) {
		// failed to init CC
		return(INIT_MODULE_FAILED);
	}
	
	for(Channel=0; Channel<MAX_COUNTER_CHANNEL;Channel++) 
		ret=elkWriteData(ELK_COUNTER_DELAY_SHIFT+2*Channel,ptrElekStatus->CounterCardMaster.Channel[Channel].ShiftDelay);
	
	// CounterCard GateDelays
	for(Channel=1; Channel<MAX_COUNTER_CHANNEL;Channel++) { // skip channel 0 which is the PMT and has no gate register 
		ret=elkWriteData(ELK_COUNTER_DELAY_GATE+4*(Channel-1),ptrElekStatus->CounterCardMaster.Channel[Channel].GateDelay);
		ret=elkWriteData(ELK_COUNTER_DELAY_GATE+4*(Channel-1)+2,ptrElekStatus->CounterCardMaster.Channel[Channel].GateWidth);	
	}
	
	// initialize mask shift register for all channels
	SetCounterCardMask(ptrElekStatus, IsMaster);
	
	// initialize data counter registers  
	for (Channel=0; Channel<MAX_COUNTER_CHANNEL; Channel++) {
		
		CounterStatus=(Channel <<4);
		elkWriteData(ELK_COUNTER_STATUS, CounterStatus);
		
		// init Data to 0
		TimeSlot=0;
		for (Page=0; Page<MAX_COUNTER_PAGE; Page++) {
			// set page and Channel in status register
			CounterStatus=(Channel <<4) | Page;
			elkWriteData(ELK_COUNTER_STATUS, CounterStatus);
			
			for(TimeSlotinPage=0; TimeSlotinPage<MAX_COUNTER_SLOTS_PER_PAGE; TimeSlotinPage++, TimeSlot++) {
		// clear buffer for now write test pattern
		elkWriteData(ELK_COUNTER_COUNTS+TimeSlotinPage*2,TimeSlot+Channel);
		// printf("C %d T %d %3d \n",Channel,TimeSlot,TimeSlotinPage);
			}/* for TimeSlotinPage*/
		} /*Page*/
		
		
	}/*Channel*/
	
	// request switch of banks so we add upcoming 
	// counts into the initialized registers 
	
	elkWriteData(ELK_COUNTER_STATUS, ELK_COUNTER_STATUS_FLIP );
	
	return (INIT_MODULE_SUCCESS);
  }
  else // SLAVE Mode
  {
	ret=elkWriteData(ELK_COUNTER_DELAY_SHIFT+2*MAX_COUNTER_CHANNEL, 
				ptrElekStatus->CounterCardSlave.MasterDelay);
	// proofread
	ret=elkReadData(ELK_COUNTER_DELAY_SHIFT+2*MAX_COUNTER_CHANNEL);
	if (ret!=ptrElekStatus->CounterCardSlave.MasterDelay) {
		// failed to init CC
		return(INIT_MODULE_FAILED);
	}
	
	for(Channel=0; Channel<MAX_COUNTER_CHANNEL;Channel++) 
		ret=elkWriteData(ELK_COUNTER_DELAY_SHIFT+2*Channel,ptrElekStatus->CounterCardSlave.Channel[Channel].ShiftDelay);
	
	// CounterCard GateDelays
	for(Channel=1; Channel<MAX_COUNTER_CHANNEL;Channel++) { // skip channel 0 which is the PMT and has no gate register 
		ret=elkWriteData(ELK_COUNTER_DELAY_GATE+4*(Channel-1),ptrElekStatus->CounterCardSlave.Channel[Channel].GateDelay);
		ret=elkWriteData(ELK_COUNTER_DELAY_GATE+4*(Channel-1)+2,ptrElekStatus->CounterCardSlave.Channel[Channel].GateWidth);	
	}
	
	// initialize mask shift register for all channels
	SetCounterCardMask(ptrElekStatus, IsMaster);
	
	// initialize data counter registers  
	for (Channel=0; Channel<MAX_COUNTER_CHANNEL; Channel++) {
		
		CounterStatus=(Channel <<4);
		elkWriteData(ELK_COUNTER_STATUS, CounterStatus);
		
		// init Data to 0
		TimeSlot=0;
		for (Page=0; Page<MAX_COUNTER_PAGE; Page++) {
			// set page and Channel in status register
			CounterStatus=(Channel <<4) | Page;
			elkWriteData(ELK_COUNTER_STATUS, CounterStatus);
			
			for(TimeSlotinPage=0; TimeSlotinPage<MAX_COUNTER_SLOTS_PER_PAGE; TimeSlotinPage++, TimeSlot++) {
		// clear buffer for now write test pattern
		elkWriteData(ELK_COUNTER_COUNTS+TimeSlotinPage*2,TimeSlot+Channel);
		// printf("C %d T %d %3d \n",Channel,TimeSlot,TimeSlotinPage);
			}/* for TimeSlotinPage*/
		} /*Page*/
		
		
	}/*Channel*/
	
	// request switch of banks so we add upcoming 
	// counts into the initialized registers 
	
	elkWriteData(ELK_COUNTER_STATUS, ELK_COUNTER_STATUS_FLIP );
	
	return (INIT_MODULE_SUCCESS);
  }
} /* Init CounterCard */

/**********************************************************************************************************/
/* Init ADCCardMaster                                                                                       */
/**********************************************************************************************************/

int InitADCCard (struct elekStatusType *ptrElekStatus, int IsMaster) {

  int       Channel;
  int       Card;
  int       ret=1;
  unsigned  ADC_CfgAddress;
  
  
  if(IsMaster)
  {
	// configure ADC Card (Master)
	
	for (Card=0; Card<MAX_ADC_CARD_LIFT; Card ++) {
		ADC_CfgAddress=ELK_ADC_BASE+ELK_ADC_CONFIG+Card*ELK_ADC_NUM_ADR;
		for (Channel=0;Channel<MAX_ADC_CHANNEL_PER_CARD; Channel++) {	    
			ret=elkWriteData(ADC_CfgAddress+2*Channel,
					ptrElekStatus->ADCCardMaster[Card].ADCChannelConfig[Channel].ADCChannelConfig);
			// proof read
			if (elkReadData(ADC_CfgAddress+2*Channel)!=
		ptrElekStatus->ADCCardMaster[Card].ADCChannelConfig[Channel].ADCChannelConfig) {
		return (INIT_MODULE_FAILED); // mark ret=0;
			} /* if elkRead */
			
		} /* for Channel */
	} /* for Card */
	
	return (INIT_MODULE_SUCCESS);
  }
  else /*IsMaster*/
  {
		// configure ADC Card (Slave)
		
		for (Card=0; Card<MAX_ADC_CARD_WP; Card ++) 
		{
			ADC_CfgAddress=ELK_ADC_BASE_WP+ELK_ADC_CONFIG+Card*ELK_ADC_NUM_ADR;
			for (Channel=0;Channel<MAX_ADC_CHANNEL_PER_CARD; Channel++) 
			{	    
				ret=elkWriteData(ADC_CfgAddress+2*Channel,
						ptrElekStatus->ADCCardSlave[Card].ADCChannelConfig[Channel].ADCChannelConfig);
				// proof read
				if (elkReadData(ADC_CfgAddress+2*Channel)!=
					ptrElekStatus->ADCCardSlave[Card].ADCChannelConfig[Channel].ADCChannelConfig) 
				{
					return (INIT_MODULE_FAILED); // mark ret=0;
				} /* if elkRead */
				
			} /* for Channel */
		} /* for Card */
		return (INIT_MODULE_SUCCESS);
	};
  
} /* Init ADCCardMaster */

/**********************************************************************************************************/
/* Init MFCCard                                                                                       */
/**********************************************************************************************************/

int InitMFCCard (struct elekStatusType *ptrElekStatus, int IsMaster) {

  int       Channel;
  int       Card;
  int       ret=1;
  unsigned  MFC_CfgAddress;
  unsigned  DAC_CfgAddress;
  
  
  if(IsMaster)
  {
	// configure MFC Card Lift
	
	for (Card=0; Card<MAX_MFC_CARD_LIFT; Card ++) {
		MFC_CfgAddress=ELK_MFC_BASE+ELK_MFC_CONFIG+Card*ELK_MFC_NUM_ADR;
		DAC_CfgAddress=ELK_DAC_BASE+Card*ELK_DAC_NUM_ADR;
		for (Channel=0;Channel<MAX_MFC_CHANNEL_PER_CARD; Channel++) {
			// init ADC part of each channel
			ret=elkWriteData(MFC_CfgAddress+2*Channel,
					ptrElekStatus->MFCCardMaster[Card].MFCChannelConfig[Channel].MFCChannelConfig);
			// proof read
			if (elkReadData(MFC_CfgAddress+2*Channel)!=
		ptrElekStatus->MFCCardMaster[Card].MFCChannelConfig[Channel].MFCChannelConfig) {
		return (INIT_MODULE_FAILED); // mark ret=0;
			} /* if elkRead */
			
			// init DAC part
			ret=elkWriteData(DAC_CfgAddress+2*Channel,
					ptrElekStatus->MFCCardMaster[Card].MFCChannelData[Channel].SetFlow);
			
		} /* for Channel */
	} /* for Card */
	
	// enable DACPWM
	ret=elkWriteData(ELK_DACPWM_BASE,0xff);
	
	
	return (INIT_MODULE_SUCCESS);
  }
  else // SLAVE
  {
	// configure MFC Card Wingpod
	
	for (Card=0; Card<MAX_MFC_CARD_WP; Card ++) {
		MFC_CfgAddress=ELK_MFC_BASE+ELK_MFC_CONFIG+Card*ELK_MFC_NUM_ADR;
		DAC_CfgAddress=ELK_DAC_BASE+Card*ELK_DAC_NUM_ADR;
		for (Channel=0;Channel<MAX_MFC_CHANNEL_PER_CARD; Channel++) {
			// init ADC part of each channel
			ret=elkWriteData(MFC_CfgAddress+2*Channel,
					ptrElekStatus->MFCCardSlave[Card].MFCChannelConfig[Channel].MFCChannelConfig);
			// proof read
			if (elkReadData(MFC_CfgAddress+2*Channel)!=
		ptrElekStatus->MFCCardSlave[Card].MFCChannelConfig[Channel].MFCChannelConfig) {
		return (INIT_MODULE_FAILED); // mark ret=0;
			} /* if elkRead */
			
			// init DAC part
			ret=elkWriteData(DAC_CfgAddress+2*Channel,
					ptrElekStatus->MFCCardSlave[Card].MFCChannelData[Channel].SetFlow);
			
		} /* for Channel */
	} /* for Card */
	
	// enable DACPWM
	ret=elkWriteData(ELK_DACPWM_BASE,0xff);
	
	
	return (INIT_MODULE_SUCCESS);
  }
  
} /* Init MFCCard */

/**********************************************************************************************************/
/* Init EtalonCard                                                                                        */
/**********************************************************************************************************/

int InitEtalonCard (struct elekStatusType *ptrElekStatus) {
  
  int ret;

  // configure Etalon Card
  // set speed to 0x10 and acceleration to 0x10
  elkWriteData(ELK_STEP_SETSPD,0x1010);
  ret=elkReadData(ELK_STEP_SETSPD);
  
  // check if we got the same value that we wrote
  if (ret!=0x1010) {
    return (INIT_MODULE_FAILED);
  }

  // set voltages for hold pos and slow moving
  elkWriteData(ELK_STEP_SETVOLT,0x1010);


  return (INIT_MODULE_SUCCESS);
  
} /* Init EtalonCard */


/**********************************************************************************************************/
/* Init ValveCard                                                                                        */
/**********************************************************************************************************/

int InitValveCard (struct elekStatusType *ptrElekStatus, int IsMaster) {
  
  int Card;
  int ret;

  if(IsMaster)
  {
	// configure Valve Card for LIFT
	
	for (Card=0; Card<MAX_VALVE_CARD_LIFT; Card ++) {
		// switch all Valves off
		ret=elkWriteData(ELK_VALVE_BASE+Card*2,0);
	// set voltage to 0
		ret=elkWriteData(ELK_PWM_VALVE_BASE+Card*2,0);
		ret=elkReadData(ELK_PWM_VALVE_BASE+Card*2);
		if (ret!=0) {
			return (INIT_MODULE_FAILED);
		}
	} /* for Card */
	
	return (INIT_MODULE_SUCCESS);
  }
  else
  {
	// configure Valve Card for Wingpod
	
	for (Card=0; Card<MAX_VALVE_CARD_WP; Card ++) {
		// switch all Valves off
		ret=elkWriteData(ELK_VALVE_BASE+Card*2,0);
	// set voltage to 0
		ret=elkWriteData(ELK_PWM_VALVE_BASE+Card*2,0);
		ret=elkReadData(ELK_PWM_VALVE_BASE+Card*2);
		if (ret!=0) {
			return (INIT_MODULE_FAILED);
		}
	} /* for Card */
	
	return (INIT_MODULE_SUCCESS);
  }
  
} /* Init ValveCard */

/**********************************************************************************************************/
/* Init DCDC4Card                                                                                        */
/**********************************************************************************************************/

int InitDCDC4Card (struct elekStatusType *ptrElekStatus) {
  
  int ret;
  int Channel;
  int Card;

  // configure DCDC4 Card
  // set voltage to 0
  for (Card=0;Card<MAX_DCDC4_CARD_LIFT;Card++) {
    
    for (Channel=0;Channel<MAX_DCDC4_CHANNEL_PER_CARD; Channel++) {
      elkWriteData(ELK_PWM_DCDC4_BASE+2*Channel,0);
      ret=elkReadData(ELK_PWM_DCDC4_BASE+2*Channel);      
      // check if we got the same value that we wrote
      if (ret!=0) {
	return (INIT_MODULE_FAILED);
      }
    } /* for Channel */
  } /* for Card */
  return (INIT_MODULE_SUCCESS);
  
} /* Init DCDC4Card */

/**********************************************************************************************************/
/* Init TemperatureCard                                                                                        */
/**********************************************************************************************************/

int InitTempCard (struct elekStatusType *ptrElekStatus, int IsMaster) {

  int ret;
  int Card;

  if(IsMaster)
  {
	// configure Temperature Card
	// reset Busy bit
	elkWriteData(ELK_TEMP_CTRL,0);
	
	ret=elkReadData(ELK_TEMP_CTRL);
	
	// check if we got the same value that we wrote
	if (ret!=0) {
		return (INIT_MODULE_FAILED);
	}
	
	for (Card=0; Card<MAX_TEMP_SENSOR_CARD_LIFT; Card++) {
		ptrElekStatus->TempSensCardMaster[Card].NumMissed=0;              // reset Missed Reading Counter
	};
	
	
	return (INIT_MODULE_SUCCESS);
  }
  else // SLAVE
  {
	// configure Temperature Card
	// reset Busy bit
	elkWriteData(ELK_TEMP_CTRL,0);
	
	ret=elkReadData(ELK_TEMP_CTRL);
	
	// check if we got the same value that we wrote
	if (ret!=0) {
		return (INIT_MODULE_FAILED);
	}
	
	for (Card=0; Card<MAX_TEMP_SENSOR_CARD_WP; Card++) {
		ptrElekStatus->TempSensCardSlave[Card].NumMissed=0;              // reset Missed Reading Counter
	};
	
	
	return (INIT_MODULE_SUCCESS);
  }
} /* Init TempCard */

/**********************************************************************************************************/
/* Init GPS receiver                                                                                   */
/**********************************************************************************************************/

int InitGPSReceiver(struct elekStatusType *ptrElekStatus, int IsMaster) {

   extern struct MessagePortType MessageInPortList[];
   extern struct MessagePortType MessageOutPortList[];
   extern int fdGPS;
   char debugbuf[GENERIC_BUF_LEN];

   int ret;

	if(IsMaster)
	{
	
		// init NMEA parser statemachine etc.
		NMEAParserInit();
	
		// open TTY
		fdGPS = serial_open(port , baud);
	
		if(fdGPS == 1)
		{
			sprintf(debugbuf,"ElekIOServ(M) : Error opening %s !\n\r", port);
			SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],debugbuf);
	
			return (INIT_MODULE_FAILED);
		};
	
		// success
		sprintf(debugbuf,"ElekIOServ(M) : Opened %s with %d BAUD!\n\r", port, baud);
		SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],debugbuf);
	
		// set data to initial values
	
		ptrElekStatus->GPSDataMaster.ucUTCHours   = 0;           // Time -> 00:00:00
		ptrElekStatus->GPSDataMaster.ucUTCMins    = 0;
		ptrElekStatus->GPSDataMaster.ucUTCSeconds = 0;
	
		ptrElekStatus->GPSDataMaster.dLongitude   = 999.99;      // normal range -180 to +180
		ptrElekStatus->GPSDataMaster.dLatitude    = 99.99;       // normal range -90 to +90
	
		ptrElekStatus->GPSDataMaster.fAltitude    = -99999;      // normal range 0 to 18000 m
		ptrElekStatus->GPSDataMaster.fHDOP        = 999;         // normal range 0 to 100 ?
	
		ptrElekStatus->GPSDataMaster.ucNumberOfSatellites = 0;   // normal range 1-12
		ptrElekStatus->GPSDataMaster.ucLastValidData = 255;      // normal range 0 to 6
	
		ptrElekStatus->GPSDataMaster.uiGroundSpeed   = 65000;    // normal range 0 to 30000 cm/s
		ptrElekStatus->GPSDataMaster.uiHeading       = 9999;     // normal range 0 to 3599 (tenth degrees)
	
		ucPortOpened = 1;                                  // set global flag for timer service routine
	
		return (INIT_MODULE_SUCCESS);
	}
	else // SLAVE
	{
		// init NMEA parser statemachine etc.
		NMEAParserInit();
	
		// open TTY
		fdGPS = serial_open(port , baud);
	
		if(fdGPS == 1)
		{
			sprintf(debugbuf,"ElekIOServ(S) : Error opening %s !\n\r", port);
			SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],debugbuf);
	
			return (INIT_MODULE_FAILED);
		};
	
		// success
		sprintf(debugbuf,"ElekIOServ(S) : Opened %s with %d BAUD!\n\r", port, baud);
		SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],debugbuf);
	
		// set data to initial values
	
		ptrElekStatus->GPSDataSlave.ucUTCHours   = 0;           // Time -> 00:00:00
		ptrElekStatus->GPSDataSlave.ucUTCMins    = 0;
		ptrElekStatus->GPSDataSlave.ucUTCSeconds = 0;
	
		ptrElekStatus->GPSDataSlave.dLongitude   = 999.99;      // normal range -180 to +180
		ptrElekStatus->GPSDataSlave.dLatitude    = 99.99;       // normal range -90 to +90
	
		ptrElekStatus->GPSDataSlave.fAltitude    = -99999;      // normal range 0 to 18000 m
		ptrElekStatus->GPSDataSlave.fHDOP        = 999;         // normal range 0 to 100 ?
	
		ptrElekStatus->GPSDataSlave.ucNumberOfSatellites = 0;   // normal range 1-12
		ptrElekStatus->GPSDataSlave.ucLastValidData = 255;      // normal range 0 to 6
	
		ptrElekStatus->GPSDataSlave.uiGroundSpeed   = 65000;    // normal range 0 to 30000 cm/s
		ptrElekStatus->GPSDataSlave.uiHeading       = 9999;     // normal range 0 to 3599 (tenth degrees)
	
		ucPortOpened = 1;                                  // set global flag for timer service routine
	
		return (INIT_MODULE_SUCCESS);
	}
} /* Init TempCard */


/**********************************************************************************************************/
/* Init Modules                                                                                        */
/**********************************************************************************************************/

/* function to initialize all Modules */

void InitModules(struct elekStatusType *ptrElekStatus, int IsMaster) {
   
  extern struct MessagePortType MessageInPortList[];
  extern struct MessagePortType MessageOutPortList[];

  int       ret; 
  char      buf[GENERIC_BUF_LEN];
  
  

  // set instrment action at the beginning to NOP
  ptrElekStatus->InstrumentFlags.InstrumentAction=INSTRUMENT_ACTION_NOP;


  LoadModulesConfig(ptrElekStatus, IsMaster);
  
  if(IsMaster)
  {
	if (INIT_MODULE_SUCCESS == (ret=InitCounterCard(ptrElekStatus, IsMaster))) {
		SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],"ElekIOServ(M) : init CounterCard successfull");
	} else {
		SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],"ElekIOServ(M) : init CounterCard failed !!");
	}
	
	if (INIT_MODULE_SUCCESS == (ret=InitADCCard(ptrElekStatus, IsMaster))) {
		SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],"ElekIOServ(M) : init ADCCard successfull");
	} else {
		SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],"ElekIOServ(M) : init ADCCard failed !!");
	}
	
	if (INIT_MODULE_SUCCESS == (ret=InitMFCCard(ptrElekStatus, IsMaster))) {
		SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],"ElekIOServ(M) : init MFCCard successfull");
	} else {
		SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],"ElekIOServ(M) : init MFCCard failed !!");
	}
	
	if (INIT_MODULE_SUCCESS == (ret=InitEtalonCard(ptrElekStatus))) {
		SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],"ElekIOServ(M) : init EtalonCard successfull");
	} else {
		SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],"ElekIOServ(M) : init EtalonCard failed !!");
	}
	
	if (INIT_MODULE_SUCCESS == (ret=InitValveCard(ptrElekStatus, IsMaster))) {
		SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],"ElekIOServ(M) : init ValveCard successfull");
	} else {
		SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],"ElekIOServ(M) : init ValveCard failed !!");
	}
	
	if (INIT_MODULE_SUCCESS == (ret=InitDCDC4Card(ptrElekStatus))) {
		SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],"ElekIOServ(M) : init DCDC4Card successfull");
	} else {
		SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],"ElekIOServ(M) : init DCDC4Card failed !!");
	}
	
	if (INIT_MODULE_SUCCESS == (ret=InitTempCard(ptrElekStatus, IsMaster))) {
		SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],"ElekIOServ(M) : init TemperatureCard successfull");
	} else {
		SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],"ElekIOServ(M) : init TemperatureCard failed !!");
	}
	
	if (INIT_MODULE_SUCCESS == (ret=InitGPSReceiver(ptrElekStatus, IsMaster))) {
		SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],"ElekIOServ(M) : init GPS successfull");
	} else {
		SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],"ElekIOServ(M) : init GPS failed !!");
	}
  }
  else // SLAVE
  {
	if (INIT_MODULE_SUCCESS == (ret=InitCounterCard(ptrElekStatus, IsMaster))) {
		SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],"ElekIOServ(S) : init CounterCard successfull");
	} else {
		SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],"ElekIOServ(S) : init CounterCard failed !!");
	}
	
	if (INIT_MODULE_SUCCESS == (ret=InitADCCard(ptrElekStatus,IsMaster))) {
		SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],"ElekIOServ(S) : init ADCCard successfull");
	} else {
		SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],"ElekIOServ(S) : init ADCCard failed !!");
	}
	
	if (INIT_MODULE_SUCCESS == (ret=InitMFCCard(ptrElekStatus, IsMaster))) {
		SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],"ElekIOServ(S) : init MFCCard successfull");
	} else {
		SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],"ElekIOServ(S) : init MFCCard failed !!");
	}
	
	if (INIT_MODULE_SUCCESS == (ret=InitValveCard(ptrElekStatus, IsMaster))) {
		SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],"ElekIOServ(S) : init ValveCard successfull");
	} else {
		SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],"ElekIOServ(S) : init ValveCard failed !!");
	}
	
	if (INIT_MODULE_SUCCESS == (ret=InitTempCard(ptrElekStatus, IsMaster))) {
		SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],"ElekIOServ(S) : init TemperatureCard successfull");
	} else {
		SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],"ElekIOServ(S) : init TemperatureCard failed !!");
	}
	
	if (INIT_MODULE_SUCCESS == (ret=InitGPSReceiver(ptrElekStatus, IsMaster))) 
	{
		SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],"ElekIOServ(S) : init GPS successfull");
	} 
	else 
	{
		SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],"ElekIOServ(S) : init GPS failed !!");
   }
 };
} /* InitModules */

/**********************************************************************************************************/
/* Test Channel Mask                                                                                      */
/**********************************************************************************************************/

int TestChannelMask(uint16_t *Mask, int TimeSlot) {

  int Cell;
  int BitNo;
  int Bit;
   
  Cell=(int) (TimeSlot/16);
  BitNo=1<<(TimeSlot % 16);
  if (Cell<COUNTER_MASK_WIDTH) Bit=(BitNo==(Mask[Cell] & BitNo));
  else {
    printf("PROBLEM WITH MASK ADDING %d %d\n",Cell,BitNo);
    Bit=0;
  }
  return(Bit);

} /* TestChannelMask */
/**********************************************************************************************************/
/* Set Channel Mask                                                                                      */
/**********************************************************************************************************/

int SetChannelMask(uint16_t *Mask, int TimeSlot, int Value) {

  int Cell;
  int BitNo;
  int Bit;
   
  Cell=(int) (TimeSlot/16);
  BitNo=1<<(TimeSlot % 16);
  if (Cell<COUNTER_MASK_WIDTH) {
    if (Value) {
      Mask[Cell]=Mask[Cell] | BitNo;
    } else {
      Mask[Cell]=Mask[Cell] & (~BitNo);
    } /* if value */
  } else {
    printf("PROBLEM WITH MASK Setting %d %d %d %d\n",TimeSlot,Cell,BitNo,Value);
  }
  return(Bit);

} /* SetChannelMask */

/**********************************************************************************************************/
/* GetEtalonCardData                                                                                          */
/**********************************************************************************************************/

/* function to retrieve Statusinformation */
void GetEtalonCardData ( struct elekStatusType *ptrElekStatus ) {

  uint16_t value;

  // get Etalon Data
  ptrElekStatus->EtalonData.Set.PositionWord.Low=elkReadData(ELK_STEP_SETPOS);
  ptrElekStatus->EtalonData.Set.PositionWord.High=elkReadData(ELK_STEP_SETPOS+2);

  ptrElekStatus->EtalonData.Current.PositionWord.Low=elkReadData(ELK_STEP_GETPOS);
  ptrElekStatus->EtalonData.Current.PositionWord.High=elkReadData(ELK_STEP_GETPOS+2);

  ptrElekStatus->EtalonData.Encoder.PositionWord.Low=elkReadData(ELK_STEP_GETENCPOS);
  ptrElekStatus->EtalonData.Encoder.PositionWord.High=elkReadData(ELK_STEP_GETENCPOS+2);

  ptrElekStatus->EtalonData.Index.PositionWord.Low=elkReadData(ELK_STEP_GETINDPOS);
  ptrElekStatus->EtalonData.Index.PositionWord.High=elkReadData(ELK_STEP_GETINDPOS+2);

  ptrElekStatus->EtalonData.CurSpeed   =0x0f & elkReadData(ELK_STEP_GETSPD);

  value=elkReadData(ELK_STEP_SETSPD);
  ptrElekStatus->EtalonData.SetSpeed   =0x0f & value;
  ptrElekStatus->EtalonData.SetAccl    =(0xf0 & value)>>8;

 
  ptrElekStatus->EtalonData.Status=elkReadData(ELK_STEP_STATUS);

  //    sprintf(buf,"ElekIOServ Etalon: %d %d",ptrElekStatus->EtalonData.CurPosition,
  //                   ptrElekStatus->EtalonData.CurSpeed);
  //    SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],buf);


} /* GetEtalonCardData */

/**********************************************************************************************************/
/* GetADCCardData                                                                                          */
/**********************************************************************************************************/

/* function to retrieve Statusinformation */
void GetADCCardData ( struct elekStatusType *ptrElekStatus ) {

  int Card;
  int Channel;
  uint16_t  ADCData;
  unsigned  ADC_Address;


  // normal ADC Card
  for (Card=0; Card<MAX_ADC_CARD_LIFT; Card ++) {
    ADC_Address=ELK_ADC_BASE+Card*ELK_ADC_NUM_ADR;
    for (Channel=0;Channel<MAX_ADC_CHANNEL_PER_CARD; Channel++) {	    
      ADCData=elkReadData(ADC_Address+2*Channel);
      ptrElekStatus->ADCCardMaster[Card].ADCChannelData[Channel].ADCData=ADCData;
      ptrElekStatus->ADCCardMaster[Card].ADCChannelData[Channel].SumDat+=ADCData;
      ptrElekStatus->ADCCardMaster[Card].ADCChannelData[Channel].SumSqr+=ADCData*ADCData;	    
    } /* for Channel */
  } /* for Card */    

} /* GetADCCardData */

/**********************************************************************************************************/
/* GetMFCCardData                                                                                          */
/**********************************************************************************************************/

/* function to retrieve Statusinformation */
void GetMFCCardData ( struct elekStatusType *ptrElekStatus ) {

  int Card;
  int Channel;
  uint16_t  MFCFlow;
  unsigned  MFC_Address;
  unsigned  DAC_Address;


  // normal MFC Card
  for (Card=0; Card<MAX_MFC_CARD_LIFT; Card ++) {
    MFC_Address=ELK_MFC_BASE+Card*ELK_MFC_NUM_ADR;
    DAC_Address=ELK_DAC_BASE+Card*ELK_DAC_NUM_ADR;
    ptrElekStatus->MFCCardMaster[Card].NumSamples++;
    for (Channel=0;Channel<MAX_MFC_CHANNEL_PER_CARD; Channel++) {	    
      MFCFlow=elkReadData(MFC_Address+2*Channel);
      
      ptrElekStatus->MFCCardMaster[Card].MFCChannelData[Channel].Flow    = MFCFlow;	    
      ptrElekStatus->MFCCardMaster[Card].MFCChannelData[Channel].SumDat += MFCFlow;	    
      ptrElekStatus->MFCCardMaster[Card].MFCChannelData[Channel].SumSqr += MFCFlow*MFCFlow;
      
      ptrElekStatus->MFCCardMaster[Card].MFCChannelData[Channel].SetFlow =elkReadData(DAC_Address+2*Channel) ;	    

    } /* for Channel */
  } /* for Card */    

} /* GetMFCCardData */



/**********************************************************************************************************/
/* Get DCDC4CardData                                                                                        */
/**********************************************************************************************************/

int GetDCDC4CardData (struct elekStatusType *ptrElekStatus) {
  
  int ret;
  int Channel;
  int Card;

  for (Card=0;Card<MAX_DCDC4_CARD_LIFT;Card++) {
    for (Channel=0;Channel<MAX_DCDC4_CHANNEL_PER_CARD; Channel++) {
      ptrElekStatus->DCDC4CardMaster[Card].Channel[Channel]=elkReadData(ELK_PWM_DCDC4_BASE+2*Channel);      
    } /* for Channel */
  } /* for Card */
} /* GetDCDC4CardData */



/**********************************************************************************************************/
/* Get ValveCard                                                                                        */
/**********************************************************************************************************/

int GetValveCardData (struct elekStatusType *ptrElekStatus) {
 
  int Card;
  int ret;

  for (Card=0; Card<MAX_VALVE_CARD_LIFT; Card ++) {
    ptrElekStatus->ValveCardMaster[Card].Valve=elkReadData(ELK_VALVE_BASE+Card*2);
    ptrElekStatus->ValveCardMaster[Card].ValveVolt=elkReadData(ELK_PWM_VALVE_BASE+Card*2);
  } /* for Card */
} /* GetValveCardData */


/**********************************************************************************************************/
/* GetCounterCardData                                                                                          */
/**********************************************************************************************************/

void GetCounterCardData ( struct elekStatusType *ptrElekStatus ) {
  
  extern struct MessagePortType MessageInPortList[];
  extern struct MessagePortType MessageOutPortList[];
    
  int       i;
  int       Channel;
  int       TimeSlot;
  int       TimeSlotinPage;
  int       Page;
  int       SumCounts;
  uint16_t  CounterStatus; 
  uint16_t  ADCData; 
  long      TimeOut;
  int       ret;
  char buf[GENERIC_BUF_LEN];
  
  
  // Counter Card ADC Channel
  for (i=0; i<ADC_CHANNEL_COUNTER_CARD; i++) {
      ADCData=elkReadData(ELK_COUNTER_ADC+i*2);   
      // mask channel in upper 4 bits
      if ((ADCData>>12)!=i) {
	  sprintf(buf,"elekIOServ: ccADC tried to read channel %d, got channel %d",i,ADCData>>12 );
	  SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],buf);
	  ADCData=0xffff;
      }      
      ptrElekStatus->CounterCardMaster.ADCData[i]=ADCData;   
  } /* for i */
  
  // CounterCard ShiftDelays
  for(i=0; i<MAX_COUNTER_CHANNEL;i++)
      ptrElekStatus->CounterCardMaster.Channel[i].ShiftDelay=elkReadData(ELK_COUNTER_DELAY_SHIFT+2*i);
  
  //Master Delay
  ptrElekStatus->CounterCardMaster.MasterDelay=elkReadData(ELK_COUNTER_DELAY_SHIFT+6);
  
  // CounterCard GateDelays
  for(i=1; i<MAX_COUNTER_CHANNEL;i++) {// skip channel 0 which is the PMT and has no gate register 
    ptrElekStatus->CounterCardMaster.Channel[i].GateDelay=elkReadData(ELK_COUNTER_DELAY_GATE+4*(i-1));
    ptrElekStatus->CounterCardMaster.Channel[i].GateWidth=elkReadData(ELK_COUNTER_DELAY_GATE+4*(i-1)+2);	
  }
  
  TimeOut=0;
  do {
    CounterStatus=elkReadData(ELK_COUNTER_STATUS); // lets see if swap was successfull
    TimeOut++;
    //    sprintf(buf,"CCStat: %4x",CounterStatus);
    //    SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],buf);
  } while ( (TimeOut<0x050) && (CounterStatus & ELK_COUNTER_STATUS_BUSY));
  
  if (TimeOut>30) { // had to wait too long....	
    sprintf(buf,"CCStat: %4x %ld BUSY...WE HAVE A PROBLEM",CounterStatus, TimeOut );
    SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],buf);
  }
  
  for (Channel=0; Channel<MAX_COUNTER_CHANNEL; Channel++) {
    TimeSlot=0;
    SumCounts=0;
    for (Page=0; Page<MAX_COUNTER_PAGE; Page++) {	
      // set page and Channel in status register
      CounterStatus=(Channel <<4) | Page;
      elkWriteData(ELK_COUNTER_STATUS, CounterStatus);
      // get masked sumcounts 
      
      for(TimeSlotinPage=0; TimeSlotinPage<MAX_COUNTER_SLOTS_PER_PAGE; TimeSlotinPage++, TimeSlot++) {
	ptrElekStatus->CounterCardMaster.Channel[Channel].Data[TimeSlot]
	  =elkReadData(ELK_COUNTER_COUNTS+TimeSlotinPage*2);
	// clear buffer
	ret=elkWriteData(ELK_COUNTER_COUNTS+TimeSlotinPage*2,0);
	
	// sum counts on our own until it works correctly
	if (TestChannelMask(ptrElekStatus->CounterCardMaster.Channel[Channel].Mask,TimeSlot)) {
	  SumCounts=SumCounts+ptrElekStatus->CounterCardMaster.Channel[Channel].Data[TimeSlot];
	  //if (Channel==0) {
	  //  sprintf(buf,"Add #%d : %d %d",TimeSlot,SumCounts,ptrElekStatus->CounterCard.Channel[Channel].Data[TimeSlot]);
	  //  SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],buf);
	  //} /* if Channel */
	} /* if testChannelMask */
      }/* for TimeSlotinPage*/
    } /*Page*/
    // get sumcounts for the selected channel
    
    //	ptrElekStatus->CounterCard.Channel[Channel].Counts=elkReadData(ELK_COUNTER_SUMCOUNTS);
    ptrElekStatus->CounterCardMaster.Channel[Channel].Counts=SumCounts;
    ptrElekStatus->CounterCardMaster.Channel[Channel].Pulses=ptrElekStatus->CounterCardMaster.Channel[Channel].Data[0];
    //    if (Channel==0) {
    //sprintf(buf,"SumCounts #%d : %d",Channel,ptrElekStatus->CounterCard.Channel[Channel].Counts);
    //SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],buf);
    //} /* if Channel */
  }/*Channel*/
} /* GetCounterCardData */

/**********************************************************************************************************/
/* GetTemperatureCardData                                                                                     */
/**********************************************************************************************************/

void GetTemperatureCardData ( struct elekStatusType *ptrElekStatus ) {

  extern struct MessagePortType MessageInPortList[];
  extern struct MessagePortType MessageOutPortList[];

  int            Card;
  int            Sensor;
  long           TimeOut=0;
  uint16_t       ret;
  uint16_t       Control;
  char           buf[GENERIC_BUF_LEN];

  for (Card=0; Card<MAX_TEMP_SENSOR_CARD_LIFT; Card++) {

    // check if AVR busy
    do {
      ptrElekStatus->TempSensCardMaster[Card].Control.Word=(uint16_t)elkReadData(ELK_TEMP_CTRL);
    } while ( (TimeOut++<MAX_TEMP_TIMEOUT) && (ptrElekStatus->TempSensCardMaster[Card].Control.Field.Update));

    //    sprintf(buf,"GetTemp: Control : %x",ptrElekStatus->TempSensCard[Card].Control.Word);
    //    SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],buf);

    if (TimeOut>MAX_TEMP_TIMEOUT) {
      if ( ((ptrElekStatus->TempSensCardMaster[Card].NumMissed)++)>MAX_TEMP_MISSED_READING) {
	// mark Sensor Data as not valid and mark temperature as invalid
	for (Sensor=0; Sensor<MAX_TEMP_SENSOR; Sensor++) {
	  ptrElekStatus->TempSensCardMaster[Card].TempSensor[Sensor].Field.TempFrac=0x0;
	  ptrElekStatus->TempSensCardMaster[Card].TempSensor[Sensor].Field.TempMain=0x80;
	} /* for Sensor */
	sprintf(buf,"GetTemp: Problem with Card : %d Missed Reading %d",Card, ptrElekStatus->TempSensCardMaster[Card].NumMissed);
	SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],buf);
      } else {
	// mark Sensor Data as not valid
	for (Sensor=0; Sensor<MAX_TEMP_SENSOR; Sensor++)
	  ptrElekStatus->TempSensCardMaster[Card].TempSensor[Sensor].Field.bValid=0;
      }
    } else {
      // we set the busy flag to get exclusive access
      //    ptrElekStatus->TempSensCard[Card].Control.Field.Busy=1;
      // ret=elkWriteData(ELK_TEMP_CTRL, ptrElekStatus->TempSensCard[Card].Control.Word );

      ptrElekStatus->TempSensCardMaster[Card].NumMissed=0;              // reset Missed Reading Counter

      // first we read the number of Sensor we have
      ptrElekStatus->TempSensCardMaster[Card].NumSensor=elkReadData(ELK_TEMP_FOUND);
      ptrElekStatus->TempSensCardMaster[Card].NumErrCRC=elkReadData(ELK_TEMP_ERR_CRC);
      ptrElekStatus->TempSensCardMaster[Card].NumErrNoResponse=elkReadData(ELK_TEMP_ERR_NORESPONSE);

      // now check each sensor
      for (Sensor=0; Sensor<MAX_TEMP_SENSOR; Sensor++) {
	ptrElekStatus->TempSensCardMaster[Card].TempSensor[Sensor].Word.WordTemp=elkReadData((Sensor*10)+ELK_TEMP_BASE);
	//	ptrElekStatus->TempSensCard[Card].TempSensor[Sensor].Temperatur=
	//  ptrElekStatus->TempSensCard[Card].TempSensor[Sensor].TempSens.TempMain+
	//  ptrElekStatus->TempSensCard[Card].TempSensor[Sensor].TempSens.TempFrac/16;
	ptrElekStatus->TempSensCardMaster[Card].TempSensor[Sensor].Word.WordID[0]=elkReadData((Sensor*10)+2+ELK_TEMP_BASE);
	ptrElekStatus->TempSensCardMaster[Card].TempSensor[Sensor].Word.WordID[1]=elkReadData((Sensor*10)+4+ELK_TEMP_BASE);
	ptrElekStatus->TempSensCardMaster[Card].TempSensor[Sensor].Word.WordID[2]=elkReadData((Sensor*10)+6+ELK_TEMP_BASE);
	ptrElekStatus->TempSensCardMaster[Card].TempSensor[Sensor].Word.WordLimit=elkReadData((Sensor*10)+8+ELK_TEMP_BASE);
      } /* for Sensor */


      //      sprintf(buf,"...GetTemp: Timeout %d Sensor0 %x",TimeOut,ptrElekStatus->TempSensCard[0].TempSensor[0].Word.WordTemp );
      // SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],buf);

      // we release the busy flag
      ptrElekStatus->TempSensCardMaster[Card].Control.Field.Busy=0;
      ret=elkWriteData(ELK_TEMP_CTRL, ptrElekStatus->TempSensCardMaster[Card].Control.Word );

    } /* if TimeOut */
  } /* for Card */


} /* GetTemperatureCardData */

/**********************************************************************************************************/
/* GetGPSData                                                                                     */
/**********************************************************************************************************/

void GetGPSData ( struct elekStatusType *ptrElekStatus ) {

  extern struct MessagePortType MessageInPortList[];
  extern struct MessagePortType MessageOutPortList[];

  uint16_t       ret;
  uint16_t       Control;
  char           buf[GENERIC_BUF_LEN];

  if(1 == ucDataReadyFlag)
  {
      if(ucGGAGPSQuality > 0) // check if data valid
      {
         // copy values into structure

         ptrElekStatus->GPSDataMaster.ucUTCHours   = ucGGAHour;      // Time UTC
         ptrElekStatus->GPSDataMaster.ucUTCMins    = ucGGAMinute;
         ptrElekStatus->GPSDataMaster.ucUTCSeconds = ucGGASecond;

         ptrElekStatus->GPSDataMaster.dLongitude   = dGGALongitude;  // Longitude (Laengengrad)
         ptrElekStatus->GPSDataMaster.dLatitude    = dGGALatitude;   // Latitude (Breitengrad)

         ptrElekStatus->GPSDataMaster.fAltitude    = dGGAAltitude;   // normal range 0 to 18000 m
         ptrElekStatus->GPSDataMaster.fHDOP        = dGGAHDOP;       // normal range 0 to 100 ?

         ptrElekStatus->GPSDataMaster.ucNumberOfSatellites = ucGGANumOfSatsInUse;   // normal range 1-12
         ptrElekStatus->GPSDataMaster.ucLastValidData = 0;           // normal range 0 to 6

         // speed

         if(dVTGSpeedInKmh > 0)                                // check for positive speed
         {
            dVTGSpeedInKmh = dVTGSpeedInKmh / 3.6;             // will give metres per second
            dVTGSpeedInKmh = dVTGSpeedInKmh * 100;             // will give cetimetres per second

            ptrElekStatus->GPSDataMaster.uiGroundSpeed   = (uint16_t)dVTGSpeedInKmh;
         }
         else
         {
            ptrElekStatus->GPSDataMaster.uiGroundSpeed   = 0;        // no speed, sets to 0
         };

         // heading
         // multiply heading by 10, will result in an int from 0 to 3599
         dVTGTrueHeading = dVTGTrueHeading * 10;
         ptrElekStatus->GPSDataMaster.uiHeading       = (uint16_t)dVTGTrueHeading;

         ucDataReadyFlag = FALSE;
      }
      else
      {
         // set data to initial values, which normaly cannot exist
/*
         ptrElekStatus->GPSData.ucUTCHours   = 0;           // Time -> 00:00:00
         ptrElekStatus->GPSData.ucUTCMins    = 0;
         ptrElekStatus->GPSData.ucUTCSeconds = 0;

         ptrElekStatus->GPSData.dLongitude   = 999.99;      // normal range -180 to +180
         ptrElekStatus->GPSData.dLatitude    = 99.99;       // normal range -90 to +90

         ptrElekStatus->GPSData.fAltitude    = -99999;      // normal range 0 to 18000 m
         ptrElekStatus->GPSData.fHDOP        = 999;         // normal range 0 to 100 ?

         ptrElekStatus->GPSData.ucLastValidData = 255;      // normal range 0 to 6

         ptrElekStatus->GPSData.uiGroundSpeed   = 65000;    // normal range 0 to 30000 cm/s
         ptrElekStatus->GPSData.uiHeading       = 9999;     // normal range 0 to 3599 (tenth degrees)
*/
         // no valid data, so increment counter up to 255

         if(ptrElekStatus->GPSDataMaster.ucLastValidData < 255)
         {
            ptrElekStatus->GPSDataMaster.ucLastValidData++;
            ptrElekStatus->GPSDataMaster.ucNumberOfSatellites = ucGGANumOfSatsInUse;   // copy number of sats to see where problem is
         }
      };
   };
} /* GetGPSData */

/**********************************************************************************************************/
/* GetElekStatus                                                                                          */
/**********************************************************************************************************/

/* function to retrieve Statusinformation */
void GetElekStatus ( struct elekStatusType *ptrElekStatus ) {

  // get time
  gettimeofday(&(ptrElekStatus->TimeOfDayMaster), NULL);

  GetEtalonCardData(ptrElekStatus);

  // Counter Card
    
  elkWriteData(ELK_COUNTER_STATUS, ELK_COUNTER_STATUS_FLIP );                  // ask for flip Buffer
  elkWriteData(ELK_COUNTER_STATUS, 0);                                         // and reset flip flop
  // ..will take some time, so we have time for something different....
  // now lets see what the counter card has...
  GetCounterCardData(ptrElekStatus);
    
  // ADC Card
  GetADCCardData(ptrElekStatus);
    
  // MFC Card
  GetMFCCardData(ptrElekStatus);

  // DCDC4 Card
  GetDCDC4CardData(ptrElekStatus);

  // Valve Card
  GetValveCardData(ptrElekStatus);
    
  // now get the temperature data
  GetTemperatureCardData(ptrElekStatus);

  // now get the GPS date
  GetGPSData(ptrElekStatus);

} /* GetElekStatus */

/**********************************************************************************************************/
/* Init UDP Ports                                                                                         */
/**********************************************************************************************************/

int InitUDPPorts(fd_set *pFDsMaster, int *fdMax) {

  extern struct MessagePortType MessageInPortList[];
  extern struct MessagePortType MessageOutPortList[];
  int    MessagePort;
    
  // init inports
  for (MessagePort=0; MessagePort<MAX_MESSAGE_INPORTS;MessagePort++) {
	
    printf("opening IN Port %s on Port %d\n",
	   MessageInPortList[MessagePort].PortName,
	   MessageInPortList[MessagePort].PortNumber);
	
    MessageInPortList[MessagePort].fdSocket=InitUDPInSocket(MessageInPortList[MessagePort].PortNumber);
    
	
    FD_SET(MessageInPortList[MessagePort].fdSocket, pFDsMaster);     // add the manual port to the master set
    (*fdMax)=MessageInPortList[MessagePort].fdSocket;                   // the last one will give the max number
  } /* for MessageInPort */

    // init outports
  for (MessagePort=0; MessagePort<MAX_MESSAGE_OUTPORTS;MessagePort++) {
    printf("opening OUT Port %s on Port %d\n",
	   MessageOutPortList[MessagePort].PortName,MessageOutPortList[MessagePort].PortNumber);
    MessageOutPortList[MessagePort].fdSocket=InitUDPOutSocket(MessageOutPortList[MessagePort].PortNumber);
	
  } /* for MessageOutPort */

  return(1);


} /*InitUDPPorts*/

/**********************************************************************************************************/
/* Change Priority                                                                                        */
/**********************************************************************************************************/
                                                                                    
int ChangePriority() {
  
  struct sched_param param;
  int scheduler;
  int prio;
  struct timespec tp;
  int ret;
  int min,max;
  
  sched_getparam(0,&param);
  printf("sched prio  %d\n",param.sched_priority);  
  
  printf("min max prio %d %d\n",sched_get_priority_min(SCHED_RR),sched_get_priority_max(SCHED_RR));
  sched_rr_get_interval(0, &tp);
  printf("RR Quantum : %f\n",tp.tv_nsec*1e-9);
  

  max=sched_get_priority_max(SCHED_RR);
  min=sched_get_priority_max(SCHED_RR);
  param.sched_priority=max; // (int)((max-min)/2);
  if (-1==(ret=sched_setscheduler(0,SCHED_RR, &param))) {
    perror("kann scheduler nicht wechseln");
  }

  return (ret);
  
} /* ChangePriority */

/**********************************************************************************************************/
/* MAIN                                                                                                   */
/**********************************************************************************************************/


int main(int argc, char *argv[]) 
{
  extern int errno;
  extern int StatusFlag;
  
  int IsMaster = 1;
  
  extern struct MessagePortType MessageInPortList[];
  extern struct MessagePortType MessageOutPortList[];
    
  struct elekStatusType ElekStatus;
  struct elekStatusType ElekStatusFromSlave;

  int fdMax;                      // max fd for select
  int i;                          // loop counter
  int fdNum;                      // fd number in loop
  fd_set fdsMaster;               // master file descriptor list
  fd_set fdsSelect;               // temp file descriptor list for select()
  int ret;
  uint64_t TSC,TSCin;
  uint64_t MinTimeDiff=1e6;
  uint64_t MaxTimeDiff=0;
  struct timeval StartAction;
  struct timeval StopAction;
  struct timeval LastAction;
    
  struct sigaction  SignalAction;
  struct sigevent   SignalEvent;
  sigset_t          SignalMask;

  #ifdef RUNONARM
  struct itimerval  StatusTimer;
  #else
  struct itimerspec StatusTimer;
  #endif

  timer_t           StatusTimer_id;
  clock_t           clock = CLOCK_REALTIME;
  int               StatusInterval=STATUS_INTERVAL;

  #ifdef RUNONARM
  struct timeval select_timeout;         // timeout
  #else
  struct timespec pselect_timeout;
  #endif

  struct timespec RealTime;         // Real time clock 
  struct sockaddr_in my_addr;     // my address information
  struct sockaddr_in their_addr;  // connector's address information
  int    addr_len, numbytes;
  char   buf[GENERIC_BUF_LEN];
  bool   EndOfSession;    
  int    MessagePort;
  int    MessageNumber;
  struct ElekMessageType Message;
  
  int SlaveNum;
  int Task;
  int Channel;
  int MaskAddr;
  struct SyncFlagType SyncFlag;
  
  if (argc<2) 
    {
      printf("Usage :\t%s m/s\n", argv[0]);
      exit(EXIT_FAILURE);
    };
  
  // Check if we should run as Master or Slave elekIOServ
  if ((argv[1][0] == 'm') || (argv[1][0] == 'M'))
    IsMaster = 1;
  else
    IsMaster = 0;
  
  if (elkInit()) {                // grant IO access
    printf("Error: failed to grant IO access rights\n");
    exit(EXIT_FAILURE);
  }  
  
  // setup master fd
  FD_ZERO(&fdsMaster);              // clear the master and temp sets
  FD_ZERO(&fdsSelect);
  InitUDPPorts(&fdsMaster,&fdMax);                  // Setup UDP in and out Ports
  
  addr_len = sizeof(struct sockaddr);
  
  // output version info on debugMon and Console
  
#ifdef RUNONARM
  printf("This is elekIOServ Version %3.2f (CVS: $RCSfile: elekIOServ.c,v $ $Revision: 1.23 $) for ARM\n",VERSION);
  
  sprintf(buf,"This is elekIOServ Version %3.2f (CVS: $RCSfile: elekIOServ.c,v $ $Revision: 1.23 $) for ARM\n",VERSION);
#else
  printf("This is elekIOServ Version %3.2f (CVS: $RCSfile: elekIOServ.c,v $ $Revision: 1.23 $) for i386\n",VERSION);
  
  sprintf(buf,"This is elekIOServ Version %3.2f (CVS: $RCSfile: elekIOServ.c,v $ $Revision: 1.23 $) for i386\n",VERSION);
#endif
  SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],buf);
  
  // output the working mode in debug mon and console
  
  printf("working in %s-Mode\n", IsMaster ?"MASTER" : "SLAVE");  
  sprintf(buf,"working in %s-Mode\n", IsMaster ?"MASTER" : "SLAVE");
  SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],buf);
  
  /* init all modules */
  InitModules(&ElekStatus,IsMaster);
  
  /* set up signal haendler */
  
  sigfillset(&SignalAction.sa_mask);
  SignalAction.sa_flags = 0;
  SignalAction.sa_handler = signalstatus;
  sigaction(SIGNAL_STATUS, &SignalAction, NULL);
  
  /* Set up timer: */
  memset(&SignalEvent, 0, sizeof(SignalEvent));
  SignalEvent.sigev_notify = SIGEV_SIGNAL;
  SignalEvent.sigev_signo = SIGNAL_STATUS;
  SignalEvent.sigev_value.sival_int = 0;
  
#ifdef RUNONPC
  ret= timer_create(clock, &SignalEvent, &StatusTimer_id);
  if (ret < 0) {
    perror("timer_create");
    return EXIT_FAILURE;
  }
  
  /* Start timer: */
  StatusTimer.it_interval.tv_sec =   StatusInterval / 1000;
  StatusTimer.it_interval.tv_nsec = (StatusInterval % 1000) * 1000000;
  StatusTimer.it_value = StatusTimer.it_interval;
#endif
  
#ifdef RUNONARM
  /* Start timer: */
  StatusTimer.it_interval.tv_sec =   0; //StatusInterval / 1000;
  StatusTimer.it_interval.tv_usec = 190000;
  StatusTimer.it_value = StatusTimer.it_interval;
#endif
  
#ifdef RUNONPC
  ret = timer_settime(StatusTimer_id, 0, &StatusTimer, NULL);
  if (ret < 0) {
    perror("timer_settime");
    return EXIT_FAILURE;
  }
#endif
  
#ifdef RUNONARM
  ret = setitimer(ITIMER_REAL, (const struct itimerval*)(&StatusTimer), NULL);
  if (ret < 0) {
    perror("settimer");
    return EXIT_FAILURE;
  }
  
#endif
  
#ifdef RUNONPC
  // change scheduler and set priority
  if (-1==(ret=ChangePriority())) {
    SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],"ElekIOServ: cannot set Priority");
  } 
  
#endif
  
  sigemptyset(&SignalMask);
  //    sigsuspend(&SignalMask);
  
  gettimeofday(&LastAction, NULL);  
  EndOfSession=FALSE;
  while (!EndOfSession) {
    write(2,"Wait for data..\r",16);
    
    fdsSelect=fdsMaster;
    
#ifdef RUNONPC
    pselect_timeout.tv_sec= UDP_SERVER_TIMEOUT;
    pselect_timeout.tv_nsec=0;
#else
    select_timeout.tv_sec = UDP_SERVER_TIMEOUT;
    select_timeout.tv_usec = 0;
#endif
    
#ifdef RUNONPC
    ret=pselect(fdMax+1, &fdsSelect, NULL, NULL, &pselect_timeout, &SignalMask);             // wiat until incoming udp or Signal
#else
    ret=select(fdMax+1, &fdsSelect, NULL, NULL, &select_timeout);             // wiat until incoming udp or Signal
#endif
    
    gettimeofday(&StartAction, NULL);
    
    //	printf("ret %d StatusFlag %d\n",ret,StatusFlag);
    if (ret ==-1 ) { // select error
      
      if (errno==EINTR) {  // got interrupted by timer
	
	if ((StatusFlag % 100)==0) 
	  printf("get Status %6d..\r",StatusFlag);		
	write(2,"get Status.....\r",16);
	
	if (ElekStatus.InstrumentFlags.StatusQuery) {
	  
	  if (IsMaster) {

	    // get TSC for timing
#ifdef RUNONPC // but only if run on PC
	    rdtscll(TSC);
#endif
	    
	    // ask all slave units to gather their data
	    for(SlaveNum=0;SlaveNum < MAXSLAVES;SlaveNum++)
	    {
			if(SlaveList[SlaveNum].SlaveName) 
	      {
				Message.MsgType=MSG_TYPE_FETCH_DATA;
				Message.MsgTime=TSC;                                       // send TSC to slave
				Message.MsgID=-1;
				SendUDPDataToIP(&MessageOutPortList[ELEK_ELEKIO_SLAVE_OUT],
			      SlaveList[SlaveNum].SlaveIP,
			      sizeof(struct ElekMessageType), &Message);  
#ifdef DEBUG_SLAVECOM
		      sprintf(buf,"Send request for data to %s @%08x",SlaveList[SlaveNum].SlaveName,Message.MsgTime);
		      SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],buf);			       
#endif 
			} /* end if(Slavelist) */
		} // end for() 
	  } // If IsMaster

	  GetElekStatus(&ElekStatus);
	  SendUDPData(&MessageOutPortList[ELEK_STATUS_OUT],sizeof(struct elekStatusType), &ElekStatus);
	  
	  //		sprintf(buf,"ElekIOServ: send Status of Numbytes : %d\n",sizeof(struct elekStatusType));
	  //		SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],buf);
	} else {
	  SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],"ElekIOServ: not Querying Status !!");  
	} /* if Query */
	
	if (SyncFlag.MaskChange==TRUE){
	  SetCounterCardMask(&ElekStatus, IsMaster);
	} /* if Syncflag.MaskChange */
	
	// let the other tasks know that we are done with retrieving the status
	Task=0;
	Message.MsgType=MSG_TYPE_SIGNAL;
	Message.MsgID=-1;
	while (TasktoWakeList[Task].TaskConn>-1) {
	  
	  //		  sprintf(buf,"ElekIOServ: send wake Signal to Task %s %d",
	  //	  TasktoWakeList[Task].TaskName, TasktoWakeList[Task].TaskConn);
	  // SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],buf);
	  
	  // first we send the status information to the task
	  if (TasktoWakeList[Task].TaskWantStatusOnPort>-1) { // but only if he wants to
	    SendUDPData(&MessageOutPortList[TasktoWakeList[Task].TaskWantStatusOnPort],sizeof(struct elekStatusType), &ElekStatus);
	  }
	  SendUDPData(&MessageOutPortList[TasktoWakeList[Task].TaskConn],sizeof(struct ElekMessageType), &Message);  
	  Task++;
	} // while task
	
      } else {
	perror("select");
	SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],"ElekIOServ: Problem with select");
      } 
    } else if (ret>0) {
      //	    printf("woke up...");
      write(2,"incoming Call..\r",16);
      
      for (MessagePort=0; MessagePort<MAX_MESSAGE_INPORTS;MessagePort++) { 
	
	if (FD_ISSET(MessageInPortList[MessagePort].fdSocket,&fdsSelect)) {   // new msg on fdNum. socket ... 
	  //		    fdElekManual=MessagePortList[MessagePort].fdSocket;
	  //		    fdElekManual=MessagePortList[0].fdSocket;
	  switch (MessagePort) {
	    case ELEK_MANUAL_IN:       // port for incoming commands from  eCmd
	    case ELEK_ETALON_IN:       // port for incoming commands from  etalon
	    case ELEK_SCRIPT_IN:       // port for incoming commands from  scripting host (not yet existing, HH, Feb2005

	      if ((numbytes=recvfrom(MessageInPortList[MessagePort].fdSocket, 
				     &Message,sizeof(struct ElekMessageType)  , 0,
				     (struct sockaddr *)&their_addr, &addr_len)) == -1) {
		perror("recvfrom");
		SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],"ElekIOServ: Problem with recieve");
	      }
	      
	      switch (Message.MsgType) {
		
	      case MSG_TYPE_FETCH_DATA:
#ifdef DEBUG_SLAVECOM
		printf("ElekIOServ: FETCH_DATA received, TSC was: %08x", Message.MsgTime);
#endif
		ElekStatus.TimeStampCommand.TSCReceived.TSCValue = Message.MsgTime;
		GetElekStatus(&ElekStatus);
		
		Message.MsgType=MSG_TYPE_ACK;
		
		if (MessagePort!=ELEK_ETALON_IN) 
		  {
		    sprintf(buf,"ElekIOServ: FETCH_DATA from %4d Port %04x Value %d (%04x)",
			    MessageInPortList[MessagePort].PortNumber,
			    Message.Addr,Message.Value,Message.Value);
		    SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],buf);
		    
		    sprintf(buf,"%d",MessageInPortList[MessagePort].RevMessagePort);
		    SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],buf);			       
		  } /* if MessagePort */
		
		SendUDPData(&MessageOutPortList[MessageInPortList[MessagePort].RevMessagePort],
			    sizeof(struct ElekMessageType), &Message); // send acknowledge
		
		SendUDPData(&MessageOutPortList[ELEK_ELEKIO_STATUS_OUT],
			    sizeof(struct elekStatusType), &ElekStatus); // send data packet
		
		break;
		
	      case MSG_TYPE_READ_DATA:
		printf("elekIOServ: manual read from Address %04x\n", Message.Addr);
		Message.Value=elkReadData(Message.Addr);
		Message.MsgType=MSG_TYPE_ACK;
		
		if (MessagePort!=ELEK_ETALON_IN) {
		  sprintf(buf,"ElekIOServ: ReadCmd from %4d Port %04x Value %d (%04x)",
			  MessageInPortList[MessagePort].PortNumber,
			  Message.Addr,Message.Value,Message.Value);
		  SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],buf);
		  
		  sprintf(buf,"%d",MessageInPortList[MessagePort].RevMessagePort);
		  SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],buf);			       
		} /* if MessagePort */
		SendUDPData(&MessageOutPortList[MessageInPortList[MessagePort].RevMessagePort],
			    sizeof(struct ElekMessageType), &Message);
		break;
	      case MSG_TYPE_WRITE_DATA:
		if (MessagePort!=ELEK_ETALON_IN) {
		  sprintf(buf,"ElekIOServ: WriteCmd from %4d Port %04x Value %d (%04x)",
			  MessageInPortList[MessagePort].PortNumber,
			  Message.Addr,Message.Value,Message.Value);
		  SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],buf);
		} /* if MessagePort */
		
		printf("elekIOServ: manual write to Address %04x Value is: %04x\n", Message.Addr, Message.Value);
		
		Message.Status=elkWriteData(Message.Addr,Message.Value);
		Message.MsgType=MSG_TYPE_ACK;			    
		SendUDPData(&MessageOutPortList[MessageInPortList[MessagePort].RevMessagePort],
			    sizeof(struct ElekMessageType), &Message);
		break;
		
	      case MSG_TYPE_CHANGE_FLAG_STATUS_QUERY:
		
		if (Message.Value) ElekStatus.InstrumentFlags.StatusQuery=1;
		else ElekStatus.InstrumentFlags.StatusQuery=0;
		sprintf(buf,"elekIOServ: Set StatusQuery to %d %d",Message.Value,ElekStatus.InstrumentFlags.StatusQuery);
		SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],buf);
		Message.MsgType=MSG_TYPE_ACK;			    
		SendUDPData(&MessageOutPortList[MessageInPortList[MessagePort].RevMessagePort],
			    sizeof(struct ElekMessageType), &Message);
		
		break;
		
	      case MSG_TYPE_CHANGE_FLAG_STATUS_SAVE:
		if (Message.Value) ElekStatus.InstrumentFlags.StatusSave=1;
		else ElekStatus.InstrumentFlags.StatusSave=0;
		sprintf(buf,"elekIOServ: Set StatusSave to %d",ElekStatus.InstrumentFlags.StatusSave);
		SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],buf);
		Message.MsgType=MSG_TYPE_ACK;			    
		SendUDPData(&MessageOutPortList[MessageInPortList[MessagePort].RevMessagePort],
			    sizeof(struct ElekMessageType), &Message);
		
		break;
		
	      case MSG_TYPE_CHANGE_FLAG_INSTRUMENT_ACTION:
		ElekStatus.InstrumentFlags.InstrumentAction=Message.Value;
		sprintf(buf,"elekIOServ: Set InstrumentAction to %d",ElekStatus.InstrumentFlags.InstrumentAction);
		SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],buf);
		Message.MsgType=MSG_TYPE_ACK;			    
		SendUDPData(&MessageOutPortList[MessageInPortList[MessagePort].RevMessagePort],
			    sizeof(struct ElekMessageType), &Message);
		
		break;
		
	      case MSG_TYPE_CHANGE_FLAG_ETALON_ACTION:
		ElekStatus.InstrumentFlags.EtalonAction=Message.Value;
		if (MessagePort!=ELEK_ETALON_IN) {
		  sprintf(buf,"elekIOServ: Set EtalonAction to %d",ElekStatus.InstrumentFlags.EtalonAction);
		  SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],buf);
		} /* if MessagePort */
		Message.MsgType=MSG_TYPE_ACK;			    
		SendUDPData(&MessageOutPortList[MessageInPortList[MessagePort].RevMessagePort],
			    sizeof(struct ElekMessageType), &Message);
		
		break;
		
	      case MSG_TYPE_CHANGE_FLAG_SYSTEM_PARAMETER:
		switch (Message.Addr) {
		case SYS_PARAMETER_ETALON_ONLINE:
		  ElekStatus.EtalonData.Online.Position=Message.Value;
		  break;
		  
		case SYS_PARAMETER_ETALON_OFFLINE_LEFT:
		  ElekStatus.EtalonData.OfflineStepLeft=Message.Value;
		  break;
		  
		case SYS_PARAMETER_ETALON_OFFLINE_RIGHT:
		  ElekStatus.EtalonData.OfflineStepRight=Message.Value;
		  break;
		  
		case SYS_PARAMETER_ETALON_DITHER:
		  ElekStatus.EtalonData.DitherStepWidth=Message.Value;
		  break;
		  
		case SYS_PARAMETER_ETALON_SCAN_START:
		  ElekStatus.EtalonData.ScanStart.Position=Message.Value;
		  break;
		  
		case SYS_PARAMETER_ETALON_SCAN_STOP:
		  ElekStatus.EtalonData.ScanStop.Position=Message.Value;
		  break;
		  
		case SYS_PARAMETER_ETALON_SCAN_STEP:
		  ElekStatus.EtalonData.ScanStepWidth=Message.Value;
		  break;
		  
		default:
		  sprintf(buf,"elekIOServ: Unkown SysParameter %d",Message.Addr);
		  SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],buf);
		  break;
		} /* switch Message.Addr Sysparameter */
		
		if (MessagePort!=ELEK_ETALON_IN) {
		  sprintf(buf,"elekIOServ: Change %s to %d",strSysParameterDescription[Message.Addr],Message.Value);
		  SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],buf);
		} /* if MessagePort */
		Message.MsgType=MSG_TYPE_ACK;			    
		SendUDPData(&MessageOutPortList[MessageInPortList[MessagePort].RevMessagePort],
			    sizeof(struct ElekMessageType), &Message);
		
		break;
		
	      case MSG_TYPE_CHANGE_MASK:
		if ((Message.Addr<MAX_COUNTER_CHANNEL*10)){
		  Channel=(int)(Message.Addr/10);
		  MaskAddr=Message.Addr%10;
		  ElekStatus.CounterCardMaster.Channel[Channel].Mask[MaskAddr]=Message.Value;
		  sprintf(buf,"elekIOServ: Set Mask Addr %d of Channel %d to %x", MaskAddr, Channel, Message.Value);
		  SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],buf);
		} else { // we found that Message.Addr is larger than MAX_COUNTER_CHANNEL*10
		  sprintf(buf,"elekIOServ: Set Mask : Addr %d is larger than %d", Message.Addr, MAX_COUNTER_CHANNEL*10);
		  SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],buf);
		} /* if Message.Addr >=0 ... */
		
		Message.MsgType=MSG_TYPE_ACK;			    
		SendUDPData(&MessageOutPortList[MessageInPortList[MessagePort].RevMessagePort],
			    sizeof(struct ElekMessageType), &Message);
		
		SyncFlag.MaskChange=TRUE;
		
		break;
		
		
	      default:
		SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],"ElekIOServ: Unknown Message Type");
		break;
		
	      } /* switch MsgType */
	  case ELEK_STATUS_IN:       // port to receive status data from slaves
	    if ((numbytes=recvfrom(MessageInPortList[MessagePort].fdSocket, 
				   &ElekStatusFromSlave,sizeof(struct ElekMessageType) , MSG_WAITALL,
				   (struct sockaddr *)&their_addr, &addr_len)) == -1) {
	      perror("recvfrom");
	      SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],"elekIOServ : Problem with recieve from slave");
	    }
	    
	    break;
	  default:
	    SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],"ElekIOServ: Unknown Port Type");
	    break;
	    
	  } // switch MessagePort
#ifdef RUNONPC
	  rdtscll(TSC);
#endif
	  //		    printf("%lld got Message on Port %d from %s\n",TSC,inet_ntoa(their_addr.sin_addr));
	  //		    printf("packet is %d bytes long\n",numbytes);
	  TSCin=Message.MsgTime;
	  MaxTimeDiff= MaxTimeDiff<TSC-TSCin ? TSC-TSCin : MaxTimeDiff;
	  MinTimeDiff= MinTimeDiff>TSC-TSCin ? TSC-TSCin : MinTimeDiff;
	  
	  //		    printf("diff : %9lld Max: %9lld Min: %9lld\n",TSC-TSCin,MaxTimeDiff,MinTimeDiff);
	  //		    printf("%9lld\n",TSC-TSCin);
	  // check for end signature
	  EndOfSession=(bool)(strstr(buf,"ende")!=NULL);
	  //		    printf("found %u\n",EndOfSession);
	} /* if fd_isset */		
      } /* for MessagePort */
      
    } else { /* ret==0*/
      //	    printf("timeout...\n");
      write(2,"timeout........\r",16);
      SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],"ElekIOServ: TimeOut");	    
    }
    
#ifdef RUNONPC
    if (timer_getoverrun(StatusTimer_id)>0) {
      printf("OVERRUN");
      SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],"ElekIOServ: Overrun");
    } /* if overrun */
#endif
    
    gettimeofday(&StopAction, NULL);
    
#ifdef DEBUG_TIME_TASK    
    sprintf(buf,"ElekIOServ: %ld RT: %ld DT: %ld",StartAction.tv_usec/1000,
	    StopAction.tv_usec-StartAction.tv_usec, (StartAction.tv_usec-LastAction.tv_usec)/1000);
    SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],buf);
#endif
    
    LastAction=StartAction;
  } /* while */
  
#ifdef RUNONPC
  /* delete timer */
  timer_delete(StatusTimer_id);
#endif
  
  // close all in bound sockets
  for (MessagePort=0; MessagePort<MAX_MESSAGE_INPORTS;MessagePort++) {	
    close(MessageInPortList[MessagePort].fdSocket);
  } /*for MessagePort */ 
  
  // close all out bound sockets
  for (MessagePort=0; MessagePort<MAX_MESSAGE_OUTPORTS;MessagePort++) {	
    close(MessageOutPortList[MessagePort].fdSocket);
  } /*for MessagePort */
  
  if (elkExit()) {                // release IO access
    printf("Error: failed to release IO access rights\n");
    exit(EXIT_FAILURE);
  }
  
  exit(EXIT_SUCCESS);
}

