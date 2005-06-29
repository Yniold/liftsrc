/*
 * $RCSfile: elekStatus.c,v $ last changed on $Date: 2005-06-29 12:46:18 $ by $Author: rudolf $
 *
 * $Log: elekStatus.c,v $
 * Revision 1.19  2005-06-29 12:46:18  rudolf
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
#include <asm/msr.h>
#endif

#include <signal.h>
#include <errno.h>
#include <termios.h>

#include "elekStatus.h"
#include "../include/elekGeneral.h"
#include "../include/elekIO.h"
#include "../include/elekIOPorts.h"

#define STATUSFILE_RING_LEN 500

#define DEBUGLEVEL 1

enum InPortListEnum {  // this list has to be coherent with MessageInPortList
  ELEK_STATUS_REQ_IN,
  ELEK_ELEKIO_IN,
  MAX_MESSAGE_INPORTS }; 

enum OutPortListEnum {  // this list has to be coherent with MessageOutPortList
  ELEK_STATUS_OUT,
  ELEK_DEBUG_OUT,
  ELEK_CLIENT_OUT,
  MAX_MESSAGE_OUTPORTS }; 


static struct MessagePortType MessageInPortList[MAX_MESSAGE_INPORTS]={   // order in list defines sequence of polling 
  /* Name, PortNo, ReversePortNo, fdSocket, MaxMessages, Direction */
  {"StatusReq",    UDP_ELEK_STATUS_REQ_INPORT,    UDP_ELEK_STATUS_REQ_OUTPORT, IP_LOCALHOST, -1, 1,  UDP_IN_PORT},
  { "ElekIOIn",UDP_ELEK_STATUS_STATUS_OUTPORT,                             -1, IP_LOCALHOST, -1, 1,  UDP_IN_PORT} // status inport from elekIOServ
};

static struct MessagePortType MessageOutPortList[MAX_MESSAGE_OUTPORTS]={                                    // order in list defines sequence of polling 
  /* Name, PortNo, ReversePortNo, fdSocket, MaxMessages, Direction */
  {"StatusReq",   UDP_ELEK_STATUS_REQ_OUTPORT, UDP_ELEK_STATUS_REQ_INPORT, IP_ELEK_SERVER,  -1, 0, UDP_OUT_PORT},
  {"DebugPort",   UDP_ELEK_DEBUG_OUTPORT,                              -1, IP_DEBUG_CLIENT, -1, 0, UDP_OUT_PORT},
  {"StatusClient",UDP_ELEK_STATUS_STATUS_OUTPORT,                      -1, IP_DEBUG_CLIENT, -1, 0, UDP_OUT_PORT} // status inport from elekIOServ
};

