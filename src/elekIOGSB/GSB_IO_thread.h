// ============================================
// GSB_IO_thread.h
// Headerfile
// ============================================


#ifndef GSB_IOTHREAD_H
#define GSB_IOTHREAD_H

#include "../include/elekIO.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "gsb.h"

#ifndef false 
#define false 0
#endif

#ifndef true
#define true (!false)
#endif

struct  sGSBIOThreadType
{
	int iFD;
	int iCommand;
	struct GSBStatusType *psStatus;
	struct sI2CMessageQueue *pMessageQueue;
};

typedef struct sADCStruct0_t
{
	int iChannel[8];
	int iTemperature;
	double dMFC0Flow;
	double dMFC1Flow;
	double dMFC2Flow;
	double dPressSens0;
	double dPressSens1;
	double dPressSens2;
	double dPT100Temp0;
	double dPT100Temp1;
	double dADCTemp;
		
} RawADCType0;

typedef struct sADCStruct1_t
{
	int iChannel[8];
	int iTemperature;
	double dPressSens0;
	double dPressSens1;
	double dPressSens2;
	double dPT100Temp0;
	double dADCTemp;

} RawADCType1;

struct sRawADCStruct
{
	struct sADCStruct0_t ADC0;
	struct sADCStruct1_t ADC1;
};

extern struct sGSBIOThreadType sGSBIOThread;
extern struct GSBStatusType *pGSBStatus;
extern pthread_mutex_t mGSBIOThreadMutex;

// ====================================
// prototypes
// ====================================

extern int GSBIOThreadInit(void);
extern void GSBIOThreadFunc(void* pArgument);
extern int i2c_recv_from_avr(int fd, int command, char *rxdata, unsigned int *recvlen);
extern int ADC_FetchData(int iFD, int iADC, struct sRawADCStruct *sADCStruct);
extern int DAC_SetChannel(int iFD, int iChannel, unsigned short usDACValue);

#endif
