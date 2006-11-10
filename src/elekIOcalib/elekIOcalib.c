/*
 * $RCSfile: elekIOcalib.c,v $ last changed on $Date: 2006-11-10 17:28:08 $ by $Author: rudolf $
 *
 * $Log: elekIOcalib.c,v $
 * Revision 1.8  2006-11-10 17:28:08  rudolf
 * more work on LICOR integration
 *
 * Revision 1.7  2006/11/03 15:43:35  rudolf
 * made PID regulation work
 *
 * Revision 1.6  2006/11/02 12:42:01  rudolf
 * more work on elekIOcalib
 *
 * Revision 1.5  2006/09/04 13:04:21  rudolf
 * fix for GCC 4.03
 *
 * Revision 1.4  2006/09/01 15:49:08  rudolf
 * more work on calibrator UDP handling
 *
 * Revision 1.3  2006/08/31 17:13:51  rudolf
 * formatted debug output, more work in progress
 *
 * Revision 1.1  2006/08/30 15:56:30  rudolf
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
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <ctype.h> /*for NAN*/
#include <stdlib.h>
#include <math.h>

#include <signal.h>
#include <errno.h>
#include <sched.h>

#include "../include/elekGeneral.h"
#include "../include/elekIO.h"
#include "../include/elekIOPorts.h"
#include "elekIOcalib.h"
#include "serial.h"
#include "licor.h"

#define STATUS_INTERVAL  100

#define DEBUGLEVEL 0
// #define DEBUG_SLAVECOM
#undef DEBUG_TIMER

#define CPUCLOCK 500000000UL 	// CPU clock of Markus' Athlon XP

enum InPortListEnum
{
   // this list has to be coherent with MessageInPortList
   ELEK_MANUAL_IN,       // port for incoming commands from  eCmd
     ELEK_ETALON_IN,       // port for incoming commands from  etalon
     ELEK_SCRIPT_IN,       // port for incoming commands from  scripting host (not yet existing, HH, Feb2005
     //     ELEK_STATUS_IN,       // port to receive status data from slaves
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
     ELEK_ELEKIO_CALIB_MASTER_OUT,   // port for outgoing data packets from calib to master
     MAX_MESSAGE_OUTPORTS
};

static struct MessagePortType MessageInPortList[MAX_MESSAGE_INPORTS]=
{
   // order in list defines sequence of polling
    /* Name   , PortNo                    , ReversePort  , IPAddr, fdSocket, MaxMsg, Direction */
     {"Manual"        , UDP_ELEK_MANUAL_INPORT        , ELEK_MANUAL_OUT, IP_LOCALHOST, -1, 1,  UDP_IN_PORT},
     {"Etalon"        , UDP_ELEK_ETALON_INPORT        , ELEK_ETALON_OUT, IP_LOCALHOST, -1, 10, UDP_IN_PORT},
     {"Script"        , UDP_ELEK_SCRIPT_INPORT        , ELEK_SCRIPT_OUT, IP_LOCALHOST, -1, 5,  UDP_IN_PORT},
   //     {"ElekIOCalibIn" , UDP_ELEK_CALIB_DATA_INPORT    , -1             , IP_LOCALHOST, -1, 1,  UDP_IN_PORT} // status inport from elekIOServ
};