static long LastStatusNumber;


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
      localtime_r(&Seconds,&tmZeit);
		
      printf("%02d.%02d %02d:%02d:%02d.%03d :",tmZeit.tm_mon+1,tmZeit.tm_mday, 
	     tmZeit.tm_hour, tmZeit.tm_min, tmZeit.tm_sec, ptrElekStatus->TimeOfDayMaster.tv_usec/1000);
		
      printf("Time(S):");
      Seconds=ptrElekStatus->TimeOfDaySlave.tv_sec;
      localtime_r(&Seconds,&tmZeit);
		
      printf("%02d.%02d %02d:%02d:%02d.%03d :",tmZeit.tm_mon+1,tmZeit.tm_mday, 
	     tmZeit.tm_hour, tmZeit.tm_min, tmZeit.tm_sec, ptrElekStatus->TimeOfDaySlave.tv_usec/1000);
    };
	
  // ***************** ETALON DATA **************	
	
  if(uiGroupFlags & GROUP_ETALONDATA)
    {
      printf("Etalon:%ld(%d %d)/%ld %ld %d %4x",ptrElekStatus->EtalonData.Current.Position,
	     ptrElekStatus->EtalonData.Current.PositionWord.High,
	     ptrElekStatus->EtalonData.Current.PositionWord.Low,
	     ptrElekStatus->EtalonData.Encoder.Position,
	     ptrElekStatus->EtalonData.Index.Position,
	     ptrElekStatus->EtalonData.CurSpeed,
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
    for(i=0; i<MAX_COUNTER_CHANNEL;i++) {
      printf("%d ",ptrElekStatus->CounterCardSlave.Channel[i].Counts);
    } // for i
     
    

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
		
      // SLAVE
      // GPS Data
	
      printf("GPS(M) T:");
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


int WriteElekStatus(char *PathToRamDisk, char *FileName, struct elekStatusType *ptrElekStatus) 
{
	
  extern struct MessagePortType MessageOutPortList[];
  extern struct MessagePortType MessageInPortList[];
	
  extern long LastStatusNumber;
	
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
  localtime_r(&Seconds,&tmZeit);
	
  strncpy(buf,FileName,GENERIC_BUF_LEN);
  strcat(buf,".bin");
	
  if ((fp=fopen(FileName,"a"))==NULL)
    {
      sprintf(buf,"ElekStatus: can't open %s",FileName);
      SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],buf);
    }
  else
    {
      // write data. may return with 1 even if disk is full.
      ret=fwrite(ptrElekStatus,sizeof (struct elekStatusType),1,fp);
      if (ret!=1)
	{
	  char* pErrorMessage = strerror(errno);
	  sprintf(buf,"ElekStatus: DATA NOT WRITTEN, fwrite() returned with error %d: ",ret);
	  strcat(buf,pErrorMessage);
	  SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],buf);
	};
		
      // flush buffer to check if disk is full an to prevent data loss
      ret=fflush(fp);
      if (ret == EOF)
	{
	  char* pErrorMessage = strerror(errno);
	  sprintf(buf,"ElekStatus: DATA NOT WRITTEN, fflush() returned with error %d: ",ret);
	  strcat(buf,pErrorMessage);
	  SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],buf);
	};
      fclose(fp);
    } // if fopen
	
  //    strncpy(buf,Path,GENERIC_BUF_LEN);
  strncpy(buf,PathToRamDisk,GENERIC_BUF_LEN);
  strcat(buf,"/status.bin");
	
  if ((fp=fopen(buf,"r+"))==NULL) 
    {
      SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],"ElekStatus: status.bin does not exist");
      if ((fp=fopen(buf,"w+"))==NULL) 
	{
	  SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],"ElekStatus: can't create status.bin");
	  return (-1);
	}
    } /* if open */
  //go to the next entry position
  ret=fseek(fp,LastStatusNumber*sizeof (struct elekStatusType),SEEK_SET);
  //    sprintf(buf,"ElekStatus: write Status to %d,%d %d %ld",LastStatusNumber,
  //	    LastStatusNumber*sizeof (struct elekStatusType),ret,ftell(fp) );
  // SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],buf);      
  ret=fwrite(ptrElekStatus,sizeof (struct elekStatusType),1,fp);
	
  if (ret!=1) 
    {
      SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],"ElekStatus: can't write status.bin");
    }
  LastStatusNumber=(LastStatusNumber+1) % STATUSFILE_RING_LEN;	    
  fclose(fp);
	
  strncpy(buf,PathToRamDisk,GENERIC_BUF_LEN);
  strcat(buf,"/chPMT.txt");
  if ((fp=fopen(buf,"w"))==NULL) 
    {
      SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],"ElekStatus: can't open chPMT.txt");
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
	    } // for channel
	  fprintf(fp,"\n");
	} // for i
      fclose(fp);
    } // if fopen
} /*WriteElekStatus*/


void GenerateFileName(char *Path, char *FileName, struct tm *ReqTime) {
  struct tm DateNow;
  time_t SecondsNow;

  if (ReqTime==NULL) {

    time(&SecondsNow);
    localtime_r(&SecondsNow,&DateNow);
    ReqTime=&DateNow;
  }
    
  sprintf(FileName,"%s/%1d%03d%02d%02d.bin",
	  Path,
	  ReqTime->tm_year-100,
	  ReqTime->tm_yday,
	  ReqTime->tm_hour,
	  10*(int)(ReqTime->tm_min/10));

} /* Generate Filename */

