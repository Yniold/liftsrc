/* $RCSfile: elekIO.h,v $ header file for elekIO
*
* $RCSfile: elekIO.h,v $ last edit on $Date: 2007-06-11 17:09:53 $ by $Author: rudolf $
*
* $Log: elekIO.h,v $
* Revision 1.49  2007-06-11 17:09:53  rudolf
* more fixes...
*
* Revision 1.48  2007/06/11 17:06:18  rudolf
* changes for steppercard etalon
*
* Revision 1.47  2007/06/11 15:46:20  rudolf
* Changed not properly specified bitfields to uint16_t otherwise Win32 compiler will treat them as 32bit ints which will lead to structure size mismatch. Unix part untested, must be tested on LIFT asap
*
* Revision 1.46  2007-03-20 21:00:48  martinez
* MovingFlags is unsigned
*
* Revision 1.45  2007-03-20 20:52:05  martinez
* included typedefs for  not Unix instead of stdint.h
*
* Revision 1.44  2007-03-20 07:54:34  martinez
* different delta_positions for different mirrors, realign only second green and first UV mirror
*
* Revision 1.43  2007-03-08 21:07:03  harder
* enabled automirror
*
* Revision 1.42  2007-03-07 19:16:54  rudolf
* added missing field for Licor Pressure
*
* Revision 1.41  2007-03-07 16:05:31  rudolf
* added structures for elekIOaux
*
* Revision 1.40  2007-03-05 16:06:58  martinez
* mirror moving implemented
*
* Revision 1.39  2007-02-21 16:05:35  rudolf
* fixed typo, delta H2O and CO2 for licor is signed now
*
* Revision 1.38  2007-02-20 19:28:35  rudolf
* removed mysterious A
*
* Revision 1.37  2007-02-20 19:26:37  harder
* *** empty log message ***
*
* Revision 1.36  2007-02-20 19:23:14  harder
* bug fix
*
* Revision 1.35  2007-02-20 18:51:43  harder
* update calib constants
*
* Revision 1.34  2007-02-19 21:52:27  harder
* included MFCConfig struct
*
* Revision 1.33  2007-02-13 20:40:50  harder
* disabled DEBUG_NOHARDWARE
*
* Revision 1.32  2006-11-03 15:40:52  rudolf
* added Heater Temp to struct
*
* Revision 1.31  2006/11/02 16:29:00  rudolf
* added PID structure
*
* Revision 1.30  2006/11/02 14:30:51  rudolf
* added missing define
*
* Revision 1.29  2006/11/02 14:04:07  rudolf
* extended calib structure to hold also current heater power from SCR card
*
* Revision 1.28  2006/10/15 11:25:43  harder
* fixed BitPos of R&L end sw.
*
* Revision 1.27  2006/10/15 08:55:19  harder
* ref channel can be now assigned to any counter channel
* eCmd: new command 'refchannel'
* elekIOServ : used etalon Status info to store channel info
* elekIO.h modified etalon structure in status
*
* Revision 1.26  2006/10/09 12:16:27  rudolf
* fixed c++ style comments for MEX compiler
*
* Revision 1.25  2006/10/06 13:36:44  rudolf
* added define for switching off timeout when no backplane is physically present
*
* Revision 1.24  2006/10/05 15:32:40  rudolf
* extenden slave structure with butterfly data
*
* Revision 1.23  2006/08/31 13:52:07  rudolf
* added elekIOcalib to TL makefile, made it at least compile properly, work in progress
*
* Revision 1.22  2006/08/30 15:06:40  rudolf
* startet work on calibrator structures
*
* Revision 1.21  2006/08/07 11:50:16  martinez
* corrected syntax errors, removed option "RECAL" from etalon Actions
*
* Revision 1.20  2006/08/04 17:41:05  martinez
* related all etalon positions to encoder position;
* homing etalon sets encoder position to 0 at left end switch in etalon.c;
* homing is done only in horusStart, home etalon in Dyelaser.m only moves etalon to 0 position;
* included online find in etalon.c and eCmd.c, replaced in Dyelaser.m
*
* Revision 1.19  2006/08/03 15:38:52  martinez
* define ETALON_DEFAULT_ACCSPD as 0x2020 in elekIO.h and use it in etalon.c and elekIOServ.c
*
* Revision 1.18  2005/09/20 21:29:51  harder
* fixed order of temp card status words
*
* Revision 1.17  2005/09/20 14:36:53  harder
* removed ADC24, increased num of ADC in WP to 3
*
* Revision 1.16  2005/09/19 22:16:24  harder
* extended Temperature Card sensors to 40, added info for second bank and changed addresses of temp card status words
*
* Revision 1.15  2005/09/18 22:44:49  martinez
* switch ARM LED
*
* Revision 1.14  2005/07/23 09:00:29  rudolf
* added etalonditheronline command
*
* Revision 1.13  2005/06/28 15:34:12  harder
* added sep. adr. const. for MFC @ WP
*
* Revision 1.12  2005/06/28 13:55:31  rudolf
* included MFCSetFlow
*
* Revision 1.11  2005/06/27 10:55:08  rudolf
* fixed comment, has to be in elekIO for matlab (HH)
*
* Revision 1.10  2005/06/27 09:43:56  rudolf
* added uiValidSlaveDataFlag
*
* Revision 1.9  2005/06/26 19:41:51  harder
* added different Valve PWM address for Slave
*
* Revision 1.8  2005/06/08 17:45:55  rudolf
* merged file
*
* Revision 1.6  2005/05/23 15:06:33  rudolf
* changed some #defines for ARM
*
* Revision 1.5  2005/05/22 19:10:24  rudolf
* extended structure to hold data from the wingpod ARM9 as well
*
* Revision 1.4  2005/02/11 12:36:12  martinez
* started including gatings in CounterCards, started including instrument action structure in elekIO.h and elekIOServ.c
*
* Revision 1.3  2005/01/27 15:48:10  rudolf
* Added header field for proper CVS version history
*
*
* 24.05.2004    HH   added ADC Card structure
*
*
*
*/
/* #define DEBUG_NOHARDWARE 0 */

