// ============================================
// licor.h
// Headerfile
// ============================================

// $RCSfile: licor.h,v $ last changed on $Date: 2007-02-12 18:32:18 $ by $Author: rudolf $

// History:
//
// $Log: licor.h,v $
// Revision 1.1  2007-02-12 18:32:18  rudolf
// added missing files for calibrator
//
//
//
//

#ifndef LICOR_H
#define LICOR_H

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
// Globals for Licor Control Thread
// ====================================

extern long lLicorBaudrate;
extern int iLicorFile;
extern unsigned char ucLicorDeviceName[];
extern unsigned char ucLicorDeviceOpened;
extern unsigned char aLicorRxBuffer[];
extern unsigned char aLicorTxBuffer[];

struct  sLicorType
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

extern struct sLicorType sLicorThread;
extern pthread_mutex_t mLicorMutex;

// ====================================
// prototypes
// ====================================

extern int LicorInit(void);

extern void LicorThreadFunc(void* pArgument);
extern void LicorParseLine(unsigned char* aBuffer, int iLength, struct sLicorType* sStructure);
extern void LicorUpdateData(void);
extern void HexDump(unsigned char *aBuffer, int iBytes); 
// ====================================
// state machine definition for parser
// ====================================

enum LicorDecodeStateEnum 
{
   LIC_STATE_WAIT_FOR_CR,
   LIC_STATE_PARSE_LINE,
   LIC_STATE_MAX
};


#endif
