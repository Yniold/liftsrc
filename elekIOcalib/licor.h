// ============================================
// licor.h
// Headerfile
// ============================================
//
// $RCSfile: licor.h,v $ last changed on $Date: 2007-07-12 17:37:56 $ by $Author: rudolf $
//
// History:
//
// $Log: licor.h,v $
// Revision 1.4  2007-07-12 17:37:56  rudolf
// LICOR channels can also be negative, changed uint16 to int16
//
// Revision 1.3  2007-02-21 16:06:30  rudolf
// licor struct now read from parsing thread and copied to status struct
//
// Revision 1.2  2007-02-21 13:17:28  rudolf
// more work on structure for licor
//
// Revision 1.1  2007-02-12 18:32:18  rudolf
// added missing files for calibrator
//
//
//
//

#ifndef LICOR_H
# define LICOR_H

# include <stdio.h>
# include <stdlib.h>
# include <unistd.h>
# include <time.h>
# include <string.h>
# include <sys/types.h>
# include <sys/socket.h>
# include <sys/time.h>
# include <netinet/in.h>
# include <arpa/inet.h>
# include <netdb.h>

# ifndef false
#  define false 0
# endif

# ifndef true
#  define true (!false)
# endif

// ====================================
// Globals for Licor Control Thread
// ====================================
//
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
   uint16_t LicorTemperature; /* Unit: degree kelvin * 100 e.g. 20 degree celsius -> 273,15 + 20,0 => 29315 */
   uint16_t AmbientPressure;  /* Unit: kPA * 100 e.g. 1002.7 mBar => 10027 */

   int16_t CO2A;             /* CO2 concentration cell A in mymol/mol, coding scheme T.B.D. */
   int16_t CO2B;             /* CO2 concentration cell B in mymol/mol, coding scheme T.B.D. */
   int16_t CO2D;              /* CO2 differential concentration in mymol/mol, coding scheme T.B.D. */

   int16_t H2OA;             /* H2O concentration cell A in mmol/mol, coding scheme T.B.D. */
   int16_t H2OB;             /* H2O concentration cell B in mmol/mol, coding scheme T.B.D. */
   int16_t H2OD;              /* H2O differential concentration in mmol/mol, coding scheme T.B.D. */
};

extern struct sLicorType sLicorThread;
extern pthread_mutex_t mLicorMutex;

// ====================================
// prototypes
// ====================================
//
extern int LicorInit(void);

extern void LicorThreadFunc(void* pArgument);
extern void LicorParseLine(unsigned char* aBuffer, int iLength, struct sLicorType* sStructure);
extern void LicorUpdateData(void);
extern void HexDump(unsigned char *aBuffer, int iBytes);
// ====================================
// state machine definition for parser
// ====================================
//
enum LicorDecodeStateEnum
{
   LIC_STATE_WAIT_FOR_CR,
     LIC_STATE_PARSE_LINE,
     LIC_STATE_MAX
};

#endif
