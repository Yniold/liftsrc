/*
 * $RCSfile: elekIOcalib.c,v $ last changed on $Date: 2006-08-30 15:56:30 $ by $Author: rudolf $
 *
 * $Log: elekIOcalib.c,v $
 * Revision 1.1  2006-08-30 15:56:30  rudolf
 * started writing new elekIOcalib based on elekIOserv
 *
 *
 */

#define VERSION 0.1
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
# include <asm/msr.h>
#endif

#include <signal.h>
#include <errno.h>
#include <sched.h>

#include "../include/elekGeneral.h"
#include "../include/elekIO.h"
#include "../include/elekIOPorts.h"
#include "elekIOcalib.h"
#include "serial.h"

#define STATUS_INTERVAL  100

#define DEBUGLEVEL 0

#define CPUCLOCK 500000000UL 	// CPU clock of Markus' Athlon XP

enum InPortListEnum
{
   // this list has to be coherent with MessageInPortList
   ELEK_MANUAL_IN,       // port for incoming commands from  eCmd
     ELEK_ETALON_IN,       // port for incoming commands from  etalon
     ELEK_SCRIPT_IN,       // port for incoming commands from  scripting host (not yet existing, HH, Feb2005
     ELEK_STATUS_IN,       // port to receive status data from slaves
     MAX_MESSAGE_INPORTS
};

enum OutPortListEnum
{
   // this list has to be coherent with MessageOutPortList
   ELEK_STATUS_OUT,                // port for outgoing messages to status
     ELEK_ELEKIO_STATUS_OUT,         // port for outgoing status to elekIO
     ELEK_ELEKIO_SLAVE_OUT,          // port for outgoing messages to slaves
     ELEK_MANUAL_OUT,                // port for outgoing messages to eCmd
     ELEK_ETALON_OUT,                // port for outgoing messages to etalon
     ELEK_ETALON_STATUS_OUT,         // port for outgoing messages to etalon status, so etalon is directly informed of the status
     ELEK_SCRIPT_OUT,                // port for outgoing messages to script
     ELEK_DEBUG_OUT,                 // port for outgoing messages to debug
     ELEK_ELEKIO_SLAVE_MASTER_OUT,   // port for outgoing data packets from slave to master
     MAX_MESSAGE_OUTPORTS
};

static struct MessagePortType MessageInPortList[MAX_MESSAGE_INPORTS]=
{
   // order in list defines sequence of polling
    /* Name   , PortNo                    , ReversePort  , IPAddr, fdSocket, MaxMsg, Direction */
     {"Manual"  , UDP_ELEK_MANUAL_INPORT        , ELEK_MANUAL_OUT, IP_LOCALHOST, -1, 1,  UDP_IN_PORT},
     {"Etalon"  , UDP_ELEK_ETALON_INPORT        , ELEK_ETALON_OUT, IP_LOCALHOST, -1, 10, UDP_IN_PORT},
     {"Script"  , UDP_ELEK_SCRIPT_INPORT        , ELEK_SCRIPT_OUT, IP_LOCALHOST, -1, 5,  UDP_IN_PORT},
     {"ElekIOIn", UDP_ELEK_SLAVE_DATA_INPORT    , -1             , IP_LOCALHOST, -1, 1,  UDP_IN_PORT} // status inport from elekIOServ
};

static struct MessagePortType MessageOutPortList[MAX_MESSAGE_OUTPORTS]=
{
   // order in list defines sequence of polling
    /* Name      ,PortNo                     , ReversePort        , IPAddr, fdSocket, MaxMsg, Direction */
     {"Status"      ,UDP_ELEK_STATUS_STATUS_OUTPORT, -1                    , IP_STATUS_CLIENT, -1, 0,  UDP_OUT_PORT},
     {"ElekIOStatus",UDP_ELEK_SLAVE_DATA_INPORT    , -1                    , IP_ELEKIO_MASTER, -1, 0,  UDP_OUT_PORT},
     {"ElekIOServer",UDP_ELEK_MANUAL_INPORT        , ELEK_ELEKIO_STATUS_OUT, IP_ELEK_SERVER  , -1, 0,  UDP_OUT_PORT},
     {"Manual"      ,UDP_ELEK_MANUAL_OUTPORT       , ELEK_MANUAL_IN        , IP_LOCALHOST    , -1, 0,  UDP_OUT_PORT},
     {"Etalon"      ,UDP_ELEK_ETALON_OUTPORT       , ELEK_ETALON_IN        , IP_ETALON_CLIENT, -1, 0,  UDP_OUT_PORT},
     {"EtalonStatus",UDP_ELEK_ETALON_STATUS_OUTPORT, -1                    , IP_STATUS_CLIENT, -1, 0,  UDP_OUT_PORT},
     {"Script"      ,UDP_ELEK_SCRIPT_OUTPORT       , ELEK_SCRIPT_IN        , IP_SCRIPT_CLIENT, -1, 0,  UDP_OUT_PORT},
     {"DebugPort"   ,UDP_ELEK_DEBUG_OUTPORT        , -1                    , IP_DEBUG_CLIENT , -1, 0,  UDP_OUT_PORT},
     {"ElekIOOut"   ,UDP_ELEK_SLAVE_DATA_INPORT    , -1                    , IP_ELEKIO_MASTER, -1, 0,  UDP_OUT_PORT}
};

static struct TaskListType TasktoWakeList[MAX_TASKS_TO_WAKE]=
{
   // order defines sequence of wake up after timer
    /* TaskName TaskConn TaskWantStatusOnPort */
     {"Etalon",     ELEK_ETALON_OUT,ELEK_ETALON_STATUS_OUT},    // Etalon Task needs Status info
     {"Script",     ELEK_SCRIPT_OUT,                    -1},
     {      "",                  -1,                    -1}
};

static struct SlaveListType SlaveList[MAXSLAVES]=
{
   // list of slave elekIO units like wingpod or cal box
    /* SlaveName  SlaveIP */
     {"ArmAxis",   IP_ARMAXIS},
     {NULL,NULL},
     {NULL,NULL}
};

static char *strSysParameterDescription[MAX_SYS_PARAMETER]=
{
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
   extern enum TimerSignalStateEnum TimerState;
   char buf[GENERIC_BUF_LEN];

   ++StatusFlag;
   TimerState=(TimerState+1) % TIMER_SIGNAL_STATE_MAX;

   if (TimerState==TIMER_SIGNAL_STATE_GATHER)
     {
        if(ucPortOpened) 	// check if main() has opened the port already
	  {
	  }
     }
   // if ucPortOpened
}
// If TimerState
}
// signalstatus
//
/**********************************************************************************************************/
/* Load Module Config                                                                                     */
/**********************************************************************************************************/

/* function to load the config settings for all modules */

void LoadModulesConfig(struct calibStatusType *ptrCalibStatus)
{

   extern struct MessagePortType MessageInPortList[];
   extern struct MessagePortType MessageOutPortList[];

   int       i;
   int       Channel;
   int       Card;
   char      buf[GENERIC_BUF_LEN];

   // ADC Channels
   for (Card=0; Card<MAX_ADC_CARD_CALIB; Card ++)
     {
	ptrCalibStatus->ADCCardCalib[Card].NumSamples=0;

	for (Channel=0;Channel<MAX_ADC_CHANNEL_PER_CARD; Channel++)
	  {
	     ptrCalibStatus->ADCCardCalib[Card].ADCChannelData[Channel].ADCData                         =0x00;
	     ptrCalibStatus->ADCCardCalib[Card].ADCChannelData[Channel].SumDat                          =0x00;
	     ptrCalibStatus->ADCCardCalib[Card].ADCChannelData[Channel].SumSqr                          =0x00;

	     ptrCalibStatus->ADCCardCalib[Card].ADCChannelConfig[Channel].ADCChannelConfigBit.Unused    =0x00;
	     ptrCalibStatus->ADCCardCalib[Card].ADCChannelConfig[Channel].ADCChannelConfigBit.Offset    =0x00;
	     ptrCalibStatus->ADCCardCalib[Card].ADCChannelConfig[Channel].ADCChannelConfigBit.Gain      =0x00;
	     ptrCalibStatus->ADCCardCalib[Card].ADCChannelConfig[Channel].ADCChannelConfigBit.Bridge    =0x00;
	     ptrCalibStatus->ADCCardCalib[Card].ADCChannelConfig[Channel].ADCChannelConfigBit.MuxChannel=Channel;
	  }
	/* for Channel */
     }
   /* for Card */

   // for bridge pressure sensors we need two adc channels, one for the signal, the other for the bridge power
   // those pairs are set here
   Card=0; Channel=0;
   ptrCalibStatus->ADCCardCalib[Card].ADCChannelConfig[Channel].ADCChannelConfigBit.Unused    =0x00;
   ptrCalibStatus->ADCCardCalib[Card].ADCChannelConfig[Channel].ADCChannelConfigBit.Offset    =0x01;
   ptrCalibStatus->ADCCardCalib[Card].ADCChannelConfig[Channel].ADCChannelConfigBit.Gain      =0x2;
   ptrCalibStatus->ADCCardCalib[Card].ADCChannelConfig[Channel].ADCChannelConfigBit.Bridge    =0x1;
   ptrCalibStatus->ADCCardCalib[Card].ADCChannelConfig[Channel].ADCChannelConfigBit.MuxChannel=Channel;

   Card=0; Channel=1;
   ptrCalibStatus->ADCCardCalib[Card].ADCChannelConfig[Channel].ADCChannelConfigBit.Unused    =0x00;
   ptrCalibStatus->ADCCardCalib[Card].ADCChannelConfig[Channel].ADCChannelConfigBit.Offset    =0x01;
   ptrCalibStatus->ADCCardCalib[Card].ADCChannelConfig[Channel].ADCChannelConfigBit.Gain      =0x2;
   ptrCalibStatus->ADCCardCalib[Card].ADCChannelConfig[Channel].ADCChannelConfigBit.Bridge    =0x1;
   ptrCalibStatus->ADCCardCalib[Card].ADCChannelConfig[Channel].ADCChannelConfigBit.MuxChannel=Channel;

   Card=0; Channel=6;
   ptrCalibStatus->ADCCardCalib[Card].ADCChannelConfig[Channel].ADCChannelConfigBit.Unused    =0x00;
   ptrCalibStatus->ADCCardCalib[Card].ADCChannelConfig[Channel].ADCChannelConfigBit.Offset    =0x01;
   ptrCalibStatus->ADCCardCalib[Card].ADCChannelConfig[Channel].ADCChannelConfigBit.Gain      =0x2;
   ptrCalibStatus->ADCCardCalib[Card].ADCChannelConfig[Channel].ADCChannelConfigBit.Bridge    =0x1;
   ptrCalibStatus->ADCCardCalib[Card].ADCChannelConfig[Channel].ADCChannelConfigBit.MuxChannel=Channel;

   Card=0; Channel=7;
   ptrCalibStatus->ADCCardCalib[Card].ADCChannelConfig[Channel].ADCChannelConfigBit.Unused    =0x00;
   ptrCalibStatus->ADCCardCalib[Card].ADCChannelConfig[Channel].ADCChannelConfigBit.Offset    =0x01;
   ptrCalibStatus->ADCCardCalib[Card].ADCChannelConfig[Channel].ADCChannelConfigBit.Gain      =0x2;
   ptrCalibStatus->ADCCardCalib[Card].ADCChannelConfig[Channel].ADCChannelConfigBit.Bridge    =0x1;
   ptrCalibStatus->ADCCardCalib[Card].ADCChannelConfig[Channel].ADCChannelConfigBit.MuxChannel=Channel;

   // MFC Channels
   for (Card=0; Card<MAX_MFC_CARD_CALIB; Card ++)
     {
	ptrCalibStatus->MFCCardCalib[Card].NumSamples=0;
	for (Channel=0;Channel<MAX_MFC_CHANNEL_PER_CARD; Channel++)
	  {
	     ptrCalibStatus->MFCCardCalib[Card].MFCChannelData[Channel].SetFlow                         =0x00;
	     ptrCalibStatus->MFCCardCalib[Card].MFCChannelData[Channel].Flow                            =0x00;
	     ptrCalibStatus->MFCCardCalib[Card].MFCChannelData[Channel].SumDat                          =0x00;
	     ptrCalibStatus->MFCCardCalib[Card].MFCChannelData[Channel].SumSqr                          =0x00;

	     ptrCalibStatus->MFCCardCalib[Card].MFCChannelConfig[Channel].MFCChannelConfigBit.Unused    =0x00;
	     ptrCalibStatus->MFCCardCalib[Card].MFCChannelConfig[Channel].MFCChannelConfigBit.Ch0       =0x00;   // all valve forcings off
	     ptrCalibStatus->MFCCardCalib[Card].MFCChannelConfig[Channel].MFCChannelConfigBit.Ch1       =0x00;
	     ptrCalibStatus->MFCCardCalib[Card].MFCChannelConfig[Channel].MFCChannelConfigBit.Ch2       =0x00;
	     ptrCalibStatus->MFCCardCalib[Card].MFCChannelConfig[Channel].MFCChannelConfigBit.Ch3       =0x00;
	     ptrCalibStatus->MFCCardCalib[Card].MFCChannelConfig[Channel].MFCChannelConfigBit.MuxChannel=Channel;
	  }
	/* for Channel */
     }
   /* for Card */

};

