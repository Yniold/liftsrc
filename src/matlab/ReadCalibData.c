/*
 * ReadCalibData.c
 *	        read binary data files from calibrator
 *
 *   [statusdata] = ReadCalibData('filename')
 *
 * $Id: ReadCalibData.c,v 1.6 2007-07-13 08:04:54 martinez Exp $
 *
 * $Log: ReadCalibData.c,v $
 * Revision 1.6  2007-07-13 08:04:54  martinez
 * commented out redefinition of timeval
 *
 * Revision 1.5  2007-07-12 17:51:52  rudolf
 * LICOR channels can also be negative, changed uint16 to int16
 *
 * Revision 1.4  2007-07-12 17:45:44  martinez
 * corrected errors
 *
 * Revision 1.3  2007-06-11 14:59:21  rudolf
 * added c file for reading binary data files into MATLAB structures
 *
 *
 *
 *
 *
 */

#include <math.h>
#include <stdio.h>
#include <time.h>
#include <string.h>

/*
#define X_DEBUG 
*/

/* #define D_HEADER     1               /* will print with mexPrintf the position of each row */
/*#define UNIX   */                     /* has to be set for unix compilation */

#ifdef UNIX                             /* there are some differences between windows & linux header files */
#include <stdint.h>
#else
typedef unsigned char uint8_t;
typedef signed char int8_t;
typedef unsigned short uint16_t;
typedef short int16_t;
typedef unsigned uint32_t;
typedef int int32_t;

typedef unsigned __int64 uint64_t;

/*struct timeval {
    long    tv_sec;
    long    tv_usec;
};
 */
#endif

#include "mex.h"
#include "../include/elekIO.h"

/* for field names, see elekIO.h */
#define NUMBER_OF_STRUCT_FIELDS (7)
const char *struct_field_names[] = 
{"TimeOfDayCalib",                               
 "ADCCardCalib",             
 "MFCCardCalib",             
 "SCRCardCalib",               
 "TempSensCardCalib",
 "LicorCalib",                                   
 "PIDRegulator"
};                                 
mxArray *pTimeOfDayCalibArray;
mxArray *pADCCardCalibArray;
mxArray *pMFCCardCalibArray;
mxArray *pSCRCardCalibArray;
mxArray *pTempSensCardCalibArray;
mxArray *pLicorCalibArray;
mxArray *pPidRegulatorArray;

#define NUMBER_OF_TIMEVAL_FIELDS (2)
const char *timeval_field_names[] = 
{"tv_sec", 
 "tv_usec"
};
mxArray *pTimeValTvSecArray;
mxArray *pTimeValTvUsecArray;

#define NUMBER_OF_LICORSTATUS_FIELDS (9)
const char *licorstatus_field_names[]=
{"Status",
"LicorTemperature",
"AmbientPressure",
"CO2A",
"CO2B",
"CO2D",
"H2OA",
"H2OB",
"H2OD"};

mxArray *pLicorStatusStatusArray;
mxArray *pLicorStatusLicorTemperatureArray;
mxArray *pLicorStatusAmbientPressureArray;
mxArray *pLicorStatusCO2AArray;
mxArray *pLicorStatusCO2BArray;
mxArray *pLicorStatusCO2DArray;
mxArray *pLicorStatusH2OAArray;
mxArray *pLicorStatusH2OBArray;
mxArray *pLicorStatusH2ODArray;

#define NUMBER_OF_PIDREGULATOR_FIELDS (7)
const char *pidregulator_field_names[]=
{"Setpoint",
"ActualValueH2O",
"ActualValueHeater",
"KP",
"KI",
"KD",
"ControlValue"};

mxArray *pPidRegulatorSetpointArray;
mxArray *pPidRegulatorActualValueH2O;
mxArray *pPidRegulatorActualValueHeater;
mxArray *pPidRegulatorKP;
mxArray *pPidRegulatorKI;
mxArray *pPidRegulatorKD;
mxArray *pPidRegulatorControlValue;

#define NUMBER_OF_MFC_FIELDS (9)
const char *mfc_field_names[]=
{"NumSamples",
"ChannelData0",
"ChannelData1",
"ChannelData2",
"ChannelData3",
"ChannelConfig0",
"ChannelConfig1",
"ChannelConfig2",
"ChannelConfig3"};

mxArray *pNumSamplesArray;
mxArray *pChannelData0Array;
mxArray *pChannelData1Array;
mxArray *pChannelData2Array;
mxArray *pChannelData3Array;
mxArray *pChannelConfig0Array;
mxArray *pChannelConfig1Array;
mxArray *pChannelConfig2Array;
mxArray *pChannelConfig3Array;

#define NUMBER_OF_MFC_DATA_FIELDS (4)
const char *mfc_data_field_names[]=
{"SetFlow",
"Flow",
"SumDat",
"SumSqr"};

mxArray *pSetFlowArray;
mxArray *pFlowArray;
mxArray *pSumDatArray;
mxArray *pSumSqrArray;

#define NUMBER_OF_SCR_POWER_FIELDS (3)
const char *scr_power_field_names[]=
{"SCRPowerValue0",
"SCRPowerValue1",
"SCRPowerValue2"};

mxArray *pSCRPowerValue0Array;
mxArray *pSCRPowerValue1Array;
mxArray *pSCRPowerValue2Array;

#define NUMBER_OF_MFC_CONFIG_FIELDS (1)
const char *mfc_config_field_names[]=
{"MFCChannelConfig"};

mxArray *pMFCChannelConfigArray;

int dims[2];
int small_dims[2];

int i,iArrayIndex;

int iElement;
void* pValue;
struct calibStatusType* pTempPointer;
uint32_t* pU32Target;
uint16_t* pU16Target;
int16_t* pI16Target;
uint8_t* pU08Target;
uint8_t* pBoolTarget;
uint8_t ucTrue = 1;
uint8_t ucFalse = 0;

