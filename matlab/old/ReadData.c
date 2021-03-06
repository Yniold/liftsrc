/*=================================================================
 *
 * READData.c
 *	        
 *  Read binary data from instrument software
 * The calling syntax is:
 *
 *		[yp] = ReadData('filename')
 *
 *=================================================================*/

 /* $Revision: 1.3 $ */
#include <math.h>
#include <stdio.h>
#include <time.h>

/* #define D_HEADER     1               /* will print with mexPrintf the position of each row */
/*#define UNIX   */                     /* has to be set for unix compilation */
#ifdef UNIX
#include <stdint.h>
#else
typedef unsigned short uint16_t;
typedef unsigned uint32_t;
struct timeval {
  long    tv_sec; 
  long    tv_usec;
};

#endif

#include "mex.h"
#include "../include/elekIO.h"


/* Input Arguments */

#define	T_IN	prhs[0]
#define	Y_IN	prhs[1]


/* Output Arguments */

#define	YP_OUT	plhs[0]

#if !defined(MAX)
#define	MAX(A, B)	((A) > (B) ? (A) : (B))
#endif

#if !defined(MIN)
#define	MIN(A, B)	((A) < (B) ? (A) : (B))
#endif

#define PI 3.14159265

static	double	mu = 1/82.45;
static	double	mus = 1 - 1/82.45;


static void ReadData(
		     double	yp[],
		     double	*t,
		     double	y[]
		     )
{
  double	r1,r2;
   
  r1 = sqrt((y[0]+mu)*(y[0]+mu) + y[2]*y[2]); 
  r2 = sqrt((y[0]-mus)*(y[0]-mus) + y[2]*y[2]);
  
  /* Print warning if dividing by zero. */    
  if (r1 == 0.0 || r2 == 0.0 ){
    mexWarnMsgTxt("Division by zero!\n");
  }
  
  yp[0] = y[1];
  yp[1] = 2*y[3]+y[0]-mus*(y[0]+mu)/(r1*r1*r1)-mu*(y[0]-mus)/(r2*r2*r2);
  yp[2] = y[3];
  yp[3] = -2*y[1] + y[2] - mus*y[2]/(r1*r1*r1) - mu*y[2]/(r2*r2*r2);
  return;
}

void mexFunction( int nlhs, mxArray *plhs[], 
		  int nrhs, const mxArray*prhs[] )
     
