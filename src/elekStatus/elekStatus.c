/*
 * $RCSfile: elekStatus.c,v $ last changed on $Date: 2007-10-24 15:14:57 $ by $Author: rudolf $
 *
 * $Log: elekStatus.c,v $
 * Revision 1.39  2007-10-24 15:14:57  rudolf
 * added debug output for etalon mode
 *
 * Revision 1.38  2007/10/24 14:23:02  rudolf
 * integration of spectra data recording every 10 sec and 100 sec live data at 1Hz
 *
 * Revision 1.37  2007/08/07 12:35:09  rudolf
 * preparations for recording spectral data as well
 *
 * Revision 1.36  2007-03-11 17:06:55  rudolf
 * fixed typo
 *
 * Revision 1.35  2007-03-11 17:05:14  rudolf
 * added last fields missing
 *
 * Revision 1.34  2007-03-11 16:02:52  rudolf
 * added more fields from Ship and Meteo in dumping
 *
 * Revision 1.33  2007-03-11 15:30:37  rudolf
 * added creation of running status file also for calibration and aux data
 *
 * Revision 1.32  2007-03-11 11:17:15  rudolf
 * use timestamp from structure, not local
 *
 * Revision 1.31  2007-03-09 16:22:35  rudolf
 * fixed bug using wrong structure for auxdata for filename and saving
 *
 * Revision 1.30  2007-03-09 13:09:21  rudolf
 * added new port for aux data, recording of aux data and dumping of aux data
 *
 * Revision 1.29  2007-02-21 22:57:57  rudolf
 * added new group for LICOR in status printing
 *
 * Revision 1.28  2007-02-21 21:49:52  rudolf
 * added saving of calibration data when available in seperate .CAL file
 *
 * Revision 1.27  2007-02-21 20:21:27  rudolf
 * added seperate file for calib data
 *
 * Revision 1.26  2006-10-06 10:19:45  rudolf
 * added butterfly structure as new group
 *
 * Revision 1.25  2006/09/04 10:19:46  rudolf
 * fixed compiler warning for GCC 4.03
 *
 * Revision 1.24  2006/08/17 15:58:56  rudolf
 * fixed GPS typo
 *
 * Revision 1.23  2005/12/14 13:53:28  rudolf
 * GABRIEL campaign changes
 *
 * Revision 1.22  2005/07/08 06:14:43  rudolf
 * replaced localtime by gmtime, added 1 to yday in filename generation, display ditherstepwidth in etalonstatus
 *
 * Revision 1.21  2005/06/30 21:52:09  rudolf
 * added separator in mask display
 *
 * Revision 1.20  2005/06/29 15:11:38  harder
 * added mask disp
 *
 * Revision 1.19  2005/06/29 12:46:18  rudolf
 * preparation to display mask in status
 *
 * Revision 1.18  2005/06/27 09:16:29  rudolf
 * cosmetic in help (HH)
 *
 * Revision 1.17  2005/06/26 17:10:15  rudolf
 * reduced ring length to 500 (HH)
 *
 * Revision 1.16  2005/06/26 16:50:16  rudolf
 * added Valve display (HH)
 *
 * Revision 1.15  2005/06/25 19:46:17  rudolf
 * fixed typo
 *
 * Revision 1.14  2005/06/25 19:42:40  rudolf
 * added keys for [S]how all and [R] show none, fixed typo in Temp output
 *
 * Revision 1.13  2005/06/25 19:20:18  rudolf
 * removed debug output, added small helpscreen invokeable by pressing [H]
 *
 * Revision 1.12  2005/06/25 18:18:46  rudolf
 * fixed keyboard handling
 *
 * Revision 1.11  2005/06/25 14:49:46  rudolf
 * added keyboard scan in source
 *
 * Revision 1.10  2005/06/24 17:17:04  rudolf
 * more work on user-selectable groups of data
 *
 * Revision 1.9  2005/05/22 19:09:45  rudolf
 * fixes for new elekStatus structure
 *
 * Revision 1.8  2005/04/22 12:38:48  rudolf
 * fixed handling if instrument data could not be written due to lack of diskspace etc.
 *
 * Revision 1.7  2005/04/22 11:30:18  rudolf
 * don't return from WriteElekStatus() with error if no datafile could be created, continue with status.bin
 *
 * Revision 1.6  2005/04/22 10:45:04  rudolf
 * changed Revision Info to be more verbose
 *
 * Revision 1.5  2005/04/22 10:36:24  rudolf
 * changed path for chPMT.txt to /lift /ramdisk
 *
 * Revision 1.4  2005/04/21 16:29:38  rudolf
 * Added Revision in startup output
 *
 * Revision 1.3  2005/04/21 13:53:31  rudolf
 * more work on conditional compile
 *
 * Revision 1.2  2005/01/31 10:06:03  rudolf
 * Added printing of GPS data, a 200 character wide display would be really nice.... :-/
 *
 *
 */

#define VERSION 0.8
#define POSIX_SOURCE 1
#define USE_POSIX

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#ifdef RUNONPC
# include <asm/msr.h>
#endif

#include <signal.h>
#include <errno.h>
#include <termios.h>

#include "elekStatus.h"
#include "../include/elekGeneral.h"
#include "../include/elekIO.h"
#include "../include/elekIOPorts.h"

#define STATUSFILE_RING_LEN 250

#define DEBUGLEVEL 1

#undef DEBUG_SPECTROMETER
#define DEBUG_SPECT_ON_OFFLINE

// define which groups are shown on startup of elekStatus
unsigned int uiGroupFlags = GROUP_DATASETDATA | GROUP_ADCDATA | GROUP_TIMEDATA;

// number of status counts total
long StatusCount;

enum InPortListEnum
{
   // this list has to be coherent with MessageInPortList
   ELEK_STATUS_REQ_IN,
     ELEK_ELEKIO_IN,
     CALIB_IN,
     AUX_IN,
     SPECTRA_IN,
     MAX_MESSAGE_INPORTS
};

enum OutPortListEnum
{
   // this list has to be coherent with MessageOutPortList
   ELEK_STATUS_OUT,
     ELEK_DEBUG_OUT,
     ELEK_CLIENT_OUT,
     MAX_MESSAGE_OUTPORTS
};

static struct MessagePortType MessageInPortList[MAX_MESSAGE_INPORTS]=
{
   // order in list defines sequence of polling
  /* Name, PortNo, ReversePortNo, fdSocket, MaxMessages, Direction */
     {"StatusReq",    UDP_ELEK_STATUS_REQ_INPORT,      UDP_ELEK_STATUS_REQ_OUTPORT, IP_LOCALHOST, -1, 1,  UDP_IN_PORT},
     {"ElekIOIn"     ,UDP_ELEK_STATUS_STATUS_OUTPORT,  -1                         , IP_LOCALHOST, -1, 1,  UDP_IN_PORT}, // status inport from elekIOServ (instrument data)
     {"CalibIn "     ,UDP_CALIB_STATUS_STATUS_OUTPORT, -1                         , IP_LOCALHOST, -1, 1,  UDP_IN_PORT}, // status inport from elekIOServ (calibration data)
     {"AuxIn"        ,UDP_ELEK_AUX_INPORT,             -1                         , IP_LOCALHOST, -1, 1,  UDP_IN_PORT}, // status inport from elekIOServ (aux/meteo data)
     {"SpectraIn"    ,UDP_ELEK_SPECTRA_INPORT,         -1                         , IP_LOCALHOST, -1, 1,  UDP_IN_PORT}  // status inport from spectrometerServer (spectral data)
   // uses same portnumber as elekIOaux uses to send
};                                                                                                                   // the data to elekIOServ to simplify direct connection
// elekIOaux->elekStatus if needed
//
static struct MessagePortType MessageOutPortList[MAX_MESSAGE_OUTPORTS]=
{
   // order in list defines sequence of polling
  /* Name, PortNo, ReversePortNo, fdSocket, MaxMessages, Direction */
     {"StatusReq",   UDP_ELEK_STATUS_REQ_OUTPORT, UDP_ELEK_STATUS_REQ_INPORT, IP_ELEK_SERVER,  -1, 0, UDP_OUT_PORT},
     {"DebugPort",   UDP_ELEK_DEBUG_OUTPORT,                              -1, IP_DEBUG_CLIENT, -1, 0, UDP_OUT_PORT},
     {"StatusClient",UDP_ELEK_STATUS_STATUS_OUTPORT,                      -1, IP_DEBUG_CLIENT, -1, 0, UDP_OUT_PORT} // status inport from elekIOServ
};

static long LastElekStatusNumber;
static long LastCalibStatusNumber;
static long LastAuxStatusNumber;
static long LastSpectraStatusNumber;

