/*
 * ReadCalibData.c
 *	        read binary data files from calibrator
 *
 *   [statusdata] = ReadCalibData('filename')
 *
 * $Id: ReadCalibData.c,v 1.1 2007-03-20 08:03:34 rudolf Exp $
 * $Log: ReadCalibData.c,v $
 * Revision 1.1  2007-03-20 08:03:34  rudolf
 * first try to read structures directly into Matlab, work in progress
 *
 *
 *
 *
 *
 */

#include <math.h>
#include <stdio.h>
#include <time.h>

#define X_DEBUG

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

struct timeval {
    long    tv_sec;
    long    tv_usec;
};
#endif

#include "mex.h"
#include "../include/elekIO.h"

#define NUMBER_OF_TIMEVAL_FIELDS (2)
#define NUMBER_OF_STRUCT_FIELDS (7)

const char *struct_field_names[] = 
{"TimeOfDayCalib",                               
"ADCCardCalib",             
"MFCCardCalib",             
"SCRCardCalib",               
"TempSensCardCalib",
"LicorCalib",                                   
"PIDRegulator"};                                 

const char *timeval_field_names[] = 
{"tv_sec", 
"tv_usec"};

int dims[2];
int small_dims[2];

int i, tv_sec_field, tv_usec_field, time_val_field_no;

mxArray *pTimeValArray;
mxArray *TimeOfDayCalibFieldValue;

uint32_t* pValue;

/* function for sorting structure by timestamp in case we
 * are reading the running status file calibstatus.bin
 */

int cmptimesort(const void *ptrCalibStatus1, const void *ptrCalibStatus2)
{
    int ret;
    ret=0;
    if (((struct calibStatusType*)ptrCalibStatus1)->TimeOfDayCalib.tv_sec>((struct calibStatusType*)ptrCalibStatus2)->TimeOfDayCalib.tv_sec) ret=1;
    if (((struct calibStatusType*)ptrCalibStatus2)->TimeOfDayCalib.tv_sec>((struct calibStatusType*)ptrCalibStatus1)->TimeOfDayCalib.tv_sec) ret=-1;
    if (ret==0) { /* same second, now look for usec */
        if (((struct calibStatusType*)ptrCalibStatus1)->TimeOfDayCalib.tv_usec>((struct calibStatusType*)ptrCalibStatus2)->TimeOfDayCalib.tv_usec) ret=1;
        if (((struct calibStatusType*)ptrCalibStatus2)->TimeOfDayCalib.tv_usec>((struct calibStatusType*)ptrCalibStatus1)->TimeOfDayCalib.tv_usec) ret=-1;
    } /* if ret */
    return (ret);
}

void mexFunction( int nlhs, mxArray *plhs[],
int nrhs, const mxArray*prhs[] )