{ 
  /*    unsigned int m,n;  */
  unsigned short *z,*ptrData;
  char *input_buf;
  int buflen;
  int status;
  int i,j,k;
  int Card;
  FILE *fp;
  long flen;
  long nelements;
  int dims[2];
  unsigned char *Databuf;
  long count;
  struct tm *ptrTmZeit;
  time_t Seconds;
  
  struct elekStatusType *elekStatus;
  
  
  /* Check for proper number of arguments */
  
  if (nrhs != 1) { 
    mexErrMsgTxt("Two input arguments required."); 
  } else if (nlhs > 1) {
    mexErrMsgTxt("Too many output arguments."); 
  } 
  
  /* Input must be a string. */
  if (mxIsChar(prhs[0]) != 1)
    mexErrMsgTxt("Input must be a string.");
  
  /* Input must be a row vector. */
  if (mxGetM(prhs[0]) != 1)
    mexErrMsgTxt("Input must be a row vector.");
  
  /* Get the length of the input string. */
  buflen = (mxGetM(prhs[0]) * mxGetN(prhs[0])) + 1;
  
  /* Allocate memory for input and output strings. */
  input_buf = mxCalloc(buflen, sizeof(char));
  
  /* Copy the string data from prhs[0] into a C string 
   * input_buf. If the string array contains several rows, 
   * they are copied, one column at a time, into one long 
   * string array. */
  status = mxGetString(prhs[0], input_buf, buflen);
  if (status != 0) 
    mexWarnMsgTxt("Not enough space. String is truncated.");  
  
  fp=fopen(input_buf,"r");
  
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
  
  elekStatus=Databuf;
  
  dims[0]= nelements;

  dims[1]= 6;              /* Jahr JulTag Stunde Minute Sekunde Mikrosek */
  dims[1]= dims[1] + ADC_CHANNEL_COUNTER_CARD+1+ 
    MAX_COUNTER_CHANNEL*(5+MAX_COUNTER_TIMESLOT+COUNTER_MASK_WIDTH);
  dims[1]= dims[1] + 12;   /* etalon data type */
  dims[1]= dims[1] + MAX_ADC_CARD*(1+MAX_ADC_CHANNEL_PER_CARD*3+MAX_ADC_CHANNEL_PER_CARD*1);
  dims[1]= dims[1] + 2;  /* mfc data */
  dims[1]= dims[1] + 2;  /* valve data */  
  dims[1]= dims[1] + MAX_DCDC4_CHANNEL_PER_CARD;  /* dcdc data */
  dims[1]= dims[1] + 4 + MAX_TEMP_SENSOR*3;  /* temp data */
  dims[1]= dims[1] + 1;  /* instrument flags */
  dims[1]= dims[1] + 3;  /* extra reserve */
  
  plhs[0] = mxCreateNumericArray(2,dims,mxUINT16_CLASS,mxREAL);
  
  /* Assign pointers to each input and output. */
  z = mxGetPr(plhs[0]);
  ptrData=z;
  count=0;
  
  /* Date */
  
  for (i=0; i<nelements;i++) {
    Seconds=elekStatus[i].TimeOfDay.tv_sec;
    ptrTmZeit=localtime(&Seconds);
    *(z+count++)=ptrTmZeit->tm_year;
  }
  
  for (i=0; i<nelements;i++) {
    Seconds=elekStatus[i].TimeOfDay.tv_sec;
    ptrTmZeit=localtime(&Seconds);
    *(z+count++)=ptrTmZeit->tm_yday;
  }
  for (i=0; i<nelements;i++) {
    Seconds=elekStatus[i].TimeOfDay.tv_sec;
    ptrTmZeit=localtime(&Seconds);
    *(z+count++)=ptrTmZeit->tm_hour;
  }
  for (i=0; i<nelements;i++) {
    Seconds=elekStatus[i].TimeOfDay.tv_sec;
    ptrTmZeit=localtime(&Seconds);
    *(z+count++)=ptrTmZeit->tm_min;
  }
  for (i=0; i<nelements;i++) {
    Seconds=elekStatus[i].TimeOfDay.tv_sec;
    ptrTmZeit=localtime(&Seconds);
    *(z+count++)=ptrTmZeit->tm_sec;
  }
  for (i=0; i<nelements;i++) {
    *(z+count++)=elekStatus[i].TimeOfDay.tv_usec/1000;
  }


  

  /******************* Counter Card ***************************/ 
  /* mexPrintf("%d %d %d\n",sizeof(uint16_t),sizeof(struct timeval),sizeof(struct elekStatusType));
  mexPrintf("%d/%d %d %d %d\n",count,dims[0]*dims[1],dims[0],dims[1],ADC_CHANNEL_COUNTER_CARD);
  */

#ifdef D_HEADER
  mexPrintf("ccADC %f\n",1+(double)count/(double)nelements);
#endif
    for (j=0;j<ADC_CHANNEL_COUNTER_CARD;j++) {
      for (i=0; i<nelements;i++) {
	*(z+count++)=elekStatus[i].CounterCard.ADCData[j];       
	/*       mexPrintf("%d %",elekStatus[i].CounterCard.ADCData[j]); */
      }	
    }

#ifdef D_HEADER
  mexPrintf("ccMasterDelay %d\n",1+count/nelements);      
#endif
     
     for (i=0; i<nelements;i++) {
	*(z+count++)=elekStatus[i].CounterCard.MasterDelay;       
      }
  

     for (j=0;j<MAX_COUNTER_CHANNEL;j++) { 
#ifdef D_HEADER
       mexPrintf("ccShiftDelay #%d %d\n",j,1+count/nelements);      
#endif
       for (i=0; i<nelements;i++) {
	 *(z+count++)=elekStatus[i].CounterCard.Channel[j].ShiftDelay;       
       }	

#ifdef D_HEADER
  mexPrintf("ccGateDelay #%d %d\n",j,1+count/nelements);      
#endif
     
     for (i=0; i<nelements;i++) {
	*(z+count++)=elekStatus[i].CounterCard.Channel[j].GateDelay;       
      }
    
#ifdef D_HEADER
  mexPrintf("ccGateWidth #%d %d\n",j,1+count/nelements);      
#endif
        
    for (i=0; i<nelements;i++) {
      *(z+count++)=elekStatus[i].CounterCard.Channel[j].GateWidth;       
    }
    
#ifdef D_HEADER
  mexPrintf("ccData #%d %d\n",j,1+count/nelements);      
#endif

    for (k=0; k<MAX_COUNTER_TIMESLOT;k++) {
        for (i=0; i<nelements;i++) {
	*(z+count++)=elekStatus[i].CounterCard.Channel[j].Data[k];       
	}	
    }
    
#ifdef D_HEADER
  mexPrintf("ccMask #%d %d\n",j,1+count/nelements);      
#endif

     for (k=0; k<COUNTER_MASK_WIDTH;k++) {
        for (i=0; i<nelements;i++) {
	  *(z+count++)=elekStatus[i].CounterCard.Channel[j].Mask[k];       
	}	
     }
    
#ifdef D_HEADER
  mexPrintf("ccCounts #%d %d\n",j,1+count/nelements);      
#endif
     for (i=0; i<nelements;i++) {
       *(z+count++)=elekStatus[i].CounterCard.Channel[j].Counts;       
     }	
    
#ifdef D_HEADER
  mexPrintf("ccPulses #%d %d\n",j,1+count/nelements);      
#endif
     for (i=0; i<nelements;i++) {
       *(z+count++)=elekStatus[i].CounterCard.Channel[j].Pulses;       
     }	   
    } 
 
      

/******************* etalon ***************************/  

#ifdef D_HEADER
  mexPrintf("etaSetPosLow %d\n",1+count/nelements);      
#endif
/* 0*/
    for (i=0; i<nelements;i++) {
      *(z+count++)=elekStatus[i].EtalonData.Set.PositionWord.Low;       
    }
#ifdef D_HEADER
  mexPrintf("etaSetPosHigh %d\n",1+count/nelements);      
#endif
/* 1*/
    for (i=0; i<nelements;i++) {
      *(z+count++)=elekStatus[i].EtalonData.Set.PositionWord.High;       
    }

#ifdef D_HEADER
  mexPrintf("etaCurPosLow %d\n",1+count/nelements);      
#endif
/* 2*/
    for (i=0; i<nelements;i++) {
      *(z+count++)=elekStatus[i].EtalonData.Current.PositionWord.Low;       
    }
#ifdef D_HEADER
  mexPrintf("etaCurPosHigh %d\n",1+count/nelements);      
#endif
/* 3*/
    for (i=0; i<nelements;i++) {
      *(z+count++)=elekStatus[i].EtalonData.Current.PositionWord.High;       
    }

#ifdef D_HEADER
  mexPrintf("etaEncoderPosLow %d\n",1+count/nelements);      
#endif
/* 4*/
    for (i=0; i<nelements;i++) {
      *(z+count++)=elekStatus[i].EtalonData.Encoder.PositionWord.Low;       
    }
#ifdef D_HEADER
  mexPrintf("etaEncoderPosHigh %d\n",1+count/nelements);      
#endif
/* 5*/
    for (i=0; i<nelements;i++) {
      *(z+count++)=elekStatus[i].EtalonData.Encoder.PositionWord.High;       
    }

#ifdef D_HEADER
  mexPrintf("etaIndexPosLow %d\n",1+count/nelements);      
#endif
/* 6*/
    for (i=0; i<nelements;i++) {
      *(z+count++)=elekStatus[i].EtalonData.Index.PositionWord.Low;       
    }
#ifdef D_HEADER
  mexPrintf("etaIndexPosHigh %d\n",1+count/nelements);      
#endif
/* 7*/
    for (i=0; i<nelements;i++) {
      *(z+count++)=elekStatus[i].EtalonData.Index.PositionWord.High;       
    }


#ifdef D_HEADER
  mexPrintf("etaSetSpd %d\n",1+count/nelements);      
#endif
/* 8*/
    for (i=0; i<nelements;i++) {
      *(z+count++)=elekStatus[i].EtalonData.SetSpeed;       
    }

#ifdef D_HEADER
  mexPrintf("etaSetAcclSpd %d\n",1+count/nelements);      
#endif
/* 9*/
    for (i=0; i<nelements;i++) {
      *(z+count++)=elekStatus[i].EtalonData.SetAccl;       
    }

#ifdef D_HEADER
  mexPrintf("etaCurSpd %d\n",1+count/nelements);      
#endif
/* 10*/
    for (i=0; i<nelements;i++) {
      *(z+count++)=elekStatus[i].EtalonData.CurSpeed;       
    }

#ifdef D_HEADER
  mexPrintf("etaStatus %d\n",1+count/nelements);      
#endif
/* 11*/
    for (i=0; i<nelements;i++) {
      *(z+count++)=elekStatus[i].EtalonData.Status;       
    }
    
    /*  mexPrintf("%d/%d\n",count,dims[1]); */
    /******************* ADC Cards ***************************/     	 
    
    for (k=0; k<MAX_ADC_CARD; k++) {

#ifdef D_HEADER
  mexPrintf("NumSamples %d\n",1+count/nelements);      
#endif
      for (i=0; i<nelements;i++) 
	*(z+count++)=elekStatus[i].ADCCard[k].NumSamples;       
    
      for (j=0;j<MAX_ADC_CHANNEL_PER_CARD;j++) {       	
#ifdef D_HEADER
  mexPrintf("adcData #%d.%d %d\n",k,j,1+count/nelements);      
#endif
        for (i=0; i<nelements;i++) 
	  *(z+count++)=elekStatus[i].ADCCard[k].ADCChannelData[j].ADCData;       
	
#ifdef D_HEADER
  mexPrintf("adcSumDat %d\n",1+count/nelements);      
#endif
	for (i=0; i<nelements;i++) 
	  *(z+count++)=elekStatus[i].ADCCard[k].ADCChannelData[j].SumDat;       
	
#ifdef D_HEADER
  mexPrintf("adcSumSqr %d\n",1+count/nelements);      
#endif
	for (i=0; i<nelements;i++) 
	  *(z+count++)=elekStatus[i].ADCCard[k].ADCChannelData[j].SumSqr;                              	
      }
      
      for (j=0;j<MAX_ADC_CHANNEL_PER_CARD;j++) {  
#ifdef D_HEADER
  mexPrintf("adcConfig #%d.%d %d\n",k,j,1+count/nelements);      
#endif
	for (i=0; i<nelements;i++) 
	  *(z+count++)=elekStatus[i].ADCCard[k].ADCChannelConfig[j].ADCChannelConfig;
      }
      
    }
    

   /*  mexPrintf("%d/%d %d %d\n",count,dims[0]*dims[1],dims[0],dims[1]);
    */


    
/******************* MFC Card ***************************/  
/* we are only interested in Data of Card 0 */
    Card=0;
#ifdef D_HEADER
  mexPrintf("MFCSetFlow %d\n",1+count/nelements);      
#endif
    for (i=0; i<nelements;i++) {
      *(z+count++)=elekStatus[i].MFCCard[Card].MFCChannelData[0].SetFlow;       
    }
#ifdef D_HEADER
    mexPrintf("MFCFlow %d\n",1+count/nelements); 
#endif
    for (i=0; i<nelements;i++) {
      *(z+count++)=elekStatus[i].MFCCard[Card].MFCChannelData[0].Flow;    
    }

/******************* Valve Cards ***************************/     
/* for (k=0; k<MAX_VALVE_CARD; k++) { */
/* we are only interested in Data of Card 0 */
    Card=0;
#ifdef D_HEADER
  mexPrintf("ValveVolt %d\n",1+count/nelements);      
#endif
    for (i=0; i<nelements;i++) {
      *(z+count++)=elekStatus[i].ValveCard[Card].ValveVolt;       
    }

#ifdef D_HEADER
  mexPrintf("Valve %d\n",1+count/nelements);      
#endif
    for (i=0; i<nelements;i++) {
      *(z+count++)=elekStatus[i].ValveCard[Card].Valve;       
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
	*(z+count++)=elekStatus[i].DCDC4Card[Card].Channel[j];       
      }
    }

