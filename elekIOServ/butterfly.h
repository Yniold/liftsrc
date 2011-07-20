// ============================================
// butterfly.h
// Headerfile
// ============================================

// $RCSfile: butterfly.h,v $ last changed on $Date: 2006-10-05 14:34:59 $ by $Author: rudolf $

// History:
//
// $Log: butterfly.h,v $
// Revision 1.1  2006-10-05 14:34:59  rudolf
// preperations for butterfly integration
//
//
//

#ifndef BUTTERFLY_H
#define BUTTERFLY_H

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

#ifndef false 
#define false 0
#endif

#ifndef true
#define true (!false)
#endif

#define MAGIC_POSITION_NUMBER (0x4711)

// ====================================
// Globals for Butterfly Control Thread
// ====================================

extern long lButterflyBaudrate;
extern int iButterflyFile;
extern unsigned char ucButterflyDeviceName[];
extern unsigned char ucButterflyDeviceOpened;
extern unsigned char aButterflyRxBuffer[];
extern unsigned char aButterflyTxBuffer[];

struct  sButterflyType
{
	int iFD;
	int iCommand;
	uint16_t sCurrentPosition;
	uint16_t sTargetPositionRead;
	uint16_t sTargetPositionSet;
	uint16_t sMotorControlWord;
	unsigned char ucPositionValid;
	unsigned char ucCPUFlags;
};

extern struct sButterflyType sButterflyThread;
extern pthread_mutex_t mButterflyMutex;

// ====================================
// prototypes
// ====================================

extern int ButterflyInit(void);

extern void ButterflyThreadFunc(void* pArgument);
extern void ButterflyParseLine(unsigned char* aBuffer, int iLength, struct sButterflyType* sStructure);
extern void ButterflyUpdateData(void);



#endif