static struct MessagePortType MessageOutPortList[MAX_MESSAGE_OUTPORTS]=
{
   // order in list defines sequence of polling
    /* Name           ,PortNo                        , ReversePort        , IPAddr, fdSocket, MaxMsg, Direction */
     {"Status"        ,UDP_ELEK_STATUS_STATUS_OUTPORT, -1                    , IP_STATUS_CLIENT, -1, 0,  UDP_OUT_PORT},
     {"ElekIOStatus"  ,UDP_ELEK_SLAVE_DATA_INPORT    , -1                    , IP_ELEKIO_MASTER, -1, 0,  UDP_OUT_PORT},
     {"ElekIOServer"  ,UDP_ELEK_MANUAL_INPORT        , ELEK_ELEKIO_STATUS_OUT, IP_ELEK_SERVER  , -1, 0,  UDP_OUT_PORT},
     {"Manual"        ,UDP_ELEK_MANUAL_OUTPORT       , ELEK_MANUAL_IN        , IP_LOCALHOST    , -1, 0,  UDP_OUT_PORT},
     {"Etalon"        ,UDP_ELEK_ETALON_OUTPORT       , -1                    , IP_ETALON_CLIENT, -1, 0,  UDP_OUT_PORT},
     {"EtalonStatus"  ,UDP_ELEK_ETALON_STATUS_OUTPORT, -1                    , IP_STATUS_CLIENT, -1, 0,  UDP_OUT_PORT},
     {"Script"        ,UDP_ELEK_SCRIPT_OUTPORT       , ELEK_SCRIPT_IN        , IP_SCRIPT_CLIENT, -1, 0,  UDP_OUT_PORT},
     {"DebugPort"     ,UDP_ELEK_DEBUG_OUTPORT        , -1                    , IP_DEBUG_CLIENT , -1, 0,  UDP_OUT_PORT},
     {"ElekIOOut"     ,UDP_ELEK_SLAVE_DATA_INPORT    , -1                    , IP_ELEKIO_MASTER, -1, 0,  UDP_OUT_PORT},
     {"ElekIOcalibOut",UDP_ELEK_CALIB_DATA_INPORT    , -1                    , IP_ELEKIO_MASTER, -1, 0,  UDP_OUT_PORT}
};

static struct TaskListType TasktoWakeList[MAX_TASKS_TO_WAKE]=
{
   // order defines sequence of wake up after timer
    /* TaskName TaskConn TaskWantStatusOnPort */
     {"Etalon",     ELEK_ETALON_OUT,ELEK_ETALON_STATUS_OUT},    // Etalon Task needs Status info
     {"Script",     ELEK_SCRIPT_OUT,                    -1},
     {      "",                  -1,                    -1}
};
/**********************************************************************************************************/
/* PID algorithm                                                                                          */
/**********************************************************************************************************/

double ProcessPID(double dSetPoint, double dActualValue, struct calibStatusType *ptrCalibStatus)
{
   static double dErrorCurrent;
   static double dErrorLast;
   static double dErrorPenultimate;

   static double dControlVariableLast;
   static double dControlVariable;

   // coefficients for the PID regulator
   double dKP = (double) ptrCalibStatus->PIDRegulator.KP;
   double dKD = (double) ptrCalibStatus->PIDRegulator.KD;
   double dKI = (double) ptrCalibStatus->PIDRegulator.KI;

   // save the last 2 error values
   //
   dErrorPenultimate = dErrorLast;
   dErrorLast = dErrorCurrent;
   dErrorCurrent = dSetPoint - dActualValue;

   // do PID calcualtions
   dControlVariableLast = dControlVariable;
   dControlVariable = dControlVariableLast + (dKP * (dErrorCurrent - dErrorLast)) + (dKI * (dErrorCurrent - dErrorLast) / 2) + (dKD * (dErrorCurrent - (2*dErrorLast) + dErrorPenultimate));

   return dControlVariable;
}

/**********************************************************************************************************/
/* Signal Handler                                                                                         */
/**********************************************************************************************************/

static int StatusFlag=0;
static enum TimerSignalStateEnum TimerState=TIMER_SIGNAL_STATE_INITIAL;

/* Signalhandler */
void signalstatus(int signo)
{
   /* locale variables for Timer*/
   extern int StatusFlag;
   extern enum TimerSignalStateEnum TimerState;

   char buf[GENERIC_BUF_LEN];

   /* local variables for PID */
   extern struct calibStatusType CalibStatus;
   static int iPIDdelay = 0;
   double dControlValue;
   double dSetPoint;
   double dActualValue;

   ++StatusFlag;
   TimerState=(TimerState+1) % TIMER_SIGNAL_STATE_MAX;

   iPIDdelay++;

   /* PID is done here */
   if(iPIDdelay >= 10)
     {
	iPIDdelay = 0;
	/* check if setpoint valid, if not, turn heater off for safety reasons */
	if(CalibStatus.PIDRegulator.Setpoint > 0)
	  {

	     dSetPoint = ((double)CalibStatus.PIDRegulator.Setpoint)/100;
	     dActualValue = ((double)CalibStatus.PIDRegulator.ActualValueH2O)/100;

	     dControlValue = ProcessPID(dSetPoint,dActualValue,&CalibStatus);

	     // FIXME: add check for heater overtemp
	     // water to warm, turn off completely
	     if(dControlValue < 0)
	       elkWriteData(ELK_SCR_BASE + 0, 0);

	     // water much too cold, set full power
	     if(dControlValue > 255)
	       elkWriteData(ELK_SCR_BASE + 0, 255);
	     // else set calculated power value
	     else
	       elkWriteData(ELK_SCR_BASE + 0, (uint16_t)dControlValue);
	  }
	else
	  {
	     elkWriteData(ELK_SCR_BASE + 0, (uint16_t)dControlValue);
	  }
	CalibStatus.PIDRegulator.ControlValue = elkReadData(ELK_SCR_BASE + 0);
     }
}