void PrintAuxStatus(struct auxStatusType *ptrAuxStatus, int PacketSize)
{

   int i;
   struct tm tmZeit;
   time_t    Seconds;

   // ***************** DATASET DATA (number of dataset etc. **************
   if(uiGroupFlags & GROUP_AUXDATA)
     {
	printf("Time(aux)");
	Seconds=ptrAuxStatus->TimeOfDayAux.tv_sec;
	gmtime_r(&Seconds,&tmZeit);

	printf("JD:%03d %02d.%02d %02d:%02d:%02d.%03d :",tmZeit.tm_yday+1,tmZeit.tm_mon+1,tmZeit.tm_mday,
	       tmZeit.tm_hour, tmZeit.tm_min, tmZeit.tm_sec, ptrAuxStatus->TimeOfDayAux.tv_usec/1000);

	printf("WS(MeteoBox): %5.2f WDir: %03d Temp: %+5.2f RH: %5.2f GS: %5.3f ",\
       ptrAuxStatus->MeteoBox.dWindSpeed,\
       ptrAuxStatus->MeteoBox.uiWindDirection,\
       ptrAuxStatus->MeteoBox.dAirTemp,\
       ptrAuxStatus->MeteoBox.dRelHum,\
       ptrAuxStatus->MeteoBox.dGasSensorVoltage);

	printf("PLicor: %05d ",\
       ptrAuxStatus->LicorPressure.uiAmbientPressure);

	printf("(ShGPS)UTC %02d:%02d:%02d Date:%02d.%02d.%04d Lat: %+9.4f Lon: %+9.4f COG: %5.2f Speed: %5.2f ",\
       ptrAuxStatus->ShipGPS.ucUTCHours,\
       ptrAuxStatus->ShipGPS.ucUTCMins,\
       ptrAuxStatus->ShipGPS.ucUTCSeconds,\

	       ptrAuxStatus->ShipGPS.ucUTCDay,\
       ptrAuxStatus->ShipGPS.ucUTCMonth,\
       ptrAuxStatus->ShipGPS.uiUTCYear,\
       ptrAuxStatus->ShipGPS.dLatitude,\
       ptrAuxStatus->ShipGPS.dLongitude,\
       ptrAuxStatus->ShipGPS.dCourseOverGround,\
       ptrAuxStatus->ShipGPS.dGroundSpeed);

	printf("(ShMeteo)WDir: %5.2f WS: %5.2f ",\
       ptrAuxStatus->ShipMeteo.dWindDirection,\
       ptrAuxStatus->ShipMeteo.dWindSpeed);

	printf("(ShGyro)Hdg: %5.2f ",\
       ptrAuxStatus->ShipGyro.dDirection);

	printf("(ShSonar)Freq: %5.2f Depth: %6.2f ",\
       ptrAuxStatus->ShipSonar.dFrequency,\
       ptrAuxStatus->ShipSonar.dWaterDepth);

	printf("(ShWater)Salt: %5.2f Temp: %+5.2f",\
       ptrAuxStatus->ShipWater.dSalinity,\
       ptrAuxStatus->ShipWater.dWaterTemp);

	printf("\n\r");
     };

};

void PrintCalibStatus(struct calibStatusType *ptrCalibStatus, int PacketSize)
{

   int i;
   struct tm tmZeit;
   time_t    Seconds;

   // ***************** DATASET DATA (number of dataset etc. **************
   if(uiGroupFlags & GROUP_CALIBDATA)
     {
	printf("Time(cal),faked:");
	Seconds=ptrCalibStatus->TimeOfDayCalib.tv_sec;
	gmtime_r(&Seconds,&tmZeit);

	printf("%d %02d.%02d %02d:%02d:%02d.%03d :",tmZeit.tm_yday+1,tmZeit.tm_mon+1,tmZeit.tm_mday,
	       tmZeit.tm_hour, tmZeit.tm_min, tmZeit.tm_sec, ptrCalibStatus->TimeOfDayCalib.tv_usec/1000);

	printf("H2O_A: %06.4f H2O_B: %06.4f H2O_DELTA: %06.4f",\
       (double)(ptrCalibStatus->LicorCalib.H2OA)/1000.0f,\
       (double)(ptrCalibStatus->LicorCalib.H2OB)/1000.0f,\
       (double)(ptrCalibStatus->LicorCalib.H2OD)/1000.0f);
	printf("\n\r");
     };

};