/**********************************************************************************************************/
/* Init ADCCardCalib                                                                                       */
/**********************************************************************************************************/

int InitADCCard (struct calibStatusType *ptrCalibStatus)
{

   int       Channel;
   int       Card;
   int       ret=1;
   unsigned  ADC_CfgAddress;

   // configure ADC Card (Master)
   //
   for (Card=0; Card<MAX_ADC_CARD_CALIB; Card ++)
     {
	ADC_CfgAddress=ELK_ADC_BASE+ELK_ADC_CONFIG+Card*ELK_ADC_NUM_ADR;
	for (Channel=0;Channel<MAX_ADC_CHANNEL_PER_CARD; Channel++)
	  {
	     ret=elkWriteData(ADC_CfgAddress+2*Channel,
			      ptrCalibStatus->ADCCardCalib[Card].ADCChannelConfig[Channel].ADCChannelConfig);
	     // proof read
	     if (elkReadData(ADC_CfgAddress+2*Channel)!=
		 ptrCalibStatus->ADCCardMaster[Card].ADCChannelConfig[Channel].ADCChannelConfig)
	       {
		  return (INIT_MODULE_FAILED); // mark ret=0;
	       }
	     /* if elkRead */

	  }
	/* for Channel */
     }
   /* for Card */

   return (INIT_MODULE_SUCCESS);

}
/* Init ADCCardMaster */

/**********************************************************************************************************/
/* Init MFCCard                                                                                       */
/**********************************************************************************************************/

int InitMFCCard (struct calibStatusType *ptrCalibStatus)
{

   int       Channel;
   int       Card;
   int       ret=1;
   unsigned  MFC_CfgAddress;
   unsigned  DAC_CfgAddress;

   for (Card=0; Card<MAX_MFC_CARD_LIFT; Card ++)
     {
	MFC_CfgAddress=ELK_MFC_BASE+ELK_MFC_CONFIG+Card*ELK_MFC_NUM_ADR;
	DAC_CfgAddress=ELK_DAC_BASE+Card*ELK_DAC_NUM_ADR;
	for (Channel=0;Channel<MAX_MFC_CHANNEL_PER_CARD; Channel++)
	  {
	     // init ADC part of each channel
	     ret=elkWriteData(MFC_CfgAddress+2*Channel,
			      ptrCalibStatus->MFCCardMaster[Card].MFCChannelConfig[Channel].MFCChannelConfig);
	     // proof read
	     if (elkReadData(MFC_CfgAddress+2*Channel)!=
		 ptrCalibStatus->MFCCardMaster[Card].MFCChannelConfig[Channel].MFCChannelConfig)
	       {
		  return (INIT_MODULE_FAILED); // mark ret=0;
	       }
	     /* if elkRead */

	     // init DAC part
	     ret=elkWriteData(DAC_CfgAddress+2*Channel,
			      ptrCalibStatus->MFCCardMaster[Card].MFCChannelData[Channel].SetFlow);

	  }
	/* for Channel */
     }
   /* for Card */

   // enable DACPWM
   ret=elkWriteData(ELK_DACPWM_BASE,0xff);

   return (INIT_MODULE_SUCCESS);
}
/* Init MFCCard */

/**********************************************************************************************************/
/* Init TemperatureCard                                                                                        */
/**********************************************************************************************************/

int InitTempCard (struct calibStatusType *ptrCalibStatus)
{
   int ret;
   int Card;
   // configure Temperature Card
   // reset Busy bit
   elkWriteData(ELK_TEMP_CTRL,0);

   ret=elkReadData(ELK_TEMP_CTRL);

   // check if we got the same value that we wrote
   if (ret!=0)
     {
	return (INIT_MODULE_FAILED);
     }

   for (Card=0; Card<MAX_TEMP_SENSOR_CARD_LIFT; Card++)
     {
	ptrCalibStatus->TempSensCardMaster[Card].NumMissed=0;              // reset Missed Reading Counter
     };

   return (INIT_MODULE_SUCCESS);
}
/* Init TempCard */

/**********************************************************************************************************/
/* Init Modules                                                                                        */
/**********************************************************************************************************/

/* function to initialize all Modules */

void InitModules(struct calibStatusType *ptrCalibStatus)
{

   extern struct MessagePortType MessageInPortList[];
   extern struct MessagePortType MessageOutPortList[];

   int       ret;
   char      buf[GENERIC_BUF_LEN];

   // set instrment action at the beginning to NOP
   ptrCalibStatus->InstrumentFlags.InstrumentAction=INSTRUMENT_ACTION_NOP;

   LoadModulesConfig(ptrCalibStatus);

   if (INIT_MODULE_SUCCESS == (ret=InitADCCard(ptrCalibStatus)))
     {
	SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],"ElekIOcalib : init ADCCard successfull");
     }
   else
     {
	SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],"ElekIOcalib : init ADCCard failed !!");
     }

   if (INIT_MODULE_SUCCESS == (ret=InitMFCCard(ptrCalibStatus)))
     {
	SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],"ElekIOcalib : init MFCCard successfull");
     }
   else
     {
	SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],"ElekIOcalib : init MFCCard failed !!");
     }

   if (INIT_MODULE_SUCCESS == (ret=InitTempCard(ptrCalibStatus)))
     {
	SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],"ElekIOcalib : init TemperatureCard successfull");
     }
   else
     {
	SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],"ElekIOcalib : init TemperatureCard failed !!");
     }
}
/* InitModules */

/**********************************************************************************************************/
/* GetADCCardData                                                                                          */
/**********************************************************************************************************/

/* function to retrieve Statusinformation */
void GetADCCardData ( struct calibStatusType *ptrCalibStatus)
{

   int Card;
   int Channel;
   uint16_t  ADCData;
   unsigned  ADC_Address;
   for (Card=0; Card<MAX_ADC_CARD_CALIB; Card ++)
     {
	ADC_Address=ELK_ADC_BASE+Card*ELK_ADC_NUM_ADR;
	for (Channel=0;Channel<MAX_ADC_CHANNEL_PER_CARD; Channel++)
	  {
	     ADCData=elkReadData(ADC_Address+2*Channel);
	     ptrCalibStatus->ADCCardCalib[Card].ADCChannelData[Channel].ADCData=ADCData;
	     ptrCalibStatus->ADCCardCalib[Card].ADCChannelData[Channel].SumDat+=ADCData;
	     ptrCalibStatus->ADCCardCalib[Card].ADCChannelData[Channel].SumSqr+=ADCData*ADCData;
	  }
	     /* for Channel */
     }
	/* for Card */
}
/* GetADCCardData */

/**********************************************************************************************************/
/* GetMFCCardData                                                                                          */
/**********************************************************************************************************/