#ifndef ELEKIO_H
#define ELEKIO_H

//#include <stdint.h>
#include <sys/types.h>

#ifndef uint16_t
#define uint16_t unsigned short
#endif

#ifndef int16_t
#define int16_t short
#endif

#define INIT_MODULE_FAILED  0
#define INIT_MODULE_SUCCESS 1

#define MAX_ADC_CARD_LIFT              2       /* number of 16bit ADC Cards in Lift */
#define MAX_ADC_CARD_WP                3       /* number of 16bit ADC Cards in Wingpod */
#define MAX_ADC_CARD_CALIB             1       /* number of 16bit ADC Cards in Calibrator */
#define MAX_ADC_CHANNEL_PER_CARD       8       /* number of Channels on each 16bit ADC Card */

#define MAX_24BIT_ADC_CARDS_LIFT       0       /* number of 24bit ADC Cards in Lift */
#define MAX_24BIT_ADC_CARDS_WP         1       /* number of 24bit ADC Cards in Wingpod */
#define MAX_24BIT_ADC_CHANNEL_PER_CARD 8       /* number of Channels on each 24bit ADC card */

#define MAX_MFC_CARD_LIFT              1       /* number of MFC Cards in Lift */
#define MAX_MFC_CARD_WP                1       /* number of MFC Cards in Wingpod */
#define MAX_MFC_CARD_CALIB             1       /* number of MFC Cards in Calibrator */
#define MAX_MFC_CHANNEL_PER_CARD       4       /* number of Channels on each MFC Card */

#define MAX_VALVE_CARD_LIFT            2       /* number of Valve Cards in Lift */
#define MAX_VALVE_CARD_WP              2       /* number of Valve Cards in WP */
#define MAX_VALVE_CHANNEL_PER_CARD     14      /* number of Valves on Card */

#define MAX_DCDC4_CARD_LIFT            1       /* number of DCDC4 in Lift */
#define MAX_DCDC4_CHANNEL_PER_CARD     4       /* number of Channels on DCDC4 */

#define MAX_SCR3XB_CALIB               1       /* number of SCR3XB Cards in Calibrator (for AC power devices) */
#define MAX_SCR3XB_CHANNEL_PER_CARD    3       /* number of Channels on SCR3XB*/

#define MAX_LICORS_CALIB               1       /* number of LICORs on Calibrator Unit */


#ifndef DEBUG_NOHARDWARE
#define ELK_TIMEOUT (unsigned) 0x1000
#else
#define ELK_TIMEOUT (unsigned) 0x0001
#endif

#define ELK_BASE (uint16_t)0x200
#define ELK_ADR  ELK_BASE
#define ELK_DATA (ELK_BASE + (uint16_t) 2)
#define ELK_TODO (ELK_BASE + (uint16_t) 4)
#define ELK_QSIZE 4

#define ELK_BACKPLANE_BASE   0xa400

#define ELK_STEP_BASE        0xa500
#define ELK_STEP_SETPOS      (ELK_STEP_BASE + 0x0010)
#define ELK_STEP_GETPOS      (ELK_STEP_BASE + 0x0008)
#define ELK_STEP_SETSPD      (ELK_STEP_BASE + 0x0014) /* velo=0x14 acc=0x15 */
#define ELK_STEP_GETSPD      (ELK_STEP_BASE + 0x000c)
#define ELK_STEP_GETENCPOS   (ELK_STEP_BASE + 0x0020)
#define ELK_STEP_GETINDPOS   (ELK_STEP_BASE + 0x0024)
#define ELK_STEP_SETVOLT     (ELK_STEP_BASE + 0x0016) /* zero=x16 slow=x17 */
#define ELK_STEP_STATUS      (ELK_STEP_BASE + 0x001e) /* switch is bit 0&1 */
#define ELK_STEP_MODE        (ELK_STEP_BASE + 0x001c) /* mode, 4=copy encoder to index with next index */


#define ELK_DACPWM_BASE      (ELK_BACKPLANE_BASE+0x04)
#define ELK_DACPWM_BASE_WP   (ELK_BACKPLANE_BASE+0x04)
#define ELK_VALVE_BASE       (ELK_BACKPLANE_BASE+0x08)
#define ELK_DAC_BASE         (ELK_BACKPLANE_BASE+0x40)
#define ELK_DAC_BASE_WP      (ELK_BACKPLANE_BASE+0x40)
#define ELK_DAC_BASE_CALIB   (ELK_BACKPLANE_BASE+0x40)
#define ELK_PWM_BASE         (ELK_BACKPLANE_BASE+0x60)
#define ELK_PWM_DCDC4_BASE   (ELK_PWM_BASE)
#define ELK_PWM_VALVE_BASE   (ELK_PWM_BASE+0x08)


#define ELK_PWM_VALVE_BASE_WP (0xa460)

#define ELK_ADC_BASE         (ELK_BACKPLANE_BASE+0x80)
#define ELK_ADC_BASE_WP      (ELK_BACKPLANE_BASE+0x80)