/**********************************************************************************************************/
/* Converts a resistance value into kelvin                                                                */
/* valid for the blue NTCs from RS-Components with 3K @ 25°C                                              */
/* RS Part Number: 151-215                                                                                */
/*                                                                                                        */
/* Constant values taken from datasheet "NTC_EC_95data_2.pdf" on our server                               */
/**********************************************************************************************************/

double Resistance2Temperature(double dResistance)
{
   double dResRatio = dResistance/3000.0f; // resistance at 25 degrees is 3000 Ohm
   double dA,dB,dC,dD;
   double dNTCTemp;

   if((dResRatio <= 68.600f) && (dResRatio >= 3.274f))
     {
	dA = 3.35386E-03f;
	dB = 2.5654090E-04f;
	dC = 1.9243889E-06f;
	dD = 1.0969244E-07f;
     }
   else
     if((dResRatio < 3.274f) && (dResRatio >= 0.36036f))
       {
	  dA = 3.3540154E-03f;
	  dB = 2.5627725E-04f;
	  dC = 2.0829210E-06f;
	  dD = 7.3003206E-08f;
       }
   else
     if((dResRatio < 0.36036f) && (dResRatio >= 0.06831f))
       {
	  dA = 3.3539264E-03f;
	  dB = 2.5609446E-04f;
	  dC = 1.9621987E-06f;
	  dD = 4.6045930E-08f;
       }
   else
     if((dResRatio < 0.06831f) && (dResRatio >= 0.01872f))
       {
	  dA = 3.3368620E-03f;
	  dB = 2.4057263E-04f;
	  dC = -2.6687093E-06f;
	  dD = -4.0719355E-07f;
       }
   else
     {
	return NAN;
     };

   dNTCTemp = 1/(dA + (dB*log(dResRatio)) + (dC * pow((log(dResistance/3000.0f)),2)) + (dD * pow((log(dResistance/3000.0f)),3)));
   return dNTCTemp;
};

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

   // configure ADC Card
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
		 ptrCalibStatus->ADCCardCalib[Card].ADCChannelConfig[Channel].ADCChannelConfig)
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
/* Init ADCCard*/

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
			      ptrCalibStatus->MFCCardCalib[Card].MFCChannelConfig[Channel].MFCChannelConfig);
	     // proof read
	     if (elkReadData(MFC_CfgAddress+2*Channel)!=
		 ptrCalibStatus->MFCCardCalib[Card].MFCChannelConfig[Channel].MFCChannelConfig)
	       {
		  return (INIT_MODULE_FAILED); // mark ret=0;
	       }
	     /* if elkRead */

	     // init DAC part
	     ret=elkWriteData(DAC_CfgAddress+2*Channel,
			      ptrCalibStatus->MFCCardCalib[Card].MFCChannelData[Channel].SetFlow);

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

   for (Card=0; Card<MAX_TEMP_SENSOR_CARD_CALIB; Card++)
     {
	ptrCalibStatus->TempSensCardCalib[Card].NumMissed=0;              // reset Missed Reading Counter
     };

   return (INIT_MODULE_SUCCESS);
}
/* Init TempCard */

/**********************************************************************************************************/
/* Init SCRCard                                                                                        */
/**********************************************************************************************************/

int InitSCRCard (struct calibStatusType *ptrCalibStatus)
{
   int iChannel = 0;

   // turn all 230VAC outputs off
   for(iChannel=0;iChannel < MAX_SCR3XB_CHANNEL_PER_CARD;iChannel++)
     {
	elkWriteData(ELK_SCR_BASE + (2*iChannel), 0);
	ptrCalibStatus->SCRCardCalib[0].SCRPowerValue[iChannel] = 0x0000;
     };

   return (INIT_MODULE_SUCCESS);
}
/* Init SCRCard */