/******************************************************************************************/
/* search Status file for last entry and set global variable accordingly                  */
/******************************************************************************************/


int InitStatusFile(char *Path) {

  extern struct MessagePortType MessageOutPortList[];
  extern struct MessagePortType MessageInPortList[];
  extern long LastStatusNumber;


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
  if ((fp=fopen(buf,"r"))==NULL) {
    // file does not exist yet, so we set LastStatusNumber to 0
    LastStatusNumber=0;
    return (0);
  }
 
  // find out what the last entry is
  LastStatusNumber=0;
  RecordNo=0;
  LastTime.tv_sec=0;
  LastTime.tv_usec=0;
   
  while (!feof(fp)) {
      
    ret=fread(&elekStatus,sizeof (struct elekStatusType),1,fp);
    //      sprintf(buf,"ElekStatus InitStatusFile: %d %d. read %ld compare %ld\n",
    //	      feof(fp),RecordNo,
    //	      elekStatus.TimeOfDay.tv_sec,
    //	      LastTime.tv_sec);
    //      SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],buf);
	    
    if ((LastTime.tv_sec<elekStatus.TimeOfDayMaster.tv_sec) && 
	(LastTime.tv_sec<elekStatus.TimeOfDayMaster.tv_sec)) {
	
      LastStatusNumber=RecordNo;
      LastTime=elekStatus.TimeOfDayMaster;
    }/* if time */
      
    RecordNo++;
  } /* while feof */
    
  fclose(fp);
    


} /* Init Status File */



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




