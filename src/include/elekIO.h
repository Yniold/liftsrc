/* header file for elekIO 
* created by HH Oct 2003 
* changelog 

24.05.2004    HH   added ADC Card structure

*
*
*/

#define INIT_MODULE_FAILED  0
#define INIT_MODULE_SUCCESS 1

#define MAX_ADC_CARD             2             /* number of MFC Cards in System */
#define MAX_ADC_CHANNEL_PER_CARD 8             /* number of Channels on each MFC Card */

#define MAX_MFC_CARD             1             /* number of MFC Cards in System */
#define MAX_MFC_CHANNEL_PER_CARD 4             /* number of Channels on each MFC Card */

#define MAX_VALVE_CARD     2                                         /* number of Valve Cards */
#define MAX_VALVE_CHANNEL_PER_CARD 14                                        /* number of Valves on Card */

#define MAX_DCDC4_CARD     1                                         /* number of DCDC4 */
#define MAX_DCDC4_CHANNEL_PER_CARD 4                                       /* number of Channels on DCDC4 */



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


#define ELK_DACPWM_BASE    (ELK_BACKPLANE_BASE+0x04)
#define ELK_VALVE_BASE     (ELK_BACKPLANE_BASE+0x08)
#define ELK_DAC_BASE       (ELK_BACKPLANE_BASE+0x40)
#define ELK_PWM_BASE       (ELK_BACKPLANE_BASE+0x60)
#define ELK_PWM_DCDC4_BASE    (ELK_PWM_BASE)
#define ELK_PWM_VALVE_BASE    (ELK_PWM_BASE+0x08)
#define ELK_ADC_BASE       (ELK_BACKPLANE_BASE+0x80)
#define ELK_MFC_BASE       (ELK_BACKPLANE_BASE+0xa0)

#define ELK_ADC_CONFIG          (0x0010)                        /* add to base addr */
#define ELK_ADC_NUM_ADR         (0x0020)                        /* number of addresses each ADC channel has */
#define ELK_MFC_CONFIG          (ELK_ADC_CONFIG)                /* base addr for MFC Config*/
#define ELK_MFC_NUM_ADR         (ELK_ADC_NUM_ADR)               /* number of addresses each MFC channel has */
#define ELK_DAC_NUM_ADR         (MAX_DCDC4_CHANNEL_PER_CARD<<1) /* number of addresses each MFC channel has */



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
    ETALON_ACTION_TOGGLE_ONLINE_LEFT,          /* etalon is on the left OFFLINE Position */
    ETALON_ACTION_TOGGLE_ONLINE_RIGHT,         /* etalon is on the right OFFLINE Position */
    ETALON_ACTION_TOGGLE,                      /* etalon is toggling between on and offline */
    ETALON_ACTION_TOGGLE_OFFLINE_LEFT,         /* etalon is on the left OFFLINE Position */
    ETALON_ACTION_TOGGLE_OFFLINE_RIGHT,        /* etalon is on the right OFFLINE Position */
    ETALON_ACTION_NOP,                         /* etalon is doing no atuomated operation */
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

struct ADCCardType {                                               
  uint16_t NumSamples;                                                    /* number of Samples for statistik */
  struct ADCChannelDataType ADCChannelData[MAX_ADC_CHANNEL_PER_CARD];
  union  ADCChannelConfigType ADCChannelConfig[MAX_ADC_CHANNEL_PER_CARD];
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
#define MAX_TEMP_SENSOR_CARD 1
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

struct InstrumentFlagsType {                      /* set of flags for the instrument Server status */
    uint16_t  StatusSave:1;                       /* indicates if Status should be saved to disk */
    uint16_t  StatusQuery:1;                      /* indicates if Status should be Queried from elekIOServ */
    enum EtalonActionType EtalonAction;           /* indicates what the etalon is doing */
  /*  enum DebugType        Debug;                    /* indicates */ 
}; /* ServerFlagsType */

/*************************************************************************************************************/

struct elekStatusType {                                             /* combined status information of instrument */
  struct timeval             TimeOfDay;                             /* time of data */
  struct CounterCardType     CounterCard;                           /* Counter Card for Ref, OH and HO2 */
  struct EtalonDataType      EtalonData;                            /* All Etalonstepper data */
  struct ADCCardType         ADCCard[MAX_ADC_CARD];                 /* ADC Card */
  struct MFCCardType         MFCCard[MAX_MFC_CARD];                 /* MFC Card */
  struct ValveCardType       ValveCard[MAX_VALVE_CARD];             /* Valve Card */
  struct DCDC4CardType       DCDC4Card[MAX_DCDC4_CARD];             /* Valve Card */
  struct TempSensorCardType  TempSensCard[MAX_TEMP_SENSOR_CARD];    /* Temperature Sensor Card */
  struct InstrumentFlagsType InstrumentFlags;                       /* Instrument flags */
}; /* elekStatusType */



extern int elkInit(void);
extern int elkExit(void);
extern int elkWriteData(uint16_t Adress, uint16_t Data);
extern int elkReadData(uint16_t Adress);