/* function to retrieve Statusinformation */
void GetMFCCardData ( struct calibStatusType *ptrCalibStatus, int IsMaster )
{

   int Card;
   int Channel;
   uint16_t  MFCFlow;
   unsigned  MFC_Address;
   unsigned  DAC_Address;

   if(IsMaster)
     {
        // *********************
        // MASTER MFC Card
        // *********************
	//
        for (Card=0; Card<MAX_MFC_CARD_LIFT; Card ++)
	  {
	     MFC_Address=ELK_MFC_BASE+Card*ELK_MFC_NUM_ADR;
	     DAC_Address=ELK_DAC_BASE+Card*ELK_DAC_NUM_ADR;
	     ptrCalibStatus->MFCCardMaster[Card].NumSamples++;

	     for (Channel=0;Channel<MAX_MFC_CHANNEL_PER_CARD; Channel++)
	       {
		  MFCFlow=elkReadData(MFC_Address+2*Channel);

		  ptrCalibStatus->MFCCardMaster[Card].MFCChannelData[Channel].Flow    = MFCFlow;
		  ptrCalibStatus->MFCCardMaster[Card].MFCChannelData[Channel].SumDat += MFCFlow;
		  ptrCalibStatus->MFCCardMaster[Card].MFCChannelData[Channel].SumSqr += MFCFlow*MFCFlow;

		  ptrCalibStatus->MFCCardMaster[Card].MFCChannelData[Channel].SetFlow =elkReadData(DAC_Address+2*Channel) ;

	       }
	     /* for Channel */
	  }
	/* for Card */
     }
   else
     {
        // *********************
        // SLAVE MFC Card
        // *********************
	//
        for (Card=0; Card<MAX_MFC_CARD_WP; Card ++)
	  {
	     MFC_Address=ELK_MFC_BASE_WP+Card*ELK_MFC_NUM_ADR_WP;
	     DAC_Address=ELK_DAC_BASE_WP+Card*ELK_DAC_NUM_ADR_WP;
	     ptrCalibStatus->MFCCardSlave[Card].NumSamples++;

	     for (Channel=0;Channel<MAX_MFC_CHANNEL_PER_CARD; Channel++)
	       {
		  MFCFlow=elkReadData(MFC_Address+2*Channel);

		  ptrCalibStatus->MFCCardSlave[Card].MFCChannelData[Channel].Flow    = MFCFlow;
		  ptrCalibStatus->MFCCardSlave[Card].MFCChannelData[Channel].SumDat += MFCFlow;
		  ptrCalibStatus->MFCCardSlave[Card].MFCChannelData[Channel].SumSqr += MFCFlow*MFCFlow;

		  ptrCalibStatus->MFCCardSlave[Card].MFCChannelData[Channel].SetFlow =elkReadData(DAC_Address+2*Channel) ;

	       }
	     /* for Channel */
	  }
	/* for Card */
     }

}
/* GetMFCCardData */

/**********************************************************************************************************/
/* Get DCDC4CardData                                                                                        */
/**********************************************************************************************************/

int GetDCDC4CardData (struct calibStatusType *ptrCalibStatus)
{

   int ret;
   int Channel;
   int Card;

   for (Card=0;Card<MAX_DCDC4_CARD_LIFT;Card++)
     {
        for (Channel=0;Channel<MAX_DCDC4_CHANNEL_PER_CARD; Channel++)
	  {
	     ptrCalibStatus->DCDC4CardMaster[Card].Channel[Channel]=elkReadData(ELK_PWM_DCDC4_BASE+2*Channel);
	  }
	/* for Channel */
     }
   /* for Card */
}
/* GetDCDC4CardData */

/**********************************************************************************************************/
/* Get ValveCard                                                                                        */
/**********************************************************************************************************/

int GetValveCardData (struct calibStatusType *ptrCalibStatus, int IsMaster)
{

   int Card;
   int ret;

   if(IsMaster)
     { // MASTER
        for (Card=0; Card<MAX_VALVE_CARD_LIFT; Card ++)
	  {
	     ptrCalibStatus->ValveCardMaster[Card].Valve=elkReadData(ELK_VALVE_BASE+Card*2);
	     ptrCalibStatus->ValveCardMaster[Card].ValveVolt=elkReadData(ELK_PWM_VALVE_BASE+Card*2);
	  }
	/* for Card */
     }
   else
     {
        // SLAVE
        for (Card=0; Card<MAX_VALVE_CARD_WP; Card ++)
	  {
	     ptrCalibStatus->ValveCardSlave[Card].Valve=elkReadData(ELK_VALVE_BASE+Card*2);
	     ptrCalibStatus->ValveCardSlave[Card].ValveVolt=elkReadData(ELK_PWM_VALVE_BASE_WP+Card*2);
	  }
	/* for Card */
     };
}
/* GetValveCardData */

/**********************************************************************************************************/
/* GetCounterCardData                                                                                          */
/**********************************************************************************************************/

void GetCounterCardData ( struct calibStatusType *ptrCalibStatus, int IsMaster)
{

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

   if(IsMaster)
     {
        // Counter Card ADC Channel
        for (i=0; i<ADC_CHANNEL_COUNTER_CARD; i++)
	  {
	     ADCData=elkReadData(ELK_COUNTER_ADC+i*2);
	     // mask channel in upper 4 bits
	     if ((ADCData>>12)!=i)
	       {
		  sprintf(buf,"elekIOServ(M): ccADC tried to read channel %d, got channel %d",i,ADCData>>12 );
		  SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],buf);
		  ADCData=0xffff;
	       }
	     ptrCalibStatus->CounterCardMaster.ADCData[i]=ADCData;
	  }
	/* for i */

        // CounterCard ShiftDelays
        for(i=0; i<MAX_COUNTER_CHANNEL;i++)
	  ptrCalibStatus->CounterCardMaster.Channel[i].ShiftDelay=elkReadData(ELK_COUNTER_DELAY_SHIFT+2*i);

        //Master Delay
        ptrCalibStatus->CounterCardMaster.MasterDelay=elkReadData(ELK_COUNTER_DELAY_SHIFT+6);

        // CounterCard GateDelays
        for(i=1; i<MAX_COUNTER_CHANNEL;i++)
	  {
	     // skip channel 0 which is the PMT and has no gate register
	     ptrCalibStatus->CounterCardMaster.Channel[i].GateDelay=elkReadData(ELK_COUNTER_DELAY_GATE+4*(i-1));
	     ptrCalibStatus->CounterCardMaster.Channel[i].GateWidth=elkReadData(ELK_COUNTER_DELAY_GATE+4*(i-1)+2);
	  }

        TimeOut=0;
        do
	  {
	     CounterStatus=elkReadData(ELK_COUNTER_STATUS); // lets see if swap was successfull
	     TimeOut++;
	     //    sprintf(buf,"CCStat: %4x",CounterStatus);
	     //    SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],buf);
	  }
	while ( (TimeOut<0x050) && (CounterStatus & ELK_COUNTER_STATUS_BUSY));

        if (TimeOut>30)
	  {
	     // had to wait too long....
	     sprintf(buf,"CCStat(M): %4x %ld BUSY...WE HAVE A PROBLEM",CounterStatus, TimeOut );
	     SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],buf);
	  }

        for (Channel=0; Channel<MAX_COUNTER_CHANNEL; Channel++)
	  {
	     TimeSlot=0;
	     SumCounts=0;
	     for (Page=0; Page<MAX_COUNTER_PAGE; Page++)
	       {
		  // set page and Channel in status register
		  CounterStatus=(Channel <<4) | Page;
		  elkWriteData(ELK_COUNTER_STATUS, CounterStatus);
		  // get masked sumcounts
		  //
		  for(TimeSlotinPage=0; TimeSlotinPage<MAX_COUNTER_SLOTS_PER_PAGE; TimeSlotinPage++, TimeSlot++)
		    {
		       ptrCalibStatus->CounterCardMaster.Channel[Channel].Data[TimeSlot]
			 =elkReadData(ELK_COUNTER_COUNTS+TimeSlotinPage*2);
		       // clear buffer
		       ret=elkWriteData(ELK_COUNTER_COUNTS+TimeSlotinPage*2,0);

		       // sum counts on our own until it works correctly
		       if (TestChannelMask(ptrCalibStatus->CounterCardMaster.Channel[Channel].Mask,TimeSlot))
			 {
			    SumCounts=SumCounts+ptrCalibStatus->CounterCardMaster.Channel[Channel].Data[TimeSlot];
			    //if (Channel==0) {
			    //  sprintf(buf,"Add #%d : %d %d",TimeSlot,SumCounts,ptrCalibStatus->CounterCard.Channel[Channel].Data[TimeSlot]);
			    //  SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],buf);
			    //} /* if Channel */
			 }
		       /* if testChannelMask */
		    }
		  /* for TimeSlotinPage*/
	       }
	     /*Page*/
	     // get sumcounts for the selected channel
	     //
	     //	ptrCalibStatus->CounterCard.Channel[Channel].Counts=elkReadData(ELK_COUNTER_SUMCOUNTS);
	     ptrCalibStatus->CounterCardMaster.Channel[Channel].Counts=SumCounts;
	     ptrCalibStatus->CounterCardMaster.Channel[Channel].Pulses=ptrCalibStatus->CounterCardMaster.Channel[Channel].Data[0];
	     //    if (Channel==0) {
	     //sprintf(buf,"SumCounts #%d : %d",Channel,ptrCalibStatus->CounterCard.Channel[Channel].Counts);
	     //SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],buf);
	     //} /* if Channel */
	  }
	/*Channel*/
     }

   // *******************
   // * Slave CounterCard
   // *******************
   //
   else
     {
        // Counter Card ADC Channel
        for (i=0; i<ADC_CHANNEL_COUNTER_CARD; i++)
	  {
	     ADCData=elkReadData(ELK_COUNTER_ADC+i*2);
	     // mask channel in upper 4 bits
	     if ((ADCData>>12)!=i)
	       {
		  sprintf(buf,"elekIOServ(S): ccADC tried to read channel %d, got channel %d",i,ADCData>>12 );
		  SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],buf);
		  ADCData=0xffff;
	       }
	     ptrCalibStatus->CounterCardSlave.ADCData[i]=ADCData;
	  }
	/* for i */

        // CounterCard ShiftDelays
        for(i=0; i<MAX_COUNTER_CHANNEL;i++)
	  ptrCalibStatus->CounterCardSlave.Channel[i].ShiftDelay=elkReadData(ELK_COUNTER_DELAY_SHIFT+2*i);

        //Master Delay
        ptrCalibStatus->CounterCardSlave.MasterDelay=elkReadData(ELK_COUNTER_DELAY_SHIFT+6);

        // CounterCard GateDelays
        for(i=1; i<MAX_COUNTER_CHANNEL;i++)
	  {
	     // skip channel 0 which is the PMT and has no gate register
	     ptrCalibStatus->CounterCardSlave.Channel[i].GateDelay=elkReadData(ELK_COUNTER_DELAY_GATE+4*(i-1));
	     ptrCalibStatus->CounterCardSlave.Channel[i].GateWidth=elkReadData(ELK_COUNTER_DELAY_GATE+4*(i-1)+2);
	  }

        TimeOut=0;
        do
	  {
	     CounterStatus=elkReadData(ELK_COUNTER_STATUS); // lets see if swap was successfull
	     TimeOut++;
	     //    sprintf(buf,"CCStat: %4x",CounterStatus);
	     //    SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],buf);
	  }
	while ( (TimeOut<0x050) && (CounterStatus & ELK_COUNTER_STATUS_BUSY));

        if (TimeOut>30)
	  {
	     // had to wait too long....
	     sprintf(buf,"CCStat(S): %4x %ld BUSY...WE HAVE A PROBLEM",CounterStatus, TimeOut );
	     SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],buf);
	  }

        for (Channel=0; Channel<MAX_COUNTER_CHANNEL; Channel++)
	  {
	     TimeSlot=0;
	     SumCounts=0;
	     for (Page=0; Page<MAX_COUNTER_PAGE; Page++)
	       {
		  // set page and Channel in status register
		  CounterStatus=(Channel <<4) | Page;
		  elkWriteData(ELK_COUNTER_STATUS, CounterStatus);
		  // get masked sumcounts
		  //
		  for(TimeSlotinPage=0; TimeSlotinPage<MAX_COUNTER_SLOTS_PER_PAGE; TimeSlotinPage++, TimeSlot++)
		    {
		       ptrCalibStatus->CounterCardSlave.Channel[Channel].Data[TimeSlot]
			 =elkReadData(ELK_COUNTER_COUNTS+TimeSlotinPage*2);
		       // clear buffer
		       ret=elkWriteData(ELK_COUNTER_COUNTS+TimeSlotinPage*2,0);

		       // sum counts on our own until it works correctly
		       if (TestChannelMask(ptrCalibStatus->CounterCardSlave.Channel[Channel].Mask,TimeSlot))
			 {
			    SumCounts=SumCounts+ptrCalibStatus->CounterCardSlave.Channel[Channel].Data[TimeSlot];
			    //if (Channel==0) {
			    //  sprintf(buf,"Add #%d : %d %d",TimeSlot,SumCounts,ptrCalibStatus->CounterCard.Channel[Channel].Data[TimeSlot]);
			    //  SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],buf);
			    //} /* if Channel */
			 }
		       /* if testChannelMask */
		    }
		  /* for TimeSlotinPage*/
	       }
	     /*Page*/
	     // get sumcounts for the selected channel
	     //
	     //	ptrCalibStatus->CounterCard.Channel[Channel].Counts=elkReadData(ELK_COUNTER_SUMCOUNTS);
	     ptrCalibStatus->CounterCardSlave.Channel[Channel].Counts=SumCounts;
	     ptrCalibStatus->CounterCardSlave.Channel[Channel].Pulses=ptrCalibStatus->CounterCardSlave.Channel[Channel].Data[0];
	     //    if (Channel==0) {
	     //sprintf(buf,"SumCounts #%d : %d",Channel,ptrCalibStatus->CounterCard.Channel[Channel].Counts);
	     //SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],buf);
	     //} /* if Channel */
	  }
	/*Channel*/
     }

}
/* GetCounterCardData */

