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
#include <asm/msr.h>
#include <signal.h>
#include <errno.h>


#include "../include/elekGeneral.h"
#include "../include/elekIO.h"
#include "../include/elekIOPorts.h"

#define STATUSFILE_RING_LEN 1000

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


void PrintElekStatus(struct elekStatusType *ptrElekStatus) {

    int i;
    struct tm tmZeit;
    time_t    Seconds;
    int Channel;
    int Card;

    Seconds=ptrElekStatus->TimeOfDay.tv_sec;
    localtime_r(&Seconds,&tmZeit);

    
    printf("%02d.%02d %02d:%02d:%02d.%03d :",tmZeit.tm_mon+1,tmZeit.tm_mday, 
	   tmZeit.tm_hour, tmZeit.tm_min, tmZeit.tm_sec, ptrElekStatus->TimeOfDay.tv_usec/1000);
    printf("E%ld(%d %d)/%ld %ld %d %4x",ptrElekStatus->EtalonData.Current.Position,
	   ptrElekStatus->EtalonData.Current.PositionWord.High,
	   ptrElekStatus->EtalonData.Current.PositionWord.Low,
	   ptrElekStatus->EtalonData.Encoder.Position,
	   ptrElekStatus->EtalonData.Index.Position,
	   ptrElekStatus->EtalonData.CurSpeed,
	   ptrElekStatus->EtalonData.Status);

    printf("cA");
    // Counter Card ADC Channel
    for (i=0; i<ADC_CHANNEL_COUNTER_CARD; i++) {
	printf("%4x ",ptrElekStatus->CounterCard.ADCData[i]);   
    }

    printf("cD%4x ",ptrElekStatus->CounterCard.MasterDelay);

    for(i=0; i<MAX_COUNTER_CHANNEL;i++)
        printf("%4x ",ptrElekStatus->CounterCard.Channel[i].ShiftDelay);

    for(i=0; i<MAX_COUNTER_GATE;i++) {
	printf("%4x/%4x ",ptrElekStatus->CounterCard.Channel[i].GateDelay,
	       ptrElekStatus->CounterCard.Channel[i].GateWidth);
    }

    for(i=0; i<MAX_COUNTER_CHANNEL;i++) {
	printf("%d ",ptrElekStatus->CounterCard.Channel[i].Counts);
    }

    // normal ADC Card
    printf("eA");
    for (Card=0; Card<MAX_ADC_CARD; Card ++) {
	for (Channel=0;Channel<MAX_ADC_CHANNEL_PER_CARD; Channel++) {	    
	    printf("%4x ",ptrElekStatus->ADCCard[Card].ADCChannelData[Channel].ADCData);
	} /* for Channel */
    } /* for Card */    
  

    // temperature Sensor

    printf("T");
    printf("%d ",ptrElekStatus->TempSensCard[0].NumSensor);
    printf("%d ",ptrElekStatus->TempSensCard[0].NumErrCRC);
    printf("%d ",ptrElekStatus->TempSensCard[0].NumErrNoResponse);
    printf("%d ",ptrElekStatus->TempSensCard[0].NumMissed);
    //    printf("%x;",ptrElekStatus->TempSensCard[0].TempSensor[0].Word.WordTemp);

    for (i=0;i<MAX_TEMP_SENSOR;i++) {
      printf("%x %3.2f,",
	     ptrElekStatus->TempSensCard[0].TempSensor[i].Field.aROMCode[0],
	     ptrElekStatus->TempSensCard[0].TempSensor[i].Field.TempMain+
	     ptrElekStatus->TempSensCard[0].TempSensor[i].Field.TempFrac/16.0
	     );
      //	     ptrElekStatus->TempSensCard[0].TempSensor[i].Word.WordTemp);
    } 

    printf("\n");

    
    // CounterCard ShiftDelays
//    for(i=0; i<MAX_COUNTER_CHANNEL;i++)
//	ptrElekStatus->CounterCard.Channel[i].ShiftDelay=elkReadData(ELK_COUNTER_DELAY_SHIFT+2*i);
//    ptrElekStatus->CounterCard.MasterDelay=elkReadData(ELK_COUNTER_DELAY_SHIFT+6);  // MasterDelay
    
    // CounterCard GateDelays
//    for(i=0; i<MAX_COUNTER_GATE;i++) {
//	ptrElekStatus->CounterCard.Channel[i].GateDelay=elkReadData(ELK_COUNTER_DELAY_GATE+4*i);
//	ptrElekStatus->CounterCard.Channel[i].GateWidth=elkReadData(ELK_COUNTER_DELAY_GATE+4*i+2);	
//    }

    
} /*PrintElekStatus*/