/******************* TempSensor Card ***************************/  
/* we are only interested in Data of Card 0 */
    Card=0;   
#ifdef D_HEADER
  mexPrintf("TempMissed #%d %d\n",j,1+count/nelements);      
#endif
    for (i=0; i<nelements;i++) {
      *(z+count++)=elekStatus[i].TempSensCard[Card].NumMissed;       
    }

#ifdef D_HEADER
  mexPrintf("TempNumber #%d %d\n",j,1+count/nelements);      
#endif
    for (i=0; i<nelements;i++) {
      *(z+count++)=elekStatus[i].TempSensCard[Card].NumSensor;       
    }

#ifdef D_HEADER
  mexPrintf("TempErrCRC #%d %d\n",j,1+count/nelements);      
#endif
    for (i=0; i<nelements;i++) {
      *(z+count++)=elekStatus[i].TempSensCard[Card].NumErrCRC;       
    }

#ifdef D_HEADER
  mexPrintf("TempNoResponse #%d %d\n",j,1+count/nelements);      
#endif
    for (i=0; i<nelements;i++) {
      *(z+count++)=elekStatus[i].TempSensCard[Card].NumErrNoResponse;       
    }

    for (j=0; j<MAX_TEMP_SENSOR; j++) { 

#ifdef D_HEADER
  mexPrintf("Temp #%d %d\n",j,1+count/nelements);      
#endif
  /* we represent the temperature in 1/100K) */
      for (i=0; i<nelements;i++) {
	*(z+count++)=(uint16_t) ( (elekStatus[i].TempSensCard[Card].TempSensor[j].Field.TempFrac*100.0/16.0+
	  elekStatus[i].TempSensCard[Card].TempSensor[j].Field.TempMain*100)+27320);  
      }

#ifdef D_HEADER
  mexPrintf("Temp #%d valid_CRCerr_noresp_alarm %d\n",j,1+count/nelements);      
#endif
      for (i=0; i<nelements;i++) {
	*(z+count++)=elekStatus[i].TempSensCard[Card].TempSensor[j].Word.WordTemp & 17;       
      }

#ifdef D_HEADER
  mexPrintf("Temp #%d ID %d\n",j,1+count/nelements);      
#endif
      for (i=0; i<nelements;i++) {
	*(z+count++)=elekStatus[i].TempSensCard[Card].TempSensor[j].Word.WordID[1];       
      }
    }

/******************* Instrument Flags ***************************/     
#ifdef D_HEADER
  mexPrintf("EtalonAction %d\n",1+count/nelements);      
#endif
    for (i=0; i<nelements;i++) {
      *(z+count++)=elekStatus[i].InstrumentFlags.EtalonAction;       
    }

#ifdef D_HEADER
  mexPrintf("etaOnlinePosLow %d\n",1+count/nelements);      
#endif
/* 6*/
    for (i=0; i<nelements;i++) {
      *(z+count++)=elekStatus[i].EtalonData.Online.PositionWord.Low;       
    }
#ifdef D_HEADER
  mexPrintf("etaOnlinePosHigh %d\n",1+count/nelements);      
#endif
/* 7*/
    for (i=0; i<nelements;i++) {
      *(z+count++)=elekStatus[i].EtalonData.Online.PositionWord.High;       
    }

    return;
}


