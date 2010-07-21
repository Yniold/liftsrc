/*
* $RCSfile: debugMon.c,v $ last changed on $Date: 2006/09/04 10:17:20 $ by $Author: rudolf $
*
* $Log: debugMon.c,v $
* Revision 1.4  2006/09/04 10:17:20  rudolf
* fixed compiler warning for GCC 4.03
*
* Revision 1.3  2005/04/22 13:13:58  rudolf
* added timestamp to debugMon (local time)
*
* Revision 1.2  2005/04/21 13:48:32  rudolf
* more work on conditional compile, added revision history
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
#include <asm/msr.h>
#endif

#include "../include/elekIOPorts.h"

#define MAXBUFLEN 256


int main(int argc, char *argv[])
{
    int sockfd;
    short Port;
    struct sockaddr_in my_addr;    // my address information
    struct sockaddr_in their_addr; // connector's address information
    struct timespec RealTime;         // Real time clock
    int numbytes;
    socklen_t addr_len;
    char buf[MAXBUFLEN];
    char timebuf[MAXBUFLEN];
    uint64_t TSC;

    struct tm DateNow;           // for timestamps
    time_t SecondsNow;

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }

    Port=UDP_ELEK_DEBUG_OUTPORT;
    my_addr.sin_family = AF_INET;         // host byte order
    my_addr.sin_port = htons(Port);     // short, network byte order
    my_addr.sin_addr.s_addr = INADDR_ANY; // automatically fill with my IP
    memset(&(my_addr.sin_zero), '\0', 8); // zero the rest of the struct
    addr_len = sizeof(struct sockaddr);

    if (bind(sockfd, (struct sockaddr *)&my_addr, addr_len) == -1) {
        perror("bind");
        exit(1);
    }

    while (1)
    {
//	printf("wait for data ....\n");
      if ((numbytes=recvfrom(sockfd, buf, MAXBUFLEN-1 , 0,
          (struct sockaddr *)&their_addr, &addr_len)) == -1)
      {
	      perror("recvfrom");
	      exit(1);
      }
//	rdtscll(TSC);
//	printf("%lld got packet from %s\n",TSC,inet_ntoa(their_addr.sin_addr));
//	clock_gettime(CLOCK_REALTIME,&RealTime);
//	printf("%ld %ld got packet from %s\n",RealTime.tv_sec,RealTime.tv_nsec,inet_ntoa(their_addr.sin_addr));
//	printf("packet is %d bytes long\n",numbytes);
	   buf[numbytes] = '\0';
      time(&SecondsNow);
	   localtime_r(&SecondsNow,&DateNow);

      sprintf(timebuf,"[%02d:%02d:%02d]", DateNow.tm_hour, DateNow.tm_min, DateNow.tm_sec);

	   printf("%s %s\n",timebuf,buf);
   }
   close(sockfd);
   return 0;
}
