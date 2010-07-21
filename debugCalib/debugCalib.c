/*
* $RCSfile: debugCalib.c,v $ last changed on $Date: 2006/09/01 15:47:29 $ by $Author: rudolf $
*
* $Log: debugCalib.c,v $
* Revision 1.1  2006/09/01 15:47:29  rudolf
* added small testprogram based on debugMon to test new calibrator structure and UDP stuff
*
*
*
*
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>

#ifdef RUNONPC
# include <asm/msr.h>
#endif

#include "../include/elekIOPorts.h"
#include "../include/elekIO.h"

#define MAXBUFLEN 256
#define VERSION 0.1

int main(int argc, char *argv[])
{
   int Card;
   int Channel;
   int sockfd;
   short Port;
   struct sockaddr_in my_addr;    // my address information
   struct sockaddr_in their_addr; // connector's address information
   struct timespec RealTime;         // Real time clock
   int addr_len, numbytes;
   struct calibStatusType CalibStatus; // calibrator structure
   char timebuf[MAXBUFLEN];
   uint64_t TSC;

   struct tm DateNow;           // for timestamps
   time_t SecondsNow;

   if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
     {
        perror("socket");
        exit(1);
     }

   Port = UDP_ELEK_CALIB_DATA_INPORT;    // listen on data port
   my_addr.sin_family = AF_INET;         // host byte order
   my_addr.sin_port = htons(Port);     // short, network byte order
   my_addr.sin_addr.s_addr = INADDR_ANY; // automatically fill with my IP
   memset(&(my_addr.sin_zero), '\0', 8); // zero the rest of the struct
   addr_len = sizeof(struct sockaddr);

   if (bind(sockfd, (struct sockaddr *)&my_addr, addr_len) == -1)
     {
        perror("bind");
        exit(1);
     }
   printf("This is debugCalib Version %3.2f (CVS: $RCSfile: debugCalib.c,v $ $Revision: 1.1 $) for ARM\n",VERSION);
   printf("Listening on port %05d. Expected structure size :%04d bytes\n",Port,sizeof(CalibStatus));

   while (1)
     {
	if ((numbytes=recvfrom(sockfd,&CalibStatus, sizeof(CalibStatus) , 0,
			       (struct sockaddr *)&their_addr, &addr_len)) == -1)
	  {
	     perror("recvfrom");
	     exit(1);
	  }
	if(numbytes == sizeof(CalibStatus))
	  {
             printf("Ana(C):");
             for (Card=0; Card<MAX_ADC_CARD_CALIB; Card ++)
	       {
		  printf("Card#%d:",Card);
		  for (Channel=0;Channel<MAX_ADC_CHANNEL_PER_CARD; Channel++)
		    {
		       printf("%05d ",CalibStatus.ADCCardCalib[Card].ADCChannelData[Channel].ADCData);
		    }
		  /* for Channel */
	       }
	     /* for Card */

	     printf("Sensors found: %05d\n",CalibStatus.TempSensCardCalib[0].NumSensor);
	  }
	else
	  printf("Number of RX bytes %05d, wanted %05d bytes, ignoring data!\n",numbytes,sizeof(CalibStatus));
     }
   close(sockfd);
   return 0;
}
