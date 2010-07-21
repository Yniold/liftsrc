/*
 * $RCSfile: elekConverter.c,v $ last changed on $Date: 2005/09/07 09:32:50 $ by $Author: rudolf $
 *
 * $Log: elekConverter.c,v $
 * Revision 1.1  2005/09/07 09:32:50  rudolf
 * added a small tool for reading a binary dataset. Based on elekStatus
 *
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
#include <asm/msr.h>
#endif

#include <signal.h>
#include <errno.h>
#include <termios.h>

#include "elekConverter.h"
#include "../include/elekGeneral.h"
#include "../include/elekIO.h"
#include "../include/elekIOPorts.h"


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

  if(uiGroupFlags & GROUP_CCMASK) {
    // *****************
    // MASTER CounterCard
    // *****************
      
    printf("Mask");
    printf("(M):");
    
    for (Channel=0; Channel<MAX_COUNTER_CHANNEL; Channel++) {

      printf("c%d:", Channel);
      for (i=0;i<10; i++) {
	printf("%04x.",ptrElekStatus->CounterCardMaster.Channel[Channel].Mask[i]);
      } // for i
    } // for Channel


    // SLAVE

    printf("(S):");
    
    for (Channel=0; Channel<MAX_COUNTER_CHANNEL; Channel++) {

      printf("c%d:", Channel);
      for (i=0;i<10; i++) {
	printf("%04x.",ptrElekStatus->CounterCardSlave.Channel[Channel].Mask[i]);
      } // for i
    } // for Channel

    
    
    
  } // uiGroupFlags & GROUP_CCMASK
  


	
  // ***************** ADC CARDS DATA **************	
		
  if(uiGroupFlags & GROUP_ADCDATA)
    {
      // MASTER
      // normal 16bit ADC Card

      printf("Ana(M):");
      for (Card=0; Card<MAX_ADC_CARD_LIFT; Card ++) 
	{
	  printf("Card#%d:",Card);
	  for (Channel=0;Channel<MAX_ADC_CHANNEL_PER_CARD; Channel++) 
	    {	    
	      printf("%05d ",ptrElekStatus->ADCCardMaster[Card].ADCChannelData[Channel].ADCData);
	    } /* for Channel */
	} /* for Card */ 
		
      // SLAVE
      // normal 16bit ADC Card
      printf("Ana(S):");
      for (Card=0; Card<MAX_ADC_CARD_WP; Card ++) 
	{
	  printf("Card#%d:",Card);
	  for (Channel=0;Channel<MAX_ADC_CHANNEL_PER_CARD; Channel++) 
	    {	    
	      printf("%05d ",ptrElekStatus->ADCCardSlave[Card].ADCChannelData[Channel].ADCData);
	    } /* for Channel */
	} /* for Card */    
		   
    };  

  // ***************** TEMPERATURE DATA **************	

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
	
  if(uiGroupFlags & GROUP_GPSDATA)
    {
	if(0)
	{
		// MASTER
		// GPS Data
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
	};	
      // SLAVE
      // GPS Data
	
      printf("GPS(S) T:");
      printf("%02d:",ptrElekStatus->GPSDataSlave.ucUTCHours);   /* binary, not BCD coded (!) 0 - 23 decimal*/
      printf("%02d:",ptrElekStatus->GPSDataSlave.ucUTCMins);    /* binary, 0-59 decimal */
      printf("%02d ",ptrElekStatus->GPSDataSlave.ucUTCSeconds); /* binary 0-59 decimal */
	
      if(0)
      {
		printf("LON:%3.8f ",ptrElekStatus->GPSDataSlave.dLongitude);     /* "Laengengrad" I always mix it up..
										signed notation,
										negative values mean "W - west of Greenwich"
										positive values mean "E - east of Greenwich" */
		
		printf("LAT:%3.8f ",ptrElekStatus->GPSDataSlave.dLatitude);      /* "Breitengrad" I always mix it up...
										signed notation,
										negative values mean "S - south of the equator
										positive values mean "N - north of the equator */
	};	
      printf("ALT:%f ",ptrElekStatus->GPSDataSlave.fAltitude);   /* Altitude */
      
      printf("DOP:%f ",ptrElekStatus->GPSDataSlave.fHDOP);          /* Horizontal Dillution Of Precision, whatever it means....*/
	
      printf("Sat:%d ",ptrElekStatus->GPSDataSlave.ucNumberOfSatellites); /* number of satellites seen by the GPS receiver */
      printf("Val:%d ",ptrElekStatus->GPSDataSlave.ucLastValidData);      /* number of data aquisitions (5Hz) with no valid GPS data
									     will stick at 255 if no data received for a long period */
	
      printf("SPD:%d ",ptrElekStatus->GPSDataSlave.uiGroundSpeed);  /* speed in cm/s above ground */
      printf("HDG:%d ",ptrElekStatus->GPSDataSlave.uiHeading);      /* 10 times heading in degrees e.g. 2700 decimal = 270,0 Degress = west */
			

    };

  // ***************** VALVE DATA **************		 
	
  if (uiGroupFlags & GROUP_VALVEDATA) {

    // MASTER
    printf("Val(M):");
    for (Card=0; Card<MAX_VALVE_CARD_LIFT; Card++) {
      printf("%4x/%4x ",ptrElekStatus->ValveCardMaster[Card].ValveVolt,
	     ptrElekStatus->ValveCardMaster[Card].Valve);
    } // for Card
	   
    printf("Val(S):");
    for (Card=0; Card<MAX_VALVE_CARD_WP; Card++) {
      printf("%4x/%4x ",ptrElekStatus->ValveCardSlave[Card].ValveVolt,
	     ptrElekStatus->ValveCardSlave[Card].Valve);
    } // for Card			   

  } // if GROUP_VALVEDATA
	   


    
    // CounterCard ShiftDelays
  //    for(i=0; i<MAX_COUNTER_CHANNEL;i++)
  //	ptrElekStatus->CounterCard.Channel[i].ShiftDelay=elkReadData(ELK_COUNTER_DELAY_SHIFT+2*i);
  //    ptrElekStatus->CounterCard.MasterDelay=elkReadData(ELK_COUNTER_DELAY_SHIFT+6);  // MasterDelay
    
  // CounterCard GateDelays
  //    for(i=0; i<MAX_COUNTER_GATE;i++) {
  //	ptrElekStatus->CounterCard.Channel[i].GateDelay=elkReadData(ELK_COUNTER_DELAY_GATE+4*i);
  //	ptrElekStatus->CounterCard.Channel[i].GateWidth=elkReadData(ELK_COUNTER_DELAY_GATE+4*i+2);	
  //    }
  printf("\n\r");
  //	refresh();    
} /*PrintElekStatus*/