/**********************************************************************************************************/
/* Init PIDregulator                                                                                      */
/**********************************************************************************************************/

int InitPIDregulator (struct calibStatusType *ptrCalibStatus)
{
   ptrCalibStatus->PIDRegulator.Setpoint = 0;
   ptrCalibStatus->PIDRegulator.ActualValueH2O = 0;
   ptrCalibStatus->PIDRegulator.ActualValueHeater = 0;

   /* change here if regulation is not stable, might be changed at runtime also */
   ptrCalibStatus->PIDRegulator.KP = 100;
   ptrCalibStatus->PIDRegulator.KI = 80;
   ptrCalibStatus->PIDRegulator.KD = 20;

   /* heater off */
   ptrCalibStatus->PIDRegulator.ControlValue = 0;

   return (INIT_MODULE_SUCCESS);
}
/* Init SCRCard */

/**********************************************************************************************************/
/* Init LICOR                                                                                             */
/**********************************************************************************************************/

int InitLICOR(struct elekStatusType *ptrCalibStatus)
{

   extern struct MessagePortType MessageInPortList[];
   extern struct MessagePortType MessageOutPortList[];
   char debugbuf[GENERIC_BUF_LEN];

   int ret;

   // create butterfly thread
   ret = LicorInit();

   if(ret == 1)
     {
	sprintf(debugbuf,"elekIOcalib : Can't create LICOR Thread!\n\r");
	SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],debugbuf);

	return (INIT_MODULE_FAILED);
     };

   // success
   sprintf(debugbuf,"elekIOcalib: LICOR Thread running!\n\r");
   SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],debugbuf);

   return (INIT_MODULE_SUCCESS);
}
/* Init Butterfly */

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

   if (INIT_MODULE_SUCCESS == (ret=InitSCRCard(ptrCalibStatus)))
     {
	SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],"ElekIOcalib : init SCRCard successfull");
     }
   else
     {
	SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],"ElekIOcalib : init SCRCard failed !!");
     }

   if (INIT_MODULE_SUCCESS == (ret=InitPIDregulator(ptrCalibStatus)))
     {
	SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],"ElekIOcalib : init PID regulator successfull");
     }
   else
     {
	SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],"ElekIOcalib : init PID regulator failed !!");
     }

   if (INIT_MODULE_SUCCESS == (ret=InitLICOR(ptrCalibStatus)))
     {
	SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],"ElekIOcalib : init LICOR successfull");
     }
   else
     {
	SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],"ElekIOcalib : init LICOR regulator failed !!");
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
void GetMFCCardData ( struct calibStatusType *ptrCalibStatus)
{

   int Card;
   int Channel;
   uint16_t  MFCFlow;
   unsigned  MFC_Address;
   unsigned  DAC_Address;

   // *********************
   // Calibrator MFC Card
   // *********************
   //
   for (Card=0; Card<MAX_MFC_CARD_LIFT; Card ++)
     {
	MFC_Address=ELK_MFC_BASE+Card*ELK_MFC_NUM_ADR;
	DAC_Address=ELK_DAC_BASE+Card*ELK_DAC_NUM_ADR;
	ptrCalibStatus->MFCCardCalib[Card].NumSamples++;

	for (Channel=0;Channel<MAX_MFC_CHANNEL_PER_CARD; Channel++)
	  {
	     MFCFlow=elkReadData(MFC_Address+2*Channel);

	     ptrCalibStatus->MFCCardCalib[Card].MFCChannelData[Channel].Flow    = MFCFlow;
	     ptrCalibStatus->MFCCardCalib[Card].MFCChannelData[Channel].SumDat += MFCFlow;
	     ptrCalibStatus->MFCCardCalib[Card].MFCChannelData[Channel].SumSqr += MFCFlow*MFCFlow;

	     ptrCalibStatus->MFCCardCalib[Card].MFCChannelData[Channel].SetFlow =elkReadData(DAC_Address+2*Channel) ;

	  }
	     /* for Channel */
     }
	/* for Card */
}
/* GetMFCCardData */

