// ============================================
// NMEA Parser
// Headerfile
// ============================================

// $RCSfile: NMEAParser.h,v $ last changed on $Date: 2005-01-27 14:59:36 $ by $Author: rudolf $

// History:
//
// $Log: NMEAParser.h,v $
// Revision 1.1  2005-01-27 14:59:36  rudolf
// added files for GPS receiver connection
//
//

#ifndef _NMEAPARSER_H_
#define _NMEAPARSER_H_

#define TRUE (1)
#define FALSE (0)
enum NP_STATE {
				NP_STATE_SOM,		// Search for start of message
				NP_STATE_CMD,		// Get command
				NP_STATE_DATA,		// Get data
				NP_STATE_CHECKSU1,	// Get first checksum character
				NP_STATE_CHECKSU2,	// get second checksum character
};

#define NP_MAX_CMD_LEN			8		// maximum command length (NMEA address)
#define NP_MAX_DATA_LEN			256		// maximum data length
#define NP_MAX_CHAN				36		// maximum number of channels
#define NP_WAYPOINT_ID_LEN		32		// waypoint max string len

// ==============
// Globals
// ==============

enum NP_STATE nState;					// Parser Statemachine
unsigned char ucChecksum;				// Calculated NMEA sentence checksum
unsigned char ucReceivedChecksum;		// Received NMEA sentence checksum (if exists)
int wIndex;								// Index used for command and data
unsigned char pCommand[NP_MAX_CMD_LEN];	// NMEA command
unsigned char pData[NP_MAX_DATA_LEN];	// NMEA data

int dwCommandCount;						// number of NMEA commands received (processed or not processed)

// ==========================================
// GPGGA Data (Time/Position/Quality dataset
// ==========================================

unsigned char ucGGAHour;				// UTC Time
unsigned char ucGGAMinute;				//
unsigned char ucGGASecond;				//

double dGGALatitude;					// < 0 = South, > 0 = North
double dGGALongitude;					// < 0 = West, > 0 = East

unsigned char ucGGAGPSQuality;			// 0 = fix not available,
										// 1 = GPS sps mode,
										// 2 = Differential GPS, SPS mode, fix valid,
										// 3 = GPS PPS mode, fix valid

unsigned char ucGGANumOfSatsInUse;		// number of sats
double dGGAHDOP;						// HDOP
double dGGAAltitude;					// Altitude: mean-sea-level (geoid) meters
int dwGGACount;							// number of processed datasets

int nGGAOldVSpeedSeconds;				// used to calculate a speed vector
double dGGAOldVSpeedAlt;				// (but better use $GPVTG dataset)
double dGGAVertSpeed;					//

// ==============
// Prototypes
// ==============

void ProcessGPGGA(unsigned char *pData);
void Reset();
unsigned char GetField(unsigned char *pData, unsigned char *pField, int nFieldNum, int nMaxFieldLen);
unsigned char ProcessCommand(unsigned char *pCommand, unsigned char *pData);
void ProcessNMEA(unsigned char ucData);
unsigned char ParseBuffer(unsigned char *pBuff, int dwLen);

#endif // _NMEAPARSER_H_