#define ELK_MFC_BASE         (ELK_BACKPLANE_BASE+0xa0)          /* Base adr. of MFC ADC channels */ 
#define ELK_MFC_BASE_WP      (ELK_BACKPLANE_BASE+0xe0)
#define ELK_MFC_BASE_CALIB   (ELK_BACKPLANE_BASE+0xa0)


#define ELK_ADC_CONFIG          (0x0010)                        /* add to base addr */
#define ELK_ADC_NUM_ADR         (0x0020)                        /* number of addresses each ADC channel has */

#define ELK_MFC_CONFIG          (ELK_ADC_CONFIG)                /* offs. base addr for MFC Config*/
#define ELK_MFC_NUM_ADR         (ELK_ADC_NUM_ADR)               /* number of addresses each MFC channel has */
#define ELK_DAC_NUM_ADR         (MAX_DCDC4_CHANNEL_PER_CARD<<1) /* number of addresses each MFC channel has */

#define ELK_MFC_CONFIG_WP       (ELK_ADC_CONFIG)                /* offs. base addr for MFC Config*/
#define ELK_MFC_NUM_ADR_WP      (ELK_ADC_NUM_ADR)               /* number of addresses each MFC channel has */
#define ELK_DAC_NUM_ADR_WP      (MAX_DCDC4_CHANNEL_PER_CARD<<1) /* number of addresses each MFC channel has */

#define ELK_MFC_CONFIG_CALIB    (ELK_ADC_CONFIG)                /* offs. base addr for MFC Config*/
#define ELK_MFC_NUM_ADR_CALIB   (ELK_ADC_NUM_ADR)               /* number of addresses each MFC channel has */
#define ELK_DAC_NUM_ADR_CALIB   (MAX_DCDC4_CHANNEL_PER_CARD<<1) /* number of addresses each MFC channel has */

/* defines for the 24bit ADC */

#define ELK_ADC_24_BASE        0xa500                           /* Base Address of 24bit ADC */
#define ELK_ADC_24_POWER       (ELK_ADC_24_BASE + 0x0010)       /* Power register for the DCDC */
#define ELK_ADC_24_DATA        (ELK_ADC_24_BASE + 0x0020)       /* Channel Data registers */

/* define for SCR3 in calibrator unit */
#define ELK_SCR_BASE           (ELK_BACKPLANE_BASE+0x10)        /* SCR Power card */


/*#define ELK_VAL2_BASE    (ELK_BACKPLANE_BASE+0x72) */


#define ELK_STAT_TIMEOUT 0
#define ELK_STAT_OK      1

/*************************************************************************************************************/

#define ELK_COUNTER_BASE        0xa300
#define ELK_COUNTER_ADC         (ELK_COUNTER_BASE)
#define ELK_COUNTER_DELAY_SHIFT (ELK_COUNTER_BASE + 0x0010)
#define ELK_COUNTER_DELAY_GATE  (ELK_COUNTER_BASE + 0x0018)
#define ELK_COUNTER_STATUS      (ELK_COUNTER_BASE + 0x0020)
#define ELK_COUNTER_MASK        (ELK_COUNTER_BASE + 0x0030)
#define ELK_COUNTER_SUMCOUNTS   (ELK_COUNTER_BASE + 0x0038)
#define ELK_COUNTER_COUNTS      (ELK_COUNTER_BASE + 0x0080)

#define ELK_COUNTER_STATUS_FLIP 0x8000
#define ELK_COUNTER_STATUS_BUSY 0x0200

#define MAX_COUNTER_CHANNEL        3    /* number of channels per Card */
#define MAX_COUNTER_GATE           2    /* number of Gate Channel      */
#define MAX_COUNTER_PAGE           3    /* number of pages per Channel */
#define MAX_COUNTER_TIMESLOT       (MAX_COUNTER_SLOTS_PER_PAGE*MAX_COUNTER_PAGE)  /* number of timeslots for each channel */
#define MAX_COUNTER_SLOTS_PER_PAGE 64   /* number of timeslots on each page */


#define ADC_CHANNEL_COUNTER_CARD 8
#define COUNTER_MASK_WIDTH       12     /* number of words for the mask reg. (12x16=192=160+32) */

struct ChannelListType {              /* generic Type for data channels */
    uint16_t Addr;
    uint16_t Value;
}; /* ChannelListType */

struct CounterChannelDataType {
    uint16_t  ShiftDelay;              /* delay after Masterdelay until shift register starts shifting */
    uint16_t  GateDelay;               /* delay after Masterdelay until GainGate is activated */
    uint16_t  GateWidth;               /* time width while GainGate is on */
    uint16_t  Data[MAX_COUNTER_TIMESLOT]; /* Shift register in which incoming counts are stored and summed */
    uint16_t  Mask[COUNTER_MASK_WIDTH];/* Masks interesting counts from data */
    uint16_t  Counts;                  /* sum of all Data counts masked by Mask */
    uint16_t  Pulses;                  /* number of pulses accumulated in Data */
}; /* ChannleDataType */

struct CounterCardType {
    uint16_t  ADCData[ADC_CHANNEL_COUNTER_CARD];  /* PMTthresh, LaserTrigThresh, MCP1Thresh,  */
                                       /* MCP2Thresh, +5, +28 +3.3 +1.5 */
    uint16_t  MasterDelay;
    struct CounterChannelDataType Channel[MAX_COUNTER_CHANNEL];
}; /* CounterCardType */

/*************************************************************************************************************/