/**********************************************************************************************************/
/* GetTemperatureCardData                                                                                     */
/**********************************************************************************************************/

void GetTemperatureCardData ( struct calibStatusType *ptrCalibStatus)
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

   for (Card=0; Card<MAX_TEMP_SENSOR_CARD_CALIB; Card++)
     {
	// get current control word status
	ptrCalibStatus->TempSensCardCalib[Card].Control.Word=(uint16_t)elkReadData(ELK_TEMP_CTRL);
	// we set the busy flag to get exclusive access
	ptrCalibStatus->TempSensCardCalib[Card].Control.Field.Busy=1;
	ret=elkWriteData(ELK_TEMP_CTRL, ptrCalibStatus->TempSensCardCalib[Card].Control.Word );

	// check if AVR busy
	do
	  {
	     ptrCalibStatus->TempSensCardCalib[Card].Control.Word=(uint16_t)elkReadData(ELK_TEMP_CTRL);
	  }
	while ((TimeOut++<MAX_TEMP_TIMEOUT)&&(ptrCalibStatus->TempSensCardCalib[Card].Control.Field.Update));

	if (TimeOut>MAX_TEMP_TIMEOUT)
	  {
	     if ( ((ptrCalibStatus->TempSensCardCalib[Card].NumMissed)++)>MAX_TEMP_MISSED_READING)
	       {
		  // mark Sensor Data as not valid and mark temperature as invalid
		  for (Sensor=0; Sensor<MAX_TEMP_SENSOR; Sensor++)
		    {
		       ptrCalibStatus->TempSensCardCalib[Card].TempSensor[Sensor].Field.TempFrac=0x0;
		       ptrCalibStatus->TempSensCardCalib[Card].TempSensor[Sensor].Field.TempMain=0x80;
		    }
		       /* for Sensor */
		  sprintf(buf,"GetTemp (C): Problem with Card : %d Missed Reading %d",Card, ptrCalibStatus->TempSensCardCalib[Card].NumMissed);
		  SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],buf);
	       }
	     else
	       {
		  // mark Sensor Data as not valid
		  for (Sensor=0; Sensor<MAX_TEMP_SENSOR; Sensor++)
		    ptrCalibStatus->TempSensCardCalib[Card].TempSensor[Sensor].Field.bValid=0;
	       }
	  }
	else
	  {

	     ptrCalibStatus->TempSensCardCalib[Card].NumMissed=0;              // reset Missed Reading Counter

	     // first we read the number of Sensor we have
	     ptrCalibStatus->TempSensCardCalib[Card].NumSensor=elkReadData(ELK_TEMP_FOUND);
	     ptrCalibStatus->TempSensCardCalib[Card].NumErrCRC=elkReadData(ELK_TEMP_ERR_CRC);
	     ptrCalibStatus->TempSensCardCalib[Card].NumErrNoResponse=elkReadData(ELK_TEMP_ERR_NORESPONSE);
	     // printf("found %d sensors\n",ptrCalibStatus->TempSensCardCalib[Card].NumSensor);
	     // now check each sensor
	     // we can have more sensors, but for now we use only 30
	     // they are split in two groups, 24 & 6
	     TempBaseAddr=ELK_TEMP_DATA; // first range
	     for (Sensor=0; Sensor<MAX_TEMP_SENSOR; Sensor++)
	       {
		  if (Sensor==25) TempBaseAddr=(ELK_TEMP_DATA2-(Sensor*10));
		  ptrCalibStatus->TempSensCardCalib[Card].TempSensor[Sensor].Word.WordTemp=elkReadData((Sensor*10)+TempBaseAddr);
		  //	ptrCalibStatus->TempSensCard[Card].TempSensor[Sensor].Temperatur=
		  //  ptrCalibStatus->TempSensCard[Card].TempSensor[Sensor].TempSens.TempMain+
		  //  ptrCalibStatus->TempSensCard[Card].TempSensor[Sensor].TempSens.TempFrac/16;
		  ptrCalibStatus->TempSensCardCalib[Card].TempSensor[Sensor].Word.WordID[0]=elkReadData((Sensor*10)+2+TempBaseAddr);
		  ptrCalibStatus->TempSensCardCalib[Card].TempSensor[Sensor].Word.WordID[1]=elkReadData((Sensor*10)+4+TempBaseAddr);
		  ptrCalibStatus->TempSensCardCalib[Card].TempSensor[Sensor].Word.WordID[2]=elkReadData((Sensor*10)+6+TempBaseAddr);
		  ptrCalibStatus->TempSensCardCalib[Card].TempSensor[Sensor].Word.WordLimit=elkReadData((Sensor*10)+8+TempBaseAddr);
	       }
		  /* for Sensor */

	     //      sprintf(buf,"...GetTemp: Timeout %d Sensor0 %x",TimeOut,ptrCalibStatus->TempSensCard[0].TempSensor[0].Word.WordTemp );
	     // SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],buf);
	     //
	     // we release the busy flag
	     ptrCalibStatus->TempSensCardCalib[Card].Control.Field.Busy=0;
	     ret=elkWriteData(ELK_TEMP_CTRL, ptrCalibStatus->TempSensCardCalib[Card].Control.Word );

	  }
	     /* if TimeOut */
     }
	/* for Card */
}
/* GetTemperatureCardData */