/**********************************************************************************************************/
/* GetTemperatureCardData                                                                                     */
/**********************************************************************************************************/

void GetTemperatureCardData ( struct calibStatusType *ptrCalibStatus, int IsMaster)
{

   extern struct MessagePortType MessageInPortList[];
   extern struct MessagePortType MessageOutPortList[];

   int            Card;
   int            TempBaseAddr;
   int            Sensor;
   long           TimeOut=0;
   uint16_t       ret;
   uint16_t       Control;
   char           buf[GENERIC_BUF_LEN];

   if(IsMaster)
     {
        for (Card=0; Card<MAX_TEMP_SENSOR_CARD_LIFT; Card++)
	  {
	     // get current control word status
	     ptrCalibStatus->TempSensCardMaster[Card].Control.Word=(uint16_t)elkReadData(ELK_TEMP_CTRL);
	     // we set the busy flag to get exclusive access
	     ptrCalibStatus->TempSensCardMaster[Card].Control.Field.Busy=1;
	     ret=elkWriteData(ELK_TEMP_CTRL, ptrCalibStatus->TempSensCardMaster[Card].Control.Word );

	     // check if AVR busy
	     do
	       {
		  ptrCalibStatus->TempSensCardMaster[Card].Control.Word=(uint16_t)elkReadData(ELK_TEMP_CTRL);
	       }
	     while ((TimeOut++<MAX_TEMP_TIMEOUT)&&(ptrCalibStatus->TempSensCardMaster[Card].Control.Field.Update));

	     if (TimeOut>MAX_TEMP_TIMEOUT)
	       {
		  if ( ((ptrCalibStatus->TempSensCardMaster[Card].NumMissed)++)>MAX_TEMP_MISSED_READING)
		    {
		       // mark Sensor Data as not valid and mark temperature as invalid
		       for (Sensor=0; Sensor<MAX_TEMP_SENSOR; Sensor++)
			 {
			    ptrCalibStatus->TempSensCardMaster[Card].TempSensor[Sensor].Field.TempFrac=0x0;
			    ptrCalibStatus->TempSensCardMaster[Card].TempSensor[Sensor].Field.TempMain=0x80;
			 }
		       /* for Sensor */
		       sprintf(buf,"GetTemp(M): Problem with Card : %d Missed Reading %d",Card, ptrCalibStatus->TempSensCardMaster[Card].NumMissed);
		       SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],buf);
		    }
		  else
		    {
		       // mark Sensor Data as not valid
		       for (Sensor=0; Sensor<MAX_TEMP_SENSOR; Sensor++)
			 ptrCalibStatus->TempSensCardMaster[Card].TempSensor[Sensor].Field.bValid=0;
		    }
	       }
	     else
	       {

		  ptrCalibStatus->TempSensCardMaster[Card].NumMissed=0;              // reset Missed Reading Counter

		  // first we read the number of Sensor we have
		  ptrCalibStatus->TempSensCardMaster[Card].NumSensor=elkReadData(ELK_TEMP_FOUND);
		  ptrCalibStatus->TempSensCardMaster[Card].NumErrCRC=elkReadData(ELK_TEMP_ERR_CRC);
		  ptrCalibStatus->TempSensCardMaster[Card].NumErrNoResponse=elkReadData(ELK_TEMP_ERR_NORESPONSE);
		  // printf("found %d sensors\n",ptrCalibStatus->TempSensCardMaster[Card].NumSensor);
		  // now check each sensor
		  // we can have more sensors, but for now we use only 30
		  // they are split in two groups, 24 & 6
		  TempBaseAddr=ELK_TEMP_DATA; // first range
		  for (Sensor=0; Sensor<MAX_TEMP_SENSOR; Sensor++)
		    {
		       if (Sensor==25) TempBaseAddr=(ELK_TEMP_DATA2-(Sensor*10));
		       ptrCalibStatus->TempSensCardMaster[Card].TempSensor[Sensor].Word.WordTemp=elkReadData((Sensor*10)+TempBaseAddr);
		       //	ptrCalibStatus->TempSensCard[Card].TempSensor[Sensor].Temperatur=
		       //  ptrCalibStatus->TempSensCard[Card].TempSensor[Sensor].TempSens.TempMain+
		       //  ptrCalibStatus->TempSensCard[Card].TempSensor[Sensor].TempSens.TempFrac/16;
		       ptrCalibStatus->TempSensCardMaster[Card].TempSensor[Sensor].Word.WordID[0]=elkReadData((Sensor*10)+2+TempBaseAddr);
		       ptrCalibStatus->TempSensCardMaster[Card].TempSensor[Sensor].Word.WordID[1]=elkReadData((Sensor*10)+4+TempBaseAddr);
		       ptrCalibStatus->TempSensCardMaster[Card].TempSensor[Sensor].Word.WordID[2]=elkReadData((Sensor*10)+6+TempBaseAddr);
		       ptrCalibStatus->TempSensCardMaster[Card].TempSensor[Sensor].Word.WordLimit=elkReadData((Sensor*10)+8+TempBaseAddr);
		    }
		  /* for Sensor */

		  //      sprintf(buf,"...GetTemp: Timeout %d Sensor0 %x",TimeOut,ptrCalibStatus->TempSensCard[0].TempSensor[0].Word.WordTemp );
		  // SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],buf);
		  //
		  // we release the busy flag
		  ptrCalibStatus->TempSensCardMaster[Card].Control.Field.Busy=0;
		  ret=elkWriteData(ELK_TEMP_CTRL, ptrCalibStatus->TempSensCardMaster[Card].Control.Word );

	       }
	     /* if TimeOut */
	  }
	/* for Card */
     }
   //**************************
   // SLAVE Temperature Card
   //**************************
   else
     {
	for (Card=0; Card<MAX_TEMP_SENSOR_CARD_WP; Card++)
	  {

	     // we want exclusive access, get control word, mark busy and write back
	     ptrCalibStatus->TempSensCardSlave[Card].Control.Word=(uint16_t)elkReadData(ELK_TEMP_CTRL);
	     ptrCalibStatus->TempSensCardSlave[Card].Control.Field.Busy=1;
	     ret=elkWriteData(ELK_TEMP_CTRL, ptrCalibStatus->TempSensCardSlave[Card].Control.Word );

	     // check if AVR busy
	     do
	       {
		  ptrCalibStatus->TempSensCardSlave[Card].Control.Word=(uint16_t)elkReadData(ELK_TEMP_CTRL);
	       }
	     while ((TimeOut++<MAX_TEMP_TIMEOUT)&&(ptrCalibStatus->TempSensCardSlave[Card].Control.Field.Update));

	     if (TimeOut>MAX_TEMP_TIMEOUT)
	       {
		  if ( ((ptrCalibStatus->TempSensCardSlave[Card].NumMissed)++)>MAX_TEMP_MISSED_READING)
		    {
		       // mark Sensor Data as not valid and mark temperature as invalid
		       for (Sensor=0; Sensor<MAX_TEMP_SENSOR; Sensor++)
			 {
			    ptrCalibStatus->TempSensCardSlave[Card].TempSensor[Sensor].Field.TempFrac=0x0;
			    ptrCalibStatus->TempSensCardSlave[Card].TempSensor[Sensor].Field.TempMain=0x80;
			 }
		       /* for Sensor */
		       sprintf(buf,"GetTemp(S): Problem with Card : %d Missed Reading %d",Card, ptrCalibStatus->TempSensCardMaster[Card].NumMissed);
		       SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],buf);
		    }
		  else
		    {
		       // mark Sensor Data as not valid
		       for (Sensor=0; Sensor<MAX_TEMP_SENSOR; Sensor++)
			 ptrCalibStatus->TempSensCardSlave[Card].TempSensor[Sensor].Field.bValid=0;
		    }
	       }
	     else
	       {
		  // we set the busy flag to get exclusive access
		  //    ptrCalibStatus->TempSensCard[Card].Control.Field.Busy=1;
		  // ret=elkWriteData(ELK_TEMP_CTRL, ptrCalibStatus->TempSensCard[Card].Control.Word );
		  //
		  ptrCalibStatus->TempSensCardSlave[Card].NumMissed=0;              // reset Missed Reading Counter

		  // first we read the number of Sensor we have
		  ptrCalibStatus->TempSensCardSlave[Card].NumSensor=elkReadData(ELK_TEMP_FOUND);
		  ptrCalibStatus->TempSensCardSlave[Card].NumErrCRC=elkReadData(ELK_TEMP_ERR_CRC);
		  ptrCalibStatus->TempSensCardSlave[Card].NumErrNoResponse=elkReadData(ELK_TEMP_ERR_NORESPONSE);

		  // now check each sensor
		  //
		  //
		  // we can have more sensors, but for now we use only 30
		  // they are split in two groups, 24 & 6
		  TempBaseAddr=ELK_TEMP_DATA; // first range
		  for (Sensor=0; Sensor<MAX_TEMP_SENSOR; Sensor++)
		    {
		       if (Sensor==25) TempBaseAddr=(ELK_TEMP_DATA2-(Sensor*10));
		       ptrCalibStatus->TempSensCardSlave[Card].TempSensor[Sensor].Word.WordTemp=elkReadData((Sensor*10)+TempBaseAddr);
		       //	ptrCalibStatus->TempSensCard[Card].TempSensor[Sensor].Temperatur=
		       //  ptrCalibStatus->TempSensCard[Card].TempSensor[Sensor].TempSens.TempMain+
		       //  ptrCalibStatus->TempSensCard[Card].TempSensor[Sensor].TempSens.TempFrac/16;
		       ptrCalibStatus->TempSensCardSlave[Card].TempSensor[Sensor].Word.WordID[0]=elkReadData((Sensor*10)+2+TempBaseAddr);
		       ptrCalibStatus->TempSensCardSlave[Card].TempSensor[Sensor].Word.WordID[1]=elkReadData((Sensor*10)+4+TempBaseAddr);
		       ptrCalibStatus->TempSensCardSlave[Card].TempSensor[Sensor].Word.WordID[2]=elkReadData((Sensor*10)+6+TempBaseAddr);
		       ptrCalibStatus->TempSensCardSlave[Card].TempSensor[Sensor].Word.WordLimit=elkReadData((Sensor*10)+8+TempBaseAddr);
		    }
		  /* for Sensor */

		  // for (Sensor=0; Sensor<MAX_TEMP_SENSOR; Sensor++)
		  //{
		  //    ptrCalibStatus->TempSensCardSlave[Card].TempSensor[Sensor].Word.WordTemp=elkReadData((Sensor*10)+ELK_TEMP_BASE);
		  //    //	ptrCalibStatus->TempSensCard[Card].TempSensor[Sensor].Temperatur=
		  //    //  ptrCalibStatus->TempSensCard[Card].TempSensor[Sensor].TempSens.TempMain+
		  //    //  ptrCalibStatus->TempSensCard[Card].TempSensor[Sensor].TempSens.TempFrac/16;
		  //    ptrCalibStatus->TempSensCardSlave[Card].TempSensor[Sensor].Word.WordID[0]=elkReadData((Sensor*10)+2+ELK_TEMP_BASE);
		  //    ptrCalibStatus->TempSensCardSlave[Card].TempSensor[Sensor].Word.WordID[1]=elkReadData((Sensor*10)+4+ELK_TEMP_BASE);
		  //    ptrCalibStatus->TempSensCardSlave[Card].TempSensor[Sensor].Word.WordID[2]=elkReadData((Sensor*10)+6+ELK_TEMP_BASE);
		  //    ptrCalibStatus->TempSensCardSlave[Card].TempSensor[Sensor].Word.WordLimit=elkReadData((Sensor*10)+8+ELK_TEMP_BASE);
		  // } /* for Sensor */
		  //
		  //
		  //      sprintf(buf,"...GetTemp: Timeout %d Sensor0 %x",TimeOut,ptrCalibStatus->TempSensCard[0].TempSensor[0].Word.WordTemp );
		  // SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],buf);
		  //
		  // we release the busy flag
		  ptrCalibStatus->TempSensCardSlave[Card].Control.Field.Busy=0;
		  ret=elkWriteData(ELK_TEMP_CTRL, ptrCalibStatus->TempSensCardSlave[Card].Control.Word );

	       }
	     /* if TimeOut */
	  }
	/* for Card */
     }
   /* else if(IsMaster)*/

}
/* GetTemperatureCardData */