void PrintElekStatus(struct elekStatusType *ptrElekStatus, int PacketSize)
{

   int i;
   struct tm tmZeit;
   time_t    Seconds;
   int Channel;
   int Card;

   // ***************** DATASET DATA (number of dataset etc. **************
   if(uiGroupFlags & GROUP_DATASETDATA)
     printf("StatusCount %ld nByte %d ", StatusCount,PacketSize);

   // ***************** TIME DATA **************
   if(uiGroupFlags & GROUP_TIMEDATA)
     {
	printf("Time(M):");
	Seconds=ptrElekStatus->TimeOfDayMaster.tv_sec;
	gmtime_r(&Seconds,&tmZeit);

	printf("%d %02d.%02d %02d:%02d:%02d.%03d :",tmZeit.tm_yday+1,tmZeit.tm_mon+1,tmZeit.tm_mday,
	       tmZeit.tm_hour, tmZeit.tm_min, tmZeit.tm_sec, ptrElekStatus->TimeOfDayMaster.tv_usec/1000);

	printf("Time(S):");
	Seconds=ptrElekStatus->TimeOfDaySlave.tv_sec;
	gmtime_r(&Seconds,&tmZeit);

	printf("%02d.%02d %02d:%02d:%02d.%03d :",tmZeit.tm_mon+1,tmZeit.tm_mday,
	       tmZeit.tm_hour, tmZeit.tm_min, tmZeit.tm_sec, ptrElekStatus->TimeOfDaySlave.tv_usec/1000);
     };

   // ***************** ETALON DATA **************
   //
   if(uiGroupFlags & GROUP_ETALONDATA)
     {
	printf("Etalon:%ld(%d %d)/%ld %ld %d %d %4x",ptrElekStatus->EtalonData.Current.Position,
	       ptrElekStatus->EtalonData.Current.PositionWord.High,
	       ptrElekStatus->EtalonData.Current.PositionWord.Low,
	       ptrElekStatus->EtalonData.Encoder.Position,
	       ptrElekStatus->EtalonData.Index.Position,
	       ptrElekStatus->EtalonData.CurSpeed,
	       ptrElekStatus->EtalonData.DitherStepWidth,

	       ptrElekStatus->EtalonData.Status);
     };

   // ***************** COUNTER CARDS DATA **************
   //
   if(uiGroupFlags & GROUP_CCDATA)
     {
	// *****************
	// MASTER CounterCard
	// *****************
	printf("CC(M) ADC:");
	// Counter Card ADC Channel
	for (i=0; i<ADC_CHANNEL_COUNTER_CARD; i++)
	  {
	     printf("%4x ",ptrElekStatus->CounterCardMaster.ADCData[i]);
	  }

	printf("MasDel:%4x ",ptrElekStatus->CounterCardMaster.MasterDelay);
	printf("ShDel:");

	for(i=0; i<MAX_COUNTER_CHANNEL;i++)
	  printf("%4x ",ptrElekStatus->CounterCardMaster.Channel[i].ShiftDelay);

	printf("GD/GW");

	for(i=0; i<MAX_COUNTER_GATE;i++)
	  {
	     printf("%4x/%4x ",ptrElekStatus->CounterCardMaster.Channel[i].GateDelay,
		    ptrElekStatus->CounterCardMaster.Channel[i].GateWidth);
	  }

	printf("Cnts:");

	for(i=0; i<MAX_COUNTER_CHANNEL;i++)
	  {
	     printf("%d ",ptrElekStatus->CounterCardMaster.Channel[i].Counts);
	  }
	// *****************
	// SLAVE CounterCard
	// *****************
	//
	printf("CC(S) ADC:");
	// Counter Card ADC Channel
	for (i=0; i<ADC_CHANNEL_COUNTER_CARD; i++)
	  {
	     printf("%4x ",ptrElekStatus->CounterCardSlave.ADCData[i]);
	  }

	printf("MasDel:%4x ",ptrElekStatus->CounterCardSlave.MasterDelay);
	printf("ShDel:");

	for(i=0; i<MAX_COUNTER_CHANNEL;i++)
	  printf("%4x ",ptrElekStatus->CounterCardSlave.Channel[i].ShiftDelay);

	printf("GD/GW");

	for(i=0; i<MAX_COUNTER_GATE;i++)
	  {
	     printf("%4x/%4x ",ptrElekStatus->CounterCardSlave.Channel[i].GateDelay,
		    ptrElekStatus->CounterCardSlave.Channel[i].GateWidth);
	  }

	printf("Cnts:");

	for(i=0; i<MAX_COUNTER_CHANNEL;i++)
	  {
	     printf("%d ",ptrElekStatus->CounterCardSlave.Channel[i].Counts);
	  }
     }

   // ***************** COUNTER CARDS MASK **************
   //
   if(uiGroupFlags & GROUP_CCMASK)
     {
	// *****************
	// MASTER CounterCard
	// *****************
	//
	printf("Mask");
	printf("(M):");

	for (Channel=0; Channel<MAX_COUNTER_CHANNEL; Channel++)
	  {

	     printf("c%d:", Channel);
	     for (i=0;i<10; i++)
	       {
		  printf("%04x.",ptrElekStatus->CounterCardMaster.Channel[Channel].Mask[i]);
	       }
	     // for i
	  }
	// for Channel
	//
	//
	// SLAVE
	//
	printf("(S):");

	for (Channel=0; Channel<MAX_COUNTER_CHANNEL; Channel++)
	  {

	     printf("c%d:", Channel);
	     for (i=0;i<10; i++)
	       {
		  printf("%04x.",ptrElekStatus->CounterCardSlave.Channel[Channel].Mask[i]);
	       }
	     // for i
	  }
	// for Channel
	//
	//
	//
	//
     }
   // uiGroupFlags & GROUP_CCMASK
   //
   //
   //
   //
   // ***************** ADC CARDS DATA **************
   //
   if(uiGroupFlags & GROUP_ADCDATA)
     {
	// MASTER
	// normal 16bit ADC Card
	//
	printf("Ana(M):");
	for (Card=0; Card<MAX_ADC_CARD_LIFT; Card ++)
	  {
	     printf("Card#%d:",Card);
	     for (Channel=0;Channel<MAX_ADC_CHANNEL_PER_CARD; Channel++)
	       {
		  printf("%05d ",ptrElekStatus->ADCCardMaster[Card].ADCChannelData[Channel].ADCData);
	       }
	     /* for Channel */
	  }
	/* for Card */

	// SLAVE
	// normal 16bit ADC Card
	printf("Ana(S):");
	for (Card=0; Card<MAX_ADC_CARD_WP; Card ++)
	  {
	     printf("Card#%d:",Card);
	     for (Channel=0;Channel<MAX_ADC_CHANNEL_PER_CARD; Channel++)
	       {
		  printf("%05d ",ptrElekStatus->ADCCardSlave[Card].ADCChannelData[Channel].ADCData);
	       }
	     /* for Channel */
	  }
	/* for Card */

     };

   // ***************** TEMPERATURE DATA **************
   //
   if(uiGroupFlags & GROUP_TEMPDATA)
     {
	// MASTER
	// temperature Sensors
	printf("TS(M):");
	printf("SRS:%d ",ptrElekStatus->TempSensCardMaster[0].NumSensor);
	printf("CRC:%d ",ptrElekStatus->TempSensCardMaster[0].NumErrCRC);
	printf("NR:%d ",ptrElekStatus->TempSensCardMaster[0].NumErrNoResponse);
	printf("MIS:%d ",ptrElekStatus->TempSensCardMaster[0].NumMissed);
	//    printf("%x;",ptrElekStatus->TempSensCard[0].TempSensor[0].Word.WordTemp);
	//
	// it only makes sense to display the sensors actually detected by the hardware
	if(ptrElekStatus->TempSensCardMaster[0].NumSensor < MAX_TEMP_SENSOR) // sanity check
	  {
	     for (i=0;i<ptrElekStatus->TempSensCardMaster[0].NumSensor;i++)
	       {
		  printf("[%02x:%02x:%02x] %03.02f,",
			 ptrElekStatus->TempSensCardMaster[0].TempSensor[i].Field.aROMCode[0],
			 ptrElekStatus->TempSensCardMaster[0].TempSensor[i].Field.aROMCode[1],
			 ptrElekStatus->TempSensCardMaster[0].TempSensor[i].Field.aROMCode[2],
			 (float)(ptrElekStatus->TempSensCardMaster[0].TempSensor[i].Field.TempMain)+
			 (float)(ptrElekStatus->TempSensCardMaster[0].TempSensor[i].Field.TempFrac)/16
			 );
		  //	     ptrElekStatus->TempSensCard[0].TempSensor[i].Word.WordTemp);
	       }
	  }
	// SLAVE
	// temperature Sensors
	printf("TS(S):");
	printf("SRS:%d ",ptrElekStatus->TempSensCardSlave[0].NumSensor);
	printf("CRC:%d ",ptrElekStatus->TempSensCardSlave[0].NumErrCRC);
	printf("NR:%d ",ptrElekStatus->TempSensCardSlave[0].NumErrNoResponse);
	printf("MIS:%d ",ptrElekStatus->TempSensCardSlave[0].NumMissed);
	//    printf("%x;",ptrElekStatus->TempSensCard[0].TempSensor[0].Word.WordTemp);
	//
	// it only makes sense to display the sensors actually detected by the hardware
	if(ptrElekStatus->TempSensCardSlave[0].NumSensor < MAX_TEMP_SENSOR) // sanity check
	  {
	     for (i=0;i<ptrElekStatus->TempSensCardSlave[0].NumSensor;i++)
	       {
		  printf("[%02x:%02x:%02x] %03.02f,",
			 ptrElekStatus->TempSensCardSlave[0].TempSensor[i].Field.aROMCode[0],
			 ptrElekStatus->TempSensCardSlave[0].TempSensor[i].Field.aROMCode[1],
			 ptrElekStatus->TempSensCardSlave[0].TempSensor[i].Field.aROMCode[2],
			 (float)(ptrElekStatus->TempSensCardSlave[0].TempSensor[i].Field.TempMain)+
			 (float)(ptrElekStatus->TempSensCardSlave[0].TempSensor[i].Field.TempFrac)/16
			 );
		  //	     ptrElekStatus->TempSensCard[0].TempSensor[i].Word.WordTemp);
	       }
	  }
     }

   // ***************** GPS DATA **************
   //
   if(uiGroupFlags & GROUP_GPSDATA)
     {
	// MASTER
	// GPS Data
	//
	printf("GPS(M) T:");
	printf("%02d:",ptrElekStatus->GPSDataMaster.ucUTCHours);   /* binary, not BCD coded (!) 0 - 23 decimal*/
	printf("%02d:",ptrElekStatus->GPSDataMaster.ucUTCMins);    /* binary, 0-59 decimal */
	printf("%02d ",ptrElekStatus->GPSDataMaster.ucUTCSeconds); /* binary 0-59 decimal */

	printf("LON:%f ",ptrElekStatus->GPSDataMaster.dLongitude);     /* "Laengengrad" I always mix it up..
									signed notation,
									negative values mean "W - west of Greenwich"
									positive values mean "E - east of Greenwich" */

	printf("LAT:%f ",ptrElekStatus->GPSDataMaster.dLatitude);      /* "Breitengrad" I always mix it up...
									signed notation,
									negative values mean "S - south of the equator
									positive values mean "N - north of the equator */

	printf("DOP:%f ",ptrElekStatus->GPSDataMaster.fHDOP);          /* Horizontal Dillution Of Precision, whatever it means....*/

	printf("Sat:%d ",ptrElekStatus->GPSDataMaster.ucNumberOfSatellites); /* number of satellites seen by the GPS receiver */
	printf("Val:%d ",ptrElekStatus->GPSDataMaster.ucLastValidData);      /* number of data aquisitions (5Hz) with no valid GPS data
									      will stick at 255 if no data received for a long period */

	printf("SPD:%d ",ptrElekStatus->GPSDataMaster.uiGroundSpeed);  /* speed in cm/s above ground */
	printf("HDG:%d ",ptrElekStatus->GPSDataMaster.uiHeading);      /* 10 times heading in degrees e.g. 2700 decimal = 270,0 Degress = west */

	// SLAVE
	// GPS Data
	//
	printf("GPS(S) T:");
	printf("%02d:",ptrElekStatus->GPSDataSlave.ucUTCHours);   /* binary, not BCD coded (!) 0 - 23 decimal*/
	printf("%02d:",ptrElekStatus->GPSDataSlave.ucUTCMins);    /* binary, 0-59 decimal */
	printf("%02d ",ptrElekStatus->GPSDataSlave.ucUTCSeconds); /* binary 0-59 decimal */

	printf("LON:%f ",ptrElekStatus->GPSDataSlave.dLongitude);     /* "Laengengrad" I always mix it up..
								       signed notation,
								       negative values mean "W - west of Greenwich"
								       positive values mean "E - east of Greenwich" */

	printf("LAT:%f ",ptrElekStatus->GPSDataSlave.dLatitude);      /* "Breitengrad" I always mix it up...
								       signed notation,
								       negative values mean "S - south of the equator
								       positive values mean "N - north of the equator */

	printf("DOP:%f ",ptrElekStatus->GPSDataSlave.fHDOP);          /* Horizontal Dillution Of Precision, whatever it means....*/

	printf("Sat:%d ",ptrElekStatus->GPSDataSlave.ucNumberOfSatellites); /* number of satellites seen by the GPS receiver */
	printf("Val:%d ",ptrElekStatus->GPSDataSlave.ucLastValidData);      /* number of data aquisitions (5Hz) with no valid GPS data
									     will stick at 255 if no data received for a long period */

	printf("SPD:%d ",ptrElekStatus->GPSDataSlave.uiGroundSpeed);  /* speed in cm/s above ground */
	printf("HDG:%d ",ptrElekStatus->GPSDataSlave.uiHeading);      /* 10 times heading in degrees e.g. 2700 decimal = 270,0 Degress = west */

     };

   // ***************** VALVE DATA **************
   //
   if (uiGroupFlags & GROUP_VALVEDATA)
     {

	// MASTER
	printf("Val(M):");
	for (Card=0; Card<MAX_VALVE_CARD_LIFT; Card++)
	  {
	     printf("%4x/%4x ",ptrElekStatus->ValveCardMaster[Card].ValveVolt,
		    ptrElekStatus->ValveCardMaster[Card].Valve);
	  }
	// for Card
	//
	printf("Val(S):");
	for (Card=0; Card<MAX_VALVE_CARD_WP; Card++)
	  {
	     printf("%4x/%4x ",ptrElekStatus->ValveCardSlave[Card].ValveVolt,
		    ptrElekStatus->ValveCardSlave[Card].Valve);
	  }
	// for Card
	//
     }
   // if GROUP_VALVEDATA
   //
   // ***************** BUTTERFLY DATA **************
   //
   if (uiGroupFlags & GROUP_BUTTERFLYDATA)
     {

	printf("BFV(S):");
	printf("VAL: %01d CP:%05d TPR:%05d TPS:%05d MCW:0x%04X CPU:0x%02X",
	       ptrElekStatus->ButterflySlave.PositionValid,
	       ptrElekStatus->ButterflySlave.CurrentPosition,
	       ptrElekStatus->ButterflySlave.TargetPositionGot,
	       ptrElekStatus->ButterflySlave.TargetPositionSet,
	       ptrElekStatus->ButterflySlave.MotorStatus.ButterflyStatusWord,
	       ptrElekStatus->ButterflySlave.CPUStatus.ButterflyCPUWord);
     }
   // if GROUP_BUTTERFLYDATA
   //
   //
   //
   //
   // CounterCard ShiftDelays
   //    for(i=0; i<MAX_COUNTER_CHANNEL;i++)
   //	ptrElekStatus->CounterCard.Channel[i].ShiftDelay=elkReadData(ELK_COUNTER_DELAY_SHIFT+2*i);
   //    ptrElekStatus->CounterCard.MasterDelay=elkReadData(ELK_COUNTER_DELAY_SHIFT+6);  // MasterDelay
   //
   // CounterCard GateDelays
   //    for(i=0; i<MAX_COUNTER_GATE;i++) {
   //	ptrElekStatus->CounterCard.Channel[i].GateDelay=elkReadData(ELK_COUNTER_DELAY_GATE+4*i);
   //	ptrElekStatus->CounterCard.Channel[i].GateWidth=elkReadData(ELK_COUNTER_DELAY_GATE+4*i+2);
   //    }
   printf("\n\r");
   //	refresh();
}
/*PrintElekStatus*/

