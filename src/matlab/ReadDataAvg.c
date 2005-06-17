/*=================================================================
 *
 * READDataAvg.c
 *	        
 *  Read binary data from instrument software
 * The calling syntax is:
 *
 *   [statusdata,Avgdata] = ReadData('filename',RunAverageLen,MinRefCellCounts)
 *   where RunAvglength is number of 1/5 s data -> average time period in s = RunAverageLen/5
 *    and MinRefCellCounts is min. PMT count value that must be reached in online modus
 * $ID:$
 * $Log: ReadDataAvg.c,v $
 * Revision 1.14  2005-06-17 15:54:44  rudolf
 * fix for using Borland C++ 5.5.1 with ReadDataAvg.c under Mathlab for Windows
 *
 *
 *
 *
 *=================================================================*/
 
 /* $Revision: 1.14 $ */
#include <math.h>
#include <stdio.h>
#include <time.h>

/* #define D_HEADER     1               /* will print with mexPrintf the position of each row */
/*#define UNIX   */                     /* has to be set for unix compilation */

#define MAX_AVERAGE_TIME 1000            /* maximum number of average data points for running avg */



#ifdef UNIX                             /* there are some differences between windows & linux header files */
#include <stdint.h>
#else
typedef unsigned short uint16_t;
typedef unsigned uint32_t;
typedef int int32_t;

typedef unsigned __int64 uint64_t;

struct timeval {
  long    tv_sec; 
  long    tv_usec;
};
#endif

#include "mex.h"
#include "../include/elekIO.h"

struct OnlineOfflineCountsType {
  int *Online;
  int *OfflineLeft;
  int *OfflineRight;
  long OnlineSum;
  long OfflineLeftSum;
  long OfflineRightSum;
};

struct RunningAVGType {
  int *Avg;
  int *Std;
  int *Num;
  int *OnOffFlag;
};

int cmptimesort(const void *ptrElekStatus1,
                const void *ptrElekStatus2) {
        
    int ret;
    
    ret=0;
    if (((struct elekStatusType*)ptrElekStatus1)->TimeOfDayMaster.tv_sec>((struct elekStatusType*)ptrElekStatus2)->TimeOfDayMaster.tv_sec) ret=1;
    if (((struct elekStatusType*)ptrElekStatus2)->TimeOfDayMaster.tv_sec>((struct elekStatusType*)ptrElekStatus1)->TimeOfDayMaster.tv_sec) ret=-1;
    
    return (ret);
} /* timesort */

void mexFunction( int nlhs, mxArray *plhs[], 
		  int nrhs, const mxArray*prhs[] )
     