/**********************************************************************************************************/
/* GetGPSData                                                                                     */
/**********************************************************************************************************/

void GetGPSData ( struct calibStatusType *ptrCalibStatus, int IsMaster)
{

   extern struct MessagePortType MessageInPortList[];
   extern struct MessagePortType MessageOutPortList[];

   uint16_t       ret;
   uint16_t       Control;
   char           buf[GENERIC_BUF_LEN];

   if(IsMaster)
     {
        if(1 == ucDataReadyFlag)
	  {
	     if(ucGGAGPSQuality > 0) // check if data valid
	       {
		  // copy values into structure
		  //
		  ptrCalibStatus->GPSDataMaster.ucUTCHours   = ucGGAHour;      // Time UTC
		  ptrCalibStatus->GPSDataMaster.ucUTCMins    = ucGGAMinute;
		  ptrCalibStatus->GPSDataMaster.ucUTCSeconds = ucGGASecond;

		  ptrCalibStatus->GPSDataMaster.dLongitude   = dGGALongitude;  // Longitude (Laengengrad)
		  ptrCalibStatus->GPSDataMaster.dLatitude    = dGGALatitude;   // Latitude (Breitengrad)

		  ptrCalibStatus->GPSDataMaster.fAltitude    = dGGAAltitude;   // normal range 0 to 18000 m
		  ptrCalibStatus->GPSDataMaster.fHDOP        = dGGAHDOP;       // normal range 0 to 100 ?

		  ptrCalibStatus->GPSDataMaster.ucNumberOfSatellites = ucGGANumOfSatsInUse;   // normal range 1-12
		  ptrCalibStatus->GPSDataMaster.ucLastValidData = 0;           // normal range 0 to 6

		  // speed
		  //
		  if(dVTGSpeedInKmh > 0)                                // check for positive speed
		    {
		       dVTGSpeedInKmh = dVTGSpeedInKmh / 3.6;             // will give metres per second
		       dVTGSpeedInKmh = dVTGSpeedInKmh * 100;             // will give cetimetres per second

		       ptrCalibStatus->GPSDataMaster.uiGroundSpeed   = (uint16_t)dVTGSpeedInKmh;
		    }
		  else
		    {
		       ptrCalibStatus->GPSDataMaster.uiGroundSpeed   = 0;        // no speed, sets to 0
		    };

		  // heading
		  // multiply heading by 10, will result in an int from 0 to 3599
		  dVTGTrueHeading = dVTGTrueHeading * 10;
		  ptrCalibStatus->GPSDataMaster.uiHeading       = (uint16_t)dVTGTrueHeading;

		  ucDataReadyFlag = FALSE;
	       }
	     else
	       {
		  // set data to initial values, which normaly cannot exist
                /*
                  ptrCalibStatus->GPSData.ucUTCHours   = 0;           // Time -> 00:00:00
                  ptrCalibStatus->GPSData.ucUTCMins    = 0;
                  ptrCalibStatus->GPSData.ucUTCSeconds = 0;

                  ptrCalibStatus->GPSData.dLongitude   = 999.99;      // normal range -180 to +180
                  ptrCalibStatus->GPSData.dLatitude    = 99.99;       // normal range -90 to +90

                  ptrCalibStatus->GPSData.fAltitude    = -99999;      // normal range 0 to 18000 m
                  ptrCalibStatus->GPSData.fHDOP        = 999;         // normal range 0 to 100 ?

                  ptrCalibStatus->GPSData.ucLastValidData = 255;      // normal range 0 to 6

                  ptrCalibStatus->GPSData.uiGroundSpeed   = 65000;    // normal range 0 to 30000 cm/s
                  ptrCalibStatus->GPSData.uiHeading       = 9999;     // normal range 0 to 3599 (tenth degrees)
                */
		  // no valid data, so increment counter up to 255
		  //
		  if(ptrCalibStatus->GPSDataMaster.ucLastValidData < 255)
		    {
		       ptrCalibStatus->GPSDataMaster.ucLastValidData++;
		       ptrCalibStatus->GPSDataMaster.ucNumberOfSatellites = ucGGANumOfSatsInUse;   // copy number of sats to see where problem is
		    }
	       }; /* if quality */
	  }
	/* if ucDataFlag */
     }
   /* if IsMaster */

   else

     // *****************
     // * SLAVE GPS
     // *****************
     {
        if(1 == ucDataReadyFlag)
	  {
	     if(ucGGAGPSQuality > 0) // check if data valid
	       {
		  // copy values into structure
		  //
		  ptrCalibStatus->GPSDataSlave.ucUTCHours   = ucGGAHour;      // Time UTC
		  ptrCalibStatus->GPSDataSlave.ucUTCMins    = ucGGAMinute;
		  ptrCalibStatus->GPSDataSlave.ucUTCSeconds = ucGGASecond;

		  ptrCalibStatus->GPSDataSlave.dLongitude   = dGGALongitude;  // Longitude (Laengengrad)
		  ptrCalibStatus->GPSDataSlave.dLatitude    = dGGALatitude;   // Latitude (Breitengrad)

		  ptrCalibStatus->GPSDataSlave.fAltitude    = dGGAAltitude;   // normal range 0 to 18000 m
		  ptrCalibStatus->GPSDataSlave.fHDOP        = dGGAHDOP;       // normal range 0 to 100 ?

		  ptrCalibStatus->GPSDataSlave.ucNumberOfSatellites = ucGGANumOfSatsInUse;   // normal range 1-12
		  ptrCalibStatus->GPSDataSlave.ucLastValidData = 0;           // normal range 0 to 6

		  // speed
		  //
		  if(dVTGSpeedInKmh > 0)                                // check for positive speed
		    {
		       dVTGSpeedInKmh = dVTGSpeedInKmh / 3.6;             // will give metres per second
		       dVTGSpeedInKmh = dVTGSpeedInKmh * 100;             // will give cetimetres per second

		       ptrCalibStatus->GPSDataSlave.uiGroundSpeed   = (uint16_t)dVTGSpeedInKmh;
		    }
		  else
		    {
		       ptrCalibStatus->GPSDataSlave.uiGroundSpeed   = 0;        // no speed, sets to 0
		    };

		  // heading
		  // multiply heading by 10, will result in an int from 0 to 3599
		  dVTGTrueHeading = dVTGTrueHeading * 10;
		  ptrCalibStatus->GPSDataSlave.uiHeading       = (uint16_t)dVTGTrueHeading;

		  ucDataReadyFlag = FALSE;
	       }
	     else
	       {
		  // set data to initial values, which normaly cannot exist
                /*
                  ptrCalibStatus->GPSData.ucUTCHours   = 0;           // Time -> 00:00:00
                  ptrCalibStatus->GPSData.ucUTCMins    = 0;
                  ptrCalibStatus->GPSData.ucUTCSeconds = 0;

                  ptrCalibStatus->GPSData.dLongitude   = 999.99;      // normal range -180 to +180
                  ptrCalibStatus->GPSData.dLatitude    = 99.99;       // normal range -90 to +90

                  ptrCalibStatus->GPSData.fAltitude    = -99999;      // normal range 0 to 18000 m
                  ptrCalibStatus->GPSData.fHDOP        = 999;         // normal range 0 to 100 ?

                  ptrCalibStatus->GPSData.ucLastValidData = 255;      // normal range 0 to 6

                  ptrCalibStatus->GPSData.uiGroundSpeed   = 65000;    // normal range 0 to 30000 cm/s
                  ptrCalibStatus->GPSData.uiHeading       = 9999;     // normal range 0 to 3599 (tenth degrees)
                */
		  // no valid data, so increment counter up to 255
		  //
		  if(ptrCalibStatus->GPSDataSlave.ucLastValidData < 255)
		    {
		       ptrCalibStatus->GPSDataSlave.ucLastValidData++;
		       ptrCalibStatus->GPSDataSlave.ucNumberOfSatellites = ucGGANumOfSatsInUse;   // copy number of sats to see where problem is
		    }
	       }; /* if quality */
	  }
	/* if ucDataFlag */
     }
   /* if IsSlave */
}
/* GetGPSData */