int WriteElekStatus(char *PathToRamDisk, char *FileName, struct elekStatusType *ptrElekStatus)
{

   extern struct MessagePortType MessageOutPortList[];
   extern struct MessagePortType MessageInPortList[];

   extern long LastElekStatusNumber;

   FILE *fp;
   int i;
   int Channel;
   int Card;
   int ret;
   long len;
   int nelements;
   struct tm tmZeit;
   time_t    Seconds;
   char buf[GENERIC_BUF_LEN];

   Seconds=ptrElekStatus->TimeOfDayMaster.tv_sec;
   gmtime_r(&Seconds,&tmZeit);

   strncpy(buf,FileName,GENERIC_BUF_LEN);
   strcat(buf,".bin");

   if ((fp=fopen(FileName,"a"))==NULL)
     {
	sprintf(buf,"elekStatus : %s() can't open %s",__FUNCTION__,FileName);
	SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],buf);
     }
   else
     {
	// write data. may return with 1 even if disk is full.
	ret=fwrite(ptrElekStatus,sizeof (struct elekStatusType),1,fp);
	if (ret!=1)
	  {
	     char* pErrorMessage = strerror(errno);
	     sprintf(buf,"elekStatus : DATA NOT WRITTEN, fwrite() returned with error %d: ",ret);
	     strcat(buf,pErrorMessage);
	     SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],buf);
	  };

	// flush buffer to check if disk is full an to prevent data loss
	ret=fflush(fp);
	if (ret == EOF)
	  {
	     char* pErrorMessage = strerror(errno);
	     sprintf(buf,"elekStatus : DATA NOT WRITTEN, fflush() returned with error %d: ",ret);
	     strcat(buf,pErrorMessage);
	     SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],buf);
	  };
	fclose(fp);
     }
   // if fopen
   //
   //    strncpy(buf,Path,GENERIC_BUF_LEN);
   strncpy(buf,PathToRamDisk,GENERIC_BUF_LEN);
   strcat(buf,"/status.bin");

   if ((fp=fopen(buf,"r+"))==NULL)
     {
	SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],"elekStatus : status.bin does not exist");
	if ((fp=fopen(buf,"w+"))==NULL)
	  {
	     SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],"elekStatus : can't create status.bin");
	     return (-1);
	  }
     }
   /* if open */
   //go to the next entry position
   ret=fseek(fp,LastElekStatusNumber*sizeof (struct elekStatusType),SEEK_SET);
   //    sprintf(buf,"ElekStatus: write Status to %d,%d %d %ld",LastElekStatusNumber,
   //	    LastElekStatusNumber*sizeof (struct elekStatusType),ret,ftell(fp) );
   // SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],buf);
   ret=fwrite(ptrElekStatus,sizeof (struct elekStatusType),1,fp);

   if (ret!=1)
     {
	SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],"elekStatus : can't write status.bin");
     }
   LastElekStatusNumber=(LastElekStatusNumber+1) % STATUSFILE_RING_LEN;
   fclose(fp);

   strncpy(buf,PathToRamDisk,GENERIC_BUF_LEN);
   strcat(buf,"/chPMT.txt");
   if ((fp=fopen(buf,"w"))==NULL)
     {
	SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],"elekStatus : can't open chPMT.txt");
	return (-1);
     }
   else
     {
	for(i=0; i<MAX_COUNTER_TIMESLOT;i++)
	  {
	     fprintf(fp,"%3d ",i);
	     for (Channel=0; Channel<MAX_COUNTER_CHANNEL; Channel++)
	       {
		  fprintf(fp,"%4d ",ptrElekStatus->CounterCardMaster.Channel[Channel].Data[i]);
	       }
	     // for channel
	     fprintf(fp,"\n");
	  }
	// for i
	fclose(fp);
     }
   // if fopen
}
/*WriteElekStatus*/

int WriteAuxStatus(char *PathToRamDisk, char *FileName, struct auxStatusType *ptrAuxStatus)
{

   extern struct MessagePortType MessageOutPortList[];
   extern struct MessagePortType MessageInPortList[];

   extern long LastAuxStatusNumber;

   FILE *fp;
   int i;
   int ret;
   long len;
   int nelements;
   char buf[GENERIC_BUF_LEN];

   if ((fp=fopen(FileName,"a"))==NULL)
     {
	sprintf(buf,"elekStatus : can't open %s",FileName);
	SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],buf);
     }
   else
     {
	// write data. may return with 1 even if disk is full.
	ret=fwrite(ptrAuxStatus,sizeof (struct auxStatusType),1,fp);
	if (ret!=1)
	  {
	     char* pErrorMessage = strerror(errno);
	     sprintf(buf,"elekStatus : AUX DATA NOT WRITTEN, fwrite() returned with error %d: ",ret);
	     strcat(buf,pErrorMessage);
	     SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],buf);
	  };

	// flush buffer to check if disk is full an to prevent data loss
	ret=fflush(fp);
	if (ret == EOF)
	  {
	     char* pErrorMessage = strerror(errno);
	     sprintf(buf,"elekStatus : AUX DATA NOT WRITTEN, fflush() returned with error %d: ",ret);
	     strcat(buf,pErrorMessage);
	     SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],buf);
	  };
	fclose(fp);
     }
   // if fopen
   //
   strncpy(buf,PathToRamDisk,GENERIC_BUF_LEN);
   strcat(buf,"/status.aux");

   if ((fp=fopen(buf,"r+"))==NULL)
     {
	SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],"elekStatus : status.aux does not exist");
	if ((fp=fopen(buf,"w+"))==NULL)
	  {
	     SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],"elekStatus : can't create status.aux");
	     return (-1);
	  }
     }
   /* if open */
   //go to the next entry position
   ret=fseek(fp,LastAuxStatusNumber*sizeof (struct auxStatusType),SEEK_SET);
   ret=fwrite(ptrAuxStatus,sizeof (struct auxStatusType),1,fp);

   if (ret!=1)
     {
	SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],"elekStatus : can't write status.aux");
     }
   LastAuxStatusNumber=(LastAuxStatusNumber+1) % STATUSFILE_RING_LEN;
   fclose(fp);

}
/*WriteAuxStatus*/

int WriteCalibStatus(char *PathToRamDisk, char *FileName, struct calibStatusType *ptrCalibStatus)
{

   extern struct MessagePortType MessageOutPortList[];
   extern struct MessagePortType MessageInPortList[];

   extern long LastElekStatusNumber;

   FILE *fp;
   int i;
   int ret;
   long len;
   int nelements;
   char buf[GENERIC_BUF_LEN];

   if ((fp=fopen(FileName,"a"))==NULL)
     {
	sprintf(buf,"elekStatus : can't open %s",FileName);
	SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],buf);
     }
   else
     {
	// write data. may return with 1 even if disk is full.
	ret=fwrite(ptrCalibStatus,sizeof (struct calibStatusType),1,fp);
	if (ret!=1)
	  {
	     char* pErrorMessage = strerror(errno);
	     sprintf(buf,"elekStatus : CALIBDATA NOT WRITTEN, fwrite() returned with error %d: ",ret);
	     strcat(buf,pErrorMessage);
	     SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],buf);
	  };

	// flush buffer to check if disk is full an to prevent data loss
	ret=fflush(fp);
	if (ret == EOF)
	  {
	     char* pErrorMessage = strerror(errno);
	     sprintf(buf,"elekStatus : CALIBDATA NOT WRITTEN, fflush() returned with error %d: ",ret);
	     strcat(buf,pErrorMessage);
	     SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],buf);
	  };
	fclose(fp);
     }
   // if fopen
   //
   strncpy(buf,PathToRamDisk,GENERIC_BUF_LEN);
   strcat(buf,"/status.cal");

   if ((fp=fopen(buf,"r+"))==NULL)
     {
	SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],"elekStatus : status.cal does not exist");
	if ((fp=fopen(buf,"w+"))==NULL)
	  {
	     SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],"elekStatus : can't create status.cal");
	     return (-1);
	  }
     }
   /* if open */
   //go to the next entry position
   ret=fseek(fp,LastCalibStatusNumber*sizeof (struct calibStatusType),SEEK_SET);
   ret=fwrite(ptrCalibStatus,sizeof (struct calibStatusType),1,fp);

   if (ret!=1)
     {
	SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],"elekStatus : can't write status.cal");
     }
   LastCalibStatusNumber=(LastCalibStatusNumber+1) % STATUSFILE_RING_LEN;
   fclose(fp);

}
/*WriteCalibStatus*/

