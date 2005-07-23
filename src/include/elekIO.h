/* $RCSfile: elekIO.h,v $ header file for elekIO
*
* $RCSfile: elekIO.h,v $ last edit on $Date: 2005-07-23 09:00:29 $ by $Author: rudolf $
*
* $Log: elekIO.h,v $
* Revision 1.14  2005-07-23 09:00:29  rudolf
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

#define INIT_MODULE_FAILED  0
#define INIT_MODULE_SUCCESS 1

#define MAX_ADC_CARD_LIFT              2       /* number of 16bit ADC Cards in Lift */
#define MAX_ADC_CARD_WP                2       /* number of 16bit ADC Cards in Wingpod */
#define MAX_ADC_CHANNEL_PER_CARD       8       /* number of Channels on each 16bit ADC Card */

#define MAX_24BIT_ADC_CARDS_LIFT       0       /* number of 24bit ADC Cards in Lift */
#define MAX_24BIT_ADC_CARDS_WP         1       /* number of 24bit ADC Cards in Wingpod */
#define MAX_24BIT_ADC_CHANNEL_PER_CARD 8       /* number of Channels on each 24bit ADC card */

#define MAX_MFC_CARD_LIFT              1       /* number of MFC Cards in Lift */
#define MAX_MFC_CARD_WP                1       /* number of MFC Cards in Lift */
#define MAX_MFC_CHANNEL_PER_CARD       4       /* number of Channels on each MFC Card */

#define MAX_VALVE_CARD_LIFT            2       /* number of Valve Cards in Lift */
#define MAX_VALVE_CARD_WP              2       /* number of Valve Cards in WP */
#define MAX_VALVE_CHANNEL_PER_CARD     14      /* number of Valves on Card */

#define MAX_DCDC4_CARD_LIFT            1       /* number of DCDC4 in Lift */
#define MAX_DCDC4_CHANNEL_PER_CARD     4       /* number of Channels on DCDC4 */



#define ELK_TIMEOUT (unsigned) 0x1000
#define ELK_BASE (unsigned)0x200
#define ELK_ADR  ELK_BASE
#define ELK_DATA (ELK_BASE + (unsigned) 2)
#define ELK_TODO (ELK_BASE + (unsigned) 4)
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
#define ELK_PWM_BASE         (ELK_BACKPLANE_BASE+0x60)
#define ELK_PWM_DCDC4_BASE   (ELK_PWM_BASE)
#define ELK_PWM_VALVE_BASE   (ELK_PWM_BASE+0x08)


#define ELK_PWM_VALVE_BASE_WP (0xa460)

#define ELK_ADC_BASE         (ELK_BACKPLANE_BASE+0x80)
#define ELK_ADC_BASE_WP      (ELK_BACKPLANE_BASE+0x80)

#define ELK_MFC_BASE         (ELK_BACKPLANE_BASE+0xa0)          /* Base adr. of MFC ADC channels */ 
#define ELK_MFC_BASE_WP      (ELK_BACKPLANE_BASE+0xc0)


#define ELK_ADC_CONFIG          (0x0010)                        /* add to base addr */
#define ELK_ADC_NUM_ADR         (0x0020)                        /* number of addresses each ADC channel has */

#define ELK_MFC_CONFIG          (ELK_ADC_CONFIG)                /* offs. base addr for MFC Config*/
#define ELK_MFC_NUM_ADR         (ELK_ADC_NUM_ADR)               /* number of addresses each MFC channel has */
#define ELK_DAC_NUM_ADR         (MAX_DCDC4_CHANNEL_PER_CARD<<1) /* number of addresses each MFC channel has */

#define ELK_MFC_CONFIG_WP       (ELK_ADC_CONFIG)                /* offs. base addr for MFC Config*/
#define ELK_MFC_NUM_ADR_WP      (ELK_ADC_NUM_ADR)               /* number of addresses each MFC channel has */
#define ELK_DAC_NUM_ADR_WP      (MAX_DCDC4_CHANNEL_PER_CARD<<1) /* number of addresses each MFC channel has */

/* defines for the 24bit ADC */

#define ELK_ADC_24_BASE        0xa500                           /* Base Address of 24bit ADC */
#define ELK_ADC_24_POWER       (ELK_ADC_24_BASE + 0x0010)       /* Power register for the DCDC */
#define ELK_ADC_24_DATA        (ELK_ADC_24_BASE + 0x0020)       /* Channel Data registers */




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

#define CHANNEL_REF_CELL         0     /* Reference Cell Channel Number for etalon etc.. */

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
    ETALON_ACTION_NOP,                         /* etalon is doing no atuomated operation */
    ETALON_ACTION_DITHER_ONLINE,               /* stay online and dither */
    ETALON_ACTION_SCAN,                        /* etalon is scanning */
    ETALON_ACTION_HOME,                        /* etalon is on a home run */
    ETALON_ACTION_RECAL,                       /* etalon goes to home and comes back to same position */

    ETALON_ACTION_MAX
};


#define ETALON_SCAN_POS_START      5
#define ETALON_SCAN_POS_STOP       20000
#define ETALON_SCAN_STEP_WIDTH     8

#define ETALON_STEP_POS_ONLINE     500
#define ETALON_STEP_DITHER         16
#define ETALON_STEP_OFFLINE        64



struct LongWordType {
  uint16_t Low;
  uint16_t High;
}; /* struct LongWordType */
  

union PositionType {
  struct LongWordType PositionWord;
  uint32_t Position;
}; /* union PositionType */

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
  uint16_t Status;                    /* Status, end switch, index      */

}; /* EtalonDataType */