int main() 
{

  extern int errno;
  extern struct MessagePortType MessageOutPortList[];
  extern struct MessagePortType MessageInPortList[];
              
  struct elekStatusType ElekStatus;
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
  int    addr_len, ElekStatus_len, numbytes;
  bool   EndOfSession;  

  char buf[GENERIC_BUF_LEN];

  char StatusFileName[MAX_FILENAME_LEN]; 
    
  if(cbreak(STDIN_FILENO) == -1) 
    {
      printf("Fehler bei der Funktion cbreak ... \n");
      exit(EXIT_FAILURE);
    }

  // setup master fd
  FD_ZERO(&fdsMaster);              // clear the master and temp sets
  FD_ZERO(&fdsSelect);
    
  // init inports
  for (MessagePort=0; MessagePort<MAX_MESSAGE_INPORTS;MessagePort++) {
	
    printf("opening IN Port %s on Port %d\n",
	   MessageInPortList[MessagePort].PortName,
	   MessageInPortList[MessagePort].PortNumber);
	
    MessageInPortList[MessagePort].fdSocket=InitUDPInSocket(MessageInPortList[MessagePort].PortNumber);
	
    FD_SET(MessageInPortList[MessagePort].fdSocket, &fdsMaster);     // add the manual port to the master set
    fdMax=MessageInPortList[MessagePort].fdSocket;                   // the last one will give the max number
  } /* for MessageInPort */

    // init outports
  for (MessagePort=0; MessagePort<MAX_MESSAGE_OUTPORTS;MessagePort++) {
    printf("opening OUT Port %s on Port %d, Destination IP: %s\n",
	   MessageOutPortList[MessagePort].PortName,
	   MessageOutPortList[MessagePort].PortNumber,
	   MessageOutPortList[MessagePort].IPAddr);
			 
    MessageOutPortList[MessagePort].fdSocket=InitUDPOutSocket(MessageOutPortList[MessagePort].PortNumber);
	
  } /* for MessageOutPort */

  addr_len = sizeof(struct sockaddr);
  ElekStatus_len=sizeof(struct elekStatusType);
    
  //    refresh();
#ifdef RUNONARM
  sprintf(buf,"This is elekStatus Version %3.2f ($Id: elekStatus.c,v 1.19 2005-06-29 12:46:18 rudolf Exp $) for ARM\nexpected StatusLen %d\n",VERSION,ElekStatus_len);
#else
  sprintf(buf,"This is elekStatus Version %3.2f ($Id: elekStatus.c,v 1.19 2005-06-29 12:46:18 rudolf Exp $) for i386\nexpected StatusLen %d\n",VERSION,ElekStatus_len);
#endif

  SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],buf);
	
  ShowHelp();
  //    GenerateFileName(DATAPATH,StatusFileName,NULL);

  // Check Status ringfile buffer and initialize the pointer to the appropriate position
  InitStatusFile(RAMDISKPATH);
    
  EndOfSession=FALSE;
  while (!EndOfSession) {
    //        printf("wait for data ....\n");
	
    fdsSelect=fdsMaster;        
    timeout.tv_sec= UDP_SERVER_TIMEOUT;
    timeout.tv_usec=0;
    ret=select(fdMax+1, &fdsSelect, NULL, NULL, &timeout);
        
    //	printf("ret %d\n",ret);
    if (ret ==-1 ) { // select error
      perror("select");
      SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],"Problem with select");
		
    } else if (ret>0) {
      //	    printf("woke up...");
	    
      for (MessagePort=0; MessagePort<MAX_MESSAGE_INPORTS;MessagePort++) { 
		
	if (FD_ISSET(MessageInPortList[MessagePort].fdSocket,&fdsSelect)) {   // new msg on fdNum. socket ... 
		    
	  switch (MessagePort) {
	  case ELEK_ELEKIO_IN:
	    if ((numbytes=recvfrom(MessageInPortList[MessagePort].fdSocket, 
				   &ElekStatus,ElekStatus_len , MSG_WAITALL,
				   (struct sockaddr *)&their_addr, &addr_len)) == -1) {
	      perror("recvfrom");
	      SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],"elekStatus : Problem with recieve");
	    }
	    StatusCount++;
	    EvaluateKeyboard();
	    if ((StatusCount % 5)==0) { 
	      PrintElekStatus(&ElekStatus, numbytes); 
	    }

	    GenerateFileName(DATAPATH,StatusFileName,NULL);

	    if (ElekStatus.InstrumentFlags.StatusSave)
	      WriteElekStatus(RAMDISKPATH, StatusFileName,&ElekStatus);
	    else SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],"elekStatus : DATA NOT STORED !!!");

	    // Send Statusdata to other interested clients
			    
	    // SendUDPDataToIP(&MessageOutPortList[ELEK_CLIENT_OUT],"141.5.1.178",ElekStatus_len,&ElekStatus);
	    // SendUDPDataToIP(&MessageOutPortList[ELEK_CLIENT_OUT],"10.111.111.10",ElekStatus_len,&ElekStatus);
	    //			    SendUDPDataToIP(&MessageOutPortList[ELEK_CLIENT_OUT],"172.31.178.24",ElekStatus_len,&ElekStatus);
	    //			    SendUDPDataToIP(&MessageOutPortList[ELEK_CLIENT_OUT],"172.31.178.25",ElekStatus_len,&ElekStatus);
			    
	    break;
	  case ELEK_STATUS_REQ_IN:
	    break;
	  default :
	    SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],"Unknown MessagPort");
	    break;
	  } /* switch */
		    
	} /* if fd_isset */		
      } /* for MessagePort */
		
    } else { /* ret==0*/
      printf("timeout...\n");
      SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],"elekStatus : TimeOut");	    
    }	
  } /* while */
    

  // close all in bound sockets
  for (MessagePort=0; MessagePort<MAX_MESSAGE_INPORTS;MessagePort++) {	
    close(MessageInPortList[MessagePort].fdSocket);
  } /*for MessagePort */ 

  // close all out bound sockets
  for (MessagePort=0; MessagePort<MAX_MESSAGE_OUTPORTS;MessagePort++) {	
    close(MessageOutPortList[MessagePort].fdSocket);
  } /*for MessagePort */
  /*Alten Terminal-Modus wiederherstellen*/
  restoreinput();
  exit(EXIT_SUCCESS);
}