int WriteElekStatus(char *Path, char *FileName, struct elekStatusType *ptrElekStatus) {
    
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

    Seconds=ptrElekStatus->TimeOfDay.tv_sec;
    localtime_r(&Seconds,&tmZeit);

    strncpy(buf,FileName,GENERIC_BUF_LEN);
    strcat(buf,".bin");    
    if ((fp=fopen(FileName,"a"))==NULL) {
	sprintf(buf,"ElekStatus: can't open %s",FileName);
	SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],buf);
	return (-1);
    } else {
	ret=fwrite(ptrElekStatus,sizeof (struct elekStatusType),1,fp);
	if (ret!=1) {
	    sprintf(buf,"ElekStatus: wrote %d\n",ret);
	    SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],"ElekStatus: can't write statusBin.dat");
	}	    
//	sprintf(buf,"ElekStatus: wrote %d\n",ret);
//	SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],buf);
	fclose(fp);
    } // if fopen
    

    //    strncpy(buf,Path,GENERIC_BUF_LEN);
    strncpy(buf,"/lift/ramdisk",GENERIC_BUF_LEN);
    strcat(buf,"/status.bin");
    if ((fp=fopen(buf,"r+"))==NULL) {
	SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],"ElekStatus: status.bin does not exist");
	if ((fp=fopen(buf,"w+"))==NULL) {
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
    if (ret!=1) {
      SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],"ElekStatus: can't write status.bin");
    }
    LastStatusNumber=(LastStatusNumber+1) % STATUSFILE_RING_LEN;	    
    fclose(fp);
    


    strncpy(buf,Path,GENERIC_BUF_LEN);
    strcat(buf,"/chPMT.txt");
    if ((fp=fopen(buf,"w"))==NULL) {
      SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],"ElekStatus: can't open chPMT.txt");
      return (-1);
    } else {
      for(i=0; i<MAX_COUNTER_TIMESLOT;i++) {
	fprintf(fp,"%3d ",i);
	for (Channel=0; Channel<MAX_COUNTER_CHANNEL; Channel++) {
	  fprintf(fp,"%4d ",ptrElekStatus->CounterCard.Channel[Channel].Data[i]);
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
	    
      if ((LastTime.tv_sec<elekStatus.TimeOfDay.tv_sec) && 
	  (LastTime.tv_sec<elekStatus.TimeOfDay.tv_sec)) {
	
	LastStatusNumber=RecordNo;
	LastTime=elekStatus.TimeOfDay;
      }/* if time */
      
      RecordNo++;
    } /* while feof */
    
    fclose(fp);
    


} /* Init Status File */



int main() 
{

    extern int errno;
    extern struct MessagePortType MessageOutPortList[];
    extern struct MessagePortType MessageInPortList[];
    
    long StatusCount=0;               
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
	printf("opening OUT Port %s on Port %d\n",
	       MessageOutPortList[MessagePort].PortName,MessageOutPortList[MessagePort].PortNumber);
	MessageOutPortList[MessagePort].fdSocket=InitUDPOutSocket(MessageOutPortList[MessagePort].PortNumber);
	
    } /* for MessageOutPort */

    addr_len = sizeof(struct sockaddr);
    ElekStatus_len=sizeof(struct elekStatusType);

    sprintf(buf,"This is elekStatus Version %3.2f\nexpected StatusLen %d\n",VERSION,ElekStatus_len);
    SendUDPMsg(&MessageOutPortList[ELEK_DEBUG_OUT],buf);

//    GenerateFileName(DATAPATH,StatusFileName,NULL);

    // Check Status ringfile buffer and initialize the pointer to the appropriate position
    InitStatusFile(DATAPATH);
    
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
			    if ((StatusCount % 5)==0) { 
				printf("StatusCount %ld nByte %d ", StatusCount,numbytes); 
				PrintElekStatus(&ElekStatus); 
			    }

			    GenerateFileName(DATAPATH,StatusFileName,NULL);

			    if (ElekStatus.InstrumentFlags.StatusSave) 
			      WriteElekStatus(DATAPATH, StatusFileName,&ElekStatus);
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

    exit(EXIT_SUCCESS);
}

