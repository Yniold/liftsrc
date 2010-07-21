/*
 * ReadAuxData.c
 *	        read binary data files from meteostation, ship GPS etc.
 *
 *   [statusdata] = ReadAuxData('filename')
 *
 * $Id: ReadAuxData.c,v 1.1 2007/06/11 14:59:39 rudolf Exp $
 * $Log: ReadAuxData.c,v $
 * Revision 1.1  2007/06/11 14:59:39  rudolf
 * added c file for reading binary data files into MATLAB structures
 *
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

/* uncomment for verbose debug output
#define X_DEBUG
#define POINTER_DEBUG
*/

/* #define D_HEADER     1               /* will print with mexPrintf the position of each row */
/*#define UNIX   */                     /* has to be set for unix compilation */

#ifdef UNIX                             /* there are some differences between windows & linux header files */
#include <stdint.h>
#else
typedef unsigned char uint8_t;
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

/* for field names, see elekIO.h */

static mxArray *pRootArray;

/* Top Level Structure */
#define NUMBER_OF_STRUCT_FIELDS (9)
const char *struct_field_names[] = 
{"TimeOfDayAux",                             
 "MeteoBox",             
 "ShipGPS",             
 "ShipSonar",   
 "ShipMeteo",
 "ShipGyro",
 "ShipWater",
 "LicorPressure",
 "Status"
};                                 
static mxArray *pTimeOfDayAuxArray;
static mxArray *pMeteoBoxArray;
static mxArray *pShipGPSArray;
static mxArray *pShipMeteoArray;
static mxArray *pShipSonarArray;
static mxArray *pShipGyroArray;
static mxArray *pShipWaterArray;
static mxArray *pLicorPressureArray;
static mxArray *pStatusArray;

/* TimeVal */
#define NUMBER_OF_TIMEVAL_FIELDS (2)
const char *timeval_field_names[] = 
{"tv_sec",
"tv_usec"
};
static mxArray *ptv_sec_FieldValue;
static mxArray *ptv_usec_FieldValue;
#define NUMBER_OF_METEOBOX_FIELDS (5)

/* MeteoBox */
const char *MeteoBoxFieldNames[]=
{
"dWindSpeed",
"uiWindDirection",
"dRelHum",
"dAirTemp",
"dGasSensorVoltage"
};

static mxArray *pMeteoBoxWindSpeedArray;
static mxArray *pMeteoBoxWindDirectionArray;
static mxArray *pMeteoBoxRelHumArray;
static mxArray *pMeteoBoxAirTempArray;
static mxArray *pMeteoBoxGasSensVoltageArray;


#define NUMBER_OF_SHIPGPS_FIELDS (10)
const char *ShipGPSFieldNames[]=
{"ucUTCHours",
"ucUTCMins",
"ucUTCSeconds",
"ucUTCDay",
"ucUTCMonth",
"uiUTCYear",
"dLongitude",
"dLatitude",
"dGroundSpeed",
"dCourseOverGround"};

static mxArray *pShipGPSHoursArray;
static mxArray *pShipGPSMinsArray;
static mxArray *pShipGPSSecondsArray;
static mxArray *pShipGPSDayArray;
static mxArray *pShipGPSMonthArray;
static mxArray *pShipGPSYearArray;
static mxArray *pShipGPSLongitudeArray;
static mxArray *pShipGPSLatitudeArray;
static mxArray *pShipGPSGroundSpeedArray;
static mxArray *pShipGPSCourseOverGroundArray;

/* Sonar */
#define NUMBER_OF_SHIPSONAR_FIELDS (2)
const char *ShipSonarFieldNames[]=
{"dFrequency",
"dWaterDepth"};

static mxArray *pShipSonarFrequencyArray;
static mxArray *pShipSonarWaterDepthArray;

/* Ship Meteo */
#define NUMBER_OF_SHIPMETEO_FIELDS (2)
const char *ShipMeteoFieldNames[]=
{"dWindSpeed",
"dWindDirection"};

static mxArray *pShipMeteoWindSpeedArray;
static mxArray *pShipMeteoWindDirectionArray;

/* Gyro */
#define NUMBER_OF_SHIPGYRO_FIELDS (1)
const char *ShipGyroFieldNames[]=
{"dDirection"};

static mxArray *pShipGyroDirectionArray;

/* Water Data */
#define NUMBER_OF_SHIPWATER_FIELDS (2)
const char *ShipWaterFieldNames[]=
{"dSalinity",
"dWaterTemp"};

static mxArray *pShipWaterSalinityArray;
static mxArray *pShipWaterTemperatureArray;

/* Licor Pressure */
#define NUMBER_OF_LICORPRESSURE_FIELDS (1)
const char *ShipLicorPressureFieldNames[]=
{"uiAmbientPressure"};

static mxArray *pShipLicorPressureArray;

/* status bits, needed because bitfields not supported in matlab */

#define NUMBER_OF_STATUS_FIELDS (7)
const char *StatusFieldNames[]=
{"MeteoBoxDataValid",
"ShipGPSDataValid",
"ShipMeteoDataValid",
"ShipSonarDataValid",
"ShipGyroDataValid",
"ShipWaterDataValid",
"LicorPressureDataValid"};

static mxArray *pMeteoBoxDataValidArray;
static mxArray *pShipGPSDataValidArray;
static mxArray *pShipMeteoDataValidArray;
static mxArray *pShipSonarDataValidArray;
static mxArray *pShipGyroDataValidArray;
static mxArray *pShipWaterDataValidArray;
static mxArray *pLicorPressureDataValidArray;

/* helper arrays */
int dims[2];
int small_dims[2];
int scr_dims[2];

int i,iArrayIndex,iLoop;
static void* pSave;
static void* pValue;

uint16_t* pUINT16Value;
int16_t* pINT16Value;

int iElement;
struct auxStatusType* pTempPointer;
uint32_t* pU32Target;
uint16_t* pU16Target;
uint8_t* pU08Target;
uint8_t* pBoolTarget;
uint8_t ucTrue = 1;
uint8_t ucFalse = 0;

double* pDblTarget;

/* function for sorting structure by timestamp in case we
 * are reading the running status file calibstatus.bin
 */

