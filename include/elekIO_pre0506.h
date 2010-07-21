/* header file for elekIO 
* created by HH Oct 2003 
* changelog 

24.05.2004    HH   added ADC Card structure

*
*
*/



#define ELK_TIMEOUT (unsigned) 0x1000
#define ELK_BASE (unsigned)0x200
#define ELK_ADR  ELK_BASE
#define ELK_DATA (ELK_BASE + (unsigned) 2)
#define ELK_TODO (ELK_BASE + (unsigned) 4)
#define ELK_QSIZE 4

#define ELK_STEP_BASE    0xa400
#define ELK_STEP_SETPOS  (ELK_STEP_BASE + 0x0020)
#define ELK_STEP_GETPOS  (ELK_STEP_BASE + 0x0022)
#define ELK_STEP_SETSPD  (ELK_STEP_BASE + 0x0024)
#define ELK_STEP_GETSPD  (ELK_STEP_BASE + 0x0026)

#define ELK_ADC_BASE     0xa480
#define ELK_PWM_BASE     0xa460
#define ELK_VAL1_BASE    0xa470
#define ELK_VAL2_BASE    0xa472
#define ELK_MFC_BASE     0xa4a0


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
#define COUNTER_MASK_WIDTH       12     /* number of words for the mask reg. (12x16=160) */

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

enum EtalonActionType {
    ETALON_ACTION_NOP,                       /* etalon is doing no atuomated operation */
    ETALON_ACTION_TOGGLE,                    /* etalon is toggling between on and offline */
    ETALON_ACTION_SCAN,                      /* etalon is scanning */
    ETALON_ACTION_HOME,                      /* etalon is on a home run */

    ETALON_ACTION_MAX
};

struct EtalonDataType {
    uint16_t CurPosition;            /* Current Speed */
    uint16_t CurSpeed;               /* Current Position */
    uint16_t SetPosition;          
    uint16_t SetSpeed;
    uint16_t AbsPosition;            /* Drehgeber */
    uint16_t IndexPosition;          /* position of last index hole transistion */
    uint16_t Status;                 /* Status, end switch, index      */
}; /* EtalonDataType */

/*************************************************************************************************************/

#define MAX_ADC_CHANNEL_PER_CARD 8             /* number of Channels on each ADC Card */
#define MAX_ADC_CARD             2             /* number of ADC Cards in System */
#define ELK_ADC0_BASE            0xa480        /* base addr */
#define ELK_ADC0_CONFIG          (ELK_ADC0_BASE + 0x0010)  /* base addr */


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

struct ADCCardType {                                                      /* ADC Card Type */
    struct ADCChannelDataType ADCChannelData[MAX_ADC_CHANNEL_PER_CARD];
    union  ADCChannelConfigType ADCChannelConfig[MAX_ADC_CHANNEL_PER_CARD];
}; /* ADCCardType */


/*************************************************************************************************************/

#define MAX_VALVE_CARD     2                                  /* number of Valve Cards */
#define MAX_VALVES_ON_CARD 14                                 /* number of Valves on Card */
struct ValveCardType {
	uint16_t    ValveVolt;                                /* voltage applied for Card */
	uint16_t    Valve;                                    /* each bit reprsensts one valve */
};
	
	
	

/*************************************************************************************************************/

struct InstrumentFlagsType {             /* set of flags for the instrument Server status */

    uint16_t  StatusSave:1;            /* indicates if Status should be saved to disk */
    uint16_t  StatusQuery:1;           /* indicates if Status should be Queried from elekIOServ */

    enum EtalonActionType EtalonAction;           /* indicates what the etalon is doing */
    
}; /* ServerFlagsType */

/*************************************************************************************************************/

struct elekStatusType {                                      /* combined status information of instrument */
    struct timeval             TimeOfDay;                    /* time of data */
    struct CounterCardType     CounterCard;                  /* Counter Card for Ref, OH and HO2 */
    struct EtalonDataType      EtalonData;                   /* All Etalonstepper data */
    struct ADCCardType         ADCCard[MAX_ADC_CARD];        /* ADC Card */
    struct ValveCardType       ValveCard[MAX_VALVE_CARD];    /* Valve Card */
    struct InstrumentFlagsType InstrumentFlags;              /* Instrument flags */
}; /* elekStatusType */



extern int elkInit(void);
extern int elkExit(void);
extern int elkWriteData(uint16_t Adress, uint16_t Data);
extern int elkReadData(uint16_t Adress);