/**********************************************************************************************************/
/* GetCalibStatus                                                                                          */
/**********************************************************************************************************/

/* function to retrieve Statusinformation */
void GetCalibStatus ( struct calibStatusType *ptrCalibStatus, int IsMaster)
{

   // we only have a etalon card in the master
   if(IsMaster)
     {
        // get time
        gettimeofday(&(ptrCalibStatus->TimeOfDayMaster), NULL);
     }
   else
     {
        gettimeofday(&(ptrCalibStatus->TimeOfDaySlave), NULL);
     };

   // EthalonCard is only in Master
   if(IsMaster)
     GetEtalonCardData(ptrCalibStatus);

   // Counter Card
   //
   elkWriteData(ELK_COUNTER_STATUS, ELK_COUNTER_STATUS_FLIP );                  // ask for flip Buffer
   elkWriteData(ELK_COUNTER_STATUS, 0);
   // and reset flip flop
   // ..will take some time, so we have time for something different....
   // now lets see what the counter card has...
   GetCounterCardData(ptrCalibStatus, IsMaster);

   // ADC Card
   GetADCCardData(ptrCalibStatus,IsMaster);

   // MFC Card
   GetMFCCardData(ptrCalibStatus,IsMaster);

   // DCDC4 Card only on Master
   //
   if(IsMaster)
     GetDCDC4CardData(ptrCalibStatus);

   // Valve Card
   GetValveCardData(ptrCalibStatus,IsMaster);

   // now get the temperature data
   GetTemperatureCardData(ptrCalibStatus,IsMaster);

   // now get the GPS date
   GetGPSData(ptrCalibStatus,IsMaster);

}
/* GetCalibStatus */

/**********************************************************************************************************/
/* Init UDP Ports                                                                                         */
/**********************************************************************************************************/

int InitUDPPorts(fd_set *pFDsMaster, int *fdMax)
{

   extern struct MessagePortType MessageInPortList[];
   extern struct MessagePortType MessageOutPortList[];
   int    MessagePort;

   // init inports
   for (MessagePort=0; MessagePort<MAX_MESSAGE_INPORTS;MessagePort++)
     {

        printf("opening IN Port %s on Port %d Socket:",
               MessageInPortList[MessagePort].PortName,
               MessageInPortList[MessagePort].PortNumber);

        MessageInPortList[MessagePort].fdSocket=InitUDPInSocket(MessageInPortList[MessagePort].PortNumber);

        FD_SET(MessageInPortList[MessagePort].fdSocket, pFDsMaster);     // add the manual port to the master set
        printf("%08x\n",MessageInPortList[MessagePort].fdSocket);
        (*fdMax)=MessageInPortList[MessagePort].fdSocket;                   // the last one will give the max number
     }
   /* for MessageInPort */

   // init outports
   for (MessagePort=0; MessagePort<MAX_MESSAGE_OUTPORTS;MessagePort++)
     {
        printf("opening OUT Port %s on Port %d, Destination IP: %s\n",
               MessageOutPortList[MessagePort].PortName,
               MessageOutPortList[MessagePort].PortNumber,
               MessageOutPortList[MessagePort].IPAddr);
        MessageOutPortList[MessagePort].fdSocket=InitUDPOutSocket(MessageOutPortList[MessagePort].PortNumber);

     }
   /* for MessageOutPort */

   return(1);

}
/*InitUDPPorts*/

/**********************************************************************************************************/
/* Change Priority                                                                                        */
/**********************************************************************************************************/

int ChangePriority()
{

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
   param.sched_priority= (int)((max-min)/2);
   if (-1==(ret=sched_setscheduler(0,SCHED_RR, &param)))
     {
        perror("kann scheduler nicht wechseln");
     }

   return (ret);

}
/* ChangePriority */

/**********************************************************************************************************/
/* MAIN                                                                                                   */
/**********************************************************************************************************/

int main(int argc, char *argv[])
{
   extern int errno;
   extern int StatusFlag;
   extern enum TimerSignalStateEnum TimerState;

   int IsMaster = 1;

   extern struct MessagePortType MessageInPortList[];
   extern struct MessagePortType MessageOutPortList[];

   struct calibStatusType CalibStatus;
   struct calibStatusType CalibStatusFromSlave;

   uint64_t ulCpuClock = CPUCLOCK;

   int nLostPackets = 0;				// counter for lost packets from slave

   int fdMax;                      // max fd for select
   int i;                          // loop counter
   int fdNum;                      // fd number in loop
   fd_set fdsMaster;               // master file descriptor list
   fd_set fdsSelect;               // temp file descriptor list for select()
   int ret;
   uint64_t ProcessTick;
   uint64_t TSC,TSCin;
   uint64_t TSCsentPacket;
   uint64_t MinTimeDiff=1e6;
   uint64_t MaxTimeDiff=0;
   struct timeval StartAction;
   struct timeval StopAction;
   struct timeval LastAction;
   struct timeval GetStatusStartTime;
   struct timeval GetStatusStopTime;
   float ProcessTime;
   struct tm tmZeit;

   struct sigaction  SignalAction;
   struct sigevent   SignalEvent;
   sigset_t          SignalMask;

   struct itimerspec StatusTimer;
   timer_t           StatusTimer_id;
   clock_t           clock = CLOCK_REALTIME;
   int               StatusInterval=STATUS_INTERVAL;

   struct timespec pselect_timeout;

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
   int RequestDataFlag;

   if (elkInit())
     {
	// grant IO access
        printf("Error: failed to grant IO access rights\n");
        exit(EXIT_FAILURE);
     }

   // setup master fd
   FD_ZERO(&fdsMaster);              // clear the master and temp sets
   FD_ZERO(&fdsSelect);
   InitUDPPorts(&fdsMaster,&fdMax);                  // Setup UDP in and out Ports

   addr_len = sizeof(struct sockaddr);

   // output version info on debugMon and Console
   //
   printf("This is elekIOcalib Version %3.2f (CVS: $RCSfile: elekIOcalib.c,v $ $Revision: 1.1 $) for ARM\n",VERSION);
   SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],buf);

    /* init all modules */
   InitModules(&CalibStatus,IsMaster);

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
   if (ret < 0)
     {
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
   StatusTimer.it_interval.tv_usec = (STATUS_INTERVAL*1000);
   StatusTimer.it_value = StatusTimer.it_interval;
#endif

#ifdef RUNONPC
   ret = timer_settime(StatusTimer_id, 0, &StatusTimer, NULL);
   if (ret < 0)
     {
        perror("timer_settime");
        return EXIT_FAILURE;
     }
#endif

#ifdef RUNONARM
   ret = setitimer(ITIMER_REAL, (const struct itimerval*)(&StatusTimer), NULL);
   if (ret < 0)
     {
        perror("settimer");
        return EXIT_FAILURE;
     }

#endif

#ifdef RUNONPC
   // change scheduler and set priority
   if (-1==(ret=ChangePriority()))
     {
        SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],"ElekIOServ: cannot set Priority");
     }