int WriteSpectraStatus(char *PathToRamDisk, char *FileName, struct spectralStatusType *ptrSpectraStatus, unsigned char ucRingbufferOnly)
{

   extern struct MessagePortType MessageOutPortList[];
   extern struct MessagePortType MessageInPortList[];

   extern long LastSpectraStatusNumber;

   FILE *fp;
   int i;
   int ret;
   long len;
   int nelements;
   char buf[GENERIC_BUF_LEN];

   // spectral data are very large and theirefore only recorded in large intervals
   // this flag indicates that only the "live data" in the ramdisk should be updated
   // during this write cycle
   
   if(!ucRingbufferOnly)
     {

	if ((fp=fopen(FileName,"a"))==NULL)
	  {
	     sprintf(buf,"elekStatus : in %s(): can't open %s", __FUNCTION__, FileName);
	     SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],buf);
	  }
	else
	  {
	     // write data. may return with 1 even if disk is full.
	     ret=fwrite(ptrSpectraStatus,sizeof (struct spectralStatusType),1,fp);
	     if (ret!=1)
	       {
		  char* pErrorMessage = strerror(errno);
		  sprintf(buf,"elekStatus : SPECTRALDATA NOT WRITTEN, fwrite() returned with error %d: ",ret);
		  strcat(buf,pErrorMessage);
		  SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],buf);
	       };

	     // flush buffer to check if disk is full an to prevent data loss
	     ret=fflush(fp);
	     if (ret == EOF)
	       {
		  char* pErrorMessage = strerror(errno);
		  sprintf(buf,"elekStatus : SPECTRALDATA NOT WRITTEN, fflush() returned with error %d: ",ret);
		  strcat(buf,pErrorMessage);
		  SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],buf);
	       };
	     fclose(fp);
	  }
     }

   // if fopen
   //
   strncpy(buf,PathToRamDisk,GENERIC_BUF_LEN);
   strcat(buf,"/status.spc");

   if ((fp=fopen(buf,"r+"))==NULL)
     {
	SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],"elekStatus : status.spc does not exist");
	if ((fp=fopen(buf,"w+"))==NULL)
	  {
	     SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],"elekStatus : can't create status.spc");
	     return (-1);
	  }
     }
   /* if open */
   //go to the next entry position
   ret=fseek(fp,LastSpectraStatusNumber*sizeof (struct spectralStatusType),SEEK_SET);
   ret=fwrite(ptrSpectraStatus,sizeof (struct spectralStatusType),1,fp);

   if (ret!=1)
     {
	SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],"elekStatus : can't write status.spc");
     }
   LastSpectraStatusNumber=(LastSpectraStatusNumber+1) % STATUSFILE_RING_LEN;
   fclose(fp);

}
/*WriteSpectraStatus*/

void GenerateFileName(char *Path, char *FileName, struct tm *ReqTime, char *Extension)
{
   struct tm DateNow;
   time_t SecondsNow;

   if (ReqTime==NULL)
     {

	time(&SecondsNow);
	gmtime_r(&SecondsNow,&DateNow);
	ReqTime=&DateNow;
     }

   sprintf(FileName,"%s/%1d%03d%02d%02d.%s",
	   Path,
	   ReqTime->tm_year-100,
	   ReqTime->tm_yday+1,
	   ReqTime->tm_hour,
	   10*(int)(ReqTime->tm_min/10),
	   Extension);

}
/* Generate Filename */

/******************************************************************************************/
/* search Status file for last entry and set global variable accordingly                  */
/******************************************************************************************/

int InitElekStatusFile(char *Path)
{

   extern struct MessagePortType MessageOutPortList[];
   extern struct MessagePortType MessageInPortList[];
   extern long LastElekStatusNumber;

   FILE *fp;
   struct tm tmZeit;
   time_t    Seconds;
   char buf[GENERIC_BUF_LEN];
   struct elekStatusType elekStatus;
   long RecordNo;
   struct timeval LastTime;
   int ret;

   strncpy(buf,Path,GENERIC_BUF_LEN);
   strcat(buf,"/status.bin");
   if ((fp=fopen(buf,"r"))==NULL)
     {
	// file does not exist yet, so we set LastElekStatusNumber to 0
	LastElekStatusNumber=0;
	return (0);
     }

   // find out what the last entry is
   LastElekStatusNumber=0;
   RecordNo=0;
   LastTime.tv_sec=0;
   LastTime.tv_usec=0;

   while (!feof(fp))
     {

	ret=fread(&elekStatus,sizeof (struct elekStatusType),1,fp);
	//      sprintf(buf,"ElekStatus InitElekStatusFile: %d %d. read %ld compare %ld\n",
	//	      feof(fp),RecordNo,
	//	      elekStatus.TimeOfDay.tv_sec,
	//	      LastTime.tv_sec);
	//      SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],buf);
	//
	if ((LastTime.tv_sec<elekStatus.TimeOfDayMaster.tv_sec) &&
	    (LastTime.tv_sec<elekStatus.TimeOfDayMaster.tv_sec))
	  {

	     LastElekStatusNumber=RecordNo;
	     LastTime=elekStatus.TimeOfDayMaster;
	  }
	/* if time */

	RecordNo++;
     }
   /* while feof */

   fclose(fp);

}
/* Init Elek Status File */

/******************************************************************************************/
/* search Calib Status file for last entry and set global variable accordingly            */
/******************************************************************************************/

int InitCalibStatusFile(char *Path)
{

   extern struct MessagePortType MessageOutPortList[];
   extern struct MessagePortType MessageInPortList[];
   extern long LastCalibStatusNumber;

   FILE *fp;
   struct tm tmZeit;
   time_t    Seconds;
   char buf[GENERIC_BUF_LEN];
   struct calibStatusType calibStatus;
   long RecordNo;
   struct timeval LastTime;
   int ret;

   strncpy(buf,Path,GENERIC_BUF_LEN);
   strcat(buf,"/status.cal");
   if ((fp=fopen(buf,"r"))==NULL)
     {
	// file does not exist yet, so we set LastCalibStatusNumber to 0
	LastCalibStatusNumber=0;
	return (0);
     }

   // find out what the last entry is
   LastCalibStatusNumber=0;
   RecordNo=0;
   LastTime.tv_sec=0;
   LastTime.tv_usec=0;

   while (!feof(fp))
     {

	ret=fread(&calibStatus,sizeof (struct calibStatusType),1,fp);

	if ((LastTime.tv_sec<calibStatus.TimeOfDayCalib.tv_sec) &&
	    (LastTime.tv_sec<calibStatus.TimeOfDayCalib.tv_sec))
	  {

	     LastCalibStatusNumber=RecordNo;
	     LastTime=calibStatus.TimeOfDayCalib;
	  }
	/* if time */

	RecordNo++;
     }
   /* while feof */

   fclose(fp);

}
/* Init Calib Status File */

/******************************************************************************************/
/* search Aux Status file for last entry and set global variable accordingly              */
/******************************************************************************************/

int InitAuxStatusFile(char *Path)
{

   extern struct MessagePortType MessageOutPortList[];
   extern struct MessagePortType MessageInPortList[];
   extern long LastAuxStatusNumber;

   FILE *fp;
   struct tm tmZeit;
   time_t    Seconds;
   char buf[GENERIC_BUF_LEN];
   struct auxStatusType auxStatus;
   long RecordNo;
   struct timeval LastTime;
   int ret;

   strncpy(buf,Path,GENERIC_BUF_LEN);
   strcat(buf,"/status.aux");
   if ((fp=fopen(buf,"r"))==NULL)
     {
	// file does not exist yet, so we set LastAuxStatusNumber to 0
	LastAuxStatusNumber=0;
	return (0);
     }

   // find out what the last entry is
   LastAuxStatusNumber=0;
   RecordNo=0;
   LastTime.tv_sec=0;
   LastTime.tv_usec=0;

   while (!feof(fp))
     {

	ret=fread(&auxStatus,sizeof (struct auxStatusType),1,fp);

	if ((LastTime.tv_sec<auxStatus.TimeOfDayAux.tv_sec) &&
	    (LastTime.tv_sec<auxStatus.TimeOfDayAux.tv_sec))
	  {

	     LastAuxStatusNumber=RecordNo;
	     LastTime=auxStatus.TimeOfDayAux;
	  }
	/* if time */

	RecordNo++;
     }
   /* while feof */

   fclose(fp);

}
/* Init Aux Status File */

/******************************************************************************************/
/* search Spectra Status file for last entry and set global variable accordingly          */
/******************************************************************************************/

