// ============================================
// NMEA Parser
// Implementation
// ============================================

// $RCSfile: NMEAParser.c,v $ last changed on $Date: 2006/09/04 13:05:50 $ by $Author: rudolf $

// History:
//
// $Log: NMEAParser.c,v $
// Revision 1.5  2006/09/04 13:05:50  rudolf
// fix for GCC 4.03
//
// Revision 1.4  2005/09/21 21:02:48  rudolf
// fixed Latitude and Longitude parsing, changed tty to S1
//
// Revision 1.3  2005/01/31 11:36:31  rudolf
// Added parsing of NMEA GPVTG datafield (groundspeed and heading)
//
// Revision 1.2  2005/01/27 18:16:12  rudolf
// changed NMEA parser to work with elekIO serv
//
// Revision 1.1  2005/01/27 14:59:36  rudolf
// added files for GPS receiver connection
//
//

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include "NMEAParser.h"

#define MAXFIELD	25		// maximum field length

extern volatile unsigned char ucDataReadyFlag;

// ==================
// Init NMEA Parser
// ==================

void NMEAParserInit()
{
	nState = NP_STATE_SOM;	// Reset Statemachine
	dwCommandCount = 0;
	Reset();
}

// ===============================
// Process serial input buffer
// ===============================

unsigned char ParseBuffer(unsigned char *pBuff, int iLen)
{

	int iIndex = 0;
	for(iIndex = 0; iIndex < iLen; iIndex++)
	{
		ProcessNMEA(pBuff[iIndex]);
	}
	return TRUE;
}

// ==================
// Process NMEA data
// ==================

void ProcessNMEA(unsigned char ucData)
{
	switch(nState)
	{
		// Search for start of message '$'
		case NP_STATE_SOM :
			if(ucData == '$')
			{
				ucChecksum = 0;			// reset checksum
				wIndex = 0;				// reset index
				nState = NP_STATE_CMD;
			}
		break;

		// Retrieve command (NMEA Address)
		case NP_STATE_CMD :
			if(ucData != ',' && ucData != '*')
			{
				pCommand[wIndex++] = ucData;
				ucChecksum ^= ucData;

				// Check for command overflow
				if(wIndex >= NP_MAX_CMD_LEN)
				{
					nState = NP_STATE_SOM;
				}
			}
			else
			{
				pCommand[wIndex] = '\0';	// terminate command
				ucChecksum ^= ucData;
				wIndex = 0;
				nState = NP_STATE_DATA;		// goto get data state
			}
		break;

		// Store data and check for end of sentence or checksum flag
		case NP_STATE_DATA :
			if(ucData == '*') // checksum flag?
			{
				pData[wIndex] = '\0';
				nState = NP_STATE_CHECKSU1;
			}
			else // no checksum flag, store data
			{
				//
				// Check for end of sentence with no checksum
				//
				if(ucData == '\r')
				{
					pData[wIndex] = '\0';
					ProcessCommand(pCommand, pData);
					nState = NP_STATE_SOM;
					return;
				}

				//
				// Store data and calculate checksum
				//
				ucChecksum ^= ucData;
				pData[wIndex] = ucData;
				if(++wIndex >= NP_MAX_DATA_LEN) // Check for buffer overflow
				{
					nState = NP_STATE_SOM;
				}
			}
		break;

		// first byte of checksum
		case NP_STATE_CHECKSU1 :
			if( (ucData - '0') <= 9)
			{
				ucReceivedChecksum = (ucData - '0') << 4;
			}
			else
			{
				ucReceivedChecksum = (ucData - 'A' + 10) << 4;
			}

			nState = NP_STATE_CHECKSU2;

		break;

		// second byte of checksum
		case NP_STATE_CHECKSU2 :
			if( (ucData - '0') <= 9)
			{
				ucReceivedChecksum |= (ucData - '0');
			}
			else
			{
				ucReceivedChecksum |= (ucData - 'A' + 10);
			}

			if(ucChecksum == ucReceivedChecksum)
			{
				ProcessCommand(pCommand, pData);
			}

			nState = NP_STATE_SOM;
		break;

		default : nState = NP_STATE_SOM;
	}
}