enum EtalonActionType { /* update also in etalon.c */
    ETALON_ACTION_TOGGLE_ONLINE_LEFT,          /* etalon is on the left ONLINE Position */
    ETALON_ACTION_TOGGLE_ONLINE_RIGHT,         /* etalon is on the right ONLINE Position */
    ETALON_ACTION_TOGGLE,                      /* etalon is toggling between on and offline */
    ETALON_ACTION_TOGGLE_OFFLINE_LEFT,         /* etalon is on the left OFFLINE Position */
    ETALON_ACTION_TOGGLE_OFFLINE_RIGHT,        /* etalon is on the right OFFLINE Position */
    ETALON_ACTION_NOP,                         /* etalon is doing no automated operation */
    ETALON_ACTION_DITHER_ONLINE,               /* stay online and dither */
    ETALON_ACTION_DITHER_ONLINE_LEFT,          /* stay online and dither left side */
    ETALON_ACTION_DITHER_ONLINE_RIGHT,         /* stay online and dither right side */
    ETALON_ACTION_SCAN,                        /* etalon is scanning */
    ETALON_ACTION_HOME,                        /* etalon is on a home run */
    ETALON_ACTION_FIND_ONLINE,		       /* etalon sets ONLINE Position to largest recent ref. signal */

    ETALON_ACTION_MAX
};

#define ETALON_STEPRATIO_ENCODER_MOTOR    2
#define ETALON_SCAN_POS_START      5
#define ETALON_SCAN_POS_STOP       20000
#define ETALON_SCAN_STEP_WIDTH     8

#define ETALON_STEP_POS_ONLINE     500
#define ETALON_STEP_DITHER         16
#define ETALON_STEP_OFFLINE        64

#define ETALON_STEP_ESW_RIGHT      8       /* 0x0100 Bit position of right end switch in status word of stepper card */
#define ETALON_STEP_ESW_LEFT       9       /* 0x0200 position of left end switch in status word of stepper card */

#define ETALON_CHANNEL_REF_CELL    0           /* Reference Cell Channel Number for etalon etc.. */


#define ETALON_DEFAULT_ACCSPD 0x2020

#define LIFE_REFSIGNAL 300

struct LongWordType {
  uint16_t Low;
  uint16_t High;
}; /* struct LongWordType */
 

union PositionType {
  struct LongWordType PositionWord;
  uint32_t Position;
}; /* union PositionType */

struct StatusFieldType {                                            /* Bit Field */
  uint16_t RefChannel:2;                                       /* Channel number of Reference Cell */
  uint16_t Unused1:6;                                          /* */
  uint16_t EndswitchRight:1;                                   /* Endschalter rechts aktiv */
  uint16_t EndswitchLeft:1;                                    /* Endschalter links aktiv */
  uint16_t Unused2:6;                                          /*  */
};

union StatusType {
  struct StatusFieldType StatusField;
  uint16_t StatusWord;                    /* Status, end switch, index      */
};

struct EtalonDataType {

  union PositionType Set;
  union PositionType Current;
  union PositionType Encoder;
  union PositionType Index;
  union PositionType Online;
  union PositionType ScanStart;
  union PositionType ScanStop;
  uint16_t           ScanStepWidth;
  uint16_t           DitherStepWidth;
  uint16_t           OfflineStepLeft;
  uint16_t           OfflineStepRight;

  uint16_t CurSpeed;               /* Current Speed */
  uint16_t SetSpeed;
  uint16_t SetAccl;
  union StatusType Status;		/* Status, end switch, index      */

}; /* EtalonDataType */

/*************************************************************************************************************/


struct ADCChannelConfigBitType {
    uint16_t MuxChannel:3;
    uint16_t Bridge:1;
    uint16_t Gain:2;
    uint16_t Offset:2;
    uint16_t Unused:8;
}; /* ADCChannelConfigBitType */

union ADCChannelConfigType {
    struct ADCChannelConfigBitType ADCChannelConfigBit;
    uint16_t ADCChannelConfig;
}; /* ADCConfigType */
    
struct ADCChannelDataType {
    uint16_t ADCData;
    unsigned SumDat;
    unsigned SumSqr;
}; /* ADCChannelType */

struct ADCChannel24DWType {
    uint16_t ADCDataLow;
    uint16_t ADCDataHigh;
};

union ADCChannel24DataType {
    struct ADCChannel24DWType ADCChannelDataLowHigh;
	 int32_t ADCChannelData;
}; /* ADCChannelType */

struct ADCCardType {                                               
  uint16_t NumSamples;                                                    /* number of Samples for statistik */
  struct ADCChannelDataType ADCChannelData[MAX_ADC_CHANNEL_PER_CARD];
  union  ADCChannelConfigType ADCChannelConfig[MAX_ADC_CHANNEL_PER_CARD];
}; /* ADCCardType */


#define DIODE_UV_ADCCARDMASTER_NUMBER 0
#define DIODE_UV_ADCCARDMASTER_CHANNEL 2
#define DIODE_WZ1IN_ADCCARDSLAVE_NUMBER 1
#define DIODE_WZ1IN_ADCCARDSLAVE_CHANNEL 7
#define DIODE_WZ2IN_ADCCARDSLAVE_NUMBER 0
#define DIODE_WZ2IN_ADCCARDSLAVE_CHANNEL 2
/*************************************************************************************************************/

struct ADC24CardType {                                               
  uint16_t NumSamples;                                                    /* number of Samples for statistik */
  union ADCChannel24DataType ADCChannelData[MAX_24BIT_ADC_CHANNEL_PER_CARD];
 }; /* ADCCardType */

/*************************************************************************************************************/


struct MFCChannelConfigBitType {
  uint16_t MuxChannel:3;
  uint16_t Unused:5;
  uint16_t Ch0:2;
  uint16_t Ch1:2;
  uint16_t Ch2:2;
  uint16_t Ch3:2;
}; /* MFCChannelConfigBitType */