int InitSpectraStatusFile(char *Path)
{

   extern struct MessagePortType MessageOutPortList[];
   extern struct MessagePortType MessageInPortList[];
   extern long LastSpectraStatusNumber;

   FILE *fp;
   struct tm tmZeit;
   time_t    Seconds;
   char buf[GENERIC_BUF_LEN];
   struct spectralStatusType spectraStatus;
   long RecordNo;
   struct timeval LastTime;
   int ret;

   strncpy(buf,Path,GENERIC_BUF_LEN);
   strcat(buf,"/status.spc");
   if ((fp=fopen(buf,"r"))==NULL)
     {
	// file does not exist yet, so we set LastSpectraStatusNumber to 0
	LastSpectraStatusNumber=0;
	return (0);
     }

   // find out what the last entry is
   LastSpectraStatusNumber=0;
   RecordNo=0;
   LastTime.tv_sec=0;
   LastTime.tv_usec=0;

   while (!feof(fp))
     {

	ret=fread(&spectraStatus,sizeof (struct spectralStatusType),1,fp);

	if ((LastTime.tv_sec<spectraStatus.TimeOfDaySpectra.tv_sec) &&
	    (LastTime.tv_sec<spectraStatus.TimeOfDaySpectra.tv_sec))
	  {

	     LastSpectraStatusNumber=RecordNo;
	     LastTime=spectraStatus.TimeOfDaySpectra;
	  }
	/* if time */

	RecordNo++;
     }
   /* while feof */

   fclose(fp);

}
/* Init Spectra Status File */

void EvaluateKeyboard(void)
{
   unsigned char ucChar = 0;
   int iCharacter = getch();	// getchar uses INT datatype
   if(iCharacter != EOF)			// check for error or no key pressed
     {
	ucChar = toupper((unsigned char)iCharacter);
	switch(ucChar)
	  {
	   case 'D':					// Dataset Data (record number, structure size)
	     uiGroupFlags ^= GROUP_DATASETDATA;
	     if(uiGroupFlags & GROUP_DATASETDATA)
	       printf("\n\rDisplay of DATASET DATA is now on.\n\r");
	     else
	       printf("\n\rDisplay of DATASET DATA is now off.\n\r");
	     break;

	   case 'T':					// Time Data
	     uiGroupFlags ^= GROUP_TIMEDATA;
	     if(uiGroupFlags & GROUP_TIMEDATA)
	       printf("\n\rDisplay of TIME DATA is now on.\n\r");
	     else
	       printf("\n\rDisplay of TIME DATA is now off.\n\r");
	     break;

	   case 'G':					// GPS Data
	     uiGroupFlags ^= GROUP_GPSDATA;
	     if(uiGroupFlags & GROUP_GPSDATA)
	       printf("\n\rDisplay of GPS DATA is now on.\n\r");
	     else
	       printf("\n\rDisplay of GPS DATA is now off.\n\r");
	     break;

	   case 'C':					// Countercard Data
	     uiGroupFlags ^= GROUP_CCDATA;
	     if(uiGroupFlags & GROUP_CCDATA)
	       printf("\n\rDisplay of COUNTERCARD DATA is now on.\n\r");
	     else
	       printf("\n\rDisplay of COUNTERCARD DATA is now off.\n\r");
	     break;

	   case 'M':					// Countercard Mask
	     uiGroupFlags ^= GROUP_CCMASK;
	     if(uiGroupFlags & GROUP_CCMASK)
	       printf("\n\rDisplay of COUNTERCARD MASK is now on.\n\r");
	     else
	       printf("\n\rDisplay of COUNTERCARD MASK is now off.\n\r");
	     break;

	   case 'E':					// Etalon Data
	     uiGroupFlags ^= GROUP_ETALONDATA;
	     if(uiGroupFlags & GROUP_ETALONDATA)
	       printf("\n\rDisplay of ETALON DATA is now on.\n\r");
	     else
	       printf("\n\rDisplay of ETALON DATA is now off.\n\r");
	     break;

	   case 'A':					// ADC Data
	     uiGroupFlags ^= GROUP_ADCDATA;
	     if(uiGroupFlags & GROUP_ADCDATA)
	       printf("\n\rDisplay of ADC DATA is now on.\n\r");
	     else
	       printf("\n\rDisplay of ADC DATA is now off.\n\r");
	     break;

	   case 'P':					// Temperature Probes
	     uiGroupFlags ^= GROUP_TEMPDATA;
	     if(uiGroupFlags & GROUP_TEMPDATA)
	       printf("\n\rDisplay of TEMP DATA is now on.\n\r");
	     else
	       printf("\n\rDisplay of TEMP DATA is now off.\n\r");
	     break;

	   case 'V':					// Temperature Probes
	     uiGroupFlags ^= GROUP_VALVEDATA;
	     if (uiGroupFlags & GROUP_VALVEDATA)
	       printf("\n\rDisplay of VALVE DATA is now on.\n\r");
	     else
	       printf("\n\rDisplay of VALVE DATA is now off.\n\r");
	     break;

	   case 'B':					// Butterfly
	     uiGroupFlags ^= GROUP_BUTTERFLYDATA;
	     if (uiGroupFlags & GROUP_BUTTERFLYDATA)
	       printf("\n\rDisplay of BUTTERFLY DATA is now on.\n\r");
	     else
	       printf("\n\rDisplay of BUTTERFLY DATA is now off.\n\r");
	     break;

	   case 'L':					// Butterfly
	     uiGroupFlags ^= GROUP_CALIBDATA;
	     if (uiGroupFlags & GROUP_CALIBDATA)
	       printf("\n\rDisplay of LICOR DATA is now on.\n\r");
	     else
	       printf("\n\rDisplay of LICOR DATA is now off.\n\r");
	     break;

	   case 'X':					// Butterfly
	     uiGroupFlags ^= GROUP_AUXDATA;
	     if (uiGroupFlags & GROUP_AUXDATA)
	       printf("\n\rDisplay of AUXILIARY DATA is now on.\n\r");
	     else
	       printf("\n\rDisplay of AUXILIARY DATA is now off.\n\r");
	     break;

	   case 'S':					// Show all
	     uiGroupFlags = 0xFFFFFFFF;
	     printf("\n\rDisplaying ALL data now.\n\r");
	     break;

	   case 'R':					// hide all but time
	     uiGroupFlags = GROUP_TIMEDATA;
	     printf("\n\rDisplaying NONE but TIME data now.\n\r");
	     break;

	   case 'H':
	     ShowHelp();
	     break;

	   default:
	     break;
	  };
     };
};

void ShowHelp(void)
{
   printf("\n\rPress to toggle displaying of:\n\r\n\r");
   printf("[A] ADC DATA    \t[E] ETALON DATA     \t[G] GPS DATA\n\r");
   printf("[C] CC DATA     \t[M] CC Mask info    \t[P] TEMPERATURE DATA\n\r");
   printf("[D] DATASET DATA\t[V] VALVE DATA      \t[B] BUTTERFLY DATA\n\r");
   printf("[T] TIME DATA   \t[L] LICOR DATA      \t[X] AUX DATA (METEO)\n\r");
   printf("[S] SHOW ALL    \t[R] RESET ALL\n\r");
   printf("\n\r*** PRESS [H] FOR HELP DURING DATA DUMPING! ***\n\r");
};

#define ONEMILLION 1000000

// helper routine to fix broken timestamps
void fix_timeval(struct timeval *a)
{
   if(a->tv_usec >= ONEMILLION)
     {
	a->tv_sec += a->tv_usec / ONEMILLION;
	a->tv_usec %= ONEMILLION;
     }
   else if(a->tv_usec < 0)
     {
	a->tv_sec = 0;
     };
}
// Subtract the struct timeval' values a and b,
// storing the result in result.
//
void timeval_subtract (struct timeval *result, struct timeval *a, struct timeval *b)
{
   fix_timeval(a);
   fix_timeval(b);

   result->tv_sec = a->tv_sec - b->tv_sec;
   result->tv_usec = a->tv_usec - b->tv_usec;
   if(result->tv_usec < 0)
     {
	result->tv_sec--;
	result->tv_usec += ONEMILLION;
     };
}

