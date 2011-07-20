// ============================================
// shipdata.h
// Headerfile
// ============================================
//
// $RCSfile: shipdata.h,v $ last changed on $Date: 2007-03-07 18:11:29 $ by $Author: rudolf $
//
// History:
//
//
//
//
//

#ifndef SHIPDATA_H
# define SHIPDATA_H

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
// Globals for Shipdata Control Thread
// ====================================
//

struct  sShipDataType
{
   int      iFD;                      /* socket FD */
   union AuxStatusType Valid;         /* signal if data is valid or not to main thread */
   //sonar
   double   dFrequency;               /* Frequency used for sonar */
   double   dWaterDepth;              /* meters */
   // anemometer
   double   dWindSpeed;               /* wind speed (ULTRASONIC)*/
   double   dWindDirection;           /* wind direction (ULTRASONIC */
   // gyro
   double   dDirection;               /* direction (GYRO) */
   // water analyser
   double   dSalinity;                /* gramms per litre */
   double   dWaterTemp;               /* water temp in degrees celsius */
   // GPS
   unsigned char ucUTCHours;	      /* binary, not BCD coded (!) 0 - 23 decimal*/
   unsigned char ucUTCMins;	      /* binary, 0-59 decimal */
   unsigned char ucUTCSeconds;	      /* binary 0-59 decimal */
   unsigned char ucUTCDay;            /* day 1-31 */
   unsigned char ucUTCMonth;          /* month 1-12 */
   uint16_t      uiUTCYear;           /* year 4 digits */
   double dLongitude;		      /* "Laengengrad" I always mix it up...
					 signed notation,
					 negative values mean "W - west of Greenwich"
					 positive values mean "E - east of Greenwich" */

   double dLatitude;		      /* "Breitengrad" I always mix it up...
					 signed notation,
					 negative values mean "S - south of the equator"
					 positive values mean "N - north of the equator"*/
   double dGroundSpeed;		      /* speed in knots above ground */
   double dCourseOverGround;	      /* heading in degrees */

};

extern struct sShipDataType sShipDataThread;
extern pthread_mutex_t mShipDataMutex;
extern char gPlotData;

// ====================================
// prototypes
// ====================================
//
extern int ShipDataInit(void);

extern void ShipDataThreadFunc(void* pArgument);

extern void ShipDataParseGPSBuffer(char* pBuffer, int iBuffLen, struct sShipDataType* sDataStructure);
extern void ShipDataParseWaterBuffer(char* pBuffer, int iBuffLen, struct sShipDataType* sDataStructure);
extern void ShipDataParseSonarBuffer(char* pBuffer, int iBuffLen, struct sShipDataType* sDataStructure);
extern void ShipDataParseGyroBuffer(char* pBuffer, int iBuffLen, struct sShipDataType* sDataStructure);
extern void ShipDataParseAnemoBuffer(char* pBuffer, int iBuffLen, struct sShipDataType* sDataStructure);

#endif