union MFCChannelConfigType {
    struct MFCChannelConfigBitType MFCChannelConfigBit;
    uint16_t MFCChannelConfig;
}; /* MFCChannelConfigType */
    
struct MFCChannelDataType {
    uint16_t SetFlow;
    uint16_t Flow;
    unsigned SumDat;
    unsigned SumSqr;
}; /* MFCChannelType */

struct MFCCardType {                                                      
  uint16_t NumSamples;                                                    /* number of Samples for statistik */
  struct MFCChannelDataType MFCChannelData[MAX_MFC_CHANNEL_PER_CARD];
  union  MFCChannelConfigType MFCChannelConfig[MAX_MFC_CHANNEL_PER_CARD];
}; /* MFCCardType */

struct SCRCardType {
  uint16_t SCRPowerValue[MAX_SCR3XB_CHANNEL_PER_CARD];
}; /* SCRCardType */

struct MFCConfigType {             /* this configuration data is supposed to be in a nice XML file, it is not stored in Status */
    uint64_t MaxFlow;              /* manufacturer stated max flow in SCCM */
    double   SetSlope;             /* Set point slope Flow/counts */
    double   SetOffset;            /* Set point offset in SCCM */
    double   MeasSlope;            /* Measured flow slope SCCM/counts */
    double   MeasOffset;           /* Measured flow offset in SCCM */   
} ; /* MFCConfigType */

/*************************************************************************************************************/

struct ValveCardType {
	uint16_t    ValveVolt;                                       /* voltage applied for Card */
	uint16_t    Valve;                                           /* each bit reprsensts one valve */
};
/*************************************************************************************************************/


struct DCDC4CardType {
  uint16_t    Channel[MAX_DCDC4_CHANNEL_PER_CARD];                         /* voltage applied for Channel */	
};


/*************************************************************************************************************/
#define MAX_TEMP_SENSOR   40
#define MAX_TEMP_SENSOR_CARD_LIFT 1
#define MAX_TEMP_SENSOR_CARD_WP 1
#define MAX_TEMP_SENSOR_CARD_CALIB 1
#define MAX_TEMP_TIMEOUT  100                                        /* maximum Timeout to wait for Temperature Card to be ready */
#define MAX_TEMP_MISSED_READING 5                                    /* number of maximal reading failures before removing a temperature */
#define ELK_TEMP_BASE           0xb000                               /* Base of Temperature Card */
#define ELK_TEMP_CTRL           (ELK_TEMP_BASE+0x000)                /* Controlword */
#define ELK_TEMP_ERR_CRC        (ELK_TEMP_BASE+0x002)                /* Number of CRC Errors */
#define ELK_TEMP_ERR_NORESPONSE (ELK_TEMP_BASE+0x004)                /* Number of No Response Errors */
#define ELK_TEMP_FOUND          (ELK_TEMP_BASE+0x006)                /* Number of Sensors found */
#define ELK_TEMP_DATA           (ELK_TEMP_BASE+0x008)                /* here the data field starts */
#define ELK_TEMP_DATA2          (ELK_TEMP_BASE + 0x200)              /* offset to next bank */

#define ELK_TEMP_BUSYFLAG        0x0001                               /* we Request access */
#define ELK_TEMP_UPDATEFLAG      0x0002                               /* AVR is busy       */
#define ELK_TEMP_SCANFLAG        0x0004                               /* AVR is scanning for         */

struct TempSensorFieldType {                                            /* Bit Field */
  uint16_t TempFrac:4;                                             /* Temperatur Fraction 1/16 K */
  int16_t  TempMain:8;                                             /* Temperatur K --> Ges Temp=TempMain+TempFrac/16 */ 
  uint16_t bValid:1;                                               /* Temperatur Daten Gueltig=1 */
  uint16_t bCRCError:1;                                            /* CRC Fehler, Pruefsumme falsch */
  uint16_t bNoResponse:1;                                          /* Sensor antowrtet nicht */
  uint16_t bAlarmFlag:1;                                           /* Sensor meldet Alarm */
  unsigned char aROMCode[6];                                            /* 6 Byte ID vom Sensor */
  signed char cTempLimitMax;                                           
  signed char cTempLimitMin;
};

struct TempSensorWordType {
  uint16_t WordTemp;
  uint16_t WordID[3];
  uint16_t WordLimit;
}; /*  TempSensorWordType */


union TempSensorDataType {    
  struct TempSensorFieldType Field;
  struct TempSensorWordType Word;
}; /* TempSensorDataType  */


struct TempSensorControlFieldType {
  uint16_t Busy:1;
  uint16_t Update:1;
  uint16_t Scan:1;
  uint16_t Unused:12;
  uint16_t Reset:1;
};

union TempSensorControlType {
  struct TempSensorControlFieldType Field;
  uint16_t Word;
}; 

struct TempSensorCardType {

  union TempSensorDataType    TempSensor[MAX_TEMP_SENSOR];
  union TempSensorControlType Control;
  uint16_t                    NumMissed;
  uint16_t                    NumSensor;
  uint16_t                    NumErrCRC;
  uint16_t                    NumErrNoResponse;

}; /* TempSensorCardType */

/*************************************************************************************************************/
/* LICOR stuff */
/*************************************************************************************************************/

struct LicorStatusFieldType
{
   uint16_t Reserved:13;    /* not used up to now */
   uint16_t Found:1;        /* LICOR is recognized by calibrator task */
   uint16_t Initialising:1;  /* LICOR is warming up */
   uint16_t DataValid:1;     /* LICOR data are valid */
};

union LicorStatusType
{
   struct LicorStatusFieldType Field;
   uint16_t Word;
};