int main()
{
   long CalibStatusCount;
   long AuxStatusCount;
   long SpectraStatusCount;

   extern int errno;
   extern struct MessagePortType MessageOutPortList[];
   extern struct MessagePortType MessageInPortList[];

   struct elekStatusType ElekStatus;
   struct calibStatusType CalibStatus;
   struct auxStatusType AuxStatus;
   struct spectralStatusType SpectraStatus;

   // needed for spectrum analyzer packet assembly
   // the spectral data are send in 2 UDP packets
   // which need to be assembled in a buffer to
   // finally write them to disk
   //
#define SPECBUFSIZE (8*1024) // 8K words = 16kbytes

   uint16_t usTempSpectrumBuffer[SPECBUFSIZE];

#define SRS_WAIT_FIRST (0)
#define SRS_WAIT_SECOND (1)

   int iSpectraRxState;
   struct timeval tvTimeOfLastSpectrum;    // time when last spectrum was written to disk
   struct timeval tvTimeOfLastLiveSpectrum;// time when last live spectrum was written to ramdisk
   struct timeval tvCurrentTime;           // current time

   int MessagePort;
   int fdMax;                      // max fd for select
   int i;                          // loop counter
   int fdNum;                      // fd number in loop
   fd_set fdsMaster;               // master file descriptor list
   fd_set fdsSelect;               // temp file descriptor list for select()
   int ret;
   struct timeval timeout;         // timeout
   struct timespec RealTime;         // Real time clock
   struct sockaddr_in my_addr;     // my address information
   struct sockaddr_in their_addr;  // connector's address information
   int    ElekStatus_len, CalibStatus_len,AuxStatus_len,SpectraStatus_len,numbytes;
   socklen_t addr_len;
   bool   EndOfSession;

   char buf[GENERIC_BUF_LEN];

   char StatusFileName[MAX_FILENAME_LEN];
   char CalibStatusFileName[MAX_FILENAME_LEN];
   char AuxStatusFileName[MAX_FILENAME_LEN];
   char SpectraStatusFileName[MAX_FILENAME_LEN];

   if(cbreak(STDIN_FILENO) == -1)
     {
	printf("Fehler bei der Funktion cbreak ... \n");
	exit(EXIT_FAILURE);
     }

   // setup master fd
   FD_ZERO(&fdsMaster);              // clear the master and temp sets
   FD_ZERO(&fdsSelect);

   // init inports
   for (MessagePort=0; MessagePort<MAX_MESSAGE_INPORTS;MessagePort++)
     {

	printf("opening IN Port %s on Port %d\n",
	       MessageInPortList[MessagePort].PortName,
	       MessageInPortList[MessagePort].PortNumber);

	MessageInPortList[MessagePort].fdSocket=InitUDPInSocket(MessageInPortList[MessagePort].PortNumber);

	FD_SET(MessageInPortList[MessagePort].fdSocket, &fdsMaster);     // add the manual port to the master set
	fdMax=MessageInPortList[MessagePort].fdSocket;                   // the last one will give the max number
     }
   /* for MessageInPort */

   // init outports
   for (MessagePort=0; MessagePort<MAX_MESSAGE_OUTPORTS;MessagePort++)
     {
	printf("opening OUT Port %s on Port %d, Destination IP: %s\n",
	       MessageOutPortList[MessagePort].PortName,
	       MessageOutPortList[MessagePort].PortNumber,
	       MessageOutPortList[MessagePort].IPAddr);

	MessageOutPortList[MessagePort].fdSocket=InitUDPOutSocket(MessageOutPortList[MessagePort].PortNumber);

     }
   /* for MessageOutPort */

   addr_len = sizeof(struct sockaddr);
   ElekStatus_len=sizeof(struct elekStatusType);
   CalibStatus_len=sizeof(struct calibStatusType);
   AuxStatus_len=sizeof(struct auxStatusType);
   SpectraStatus_len=sizeof(struct spectralStatusType);

   gettimeofday(&tvTimeOfLastSpectrum);

   //    refresh();
#ifdef RUNONARM
   sprintf(buf,"This is elekStatus Version %3.2f ($Id: elekStatus.c,v 1.39 2007-10-24 15:14:57 rudolf Exp $) for ARM\nexpected StatusLen\nfor elekStatus:%d\nfor calibStatus:%d\nfor auxStatus:%d\nfor spectraStatus:%d\n",VERSION,ElekStatus_len,CalibStatus_len,AuxStatus_len,SpectraStatus_len);
#else
   sprintf(buf,"This is elekStatus Version %3.2f ($Id: elekStatus.c,v 1.39 2007-10-24 15:14:57 rudolf Exp $) for i386\nexpected StatusLen\nfor elekStatus:%d\nfor calibStatus:%d\nfor auxStatus:%d\nfor spectraStatus:%d\n",VERSION,ElekStatus_len,CalibStatus_len,AuxStatus_len,SpectraStatus_len);
#endif

   SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],buf);

   ShowHelp();
   //    GenerateFileName(DATAPATH,StatusFileName,NULL);
   //
   // Check Status ringfile buffer and initialize the pointer to the appropriate position
   InitElekStatusFile(RAMDISKPATH);
   InitCalibStatusFile(RAMDISKPATH);
   InitSpectraStatusFile(RAMDISKPATH);

   iSpectraRxState = SRS_WAIT_FIRST;

   EndOfSession=FALSE;
   while (!EndOfSession)
     {
	//        printf("wait for data ....\n");
	//
	fdsSelect=fdsMaster;
	timeout.tv_sec= UDP_SERVER_TIMEOUT;
	timeout.tv_usec=0;
	ret=select(fdMax+1, &fdsSelect, NULL, NULL, &timeout);

	//	printf("ret %d\n",ret);
	if (ret ==-1 )
	  {
	     // select error
	     perror("select");
	     SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],"Problem with select");

	  }
	else if (ret>0)
	  {
	     //	    printf("woke up...");

	     for (MessagePort=0; MessagePort<MAX_MESSAGE_INPORTS;MessagePort++)
	       {

		  if (FD_ISSET(MessageInPortList[MessagePort].fdSocket,&fdsSelect))
		    {
		       // new msg on fdNum. socket ...
		       //
		       switch (MessagePort)
			 {
			  case ELEK_ELEKIO_IN:
			    if ((numbytes=recvfrom(MessageInPortList[MessagePort].fdSocket,
						   &ElekStatus,ElekStatus_len , MSG_WAITALL,
						   (struct sockaddr *)&their_addr, &addr_len)) == -1)
			      {
				 perror("recvfrom");
				 SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],"elekStatus : Problem with receive");
			      }
			    StatusCount++;
			    EvaluateKeyboard();
			    if ((StatusCount % 5)==0)
			      {
				 PrintElekStatus(&ElekStatus, numbytes);
			      }

			    GenerateFileName(DATAPATH,StatusFileName,NULL,"bin");

			    if (ElekStatus.InstrumentFlags.StatusSave)
			      WriteElekStatus(RAMDISKPATH, StatusFileName,&ElekStatus);
			    else SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],"elekStatus : DATA NOT STORED !!!");
			    break;

			  case CALIB_IN:
			    if ((numbytes=recvfrom(MessageInPortList[MessagePort].fdSocket,
						   &CalibStatus,CalibStatus_len , MSG_WAITALL,
						   (struct sockaddr *)&their_addr, &addr_len)) == -1)
			      {
				 perror("recvfrom");
				 SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],"elekStatus : Problem with receive");
			      }
			    // update timestamp in calib structure
			    gettimeofday(&CalibStatus.TimeOfDayCalib,NULL);
			    CalibStatusCount++;
			    EvaluateKeyboard();
			    if ((CalibStatusCount % 5)==0)
			      {
				 PrintCalibStatus(&CalibStatus, numbytes);
			      }

			    GenerateFileName(DATAPATH,CalibStatusFileName,NULL,"cal");
			    WriteCalibStatus(RAMDISKPATH, CalibStatusFileName,&CalibStatus);
			    break;

			  case AUX_IN:
			    if ((numbytes=recvfrom(MessageInPortList[MessagePort].fdSocket,
						   &AuxStatus,AuxStatus_len , MSG_WAITALL,
						   (struct sockaddr *)&their_addr, &addr_len)) == -1)
			      {
				 perror("recvfrom");
				 SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],"elekStatus : Problem with receive");
			      }

			    AuxStatusCount++;
			    EvaluateKeyboard();
			    if ((AuxStatusCount % 5)==0)
			      {
				 PrintAuxStatus(&AuxStatus, numbytes);
			      }

			    GenerateFileName(DATAPATH,AuxStatusFileName,NULL,"aux");
			    WriteAuxStatus(RAMDISKPATH, AuxStatusFileName,&AuxStatus);
			    break;

			    // Spectrometer Data processing
			    // collect one complete set of packets before storing
			    // timestamp is saved when first packet is received
			    //
			  case SPECTRA_IN:
			    if ((numbytes=recvfrom(MessageInPortList[MessagePort].fdSocket,
						   &usTempSpectrumBuffer, SPECBUFSIZE , MSG_WAITALL,
						   (struct sockaddr *)&their_addr, &addr_len)) == -1)
			      {
				 perror("recvfrom");SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],"elekStatus : Problem with receive");
			      }
#ifdef DEBUG_SPECTROMETER
			    sprintf(buf,"elekStatus : SPECTRA_IN: Got %d from spectrometerServer",numbytes);
			    SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],buf);
#endif
			    if((numbytes != 8192) && (numbytes != 6424) && (numbytes != 16))
			      {
				 sprintf(buf,"elekStatus : SPECTRA_IN: unexpected packet size (%d bytes)",numbytes);
				 SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],buf);
				 break;
			      };

			    // check which packet we got
			    // packet number is at offset 0
			    // LSB is current number
			    // MSB is max packet number
			    //
#ifdef DEBUG_SPECTROMETER
			    sprintf(buf,"elekStatus : RXSTATE is %d",iSpectraRxState);
			    SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],buf);
#endif
			    if((usTempSpectrumBuffer[0] & 0x00ff) == 1)
			      {
				 // check if we are waiting for packet 1
				 if(iSpectraRxState == SRS_WAIT_FIRST)
				   {
				      // copy 1st buffer to spectra structure
				      // save timestamp when first package arrives
				      //
				      gettimeofday(&SpectraStatus.TimeOfDaySpectra,NULL);
				      SpectraStatus.uiLineCount = (usTempSpectrumBuffer[3]/4); // 4 bytes per line, 2 for wavelen, 2 for sample

#ifdef DEBUG_SPECTROMETER
				      sprintf(buf,"elekStatus : SPECTRA_IN: spectrum consists of %d spectral lines", SpectraStatus.uiLineCount);
				      SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],buf);
#endif

				      //copy spectrum to structure
				      int iLoop;
				      int iLinesInThisPacket;
				      unsigned short *pusWaveLen = &usTempSpectrumBuffer[4];
				      unsigned short *pusCounts = &usTempSpectrumBuffer[5];

				      iLinesInThisPacket = (numbytes - 8) / 4; // header is 4 unsigned shorts, one spectral line is 4 bytes

#ifdef DEBUG_SPECTROMETER
				      sprintf(buf,"elekStatus : SPECTRA_IN: This packet consists of %d spectral lines", iLinesInThisPacket);
				      SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],buf);