{ 
  /*    unsigned int m,n;  */
  unsigned short *z,*ptrData;
  float *ptrAvg;
  char *input_buf;
  int buflen;
  int status;
  int i,j,k;
  int Card;
  FILE *fp;
  long flen;
  long nelements;
  int dims[2];
  int dimaverages[2];
  unsigned char *Databuf;
  long count, countAvg;
  struct tm *ptrTmZeit;
  time_t Seconds;
  int Channel;
  int RunAverageLen;
  int RunAverageLenOffl;
  int MinRefCellCounts;
  
  struct elekStatusType *elekStatus;
  struct OnlineOfflineCountsType OnlineOfflineCounts[MAX_COUNTER_CHANNEL];
  int OnlineCounter;                                   /* number of Onlines we encountered */
  int OfflineLeftCounter;                                  /* number of Offlines we encountered */
  int OfflineRightCounter;                                  /* number of Offlines we encountered */
  int maxSteps;                                        /* max steps we want to take for averaging*/
  struct RunningAVGType OnlineAverage[MAX_COUNTER_CHANNEL];
  struct RunningAVGType OfflineLeftAverage[MAX_COUNTER_CHANNEL];
  struct RunningAVGType OfflineRightAverage[MAX_COUNTER_CHANNEL];
  /*  uint16_t *MCP1RayCounts;
  uint16_t *MCP2RayCounts;
  uint16_t Sum1;
  uint16_t Sum2; */
 
  #ifdef X_DEBUG  
    mexPrintf("sizeof struct elekStatusType %d",sizeof(struct elekStatusType));
  #endif
  
  /* Check for proper number of arguments */
  
  if (nrhs != 3) { 
    mexErrMsgTxt("three input arguments required, Filename, Average length, min online ref cell counts"); 
  } else if (nlhs != 2) {
    mexErrMsgTxt("Two output arguments required: data, averages."); 
  } 
  
  /* Input must be a string. */
  if (mxIsChar(prhs[0]) != 1)
    mexErrMsgTxt("Input must be a string.");
  
  /* Input must be a row vector. */
  if (mxGetM(prhs[0]) != 1)
    mexErrMsgTxt("Input must be a row vector.");
  
  /* Get the length of the input string. */
  buflen = (mxGetM(prhs[0]) * mxGetN(prhs[0])) + 1;

  RunAverageLen = (int) *mxGetPr(prhs[1]);
  if (RunAverageLen>MAX_AVERAGE_TIME) 
    mexErrMsgTxt("AverageLen cannot be longer than 100");

  RunAverageLenOffl = RunAverageLen/2;

  MinRefCellCounts =(int) *mxGetPr(prhs[2]); 
  if (MinRefCellCounts<1) 
    mexErrMsgTxt("MinRefCell Online Counts must be greater than 1");
    

  /*mexPrintf("RunAver %d",RunAverageLen);*/
  
  /* Allocate memory for input and output strings. */
  input_buf = mxCalloc(buflen, sizeof(char));
  
  /* Copy the string data from prhs[0] into a C string 
   * input_buf. If the string array contains several rows, 
   * they are copied, one column at a time, into one long 
   * string array. */
  status = mxGetString(prhs[0], input_buf, buflen);
  if (status != 0) 
    mexWarnMsgTxt("Not enough space. String is truncated.");  
  

  fp=fopen(input_buf,"rb");
  
  if (fp==NULL) 
    mexErrMsgTxt("can't open file");
  
  fseek(fp,0,SEEK_END);
  flen=ftell(fp);
  
  /* we have to allocate some space, use mxCalloc, matlab is cleaning up memory upon exit */
  Databuf=mxCalloc(flen, sizeof(char));
  nelements=flen/sizeof(struct elekStatusType);
  /* go to the beginning */
  fseek(fp,0,SEEK_SET);
  
  status=fread(Databuf,sizeof(struct elekStatusType),nelements,fp);
  fclose(fp);
  
  elekStatus=(struct elekStatusType*)Databuf;
  
  dims[0]= nelements;

  dims[1]= 6 * 2;              /* Jahr JulTag Stunde Minute Sekunde Mikrosek f�r Master und Slave*/
  dims[1]= dims[1] + (ADC_CHANNEL_COUNTER_CARD+1)*2 + 
    MAX_COUNTER_CHANNEL*(5+MAX_COUNTER_TIMESLOT+COUNTER_MASK_WIDTH);
  dims[1]= dims[1] + 12;   /* etalon data type */
  dims[1]= dims[1] + (MAX_ADC_CARD_LIFT+MAX_ADC_CARD_WP)*(1+MAX_ADC_CHANNEL_PER_CARD*3+MAX_ADC_CHANNEL_PER_CARD*1);
  dims[1]= dims[1] + MAX_24BIT_ADC_CARDS_WP*(1+2*MAX_24BIT_ADC_CHANNEL_PER_CARD);
  dims[1]= dims[1] + 2;  /* mfc data Master*/
  dims[1]= dims[1] + 2*2;  /* valve data Master and Slave*/  
  dims[1]= dims[1] + MAX_DCDC4_CHANNEL_PER_CARD;  /* dcdc data */
  dims[1]= dims[1] + (4 + MAX_TEMP_SENSOR*3)*2;  /* temp data Master and Slave*/
  dims[1]= dims[1] + 1;  /* EtalonAction */
  dims[1]= dims[1] + 1;  /* EtalonOnlinePos high */
  dims[1]= dims[1] + 1;  /* EtalonOnlinePos low */
  
  dims[1]= dims[1] + 1;  /* InstrumentAction */
  dims[1]= dims[1] + 2*11;  /* GPS Masterand Slave*/

  dims[1]= dims[1] + 3;  /* extra reserve */

  plhs[0] = mxCreateNumericArray(2,dims,mxUINT16_CLASS,mxREAL);

  dimaverages[0]=dims[0];
  dimaverages[1]=MAX_COUNTER_CHANNEL*3;/* Online, OfflineLeft, OfflineRight */
  plhs[1] = mxCreateNumericArray(2,dimaverages,mxSINGLE_CLASS,mxREAL);
  
  /* Assign pointers to each input and output. */
  z = mxGetPr(plhs[0]);
  ptrData=z;
  count=0;
  ptrAvg=mxGetPr(plhs[1]);
  countAvg=0;
  
  #ifdef X_DEBUG  
    mexPrintf("allocate memory....\n");
  #endif

  /* allocate mem for running Average arrays and init struct */
  for (Channel=0; Channel<MAX_COUNTER_CHANNEL; Channel++) {
    OnlineOfflineCounts[Channel].Online=(int*)mxCalloc(MAX_AVERAGE_TIME, sizeof(int));
    OnlineOfflineCounts[Channel].OfflineLeft=(int*)mxCalloc(MAX_AVERAGE_TIME, sizeof(int));
    OnlineOfflineCounts[Channel].OfflineRight=(int*)mxCalloc(MAX_AVERAGE_TIME, sizeof(int));
    OnlineOfflineCounts[Channel].OnlineSum=0;
    OnlineOfflineCounts[Channel].OfflineLeftSum=0;
    OnlineOfflineCounts[Channel].OfflineRightSum=0;
    /* for each timestep a on and offline avg is avail */
    OnlineAverage[Channel].Avg=(int*)mxCalloc(nelements, sizeof(int)); 
    OnlineAverage[Channel].Num=(int*)mxCalloc(nelements, sizeof(int)); 
    OfflineLeftAverage[Channel].Avg=(int*)mxCalloc(nelements, sizeof(int)); 
    OfflineLeftAverage[Channel].Num=(int*)mxCalloc(nelements, sizeof(int)); 
    OfflineRightAverage[Channel].Avg=(int*)mxCalloc(nelements, sizeof(int)); 
    OfflineRightAverage[Channel].Num=(int*)mxCalloc(nelements, sizeof(int)); 
    OnlineAverage[Channel].OnOffFlag=(int*)mxCalloc(nelements, sizeof(int)); 
  } /* for */

  /* MCP1RayCounts=(uint16_t*)mxCalloc(nelements, sizeof(uint16_t)); */
  /* MCP2RayCounts=(uint16_t*)mxCalloc(nelements, sizeof(uint16_t)); */

  /* first we sort the data according to time */
  
  #ifdef X_DEBUG  
    mexPrintf("sort....\n");
  #endif
  qsort(elekStatus,nelements,sizeof(struct elekStatusType),cmptimesort);
  
  #ifdef X_DEBUG  
    mexPrintf("copy counts....\n");
  #endif

/*  for (i=0; i<nelements;i++) {
	/* for the Rayleigh signals we recount the first 20 channels */
/*    Sum1=0;
	Sum2=0;
    for(k=1; k<21;k++) {
		Sum1=Sum1+elekStatus[i].CounterCardSlave.Channel[1].Data[k];
		Sum2=Sum2+elekStatus[i].CounterCardSlave.Channel[2].Data[k]; 
	} /* for k */
/*    MCP1RayCounts[i]=Sum1;
	MCP2RayCounts[i]=Sum2; */
/*  } /* for i */

  
  /* Date Master*/

  #ifdef X_DEBUG  
    mexPrintf("fill year....%d elements\n",nelements);
  #endif

  
  for (i=0; i<nelements;i++) {
    Seconds=elekStatus[i].TimeOfDayMaster.tv_sec;
    #ifdef X_DEBUG  
      mexPrintf("%lu ",Seconds);
    #endif
    ptrTmZeit=(struct tm *)localtime(&Seconds); 
    #ifdef X_DEBUG  
      mexPrintf("%d ",ptrTmZeit->tm_year);
    #endif
    *(z+count++)=(uint16_t)ptrTmZeit->tm_year;
    #ifdef X_DEBUG  
      mexPrintf("%d ",i);
    #endif
  }
  
  #ifdef X_DEBUG  
    mexPrintf("fill yday....\n");
  #endif
  for (i=0; i<nelements;i++) {
    Seconds=elekStatus[i].TimeOfDayMaster.tv_sec;
    ptrTmZeit=localtime(&Seconds);
    *(z+count++)=ptrTmZeit->tm_yday;
  }
  for (i=0; i<nelements;i++) {
    Seconds=elekStatus[i].TimeOfDayMaster.tv_sec;
    ptrTmZeit=localtime(&Seconds);
    *(z+count++)=ptrTmZeit->tm_hour;
  }
  for (i=0; i<nelements;i++) {
    Seconds=elekStatus[i].TimeOfDayMaster.tv_sec;
    ptrTmZeit=localtime(&Seconds);
    *(z+count++)=ptrTmZeit->tm_min;
  }
  for (i=0; i<nelements;i++) {
    Seconds=elekStatus[i].TimeOfDayMaster.tv_sec;
    ptrTmZeit=localtime(&Seconds);
    *(z+count++)=ptrTmZeit->tm_sec;
  }
  for (i=0; i<nelements;i++) {
    *(z+count++)=elekStatus[i].TimeOfDayMaster.tv_usec/1000;
  }


  
  #ifdef X_DEBUG  
    mexPrintf("fill int16 array....\n");
  #endif

  /******************* Counter Cards ***************************/ 
  /* mexPrintf("%d %d %d\n",sizeof(uint16_t),sizeof(struct timeval),sizeof(struct elekStatusType));
  mexPrintf("%d/%d %d %d %d\n",count,dims[0]*dims[1],dims[0],dims[1],ADC_CHANNEL_COUNTER_CARD);
  */

  #ifdef D_HEADER
    mexPrintf("ccADC Master %f\n",1+(double)count/(double)nelements);
  #endif
  for (j=0;j<ADC_CHANNEL_COUNTER_CARD;j++) {
    for (i=0; i<nelements;i++) {
	*(z+count++)=elekStatus[i].CounterCardMaster.ADCData[j];       
	/*       mexPrintf("%d %",elekStatus[i].CounterCard.ADCData[j]); */
    }	
  }

  #ifdef D_HEADER
    mexPrintf("ccMasterDelay Master  %d %d\n",count,1+count/nelements);      
  #endif
  for (i=0; i<nelements;i++) {
    *(z+count++)=elekStatus[i].CounterCardMaster.MasterDelay;       
  }
  

  #ifdef D_HEADER
    mexPrintf("ccADC Slave %f\n",1+(double)count/(double)nelements);
  #endif
  for (j=0;j<ADC_CHANNEL_COUNTER_CARD;j++) {
    for (i=0; i<nelements;i++) {
	*(z+count++)=elekStatus[i].CounterCardSlave.ADCData[j];       
	/*       mexPrintf("%d %",elekStatus[i].CounterCard.ADCData[j]); */
    }	
  }

  #ifdef D_HEADER
    mexPrintf("ccMasterDelay Slave  %d %d\n",count,1+count/nelements);      
  #endif
  for (i=0; i<nelements;i++) {
    *(z+count++)=elekStatus[i].CounterCardSlave.MasterDelay;       
  }


  j=0;  
	#ifdef D_HEADER
      mexPrintf("ccShiftDelay #%d %d\n",j,1+count/nelements);      
	#endif
    for (i=0; i<nelements;i++) {
      *(z+count++)=elekStatus[i].CounterCardMaster.Channel[j].ShiftDelay;       
    }	

	#ifdef D_HEADER
	  mexPrintf("ccGateDelay #%d %d\n",j,1+count/nelements);      
	#endif
    for (i=0; i<nelements;i++) {
	  *(z+count++)=elekStatus[i].CounterCardMaster.Channel[j].GateDelay;       
    }
    
	#ifdef D_HEADER
	  mexPrintf("ccGateWidth #%d %d\n",j,1+count/nelements);      
	#endif
    for (i=0; i<nelements;i++) {
      *(z+count++)=elekStatus[i].CounterCardMaster.Channel[j].GateWidth;       
    }
    
	#ifdef D_HEADER
	  mexPrintf("ccData #%d %d\n",j,1+count/nelements);      
	#endif
    for (k=0; k<MAX_COUNTER_TIMESLOT;k++) {
      for (i=0; i<nelements;i++) {
		*(z+count++)=elekStatus[i].CounterCardMaster.Channel[j].Data[k];       
	  }	
    }
    
	#ifdef D_HEADER
	  mexPrintf("ccMask #%d %d\n",j,1+count/nelements);      
	#endif
    for (k=0; k<COUNTER_MASK_WIDTH;k++) {
      for (i=0; i<nelements;i++) {
		*(z+count++)=elekStatus[i].CounterCardMaster.Channel[j].Mask[k];       
	  }	
    }
    
	#ifdef D_HEADER
	  mexPrintf("ccCounts #%d %d\n",j,1+count/nelements);      
	#endif
    for (i=0; i<nelements;i++) {
      *(z+count++)=elekStatus[i].CounterCardMaster.Channel[j].Counts;       
    }	
    
	#ifdef D_HEADER
	  mexPrintf("ccPulses #%d %d\n",j,1+count/nelements);      
	#endif
    for (i=0; i<nelements;i++) {
      *(z+count++)=elekStatus[i].CounterCardMaster.Channel[j].Pulses;       
    }	   
   
 
  for (j=1;j<MAX_COUNTER_CHANNEL;j++) { 
	#ifdef D_HEADER
      mexPrintf("ccShiftDelay #%d %d\n",j,1+count/nelements);      
	#endif
    for (i=0; i<nelements;i++) {
      *(z+count++)=elekStatus[i].CounterCardSlave.Channel[j].ShiftDelay;       
    }	

	#ifdef D_HEADER
	  mexPrintf("ccGateDelay #%d %d\n",j,1+count/nelements);      
	#endif
    for (i=0; i<nelements;i++) {
	  *(z+count++)=elekStatus[i].CounterCardSlave.Channel[j].GateDelay;       
    }
    
	#ifdef D_HEADER
	  mexPrintf("ccGateWidth #%d %d\n",j,1+count/nelements);      
	#endif
    for (i=0; i<nelements;i++) {
      *(z+count++)=elekStatus[i].CounterCardSlave.Channel[j].GateWidth;       
    }
    
	#ifdef D_HEADER
	  mexPrintf("ccData #%d %d\n",j,1+count/nelements);      
	#endif
    for (k=0; k<MAX_COUNTER_TIMESLOT;k++) {
      for (i=0; i<nelements;i++) {
		*(z+count++)=elekStatus[i].CounterCardSlave.Channel[j].Data[k];       
	  }	
    }
    
	#ifdef D_HEADER
	  mexPrintf("ccMask #%d %d\n",j,1+count/nelements);      
	#endif
    for (k=0; k<COUNTER_MASK_WIDTH;k++) {
      for (i=0; i<nelements;i++) {
		*(z+count++)=elekStatus[i].CounterCardSlave.Channel[j].Mask[k];       
	  }	
    }
    
	#ifdef D_HEADER
	  mexPrintf("ccCounts #%d %d\n",j,1+count/nelements);      
	#endif
    for (i=0; i<nelements;i++) {
      *(z+count++)=elekStatus[i].CounterCardSlave.Channel[j].Counts;       
    }	
    
	#ifdef D_HEADER
	  mexPrintf("ccPulses #%d %d\n",j,1+count/nelements);      
	#endif
    for (i=0; i<nelements;i++) {
      *(z+count++)=elekStatus[i].CounterCardSlave.Channel[j].Pulses;       
    }	   
  } 
      

/******************* etalon ***************************/  

  #ifdef D_HEADER
    mexPrintf("etaSetPosLow  %d %d\n",count,1+count/nelements);      
  #endif
/* 0*/
  for (i=0; i<nelements;i++) {
	*(z+count++)=elekStatus[i].EtalonData.Set.PositionWord.Low;       
  }

  #ifdef D_HEADER
	mexPrintf("etaSetPosHigh  %d %d\n",count,1+count/nelements);      
  #endif
/* 1*/
  for (i=0; i<nelements;i++) {
    *(z+count++)=elekStatus[i].EtalonData.Set.PositionWord.High;       
  }

  #ifdef D_HEADER
	mexPrintf("etaCurPosLow  %d %d\n",count,1+count/nelements);      
  #endif
/* 2*/
  for (i=0; i<nelements;i++) {
    *(z+count++)=elekStatus[i].EtalonData.Current.PositionWord.Low;       
  }

  #ifdef D_HEADER
    mexPrintf("etaCurPosHigh  %d %d\n",count,1+count/nelements);      
  #endif
/* 3*/
  for (i=0; i<nelements;i++) {
    *(z+count++)=elekStatus[i].EtalonData.Current.PositionWord.High;       
  }

  #ifdef D_HEADER
    mexPrintf("etaEncoderPosLow  %d %d\n",count,1+count/nelements);      
  #endif
/* 4*/
  for (i=0; i<nelements;i++) {
    *(z+count++)=elekStatus[i].EtalonData.Encoder.PositionWord.Low;       
  }

  #ifdef D_HEADER
    mexPrintf("etaEncoderPosHigh  %d %d\n",count,1+count/nelements);      
  #endif
/* 5*/
  for (i=0; i<nelements;i++) {
    *(z+count++)=elekStatus[i].EtalonData.Encoder.PositionWord.High;       
  }

  #ifdef D_HEADER
    mexPrintf("etaIndexPosLow  %d %d\n",count,1+count/nelements);      
  #endif
/* 6*/
  for (i=0; i<nelements;i++) {
    *(z+count++)=elekStatus[i].EtalonData.Index.PositionWord.Low;       
  }
  #ifdef D_HEADER
    mexPrintf("etaIndexPosHigh  %d %d\n",count,1+count/nelements);      
  #endif
/* 7*/
  for (i=0; i<nelements;i++) {
    *(z+count++)=elekStatus[i].EtalonData.Index.PositionWord.High;       
  }


  #ifdef D_HEADER
    mexPrintf("etaSetSpd  %d %d\n",count,1+count/nelements);      
  #endif
/* 8*/
  for (i=0; i<nelements;i++) {
    *(z+count++)=elekStatus[i].EtalonData.SetSpeed;       
  }

  #ifdef D_HEADER
    mexPrintf("etaSetAcclSpd  %d %d\n",count,1+count/nelements);      
  #endif
/* 9*/
  for (i=0; i<nelements;i++) {
    *(z+count++)=elekStatus[i].EtalonData.SetAccl;       
  }

  #ifdef D_HEADER
    mexPrintf("etaCurSpd  %d %d\n",count,1+count/nelements);      
  #endif
/* 10*/
  for (i=0; i<nelements;i++) {
    *(z+count++)=elekStatus[i].EtalonData.CurSpeed;       
  }

  #ifdef D_HEADER
    mexPrintf("etaStatus  %d %d\n",count,1+count/nelements);      
  #endif
/* 11*/
  for (i=0; i<nelements;i++) {
    *(z+count++)=elekStatus[i].EtalonData.Status;       
  }
    
/*  mexPrintf("%d/%d\n",count,dims[1]); */

/******************* ADC Cards Master***************************/     	 
    
  for (k=0; k<MAX_ADC_CARD_LIFT; k++) {

	#ifdef D_HEADER
	  mexPrintf("NumSamples Master %d %d\n",count,1+count/nelements);      
	#endif
    for (i=0; i<nelements;i++) 
	  *(z+count++)=elekStatus[i].ADCCardMaster[k].NumSamples;       
    
    for (j=0;j<MAX_ADC_CHANNEL_PER_CARD;j++) {       	
	  #ifdef D_HEADER
		mexPrintf("adcData Master #%d.%d %d\n",k,j,1+count/nelements);      
	  #endif
      for (i=0; i<nelements;i++) 
	    *(z+count++)=elekStatus[i].ADCCardMaster[k].ADCChannelData[j].ADCData;       

	  #ifdef D_HEADER
		mexPrintf("adcSumDat Master  %d %d\n",count,1+count/nelements);      
	  #endif
	  for (i=0; i<nelements;i++) 
		*(z+count++)=elekStatus[i].ADCCardMaster[k].ADCChannelData[j].SumDat;       
	
	  #ifdef D_HEADER
		mexPrintf("adcSumSqr Master  %d %d\n",count,1+count/nelements);      
	  #endif
	  for (i=0; i<nelements;i++) 
	    *(z+count++)=elekStatus[i].ADCCardMaster[k].ADCChannelData[j].SumSqr;                              	
	}
      
    for (j=0;j<MAX_ADC_CHANNEL_PER_CARD;j++) {  
	  #ifdef D_HEADER
		mexPrintf("adcConfig Master #%d.%d %d\n",k,j,1+count/nelements);      
	  #endif
	  for (i=0; i<nelements;i++) 
		*(z+count++)=elekStatus[i].ADCCardMaster[k].ADCChannelConfig[j].ADCChannelConfig;
	}
      
  }
    

  /*  mexPrintf("%d/%d %d %d\n",count,dims[0]*dims[1],dims[0],dims[1]);
  */


    
/******************* MFC Card ***************************/  
  /* we are only interested in the one Card for Slave, nothing connected to the card on Master */
  Card=0;
  #ifdef D_HEADER
	mexPrintf("MFCSetFlow  %d %d\n",count,1+count/nelements);      
  #endif
  for (i=0; i<nelements;i++) {
    *(z+count++)=elekStatus[i].MFCCardSlave[Card].MFCChannelData[0].SetFlow;       
  }

  #ifdef D_HEADER
    mexPrintf("MFCFlow  %d %d\n",count,1+count/nelements); 
  #endif
  for (i=0; i<nelements;i++) {
    *(z+count++)=elekStatus[i].MFCCardSlave[Card].MFCChannelData[0].Flow;    
  }

/******************* Valve Cards Master***************************/     
  /* for (k=0; k<MAX_VALVE_CARD_LIFT; k++) { */
  /* we are only interested in Data of Card 0 */
  Card=0;
  #ifdef D_HEADER
	mexPrintf("ValveVolt Master %d %d\n",count,1+count/nelements);      
  #endif
  for (i=0; i<nelements;i++) {
    *(z+count++)=elekStatus[i].ValveCardMaster[Card].ValveVolt;       
  }

  #ifdef D_HEADER
    mexPrintf("Valve Master  %d %d\n",count,1+count/nelements);      
  #endif
  for (i=0; i<nelements;i++) {
    *(z+count++)=elekStatus[i].ValveCardMaster[Card].Valve;       
  } 
  /* }*/

/******************* DCD4 Card ***************************/ 
  /* we are only interested in Data of Card 0 */
  Card=0;    
  for (j=0; j<MAX_DCDC4_CHANNEL_PER_CARD; j++) { 

	#ifdef D_HEADER
	  mexPrintf("DCDC4channel #%d %d\n",j,1+count/nelements);      
	#endif
    for (i=0; i<nelements;i++) {
	  *(z+count++)=elekStatus[i].DCDC4CardMaster[Card].Channel[j];       
    }
  }

/******************* TempSensor Card Master ***************************/  
  /* there is only one Card */
  Card=0;   
  #ifdef D_HEADER
    mexPrintf("TempMissed Master %d\n",1+count/nelements);      
  #endif
  for (i=0; i<nelements;i++) {
    *(z+count++)=elekStatus[i].TempSensCardMaster[Card].NumMissed;       
  }

  #ifdef D_HEADER
    mexPrintf("TempNumber Master %d\n",1+count/nelements);      
  #endif
  for (i=0; i<nelements;i++) {
    *(z+count++)=elekStatus[i].TempSensCardMaster[Card].NumSensor;       
  }

  #ifdef D_HEADER
    mexPrintf("TempErrCRC Master %d\n",1+count/nelements);      
  #endif
  for (i=0; i<nelements;i++) {
    *(z+count++)=elekStatus[i].TempSensCardMaster[Card].NumErrCRC;       
  }

  #ifdef D_HEADER
    mexPrintf("TempNoResponse Master %d\n",1+count/nelements);      
  #endif
  for (i=0; i<nelements;i++) {
    *(z+count++)=elekStatus[i].TempSensCardMaster[Card].NumErrNoResponse;       
  }

  for (j=0; j<MAX_TEMP_SENSOR; j++) { 

	#ifdef D_HEADER
	  mexPrintf("Temp Master #%d %d\n",j,1+count/nelements);      
	#endif
    /* we represent the temperature in 1/100K) */
    for (i=0; i<nelements;i++) {
	  *(z+count++)=(uint16_t) ( (elekStatus[i].TempSensCardMaster[Card].TempSensor[j].Field.TempFrac*100.0/16.0+
		elekStatus[i].TempSensCardMaster[Card].TempSensor[j].Field.TempMain*100)+27320);  
    }

	#ifdef D_HEADER
	  mexPrintf("Temp Master #%d valid_CRCerr_noresp_alarm %d\n",j,1+count/nelements);      
	#endif
    for (i=0; i<nelements;i++) {
	  *(z+count++)=elekStatus[i].TempSensCardMaster[Card].TempSensor[j].Word.WordTemp <<12; /* shift 4 status bits to lsb position */
    }

	#ifdef D_HEADER
	  mexPrintf("Temp Master #%d ID %d\n",j,1+count/nelements);      
	#endif
    for (i=0; i<nelements;i++) {
	  *(z+count++)=elekStatus[i].TempSensCardMaster[Card].TempSensor[j].Word.WordID[1];       
    }
  }

/******************* Instrument Flag Etalon Action ***************************/     
  #ifdef D_HEADER
    mexPrintf("EtalonAction  %d %d\n",count,1+count/nelements);      
  #endif
  for (i=0; i<nelements;i++) {
    *(z+count++)=elekStatus[i].InstrumentFlags.EtalonAction;       
  }

/******************* Etalon Online Pos ***************************/     
  #ifdef D_HEADER
    mexPrintf("etaOnlinePosLow  %d %d\n",count,1+count/nelements);      
  #endif
/* 6*/
  for (i=0; i<nelements;i++) {
    *(z+count++)=elekStatus[i].EtalonData.Online.PositionWord.Low;       
  }
  
  #ifdef D_HEADER
    mexPrintf("etaOnlinePosHigh  %d %d\n",count,1+count/nelements);      
  #endif
/* 7*/
  for (i=0; i<nelements;i++) {
    *(z+count++)=elekStatus[i].EtalonData.Online.PositionWord.High;       
  }


/******************* Calculate running averages ***************************/    
    
  /* now calculate running Average of Online and Offlinesignals */
  OnlineCounter=0;
  OfflineLeftCounter=0;
  OfflineRightCounter=0;
  for (i=0; i<nelements;i++) {
    Channel=0;       /* PMT */
      /* reset fields */
	  OnlineAverage[Channel].Num[i]=0;
	  OfflineLeftAverage[Channel].Num[i]=0;
	  OfflineRightAverage[Channel].Num[i]=0;
	  OnlineAverage[Channel].Avg[i]=0;
	  OfflineLeftAverage[Channel].Avg[i]=0;
	  OfflineRightAverage[Channel].Avg[i]=0;

	  OnlineAverage[Channel].OnOffFlag[i]=0; /* we don't know wether we are on=3 leftoff=2 or rightoffline=1 */
	  /* first decide if we are on or offline */
	  if ( (elekStatus[i].CounterCardMaster.Channel[0].Counts>MinRefCellCounts) &&             /* when PMTCounts>MinRefCellCounts and */
		   (abs(elekStatus[i].EtalonData.Current.Position-                  /* not further than Dithersteps of OnlienPos */
	         elekStatus[i].EtalonData.Online.Position)<=elekStatus[i].EtalonData.DitherStepWidth) &&
		   (elekStatus[i].InstrumentFlags.EtalonAction==ETALON_ACTION_TOGGLE_ONLINE_LEFT ||             /* we intend to be online */
			  elekStatus[i].InstrumentFlags.EtalonAction==ETALON_ACTION_TOGGLE_ONLINE_RIGHT)
	     
		 ) {             /* so we are online */
		OnlineOfflineCounts[Channel].Online[OnlineCounter % RunAverageLen ]=
	    elekStatus[i].CounterCardMaster.Channel[Channel].Counts;
	    OnlineCounter++;                                           /* in the case of RefCell Channel, increase number of Onlinecounts */
	    OnlineAverage[Channel].OnOffFlag[i]=3;
	  } else { /* if not, are we offline ? */
		if ( (elekStatus[i].EtalonData.Current.Position==                  /* lets see if we are left side offline */
			     (elekStatus[i].EtalonData.Online.Position-elekStatus[i].EtalonData.OfflineStepLeft)) && /* left steps away from Online */
	         (elekStatus[i].InstrumentFlags.EtalonAction==ETALON_ACTION_TOGGLE_OFFLINE_LEFT) ) {      /* intend to be there */
		  OnlineOfflineCounts[Channel].OfflineLeft[OfflineLeftCounter % RunAverageLenOffl ]=
	      elekStatus[i].CounterCardMaster.Channel[Channel].Counts;
		  OfflineLeftCounter++;
	      OnlineAverage[Channel].OnOffFlag[i]=2;
	      /* mexPrintf("OfflineCounter #%d %d\n",Channel,OfflineCounter); */
		} else { 
	    /* are we right side offline ? */
	    
	      if ( (elekStatus[i].EtalonData.Current.Position==                  /* lets see if we are right side offline */
					(elekStatus[i].EtalonData.Online.Position+elekStatus[i].EtalonData.OfflineStepRight)) && /* left steps away from Online */
			   (elekStatus[i].InstrumentFlags.EtalonAction==ETALON_ACTION_TOGGLE_OFFLINE_RIGHT) ) {      /* intend to be there */
	        OnlineOfflineCounts[Channel].OfflineRight[OfflineRightCounter % RunAverageLen ]=
				elekStatus[i].CounterCardMaster.Channel[Channel].Counts;
			if (Channel==0) OfflineRightCounter++;
			OnlineAverage[Channel].OnOffFlag[i]=1;
			/* mexPrintf("OfflineCounter #%d %d\n",Channel,OfflineCounter); */
	      
		  } /* if offline right */
		} /* if offline left */
	  } /* if on/offline */
	
	  if (OnlineCounter>5) { /* do we have enough statistics for Calc Online Avg */	  
	    OnlineOfflineCounts[Channel].OnlineSum=0;
	    maxSteps=(OnlineCounter>RunAverageLen ) ? RunAverageLen : OnlineCounter;
	  
	    for (j=0; j<maxSteps; j++) {
	      OnlineOfflineCounts[Channel].OnlineSum+=OnlineOfflineCounts[Channel].Online[j];
		} /* for j */
	    OnlineAverage[Channel].Avg[i]=OnlineOfflineCounts[Channel].OnlineSum;
	    OnlineAverage[Channel].Num[i]=j;
	  } /* if OnlineCounter */
	

	  if (OfflineLeftCounter>5) { /* do we have enough statistics for Calc Offline Left Avg */
	    OnlineOfflineCounts[Channel].OfflineLeftSum=0;
	    maxSteps=(OfflineLeftCounter>RunAverageLenOffl) ? RunAverageLenOffl : OfflineLeftCounter;
	    /*if (i<10) mexPrintf("OfflineCounter #%d %d\n",Channel,OfflineCounter); */
	    for (j=0; j<maxSteps; j++) {
	      OnlineOfflineCounts[Channel].OfflineLeftSum+=OnlineOfflineCounts[Channel].OfflineLeft[j];
		} /* for j */
	    OfflineLeftAverage[Channel].Avg[i]=OnlineOfflineCounts[Channel].OfflineLeftSum;
	    OfflineLeftAverage[Channel].Num[i]=j;
	  } /* if OfflineCounter */

      if (OfflineRightCounter>5) { /* do we have enough statistics for Calc Online Avg */
	    OnlineOfflineCounts[Channel].OfflineRightSum=0;
	    maxSteps=(OfflineRightCounter>RunAverageLenOffl) ? RunAverageLenOffl : OfflineRightCounter;
	    /*if (i<10) mexPrintf("OfflineCounter #%d %d\n",Channel,OfflineCounter); */
	    for (j=0; j<maxSteps; j++) {
	      OnlineOfflineCounts[Channel].OfflineRightSum+=OnlineOfflineCounts[Channel].OfflineRight[j];
		} /* for j */
	    OfflineRightAverage[Channel].Avg[i]=OnlineOfflineCounts[Channel].OfflineRightSum;
	    OfflineRightAverage[Channel].Num[i]=j;
	  } /* if OfflineCounter */

      

    for (Channel=1; Channel<MAX_COUNTER_CHANNEL; Channel++) {           /* for each MCP Channel */
      /* reset fields */
	  OnlineAverage[Channel].Num[i]=0;
	  OfflineLeftAverage[Channel].Num[i]=0;
	  OfflineRightAverage[Channel].Num[i]=0;
	  OnlineAverage[Channel].Avg[i]=0;
	  OfflineLeftAverage[Channel].Avg[i]=0;
	  OfflineRightAverage[Channel].Avg[i]=0;

	  OnlineAverage[Channel].OnOffFlag[i]=0; /* we don't know wether we are on=3 leftoff=2 or rightoffline=1 */
	  /* first decide if we are on or offline */
	  if ( (elekStatus[i].CounterCardSlave.Channel[0].Counts>MinRefCellCounts) &&             /* when PMTCounts>MinRefCellCounts and */
		   (abs(elekStatus[i].EtalonData.Current.Position-                  /* not further than Dithersteps of OnlienPos */
	         elekStatus[i].EtalonData.Online.Position)<=elekStatus[i].EtalonData.DitherStepWidth) &&
		   (elekStatus[i].InstrumentFlags.EtalonAction==ETALON_ACTION_TOGGLE_ONLINE_LEFT ||             /* we intend to be online */
			  elekStatus[i].InstrumentFlags.EtalonAction==ETALON_ACTION_TOGGLE_ONLINE_RIGHT)
	     
		 ) {             /* so we are online */
		OnlineOfflineCounts[Channel].Online[OnlineCounter % RunAverageLen ]=
	    elekStatus[i].CounterCardSlave.Channel[Channel].Counts;
	    if (Channel==0) OnlineCounter++;                                           /* in the case of RefCell Channel, increase number of Onlinecounts */
	    OnlineAverage[Channel].OnOffFlag[i]=3;
	  } else { /* if not, are we offline ? */
		if ( (elekStatus[i].EtalonData.Current.Position==                  /* lets see if we are left side offline */
			     (elekStatus[i].EtalonData.Online.Position-elekStatus[i].EtalonData.OfflineStepLeft)) && /* left steps away from Online */
	         (elekStatus[i].InstrumentFlags.EtalonAction==ETALON_ACTION_TOGGLE_OFFLINE_LEFT) ) {      /* intend to be there */
		  OnlineOfflineCounts[Channel].OfflineLeft[OfflineLeftCounter % RunAverageLenOffl ]=
	      elekStatus[i].CounterCardSlave.Channel[Channel].Counts;
		  if (Channel==0) OfflineLeftCounter++;
	      OnlineAverage[Channel].OnOffFlag[i]=2;
	      /* mexPrintf("OfflineCounter #%d %d\n",Channel,OfflineCounter); */
		} else { 
	    /* are we right side offline ? */
	    
	      if ( (elekStatus[i].EtalonData.Current.Position==                  /* lets see if we are right side offline */
					(elekStatus[i].EtalonData.Online.Position+elekStatus[i].EtalonData.OfflineStepRight)) && /* left steps away from Online */
			   (elekStatus[i].InstrumentFlags.EtalonAction==ETALON_ACTION_TOGGLE_OFFLINE_RIGHT) ) {      /* intend to be there */
	        OnlineOfflineCounts[Channel].OfflineRight[OfflineRightCounter % RunAverageLen ]=
				elekStatus[i].CounterCardSlave.Channel[Channel].Counts;
			if (Channel==0) OfflineRightCounter++;
			OnlineAverage[Channel].OnOffFlag[i]=1;
			/* mexPrintf("OfflineCounter #%d %d\n",Channel,OfflineCounter); */
	      
		  } /* if offline right */
		} /* if offline left */
	  } /* if on/offline */
	
	  if (OnlineCounter>5) { /* do we have enough statistics for Calc Online Avg */	  
	    OnlineOfflineCounts[Channel].OnlineSum=0;
	    maxSteps=(OnlineCounter>RunAverageLen ) ? RunAverageLen : OnlineCounter;
	  
	    for (j=0; j<maxSteps; j++) {
	      OnlineOfflineCounts[Channel].OnlineSum+=OnlineOfflineCounts[Channel].Online[j];
		} /* for j */
	    OnlineAverage[Channel].Avg[i]=OnlineOfflineCounts[Channel].OnlineSum;
	    OnlineAverage[Channel].Num[i]=j;
	  } /* if OnlineCounter */
	

	  if (OfflineLeftCounter>5) { /* do we have enough statistics for Calc Offline Left Avg */
	    OnlineOfflineCounts[Channel].OfflineLeftSum=0;
	    maxSteps=(OfflineLeftCounter>RunAverageLenOffl) ? RunAverageLenOffl : OfflineLeftCounter;
	    /*if (i<10) mexPrintf("OfflineCounter #%d %d\n",Channel,OfflineCounter); */
	    for (j=0; j<maxSteps; j++) {
	      OnlineOfflineCounts[Channel].OfflineLeftSum+=OnlineOfflineCounts[Channel].OfflineLeft[j];
		} /* for j */
	    OfflineLeftAverage[Channel].Avg[i]=OnlineOfflineCounts[Channel].OfflineLeftSum;
	    OfflineLeftAverage[Channel].Num[i]=j;
	  } /* if OfflineCounter */

      if (OfflineRightCounter>5) { /* do we have enough statistics for Calc Online Avg */
	    OnlineOfflineCounts[Channel].OfflineRightSum=0;
	    maxSteps=(OfflineRightCounter>RunAverageLenOffl) ? RunAverageLenOffl : OfflineRightCounter;
	    /*if (i<10) mexPrintf("OfflineCounter #%d %d\n",Channel,OfflineCounter); */
	    for (j=0; j<maxSteps; j++) {
	      OnlineOfflineCounts[Channel].OfflineRightSum+=OnlineOfflineCounts[Channel].OfflineRight[j];
		} /* for j */
	    OfflineRightAverage[Channel].Avg[i]=OnlineOfflineCounts[Channel].OfflineRightSum;
	    OfflineRightAverage[Channel].Num[i]=j;
	  } /* if OfflineCounter */
	} /* for Channel*/
      

  } /* for i */
    
  /* now we copy them into the array */

  for (Channel=0; Channel<MAX_COUNTER_CHANNEL; Channel++) {           /* for each Channel */

	#ifdef D_HEADER
      mexPrintf("RAvgOnline #%d : 2. Ergebnismatrix %d\n",Channel,1+countAvg/nelements);      
	#endif 
    for (i=0; i<nelements;i++) {
      if (OnlineAverage[Channel].Num[i]!=0){
		*(ptrAvg+countAvg++)=(float)OnlineAverage[Channel].Avg[i]/(float)OnlineAverage[Channel].Num[i];      
      } else {
		*(ptrAvg+countAvg++)=0;
      }
    }

	#ifdef D_HEADER
      mexPrintf("RAvgOfflineLeft #%d : 2. Ergebnismatrix %d\n",Channel,1+countAvg/nelements);      
	#endif 
    for (i=0; i<nelements;i++) {
      if (OfflineLeftAverage[Channel].Num[i]!=0){
		*(ptrAvg+countAvg++)=(float)OfflineLeftAverage[Channel].Avg[i]/(float)OfflineLeftAverage[Channel].Num[i];      
      } else {
		*(ptrAvg+countAvg++)=0;
      }
    }

	#ifdef D_HEADER
      mexPrintf("RAvgOfflineRight #%d : 2. Ergebnismatrix %d\n",Channel,1+countAvg/nelements);      
	#endif 
    for (i=0; i<nelements;i++) {
      if (OfflineRightAverage[Channel].Num[i]!=0){
		*(ptrAvg+countAvg++)=(float)OfflineRightAverage[Channel].Avg[i]/(float)OfflineRightAverage[Channel].Num[i];      
      } else {
		*(ptrAvg+countAvg++)=0;
      }      
    }

  } /* for Channel*/


  
  /* we store the Online/Offline Flag only once */
  #ifdef D_HEADER
    mexPrintf("RAvgOnOffFlag #%d : %d\n",Channel,1+count/nelements);      
  #endif 
  for (i=0; i<nelements;i++) {
    *(z+count++)=OnlineAverage[0].OnOffFlag[i];                       /* use PMT aka Ref Channel for on/offline reference */   
  }

  
  /******************* Instrument Flag Instrument Action ***************************/     
  #ifdef D_HEADER
    mexPrintf("InstrumentAction  %d %d\n",count,1+count/nelements);      
  #endif
  for (i=0; i<nelements;i++) {
    *(z+count++)=elekStatus[i].InstrumentFlags.InstrumentAction;       
  }

/******************* GPS Data Master***************************/     
  #ifdef D_HEADER
    mexPrintf("GPSsecondsUTC Master  %d %d\n",count,1+count/nelements);      
  #endif
  for (i=0; i<nelements;i++) {
    *(z+count++)=elekStatus[i].GPSDataMaster.ucUTCHours*3600+elekStatus[i].GPSDataMaster.ucUTCMins*60
	  +elekStatus[i].GPSDataMaster.ucUTCSeconds;       
  }

  #ifdef D_HEADER
    mexPrintf("GPSLongitude Master  %d %d\n",count,1+count/nelements);      
  #endif
  /* we represent the longitude 2 words */
  /* the first word contains degrees and mins */
  /* 0 means 180,0�W, 15*60+30 means 74�30'W */
  for (i=0; i<nelements;i++) {
    *(z+count++)=((int)(elekStatus[i].GPSDataMaster.dLongitude/100)+180)*60+
	  (int)(elekStatus[i].GPSDataMaster.dLongitude-((int)(elekStatus[i].GPSDataMaster.dLongitude/100))*100);       
  }
  /* the second word contains 4 post decimal positions*/
  for (i=0; i<nelements;i++) {
    *(z+count++)=(elekStatus[i].GPSDataMaster.dLongitude-(int)(elekStatus[i].GPSDataMaster.dLongitude))*10000;       
  }

  #ifdef D_HEADER
    mexPrintf("GPSLatitude Master  %d %d\n",count,1+count/nelements);      
  #endif
  /* we represent the latitude 2 words */
  /* the first word contains degrees and mins */
  /* 0 means 90,0�S, 15*60+30 means 74�30'S */
  for (i=0; i<nelements;i++) {
    *(z+count++)=((int)(elekStatus[i].GPSDataMaster.dLatitude/100)+90)*60+
	  (int)(elekStatus[i].GPSDataMaster.dLatitude-((int)(elekStatus[i].GPSDataMaster.dLatitude/100))*100);       
  }
  /* the second word contains 4 post decimal positions*/
  for (i=0; i<nelements;i++) {
    *(z+count++)=(elekStatus[i].GPSDataMaster.dLatitude-(int)(elekStatus[i].GPSDataMaster.dLatitude))*10000;       
  }
  
  #ifdef D_HEADER
    mexPrintf("GPSAltitude Master  %d %d\n",count,1+count/nelements);      
  #endif
  /* altitude above the geoid in metres */
  for (i=0; i<nelements;i++) {
    *(z+count++)=elekStatus[i].GPSDataMaster.fAltitude;
  }
 
  #ifdef D_HEADER
    mexPrintf("GPSHDOP Master  %d %d\n",count,1+count/nelements);      
  #endif
  /* Horizontal Dillution Of Precision, whatever it means....*/
  for (i=0; i<nelements;i++) {
    *(z+count++)=elekStatus[i].GPSDataMaster.fHDOP;
  }

  #ifdef D_HEADER
    mexPrintf("GPSnumSat Master  %d %d\n",count,1+count/nelements);      
  #endif
  /* number of satellites seen by the GPS receiver */
  for (i=0; i<nelements;i++) {
    *(z+count++)=elekStatus[i].GPSDataMaster.ucNumberOfSatellites;
  }

  #ifdef D_HEADER
    mexPrintf("GPSLastValidData Master  %d %d\n",count,1+count/nelements);      
  #endif
  /* number of data aquisitions (5Hz) with no valid GPS data */
  /* will stick at 255 if no data received for a long period */
  for (i=0; i<nelements;i++) {
    *(z+count++)=elekStatus[i].GPSDataMaster.ucLastValidData;
  }

  #ifdef D_HEADER
    mexPrintf("GPSGroundSpeed Master  %d %d\n",count,1+count/nelements);      
  #endif
  /* speed in cm/s above ground */
  for (i=0; i<nelements;i++) {
    *(z+count++)=elekStatus[i].GPSDataMaster.uiGroundSpeed;
  }

  #ifdef D_HEADER
    mexPrintf("GPSHeading Master  %d %d\n",count,1+count/nelements);      
  #endif
  /* 10 times heading in degrees e.g. 2700 decimal = 270,0 Degress = west */
  for (i=0; i<nelements;i++) {
    *(z+count++)=elekStatus[i].GPSDataMaster.uiHeading;
  }




  /* Date Slave*/

  #ifdef X_DEBUG  
    mexPrintf("fill year....%d elements\n",nelements);
  #endif

  
  for (i=0; i<nelements;i++) {
    Seconds=elekStatus[i].TimeOfDaySlave.tv_sec;
    #ifdef X_DEBUG  
      mexPrintf("%lu ",Seconds);
    #endif
    ptrTmZeit=(struct tm *)localtime(&Seconds); 
    #ifdef X_DEBUG  
      mexPrintf("%d ",ptrTmZeit->tm_year);
    #endif
    *(z+count++)=(uint16_t)ptrTmZeit->tm_year;
    #ifdef X_DEBUG  
      mexPrintf("%d ",i);
    #endif
  }
  
  #ifdef X_DEBUG  
    mexPrintf("fill yday....\n");
  #endif
  for (i=0; i<nelements;i++) {
    Seconds=elekStatus[i].TimeOfDaySlave.tv_sec;
    ptrTmZeit=localtime(&Seconds);
    *(z+count++)=ptrTmZeit->tm_yday;
  }
  for (i=0; i<nelements;i++) {
    Seconds=elekStatus[i].TimeOfDaySlave.tv_sec;
    ptrTmZeit=localtime(&Seconds);
    *(z+count++)=ptrTmZeit->tm_hour;
  }
  for (i=0; i<nelements;i++) {
    Seconds=elekStatus[i].TimeOfDaySlave.tv_sec;
    ptrTmZeit=localtime(&Seconds);
    *(z+count++)=ptrTmZeit->tm_min;
  }
  for (i=0; i<nelements;i++) {
    Seconds=elekStatus[i].TimeOfDaySlave.tv_sec;
    ptrTmZeit=localtime(&Seconds);
    *(z+count++)=ptrTmZeit->tm_sec;
  }
  for (i=0; i<nelements;i++) {
    *(z+count++)=elekStatus[i].TimeOfDaySlave.tv_usec/1000;
  }


  
  #ifdef X_DEBUG  
    mexPrintf("fill int16 array....\n");
  #endif


 


/******************* ADC Cards Slave ***************************/     	 
    
  for (k=0; k<MAX_ADC_CARD_LIFT; k++) {

	#ifdef D_HEADER
	  mexPrintf("NumSamples Slave %d %d\n",count,1+count/nelements);      
	#endif
    for (i=0; i<nelements;i++) 
	  *(z+count++)=elekStatus[i].ADCCardSlave[k].NumSamples;       
    
    for (j=0;j<MAX_ADC_CHANNEL_PER_CARD;j++) {       	
	  #ifdef D_HEADER
		mexPrintf("adcData Slave #%d.%d %d\n",k,j,1+count/nelements);      
	  #endif
      for (i=0; i<nelements;i++) 
	    *(z+count++)=elekStatus[i].ADCCardSlave[k].ADCChannelData[j].ADCData;       

	  #ifdef D_HEADER
		mexPrintf("adcSumDat Slave  %d %d\n",count,1+count/nelements);      
	  #endif
	  for (i=0; i<nelements;i++) 
		*(z+count++)=elekStatus[i].ADCCardSlave[k].ADCChannelData[j].SumDat;       
	
	  #ifdef D_HEADER
		mexPrintf("adcSumSqr Slave  %d %d\n",count,1+count/nelements);      
	  #endif
	  for (i=0; i<nelements;i++) 
	    *(z+count++)=elekStatus[i].ADCCardSlave[k].ADCChannelData[j].SumSqr;                              	
	}
      
    for (j=0;j<MAX_ADC_CHANNEL_PER_CARD;j++) {  
	  #ifdef D_HEADER
		mexPrintf("adcConfig Slave #%d.%d %d\n",k,j,1+count/nelements);      
	  #endif
	  for (i=0; i<nelements;i++) 
		*(z+count++)=elekStatus[i].ADCCardSlave[k].ADCChannelConfig[j].ADCChannelConfig;
	}
      
  }
    

  /*  mexPrintf("%d/%d %d %d\n",count,dims[0]*dims[1],dims[0],dims[1]);
  */


    
/******************* ADC24 Cards Slave ***************************/     	 
    
  for (k=0; k<MAX_24BIT_ADC_CARDS_WP; k++) {

	#ifdef D_HEADER
	  mexPrintf("NumSamples ADC24 Slave %d %d\n",count,1+count/nelements);      
	#endif
    for (i=0; i<nelements;i++) 
	  *(z+count++)=elekStatus[i].ADC24CardsSlave[k].NumSamples;       
    
    for (j=0;j<MAX_24BIT_ADC_CHANNEL_PER_CARD;j++) {       	
	  #ifdef D_HEADER
		mexPrintf("adc24Data Low Slave #%d.%d %d\n",k,j,1+count/nelements);      
	  #endif
      for (i=0; i<nelements;i++) 
          *(z+count++)=elekStatus[i].ADC24CardsSlave[k].ADCChannelData[j].ADCChannelDataLowHigh.ADCDataLow;
	  #ifdef D_HEADER
		mexPrintf("adc24Data High Slave #%d.%d %d\n",k,j,1+count/nelements);      
	  #endif
      for (i=0; i<nelements;i++) 
	    *(z+count++)=elekStatus[i].ADC24CardsSlave[k].ADCChannelData[j].ADCChannelDataLowHigh.ADCDataHigh;       

	}
      
      
  }
    


    
/******************* Valve Cards ***************************/     
  /* for (k=0; k<MAX_VALVE_CARD_WP; k++) { */
  /* we are only interested in Data of Card 0 */
  Card=0;
  #ifdef D_HEADER
	mexPrintf("ValveVolt Slave %d %d\n",count,1+count/nelements);      
  #endif
  for (i=0; i<nelements;i++) {
    *(z+count++)=elekStatus[i].ValveCardSlave[Card].ValveVolt;       
  }

  #ifdef D_HEADER
    mexPrintf("Valve Slave  %d %d\n",count,1+count/nelements);      
  #endif
  for (i=0; i<nelements;i++) {
    *(z+count++)=elekStatus[i].ValveCardSlave[Card].Valve;       
  } 
  /* }*/

/******************* TempSensor Card Slave ***************************/  
  /* there is only one Card */
  Card=0;   
  #ifdef D_HEADER
    mexPrintf("TempMissed Slave %d\n",1+count/nelements);      
  #endif
  for (i=0; i<nelements;i++) {
    *(z+count++)=elekStatus[i].TempSensCardSlave[Card].NumMissed;       
  }

  #ifdef D_HEADER
    mexPrintf("TempNumber Slave %d\n",1+count/nelements);      
  #endif
  for (i=0; i<nelements;i++) {
    *(z+count++)=elekStatus[i].TempSensCardSlave[Card].NumSensor;       
  }

  #ifdef D_HEADER
    mexPrintf("TempErrCRC Slave %d\n",1+count/nelements);      
  #endif
  for (i=0; i<nelements;i++) {
    *(z+count++)=elekStatus[i].TempSensCardSlave[Card].NumErrCRC;       
  }

  #ifdef D_HEADER
    mexPrintf("TempNoResponse Slave %d\n",1+count/nelements);      
  #endif
  for (i=0; i<nelements;i++) {
    *(z+count++)=elekStatus[i].TempSensCardSlave[Card].NumErrNoResponse;       
  }

  for (j=0; j<MAX_TEMP_SENSOR; j++) { 

	#ifdef D_HEADER
	  mexPrintf("Temp Slave #%d %d\n",j,1+count/nelements);      
	#endif
    /* we represent the temperature in 1/100K) */
    for (i=0; i<nelements;i++) {
	  *(z+count++)=(uint16_t) ( (elekStatus[i].TempSensCardSlave[Card].TempSensor[j].Field.TempFrac*100.0/16.0+
		elekStatus[i].TempSensCardSlave[Card].TempSensor[j].Field.TempMain*100)+27320);  
    }

	#ifdef D_HEADER
	  mexPrintf("Temp Slave #%d valid_CRCerr_noresp_alarm %d\n",j,1+count/nelements);      
	#endif
    for (i=0; i<nelements;i++) {
	  *(z+count++)=elekStatus[i].TempSensCardSlave[Card].TempSensor[j].Word.WordTemp <<12; /* shift 4 status bits to lsb position */
    }

	#ifdef D_HEADER
	  mexPrintf("Temp Slave #%d ID %d\n",j,1+count/nelements);      
	#endif
    for (i=0; i<nelements;i++) {
	  *(z+count++)=elekStatus[i].TempSensCardSlave[Card].TempSensor[j].Word.WordID[1];       
    }
  }



/******************* GPS Data Slave ***************************/     
  #ifdef D_HEADER
    mexPrintf("GPSsecondsUTC Slave  %d %d\n",count,1+count/nelements);      
  #endif
  for (i=0; i<nelements;i++) {
    *(z+count++)=elekStatus[i].GPSDataSlave.ucUTCHours*3600+elekStatus[i].GPSDataSlave.ucUTCMins*60
	  +elekStatus[i].GPSDataSlave.ucUTCSeconds;       
  }

  #ifdef D_HEADER
    mexPrintf("GPSLongitude Slave  %d %d\n",count,1+count/nelements);      
  #endif
  /* we represent the longitude 2 words */
  /* the first word contains degrees and mins */
  /* 0 means 180,0�W, 15*60+30 means 74�30'W */
  for (i=0; i<nelements;i++) {
    *(z+count++)=((int)(elekStatus[i].GPSDataSlave.dLongitude/100)+180)*60+
	  (int)(elekStatus[i].GPSDataSlave.dLongitude-((int)(elekStatus[i].GPSDataSlave.dLongitude/100))*100);       
  }
  /* the second word contains 4 post decimal positions*/
  for (i=0; i<nelements;i++) {
    *(z+count++)=(elekStatus[i].GPSDataSlave.dLongitude-(int)(elekStatus[i].GPSDataSlave.dLongitude))*10000;       
  }

  #ifdef D_HEADER
    mexPrintf("GPSLatitude Slave  %d %d\n",count,1+count/nelements);      
  #endif
  /* we represent the latitude 2 words */
  /* the first word contains degrees and mins */
  /* 0 means 90,0�S, 15*60+30 means 74�30'S */
  for (i=0; i<nelements;i++) {
    *(z+count++)=((int)(elekStatus[i].GPSDataSlave.dLatitude/100)+90)*60+
	  (int)(elekStatus[i].GPSDataSlave.dLatitude-((int)(elekStatus[i].GPSDataSlave.dLatitude/100))*100);       
  }
  /* the second word contains 4 post decimal positions*/
  for (i=0; i<nelements;i++) {
    *(z+count++)=(elekStatus[i].GPSDataSlave.dLatitude-(int)(elekStatus[i].GPSDataSlave.dLatitude))*10000;       
  }
  
  #ifdef D_HEADER
    mexPrintf("GPSAltitude Slave  %d %d\n",count,1+count/nelements);      
  #endif
  /* altitude above the geoid in metres */
  for (i=0; i<nelements;i++) {
    *(z+count++)=elekStatus[i].GPSDataSlave.fAltitude;
  }
 
  #ifdef D_HEADER
    mexPrintf("GPSHDOP Slave  %d %d\n",count,1+count/nelements);      
  #endif
  /* Horizontal Dillution Of Precision, whatever it means....*/
  for (i=0; i<nelements;i++) {
    *(z+count++)=elekStatus[i].GPSDataSlave.fHDOP;
  }

  #ifdef D_HEADER
    mexPrintf("GPSnumSat Slave  %d %d\n",count,1+count/nelements);      
  #endif
  /* number of satellites seen by the GPS receiver */
  for (i=0; i<nelements;i++) {
    *(z+count++)=elekStatus[i].GPSDataSlave.ucNumberOfSatellites;
  }

  #ifdef D_HEADER
    mexPrintf("GPSLastValidData Slave  %d %d\n",count,1+count/nelements);      
  #endif
  /* number of data aquisitions (5Hz) with no valid GPS data */
  /* will stick at 255 if no data received for a long period */
  for (i=0; i<nelements;i++) {
    *(z+count++)=elekStatus[i].GPSDataSlave.ucLastValidData;
  }

  #ifdef D_HEADER
    mexPrintf("GPSGroundSpeed Slave  %d %d\n",count,1+count/nelements);      
  #endif
  /* speed in cm/s above ground */
  for (i=0; i<nelements;i++) {
    *(z+count++)=elekStatus[i].GPSDataSlave.uiGroundSpeed;
  }

  #ifdef D_HEADER
    mexPrintf("GPSHeading Slave  %d %d\n",count,1+count/nelements);      
  #endif
  /* 10 times heading in degrees e.g. 2700 decimal = 270,0 Degress = west */
  for (i=0; i<nelements;i++) {
    *(z+count++)=elekStatus[i].GPSDataSlave.uiHeading;
  }



  
  return;
}