struct LicorH2OCO2Type 
{
   union LicorStatusType      Status;
   uint16_t                   LicorTemperature; /* Unit: degree kelvin * 100 e.g. 20 degree celsius -> 273,15 + 20,0 => 29315 */
   uint16_t                   AmbientPressure;  /* Unit: kPA * 100 e.g. 1002.7 mBar => 10027 */

   uint16_t                   CO2A;             /* CO2 concentration cell A in mymol/mol, coding scheme T.B.D. */
   uint16_t                   CO2B;             /* CO2 concentration cell B in mymol/mol, coding scheme T.B.D. */
   int16_t                    CO2D;             /* CO2 differential concentration in mymol/mol, coding scheme T.B.D. */

   uint16_t                   H2OA;             /* H2O concentration cell A in mmol/mol, coding scheme T.B.D. */
   uint16_t                   H2OB;             /* H2O concentration cell B in mmol/mol, coding scheme T.B.D. */
   int16_t                    H2OD;             /* H2O differential concentration in mmol/mol, coding scheme T.B.D. */
};

struct PIDregulatorType
{
   uint16_t Setpoint;          /* Kelvin x 100, e.g. 20°C => 27315 + (20 x 100) = 29315 */
   uint16_t ActualValueH2O;    /* Actual Value converted from thermistor Water */
   uint16_t ActualValueHeater; /* Actual Value converted from thermistor Heater Catridge*/
   uint16_t KP;                /* proportional coefficient */
   uint16_t KI;                /* intergrating coefficient */
   uint16_t KD;                /* differtial coefficient */
   uint16_t ControlValue;      /* regulator output */
};

/*************************************************************************************************************/
/* BUTTERFLY  */
/*************************************************************************************************************/

/* the stepperdriver on the butterfly tells us some parameters*/
/* might be interesting for debugging */

struct ButterflyMotorStatusFieldType
{
   uint16_t OCA:1;      /* overcurrent phase A */
   uint16_t OCB:1;      /* overcurrent phase B */
   uint16_t OLA:1;      /* open load bridge A */
   uint16_t OLB:1;      /* open load bridge B */
   uint16_t OCHS:1;     /* overcurrent highside */
   uint16_t UV:1;       /* undervoltage */
   uint16_t OTPW:1;     /* overtemperature prewarning */
   uint16_t OT:1;       /* overtemperature */
   uint16_t ONE:1;      /* always "1" */
   uint16_t LOAD:3;     /* load indicator */
   uint16_t unused:4;	/* not used (probably 0) */
};

union ButterflyMotorStatusType {
  struct ButterflyMotorStatusFieldType StatusField;
  uint16_t ButterflyStatusWord;
};

/* the AVR on the butterfly tells us the cause of the last RESET */
/* might be interesting for debugging */

struct ButterflyCPUStatusFieldType
{
   uint16_t UNUSED:11;   /* not used, 0 */
   uint16_t JTRF:1;      /* JTAG reset (from debugger)*/
   uint16_t WDRF:1;      /* watchdog reset */
   uint16_t BORF:1;      /* brown out reset flag */
   uint16_t EXTRF:1;     /* external reset flag (normal RC-reset)*/
   uint16_t PORF:1;      /* Power on reset flag */
};

union ButterflyCPUStatusType {
  struct ButterflyCPUStatusFieldType StatusField;
  uint16_t ButterflyCPUWord;
};

/* The butterfly has positions from 0 to 2499, only 0-624 make sense (90°) 
The valve is closed when position is valid and zero,
and fully opened when position is valid and 624.

The AVR controller on the butterfly sends back the target position
it has understood, this number is reflected in the TargetPositionGot field

When the elekIOServ receives a Set Position command it writes the required
position into the TargetPositionSet field, from where it is read by
the serial communication thread in butterfly.c and sent via RS232 to the
AVR

Normally both fields should be identical after about 500ms, otherwise this
might indicate a communication problem */

struct ButterflyType {
  uint16_t                          PositionValid;        /* tells us if the encoder has seen the index hole once */
  uint16_t                          CurrentPosition;      /* tells us where the butterfly stands */
  uint16_t                          TargetPositionGot;    /* the target position the AVR should go to (readback from AVR)*/
  uint16_t                          TargetPositionSet;    /* the target position desired */
  union ButterflyMotorStatusType    MotorStatus;          /* motor status bitfield */
  union ButterflyCPUStatusType      CPUStatus;            /* CPU status */ 
}; /* ButterflyType */

/*************************************************************************************************************/
/* AUX & METEO stuff */
/*************************************************************************************************************/

struct AuxStatusFieldType
{
   uint16_t Reserved:9;              /* not used up to now */
   uint16_t LicorPressureDataValid:1;/* Pressure from Calibrator is valid */
   uint16_t ShipWaterDataValid:1;    /* Ships Water data are valid*/
   uint16_t ShipGyroDataValid:1;     /* Ships Gyro data are valid*/
   uint16_t ShipSonarDataValid:1;    /* Ships Sonar data are valid*/
   uint16_t ShipMeteoDataValid:1;    /* Ships Meteo data are valid*/
   uint16_t ShipGPSDataValid:1;      /* Ships GPS data are valid*/
   uint16_t MeteoBoxDataValid:1;     /* Meteobox data are valid */
};

union AuxStatusType
{
   struct AuxStatusFieldType Field;
   uint16_t Word;
};

struct AuxDataValidType 
{
   union AuxStatusType      Status;
};