#endif

				      for(iLoop=0; iLoop < (iLinesInThisPacket); iLoop++)
					{
					   SpectraStatus.SpectralData[iLoop].uiWaveLength = *pusWaveLen;
					   SpectraStatus.SpectralData[iLoop].uiCounts = *pusCounts;
					   pusWaveLen += 2;
					   pusCounts += 2;
					};
				      iSpectraRxState = SRS_WAIT_SECOND; // first packet received, now wait for the second one
				   }
				 else
				   {
				      sprintf(buf,"elekStatus : SPECTRA_IN: got packet %d while expecting 1, dropping packet...", usTempSpectrumBuffer[0] & 0x00ff);
				      SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],buf);
				      iSpectraRxState = SRS_WAIT_FIRST; // received out-of-band packet, so drop and restart with first packet
				   }
			      }

			    else

			      if((usTempSpectrumBuffer[0] & 0x00ff) == 2)
				{
				   // check if we are waiting for packet 2
				   if(iSpectraRxState == SRS_WAIT_SECOND)
				     {
#ifdef DEBUG_SPECTROMETER
					sprintf(buf,"elekStatus : SPECTRA_IN: got packet 2, expected 2");
					SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],buf);
#endif

					// copy 2nd buffer to spectra structure
					//
					SpectraStatus.uiLineCount = (usTempSpectrumBuffer[3]/4); // 4 bytes per line, 2 for wavelen, 2 for sample

#ifdef DEBUG_SPECTROMETER
					sprintf(buf,"elekStatus : SPECTRA_IN: spectrum consists of %d spectral lines", SpectraStatus.uiLineCount);
					SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],buf);
#endif

					//copy spectrum to structure
					int iLoop;
					int iLinesInThisPacket;
					int iPacketOffset;

					unsigned short *pusWaveLen = &usTempSpectrumBuffer[4];
					unsigned short *pusCounts = &usTempSpectrumBuffer[5];

					iLinesInThisPacket = (numbytes - 8) / 4; // header is 4 unsigned shorts, one spectral line is 4 bytes

#ifdef DEBUG_SPECTROMETER
					sprintf(buf,"elekStatus : SPECTRA_IN: This packet consists of %d spectral lines", iLinesInThisPacket);
					SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],buf);
#endif

					// Offset in Bytes is included in packet
					iPacketOffset = (int)(usTempSpectrumBuffer[2]) / 4;

#ifdef DEBUG_SPECTROMETER
					sprintf(buf,"elekStatus : SPECTRA_IN: Saving packet at line offset %d", iPacketOffset);
					SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],buf);
#endif

					for(iLoop=iPacketOffset; iLoop < (iLinesInThisPacket+iPacketOffset); iLoop++)
					  {
					     SpectraStatus.SpectralData[iLoop].uiWaveLength = *pusWaveLen;
					     SpectraStatus.SpectralData[iLoop].uiCounts = *pusCounts;
					     pusWaveLen += 2;
					     pusCounts += 2;
					  };

					// fill out missing fields
					//
					SpectraStatus.uiMinWaveLength = SpectraStatus.SpectralData[0].uiWaveLength;
					SpectraStatus.uiMaxWaveLength = SpectraStatus.SpectralData[iLoop-1].uiWaveLength;
					struct timeval tvDelta;
					struct timeval tvDeltaLiveData;

#ifdef DEBUG_SPECTROMETER
					sprintf(buf,"elekStatus : SPECTRA_IN: MinWaveLen is %5.4f at index 0, MaxWavelen is %5.4f at index %d",
						((double)SpectraStatus.uiMinWaveLength)/50.0f, ((double)SpectraStatus.uiMaxWaveLength)/50.0f, iLoop-1);
					SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],buf);
#endif
					iSpectraRxState = SRS_WAIT_FIRST; // got the second packet, reset statemachine for further packets

					// calculate time delta since last spectrum was saved
					gettimeofday(&tvCurrentTime);

					// calculate deltas for recording every x seconds and live data every second
					timeval_subtract(&tvDelta, &tvCurrentTime, &tvTimeOfLastSpectrum);
					timeval_subtract(&tvDeltaLiveData, &tvCurrentTime, &tvTimeOfLastLiveSpectrum);

#ifdef DEBUG_SPECTROMETER					
     					sprintf(buf,"elekStatus : SPECTRA_IN: DeltaStatus(save) %ld.%06lds",tvDelta.tv_sec, tvDelta.tv_usec);
					SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],buf);
					
					sprintf(buf,"elekStatus : SPECTRA_IN: DeltaStatus(live) %ld.%06lds",tvDeltaLiveData.tv_sec, tvDeltaLiveData.tv_usec);
					SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],buf);
#endif
					
					// save timestamps in structure
					memcpy(&(SpectraStatus.TimeOfDaySpectra),&tvCurrentTime,sizeof(struct timeval));
					memcpy(&(SpectraStatus.TimeOfDayStatus),&(ElekStatus.TimeOfDayMaster),sizeof(struct timeval));

					// copy current etalon parameters into structure
					SpectraStatus.Set.Position = ElekStatus.EtalonData.Set.Position;
					SpectraStatus.Current.Position = ElekStatus.EtalonData.Current.Position;
					SpectraStatus.Encoder.Position = ElekStatus.EtalonData.Encoder.Position;
#ifdef DEBUG_SPECT_ON_OFFLINE
					sprintf(buf,"elekStatus : Spectrum is dated %ld.%06lds",tvCurrentTime.tv_sec, tvDeltaLiveData.tv_usec);
					SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],buf);

					sprintf(buf,"elekStatus : Etalon is dated   %ld.%06lds",ElekStatus.TimeOfDayMaster.tv_sec, ElekStatus.TimeOfDayMaster.tv_usec);
					SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],buf);
					
					sprintf(buf,"elekStatus : Etalon: SET(%07d) CUR(%07d) ENC(%07d)",SpectraStatus.Set.Position,SpectraStatus.Current.Position,SpectraStatus.Encoder.Position);
					SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],buf);

					sprintf(buf,"elekStatus : Etalon Mode: %02d",ElekStatus.InstrumentFlags.EtalonAction);
					SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],buf);

#endif					// check if we have reached 10 secs between spectra for saving
					if(tvDelta.tv_sec >= 10)
					  {
					     // write ringbuffer and statusfile
       					     GenerateFileName(DATAPATH,SpectraStatusFileName,NULL,"spc");
					     WriteSpectraStatus(RAMDISKPATH, SpectraStatusFileName,&SpectraStatus,0);

					     // print delta
					     sprintf(buf,"elekStatus : SPECTRA_IN: wrote livedata and statusdata after  %ld.%06lds",tvDelta.tv_sec, tvDelta.tv_usec);
					     SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],buf);

					     // increment status count
					     SpectraStatusCount++;

					     // save time is current time(NOW)
					     memcpy(&tvTimeOfLastSpectrum,&tvCurrentTime,sizeof(struct timeval));

					     // save time is current time(NOW)
					     memcpy(&tvTimeOfLastLiveSpectrum,&tvCurrentTime,sizeof(struct timeval));

					  }
					else if(tvDeltaLiveData.tv_sec >= 1)
					  {
					     // write only ringbuffer
       					     GenerateFileName(DATAPATH,SpectraStatusFileName,NULL,"spc");
					     WriteSpectraStatus(RAMDISKPATH, SpectraStatusFileName,&SpectraStatus,1);

					     // print delta
					     sprintf(buf,"elekStatus : SPECTRA_IN: wrote livedata after %ld.%06lds",tvDeltaLiveData.tv_sec, tvDeltaLiveData.tv_usec);
					     SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],buf);

					     // increment status count
					     SpectraStatusCount++;

					     // save time is current time(NOW)
					     memcpy(&tvTimeOfLastLiveSpectrum,&tvCurrentTime,sizeof(struct timeval));

					  };
				     }
				   else
				     {
#ifdef DEBUG_SPECTROMETER
					sprintf(buf,"elekStatus : SPECTRA_IN: got packet %d while expecting 2, dropping packet...", usTempSpectrumBuffer[0] & 0x00ff);
					SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],buf);
#endif
					iSpectraRxState = SRS_WAIT_FIRST; // received out-of-band packet, so drop and restart with first packet
				     };
				};

			    EvaluateKeyboard();
			    //if ((SpectraStatusCount % 5)==0) {
			    //  PrintSpectraStatus(&SpectraStatus, numbytes);
			    //}

			    break;

			  case ELEK_STATUS_REQ_IN:
			    break;

			  default :
			    SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],"Unknown Message Port");
			    break;

			 }
		       /* switch */

		    }
		  /* if fd_isset */
	       }
	     /* for MessagePort */

	  }
	else
	  {
	     /* ret==0*/
	     printf("timeout...\n");
	     SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],"elekStatus : TimeOut");
	  }
     }
   /* while */

   // close all in bound sockets
   for (MessagePort=0; MessagePort<MAX_MESSAGE_INPORTS;MessagePort++)
     {
	close(MessageInPortList[MessagePort].fdSocket);
     }
   /*for MessagePort */

   // close all out bound sockets
   for (MessagePort=0; MessagePort<MAX_MESSAGE_OUTPORTS;MessagePort++)
     {
	close(MessageOutPortList[MessagePort].fdSocket);
     }
   /*for MessagePort */
  /*Alten Terminal-Modus wiederherstellen*/
   restoreinput();
   exit(EXIT_SUCCESS);
}