double* pDblTarget;

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
    char *input_buf;
    int bulFileLength;
    int status;
    FILE *fp;
    long lFileLength;
    long lDataSets;
    int dimaverages[2];
    unsigned char *pDataBuf;
    time_t Seconds;
    
    struct calibStatusType *ptrCalibStatus;
    
  /* Check for proper number of arguments */
    if (nrhs != 1)
    {
        mexPrintf("This is $Id: ReadCalibData.c,v 1.6 2007-07-13 08:04:54 martinez Exp $ \n");
        mexErrMsgTxt("input argument required: FILENAME.CAL");
    }
    
  /* Input must be a string. */
    if (mxIsChar(prhs[0]) != 1) {
        mexPrintf("This is $Id: ReadCalibData.c,v 1.6 2007-07-13 08:04:54 martinez Exp $\n");
        mexErrMsgTxt("Input must be a string.");
    }
  /* Input must be a row vector. */
    if (mxGetM(prhs[0]) != 1) {
        mexPrintf("This is $Id: ReadCalibData.c,v 1.6 2007-07-13 08:04:54 martinez Exp $\n");
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
	mexPrintf("distributed into:\n");
	mexPrintf("Structuresize: timeval :%d\n",sizeof(struct timeval));
	mexPrintf("Structuresize: ADCCardType :%d\n",sizeof(struct ADCCardType));
	mexPrintf("Structuresize: ADCChannelDataType :%d\n",sizeof(struct ADCChannelDataType));
	mexPrintf("Structuresize: ADCChannelConfigBitType :%d\n\n",sizeof(struct ADCChannelConfigBitType));

	mexPrintf("Structuresize: MFCCardType :%d\n",sizeof(struct MFCCardType));
	mexPrintf("Structuresize: SCRCardType :%d\n",sizeof(struct SCRCardType));
	mexPrintf("Structuresize: TempSensorCardType :%d\n",sizeof(struct TempSensorCardType));
	mexPrintf("Structuresize: LicorH2OCO2Type :%d\n",sizeof(struct LicorH2OCO2Type));
	mexPrintf("Structuresize: PIDregulatorType :%d\n",sizeof(struct PIDregulatorType));
    #endif
    
  /* test if the filesize is a multiple of the structure size, else complain */
    
    if(lFileLength % sizeof(struct calibStatusType) != 0)
    {
        mexPrintf("This is $Id: ReadCalibData.c,v 1.6 2007-07-13 08:04:54 martinez Exp $\n");
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
    mexPrintf("This is $Id: ReadCalibData.c,v 1.6 2007-07-13 08:04:54 martinez Exp $\n");
    mexPrintf("Read %ld datasets from file %s\n",lDataSets,input_buf);
#endif
    

  /*******************/
  /* create matrices */
  /*******************/
    
    dims[1] = 1;
    dims[0] = (int) lDataSets;
    
    small_dims[0] = 1;
    small_dims[1] = 1;
    
    /* 1x7 structure with field names */
    plhs[0] = mxCreateStructArray(2, small_dims, NUMBER_OF_STRUCT_FIELDS, struct_field_names);

	/*******************************/
	/* SUBSTRUCTURE TimeOfDayCalib */
	/*******************************/

	pTimeOfDayCalibArray = mxCreateStructArray(2, small_dims, NUMBER_OF_TIMEVAL_FIELDS, timeval_field_names);

	/*******************************************/
	/* 1st field of substruct TimeOfDayCalib   */
	/*******************************************/

    /* create 1xM matrix with proper data type */
    pTimeValTvSecArray = mxCreateNumericArray(2, dims, mxUINT32_CLASS, mxREAL);
    
    /* determine at which index to hook in the whole array */
    iArrayIndex = mxGetFieldNumber(pTimeOfDayCalibArray,"tv_sec");
    
    /* allocate heap for all elements, DON'T USE VARIABLES ON STACK !!! */
    pValue = mxCalloc(lDataSets,sizeof(uint32_t));

	pTempPointer = ptrCalibStatus;
	pU32Target = pValue;

	for(iElement = 0; iElement < lDataSets; iElement++)
	{
		/* copy the values into the allocated heap space */
		memcpy((void*)pU32Target,&(pTempPointer->TimeOfDayCalib.tv_sec),sizeof(uint32_t));
		pTempPointer++;
		pU32Target++;
	}
    /* let the pointer of the array point to the heap space with our data */
    mxSetData(pTimeValTvSecArray,pValue);
    mxSetFieldByNumber(pTimeOfDayCalibArray,0,iArrayIndex,pTimeValTvSecArray);

	/*******************************************/
	/* 2nd field of substruct TimeOfDayCalib   */
	/*******************************************/

    /* create 1xM matrix with proper data type */
    pTimeValTvUsecArray = mxCreateNumericArray(2, dims, mxUINT32_CLASS, mxREAL);
    
    /* determine at which index to hook in the whole array */
    iArrayIndex = mxGetFieldNumber(pTimeOfDayCalibArray,"tv_usec");
    
    /* allocate heap for all elements, DON'T USE VARIABLES ON STACK !!! */
    pValue = mxCalloc(lDataSets,sizeof(uint32_t));

	pTempPointer = ptrCalibStatus;
	pU32Target = pValue;

	for(iElement = 0; iElement < lDataSets; iElement++)
	{
		/* copy the values into the allocated heap space */
		memcpy((void*)pU32Target,&(pTempPointer->TimeOfDayCalib.tv_usec),sizeof(uint32_t));
		pTempPointer++;
		pU32Target++;
	}
    /* let the pointer of the array point to the heap space with our data */
    mxSetData(pTimeValTvUsecArray,pValue);
    mxSetFieldByNumber(pTimeOfDayCalibArray,0,iArrayIndex,pTimeValTvUsecArray);

	/*******************************/
	/* SUBSTRUCTURE LicorCalib */
	/*******************************/

	pLicorCalibArray = mxCreateStructArray(2, small_dims, NUMBER_OF_LICORSTATUS_FIELDS, licorstatus_field_names);

	/*******************************************/
	/* 1st field of substruct LicorCalib   */
	/*******************************************/

    /* create 1xM matrix with proper data type */
    pLicorStatusStatusArray = mxCreateNumericArray(2, dims, mxUINT16_CLASS, mxREAL);
    
    /* determine at which index to hook in the whole array */
    iArrayIndex = mxGetFieldNumber(pLicorCalibArray,"Status");
    
    /* allocate heap for all elements, DON'T USE VARIABLES ON STACK !!! */
    pValue = mxCalloc(lDataSets,sizeof(uint16_t));

	pTempPointer = ptrCalibStatus;
	pU16Target = pValue;

	for(iElement = 0; iElement < lDataSets; iElement++)
	{
		/* copy the values into the allocated heap space */
		memcpy((void*)pU16Target,&(pTempPointer->LicorCalib.Status.Word),sizeof(uint16_t));
		pTempPointer++;
		pU16Target++;
	}
    /* let the pointer of the array point to the heap space with our data */
    mxSetData(pLicorStatusStatusArray,pValue);
    mxSetFieldByNumber(pLicorCalibArray,0,iArrayIndex,pLicorStatusStatusArray);

	/*******************************************/
	/* 2nd field of substruct LicorCalib   */
	/*******************************************/

    /* create 1xM matrix with proper data type */
    pLicorStatusLicorTemperatureArray = mxCreateNumericArray(2, dims, mxUINT16_CLASS, mxREAL);
    
    /* determine at which index to hook in the whole array */
    iArrayIndex = mxGetFieldNumber(pLicorCalibArray,"LicorTemperature");
    
    /* allocate heap for all elements, DON'T USE VARIABLES ON STACK !!! */
    pValue = mxCalloc(lDataSets,sizeof(uint16_t));

	pTempPointer = ptrCalibStatus;
	pU16Target = pValue;

	for(iElement = 0; iElement < lDataSets; iElement++)
	{
		/* copy the values into the allocated heap space */
		memcpy((void*)pU16Target,&(pTempPointer->LicorCalib.LicorTemperature),sizeof(uint16_t));
		pTempPointer++;
		pU16Target++;
	}
    /* let the pointer of the array point to the heap space with our data */
    mxSetData(pLicorStatusLicorTemperatureArray,pValue);
    mxSetFieldByNumber(pLicorCalibArray,0,iArrayIndex,pLicorStatusLicorTemperatureArray);
    
	/*******************************************/
	/* 3rd field of substruct LicorCalib   */
	/*******************************************/

    /* create 1xM matrix with proper data type */
    pLicorStatusAmbientPressureArray = mxCreateNumericArray(2, dims, mxUINT16_CLASS, mxREAL);
    
    /* determine at which index to hook in the whole array */
    iArrayIndex = mxGetFieldNumber(pLicorCalibArray,"AmbientPressure");
    
    /* allocate heap for all elements, DON'T USE VARIABLES ON STACK !!! */
    pValue = mxCalloc(lDataSets,sizeof(uint16_t));

	pTempPointer = ptrCalibStatus;
	pU16Target = pValue;

	for(iElement = 0; iElement < lDataSets; iElement++)
	{
		/* copy the values into the allocated heap space */
		memcpy((void*)pU16Target,&(pTempPointer->LicorCalib.AmbientPressure),sizeof(uint16_t));
		pTempPointer++;
		pU16Target++;
	}
    /* let the pointer of the array point to the heap space with our data */
    mxSetData(pLicorStatusAmbientPressureArray,pValue);
    mxSetFieldByNumber(pLicorCalibArray,0,iArrayIndex,pLicorStatusAmbientPressureArray);

	/*******************************************/
	/* 4th field of substruct LicorCalib   */
	/*******************************************/

    /* create 1xM matrix with proper data type */
    pLicorStatusCO2AArray = mxCreateNumericArray(2, dims, mxINT16_CLASS, mxREAL);
    
    /* determine at which index to hook in the whole array */
    iArrayIndex = mxGetFieldNumber(pLicorCalibArray,"CO2A");
    
    /* allocate heap for all elements, DON'T USE VARIABLES ON STACK !!! */
    pValue = mxCalloc(lDataSets,sizeof(int16_t));

	pTempPointer = ptrCalibStatus;
	pI16Target = pValue;

	for(iElement = 0; iElement < lDataSets; iElement++)
	{
		/* copy the values into the allocated heap space */
		memcpy((void*)pI16Target,&(pTempPointer->LicorCalib.CO2A),sizeof(int16_t));
		pTempPointer++;
		pI16Target++;
	}
    /* let the pointer of the array point to the heap space with our data */
    mxSetData(pLicorStatusCO2AArray,pValue);
    mxSetFieldByNumber(pLicorCalibArray,0,iArrayIndex,pLicorStatusCO2AArray);

	/*******************************************/
	/* 5th field of substruct LicorCalib   */
	/*******************************************/

    /* create 1xM matrix with proper data type */
    pLicorStatusCO2BArray = mxCreateNumericArray(2, dims, mxINT16_CLASS, mxREAL);
    
    /* determine at which index to hook in the whole array */
    iArrayIndex = mxGetFieldNumber(pLicorCalibArray,"CO2B");
    
    /* allocate heap for all elements, DON'T USE VARIABLES ON STACK !!! */
    pValue = mxCalloc(lDataSets,sizeof(int16_t));

	pTempPointer = ptrCalibStatus;
	pI16Target = pValue;

	for(iElement = 0; iElement < lDataSets; iElement++)
	{
		/* copy the values into the allocated heap space */
		memcpy((void*)pI16Target,&(pTempPointer->LicorCalib.CO2B),sizeof(int16_t));
		pTempPointer++;
		pI16Target++;
	}
    /* let the pointer of the array point to the heap space with our data */
    mxSetData(pLicorStatusCO2BArray,pValue);
    mxSetFieldByNumber(pLicorCalibArray,0,iArrayIndex,pLicorStatusCO2BArray);

	/*******************************************/
	/* 6th field of substruct LicorCalib   */
	/*******************************************/

    /* create 1xM matrix with proper data type */
    pLicorStatusCO2DArray = mxCreateNumericArray(2, dims, mxINT16_CLASS, mxREAL);
    
    /* determine at which index to hook in the whole array */
    iArrayIndex = mxGetFieldNumber(pLicorCalibArray,"CO2D");
    
    /* allocate heap for all elements, DON'T USE VARIABLES ON STACK !!! */
    pValue = mxCalloc(lDataSets,sizeof(int16_t));

	pTempPointer = ptrCalibStatus;
	pI16Target = pValue;

	for(iElement = 0; iElement < lDataSets; iElement++)
	{
		/* copy the values into the allocated heap space */
		memcpy((void*)pI16Target,&(pTempPointer->LicorCalib.CO2D),sizeof(int16_t));
		pTempPointer++;
		pI16Target++;
	}
    /* let the pointer of the array point to the heap space with our data */
    mxSetData(pLicorStatusCO2DArray,pValue);
    mxSetFieldByNumber(pLicorCalibArray,0,iArrayIndex,pLicorStatusCO2DArray);

	/*******************************************/
	/* 7th field of substruct LicorCalib   */
	/*******************************************/

    /* create 1xM matrix with proper data type */
    pLicorStatusH2OAArray = mxCreateNumericArray(2, dims, mxINT16_CLASS, mxREAL);
    
    /* determine at which index to hook in the whole array */
    iArrayIndex = mxGetFieldNumber(pLicorCalibArray,"H2OA");
    
    /* allocate heap for all elements, DON'T USE VARIABLES ON STACK !!! */
    pValue = mxCalloc(lDataSets,sizeof(int16_t));

	pTempPointer = ptrCalibStatus;
	pI16Target = pValue;

	for(iElement = 0; iElement < lDataSets; iElement++)
	{
		/* copy the values into the allocated heap space */
		memcpy((void*)pI16Target,&(pTempPointer->LicorCalib.H2OA),sizeof(int16_t));
		pTempPointer++;
		pI16Target++;
	}
    /* let the pointer of the array point to the heap space with our data */
    mxSetData(pLicorStatusH2OAArray,pValue);
    mxSetFieldByNumber(pLicorCalibArray,0,iArrayIndex,pLicorStatusH2OAArray);

	/*******************************************/
	/* 8th field of substruct LicorCalib   */
	/*******************************************/

    /* create 1xM matrix with proper data type */
    pLicorStatusH2OBArray = mxCreateNumericArray(2, dims, mxINT16_CLASS, mxREAL);
    
    /* determine at which index to hook in the whole array */
    iArrayIndex = mxGetFieldNumber(pLicorCalibArray,"H2OB");
    
    /* allocate heap for all elements, DON'T USE VARIABLES ON STACK !!! */
    pValue = mxCalloc(lDataSets,sizeof(int16_t));

	pTempPointer = ptrCalibStatus;
	pI16Target = pValue;

	for(iElement = 0; iElement < lDataSets; iElement++)
	{
		/* copy the values into the allocated heap space */
		memcpy((void*)pI16Target,&(pTempPointer->LicorCalib.H2OB),sizeof(int16_t));
		pTempPointer++;
		pI16Target++;
	}
    /* let the pointer of the array point to the heap space with our data */
    mxSetData(pLicorStatusH2OBArray,pValue);
    mxSetFieldByNumber(pLicorCalibArray,0,iArrayIndex,pLicorStatusH2OBArray);

	/*******************************************/
	/* 9th field of substruct LicorCalib   */
	/*******************************************/

    /* create 1xM matrix with proper data type */
    pLicorStatusH2ODArray = mxCreateNumericArray(2, dims, mxINT16_CLASS, mxREAL);
    
    /* determine at which index to hook in the whole array */
    iArrayIndex = mxGetFieldNumber(pLicorCalibArray,"H2OD");
    
    /* allocate heap for all elements, DON'T USE VARIABLES ON STACK !!! */
    pValue = mxCalloc(lDataSets,sizeof(int16_t));

	pTempPointer = ptrCalibStatus;
	pI16Target = pValue;

	for(iElement = 0; iElement < lDataSets; iElement++)
	{
		/* copy the values into the allocated heap space */
		memcpy((void*)pI16Target,&(pTempPointer->LicorCalib.H2OD),sizeof(int16_t));
		pTempPointer++;
		pI16Target++;
	}
    /* let the pointer of the array point to the heap space with our data */
    mxSetData(pLicorStatusH2ODArray,pValue);
    mxSetFieldByNumber(pLicorCalibArray,0,iArrayIndex,pLicorStatusH2ODArray);

	/*****************************/
	/* SUBSTRUCTURE PIDRegulator */
	/*****************************/

	pPidRegulatorArray = mxCreateStructArray(2, small_dims, NUMBER_OF_PIDREGULATOR_FIELDS, pidregulator_field_names);

	/*****************************************/
	/* 1st field of substruct PIDRegulator   */
	/*****************************************/

    /* create 1xM matrix with proper data type */
    pPidRegulatorSetpointArray = mxCreateNumericArray(2, dims, mxUINT16_CLASS, mxREAL);
    
    /* determine at which index to hook in the whole array */
    iArrayIndex = mxGetFieldNumber(pPidRegulatorArray,"Setpoint");
    
    /* allocate heap for all elements, DON'T USE VARIABLES ON STACK !!! */
    pValue = mxCalloc(lDataSets,sizeof(uint16_t));

	pTempPointer = ptrCalibStatus;
	pU16Target = pValue;

	for(iElement = 0; iElement < lDataSets; iElement++)
	{
		/* copy the values into the allocated heap space */
		memcpy((void*)pU16Target,&(pTempPointer->PIDRegulator.Setpoint),sizeof(uint16_t));
		pTempPointer++;
		pU16Target++;
	}
    /* let the pointer of the array point to the heap space with our data */
    mxSetData(pPidRegulatorSetpointArray,pValue);
    mxSetFieldByNumber(pPidRegulatorArray,0,iArrayIndex,pPidRegulatorSetpointArray);

	/*****************************************/
	/* 2nd field of substruct PIDRegulator   */
	/*****************************************/

    /* create 1xM matrix with proper data type */
    pPidRegulatorActualValueH2O = mxCreateNumericArray(2, dims, mxUINT16_CLASS, mxREAL);
    
    /* determine at which index to hook in the whole array */
    iArrayIndex = mxGetFieldNumber(pPidRegulatorArray,"ActualValueH2O");
    
    /* allocate heap for all elements, DON'T USE VARIABLES ON STACK !!! */
    pValue = mxCalloc(lDataSets,sizeof(uint16_t));

	pTempPointer = ptrCalibStatus;
	pU16Target = pValue;

	for(iElement = 0; iElement < lDataSets; iElement++)
	{
		/* copy the values into the allocated heap space */
		memcpy((void*)pU16Target,&(pTempPointer->PIDRegulator.ActualValueH2O),sizeof(uint16_t));
		pTempPointer++;
		pU16Target++;
	}
    /* let the pointer of the array point to the heap space with our data */
    mxSetData(pPidRegulatorActualValueH2O,pValue);
    mxSetFieldByNumber(pPidRegulatorArray,0,iArrayIndex,pPidRegulatorActualValueH2O);

	/*****************************************/
	/* 3rd field of substruct PIDRegulator   */
	/*****************************************/

    /* create 1xM matrix with proper data type */
    pPidRegulatorActualValueHeater = mxCreateNumericArray(2, dims, mxUINT16_CLASS, mxREAL);
    
    /* determine at which index to hook in the whole array */
    iArrayIndex = mxGetFieldNumber(pPidRegulatorArray,"ActualValueHeater");
    
    /* allocate heap for all elements, DON'T USE VARIABLES ON STACK !!! */
    pValue = mxCalloc(lDataSets,sizeof(uint16_t));

	pTempPointer = ptrCalibStatus;
	pU16Target = pValue;

	for(iElement = 0; iElement < lDataSets; iElement++)
	{
		/* copy the values into the allocated heap space */
		memcpy((void*)pU16Target,&(pTempPointer->PIDRegulator.ActualValueHeater),sizeof(uint16_t));
		pTempPointer++;
		pU16Target++;
	}
    /* let the pointer of the array point to the heap space with our data */
    mxSetData(pPidRegulatorActualValueHeater,pValue);
    mxSetFieldByNumber(pPidRegulatorArray,0,iArrayIndex,pPidRegulatorActualValueHeater);

	/*****************************************/
	/* 4th field of substruct PIDRegulator   */
	/*****************************************/

    /* create 1xM matrix with proper data type */
    pPidRegulatorKP = mxCreateNumericArray(2, dims, mxUINT16_CLASS, mxREAL);
    
    /* determine at which index to hook in the whole array */
    iArrayIndex = mxGetFieldNumber(pPidRegulatorArray,"KP");
    
    /* allocate heap for all elements, DON'T USE VARIABLES ON STACK !!! */
    pValue = mxCalloc(lDataSets,sizeof(uint16_t));

	pTempPointer = ptrCalibStatus;
	pU16Target = pValue;

	for(iElement = 0; iElement < lDataSets; iElement++)
	{
		/* copy the values into the allocated heap space */
		memcpy((void*)pU16Target,&(pTempPointer->PIDRegulator.KP),sizeof(uint16_t));
		pTempPointer++;
		pU16Target++;
	}
    /* let the pointer of the array point to the heap space with our data */
    mxSetData(pPidRegulatorKP,pValue);
    mxSetFieldByNumber(pPidRegulatorArray,0,iArrayIndex,pPidRegulatorKP);

	/*****************************************/
	/* 5th field of substruct PIDRegulator   */
	/*****************************************/

    /* create 1xM matrix with proper data type */
    pPidRegulatorKI = mxCreateNumericArray(2, dims, mxUINT16_CLASS, mxREAL);
    
    /* determine at which index to hook in the whole array */
    iArrayIndex = mxGetFieldNumber(pPidRegulatorArray,"KI");
    
    /* allocate heap for all elements, DON'T USE VARIABLES ON STACK !!! */
    pValue = mxCalloc(lDataSets,sizeof(uint16_t));

	pTempPointer = ptrCalibStatus;
	pU16Target = pValue;

	for(iElement = 0; iElement < lDataSets; iElement++)
	{
		/* copy the values into the allocated heap space */
		memcpy((void*)pU16Target,&(pTempPointer->PIDRegulator.KI),sizeof(uint16_t));
		pTempPointer++;
		pU16Target++;
	}
    /* let the pointer of the array point to the heap space with our data */
    mxSetData(pPidRegulatorKI,pValue);
    mxSetFieldByNumber(pPidRegulatorArray,0,iArrayIndex,pPidRegulatorKI);

	/*****************************************/
	/* 6th field of substruct PIDRegulator   */
	/*****************************************/

    /* create 1xM matrix with proper data type */
    pPidRegulatorKD = mxCreateNumericArray(2, dims, mxUINT16_CLASS, mxREAL);
    
    /* determine at which index to hook in the whole array */
    iArrayIndex = mxGetFieldNumber(pPidRegulatorArray,"KD");
    
    /* allocate heap for all elements, DON'T USE VARIABLES ON STACK !!! */
    pValue = mxCalloc(lDataSets,sizeof(uint16_t));

	pTempPointer = ptrCalibStatus;
	pU16Target = pValue;

	for(iElement = 0; iElement < lDataSets; iElement++)
	{
		/* copy the values into the allocated heap space */
		memcpy((void*)pU16Target,&(pTempPointer->PIDRegulator.KD),sizeof(uint16_t));
		pTempPointer++;
		pU16Target++;
	}
    /* let the pointer of the array point to the heap space with our data */
    mxSetData(pPidRegulatorKD,pValue);
    mxSetFieldByNumber(pPidRegulatorArray,0,iArrayIndex,pPidRegulatorKD);

	/*****************************************/
	/* 7th field of substruct PIDRegulator   */
	/*****************************************/

    /* create 1xM matrix with proper data type */
    pPidRegulatorControlValue = mxCreateNumericArray(2, dims, mxUINT16_CLASS, mxREAL);
    
    /* determine at which index to hook in the whole array */
    iArrayIndex = mxGetFieldNumber(pPidRegulatorArray,"ControlValue");
    
    /* allocate heap for all elements, DON'T USE VARIABLES ON STACK !!! */
    pValue = mxCalloc(lDataSets,sizeof(uint16_t));

	pTempPointer = ptrCalibStatus;
	pU16Target = pValue;

	for(iElement = 0; iElement < lDataSets; iElement++)
	{
		/* copy the values into the allocated heap space */
		memcpy((void*)pU16Target,&(pTempPointer->PIDRegulator.ControlValue),sizeof(uint16_t));
		pTempPointer++;
		pU16Target++;
	}
    /* let the pointer of the array point to the heap space with our data */
    mxSetData(pPidRegulatorControlValue,pValue);
    mxSetFieldByNumber(pPidRegulatorArray,0,iArrayIndex,pPidRegulatorControlValue);

	/*****************************/
	/* SUBSTRUCTURE MFCCardCalib */
	/*****************************/

	pMFCCardCalibArray = mxCreateStructArray(2, small_dims, NUMBER_OF_MFC_FIELDS, mfc_field_names);

	pChannelData0Array = mxCreateStructArray(2, small_dims, NUMBER_OF_MFC_DATA_FIELDS, mfc_data_field_names);
	iArrayIndex = mxGetFieldNumber(pMFCCardCalibArray,"ChannelData0");
    mxSetFieldByNumber(pMFCCardCalibArray,0,iArrayIndex,pChannelData0Array);

	pChannelData1Array = mxCreateStructArray(2, small_dims, NUMBER_OF_MFC_DATA_FIELDS, mfc_data_field_names);
	iArrayIndex = mxGetFieldNumber(pMFCCardCalibArray,"ChannelData1");
    mxSetFieldByNumber(pMFCCardCalibArray,0,iArrayIndex,pChannelData1Array);

	pChannelData2Array = mxCreateStructArray(2, small_dims, NUMBER_OF_MFC_DATA_FIELDS, mfc_data_field_names);
	iArrayIndex = mxGetFieldNumber(pMFCCardCalibArray,"ChannelData2");
    mxSetFieldByNumber(pMFCCardCalibArray,0,iArrayIndex,pChannelData2Array);

	pChannelData3Array = mxCreateStructArray(2, small_dims, NUMBER_OF_MFC_DATA_FIELDS, mfc_data_field_names);
	iArrayIndex = mxGetFieldNumber(pMFCCardCalibArray,"ChannelData3");
    mxSetFieldByNumber(pMFCCardCalibArray,0,iArrayIndex,pChannelData3Array);

	pChannelConfig0Array = mxCreateStructArray(2, small_dims, NUMBER_OF_MFC_CONFIG_FIELDS, mfc_config_field_names);
	iArrayIndex = mxGetFieldNumber(pMFCCardCalibArray,"ChannelConfig0");
    mxSetFieldByNumber(pMFCCardCalibArray,0,iArrayIndex,pChannelConfig0Array);

	pChannelConfig1Array = mxCreateStructArray(2, small_dims, NUMBER_OF_MFC_CONFIG_FIELDS, mfc_config_field_names);
	iArrayIndex = mxGetFieldNumber(pMFCCardCalibArray,"ChannelConfig1");
    mxSetFieldByNumber(pMFCCardCalibArray,0,iArrayIndex,pChannelConfig1Array);

	pChannelConfig2Array = mxCreateStructArray(2, small_dims, NUMBER_OF_MFC_CONFIG_FIELDS, mfc_config_field_names);
	iArrayIndex = mxGetFieldNumber(pMFCCardCalibArray,"ChannelConfig2");
    mxSetFieldByNumber(pMFCCardCalibArray,0,iArrayIndex,pChannelConfig2Array);

	pChannelConfig3Array = mxCreateStructArray(2, small_dims, NUMBER_OF_MFC_CONFIG_FIELDS, mfc_config_field_names);
	iArrayIndex = mxGetFieldNumber(pMFCCardCalibArray,"ChannelConfig3");
    mxSetFieldByNumber(pMFCCardCalibArray,0,iArrayIndex,pChannelConfig3Array);

	/*****************************************/
	/* 1st field of substruct MFCCardCalib   */
	/*****************************************/

    /* create 1xM matrix with proper data type */
    pNumSamplesArray = mxCreateNumericArray(2, dims, mxUINT16_CLASS, mxREAL);
    
    /* determine at which index to hook in the whole array */
    iArrayIndex = mxGetFieldNumber(pMFCCardCalibArray,"NumSamples");
    
    /* allocate heap for all elements, DON'T USE VARIABLES ON STACK !!! */
    pValue = mxCalloc(lDataSets,sizeof(uint16_t));

	pTempPointer = ptrCalibStatus;
	pU16Target = pValue;

	for(iElement = 0; iElement < lDataSets; iElement++)
	{
		/* copy the values into the allocated heap space */
		memcpy((void*)pU16Target,&(pTempPointer->MFCCardCalib[0].NumSamples),sizeof(uint16_t));
		pTempPointer++;
		pU16Target++;
	}
    /* let the pointer of the array point to the heap space with our data */
    mxSetData(pNumSamplesArray,pValue);
    mxSetFieldByNumber(pMFCCardCalibArray,0,iArrayIndex,pNumSamplesArray);

	/*****************************************/
	/* 2nd field of substruct MFCCardCalib   */
	/*****************************************/

    /* create 1xM matrix with proper data type */
    pSetFlowArray = mxCreateNumericArray(2, dims, mxUINT16_CLASS, mxREAL);
    
    /* determine at which index to hook in the whole array */
    iArrayIndex = mxGetFieldNumber(pChannelData0Array,"SetFlow");
    
    /* allocate heap for all elements, DON'T USE VARIABLES ON STACK !!! */
    pValue = mxCalloc(lDataSets,sizeof(uint16_t));

	pTempPointer = ptrCalibStatus;
	pU16Target = pValue;

	for(iElement = 0; iElement < lDataSets; iElement++)
	{
		/* copy the values into the allocated heap space */
		memcpy((void*)pU16Target,&(pTempPointer->MFCCardCalib[0].MFCChannelData[0].SetFlow),sizeof(uint16_t));
		pTempPointer++;
		pU16Target++;
	}
    /* let the pointer of the array point to the heap space with our data */
    mxSetData(pSetFlowArray,pValue);
    mxSetFieldByNumber(pChannelData0Array,0,iArrayIndex,pSetFlowArray);

	/*****************************************/
	/* 3rd field of substruct MFCCardCalib   */
	/*****************************************/

    /* create 1xM matrix with proper data type */
    pSetFlowArray = mxCreateNumericArray(2, dims, mxUINT16_CLASS, mxREAL);
    
    /* determine at which index to hook in the whole array */
    iArrayIndex = mxGetFieldNumber(pChannelData1Array,"SetFlow");
    
    /* allocate heap for all elements, DON'T USE VARIABLES ON STACK !!! */
    pValue = mxCalloc(lDataSets,sizeof(uint16_t));

	pTempPointer = ptrCalibStatus;
	pU16Target = pValue;

	for(iElement = 0; iElement < lDataSets; iElement++)
	{
		/* copy the values into the allocated heap space */
		memcpy((void*)pU16Target,&(pTempPointer->MFCCardCalib[0].MFCChannelData[1].SetFlow),sizeof(uint16_t));
		pTempPointer++;
		pU16Target++;
	}
    /* let the pointer of the array point to the heap space with our data */
    mxSetData(pSetFlowArray,pValue);
    mxSetFieldByNumber(pChannelData1Array,0,iArrayIndex,pSetFlowArray);

	/*****************************************/
	/* 4th field of substruct MFCCardCalib   */
	/*****************************************/

    /* create 1xM matrix with proper data type */
    pSetFlowArray = mxCreateNumericArray(2, dims, mxUINT16_CLASS, mxREAL);
    
    /* determine at which index to hook in the whole array */
    iArrayIndex = mxGetFieldNumber(pChannelData2Array,"SetFlow");
    
    /* allocate heap for all elements, DON'T USE VARIABLES ON STACK !!! */
    pValue = mxCalloc(lDataSets,sizeof(uint16_t));

	pTempPointer = ptrCalibStatus;
	pU16Target = pValue;

	for(iElement = 0; iElement < lDataSets; iElement++)
	{
		/* copy the values into the allocated heap space */
		memcpy((void*)pU16Target,&(pTempPointer->MFCCardCalib[0].MFCChannelData[2].SetFlow),sizeof(uint16_t));
		pTempPointer++;
		pU16Target++;
	}
    /* let the pointer of the array point to the heap space with our data */
    mxSetData(pSetFlowArray,pValue);
    mxSetFieldByNumber(pChannelData2Array,0,iArrayIndex,pSetFlowArray);

	/*****************************************/
	/* 5th field of substruct MFCCardCalib   */
	/*****************************************/

    /* create 1xM matrix with proper data type */
    pSetFlowArray = mxCreateNumericArray(2, dims, mxUINT16_CLASS, mxREAL);
    
    /* determine at which index to hook in the whole array */
    iArrayIndex = mxGetFieldNumber(pChannelData3Array,"SetFlow");
    
    /* allocate heap for all elements, DON'T USE VARIABLES ON STACK !!! */
    pValue = mxCalloc(lDataSets,sizeof(uint16_t));

	pTempPointer = ptrCalibStatus;
	pU16Target = pValue;

	for(iElement = 0; iElement < lDataSets; iElement++)
	{
		/* copy the values into the allocated heap space */
		memcpy((void*)pU16Target,&(pTempPointer->MFCCardCalib[0].MFCChannelData[3].SetFlow),sizeof(uint16_t));
		pTempPointer++;
		pU16Target++;
	}
    /* let the pointer of the array point to the heap space with our data */
    mxSetData(pSetFlowArray,pValue);
    mxSetFieldByNumber(pChannelData3Array,0,iArrayIndex,pSetFlowArray);

	/*****************************************/
	/* 6th field of substruct MFCCardCalib   */
	/*****************************************/

    /* create 1xM matrix with proper data type */
    pFlowArray = mxCreateNumericArray(2, dims, mxUINT16_CLASS, mxREAL);
    
    /* determine at which index to hook in the whole array */
    iArrayIndex = mxGetFieldNumber(pChannelData0Array,"Flow");
    
    /* allocate heap for all elements, DON'T USE VARIABLES ON STACK !!! */
    pValue = mxCalloc(lDataSets,sizeof(uint16_t));

	pTempPointer = ptrCalibStatus;
	pU16Target = pValue;

	for(iElement = 0; iElement < lDataSets; iElement++)
	{
		/* copy the values into the allocated heap space */
		memcpy((void*)pU16Target,&(pTempPointer->MFCCardCalib[0].MFCChannelData[0].Flow),sizeof(uint16_t));
		pTempPointer++;
		pU16Target++;
	}
    /* let the pointer of the array point to the heap space with our data */
    mxSetData(pFlowArray,pValue);
    mxSetFieldByNumber(pChannelData0Array,0,iArrayIndex,pFlowArray);

	/*****************************************/
	/* 7th field of substruct MFCCardCalib   */
	/*****************************************/

    /* create 1xM matrix with proper data type */
    pFlowArray = mxCreateNumericArray(2, dims, mxUINT16_CLASS, mxREAL);
    
    /* determine at which index to hook in the whole array */
    iArrayIndex = mxGetFieldNumber(pChannelData1Array,"Flow");
    
    /* allocate heap for all elements, DON'T USE VARIABLES ON STACK !!! */
    pValue = mxCalloc(lDataSets,sizeof(uint16_t));

	pTempPointer = ptrCalibStatus;
	pU16Target = pValue;

	for(iElement = 0; iElement < lDataSets; iElement++)
	{
		/* copy the values into the allocated heap space */
		memcpy((void*)pU16Target,&(pTempPointer->MFCCardCalib[0].MFCChannelData[1].Flow),sizeof(uint16_t));
		pTempPointer++;
		pU16Target++;
	}
    /* let the pointer of the array point to the heap space with our data */
    mxSetData(pFlowArray,pValue);
    mxSetFieldByNumber(pChannelData1Array,0,iArrayIndex,pFlowArray);

	/*****************************************/
	/* 8th field of substruct MFCCardCalib   */
	/*****************************************/

    /* create 1xM matrix with proper data type */
    pFlowArray = mxCreateNumericArray(2, dims, mxUINT16_CLASS, mxREAL);
    
    /* determine at which index to hook in the whole array */
    iArrayIndex = mxGetFieldNumber(pChannelData2Array,"Flow");
    
    /* allocate heap for all elements, DON'T USE VARIABLES ON STACK !!! */
    pValue = mxCalloc(lDataSets,sizeof(uint16_t));

	pTempPointer = ptrCalibStatus;
	pU16Target = pValue;

	for(iElement = 0; iElement < lDataSets; iElement++)
	{
		/* copy the values into the allocated heap space */
		memcpy((void*)pU16Target,&(pTempPointer->MFCCardCalib[0].MFCChannelData[2].Flow),sizeof(uint16_t));
		pTempPointer++;
		pU16Target++;
	}
    /* let the pointer of the array point to the heap space with our data */
    mxSetData(pFlowArray,pValue);
    mxSetFieldByNumber(pChannelData2Array,0,iArrayIndex,pFlowArray);

	/*****************************************/
	/* 9th field of substruct MFCCardCalib   */
	/*****************************************/

    /* create 1xM matrix with proper data type */
    pFlowArray = mxCreateNumericArray(2, dims, mxUINT16_CLASS, mxREAL);
    
    /* determine at which index to hook in the whole array */
    iArrayIndex = mxGetFieldNumber(pChannelData3Array,"Flow");
    
    /* allocate heap for all elements, DON'T USE VARIABLES ON STACK !!! */
    pValue = mxCalloc(lDataSets,sizeof(uint16_t));

	pTempPointer = ptrCalibStatus;
	pU16Target = pValue;

	for(iElement = 0; iElement < lDataSets; iElement++)
	{
		/* copy the values into the allocated heap space */
		memcpy((void*)pU16Target,&(pTempPointer->MFCCardCalib[0].MFCChannelData[3].Flow),sizeof(uint16_t));
		pTempPointer++;
		pU16Target++;
	}
    /* let the pointer of the array point to the heap space with our data */
    mxSetData(pFlowArray,pValue);
    mxSetFieldByNumber(pChannelData3Array,0,iArrayIndex,pFlowArray);

	/******************************************/
	/* 10th field of substruct MFCCardCalib   */
	/******************************************/

    /* create 1xM matrix with proper data type */
    pSetFlowArray = mxCreateNumericArray(2, dims, mxUINT16_CLASS, mxREAL);
    
    /* determine at which index to hook in the whole array */
    iArrayIndex = mxGetFieldNumber(pChannelData3Array,"SetFlow");
    
    /* allocate heap for all elements, DON'T USE VARIABLES ON STACK !!! */
    pValue = mxCalloc(lDataSets,sizeof(uint16_t));

	pTempPointer = ptrCalibStatus;
	pU16Target = pValue;

	for(iElement = 0; iElement < lDataSets; iElement++)
	{
		/* copy the values into the allocated heap space */
		memcpy((void*)pU16Target,&(pTempPointer->MFCCardCalib[0].MFCChannelData[3].SetFlow),sizeof(uint16_t));
		pTempPointer++;
		pU16Target++;
	}
    /* let the pointer of the array point to the heap space with our data */
    mxSetData(pSetFlowArray,pValue);
    mxSetFieldByNumber(pChannelData3Array,0,iArrayIndex,pSetFlowArray);

	/******************************************/

	/******************************************/
	/* 11th field of substruct MFCCardCalib   */
	/******************************************/

    /* create 1xM matrix with proper data type */
    pFlowArray = mxCreateNumericArray(2, dims, mxUINT32_CLASS, mxREAL);
    
    /* determine at which index to hook in the whole array */
    iArrayIndex = mxGetFieldNumber(pChannelData0Array,"SumDat");
    
    /* allocate heap for all elements, DON'T USE VARIABLES ON STACK !!! */
    pValue = mxCalloc(lDataSets,sizeof(uint32_t));

	pTempPointer = ptrCalibStatus;
	pU32Target = pValue;

	for(iElement = 0; iElement < lDataSets; iElement++)
	{
		/* copy the values into the allocated heap space */
		memcpy((void*)pU32Target,&(pTempPointer->MFCCardCalib[0].MFCChannelData[0].SumDat),sizeof(uint32_t));
		pTempPointer++;
		pU32Target++;
	}
    /* let the pointer of the array point to the heap space with our data */
    mxSetData(pFlowArray,pValue);
    mxSetFieldByNumber(pChannelData0Array,0,iArrayIndex,pFlowArray);

	/******************************************/
	/* 12th field of substruct MFCCardCalib   */
	/******************************************/

    /* create 1xM matrix with proper data type */
    pFlowArray = mxCreateNumericArray(2, dims, mxUINT32_CLASS, mxREAL);
    
    /* determine at which index to hook in the whole array */
    iArrayIndex = mxGetFieldNumber(pChannelData1Array,"SumDat");
    
    /* allocate heap for all elements, DON'T USE VARIABLES ON STACK !!! */
    pValue = mxCalloc(lDataSets,sizeof(uint32_t));

	pTempPointer = ptrCalibStatus;
	pU32Target = pValue;

	for(iElement = 0; iElement < lDataSets; iElement++)
	{
		/* copy the values into the allocated heap space */
		memcpy((void*)pU32Target,&(pTempPointer->MFCCardCalib[0].MFCChannelData[1].SumDat),sizeof(uint32_t));
		pTempPointer++;
		pU32Target++;
	}
    /* let the pointer of the array point to the heap space with our data */
    mxSetData(pFlowArray,pValue);
    mxSetFieldByNumber(pChannelData1Array,0,iArrayIndex,pFlowArray);

	/******************************************/
	/* 13th field of substruct MFCCardCalib   */
	/******************************************/

    /* create 1xM matrix with proper data type */
    pFlowArray = mxCreateNumericArray(2, dims, mxUINT32_CLASS, mxREAL);
    
    /* determine at which index to hook in the whole array */
    iArrayIndex = mxGetFieldNumber(pChannelData2Array,"SumDat");
    
    /* allocate heap for all elements, DON'T USE VARIABLES ON STACK !!! */
    pValue = mxCalloc(lDataSets,sizeof(uint32_t));

	pTempPointer = ptrCalibStatus;
	pU32Target = pValue;

	for(iElement = 0; iElement < lDataSets; iElement++)
	{
		/* copy the values into the allocated heap space */
		memcpy((void*)pU32Target,&(pTempPointer->MFCCardCalib[0].MFCChannelData[2].SumDat),sizeof(uint32_t));
		pTempPointer++;
		pU32Target++;
	}
    /* let the pointer of the array point to the heap space with our data */
    mxSetData(pFlowArray,pValue);
    mxSetFieldByNumber(pChannelData2Array,0,iArrayIndex,pFlowArray);

	/******************************************/
	/* 14th field of substruct MFCCardCalib   */
	/******************************************/

    /* create 1xM matrix with proper data type */
    pFlowArray = mxCreateNumericArray(2, dims, mxUINT32_CLASS, mxREAL);
    
    /* determine at which index to hook in the whole array */
    iArrayIndex = mxGetFieldNumber(pChannelData3Array,"SumDat");
    
    /* allocate heap for all elements, DON'T USE VARIABLES ON STACK !!! */
    pValue = mxCalloc(lDataSets,sizeof(uint32_t));

	pTempPointer = ptrCalibStatus;
	pU32Target = pValue;

	for(iElement = 0; iElement < lDataSets; iElement++)
	{
		/* copy the values into the allocated heap space */
		memcpy((void*)pU32Target,&(pTempPointer->MFCCardCalib[0].MFCChannelData[3].SumDat),sizeof(uint32_t));
		pTempPointer++;
		pU32Target++;
	}
    /* let the pointer of the array point to the heap space with our data */
    mxSetData(pFlowArray,pValue);
    mxSetFieldByNumber(pChannelData3Array,0,iArrayIndex,pFlowArray);

	/******************************************/

	/******************************************/
	/* 15th field of substruct MFCCardCalib   */
	/******************************************/

    /* create 1xM matrix with proper data type */
    pFlowArray = mxCreateNumericArray(2, dims, mxUINT32_CLASS, mxREAL);
    
    /* determine at which index to hook in the whole array */
    iArrayIndex = mxGetFieldNumber(pChannelData0Array,"SumSqr");
    
    /* allocate heap for all elements, DON'T USE VARIABLES ON STACK !!! */
    pValue = mxCalloc(lDataSets,sizeof(uint32_t));

	pTempPointer = ptrCalibStatus;
	pU32Target = pValue;

	for(iElement = 0; iElement < lDataSets; iElement++)
	{
		/* copy the values into the allocated heap space */
		memcpy((void*)pU32Target,&(pTempPointer->MFCCardCalib[0].MFCChannelData[0].SumSqr),sizeof(uint32_t));
		pTempPointer++;
		pU32Target++;
	}
    /* let the pointer of the array point to the heap space with our data */
    mxSetData(pFlowArray,pValue);
    mxSetFieldByNumber(pChannelData0Array,0,iArrayIndex,pFlowArray);

	/******************************************/
	/* 16th field of substruct MFCCardCalib   */
	/******************************************/

    /* create 1xM matrix with proper data type */
    pFlowArray = mxCreateNumericArray(2, dims, mxUINT32_CLASS, mxREAL);
    
    /* determine at which index to hook in the whole array */
    iArrayIndex = mxGetFieldNumber(pChannelData1Array,"SumSqr");
    
    /* allocate heap for all elements, DON'T USE VARIABLES ON STACK !!! */
    pValue = mxCalloc(lDataSets,sizeof(uint32_t));

	pTempPointer = ptrCalibStatus;
	pU32Target = pValue;

	for(iElement = 0; iElement < lDataSets; iElement++)
	{
		/* copy the values into the allocated heap space */
		memcpy((void*)pU32Target,&(pTempPointer->MFCCardCalib[0].MFCChannelData[1].SumSqr),sizeof(uint32_t));
		pTempPointer++;
		pU32Target++;
	}
    /* let the pointer of the array point to the heap space with our data */
    mxSetData(pFlowArray,pValue);
    mxSetFieldByNumber(pChannelData1Array,0,iArrayIndex,pFlowArray);

	/******************************************/
	/* 17th field of substruct MFCCardCalib   */
	/******************************************/

    /* create 1xM matrix with proper data type */
    pFlowArray = mxCreateNumericArray(2, dims, mxUINT32_CLASS, mxREAL);
    
    /* determine at which index to hook in the whole array */
    iArrayIndex = mxGetFieldNumber(pChannelData2Array,"SumSqr");
    
    /* allocate heap for all elements, DON'T USE VARIABLES ON STACK !!! */
    pValue = mxCalloc(lDataSets,sizeof(uint32_t));

	pTempPointer = ptrCalibStatus;
	pU32Target = pValue;

	for(iElement = 0; iElement < lDataSets; iElement++)
	{
		/* copy the values into the allocated heap space */
		memcpy((void*)pU32Target,&(pTempPointer->MFCCardCalib[0].MFCChannelData[2].SumSqr),sizeof(uint32_t));
		pTempPointer++;
		pU32Target++;
	}
    /* let the pointer of the array point to the heap space with our data */
    mxSetData(pFlowArray,pValue);
    mxSetFieldByNumber(pChannelData2Array,0,iArrayIndex,pFlowArray);

	/******************************************/
	/* 18th field of substruct MFCCardCalib   */
	/******************************************/

    /* create 1xM matrix with proper data type */
    pFlowArray = mxCreateNumericArray(2, dims, mxUINT32_CLASS, mxREAL);
    
    /* determine at which index to hook in the whole array */
    iArrayIndex = mxGetFieldNumber(pChannelData3Array,"SumSqr");
    
    /* allocate heap for all elements, DON'T USE VARIABLES ON STACK !!! */
    pValue = mxCalloc(lDataSets,sizeof(uint32_t));

	pTempPointer = ptrCalibStatus;
	pU32Target = pValue;

	for(iElement = 0; iElement < lDataSets; iElement++)
	{
		/* copy the values into the allocated heap space */
		memcpy((void*)pU32Target,&(pTempPointer->MFCCardCalib[0].MFCChannelData[3].SumSqr),sizeof(uint32_t));
		pTempPointer++;
		pU32Target++;
	}
    /* let the pointer of the array point to the heap space with our data */
    mxSetData(pFlowArray,pValue);
    mxSetFieldByNumber(pChannelData3Array,0,iArrayIndex,pFlowArray);

	/******************************************/

	/******************************************/
	/* 19th field of substruct MFCCardCalib   */
	/******************************************/

    /* create 1xM matrix with proper data type */
    pMFCChannelConfigArray = mxCreateNumericArray(2, dims, mxUINT16_CLASS, mxREAL);
    
    /* determine at which index to hook in the whole array */
    iArrayIndex = mxGetFieldNumber(pChannelConfig0Array,"MFCChannelConfig");
    
    /* allocate heap for all elements, DON'T USE VARIABLES ON STACK !!! */
    pValue = mxCalloc(lDataSets,sizeof(uint16_t));

	pTempPointer = ptrCalibStatus;
	pU16Target = pValue;

	for(iElement = 0; iElement < lDataSets; iElement++)
	{
		/* copy the values into the allocated heap space */
		memcpy((void*)pU16Target,&(pTempPointer->MFCCardCalib[0].MFCChannelConfig[0].MFCChannelConfig),sizeof(uint16_t));
		pTempPointer++;
		pU16Target++;
	}
    /* let the pointer of the array point to the heap space with our data */
    mxSetData(pMFCChannelConfigArray,pValue);
    mxSetFieldByNumber(pChannelConfig0Array,0,iArrayIndex,pMFCChannelConfigArray);

	/******************************************/
	/* 20th field of substruct MFCCardCalib   */
	/******************************************/

    /* create 1xM matrix with proper data type */
    pMFCChannelConfigArray = mxCreateNumericArray(2, dims, mxUINT16_CLASS, mxREAL);
    
    /* determine at which index to hook in the whole array */
    iArrayIndex = mxGetFieldNumber(pChannelConfig1Array,"MFCChannelConfig");
    
    /* allocate heap for all elements, DON'T USE VARIABLES ON STACK !!! */
    pValue = mxCalloc(lDataSets,sizeof(uint16_t));

	pTempPointer = ptrCalibStatus;
	pU16Target = pValue;

	for(iElement = 0; iElement < lDataSets; iElement++)
	{
		/* copy the values into the allocated heap space */
		memcpy((void*)pU16Target,&(pTempPointer->MFCCardCalib[0].MFCChannelConfig[1].MFCChannelConfig),sizeof(uint16_t));
		pTempPointer++;
		pU16Target++;
	}
    /* let the pointer of the array point to the heap space with our data */
    mxSetData(pMFCChannelConfigArray,pValue);
    mxSetFieldByNumber(pChannelConfig1Array,0,iArrayIndex,pMFCChannelConfigArray);

	/******************************************/
	/* 21th field of substruct MFCCardCalib   */
	/******************************************/

    /* create 1xM matrix with proper data type */
    pMFCChannelConfigArray = mxCreateNumericArray(2, dims, mxUINT16_CLASS, mxREAL);
    
    /* determine at which index to hook in the whole array */
    iArrayIndex = mxGetFieldNumber(pChannelConfig2Array,"MFCChannelConfig");
    
    /* allocate heap for all elements, DON'T USE VARIABLES ON STACK !!! */
    pValue = mxCalloc(lDataSets,sizeof(uint16_t));

	pTempPointer = ptrCalibStatus;
	pU16Target = pValue;

	for(iElement = 0; iElement < lDataSets; iElement++)
	{
		/* copy the values into the allocated heap space */
		memcpy((void*)pU16Target,&(pTempPointer->MFCCardCalib[0].MFCChannelConfig[2].MFCChannelConfig),sizeof(uint16_t));
		pTempPointer++;
		pU16Target++;
	}
    /* let the pointer of the array point to the heap space with our data */
    mxSetData(pMFCChannelConfigArray,pValue);
    mxSetFieldByNumber(pChannelConfig2Array,0,iArrayIndex,pMFCChannelConfigArray);

	/******************************************/
	/* 22th field of substruct MFCCardCalib   */
	/******************************************/

    /* create 1xM matrix with proper data type */
    pMFCChannelConfigArray = mxCreateNumericArray(2, dims, mxUINT16_CLASS, mxREAL);
    
    /* determine at which index to hook in the whole array */
    iArrayIndex = mxGetFieldNumber(pChannelConfig3Array,"MFCChannelConfig");
    
    /* allocate heap for all elements, DON'T USE VARIABLES ON STACK !!! */
    pValue = mxCalloc(lDataSets,sizeof(uint16_t));

	pTempPointer = ptrCalibStatus;
	pU16Target = pValue;

	for(iElement = 0; iElement < lDataSets; iElement++)
	{
		/* copy the values into the allocated heap space */
		memcpy((void*)pU16Target,&(pTempPointer->MFCCardCalib[0].MFCChannelConfig[3].MFCChannelConfig),sizeof(uint16_t));
		pTempPointer++;
		pU16Target++;
	}
    /* let the pointer of the array point to the heap space with our data */
    mxSetData(pMFCChannelConfigArray,pValue);
    mxSetFieldByNumber(pChannelConfig3Array,0,iArrayIndex,pMFCChannelConfigArray);

	/*****************************/
	/* SUBSTRUCTURE SCRCardCalib */
	/*****************************/

	pSCRCardCalibArray = mxCreateStructArray(2, small_dims, NUMBER_OF_SCR_POWER_FIELDS, scr_power_field_names);

	/*****************************************/
	/* 1st field of substruct SCRCardCalib   */
	/*****************************************/

    /* create 1xM matrix with proper data type */
    pSCRPowerValue0Array = mxCreateNumericArray(2, dims, mxUINT16_CLASS, mxREAL);
    
    /* determine at which index to hook in the whole array */
    iArrayIndex = mxGetFieldNumber(pSCRCardCalibArray,"SCRPowerValue0");
    
    /* allocate heap for all elements, DON'T USE VARIABLES ON STACK !!! */
    pValue = mxCalloc(lDataSets,sizeof(uint16_t));

	pTempPointer = ptrCalibStatus;
	pU16Target = pValue;

	for(iElement = 0; iElement < lDataSets; iElement++)
	{
		/* copy the values into the allocated heap space */
		memcpy((void*)pU16Target,&(pTempPointer->SCRCardCalib[0].SCRPowerValue[0]),sizeof(uint16_t));
		pTempPointer++;
		pU16Target++;
	}
    /* let the pointer of the array point to the heap space with our data */
    mxSetData(pSCRPowerValue0Array,pValue);
    mxSetFieldByNumber(pSCRCardCalibArray,0,iArrayIndex,pSCRPowerValue0Array);

	/*****************************************/
	/* 2nd field of substruct SCRCardCalib   */
	/*****************************************/

    /* create 1xM matrix with proper data type */
    pSCRPowerValue1Array = mxCreateNumericArray(2, dims, mxUINT16_CLASS, mxREAL);
    
    /* determine at which index to hook in the whole array */
    iArrayIndex = mxGetFieldNumber(pSCRCardCalibArray,"SCRPowerValue1");
    
    /* allocate heap for all elements, DON'T USE VARIABLES ON STACK !!! */
    pValue = mxCalloc(lDataSets,sizeof(uint16_t));

	pTempPointer = ptrCalibStatus;
	pU16Target = pValue;

	for(iElement = 0; iElement < lDataSets; iElement++)
	{
		/* copy the values into the allocated heap space */
		memcpy((void*)pU16Target,&(pTempPointer->SCRCardCalib[0].SCRPowerValue[1]),sizeof(uint16_t));
		pTempPointer++;
		pU16Target++;
	}
    /* let the pointer of the array point to the heap space with our data */
    mxSetData(pSCRPowerValue1Array,pValue);
    mxSetFieldByNumber(pSCRCardCalibArray,0,iArrayIndex,pSCRPowerValue1Array);

	/*****************************************/
	/* 3rd field of substruct SCRCardCalib   */
	/*****************************************/

    /* create 1xM matrix with proper data type */
    pSCRPowerValue2Array = mxCreateNumericArray(2, dims, mxUINT16_CLASS, mxREAL);
    
    /* determine at which index to hook in the whole array */
    iArrayIndex = mxGetFieldNumber(pSCRCardCalibArray,"SCRPowerValue2");
    
    /* allocate heap for all elements, DON'T USE VARIABLES ON STACK !!! */
    pValue = mxCalloc(lDataSets,sizeof(uint16_t));

	pTempPointer = ptrCalibStatus;
	pU16Target = pValue;

	for(iElement = 0; iElement < lDataSets; iElement++)
	{
		/* copy the values into the allocated heap space */
		memcpy((void*)pU16Target,&(pTempPointer->SCRCardCalib[0].SCRPowerValue[2]),sizeof(uint16_t));
		pTempPointer++;
		pU16Target++;
	}
    /* let the pointer of the array point to the heap space with our data */
    mxSetData(pSCRPowerValue2Array,pValue);
    mxSetFieldByNumber(pSCRCardCalibArray,0,iArrayIndex,pSCRPowerValue2Array);

	/*********************************************/
    /* link main struct array to substruct array */
	/*********************************************/

    iArrayIndex = mxGetFieldNumber(plhs[0],"TimeOfDayCalib");
    mxSetFieldByNumber(plhs[0],0,iArrayIndex,pTimeOfDayCalibArray);

    iArrayIndex = mxGetFieldNumber(plhs[0],"ADCCardCalib");
    mxSetFieldByNumber(plhs[0],0,iArrayIndex,pADCCardCalibArray);

    iArrayIndex = mxGetFieldNumber(plhs[0],"MFCCardCalib");
    mxSetFieldByNumber(plhs[0],0,iArrayIndex,pMFCCardCalibArray);

	iArrayIndex = mxGetFieldNumber(plhs[0],"SCRCardCalib");
    mxSetFieldByNumber(plhs[0],0,iArrayIndex,pSCRCardCalibArray);

	iArrayIndex = mxGetFieldNumber(plhs[0],"TempSensCardCalib");
    mxSetFieldByNumber(plhs[0],0,iArrayIndex,pTempSensCardCalibArray);

	iArrayIndex = mxGetFieldNumber(plhs[0],"LicorCalib");
    mxSetFieldByNumber(plhs[0],0,iArrayIndex,pLicorCalibArray);

	iArrayIndex = mxGetFieldNumber(plhs[0],"PIDRegulator");
    mxSetFieldByNumber(plhs[0],0,iArrayIndex,pPidRegulatorArray);

    #ifdef X_DEBUG
       mexPrintf("done \n");
    #endif
}