struct MeteoBoxType
{
   double   dWindSpeed;               /* Windspeed in m/s */
   uint16_t uiWindDirection;          /* 45° resolution */
   double   dRelHum;                  /* 000.0 - 100.0 % */
   double   dAirTemp;                 /* Temperature in degree celsius */
   double   dGasSensorVoltage;        /* dirt sensor */
};

struct LicorPressureType
{
   uint16_t uiAmbientPressure;        /* Unit: kPA * 100 e.g. 1002.7 mBar => 10027 */
};                                    /* field for elekIOServ to copy Licor pressure data */

struct ShipSonarType
{
   double   dFrequency;               /* Frequency used for sonar */
   double   dWaterDepth;
};

struct ShipMeteoType
{
   double   dWindSpeed;                /* wind speed */
   double   dWindDirection;            /* wind direction */ 
};

struct ShipGyroType
{
   double   dDirection;                /* direction */ 
};

struct ShipWaterType
{
   double   dSalinity;                /* gramms per litre */
   double   dWaterTemp;               /* water temp in degrees celsius */
};

/*************************************************************************************************************/

enum InstrumentActionType { /* update also in instrument.c */
    INSTRUMENT_ACTION_NOP,			
	INSTRUMENT_ACTION_MEASURE,
    INSTRUMENT_ACTION_CAL,   
    INSTRUMENT_ACTION_DIAG,  
    INSTRUMENT_ACTION_POWERUP, 
    INSTRUMENT_ACTION_POWERDOWN, 

    INSTRUMENT_ACTION_MAX
};



struct InstrumentFlagsType {                      /* set of flags for the instrument Server status */
  uint16_t  StatusSave:1;                       /* indicates if Status should be saved to disk */
  uint16_t  StatusQuery:1;                      /* indicates if Status should be Queried from elekIOServ */
  enum EtalonActionType EtalonAction;           /* indicates what the etalon is doing */
  enum InstrumentActionType InstrumentAction;		/* indicates what the instrument is doing (measuring, calibrating, etc.) */	
  /*  enum DebugType        Debug;                     */ 
}; /* ServerFlagsType */

/* structure for TSC for time difference measurements Master <-> Slave */

struct TSCType 
{
  uint16_t  TSCWord0;
  uint16_t  TSCWord1;
  uint16_t  TSCWord2;
  uint16_t  TSCWord3;
}; /* TSCStruct */

union TSCUn 
{
    struct TSCType TSCSplit;
	 uint64_t TSCValue;
};
	 
struct TSCDataType
{
	union TSCUn TSCReceived;
	union TSCUn TSCProcessed;
	};
	

/*************************************************************************************************************/

struct GPSDataType {					/* data type for GPS data*/
	unsigned char ucUTCHours;			/* binary, not BCD coded (!) 0 - 23 decimal*/
	unsigned char ucUTCMins;			/* binary, 0-59 decimal */
	unsigned char ucUTCSeconds;			/* binary 0-59 decimal */

	double dLongitude;					/* "Laengengrad" I always mix it up...
										signed notation,
										negative values mean "W - west of Greenwich"
										positive values mean "E - east of Greenwich" */

	double dLatitude;					/* "Breitengrad" I always mix it up...
										signed notation,
										negative values mean "S - south of the equator"
										positive values mean "N - north of the equator */
	float fAltitude;					/* altitude above the geoid in metres */
	float fHDOP;						/* Horizontal Dillution Of Precision, whatever it means....*/
	unsigned char ucNumberOfSatellites; /* number of satellites seen by the GPS receiver */
	unsigned char ucLastValidData;		/* number of data aquisitions (5Hz) with no valid GPS data
										will stick at 255 if no data received for a long period */
	uint16_t uiGroundSpeed;				/* speed in cm/s above ground */
	uint16_t uiHeading;					/* 10 times heading in degrees e.g. 2700 decimal = 270,0 Degress = west */

};

struct ShipGPSDataType {				/* data type for GPS data*/
	unsigned char ucUTCHours;			/* binary, not BCD coded (!) 0 - 23 decimal*/
	unsigned char ucUTCMins;			/* binary, 0-59 decimal */
	unsigned char ucUTCSeconds;			/* binary 0-59 decimal */
        unsigned char ucUTCDay;                         /* day 1-31 */
        unsigned char ucUTCMonth;                       /* month 1-12 */
        uint16_t      uiUTCYear;                         /* year 4 digits */
	double dLongitude;				/* "Laengengrad" I always mix it up...
										signed notation,
										negative values mean "W - west of Greenwich"
										positive values mean "E - east of Greenwich" */

	double dLatitude;				/* "Breitengrad" I always mix it up...
										signed notation,
										negative values mean "S - south of the equator"
										positive values mean "N - north of the equator 
										will stick at 255 if no data received for a long period */
	double dGroundSpeed;				/* speed in knots above ground */
	double dCourseOverGround;			/* heading in degrees */

};

/*************************************************************************************************************/
/* defines for Calibrator */
#define CALIB_VMFC_TOTAL        10000        /* virtual MFC address for total (Dry+Humid Flow) calibrator flow */
#define CALIB_VMFC_ABS          1000         /* MFC ADR OFFSET to specify flow in counts instead of sccm */
#define CALIB_SETFLOW_SUCCESS   1            /* success to set flow */
#define CALIB_SETFLOW_FAIL      0            /* fail to set flow */
/*************************************************************************************************************/

enum WhichMirror {
    MIRROR_GREEN_1,
    MIRROR_GREEN_2, 
    MIRROR_UV_1, 
    MIRROR_UV_2,
    
    MAX_MIRROR
}; 

enum MirrorAxis {
    XAXIS,
    YAXIS,
    
    MAX_MIRROR_AXIS 
}; 