/**********************************************************************************************************/
/* GetSCRCardData                                                                                             */
/**********************************************************************************************************/

/* function to retrieve Statusinformation */
void GetSCRCardData ( struct calibStatusType *ptrCalibStatus)
{
   int iChannel = 0;

   // turn all 230VAC outputs off
   for(iChannel=0;iChannel < MAX_SCR3XB_CHANNEL_PER_CARD;iChannel++)
     {
	ptrCalibStatus->SCRCardCalib[0].SCRPowerValue[iChannel] = elkReadData(ELK_SCR_BASE + (2*iChannel));
     };
}
/* GetSCRCardData */

/**********************************************************************************************************/
/* Get PIDregulator Data                                                                                  */
/**********************************************************************************************************/

void GetPIDregulatorData (struct calibStatusType *ptrCalibStatus)
{
   extern struct MessagePortType MessageInPortList[];
   extern struct MessagePortType MessageOutPortList[];
   char           buf[GENERIC_BUF_LEN];

#define HEATERCHANNEL (2)
#define WATERCHANNEL (0)

   uint16_t ADCData = 0;
   ADCData=elkReadData(ELK_ADC_BASE+2*HEATERCHANNEL);
   /* if no card is present, don't allow heater to be on */
   if(ADCData == 19999)
     {
	sprintf(buf,"GetPIDregulatorData: temp probe HEATER failed (no ADC card ?)");
	SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],buf);

	ptrCalibStatus->PIDRegulator.Setpoint = 0;
	ptrCalibStatus->PIDRegulator.ControlValue = 0;
	return;
     };

   double dUdiff = ((double)(ADCData-10000)/1515)+2.5f;
   double dIres = (3.3f-dUdiff)/3900;
   double dRPTC = (dUdiff/dIres);
   double dTemperature = Resistance2Temperature(dRPTC)*100;

   ptrCalibStatus->PIDRegulator.ActualValueHeater = (uint16_t) dTemperature;

   ADCData=elkReadData(ELK_ADC_BASE+2*WATERCHANNEL);
   /* if no card is present, don't allow heater to be on */
   if(ADCData == 19999)
     {
	sprintf(buf,"GetPIDregulatorData: temp probe WATER failed (no ADC card ?)");
	SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],buf);

	ptrCalibStatus->PIDRegulator.Setpoint = 0;
	ptrCalibStatus->PIDRegulator.ControlValue = 0;
	return;
     };

   dUdiff = ((double)(ADCData-10000)/1515)+2.5f;
   dIres = (3.3f-dUdiff)/3900;
   dRPTC = (dUdiff/dIres);
   dTemperature = Resistance2Temperature(dRPTC)*100;
   ptrCalibStatus->PIDRegulator.ActualValueH2O = (uint16_t) dTemperature;

}
/* GetPIDregulatorData*/

/**********************************************************************************************************/
/* GetCalibStatus                                                                                          */
/**********************************************************************************************************/