#endif

   sigemptyset(&SignalMask);
   //    sigsuspend(&SignalMask);
   //
   gettimeofday(&LastAction, NULL);
   EndOfSession=FALSE;
   RequestDataFlag=FALSE;

   while (!EndOfSession)
     {
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
        ret=select(fdMax+1, &fdsSelect, NULL, NULL, &select_timeout);             // wait until incoming udp or Signal
#endif

        gettimeofday(&StartAction, NULL);

#ifdef DEBUG_TIMER
	printf("Time:");
	localtime_r(&StartAction.tv_sec,&tmZeit);

	printf("%02d:%02d:%02d.%03d :%d\n", tmZeit.tm_hour, tmZeit.tm_min,
	       tmZeit.tm_sec, StartAction.tv_usec/1000,TimerState);
#endif

        //	printf("ret %d StatusFlag %d\n",ret,StatusFlag);
        if (ret ==-1 )
	  {
	     // select error
	     //
	     if (errno==EINTR)
	       {
		  // got interrupted by timer
		  if (TimerState==TIMER_SIGNAL_STATE_GATHER)
		    {
		       // shall we gather data ?
		       if ((StatusFlag % 100)==0)
			 //                        printf("get Status %6d..\r",StatusFlag);
			 write(2,"get Status.....\r",16);

		       if (CalibStatus.InstrumentFlags.StatusQuery)
			 {
			    if (IsMaster)
			      {

				 // get TSC for timing
#ifdef RUNONPC // but only if run on PC
				 rdtscll(TSC);
#endif
				 // ask all slave units to gather their data
				 RequestDataFlag=0; // reset RequestDataFlag
				 CalibStatus.uiValidSlaveDataFlag=FALSE;
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
					   sprintf(buf,"Send request for data to %s @%08x",
						   SlaveList[SlaveNum].SlaveName,Message.MsgTime);
					   SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],buf);
#endif
					   RequestDataFlag++; // We need one more status to wait for completion

					}
				      /* end if(Slavelist) */
				   }
				 // end for()
				 //
				 // printf("elekIOServ(m): Request for %d data set\n",RequestDataFlag);
				 TSCsentPacket =TSC;
				 // we reset the elekIOStructure
				 // until data not properly separated we do it only for slave
				 void* pDest   = (void*)&(CalibStatus.TimeOfDaySlave); 		// copy to first slave element(M)
				 size_t numBytes = sizeof(CalibStatus)-(((void*)&CalibStatus.TimeOfDaySlave)-((void*)&CalibStatus));
				 memset(pDest,0,numBytes);

				 gettimeofday(&GetStatusStartTime, NULL);
				 GetCalibStatus(&CalibStatus,IsMaster);
				 gettimeofday(&GetStatusStopTime, NULL);
				 // printf("elekIOServ(m): Data aquisition took: %02d.%03ds\n\r",
				 //                                   GetStatusStopTime.tv_sec-GetStatusStartTime.tv_sec,
				 //       (GetStatusStopTime.tv_usec-GetStatusStartTime.tv_usec)/1000);
			      }
			    // If IsMaster
			    //
			    //   commented out, send later when message is assembled from master+slave
			    //	  SendUDPData(&MessageOutPortList[ELEK_STATUS_OUT],sizeof(struct calibStatusType), &CalibStatus);

			    //		sprintf(buf,"ElekIOServ: send Status of Numbytes : %d\n",sizeof(struct calibStatusType));
			    //		SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],buf);
			 }
		       else
			 {
			    SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],"ElekIOServ: not Querying Status !!");
			 }
		       /* if Query */

		       if (SyncFlag.MaskChange==TRUE)
			 {
			    SetCounterCardMask(&CalibStatus, IsMaster);
			 }
		       /* if Syncflag.MaskChange */

		       if(IsMaster)
			 {
			    // let the other tasks know that we are done with retrieving the status
			    // not needed if we are running as slave, as there is no Etalon nor Script
			    //
			    Task=0;
			    Message.MsgType=MSG_TYPE_SIGNAL;
			    Message.MsgID=-1;
			    while (TasktoWakeList[Task].TaskConn>-1)
			      {
				 if (TasktoWakeList[Task].TaskWantStatusOnPort>-1)
				   { // but only if he wants to
				      SendUDPData(&MessageOutPortList[TasktoWakeList[Task].TaskWantStatusOnPort],
						  sizeof(struct calibStatusType), &CalibStatus);
				   }
				 SendUDPData(&MessageOutPortList[TasktoWakeList[Task].TaskConn],
					     sizeof(struct ElekMessageType), &Message);
				 Task++;
			      }
			    // while task
			 }
		       // if isMaster
		    }
		  else
		    {
		       // lets see whether there is still an open data request
		       if (RequestDataFlag)
			 {
			    sprintf(buf,"[ElekIOServ] still missing %d data set BUT send flagged data",
				    RequestDataFlag);
			    SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],buf);
			    // Send Status to Status process
			    SendUDPData(&MessageOutPortList[ELEK_STATUS_OUT],
					sizeof(struct calibStatusType), &CalibStatus);

			 }
		       // RequestDataFlag
		       //
		    }
		  // if TimerState
	       }
	     else
	       {
		  //  if(errno==EINTR)  so was not the Timer, it was a UDP Packet that caused err
		  perror("select");
		  SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],"ElekIOServ: Problem with select");
	       }
	     // if errno
	  }
	else if (ret>0)
	  {
	     //	    printf("woke up...");
	     write(2,"incoming Call..\r",16);

	     for (MessagePort=0; MessagePort<MAX_MESSAGE_INPORTS;MessagePort++)
	       {

		  if (FD_ISSET(MessageInPortList[MessagePort].fdSocket,&fdsSelect))
		    {
		       // new msg on fdNum. socket ...
		       // printf("fdsSelect: %016lx\n\r",fdsSelect);
		       //		    fdElekManual=MessagePortList[MessagePort].fdSocket;
		       //		    fdElekManual=MessagePortList[0].fdSocket;
		       switch (MessagePort)
			 {
			  case ELEK_MANUAL_IN:       // port for incoming commands from  eCmd
			  case ELEK_ETALON_IN:       // port for incoming commands from  etalon
			  case ELEK_SCRIPT_IN:       // port for incoming commands from  scripting host (not yet existing)

                            if ((numbytes=recvfrom(MessageInPortList[MessagePort].fdSocket,
                                                   &Message,sizeof(struct ElekMessageType)  , 0,
                                                   (struct sockaddr *)&their_addr, &addr_len)) == -1)
			      {
				 perror("recvfrom");
				 SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],"ElekIOServ: Problem with recieve");
			      }
#ifdef DEBUG_SLAVECOM
                            sprintf(buf,"recv command from %s on port %d",inet_ntoa(their_addr.sin_addr),
                                    ntohs(their_addr.sin_port));
                            SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],buf);
#endif

                            switch (Message.MsgType)
			      {

			       case MSG_TYPE_FETCH_DATA:  // Master want data
#ifdef DEBUG_SLAVECOM
				 // printf("ElekIOServ(s): FETCH_DATA received, TSC was: %016lx\n\r", Message.MsgTime);
#endif
				 CalibStatus.TimeStampCommand.TSCReceived.TSCValue = Message.MsgTime;
#ifdef DEBUG_SLAVECOM
				 // printf("ElekIOServ(s): gathering Data...\n\r");
#endif
				 gettimeofday(&GetStatusStartTime, NULL);
				 GetCalibStatus(&CalibStatus,IsMaster);
				 gettimeofday(&GetStatusStopTime, NULL);
#ifdef DEBUG_SLAVECOM
				 // printf("ElekIOServ(s): Data aquisition took: %02d.%03ds\n\r",
				 GetStatusStopTime.tv_sec-GetStatusStartTime.tv_sec,
				   (GetStatusStopTime.tv_usec-GetStatusStartTime.tv_usec)/1000);