#define REALIGN_MINUTES 30 /* mirror.c starts realignment every REALIGN_MINUTES, or never for negative numbers */
#define MIN_UV_DIFF_CTS 8 /* eq. 0.1 mW UV Diode Power */
#define DELTA_XPOSITION_MIRROR_GR1 0	/* used by second green mirror */
#define DELTA_YPOSITION_MIRROR_GR1 0	/* used by second green mirror */
#define DELTA_XPOSITION_MIRROR_GR2 25	/* used by second green mirror */
#define DELTA_YPOSITION_MIRROR_GR2 50	/* used by second green mirror */
#define DELTA_XPOSITION_MIRROR_UV1 200  	/* used by UV Mirror */
#define DELTA_YPOSITION_MIRROR_UV1 200  	/* used by UV Mirror */
#define DELTA_XPOSITION_MIRROR_UV2 0  	/* used by UV Mirror */
#define DELTA_YPOSITION_MIRROR_UV2 0  	/* used by UV Mirror */

struct AxisType {
  int32_t Position;
}; /* union AxisType */

struct MirrorType {
  struct AxisType Axis[MAX_MIRROR_AXIS];
}; /* MirrorType */

struct MovingFlagFieldType
{
  uint16_t MovingFlagByte:8;	/* Bitnumber=MirrorNumber*MAX_MIRROR_AXIS+MirrorAxis */
  uint16_t Realigning:1;      /* moving due to realignment routine */
  uint16_t unused:7;	/* not used (probably 0) */
};


union MovingFlagType {
  struct MovingFlagFieldType Field;
  uint16_t Word;
}; 
  
  
struct MirrorDataType {

  struct MirrorType Mirror[MAX_MIRROR];
  union MovingFlagType MovingFlag;
  uint16_t MinUVDiffCts;
  uint16_t RealignMinutes;
	
}; /* MirrorDataType */

/*************************************************************************************************************/

struct elekStatusType {                                             /* combined status information of instrument */

  /* data structures for the Master Box (lift / pentium 3) */
  
  struct timeval             TimeOfDayMaster;                       /* time of data */
  struct CounterCardType     CounterCardMaster;                     /* Counter Card for Reference */
  struct EtalonDataType      EtalonData;                            /* All Etalonstepper data */
  struct ADCCardType         ADCCardMaster[MAX_ADC_CARD_LIFT];                 /* ADC Card */
  struct MFCCardType         MFCCardMaster[MAX_MFC_CARD_LIFT];                 /* MFC Card */
  struct ValveCardType       ValveCardMaster[MAX_VALVE_CARD_LIFT];             /* Valve Card */
  struct DCDC4CardType       DCDC4CardMaster[MAX_DCDC4_CARD_LIFT];             /* Valve Card */
  struct TempSensorCardType  TempSensCardMaster[MAX_TEMP_SENSOR_CARD_LIFT];    /* Temperature Sensor Card */
  struct InstrumentFlagsType InstrumentFlags;                       /* Instrument flags */
  struct GPSDataType         GPSDataMaster; 
  struct MirrorDataType	     MirrorData;			    /* all Mirror Stepper data */

  /* data structures for the Slave Box (Wingpod / ARM9) */
 
  uint16_t                   uiValidSlaveDataFlag;                            /* indicates weather slave data is valid or not */
  struct timeval             TimeOfDaySlave;
  struct CounterCardType     CounterCardSlave;                            /* OH and HO2*/
  struct ADCCardType         ADCCardSlave[MAX_ADC_CARD_WP];
  /*  struct ADC24CardType       ADC24CardsSlave[MAX_24BIT_ADC_CARDS_WP];   /* 24bit card is not working, so.... */
  struct MFCCardType         MFCCardSlave[MAX_ADC_CARD_WP];
  struct ValveCardType       ValveCardSlave[MAX_VALVE_CARD_WP];
  struct TempSensorCardType  TempSensCardSlave[MAX_TEMP_SENSOR_CARD_WP];
  struct GPSDataType         GPSDataSlave;
  
  /* needed for calculation of processing time */
  struct TSCDataType         TimeStampCommand;

  struct ButterflyType       ButterflySlave;
                                  
										  /* GPS Data */
}; /* elekStatusType */


struct calibStatusType     /* new structure introduced for the calibrator automation process */ 
{
  /* data structures for the calibrator box */
  struct timeval             TimeOfDayCalib;
  struct ADCCardType         ADCCardCalib[MAX_ADC_CARD_CALIB];
  struct MFCCardType         MFCCardCalib[MAX_ADC_CARD_CALIB];
  struct SCRCardType         SCRCardCalib[MAX_SCR3XB_CALIB];
  struct TempSensorCardType  TempSensCardCalib[MAX_TEMP_SENSOR_CARD_CALIB];
  struct LicorH2OCO2Type     LicorCalib;
  struct PIDregulatorType    PIDRegulator;
};

struct auxStatusType     /* structure for the meteo box and ship's auxilliary data*/ 
{
  /* data structures for the meteobox */
  struct timeval             TimeOfDayAux;
  struct MeteoBoxType        MeteoBox;
  struct ShipGPSDataType     ShipGPS;
  struct ShipSonarType       ShipSonar;
  struct ShipMeteoType       ShipMeteo;
  struct ShipGyroType        ShipGyro;
  struct ShipWaterType       ShipWater;
  struct LicorPressureType   LicorPressure;
  struct AuxDataValidType    Status;
};



extern int elkInit(void);
extern int elkExit(void);
extern int elkWriteData(uint16_t Adress, uint16_t Data);
extern int elkReadData(uint16_t Adress);
#endif

