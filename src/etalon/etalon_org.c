/*
** talker.c -- a datagram "client" demo
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
#include <netdb.h>
#include <time.h>
#include <asm/msr.h>

#include "elekIO.h"

enum MsgTypeListEnum {               // the list of available Message Types
    MSG_TYPE_READ_DATA,
    MSG_TYPE_WRITE_DATA,

    MAX_MSG_TYPE }; 


struct ElekMessageType {
    uint64_t MsgID;                    // ID of Message
    uint64_t MsgTime;              // Time of Message
    uint16_t MsgType;                  // Message Type
    uint16_t Addr;                     // address of elektronik port
    uint16_t Value;                    // value to transmit or recieve depending on MsgType
    uint16_t Status;                   // status of message (error, success...)

}; /* ElekMessageType */

static 

int ReadCommand(uint16_t Addr) {
    uint64_t TSC;
    struct ElekMessageType Message;
    
    rdtscll(TSC);
    Message.MsgID=num++;
    Message.MsgTime=TSC;
    Message.MsgType=MSG_TYPE_READ_DATA;
    Message.Addr=0x0a300+Channel*2;
    Message.Value=0;
	
	if ((numbytes=sendto(sockfd, &Message, sizeof(struct ElekMessageType), 0,
			     (struct sockaddr *)&their_addr, sizeof(struct sockaddr))) == -1) {
	    perror("sendto");
	    exit(1);
	}
	
} /* ReadCommand */
	

int main(int argc, char *argv[])
{
    int sockfd;
    short Port;
    struct sockaddr_in their_addr; // connector's address information
    struct hostent *he;
    int numbytes;
    struct timespec RealTime;         // Real time clock
    struct timespec SleepTime;
    uint64_t TSC;
    char buf[256];
    long num;
    long maxcounts;
    int ret;
    int Channel;
    struct ElekMessageType Message;   // sample message


    if (argc != 6) {
        fprintf(stderr,"usage: talker hostname port message delay num\n");
        exit(1);
    }

    if ((he=gethostbyname(argv[1])) == NULL) {  // get the host info
        perror("gethostbyname");
        exit(1);
    }

    Port=UDP_ELEK_ETALON_INPORT;
    their_addr.sin_family = AF_INET;     // host byte order
    their_addr.sin_port = htons(Port); // short, network byte order
    their_addr.sin_addr = *((struct in_addr *)he->h_addr);
    memset(&(their_addr.sin_zero), '\0', 8);  // zero the rest of the struct

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }
    
    SleepTime.tv_sec=0;
    SleepTime.tv_nsec=(long)1000000*atol(argv[4]);
   
    printf("Sleeping for <%s> %ld msec\n",argv[4],SleepTime.tv_nsec/1000000);
    maxcounts=atoi(argv[5]);
    // clock_gettime(CLOCK_REALTIME,&RealTime);

    // init etalon drive



    num=0;
    while (num<maxcounts) {
//	printf("%d %d\n",num,maxcounts);
	rdtscll(TSC);
	sprintf(buf,"%lld",TSC);
	
	for (Channel=0; Channel<8; Channel++) {
	    Message.MsgID=num++;
	    Message.MsgTime=TSC;
	    Message.MsgType=MSG_TYPE_READ_DATA;
	    Message.Addr=0x0a300+Channel*2;
	    Message.Value=0;
	    
	    if ((numbytes=sendto(sockfd, &Message, sizeof(struct ElekMessageType), 0,
				 (struct sockaddr *)&their_addr, sizeof(struct sockaddr))) == -1) {
		perror("sendto");
		exit(1);
	    }
	} /* for Channel */
	ret=nanosleep(&SleepTime,NULL);
//	printf("woke due to %d ",ret);
//    if ((numbytes=sendto(sockfd, argv[3], strlen(argv[3]), 0,
//			 (struct sockaddr *)&their_addr, sizeof(struct sockaddr))) == -1) {
//       perror("sendto");
//   exit(1);
//}
	
//    clock_gettime(CLOCK_REALTIME,&RealTime);
//	printf("%lld :sent %d bytes to %s\n", TSC,numbytes, inet_ntoa(their_addr.sin_addr));
	putchar('.');
	
    } /* while */

    close(sockfd);

    return 0;
}
