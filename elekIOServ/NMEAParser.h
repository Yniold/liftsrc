// ============================================
// NMEA Parser
// Headerfile
// ============================================

// $RCSfile: NMEAParser.h,v $ last changed on $Date: 2005/01/31 11:28:31 $ by $Author: rudolf $

// History:
//
// $Log: NMEAParser.h,v $
// Revision 1.4  2005/01/31 11:28:31  rudolf
// fixed syntax errors
//
// Revision 1.3  2005/01/31 11:27:18  rudolf
// Beautifications and added variables for GPVTG NMEA sentence
//
// Revision 1.2  2005/01/27 18:16:29  rudolf
// changed NMEA parser to work with elekIO serv
//
// Revision 1.1  2005/01/27 14:59:36  rudolf
// added files for GPS receiver connection
//
//

#ifndef _NMEAPARSER_H_
#define _NMEAPARSER_H_

#ifndef TRUE
#define TRUE (1)
#endif

#ifndef FALSE
#define FALSE (0)
#endif

enum NP_STATE {
               NP_STATE_SOM,        // Search for start of message
               NP_STATE_CMD,        // Get command
               NP_STATE_DATA,       // Get data
               NP_STATE_CHECKSU1,   // Get first checksum character
               NP_STATE_CHECKSU2,   // get second checksum character
};

#define NP_MAX_CMD_LEN  8           // maximum command length (NMEA address)
#define NP_MAX_DATA_LEN 256         // maximum data length
#define NP_MAX_CHAN 36              // maximum number of channels
#define NP_WAYPOINT_ID_LEN 32       // waypoint max string len

// ==============
// Globals
// ==============

enum NP_STATE nState;                     // Parser Statemachine
unsigned char ucChecksum;                 // Calculated NMEA sentence checksum
unsigned char ucReceivedChecksum;         // Received NMEA sentence checksum (if exists)
int wIndex;                               // Index used for command and data
unsigned char pCommand[NP_MAX_CMD_LEN];   // NMEA command
unsigned char pData[NP_MAX_DATA_LEN];     // NMEA data

int dwCommandCount;                       // number of NMEA commands received (processed or not processed)

// ==========================================
// GPGGA Data (Time/Position/Quality dataset
// ==========================================

unsigned char ucGGAHour;                  // UTC Time
unsigned char ucGGAMinute;                //
unsigned char ucGGASecond;                //

double dGGALatitude;                      // < 0 = South, > 0 = North
double dGGALongitude;                     // < 0 = West, > 0 = East

unsigned char ucGGAGPSQuality;            // 0 = fix not available,
                                          // 1 = GPS sps mode,
                                          // 2 = Differential GPS, SPS mode, fix valid,
                                          // 3 = GPS PPS mode, fix valid

unsigned char ucGGANumOfSatsInUse;        // number of sats
double dGGAHDOP;                          // HDOP
double dGGAAltitude;                      // Altitude: mean-sea-level (geoid) meters
int dwGGACount;                           // number of processed datasets
int dwVTGCount;                           // number of processed datasets

int nGGAOldVSpeedSeconds;                 // used to calculate a speed vector
double dGGAOldVSpeedAlt;                  // (but better use $GPVTG dataset)
double dGGAVertSpeed;                     // vertical speed

double dVTGTrueHeading;                   // true heading
double dVTGMagneticHeading;               // magnetic heading
double dVTGSpeedInknots;                  // groundspeed in knots
double dVTGSpeedInKmh;                    // groundspeed in kmh


// ==============
// Prototypes
// ==============

extern void NMEAParserInit();
extern void ProcessGPGGA(unsigned char *pData);
extern void ProcessGPVTG(unsigned char *pData);
extern void Reset();
extern unsigned char GetField(unsigned char *pData, unsigned char *pField, int nFieldNum, int nMaxFieldLen);
extern unsigned char ProcessCommand(unsigned char *pCommand, unsigned char *pData);
extern void ProcessNMEA(unsigned char ucData);
extern unsigned char ParseBuffer(unsigned char *pBuff, int dwLen);

#endif // _NMEAPARSER_H_