#endif
			    // send this debugmessage message to debugmon
			    sprintf(buf,"ElekIOServ(s): FETCH_DATA from Port: %05d\n\r",
				    MessageInPortList[MessagePort].PortNumber,
				    Message.Addr,Message.Value,Message.Value);
			    SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],buf);

			    // send requested data, don't send any acknowledges
			    SendUDPData(&MessageOutPortList[ELEK_ELEKIO_SLAVE_MASTER_OUT],
					sizeof(struct calibStatusType), &CalibStatus); // send data packet
			    break;

			  case MSG_TYPE_READ_DATA:
			    // printf("elekIOServ: manual read from Address %04x\n", Message.Addr);
			    Message.Value=elkReadData(Message.Addr);
			    Message.MsgType=MSG_TYPE_ACK;

			    if (MessagePort!=ELEK_ETALON_IN)
			      {
				 sprintf(buf,"ElekIOServ: ReadCmd from %4d Port %04x Value %d (%04x)",
					 MessageInPortList[MessagePort].PortNumber,
					 Message.Addr,Message.Value,Message.Value);
				 SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],buf);

				 sprintf(buf,"%d",MessageInPortList[MessagePort].RevMessagePort);
				 SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],buf);
			      }
			    /* if MessagePort */
			    SendUDPDataToIP(&MessageOutPortList[MessageInPortList[MessagePort].RevMessagePort],
					    inet_ntoa(their_addr.sin_addr),
					    sizeof(struct ElekMessageType), &Message);
			    break;
			  case MSG_TYPE_WRITE_DATA:
			    if (MessagePort!=ELEK_ETALON_IN)
			      {
				 sprintf(buf,"ElekIOServ: WriteCmd from %4d Port %04x Value %d (%04x)",
					 MessageInPortList[MessagePort].PortNumber,
					 Message.Addr,Message.Value,Message.Value);
				 SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],buf);
			      }
			    /* if MessagePort */

			    // printf("elekIOServ: manual write to Address %04x Value is: %04x\n", Message.Addr, Message.Value);
			    //
			    Message.Status=elkWriteData(Message.Addr,Message.Value);
			    Message.MsgType=MSG_TYPE_ACK;
			    SendUDPDataToIP(&MessageOutPortList[MessageInPortList[MessagePort].RevMessagePort],
					    inet_ntoa(their_addr.sin_addr),
					    sizeof(struct ElekMessageType), &Message);
			    break;

			  case MSG_TYPE_CHANGE_FLAG_STATUS_QUERY:

			    if (Message.Value) CalibStatus.InstrumentFlags.StatusQuery=1;
			    else CalibStatus.InstrumentFlags.StatusQuery=0;
			    sprintf(buf,"elekIOServ: Set StatusQuery to %d %d",Message.Value,CalibStatus.InstrumentFlags.StatusQuery);
			    SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],buf);
			    Message.MsgType=MSG_TYPE_ACK;
			    SendUDPDataToIP(&MessageOutPortList[MessageInPortList[MessagePort].RevMessagePort],
					    inet_ntoa(their_addr.sin_addr),
					    sizeof(struct ElekMessageType), &Message);

			    break;

			  case MSG_TYPE_CHANGE_FLAG_STATUS_SAVE:
			    if (Message.Value) CalibStatus.InstrumentFlags.StatusSave=1;
			    else CalibStatus.InstrumentFlags.StatusSave=0;
			    sprintf(buf,"elekIOServ: Set StatusSave to %d",CalibStatus.InstrumentFlags.StatusSave);
			    SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],buf);
			    Message.MsgType=MSG_TYPE_ACK;
			    SendUDPDataToIP(&MessageOutPortList[MessageInPortList[MessagePort].RevMessagePort],
					    inet_ntoa(their_addr.sin_addr),
					    sizeof(struct ElekMessageType), &Message);

			    break;

			  case MSG_TYPE_CHANGE_FLAG_INSTRUMENT_ACTION:
			    CalibStatus.InstrumentFlags.InstrumentAction=Message.Value;
			    sprintf(buf,"elekIOServ: Set InstrumentAction to %d",CalibStatus.InstrumentFlags.InstrumentAction);
			    SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],buf);
			    Message.MsgType=MSG_TYPE_ACK;
			    SendUDPDataToIP(&MessageOutPortList[MessageInPortList[MessagePort].RevMessagePort],
					    inet_ntoa(their_addr.sin_addr),
					    sizeof(struct ElekMessageType), &Message);

			    break;

			  case MSG_TYPE_CHANGE_FLAG_ETALON_ACTION:
			    CalibStatus.InstrumentFlags.EtalonAction=Message.Value;
			    if (MessagePort!=ELEK_ETALON_IN)
			      {
				 sprintf(buf,"elekIOServ: Set EtalonAction to %d",CalibStatus.InstrumentFlags.EtalonAction);
				 SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],buf);
			      }
			    /* if MessagePort */
			    Message.MsgType=MSG_TYPE_ACK;
			    SendUDPDataToIP(&MessageOutPortList[MessageInPortList[MessagePort].RevMessagePort],
					    inet_ntoa(their_addr.sin_addr),
					    sizeof(struct ElekMessageType), &Message);

			    break;

			  case MSG_TYPE_CHANGE_FLAG_SYSTEM_PARAMETER:
			    switch (Message.Addr)
			      {
			       case SYS_PARAMETER_ETALON_ONLINE:
				 CalibStatus.EtalonData.Online.Position=Message.Value;
				 break;

			       case SYS_PARAMETER_ETALON_OFFLINE_LEFT:
				 CalibStatus.EtalonData.OfflineStepLeft=Message.Value;
				 break;

			       case SYS_PARAMETER_ETALON_OFFLINE_RIGHT:
				 CalibStatus.EtalonData.OfflineStepRight=Message.Value;
				 break;

			       case SYS_PARAMETER_ETALON_DITHER:
				 CalibStatus.EtalonData.DitherStepWidth=Message.Value;
				 break;

			       case SYS_PARAMETER_ETALON_SCAN_START:
				 CalibStatus.EtalonData.ScanStart.Position=Message.Value;
				 break;

			       case SYS_PARAMETER_ETALON_SCAN_STOP:
				 CalibStatus.EtalonData.ScanStop.Position=Message.Value;
				 break;

			       case SYS_PARAMETER_ETALON_SCAN_STEP:
				 CalibStatus.EtalonData.ScanStepWidth=Message.Value;
				 break;

			       default:
				 sprintf(buf,"elekIOServ: Unkown SysParameter %d",Message.Addr);
				 SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],buf);
				 break;
			      }
			    /* switch Message.Addr Sysparameter */

			    if (MessagePort!=ELEK_ETALON_IN)
			      {
				 sprintf(buf,"elekIOServ: Change %s to %d",strSysParameterDescription[Message.Addr],Message.Value);
				 SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],buf);
			      }
			    /* if MessagePort */
			    Message.MsgType=MSG_TYPE_ACK;
			    SendUDPDataToIP(&MessageOutPortList[MessageInPortList[MessagePort].RevMessagePort],
					    inet_ntoa(their_addr.sin_addr),
					    sizeof(struct ElekMessageType), &Message);

			    break;

			  case MSG_TYPE_CHANGE_MASK:
			    if ((Message.Addr<MAX_COUNTER_CHANNEL*10))
			      {
				 Channel=(int)(Message.Addr/10);
				 MaskAddr=Message.Addr%10;
				 if (IsMaster)
				   CalibStatus.CounterCardMaster.Channel[Channel].Mask[MaskAddr]=Message.Value;
				 else
				   CalibStatus.CounterCardSlave.Channel[Channel].Mask[MaskAddr]=Message.Value;

				 sprintf(buf,"elekIOServ: Set Mask Addr %d of Channel %d to %x", MaskAddr, Channel, Message.Value);
				 SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],buf);
			      }
			    else
			      {
				 // we found that Message.Addr is larger than MAX_COUNTER_CHANNEL*10
				 sprintf(buf,"elekIOServ: Set Mask : Addr %d is larger than %d", Message.Addr, MAX_COUNTER_CHANNEL*10);
				 SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],buf);
			      }
			    /* if Message.Addr >=0 ... */

			    Message.MsgType=MSG_TYPE_ACK;
			    SendUDPDataToIP(&MessageOutPortList[MessageInPortList[MessagePort].RevMessagePort],
					    inet_ntoa(their_addr.sin_addr),
					    sizeof(struct ElekMessageType), &Message);

			    SyncFlag.MaskChange=TRUE;

			    break;

			  default:
			    SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],"ElekIOServ: Unknown Message Type");
			    break;

			 }
		       /* switch MsgType */
		       break;
		       // port to receive status data from slaves
		       //
		     case ELEK_STATUS_IN:
		       if(IsMaster)
			 {
			    if ((numbytes=recvfrom(MessageInPortList[MessagePort].fdSocket,
						   &CalibStatusFromSlave,sizeof(CalibStatusFromSlave) , MSG_WAITALL,
						   (struct sockaddr *)&their_addr, &addr_len)) == -1)
			      // Something went wrong
			      {
				 perror("elekIOServ(M): recvfrom ELEK_STATUS_IN:");
				 SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],"elekIOServ : ELEK_STATUS_IN: problem with receive");
			      }
			    else
			      // we should have received some data at this point, so give a lifesign
			      {
				 // printf("elekIOServ(M): ELEK_STATUS_IN: got %d bytes from Slave!\n\r",numbytes);
				 // printf("elekIOServ(M): Timestamp: %016lx\n\r",CalibStatusFromSlave.TimeStampCommand.TSCReceived.TSCValue);
#ifdef RUNONPC
				 if(TSCsentPacket != CalibStatusFromSlave.TimeStampCommand.TSCReceived.TSCValue)
				   {
				      // printf("elekIOServ(M): Timestamps from sent packet and received packet don't match!\n\r");
				      nLostPackets++;
				      // printf("elekIOServ(M): Number of lost packets: %05d\n\r");
				   }
				 rdtscll(TSC);
#endif
				 ProcessTick=(TSC-(uint64_t)CalibStatusFromSlave.TimeStampCommand.TSCReceived.TSCValue);
				 ProcessTime=ProcessTick/((double)(ulCpuClock));
				 //printf("elekIOServ(M): Packet took %lld CPU clock cycles to process.\n\r", ProcessTick);
				 //printf("elekIOServ(M): That's %f s\n\r", ProcessTime);

				 void* pDest   = (void*)&(CalibStatus.TimeOfDaySlave); 		// copy to first slave element(M)
				 void* pSource = (void*)&(CalibStatusFromSlave.TimeOfDaySlave); 	// copy from first slave element(S)
				 size_t numBytes = sizeof(CalibStatus)-(((void*)&CalibStatus.TimeOfDaySlave)-((void*)&CalibStatus));

#ifdef DEBUG_SLAVECOM
				 // printf("elekIOServ(M): copying %d bytes from SlaveStruct to MasterStruct\n\r",numBytes);
#endif
				 memcpy(pDest,pSource,numBytes);
				 CalibStatus.uiValidSlaveDataFlag=TRUE;
				 RequestDataFlag--; // we got one more data set
				 // printf("elekIOServ(m): waiting for %d more data set\n",
				 //RequestDataFlag);

				 if (RequestDataFlag==0)
				   {
				      // do we have all data yet ?
				      // Send Status to Status process
				      SendUDPData(&MessageOutPortList[ELEK_STATUS_OUT],
						  sizeof(struct calibStatusType), &CalibStatus);
				   }
				 // if RequestDataFlag
				 //
				 if (ProcessTime>MAX_AGE_SLAVE_STATUS)
				   {
				      sprintf(buf,"elekIOServ: Slave took too long for status %f",ProcessTime);
				      SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],buf);

				   }
				 // if processTime
				 //
			      };
			 }
		       else
			 {
			    // Slave Mode
			    //
			    if ((numbytes=recvfrom(MessageInPortList[MessagePort].fdSocket,
						   &CalibStatusFromSlave,sizeof(CalibStatusFromSlave) , 0,
						   (struct sockaddr *)&their_addr, &addr_len)) == -1)
			      // Something went wrong
			      {
				 perror("elekIOServ(S): recvfrom ELEK_STATUS_IN");
				 SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],"elekIOServ : ELEK_STATUS_IN: problem with receive");
			      }
			    else
			      // we should have received some data at this point, so give a lifesign
			      {
				 //printf("elekIOServ(S) :ELEK_STATUS_IN: got %d bytes from Master!\n\r",numbytes);
			      };
			 };

		       break;

		     default:
		       SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],"ElekIOServ: Unknown Port Type");
		       break;

                    }
		  // switch MessagePort
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
	       }
	     /* if fd_isset */
	  }
	/* for MessagePort */

     }
   else
     {
	/* ret==0*/
	//	    printf("timeout...\n");
	write(2,"timeout........\r",16);
	SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],"ElekIOServ: TimeOut");
     }

#ifdef RUNONPC
   if (timer_getoverrun(StatusTimer_id)>0)
     {
	printf("OVERRUN\n\r");
	SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],"ElekIOServ: Overrun");
     }
   /* if overrun */
#endif

   gettimeofday(&StopAction, NULL);

#ifdef DEBUG_TIME_TASK
   sprintf(buf,"ElekIOServ: %ld RT: %ld DT: %ld",StartAction.tv_usec/1000,
	   StopAction.tv_usec-StartAction.tv_usec, (StartAction.tv_usec-LastAction.tv_usec)/1000);
   SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],buf);
#endif

   LastAction=StartAction;
}
/* while */

#ifdef RUNONPC
    /* delete timer */
timer_delete(StatusTimer_id);
#endif

// close all in bound sockets
for (MessagePort=0; MessagePort<MAX_MESSAGE_INPORTS;MessagePort++)
{
   close(MessageInPortList[MessagePort].fdSocket);
}
/*for MessagePort */

// close all out bound sockets
for (MessagePort=0; MessagePort<MAX_MESSAGE_OUTPORTS;MessagePort++)
{
   close(MessageOutPortList[MessagePort].fdSocket);
}
/*for MessagePort */

if (elkExit())
{
   // release IO access
   printf("Error: failed to release IO access rights\n");
   exit(EXIT_FAILURE);
}

exit(EXIT_SUCCESS);
}

