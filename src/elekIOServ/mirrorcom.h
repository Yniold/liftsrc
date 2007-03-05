// ============================================
// Mirror.h
// Headerfile
// ============================================

// $RCSfile: mirrorcom.h,v $ last changed on $Date: 2007-03-05 16:28:36 $ by $Author: martinez $

// History:
//
// $Log: mirrorcom.h,v $
// Revision 1.1  2007-03-05 16:28:36  martinez
// implemented mirrors
//
// Revision 1.1  2007-2-27 14:34:59  martinez
//
//
//

#ifndef Mirror_H
#define Mirror_H

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


// ====================================
// Globals for Mirror Control Thread
// ====================================

extern long lMirrorBaudrate;
extern int iMirrorFile;
extern unsigned char ucMirrorDeviceName[];
extern unsigned char ucMirrorDeviceOpened;
extern char aMirrorRxBuffer[];

enum PosCommandStatus
{
	POS_IDLE,
	POS_PREP1,
	POS_PREP2,
	POS_PREP3,
	POS_MOVING,
	
	MAX_POS_STATUS
};

struct  sMirrorType
{
	int iFD;
	uint16_t Mirror;
	uint16_t Axis;
	int32_t RelPositionSet;
	int32_t CurrentRelPos;
 	int32_t CurrentAbsPos;
	uint16_t CommandSent;
	uint16_t PosCommandStatus;
	uint16_t ReadPosCommand;
	uint16_t StopFlag;
};

extern struct sMirrorType sMirrorThread;
extern pthread_mutex_t mMirrorMutex;

// ====================================
// prototypes
// ====================================

extern int MirrorInit(void);

extern void MirrorThreadFunc(void* pArgument);
extern void MirrorParseLine(char* aBuffer, int iLength, struct sMirrorType* sStructure);
extern void MirrorUpdateData(void);



#endif