{
  /*    unsigned int m,n;  */
    unsigned short *z,*ptrData;
    float *ptrAvg;
    char *input_buf;
    int bulFileLength;
    int status;
    int i,j,k;
    int Card;
    FILE *fp;
    long lFileLength;
    long lDataSets;
    int dims[2];
    int dimaverages[2];
    unsigned char *pDataBuf;
    long count, countAvg;
    struct tm *ptrTmZeit;
    time_t Seconds;
    
    struct calibStatusType *ptrCalibStatus;
    
  /* Check for proper number of arguments */
    if (nrhs != 1)
    {
        mexPrintf("This is $Id: ReadCalibData.c,v 1.1 2007-03-20 08:03:34 rudolf Exp $ \n");
        mexErrMsgTxt("input argument required: FILENAME.CAL");
    }
    
  /* Input must be a string. */
    if (mxIsChar(prhs[0]) != 1) {
        mexPrintf("This is $Id: ReadCalibData.c,v 1.1 2007-03-20 08:03:34 rudolf Exp $  \n");
        mexErrMsgTxt("Input must be a string.");
    }
  /* Input must be a row vector. */
    if (mxGetM(prhs[0]) != 1) {
        mexPrintf("This is $Id: ReadCalibData.c,v 1.1 2007-03-20 08:03:34 rudolf Exp $ \n");
        mexErrMsgTxt("Input must be a row vector.");
    }
    
    if(nlhs > 1)
    {
        mexErrMsgTxt("Too many output arguments.");
    };
  /* Get the length of the input string. */
    bulFileLength = (mxGetM(prhs[0]) * mxGetN(prhs[0])) + 1;
    
  /* Allocate memory for input and output strings. */
    input_buf = mxCalloc(bulFileLength, sizeof(char));
    
  /* Copy the string data from prhs[0] into a C string
   * input_buf. If the string array contains several rows,
   * they are copied, one column at a time, into one long
   * string array. */
    status = mxGetString(prhs[0], input_buf, bulFileLength);
    if (status != 0)
        mexWarnMsgTxt("Not enough space. String is truncated.");
    
    
    fp=fopen(input_buf,"rb");
    
    if (fp==NULL)
        mexErrMsgTxt("can't open file");
    
    fseek(fp,0,SEEK_END);
    lFileLength=ftell(fp);
    
    #ifdef X_DEBUG
    mexPrintf("Filelength: %ld\n",lFileLength);
    mexPrintf("Structuresize: %d\n",sizeof(struct calibStatusType));
    #endif
    
  /* test if the filesize is a multiple of the structure size, else complain */
    
    if(lFileLength % sizeof(struct calibStatusType) != 0)
    {
        mexPrintf("This is $Id: ReadCalibData.c,v 1.1 2007-03-20 08:03:34 rudolf Exp $ \n");
        mexErrMsgTxt("File size is not a multiple of structure size, please select a proper .cal file!");
    }
  /* we have to allocate some space, use mxCalloc, matlab is cleaning up memory upon exit */
    pDataBuf = mxCalloc(lFileLength, sizeof(char));
    lDataSets = lFileLength / sizeof(struct calibStatusType);
  /* go to the beginning */
    fseek(fp,0,SEEK_SET);
    
    status=fread(pDataBuf,sizeof(struct calibStatusType),lDataSets,fp);
    fclose(fp);
    
    ptrCalibStatus=(struct calibStatusType*)pDataBuf;
    
    qsort(ptrCalibStatus,lDataSets,sizeof(struct calibStatusType),cmptimesort);
    
    #ifdef X_DEBUG
    mexPrintf("This is $Id: ReadCalibData.c,v 1.1 2007-03-20 08:03:34 rudolf Exp $ \n");
    mexPrintf("Read %ld datasets from file %s\n",lDataSets,input_buf);
    #endif
    
  /*******************/
  /* create matrices */
  /*******************/
    
    dims[0] = 1;
    dims[1] = (int) lDataSets;
    
    small_dims[0] = 1;
    small_dims[1] = 1;
    
    /* 1x7 structure with field names */
    plhs[0] = mxCreateStructArray(2, dims, NUMBER_OF_STRUCT_FIELDS, struct_field_names);
    
    /* Create a 1-by-n array of structs. */
    pTimeValArray = mxCreateStructArray(2, dims, NUMBER_OF_TIMEVAL_FIELDS, timeval_field_names);
    
    tv_sec_field = mxGetFieldNumber(pTimeValArray,"tv_sec");
    tv_usec_field = mxGetFieldNumber(pTimeValArray,"tv_usec");
    
    /* check for error */
    if(tv_sec_field < 0)
        mexErrMsgTxt("tv_sec: mxGetFieldNumber() failed\n");
    if(tv_usec_field < 0)
        mexErrMsgTxt("tv_usec: mxGetFieldNumber() failed\n");
    
    /* try to populate each field with a value */
    for (i=0; i<dims[1]; i++)
    {
        mxArray *tv_sec_field_value;
        mxArray *tv_usec_field_value;
        
        tv_sec_field_value = mxCreateNumericArray(2, small_dims, mxUINT32_CLASS, mxREAL);
        tv_usec_field_value = mxCreateNumericArray(2, small_dims, mxUINT32_CLASS, mxREAL);
        
        /* get a pointer to the data field of the 1x1 "Array" */
        
        pValue = (uint32_t*) mxGetData(tv_sec_field_value);
        *pValue = ptrCalibStatus->TimeOfDayCalib.tv_sec;
        
        pValue = (uint32_t*) mxGetData(tv_usec_field_value);
        *pValue = (uint32_t)ptrCalibStatus->TimeOfDayCalib.tv_usec;
        
        mxSetFieldByNumber(pTimeValArray,i,tv_sec_field,tv_sec_field_value);
        mxSetFieldByNumber(pTimeValArray,i,tv_usec_field,tv_usec_field_value);
        ptrCalibStatus++;
    }
    
    /* link main struct array to substruct array */
    time_val_field_no = mxGetFieldNumber(plhs[0],"TimeOfDayCalib");
    mxSetFieldByNumber(plhs[0],0,time_val_field_no,pTimeValArray);
}