//=======================================================================
// Process NMEA sentence - Use the NMEA address (*pCommand) and call the
// appropriate sentense data processor.
//=======================================================================

unsigned char ProcessCommand(unsigned char *pCommand, unsigned char *pData)
{
	//
	// GPGGA
	//
	if( strcmp((char *)pCommand, "GPGGA") == 0)
	{
		ProcessGPGGA(pData);
	}

	//
	// GPVTG
	//
	if( strcmp((char *)pCommand, "GPVTG") == 0)
	{
		ProcessGPVTG(pData);
	}

	dwCommandCount++;
	return TRUE;
}

//===========================================================================
// Name:		GetField
//
// Description:	This function will get the specified field in a NMEA string.
//
// Entry:		unsigned char *pData -		Pointer to NMEA string
//				unsigned char *pField -		pointer to returned field
//				int nfieldNum -		Field offset to get
//				int nMaxFieldLen -	Maximum of unsigned chars pFiled can handle
//===========================================================================

unsigned char GetField(unsigned char *pData, unsigned char *pField, int nFieldNum, int nMaxFieldLen)
{
	//
	// Validate params
	//
	if(pData == NULL || pField == NULL || nMaxFieldLen <= 0)
	{
		return FALSE;
	}

	//
	// Go to the beginning of the selected field
	//
	int i = 0;
	int nField = 0;
	while(nField != nFieldNum && pData[i])
	{
		if(pData[i] == ',')
		{
			nField++;
		}

		i++;

		if(pData[i] == 0)
		{
			pField[0] = '\0';
			return FALSE;
		}
	}

	if(pData[i] == ',' || pData[i] == '*')
	{
		pField[0] = '\0';
		return FALSE;
	}

	//
	// copy field from pData to Field
	//
	int i2 = 0;
	while(pData[i] != ',' && pData[i] != '*' && pData[i])
	{
		pField[i2] = pData[i];
		i2++; i++;

		//
		// check if field is too big to fit on passed parameter. If it is,
		// crop returned field to its max length.
		//
		if(i2 >= nMaxFieldLen)
		{
			i2 = nMaxFieldLen-1;
			break;
		}
	}
	pField[i2] = '\0';

	return TRUE;
}

//===========================================================================
// Reset: Reset all NMEA data to start-up default values.
//===========================================================================

void Reset()
{
	int i;

	//
	// GPGGA Data
	//
	ucGGAHour = 0;				//
	ucGGAMinute = 0;			//
	ucGGASecond = 0;			//
	dGGALatitude = 0.0;			// < 0 = South, > 0 = North
	dGGALongitude = 0.0;		// < 0 = West, > 0 = East
	ucGGAGPSQuality = 0;		// 0 = fix not available,
								// 1 = GPS sps mode,
								// 2 = Differential GPS, SPS mode, fix valid,
								// 3 = GPS PPS mode, fix valid
	ucGGANumOfSatsInUse = 0;	//
	dGGAHDOP = 0.0;				//
	dGGAAltitude = 0.0;			// Altitude: mean-sea-level (geoid) meters
	dwGGACount = 0;				//
	nGGAOldVSpeedSeconds = 0;	//
	dGGAOldVSpeedAlt = 0.0;		//
	dGGAVertSpeed = 0.0;		//
}

//===========================================================================
// ProcessGPGGA: extract time, date, postition and quality from GPGGA data
//===========================================================================

