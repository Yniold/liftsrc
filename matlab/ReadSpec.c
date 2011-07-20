/*=================================================================
 *
 * ReadSpec.c
 *	        
 *  Read binary spectrometer data from instrument software
 * The calling syntax is:
 *
 *   [statusdata,Avgdata] = ReadSpec('filename')
 *
 *=================================================================*/
 
#include <math.h>
#include <stdio.h>
#include <time.h>

/* #define D_HEADER     1               /* will print with mexPrintf the position of each row */
/*#define UNIX   */                     /* has to be set for unix compilation */


#ifdef UNIX                             /* there are some differences between windows & linux header files */
#include <stdint.h>
#else
typedef unsigned short uint16_t;
typedef short int16_t;
typedef unsigned uint32_t;
typedef int int32_t;

typedef unsigned __int64 uint64_t;


#endif

#include "mex.h"
#include "../include/elekIO.h"



int cmptimesort(const void *ptrspecStatus1,
                const void *ptrspecStatus2) {
        
    int ret;
    ret=0;
    if (((struct spectralStatusType*)ptrspecStatus1)->TimeOfDaySpectra.tv_sec>((struct spectralStatusType*)ptrspecStatus2)->TimeOfDaySpectra.tv_sec) ret=1;
    if (((struct spectralStatusType*)ptrspecStatus2)->TimeOfDaySpectra.tv_sec>((struct spectralStatusType*)ptrspecStatus1)->TimeOfDaySpectra.tv_sec) ret=-1;
    if (ret==0) { /* same second, now look for usec */
      if (((struct spectralStatusType*)ptrspecStatus1)->TimeOfDaySpectra.tv_usec>((struct spectralStatusType*)ptrspecStatus2)->TimeOfDaySpectra.tv_usec) ret=1;
      if (((struct spectralStatusType*)ptrspecStatus2)->TimeOfDaySpectra.tv_usec>((struct spectralStatusType*)ptrspecStatus1)->TimeOfDaySpectra.tv_usec) ret=-1;
    } /* if ret */
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
  FILE *fp;
  long flen;
  long nelements;
  int dims[2];
  unsigned char *Databuf;
  long count;
  struct tm *ptrTmZeit;
  time_t Seconds;
  
  struct spectralStatusType *specStatus;
  
  #ifdef X_DEBUG  
    mexPrintf("sizeof struct spectralStatusType %d",sizeof(struct spectralStatusType));
  #endif
  
  /* Check for proper number of arguments */
  
  if (nrhs != 1) { 
    mexPrintf("This is ReadDataAvg CVS: $RCSfile: ReadSpec.c,v $ $Revision: 1.1 $ \n");
    mexErrMsgTxt("one input argument required: Filename"); 
  } 
  
  /* Input must be a string. */
  if (mxIsChar(prhs[0]) != 1) {
    mexPrintf("This is ReadDataAvg CVS: $RCSfile: ReadSpec.c,v $ $Revision: 1.1 $ \n");
    mexErrMsgTxt("Input must be a string.");
}  
  /* Input must be a row vector. */
  if (mxGetM(prhs[0]) != 1) {
    mexPrintf("This is ReadDataAvg CVS: $RCSfile: ReadSpec.c,v $ $Revision: 1.1 $ \n");
    mexErrMsgTxt("Input must be a row vector.");
  }
  
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
  

  fp=fopen(input_buf,"rb");
  
  if (fp==NULL) 
    mexErrMsgTxt("can't open file");
  
  fseek(fp,0,SEEK_END);
  flen=ftell(fp);
   nelements=flen/sizeof(struct spectralStatusType);
 
  if(flen % sizeof(struct spectralStatusType) != 0)
  {
	  mexPrintf(" struct size : %d mod file size = %d, number of elements : %d sizeof unsigned char %d \n",sizeof(struct spectralStatusType), flen % sizeof(struct spectralStatusType),nelements,sizeof(unsigned char));
	  mexPrintf(" enum size 1: %d enum size2 = %d, \n",sizeof(enum InstrumentActionType),sizeof(enum EtalonActionType));

	  mexPrintf("This is $Id: ReadSpec.c,v 1.1 2007-11-09 15:26:03 martinez Exp $ \n");
		mexErrMsgTxt("File size is not a multiple of structure size, please select a proper .bin file!");
  }


  /* we have to allocate some space, use mxCalloc, matlab is cleaning up memory upon exit */
  Databuf=mxCalloc(flen, sizeof(char));
  /* go to the beginning */
  fseek(fp,0,SEEK_SET);
  
  status=fread(Databuf,sizeof(struct spectralStatusType),nelements,fp);
  fclose(fp);
  
  specStatus=(struct spectralStatusType*)Databuf;
  
  dims[0]= nelements;

  dims[1]= 6 * 2;              /* Jahr JulTag Stunde Minute Sekunde Mikrosek f?r Spectrometer und Status*/
  dims[1]= dims[1] + 2;		/* Min and Max Wavelength */
  dims[1]= dims[1] + 12;   /* etalon data type */
  dims[1]= dims[1] + 2*MAXSPECTRALLINES;	/* spectrum */
  dims[1]= dims[1] + 1;  /* EtalonAction */
  dims[1]= dims[1] + 1;  /* EtalonEncoderPos high */
  dims[1]= dims[1] + 1;  /* EtalonEncoderPos low */

  dims[1]= dims[1] + 3;  /* extra reserve */

  plhs[0] = mxCreateNumericArray(2,dims,mxUINT16_CLASS,mxREAL);
 
  /* Assign pointers to each input and output. */
  z = mxGetPr(plhs[0]);
  ptrData=z;
  count=0;
  
  /* first we sort the data according to time */
  
  #ifdef X_DEBUG  
    mexPrintf("sort....\n");
  #endif
  qsort(specStatus,nelements,sizeof(struct spectralStatusType),cmptimesort);
  
  #ifdef X_DEBUG  
    mexPrintf("copy counts....\n");
  #endif


  #ifdef X_DEBUG  
    mexPrintf("fill year....%f elements\n",nelements);
  #endif
  #ifdef D_HEADER
    mexPrintf("Time Spectra %5.0f\n",1+(double)count/(double)nelements);
  #endif

  
  for (i=0; i<nelements;i++) {
    Seconds=specStatus[i].TimeOfDaySpectra.tv_sec;
    #ifdef X_DEBUG  
      mexPrintf("%lu ",Seconds);
    #endif
    ptrTmZeit=(struct tm *)gmtime(&Seconds); 
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
    Seconds=specStatus[i].TimeOfDaySpectra.tv_sec;
    ptrTmZeit=gmtime(&Seconds);
    *(z+count++)=ptrTmZeit->tm_yday;
  }
  for (i=0; i<nelements;i++) {
    Seconds=specStatus[i].TimeOfDaySpectra.tv_sec;
    ptrTmZeit=gmtime(&Seconds);
    *(z+count++)=ptrTmZeit->tm_hour;
  }
  for (i=0; i<nelements;i++) {
    Seconds=specStatus[i].TimeOfDaySpectra.tv_sec;
    ptrTmZeit=gmtime(&Seconds);
    *(z+count++)=ptrTmZeit->tm_min;
  }
  for (i=0; i<nelements;i++) {
    Seconds=specStatus[i].TimeOfDaySpectra.tv_sec;
    ptrTmZeit=gmtime(&Seconds);
    *(z+count++)=ptrTmZeit->tm_sec;
  }
  for (i=0; i<nelements;i++) {
    *(z+count++)=specStatus[i].TimeOfDaySpectra.tv_usec/1000;
  }


  #ifdef X_DEBUG  
    mexPrintf("fill status yday....\n");
  #endif
  #ifdef D_HEADER
    mexPrintf("Time Status %5.0f\n",1+(double)count/(double)nelements);
  #endif
  for (i=0; i<nelements;i++) {
    Seconds=specStatus[i].TimeOfDayStatus.tv_sec;
    ptrTmZeit=gmtime(&Seconds);
    *(z+count++)=ptrTmZeit->tm_yday;
  }
  for (i=0; i<nelements;i++) {
    Seconds=specStatus[i].TimeOfDayStatus.tv_sec;
    ptrTmZeit=gmtime(&Seconds);
    *(z+count++)=ptrTmZeit->tm_hour;
  }
  for (i=0; i<nelements;i++) {
    Seconds=specStatus[i].TimeOfDayStatus.tv_sec;
    ptrTmZeit=gmtime(&Seconds);
    *(z+count++)=ptrTmZeit->tm_min;
  }
  for (i=0; i<nelements;i++) {
    Seconds=specStatus[i].TimeOfDayStatus.tv_sec;
    ptrTmZeit=gmtime(&Seconds);
    *(z+count++)=ptrTmZeit->tm_sec;
  }
  for (i=0; i<nelements;i++) {
    *(z+count++)=specStatus[i].TimeOfDayStatus.tv_usec/1000;
  }

  
  #ifdef X_DEBUG  
    mexPrintf("fill int16 array....\n");
  #endif

  #ifdef D_HEADER
    mexPrintf("MinWaveLength %5.0f\n",1+(double)count/(double)nelements);
  #endif
    for (i=0; i<nelements;i++) {
	*(z+count++)=specStatus[i].uiMinWaveLength;       
    }	
  
  #ifdef D_HEADER
    mexPrintf("MaxWaveLength %5.0f\n",1+(double)count/(double)nelements);
  #endif
    for (i=0; i<nelements;i++) {
	*(z+count++)=specStatus[i].uiMaxWaveLength;       
    }	

  #ifdef D_HEADER
    mexPrintf("etaEncoderPosLow  %d\n",1+count/nelements);      
  #endif
  for (i=0; i<nelements;i++) {
    *(z+count++)=specStatus[i].Encoder.PositionWord.Low;       
  }

  #ifdef D_HEADER
    mexPrintf("etaEncoderPosHigh  %d\n",1+count/nelements);      
  #endif
  for (i=0; i<nelements;i++) {
    *(z+count++)=specStatus[i].Encoder.PositionWord.High;       
  }

  #ifdef D_HEADER
    mexPrintf("etalonAction %d\n",1+count/nelements);      
  #endif
  for (i=0; i<nelements;i++) {
    *(z+count++)=specStatus[i].CurrentEtalonAction;       
  }


#ifdef D_HEADER
    mexPrintf("Spectral Data WaveLengths %5.0f\n",1+(double)count/(double)nelements);
  #endif
  for (j=0;j<MAXSPECTRALLINES;j++) {
    for (i=0; i<nelements;i++) {
	*(z+count++)=specStatus[i].SpectralData[j].uiWaveLength;       
    }	
  }

  #ifdef D_HEADER
    mexPrintf("Spectral Data Counts %5.0f\n",1+(double)count/(double)nelements);
  #endif
  for (j=0;j<MAXSPECTRALLINES;j++) {
    for (i=0; i<nelements;i++) {
	*(z+count++)=specStatus[i].SpectralData[j].uiCounts;       
    }	
  }

  return;
}