/* function to retrieve Statusinformation */
void GetCalibStatus ( struct calibStatusType *ptrCalibStatus, int IsMaster)
{

   gettimeofday(&(ptrCalibStatus->TimeOfDayCalib), NULL);

   // ADC Card
   GetADCCardData(ptrCalibStatus);

   // MFC Card
   GetMFCCardData(ptrCalibStatus);

   // now get the temperature data
   GetTemperatureCardData(ptrCalibStatus);

   // get power state from SCR card
   GetSCRCardData(ptrCalibStatus);

   // get actual value for PID
   GetPIDregulatorData(ptrCalibStatus);
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
   int    numbytes;
   socklen_t addr_len;
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
   printf("This is elekIOcalib Version %3.2f (CVS: $Id: elekIOcalib.c,v 1.8 2006-11-10 17:28:08 rudolf Exp $) for ARM\n",VERSION);
   sprintf(buf, "This is elekIOcalib Version %3.2f (CVS: $Id: elekIOcalib.c,v 1.8 2006-11-10 17:28:08 rudolf Exp $) for ARM\n",VERSION);
   SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],buf);

    /* init all modules */
   InitModules(&CalibStatus);

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
   ret = timer_settime(StatusTimer_id, 0, &StatusTimer, NULL);
   if (ret < 0)
     {
	perror("timer_settime");
	return EXIT_FAILURE;
     }

   // change scheduler and set priority
   if (-1==(ret=ChangePriority()))
     {
	SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],"elekIOcalib : cannot set Priority");
     }

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

	pselect_timeout.tv_sec= UDP_SERVER_TIMEOUT;
	pselect_timeout.tv_nsec=0;

	ret=pselect(fdMax+1, &fdsSelect, NULL, NULL, &pselect_timeout, &SignalMask);             // wiat until incoming udp or Signal

	gettimeofday(&StartAction, NULL);

#ifdef DEBUG_TIMER
	printf("Time:");
	localtime_r(&StartAction.tv_sec,&tmZeit);

	printf("%02d:%02d:%02d.%03d :%d\n", tmZeit.tm_hour, tmZeit.tm_min,
	       tmZeit.tm_sec, StartAction.tv_usec/1000,TimerState);
	printf("ret %d StatusFlag %d\n",ret,StatusFlag);
#endif

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
			 {

			    // printf("get Status %6d..\r",StatusFlag);
			    write(2,"get Status.....\r",16);

			    gettimeofday(&GetStatusStartTime, NULL);
			    GetCalibStatus(&CalibStatus,IsMaster);
			    gettimeofday(&GetStatusStopTime, NULL);
			 }

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

		       // lets see whether there is still an open data request
		       if (RequestDataFlag)
			 {
			    sprintf(buf,"[ElekIOcalib] still missing %d data set BUT send flagged data",
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
		  SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],"elekIOcalib: Problem with select");
	       }
	     // if errno
	  }
	else if (ret>0)
	  {
	     //	     printf("woke up...");
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
			    //			  case ELEK_ETALON_IN:       // port for incoming commands from  etalon
			  case ELEK_SCRIPT_IN:       // port for incoming commands from  scripting host (not yet existing)

			    if ((numbytes=recvfrom(MessageInPortList[MessagePort].fdSocket,
						   &Message,sizeof(struct ElekMessageType)  , 0,
						   (struct sockaddr *)&their_addr, &addr_len)) == -1)
			      {
				 perror("recvfrom");
				 SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],"elekIOcalib: Problem with receive");
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
				 printf("elekIOcalib: FETCH_DATA received, TIME_T  was: %016lx\n\r", Message.MsgTime);
#endif

#ifdef DEBUG_SLAVECOM
				 printf("elekIOcalib: gathering Data...\n\r");
#endif
				 gettimeofday(&GetStatusStartTime, NULL);
				 GetCalibStatus(&CalibStatus,IsMaster);
				 gettimeofday(&GetStatusStopTime, NULL);
#ifdef DEBUG_SLAVECOM
				 printf("elekIOcalib: Data aquisition took: %02d.%03ds\n\r",
					GetStatusStopTime.tv_sec-GetStatusStartTime.tv_sec,
					(GetStatusStopTime.tv_usec-GetStatusStartTime.tv_usec)/1000);