void ProcessGPGGA(unsigned char *pData)
{
	unsigned char pField[MAXFIELD];
	unsigned char pBuff[10];

	//
	// Time
	//
	if(GetField(pData, pField, 0, MAXFIELD))
	{
		// Hour
		pBuff[0] = pField[0];
		pBuff[1] = pField[1];
		pBuff[2] = '\0';
		ucGGAHour = atoi((char*)pBuff);

		// minute
		pBuff[0] = pField[2];
		pBuff[1] = pField[3];
		pBuff[2] = '\0';
		ucGGAMinute = atoi((char*)pBuff);

		// Second
		pBuff[0] = pField[4];
		pBuff[1] = pField[5];
		pBuff[2] = '\0';
		ucGGASecond = atoi((char*)pBuff);
	}

	//
	// Latitude
	//
	if(GetField(pData, pField, 1, MAXFIELD))
	{
		dGGALatitude = atof((char*)pField);
	}
	if(GetField(pData, pField, 2, MAXFIELD))
	{
		if(pField[0] == 'S')
		{
			dGGALatitude = -dGGALatitude;
		}
	}

	//
	// Longitude
	//
	if(GetField(pData, pField, 3, MAXFIELD))
	{
		dGGALongitude = atof((char *)pField);
	}
	if(GetField(pData, pField, 4, MAXFIELD))
	{
		if(pField[0] == 'W')
		{
			dGGALongitude = -dGGALongitude;
		}
	}

	//
	// GPS quality
	//
	if(GetField(pData, pField, 5, MAXFIELD))
	{
		ucGGAGPSQuality = pField[0] - '0';
	}

	//
	// Satellites in use
	//
	if(GetField(pData, pField, 6, MAXFIELD))
	{
		pBuff[0] = pField[0];
		pBuff[1] = pField[1];
		pBuff[2] = '\0';
		ucGGANumOfSatsInUse = atoi((char*)pBuff);
	}

	//
	// HDOP
	//
	if(GetField(pData, pField, 7, MAXFIELD))
	{
		dGGAHDOP = atof((char *)pField);
	}

	//
	// Altitude
	//
	if(GetField(pData, pField, 8, MAXFIELD))
	{
		dGGAAltitude = atof((char *)pField);
	}

	//
	// Durive vertical speed (bonus)
	//
	int nSeconds = (int)ucGGAMinute * 60 + (int)ucGGASecond;
	if(nSeconds > nGGAOldVSpeedSeconds)
	{
		double dDiff = (double)(nGGAOldVSpeedSeconds-nSeconds);
		double dVal = dDiff/60.0;
		if(dVal != 0.0)
		{
			dGGAVertSpeed = (dGGAOldVSpeedAlt - dGGAAltitude) / dVal;
		}
	}
	dGGAOldVSpeedAlt = dGGAAltitude;
	nGGAOldVSpeedSeconds = nSeconds;

	if(ucGGAGPSQuality > 0)		// position fixes are valid
		ucDataReadyFlag = TRUE;
	dwGGACount++;
}


//===========================================================================
// ProcessGPVTG: extract speed and heading
//===========================================================================

void ProcessGPVTG(unsigned char *pData)
{
   unsigned char pField[MAXFIELD];
   unsigned char pBuff[10];
   if(GetField(pData, pField, 0, MAXFIELD))
   {
      // True Heading

      pBuff[0] = pField[0];
      pBuff[1] = pField[1];
      pBuff[2] = pField[2];
      pBuff[3] = pField[3];
      pBuff[4] = pField[4];
      pBuff[5] = '\0';

      dVTGTrueHeading = atof((char*)pBuff);
   }

   if(GetField(pData, pField, 2, MAXFIELD))
   {
      // Magnetic Heading

      pBuff[0] = pField[0];
      pBuff[1] = pField[1];
      pBuff[2] = pField[2];
      pBuff[3] = pField[3];
      pBuff[4] = pField[4];
      pBuff[5] = '\0';

      dVTGMagneticHeading = atof((char*)pBuff);
   }

   if(GetField(pData, pField, 4, MAXFIELD))
   {
      // Groundspeed in knots

      pBuff[0] = pField[0];
      pBuff[1] = pField[1];
      pBuff[2] = pField[2];
      pBuff[3] = pField[3];
      pBuff[4] = pField[4];
      pBuff[5] = '\0';

      dVTGSpeedInknots = atof((char*)pBuff);
   }

   if(GetField(pData, pField, 6, MAXFIELD))
   {
      // Groundspeed in kmh

      pBuff[0] = pField[0];
      pBuff[1] = pField[1];
      pBuff[2] = pField[2];
      pBuff[3] = pField[3];
      pBuff[4] = pField[4];
      pBuff[5] = '\0';

      dVTGSpeedInKmh = atof((char*)pBuff);
   }

   dwVTGCount++;
}
