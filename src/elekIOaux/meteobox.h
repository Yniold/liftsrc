// ============================================
// meteobox.h
// Headerfile
// ============================================
//
// $RCSfile: meteobox.h,v $ last changed on $Date: 2007-03-04 19:28:41 $ by $Author: rudolf $
//
// History:
//
// $Log: meteobox.h,v $
// Revision 1.2  2007-03-04 19:28:41  rudolf
// added parsing for data into the right structure elements
//
// Revision 1.1  2007-03-04 13:41:59  rudolf
// created new server for auxilliary data like weather data, ships GPS etc
//
//
//
//
//

#ifndef METEOBOX_H
# define METEOBOX_H

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

#define XPORT_IP_ADDRESS "10.111.111.27"
#define XPORT_PORTNUMBER 10001

// ====================================
// Globals for Meteobox Control Thread
// ====================================
//

struct  sMeteoBoxType
{
   int      iFD;                      /* socket FD */
   int      iValidFlag;               /* signal if data is valid or not to main thread */
   
   double   dWindSpeed;               /* Windspeed in m/s */
   uint16_t uiWindDirection;          /* 45° resolution */
   double   dRelHum;                  /* 000.0 - 100.0 % */
   double   dAirTemp;                 /* Temperature in degree celsius */
   double   dGasSensorVoltage;        /* dirt sensor */
};

extern struct sMeteoBoxType sMeteoBoxThread;
extern pthread_mutex_t mMeteoBoxMutex;

// ====================================
// prototypes
// ====================================
//
extern int MeteoBoxInit(void);

extern void MeteoBoxThreadFunc(void* pArgument);

extern void MeteoBoxParseBuffer(char* pBuffer, int iBuffLen, struct sMeteoBoxType* sDataStructure);
#endif