int cmptimesort(const void *ptrAuxStatus1, const void *ptrAuxStatus2)
{
    int ret;
    ret=0;
    if (((struct auxStatusType*)ptrAuxStatus1)->TimeOfDayAux.tv_sec>((struct auxStatusType*)ptrAuxStatus2)->TimeOfDayAux.tv_sec) ret=1;
    if (((struct auxStatusType*)ptrAuxStatus2)->TimeOfDayAux.tv_sec>((struct auxStatusType*)ptrAuxStatus1)->TimeOfDayAux.tv_sec) ret=-1;
    if (ret==0) { /* same second, now look for usec */
        if (((struct auxStatusType*)ptrAuxStatus1)->TimeOfDayAux.tv_usec>((struct auxStatusType*)ptrAuxStatus2)->TimeOfDayAux.tv_usec) ret=1;
        if (((struct auxStatusType*)ptrAuxStatus2)->TimeOfDayAux.tv_usec>((struct auxStatusType*)ptrAuxStatus1)->TimeOfDayAux.tv_usec) ret=-1;
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
    
    struct auxStatusType *ptrAuxStatus;
    
  /* Check for proper number of arguments */
    if (nrhs != 1)
    {
        mexPrintf("This is $Id: ReadAuxData.c,v 1.1 2007/06/11 14:59:39 rudolf Exp $ \n");
        mexErrMsgTxt("input argument required: FILENAME.AUX");
    }
    
  /* Input must be a string. */
    if (mxIsChar(prhs[0]) != 1) {
        mexPrintf("This is $Id: ReadAuxData.c,v 1.1 2007/06/11 14:59:39 rudolf Exp $  \n");
        mexErrMsgTxt("Input must be a string.");
    }
  /* Input must be a row vector. */
    if (mxGetM(prhs[0]) != 1) {
        mexPrintf("This is $Id: ReadAuxData.c,v 1.1 2007/06/11 14:59:39 rudolf Exp $ \n");
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
    mexPrintf("Structuresize: %d\n",sizeof(struct auxStatusType));
	mexPrintf("distributed into:\n\n");
	mexPrintf("Structuresize: timeval :%d\n",sizeof(struct timeval));
	mexPrintf("Structuresize: MeteoBox :%d\n",sizeof(struct MeteoBoxType));
	mexPrintf("Structuresize: ShipGPS :%d\n",sizeof(struct ShipGPSDataType));
	mexPrintf("Structuresize: ShipSonar :%d\n",sizeof(struct ShipSonarType));
	mexPrintf("Structuresize: ShipMeteo :%d\n",sizeof(struct ShipMeteoType));
	mexPrintf("Structuresize: ShipGyro :%d\n",sizeof(struct ShipGyroType));
	mexPrintf("Structuresize: ShipWater :%d\n",sizeof(struct ShipWaterType));
	mexPrintf("Structuresize: LicorPressureType :%d\n",sizeof(struct LicorPressureType));
	mexPrintf("Structuresize: AuxDataValidType :%d\n\n",sizeof(struct AuxDataValidType));
    #endif
    
  /* test if the filesize is a multiple of the structure size, else complain */
    
    if(lFileLength % sizeof(struct auxStatusType) != 0)
    {
        mexPrintf("This is $Id: ReadAuxData.c,v 1.1 2007/06/11 14:59:39 rudolf Exp $ \n");
        mexErrMsgTxt("File size is not a multiple of structure size, please select a proper .aux file!");
    }
  /* we have to allocate some space, use mxCalloc, matlab is cleaning up memory upon exit */
    pDataBuf = mxCalloc(lFileLength, sizeof(char));
    lDataSets = lFileLength / sizeof(struct auxStatusType);
  /* go to the beginning */
    fseek(fp,0,SEEK_SET);
    
    status=fread(pDataBuf,sizeof(struct auxStatusType),lDataSets,fp);
    fclose(fp);
    
    ptrAuxStatus=(struct auxStatusType*)pDataBuf;
    
    qsort(ptrAuxStatus,lDataSets,sizeof(struct auxStatusType),cmptimesort);
    
    mexPrintf("This is $Id: ReadAuxData.c,v 1.1 2007/06/11 14:59:39 rudolf Exp $ \n");
    mexPrintf("Read %ld datasets from file %s\n",lDataSets,input_buf);
        
  /*******************/
  /* create matrices */
  /*******************/
    
    dims[1] = 1;
    dims[0] = (int) lDataSets;
    
    small_dims[0] = 1;
    small_dims[1] = 1;

    /* 1x1 structure with field names */
    plhs[0] = mxCreateStructArray(2, small_dims, NUMBER_OF_STRUCT_FIELDS, struct_field_names);

	pTimeOfDayAuxArray = mxCreateStructArray(2, small_dims, NUMBER_OF_TIMEVAL_FIELDS, timeval_field_names);

	/*******************************************/
	/* first field of substruct TimeOfDayAux   */
	/*******************************************/

    /* create 1xM matrix with proper data type */
    ptv_sec_FieldValue = mxCreateNumericArray(2, dims, mxUINT32_CLASS, mxREAL);
    
    /* determine at which index to hook in the whole array */
    iArrayIndex = mxGetFieldNumber(pTimeOfDayAuxArray,"tv_sec");
    
    /* allocate heap for all elements, DON'T USE VARIABLES ON STACK !!! */
    pValue = mxCalloc(lDataSets,sizeof(uint32_t));

	pTempPointer = ptrAuxStatus;
	pU32Target = pValue;

	for(iElement = 0; iElement < lDataSets; iElement++)
	{
		/* copy the values into the allocated heap space */
		memcpy((void*)pU32Target,&(pTempPointer->TimeOfDayAux.tv_sec),sizeof(uint32_t));
		pTempPointer++;
		pU32Target++;
	}
    /* let the pointer of the array point to the heap space with our data */
    mxSetData(ptv_sec_FieldValue,pValue);
    mxSetFieldByNumber(pTimeOfDayAuxArray,0,iArrayIndex,ptv_sec_FieldValue);

	/********************************************/
	/* second field of substruct TimeOfDayAux */
	/********************************************/
    
    /* create 1xM matrix with proper data type */
    ptv_usec_FieldValue = mxCreateNumericArray(2, dims, mxUINT32_CLASS, mxREAL);

    iArrayIndex = mxGetFieldNumber(pTimeOfDayAuxArray,"tv_usec");

	/* allocate heap for all elements, DON'T USE VARIABLES ON STACK !!! */
    pValue = mxCalloc(lDataSets,sizeof(uint32_t));

	pTempPointer = ptrAuxStatus;
	pU32Target = pValue;

	for(iElement = 0; iElement < lDataSets; iElement++)
	{
		/* copy the values into the allocated heap space */
		memcpy((void*)pU32Target,&(pTempPointer->TimeOfDayAux.tv_usec),sizeof(uint32_t));
		pTempPointer++;
		pU32Target++;
	}

	/* let the pointer of the array point to the heap space with our data */
    mxSetData(ptv_usec_FieldValue,pValue);
    mxSetFieldByNumber(pTimeOfDayAuxArray,0,iArrayIndex,ptv_usec_FieldValue);

	/*************************/
	/* SUBSTRUCTURE MeteoBox */
	/*************************/

	pMeteoBoxArray = mxCreateStructArray(2, small_dims, NUMBER_OF_METEOBOX_FIELDS, MeteoBoxFieldNames);

	/*************************************/
	/* 1st field of substruct MeteoBox */
	/*************************************/

    /* create 1x1 matrix with proper data type */
    pMeteoBoxWindSpeedArray = mxCreateNumericArray(2, dims, mxDOUBLE_CLASS, mxREAL);
    
    /* determine at which index to hook in the whole array */
    iArrayIndex = mxGetFieldNumber(pMeteoBoxArray,"dWindSpeed");
    
    /* allocate heap for the data elements DON'T USE VARIABLES ON STACK !!! */
    pValue = mxCalloc(lDataSets,sizeof(double));

	pTempPointer = ptrAuxStatus;
	pDblTarget = pValue;

	for(iElement = 0; iElement < lDataSets; iElement++)
	{
		/* copy the values into the allocated heap space */
		memcpy((void*)pDblTarget,&(pTempPointer->MeteoBox.dWindSpeed),sizeof(double));
		pTempPointer++;
		pDblTarget++;
	}
    /* let the pointer of the array point to the heap space with our data */
    mxSetData(pMeteoBoxWindSpeedArray,pValue);
    mxSetFieldByNumber(pMeteoBoxArray,0,iArrayIndex,pMeteoBoxWindSpeedArray);

	/*************************************/
	/* 2nd field of substruct MeteoBox */
	/*************************************/

    /* create 1xM matrix with proper data type */
    pMeteoBoxWindDirectionArray = mxCreateNumericArray(2, dims, mxUINT16_CLASS, mxREAL);

    /* determine at which index to hook in the whole array */
    iArrayIndex = mxGetFieldNumber(pMeteoBoxArray,"uiWindDirection");

    /* allocate heap for a single element, DON'T USE VARIABLES ON STACK !!! */
    pValue = mxCalloc(lDataSets,sizeof(uint16_t));

	pTempPointer = ptrAuxStatus;
	pU16Target = pValue;

	for(iElement = 0; iElement < lDataSets; iElement++)
	{
		/* copy the values into the allocated heap space */
		memcpy((void*)pU16Target,&(pTempPointer->MeteoBox.uiWindDirection),sizeof(uint16_t));
		pTempPointer++;
		pU16Target++;
	}

    /* let the pointer of the array point to the heap space with our data */
	mxSetData(pMeteoBoxWindDirectionArray,pValue);
    mxSetFieldByNumber(pMeteoBoxArray,0,iArrayIndex,pMeteoBoxWindDirectionArray);

	/*************************************/
	/* 3rd field of substruct MeteoBox */
	/*************************************/

    /* create 1xM matrix with proper data type */
    pMeteoBoxRelHumArray = mxCreateNumericArray(2, dims, mxDOUBLE_CLASS, mxREAL);

    /* determine at which index to hook in the whole array */
    iArrayIndex = mxGetFieldNumber(pMeteoBoxArray,"dRelHum");

    /* allocate heap for a single element, DON'T USE VARIABLES ON STACK !!! */
    pValue = mxCalloc(lDataSets,sizeof(double));

	pTempPointer = ptrAuxStatus;
	pDblTarget = pValue;

	for(iElement = 0; iElement < lDataSets; iElement++)
	{
		/* copy the values into the allocated heap space */
		memcpy((void*)pDblTarget,&(pTempPointer->MeteoBox.dRelHum),sizeof(double));
		pTempPointer++;
		pDblTarget++;
	}

    /* copy the value into the allocated heap space */
    mxSetData(pMeteoBoxRelHumArray,pValue);
    mxSetFieldByNumber(pMeteoBoxArray,0,iArrayIndex,pMeteoBoxRelHumArray);

	/*************************************/
	/* 4th field of substruct MeteoBox */
	/*************************************/

    /* create 1x1 matrix with proper data type */
    pMeteoBoxAirTempArray = mxCreateNumericArray(2, dims, mxDOUBLE_CLASS, mxREAL);

    /* determine at which index to hook in the whole array */
    iArrayIndex = mxGetFieldNumber(pMeteoBoxArray,"dAirTemp");

    /* allocate heap for a single element, DON'T USE VARIABLES ON STACK !!! */
    pValue = mxCalloc(lDataSets,sizeof(double));

	pTempPointer = ptrAuxStatus;
	pDblTarget = pValue;

	for(iElement = 0; iElement < lDataSets; iElement++)
	{
		/* copy the values into the allocated heap space */
		memcpy((void*)pDblTarget,&(pTempPointer->MeteoBox.dAirTemp),sizeof(double));
		pTempPointer++;
		pDblTarget++;
	}

    /* copy the value into the allocated heap space */
    mxSetData(pMeteoBoxAirTempArray,pValue);
    mxSetFieldByNumber(pMeteoBoxArray,0,iArrayIndex,pMeteoBoxAirTempArray);

	/*************************************/
	/* 5th field of substruct MeteoBox */
	/*************************************/

    /* create 1x1 matrix with proper data type */
    pMeteoBoxGasSensVoltageArray = mxCreateNumericArray(2, dims, mxDOUBLE_CLASS, mxREAL);

    /* determine at which index to hook in the whole array */
    iArrayIndex = mxGetFieldNumber(pMeteoBoxArray,"dGasSensorVoltage");

    /* allocate heap for a single element, DON'T USE VARIABLES ON STACK !!! */
    pValue = mxCalloc(lDataSets,sizeof(double));

	pTempPointer = ptrAuxStatus;
	pDblTarget = pValue;

	for(iElement = 0; iElement < lDataSets; iElement++)
	{
		/* copy the values into the allocated heap space */
		memcpy((void*)pDblTarget,&(pTempPointer->MeteoBox.dGasSensorVoltage),sizeof(double));
		pTempPointer++;
		pDblTarget++;
	}

    /* copy the value into the allocated heap space */
    mxSetData(pMeteoBoxGasSensVoltageArray,pValue);
    mxSetFieldByNumber(pMeteoBoxArray,0,iArrayIndex,pMeteoBoxGasSensVoltageArray);

	/*************************/
	/* SUBSTRUCTURE ShipGPS  */
	/*************************/

	pShipGPSArray = mxCreateStructArray(2, small_dims, NUMBER_OF_SHIPGPS_FIELDS, ShipGPSFieldNames);

	/**********************************/
	/* 1st field of substruct ShipGPS */
	/**********************************/

    /* create 1x1 matrix with proper data type */
    pShipGPSHoursArray = mxCreateNumericArray(2, dims, mxUINT8_CLASS, mxREAL);
    
    /* determine at which index to hook in the whole array */
    iArrayIndex = mxGetFieldNumber(pShipGPSArray,"ucUTCHours");
    
    /* allocate heap for a single element, DON'T USE VARIABLES ON STACK !!! */
	pValue = mxCalloc(lDataSets,sizeof(uint8_t));

	pTempPointer = ptrAuxStatus;
	pU08Target = pValue;

	for(iElement = 0; iElement < lDataSets; iElement++)
	{
		/* copy the values into the allocated heap space */
		memcpy((void*)pU08Target,&(pTempPointer->ShipGPS.ucUTCHours),sizeof(uint8_t));
		pTempPointer++;
		pU08Target++;
	}

   /* let the pointer of the array point to the heap space with our data */
    mxSetData(pShipGPSHoursArray,pValue);
    mxSetFieldByNumber(pShipGPSArray,0,iArrayIndex,pShipGPSHoursArray);

	/**********************************/
	/* 2nd field of substruct ShipGPS */
	/**********************************/

    /* create 1x1 matrix with proper data type */
    pShipGPSMinsArray = mxCreateNumericArray(2, dims, mxUINT8_CLASS, mxREAL);
    
    /* determine at which index to hook in the whole array */
    iArrayIndex = mxGetFieldNumber(pShipGPSArray,"ucUTCMins");
    
    /* allocate heap for a single element, DON'T USE VARIABLES ON STACK !!! */
    pValue = mxCalloc(lDataSets,sizeof(uint8_t));
	
	pTempPointer = ptrAuxStatus;
	pU08Target = pValue;

	for(iElement = 0; iElement < lDataSets; iElement++)
	{
		/* copy the values into the allocated heap space */
		memcpy((void*)pU08Target,&(pTempPointer->ShipGPS.ucUTCMins),sizeof(uint8_t));
		pTempPointer++;
		pU08Target++;
	}

    /* let the pointer of the array point to the heap space with our data */
    mxSetData(pShipGPSMinsArray,pValue);
    mxSetFieldByNumber(pShipGPSArray,0,iArrayIndex,pShipGPSMinsArray);

	/**********************************/
	/* 3rd field of substruct ShipGPS */
	/**********************************/

    /* create 1x1 matrix with proper data type */
    pShipGPSSecondsArray = mxCreateNumericArray(2, dims, mxUINT8_CLASS, mxREAL);
    
    /* determine at which index to hook in the whole array */
    iArrayIndex = mxGetFieldNumber(pShipGPSArray,"ucUTCSeconds");
    
    /* allocate heap for a single element, DON'T USE VARIABLES ON STACK !!! */
    pValue = mxCalloc(lDataSets,sizeof(uint8_t));

	pTempPointer = ptrAuxStatus;
	pU08Target = pValue;

	for(iElement = 0; iElement < lDataSets; iElement++)
	{
		/* copy the values into the allocated heap space */
		memcpy((void*)pU08Target,&(pTempPointer->ShipGPS.ucUTCSeconds),sizeof(uint8_t));
		pTempPointer++;
		pU08Target++;
	}

    /* let the pointer of the array point to the heap space with our data */
    mxSetData(pShipGPSSecondsArray,pValue);
    mxSetFieldByNumber(pShipGPSArray,0,iArrayIndex,pShipGPSSecondsArray);

	/**********************************/
	/* 4th field of substruct ShipGPS */
	/**********************************/

    /* create 1x1 matrix with proper data type */
    pShipGPSDayArray = mxCreateNumericArray(2, dims, mxUINT8_CLASS, mxREAL);
    
    /* determine at which index to hook in the whole array */
    iArrayIndex = mxGetFieldNumber(pShipGPSArray,"ucUTCDay");
    
    /* allocate heap for a single element, DON'T USE VARIABLES ON STACK !!! */
    pValue = mxCalloc(lDataSets,sizeof(uint8_t));

	pTempPointer = ptrAuxStatus;
	pU08Target = pValue;

	for(iElement = 0; iElement < lDataSets; iElement++)
	{
		/* copy the values into the allocated heap space */
		memcpy((void*)pU08Target,&(pTempPointer->ShipGPS.ucUTCDay),sizeof(uint8_t));
		pTempPointer++;
		pU08Target++;
	}

    /* let the pointer of the array point to the heap space with our data */
    mxSetData(pShipGPSDayArray,pValue);
    mxSetFieldByNumber(pShipGPSArray,0,iArrayIndex,pShipGPSDayArray);

	/**********************************/
	/* 5th field of substruct ShipGPS */
	/**********************************/

    /* create 1xM matrix with proper data type */
    pShipGPSMonthArray = mxCreateNumericArray(2, dims, mxUINT8_CLASS, mxREAL);
    
    /* determine at which index to hook in the whole array */
    iArrayIndex = mxGetFieldNumber(pShipGPSArray,"ucUTCMonth");
    
    /* allocate heap for a single element, DON'T USE VARIABLES ON STACK !!! */
    pValue = mxCalloc(lDataSets,sizeof(uint8_t));

	pTempPointer = ptrAuxStatus;
	pU08Target = pValue;

	for(iElement = 0; iElement < lDataSets; iElement++)
	{
		/* copy the values into the allocated heap space */
		memcpy((void*)pU08Target,&(pTempPointer->ShipGPS.ucUTCMonth),sizeof(uint8_t));
		pTempPointer++;
		pU08Target++;
	}

	/* let the pointer of the array point to the heap space with our data */
    mxSetData(pShipGPSMonthArray,pValue);
    mxSetFieldByNumber(pShipGPSArray,0,iArrayIndex,pShipGPSMonthArray);

	/**********************************/
	/* 6th field of substruct ShipGPS */
	/**********************************/

    /* create 1xM matrix with proper data type */
    pShipGPSYearArray = mxCreateNumericArray(2, dims, mxUINT16_CLASS, mxREAL);
    
    /* determine at which index to hook in the whole array */
    iArrayIndex = mxGetFieldNumber(pShipGPSArray,"uiUTCYear");
    
    /* allocate heap for a single element, DON'T USE VARIABLES ON STACK !!! */
    pValue = mxCalloc(lDataSets,sizeof(uint16_t));

	pTempPointer = ptrAuxStatus;
	pU16Target = pValue;

	for(iElement = 0; iElement < lDataSets; iElement++)
	{
		/* copy the values into the allocated heap space */
		memcpy((void*)pU16Target,&(pTempPointer->ShipGPS.uiUTCYear),sizeof(uint16_t));
		pTempPointer++;
		pU16Target++;
	}

	/* let the pointer of the array point to the heap space with our data */
    mxSetData(pShipGPSYearArray,pValue);
    mxSetFieldByNumber(pShipGPSArray,0,iArrayIndex,pShipGPSYearArray);

	/**********************************/
	/* 7th field of substruct ShipGPS */
	/**********************************/

    /* create 1xM matrix with proper data type */
    pShipGPSLongitudeArray = mxCreateNumericArray(2, dims, mxDOUBLE_CLASS, mxREAL);
    
    /* determine at which index to hook in the whole array */
    iArrayIndex = mxGetFieldNumber(pShipGPSArray,"dLongitude");
    
    /* allocate heap for a single element, DON'T USE VARIABLES ON STACK !!! */
    pValue = mxCalloc(lDataSets,sizeof(double));
	pTempPointer = ptrAuxStatus;
	pDblTarget = pValue;

	for(iElement = 0; iElement < lDataSets; iElement++)
	{
		/* copy the values into the allocated heap space */
		memcpy((void*)pDblTarget,&(pTempPointer->ShipGPS.dLongitude),sizeof(double));
		pTempPointer++;
		pDblTarget++;
	}

	/* let the pointer of the array point to the heap space with our data */
    mxSetData(pShipGPSLongitudeArray,pValue);
    mxSetFieldByNumber(pShipGPSArray,0,iArrayIndex,pShipGPSLongitudeArray);

	/**********************************/
	/* 8th field of substruct ShipGPS */
	/**********************************/

    /* create 1xM matrix with proper data type */
    pShipGPSLatitudeArray = mxCreateNumericArray(2, dims, mxDOUBLE_CLASS, mxREAL);
    
    /* determine at which index to hook in the whole array */
    iArrayIndex = mxGetFieldNumber(pShipGPSArray,"dLatitude");
    
    /* allocate heap for a single element, DON'T USE VARIABLES ON STACK !!! */
    pValue = mxCalloc(lDataSets,sizeof(double));

	pTempPointer = ptrAuxStatus;
	pDblTarget = pValue;

	for(iElement = 0; iElement < lDataSets; iElement++)
	{
		/* copy the values into the allocated heap space */
		memcpy((void*)pDblTarget,&(pTempPointer->ShipGPS.dLatitude),sizeof(double));
		pTempPointer++;
		pDblTarget++;
	}
    
    /* let the pointer of the array point to the heap space with our data */
    mxSetData(pShipGPSLatitudeArray,pValue);
    mxSetFieldByNumber(pShipGPSArray,0,iArrayIndex,pShipGPSLatitudeArray);

	/**********************************/
	/* 9th field of substruct ShipGPS */
	/**********************************/

    /* create 1x1 matrix with proper data type */
    pShipGPSGroundSpeedArray = mxCreateNumericArray(2, dims, mxDOUBLE_CLASS, mxREAL);
    
    /* determine at which index to hook in the whole array */
    iArrayIndex = mxGetFieldNumber(pShipGPSArray,"dGroundSpeed");
    
    /* allocate heap for a single element, DON'T USE VARIABLES ON STACK !!! */
    pValue = mxCalloc(lDataSets,sizeof(double));

	pTempPointer = ptrAuxStatus;
	pDblTarget = pValue;

	for(iElement = 0; iElement < lDataSets; iElement++)
	{
		/* copy the values into the allocated heap space */
		memcpy((void*)pDblTarget,&(pTempPointer->ShipGPS.dGroundSpeed),sizeof(double));
		pTempPointer++;
		pDblTarget++;
	}
   
    /* let the pointer of the array point to the heap space with our data */
    mxSetData(pShipGPSGroundSpeedArray,pValue);
    mxSetFieldByNumber(pShipGPSArray,0,iArrayIndex,pShipGPSGroundSpeedArray);

	/***********************************/
	/* 10th field of substruct ShipGPS */
	/***********************************/

    /* create 1xM matrix with proper data type */
    pShipGPSCourseOverGroundArray = mxCreateNumericArray(2, dims, mxDOUBLE_CLASS, mxREAL);
    
    /* determine at which index to hook in the whole array */
    iArrayIndex = mxGetFieldNumber(pShipGPSArray,"dCourseOverGround");
    
    /* allocate heap for a single element, DON'T USE VARIABLES ON STACK !!! */
    pValue = mxCalloc(lDataSets,sizeof(double));

	pTempPointer = ptrAuxStatus;
	pDblTarget = pValue;

	for(iElement = 0; iElement < lDataSets; iElement++)
	{
		/* copy the values into the allocated heap space */
		memcpy((void*)pDblTarget,&(pTempPointer->ShipGPS.dCourseOverGround),sizeof(double));
		pTempPointer++;
		pDblTarget++;
	}

    /* let the pointer of the array point to the heap space with our data */
    mxSetData(pShipGPSCourseOverGroundArray,pValue);
    mxSetFieldByNumber(pShipGPSArray,0,iArrayIndex,pShipGPSCourseOverGroundArray);

	/**************************/
	/* SUBSTRUCTURE ShipSonar */
	/**************************/

	pShipSonarArray = mxCreateStructArray(2, small_dims, NUMBER_OF_SHIPSONAR_FIELDS, ShipSonarFieldNames);

	/*************************************/
	/* 1st field of substruct ShipSonar  */
	/*************************************/

    /* create 1xM matrix with proper data type */
    pShipSonarFrequencyArray = mxCreateNumericArray(2, dims, mxDOUBLE_CLASS, mxREAL);
    
    /* determine at which index to hook in the whole array */
    iArrayIndex = mxGetFieldNumber(pShipSonarArray,"dFrequency");
    
    /* allocate heap for a single element, DON'T USE VARIABLES ON STACK !!! */
    pValue = mxCalloc(lDataSets,sizeof(double));

	pTempPointer = ptrAuxStatus;
	pDblTarget = pValue;

	for(iElement = 0; iElement < lDataSets; iElement++)
	{
		/* copy the values into the allocated heap space */
		memcpy((void*)pDblTarget,&(pTempPointer->ShipSonar.dFrequency),sizeof(double));
		pTempPointer++;
		pDblTarget++;
	}

    /* let the pointer of the array point to the heap space with our data */
    mxSetData(pShipSonarFrequencyArray,pValue);
    mxSetFieldByNumber(pShipSonarArray,0,iArrayIndex,pShipSonarFrequencyArray);

	/*************************************/
	/* 2nd field of substruct ShipSonar  */
	/*************************************/

    /* create 1xM matrix with proper data type */
    pShipSonarWaterDepthArray = mxCreateNumericArray(2, dims, mxDOUBLE_CLASS, mxREAL);
    
    /* determine at which index to hook in the whole array */
    iArrayIndex = mxGetFieldNumber(pShipSonarArray,"dWaterDepth");
    
    /* allocate heap for a single element, DON'T USE VARIABLES ON STACK !!! */
    pValue = mxCalloc(lDataSets,sizeof(double));

	pTempPointer = ptrAuxStatus;
	pDblTarget = pValue;

	for(iElement = 0; iElement < lDataSets; iElement++)
	{
		/* copy the values into the allocated heap space */
		memcpy((void*)pDblTarget,&(pTempPointer->ShipSonar.dWaterDepth),sizeof(double));
		pTempPointer++;
		pDblTarget++;
	}

    /* let the pointer of the array point to the heap space with our data */
    mxSetData(pShipSonarWaterDepthArray,pValue);
    mxSetFieldByNumber(pShipSonarArray,0,iArrayIndex,pShipSonarWaterDepthArray);

	/**************************/
	/* SUBSTRUCTURE ShipGyro  */
	/**************************/

	pShipGyroArray = mxCreateStructArray(2, small_dims, NUMBER_OF_SHIPGYRO_FIELDS, ShipGyroFieldNames);

	/*************************************/
	/* 1st field of substruct ShipGyro  */
	/*************************************/

    /* create 1xM matrix with proper data type */
    pShipGyroDirectionArray = mxCreateNumericArray(2, dims, mxDOUBLE_CLASS, mxREAL);
    
    /* determine at which index to hook in the whole array */
    iArrayIndex = mxGetFieldNumber(pShipGyroArray,"dDirection");
    
    /* allocate heap for a single element, DON'T USE VARIABLES ON STACK !!! */
    pValue = mxCalloc(lDataSets,sizeof(double));

	pTempPointer = ptrAuxStatus;
	pDblTarget = pValue;

	for(iElement = 0; iElement < lDataSets; iElement++)
	{
		/* copy the values into the allocated heap space */
		memcpy((void*)pDblTarget,&(pTempPointer->ShipGyro.dDirection),sizeof(double));
		pTempPointer++;
		pDblTarget++;
	}

    /* let the pointer of the array point to the heap space with our data */
    mxSetData(pShipGyroDirectionArray,pValue);
    mxSetFieldByNumber(pShipGyroArray,0,iArrayIndex,pShipGyroDirectionArray);

	/**************************/
	/* SUBSTRUCTURE ShipWater */
	/**************************/

	pShipWaterArray = mxCreateStructArray(2, small_dims, NUMBER_OF_SHIPWATER_FIELDS, ShipWaterFieldNames);

	/*************************************/
	/* 1st field of substruct ShipWater  */
	/*************************************/

    /* create 1xM matrix with proper data type */
    pShipWaterSalinityArray = mxCreateNumericArray(2, dims, mxDOUBLE_CLASS, mxREAL);
    
    /* determine at which index to hook in the whole array */
    iArrayIndex = mxGetFieldNumber(pShipWaterArray,"dSalinity");
    
    /* allocate heap for a single element, DON'T USE VARIABLES ON STACK !!! */
    pValue = mxCalloc(lDataSets,sizeof(double));

	pTempPointer = ptrAuxStatus;
	pDblTarget = pValue;

	for(iElement = 0; iElement < lDataSets; iElement++)
	{
		/* copy the values into the allocated heap space */
		memcpy((void*)pDblTarget,&(pTempPointer->ShipWater.dSalinity),sizeof(double));
		pTempPointer++;
		pDblTarget++;
	}

    /* let the pointer of the array point to the heap space with our data */
    mxSetData(pShipWaterSalinityArray,pValue);
    mxSetFieldByNumber(pShipWaterArray,0,iArrayIndex,pShipWaterSalinityArray);

	/*************************************/
	/* 2nd field of substruct ShipWater  */
	/*************************************/

    /* create 1xM matrix with proper data type */
    pShipWaterTemperatureArray = mxCreateNumericArray(2, dims, mxDOUBLE_CLASS, mxREAL);
    
    /* determine at which index to hook in the whole array */
    iArrayIndex = mxGetFieldNumber(pShipWaterArray,"dWaterTemp");
    
    /* allocate heap for a single element, DON'T USE VARIABLES ON STACK ! */
    pValue = mxCalloc(lDataSets,sizeof(double));

	pTempPointer = ptrAuxStatus;
	pDblTarget = pValue;

	for(iElement = 0; iElement < lDataSets; iElement++)
	{
		/* copy the values into the allocated heap space */
		memcpy((void*)pDblTarget,&(pTempPointer->ShipWater.dWaterTemp),sizeof(double));
		pTempPointer++;
		pDblTarget++;
	}

    /* let the pointer of the array point to the heap space with our data */
    mxSetData(pShipWaterTemperatureArray,pValue);
    mxSetFieldByNumber(pShipWaterArray,0,iArrayIndex,pShipWaterTemperatureArray);


	/**************************/
	/* SUBSTRUCTURE ShipMeteo */
	/**************************/

	pShipMeteoArray = mxCreateStructArray(2, small_dims, NUMBER_OF_SHIPMETEO_FIELDS, ShipMeteoFieldNames);

	/*************************************/
	/* 1st field of substruct ShipMeteo  */
	/*************************************/

    /* create 1xM matrix with proper data type */
    pShipMeteoWindSpeedArray = mxCreateNumericArray(2, dims, mxDOUBLE_CLASS, mxREAL);
    
    /* determine at which index to hook in the whole array */
    iArrayIndex = mxGetFieldNumber(pShipMeteoArray,"dWindSpeed");
    
    /* allocate heap for a single element, DON'T USE VARIABLES ON STACK !!! */
    pValue = mxCalloc(lDataSets,sizeof(double));

	pTempPointer = ptrAuxStatus;
	pDblTarget = pValue;

	for(iElement = 0; iElement < lDataSets; iElement++)
	{
		/* copy the values into the allocated heap space */
		memcpy((void*)pDblTarget,&(pTempPointer->ShipMeteo.dWindSpeed),sizeof(double));
		pTempPointer++;
		pDblTarget++;
	}

    /* let the pointer of the array point to the heap space with our data */
    mxSetData(pShipMeteoWindSpeedArray,pValue);
    mxSetFieldByNumber(pShipMeteoArray,0,iArrayIndex,pShipMeteoWindSpeedArray);

	/*************************************/
	/* 2nd field of substruct ShipMeteo  */
	/*************************************/

    /* create 1xM matrix with proper data type */
    pShipMeteoWindDirectionArray = mxCreateNumericArray(2, dims, mxDOUBLE_CLASS, mxREAL);
    
    /* determine at which index to hook in the whole array */
    iArrayIndex = mxGetFieldNumber(pShipMeteoArray,"dWindDirection");
    
    /* allocate heap for a single element, DON'T USE VARIABLES ON STACK ! */
    pValue = mxCalloc(lDataSets,sizeof(double));

	pTempPointer = ptrAuxStatus;
	pDblTarget = pValue;

	for(iElement = 0; iElement < lDataSets; iElement++)
	{
		/* copy the values into the allocated heap space */
		memcpy((void*)pDblTarget,&(pTempPointer->ShipMeteo.dWindDirection),sizeof(double));
		pTempPointer++;
		pDblTarget++;
	}

    /* let the pointer of the array point to the heap space with our data */
    mxSetData(pShipMeteoWindDirectionArray,pValue);
    mxSetFieldByNumber(pShipMeteoArray,0,iArrayIndex,pShipMeteoWindDirectionArray);

	/*******************************/
	/* SUBSTRUCTURE LicorPressure  */
	/*******************************/

	pLicorPressureArray = mxCreateStructArray(2, small_dims, NUMBER_OF_LICORPRESSURE_FIELDS, ShipLicorPressureFieldNames);

	/*************************************/
	/* 1st field of substruct LicorPressure  */
	/*************************************/

    /* create 1xM matrix with proper data type */
    pShipLicorPressureArray = mxCreateNumericArray(2, dims, mxUINT16_CLASS, mxREAL);
    
    /* determine at which index to hook in the whole array */
    iArrayIndex = mxGetFieldNumber(pLicorPressureArray,"uiAmbientPressure");
    
    /* allocate heap for a single element, DON'T USE VARIABLES ON STACK !!! */
    pValue = mxCalloc(lDataSets,sizeof(uint16_t));

	pTempPointer = ptrAuxStatus;
	pU16Target = pValue;

	for(iElement = 0; iElement < lDataSets; iElement++)
	{
		/* copy the values into the allocated heap space */
		memcpy((void*)pU16Target,&(pTempPointer->LicorPressure.uiAmbientPressure),sizeof(uint16_t));
		pTempPointer++;
		pU16Target++;
	}

    /* let the pointer of the array point to the heap space with our data */
    mxSetData(pShipLicorPressureArray,pValue);
    mxSetFieldByNumber(pLicorPressureArray,0,iArrayIndex,pShipLicorPressureArray);

	/************************/
	/* SUBSTRUCTURE Status  */
	/************************/

	pStatusArray = mxCreateStructArray(2, small_dims, NUMBER_OF_STATUS_FIELDS, StatusFieldNames);

	/**********************************/
	/* 1st field of substruct Status  */
	/**********************************/

    /* create 1xM matrix with proper data type */
    pMeteoBoxDataValidArray = mxCreateNumericArray(2, dims, mxLOGICAL_CLASS, mxREAL);
    
    /* determine at which index to hook in the whole array */
    iArrayIndex = mxGetFieldNumber(pStatusArray,"MeteoBoxDataValid");
    
    /* allocate heap for a single element, DON'T USE VARIABLES ON STACK !!! */
    pValue = mxCalloc(lDataSets,sizeof(uint8_t));

	pTempPointer = ptrAuxStatus;
	pU08Target = pValue;

	for(iElement = 0; iElement < lDataSets; iElement++)
	{
		if(pTempPointer->Status.Status.Word & 0x8000)
		{
			/* check if bit is set, don't use the bitfields here because they are non portable */
			/* GNU GCC - MSVC++ 2005 :-( */
			pBoolTarget = &ucTrue;
		}
		else
		{
			pBoolTarget = &ucFalse;
		}
			

		/* copy the values into the allocated heap space */
		memcpy((void*)pU08Target,pBoolTarget,sizeof(uint8_t));
		pTempPointer++;
		pU08Target++;
	}

    /* let the pointer of the array point to the heap space with our data */
    mxSetData(pMeteoBoxDataValidArray,pValue);
    mxSetFieldByNumber(pStatusArray,0,iArrayIndex,pMeteoBoxDataValidArray);

	/**********************************/
	/* 2nd field of substruct Status  */
	/**********************************/

    /* create 1xM matrix with proper data type */
    pShipGPSDataValidArray = mxCreateNumericArray(2, dims, mxLOGICAL_CLASS, mxREAL);
    
    /* determine at which index to hook in the whole array */
    iArrayIndex = mxGetFieldNumber(pStatusArray,"ShipGPSDataValid");
    
    /* allocate heap for a single element, DON'T USE VARIABLES ON STACK !!! */
    pValue = mxCalloc(lDataSets,sizeof(uint8_t));

	pTempPointer = ptrAuxStatus;
	pU08Target = pValue;

	for(iElement = 0; iElement < lDataSets; iElement++)
	{
		if(pTempPointer->Status.Status.Word & 0x4000)
		{
			/* check if bit is set, don't use the bitfields here because they are non portable */
			/* GNU GCC - MSVC++ 2005 :-( */
			pBoolTarget = &ucTrue;
		}
		else
		{
			pBoolTarget = &ucFalse;
		}
			

		/* copy the values into the allocated heap space */
		memcpy((void*)pU08Target,pBoolTarget,sizeof(uint8_t));
		pTempPointer++;
		pU08Target++;
	}

    /* let the pointer of the array point to the heap space with our data */
    mxSetData(pShipGPSDataValidArray,pValue);
    mxSetFieldByNumber(pStatusArray,0,iArrayIndex,pShipGPSDataValidArray);

	/**********************************/
	/* 3rd field of substruct Status  */
	/**********************************/

    /* create 1xM matrix with proper data type */
    pShipMeteoDataValidArray = mxCreateNumericArray(2, dims, mxLOGICAL_CLASS, mxREAL);
    
    /* determine at which index to hook in the whole array */
    iArrayIndex = mxGetFieldNumber(pStatusArray,"ShipMeteoDataValid");
    
    /* allocate heap for a single element, DON'T USE VARIABLES ON STACK !!! */
    pValue = mxCalloc(lDataSets,sizeof(uint8_t));

	pTempPointer = ptrAuxStatus;
	pU08Target = pValue;

	for(iElement = 0; iElement < lDataSets; iElement++)
	{
		if(pTempPointer->Status.Status.Word & 0x2000)
		{
			/* check if bit is set, don't use the bitfields here because they are non portable */
			/* GNU GCC - MSVC++ 2005 :-( */
			pBoolTarget = &ucTrue;
		}
		else
		{
			pBoolTarget = &ucFalse;
		}
			

		/* copy the values into the allocated heap space */
		memcpy((void*)pU08Target,pBoolTarget,sizeof(uint8_t));
		pTempPointer++;
		pU08Target++;
	}

    /* let the pointer of the array point to the heap space with our data */
    mxSetData(pShipMeteoDataValidArray,pValue);
    mxSetFieldByNumber(pStatusArray,0,iArrayIndex,pShipMeteoDataValidArray);

	/**********************************/
	/* 4th field of substruct Status  */
	/**********************************/

    /* create 1xM matrix with proper data type */
    pShipSonarDataValidArray = mxCreateNumericArray(2, dims, mxLOGICAL_CLASS, mxREAL);
    
    /* determine at which index to hook in the whole array */
    iArrayIndex = mxGetFieldNumber(pStatusArray,"ShipSonarDataValid");
    
    /* allocate heap for a single element, DON'T USE VARIABLES ON STACK !!! */
    pValue = mxCalloc(lDataSets,sizeof(uint8_t));

	pTempPointer = ptrAuxStatus;
	pU08Target = pValue;

	for(iElement = 0; iElement < lDataSets; iElement++)
	{
		if(pTempPointer->Status.Status.Word & 0x1000)
		{
			/* check if bit is set, don't use the bitfields here because they are non portable */
			/* GNU GCC - MSVC++ 2005 :-( */
			pBoolTarget = &ucTrue;
		}
		else
		{
			pBoolTarget = &ucFalse;
		}
			

		/* copy the values into the allocated heap space */
		memcpy((void*)pU08Target,pBoolTarget,sizeof(uint8_t));
		pTempPointer++;
		pU08Target++;
	}

    /* let the pointer of the array point to the heap space with our data */
    mxSetData(pShipSonarDataValidArray,pValue);
    mxSetFieldByNumber(pStatusArray,0,iArrayIndex,pShipSonarDataValidArray);

	/**********************************/
	/* 5th field of substruct Status  */
	/**********************************/

    /* create 1xM matrix with proper data type */
    pShipGyroDataValidArray = mxCreateNumericArray(2, dims, mxLOGICAL_CLASS, mxREAL);
    
    /* determine at which index to hook in the whole array */
    iArrayIndex = mxGetFieldNumber(pStatusArray,"ShipGyroDataValid");
    
    /* allocate heap for a single element, DON'T USE VARIABLES ON STACK !!! */
    pValue = mxCalloc(lDataSets,sizeof(uint8_t));

	pTempPointer = ptrAuxStatus;
	pU08Target = pValue;

	for(iElement = 0; iElement < lDataSets; iElement++)
	{
		if(pTempPointer->Status.Status.Word & 0x0800)
		{
			/* check if bit is set, don't use the bitfields here because they are non portable */
			/* GNU GCC - MSVC++ 2005 :-( */
			pBoolTarget = &ucTrue;
		}
		else
		{
			pBoolTarget = &ucFalse;
		}
			

		/* copy the values into the allocated heap space */
		memcpy((void*)pU08Target,pBoolTarget,sizeof(uint8_t));
		pTempPointer++;
		pU08Target++;
	}

    /* let the pointer of the array point to the heap space with our data */
    mxSetData(pShipGyroDataValidArray,pValue);
    mxSetFieldByNumber(pStatusArray,0,iArrayIndex,pShipGyroDataValidArray);

	/**********************************/
	/* 6th field of substruct Status  */
	/**********************************/

    /* create 1xM matrix with proper data type */
    pShipWaterDataValidArray = mxCreateNumericArray(2, dims, mxLOGICAL_CLASS, mxREAL);
    
    /* determine at which index to hook in the whole array */
    iArrayIndex = mxGetFieldNumber(pStatusArray,"ShipWaterDataValid");
    
    /* allocate heap for a single element, DON'T USE VARIABLES ON STACK !!! */
    pValue = mxCalloc(lDataSets,sizeof(uint8_t));

	pTempPointer = ptrAuxStatus;
	pU08Target = pValue;

	for(iElement = 0; iElement < lDataSets; iElement++)
	{
		if(pTempPointer->Status.Status.Word & 0x0400)
		{
			/* check if bit is set, don't use the bitfields here because they are non portable */
			/* GNU GCC - MSVC++ 2005 :-( */
			pBoolTarget = &ucTrue;
		}
		else
		{
			pBoolTarget = &ucFalse;
		}
			

		/* copy the values into the allocated heap space */
		memcpy((void*)pU08Target,pBoolTarget,sizeof(uint8_t));
		pTempPointer++;
		pU08Target++;
	}

    /* let the pointer of the array point to the heap space with our data */
    mxSetData(pShipWaterDataValidArray,pValue);
    mxSetFieldByNumber(pStatusArray,0,iArrayIndex,pShipWaterDataValidArray);

	/**********************************/
	/* 7th field of substruct Status  */
	/**********************************/

    /* create 1xM matrix with proper data type */
    pLicorPressureDataValidArray = mxCreateNumericArray(2, dims, mxLOGICAL_CLASS, mxREAL);
    
    /* determine at which index to hook in the whole array */
    iArrayIndex = mxGetFieldNumber(pStatusArray,"LicorPressureDataValid");
    
    /* allocate heap for a single element, DON'T USE VARIABLES ON STACK !!! */
    pValue = mxCalloc(lDataSets,sizeof(uint8_t));

	pTempPointer = ptrAuxStatus;
	pU08Target = pValue;

	for(iElement = 0; iElement < lDataSets; iElement++)
	{
		if(pTempPointer->Status.Status.Word & 0x0200)
		{
			/* check if bit is set, don't use the bitfields here because they are non portable */
			/* GNU GCC - MSVC++ 2005 :-( */
			pBoolTarget = &ucTrue;
		}
		else
		{
			pBoolTarget = &ucFalse;
		}
			

		/* copy the values into the allocated heap space */
		memcpy((void*)pU08Target,pBoolTarget,sizeof(uint8_t));
		pTempPointer++;
		pU08Target++;
	}

    /* let the pointer of the array point to the heap space with our data */
    mxSetData(pLicorPressureDataValidArray,pValue);
    mxSetFieldByNumber(pStatusArray,0,iArrayIndex,pLicorPressureDataValidArray);


	/*********************************************/
    /* link main struct array to substruct array */
	/*********************************************/

	iArrayIndex = mxGetFieldNumber(plhs[0],"TimeOfDayAux");
    mxSetFieldByNumber(plhs[0],0,iArrayIndex,pTimeOfDayAuxArray);

    iArrayIndex = mxGetFieldNumber(plhs[0],"MeteoBox");
    mxSetFieldByNumber(plhs[0],0,iArrayIndex,pMeteoBoxArray);

    iArrayIndex = mxGetFieldNumber(plhs[0],"ShipGPS");
    mxSetFieldByNumber(plhs[0],0,iArrayIndex,pShipGPSArray);

	iArrayIndex = mxGetFieldNumber(plhs[0],"ShipSonar");
    mxSetFieldByNumber(plhs[0],0,iArrayIndex,pShipSonarArray);

	iArrayIndex = mxGetFieldNumber(plhs[0],"ShipMeteo");
    mxSetFieldByNumber(plhs[0],0,iArrayIndex,pShipMeteoArray);

	iArrayIndex = mxGetFieldNumber(plhs[0],"ShipGyro");
    mxSetFieldByNumber(plhs[0],0,iArrayIndex,pShipGyroArray);

	iArrayIndex = mxGetFieldNumber(plhs[0],"ShipWater");
    mxSetFieldByNumber(plhs[0],0,iArrayIndex,pShipWaterArray);

	iArrayIndex = mxGetFieldNumber(plhs[0],"LicorPressure");
    mxSetFieldByNumber(plhs[0],0,iArrayIndex,pLicorPressureArray);

	iArrayIndex = mxGetFieldNumber(plhs[0],"Status");
    mxSetFieldByNumber(plhs[0],0,iArrayIndex,pStatusArray);

    #ifdef X_DEBUG
       mexPrintf("done \n");
    #endif
}