#endif
				 // send this debugmessage message to debugmon
				 sprintf(buf,"elekIOcalib : FETCH_DATA from Port: %05d",
					 MessageInPortList[MessagePort].PortNumber,
					 Message.Addr,Message.Value,Message.Value);
				 SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],buf);

				 // send requested data, don't send any acknowledges
				 SendUDPData(&MessageOutPortList[ELEK_ELEKIO_CALIB_MASTER_OUT],
					     sizeof(struct calibStatusType), &CalibStatus); // send data packet
				 break;

			       case MSG_TYPE_READ_DATA:
				 // printf("elekIOcalib: manual read from Address %04x\n", Message.Addr);
				 Message.Value=elkReadData(Message.Addr);
				 Message.MsgType=MSG_TYPE_ACK;

				 if (MessagePort!=ELEK_ETALON_IN)
				   {
				      sprintf(buf,"elekIOcalib : ReadCmd from %05d Port %04x Value %d (%04x)",
					      MessageInPortList[MessagePort].PortNumber,
					      Message.Addr,Message.Value,Message.Value);
				      SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],buf);

				      sprintf(buf,"%d",MessageInPortList[MessagePort].RevMessagePort);
				      SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],buf);
				   }

				 SendUDPDataToIP(&MessageOutPortList[MessageInPortList[MessagePort].RevMessagePort],
						 inet_ntoa(their_addr.sin_addr),
						 sizeof(struct ElekMessageType), &Message);
				 break;
			       case MSG_TYPE_WRITE_DATA:
				 if (MessagePort!=ELEK_ETALON_IN)
				   {
				      sprintf(buf,"elekIOcalib : WriteCmd from %05d Port %04x Value %d (%04x)",
					      MessageInPortList[MessagePort].PortNumber,
					      Message.Addr,Message.Value,Message.Value);
				      SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],buf);
				   }

				 Message.Status=elkWriteData(Message.Addr,Message.Value);
				 Message.MsgType=MSG_TYPE_ACK;
				 SendUDPDataToIP(&MessageOutPortList[MessageInPortList[MessagePort].RevMessagePort],
						 inet_ntoa(their_addr.sin_addr),
						 sizeof(struct ElekMessageType), &Message);
				 break;

			       case MSG_TYPE_CALIB_SETTEMP:
				 if (MessagePort!=ELEK_ETALON_IN)
				   {
				      sprintf(buf,"elekIOcalib : SET_TEMP from %05d Port %04x Value %d (%04x)",
					      MessageInPortList[MessagePort].PortNumber,
					      Message.Addr,Message.Value,Message.Value);
				      SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],buf);
				   }
				 CalibStatus.PIDRegulator.Setpoint = Message.Value;
				 Message.Status = Message.Value;
				 Message.MsgType = MSG_TYPE_ACK;
				 SendUDPDataToIP(&MessageOutPortList[MessageInPortList[MessagePort].RevMessagePort],
						 inet_ntoa(their_addr.sin_addr),
						 sizeof(struct ElekMessageType), &Message);
				 break;
			      }
		       /* switch MsgType */
			    break;
			    // port to receive status data from slaves
			    //
		     /*case ELEK_STATUS_IN:
			 {
			    // Slave Mode
			    //
			    if ((numbytes=recvfrom(MessageInPortList[MessagePort].fdSocket,
						   &CalibStatusFromSlave,sizeof(CalibStatusFromSlave) , 0,
						   (struct sockaddr *)&their_addr, &addr_len)) == -1)
			      // Something went wrong
			      {
				 perror("elekIOcalib: recvfrom ELEK_STATUS_IN");
				 SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],"elekIOcalib : ELEK_STATUS_IN: problem with receive");
			      }
			    else
			      // we should have received some data at this point, so give a lifesign
			      {
				 //printf("elekIOcalib(S) :ELEK_STATUS_IN: got %d bytes from Master!\n\r",numbytes);
			      };
			 };

		       break;
*/
			  default:
			    SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],"elekIOcalib: unknown Port Type");
			    break;

			 }
		       // switch MessagePort
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
	     SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],"elekIOcalib : TimeOut");
	  }

#ifdef RUNONPC
	if (timer_getoverrun(StatusTimer_id)>0)
	  {
	     printf("OVERRUN\n\r");
	     SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],"elekIOcalib : Overrun");
	  }
   /* if overrun */
#endif

	gettimeofday(&StopAction, NULL);

#ifdef DEBUG_TIME_TASK
	sprintf(buf,"elekIOcalib: %ld RT: %ld DT: %ld",StartAction.tv_usec/1000,
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