/*
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
  printf("[D] DATASET DATA\t[V] VALVE DATA\n\r");
  printf("[T] TIME DATA   \t[S] SHOW ALL        \t[R] RESET ALL\n\r");
  printf("\n\r*** PRESS [H] FOR HELP DURING DATA DUMPING! ***\n\r");
};


*/

int main() 
{
	
	extern int errno;
	struct elekStatusType ElekStatus;
	int i;                          // loop counter
	FILE *fp;
	int ret;
	char buf[GENERIC_BUF_LEN];
	char StatusFileName[MAX_FILENAME_LEN]; 
	char DataSetBuf[4000];
	
	int ElekStatusLen = sizeof(struct elekStatusType);
	
	#ifdef RUNONARM
	sprintf(buf,"This is elekConverter Version %3.2f ($Id: elekConverter.c,v 1.1 2005/09/07 09:32:50 rudolf Exp $) for ARM\nexpected StatusLen %d\n",VERSION,ElekStatusLen);
	#else
	sprintf(buf,"This is elekConverter Version %3.2f ($Id: elekConverter.c,v 1.1 2005/09/07 09:32:50 rudolf Exp $) for i386\nexpected StatusLen %d\n",VERSION,ElekStatusLen);
	#endif
	
	fp=fopen("./52421440.bin","r");
	
	if(fp==NULL)
	{
		printf("Error opening 52441310.bin for reading!\n\r");
		exit(-1);
	};
	
	while(1)
	{
		ret = fread(&DataSetBuf,ElekStatusLen,1,fp);
		if(ret!=1)
		{
			printf("Read failed!\n\r");
			break;
		}
		else
		{
		PrintElekStatus(&DataSetBuf, ElekStatusLen); 
		};
	};
  	exit(EXIT_SUCCESS);
}