/*************************************************************************************************************/


struct ADCChannelConfigBitType {
    unsigned MuxChannel:3;
    unsigned Bridge:1;
    unsigned Gain:2;
    unsigned Offset:2;
    unsigned Unused:8;
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

/*************************************************************************************************************/

struct ADC24CardType {                                               
  uint16_t NumSamples;                                                    /* number of Samples for statistik */
  union ADCChannel24DataType ADCChannelData[MAX_24BIT_ADC_CHANNEL_PER_CARD];
 }; /* ADCCardType */

/*************************************************************************************************************/


struct MFCChannelConfigBitType {
  unsigned MuxChannel:3;
  unsigned Unused:5;
  unsigned Ch0:2;
  unsigned Ch1:2;
  unsigned Ch2:2;
  unsigned Ch3:2;
}; /* MFCChannelConfigBitType */

union MFCChannelConfigType {
    struct MFCChannelConfigBitType MFCChannelConfigBit;
    uint16_t MFCChannelConfig;
}; /* MFCConfigType */
    
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
#define MAX_TEMP_SENSOR   23
#define MAX_TEMP_SENSOR_CARD_LIFT 1
#define MAX_TEMP_SENSOR_CARD_WP 1
#define MAX_TEMP_TIMEOUT  100                                         /* maximum Timeout to wait for Temperature Card to be ready */
#define MAX_TEMP_MISSED_READING 5                                    /* number of maximal reading failures before removing a temperature */
#define ELK_TEMP_BASE           0xb000                                /* Base of Temperature Card */
#define ELK_TEMP_CTRL           (ELK_TEMP_BASE+0x00f8)                /* Controlword */
#define ELK_TEMP_FOUND          (ELK_TEMP_BASE+0x00fe)                /* Number of Sensors found */
#define ELK_TEMP_ERR_CRC        (ELK_TEMP_BASE+0x00fa)                /* Number of CRC Errors */
#define ELK_TEMP_ERR_NORESPONSE (ELK_TEMP_BASE+0x00fc)                /* Number of No Response Errors */

#define ELK_TEMP_BUSYFLAG        0x0001                               /* we Request access */
#define ELK_TEMP_UPDATEFLAG      0x0002                               /* AVR is busy       */
#define ELK_TEMP_SCANFLAG        0x0004                               /* AVR is scanning for         */

struct TempSensorFieldType {                                            /* Bit Field */
  unsigned int  TempFrac:4;                                             /* Temperatur Fraction 1/16 K */
  int           TempMain:8;                                             /* Temperatur K --> Ges Temp=TempMain+TempFrac/16 */ 
  unsigned int  bValid:1;                                               /* Temperatur Daten Gueltig=1 */
  unsigned int  bCRCError:1;                                            /* CRC Fehler, Pruefsumme falsch */
  unsigned int  bNoResponse:1;                                          /* Sensor antowrtet nicht */
  unsigned int  bAlarmFlag:1;                                           /* Sensor meldet Alarm */
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
  unsigned int Busy:1;
  unsigned int Update:1;
  unsigned int Scan:1;
  unsigned int Unused:12;
  unsigned int Reset:1;
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

enum InstrumentActionType { /* update also in instrument.c */
    INSTRUMENT_ACTION_NOP,			
	INSTRUMENT_ACTION_MEASURE,
    INSTRUMENT_ACTION_CAL,   
    INSTRUMENT_ACTION_DIAG,  
    INSTRUMENT_ACTION_POWERUP, 
    INSTRUMENT_ACTION_POWERDOWN, 
    INSTRUMENT_ACTION_LASERMIRRORTUNE,

    INSTRUMENT_ACTION_MAX
};



struct InstrumentFlagsType {                      /* set of flags for the instrument Server status */
    uint16_t  StatusSave:1;                       /* indicates if Status should be saved to disk */
    uint16_t  StatusQuery:1;                      /* indicates if Status should be Queried from elekIOServ */
    enum EtalonActionType EtalonAction;           /* indicates what the etalon is doing */
	enum InstrumentActionType InstrumentAction;		/* indicates what the instrument is doing (measuring, calibrating, etc.) */	
  /*  enum DebugType        Debug;                    /* indicates */ 
}; /* ServerFlagsType */

/* structure for TSC for time differenre measurements Master <-> Slave */

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

  /* data structures for the Slave Box (Wingpod / ARM9) */
 
  uint16_t                   uiValidSlaveDataFlag;                            /* indicates weather slave data is valid or not */
  struct timeval             TimeOfDaySlave;
  struct CounterCardType     CounterCardSlave;                            /* OH and HO2*/
  struct ADCCardType         ADCCardSlave[MAX_ADC_CARD_WP];
  struct ADC24CardType       ADC24CardsSlave[MAX_24BIT_ADC_CARDS_WP];
  struct MFCCardType         MFCCardSlave[MAX_ADC_CARD_WP];
  struct ValveCardType       ValveCardSlave[MAX_VALVE_CARD_WP];
  struct TempSensorCardType  TempSensCardSlave[MAX_TEMP_SENSOR_CARD_WP];
  struct GPSDataType         GPSDataSlave;
  
  /* needed for calculation of processing time */
  struct TSCDataType         TimeStampCommand;
  
                                  
										  /* GPS Data */
}; /* elekStatusType */



extern int elkInit(void);
extern int elkExit(void);
extern int elkWriteData(uint16_t Adress, uint16_t Data);
extern int elkReadData(uint16_t Adress);

