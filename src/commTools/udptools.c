/*
* $RCSfile: udptools.c,v $ last changed on $Date: 2007-07-15 11:06:50 $ by $Author: martinez $
*
* $Log: udptools.c,v $
* Revision 1.9  2007-07-15 11:06:50  martinez
* receiveUDPData uses now Select and waits max UDP_SERVER_TIMEOUT sec til timeout
*
* Revision 1.8  2007-03-09 16:19:47  rudolf
* allow broadcast addresses for UDP_OUT
*
* Revision 1.7  2006-09-04 11:53:29  rudolf
* Fixed warnings for GCC 4.03, added newline and CVS revision info
*
* Revision 1.6  2005/06/27 19:39:11  rudolf
* disabled in udp debuglevels
*
* Revision 1.5  2005/06/22 13:17:14  rudolf
* Added PORTNAME in debug printfs -> makes life much easier
*
* Revision 1.4  2005/05/18 18:26:13  rudolf
* small fixes
*
* Revision 1.3  2005/04/21 13:48:01  rudolf
* more work on conditional compile, added revision history
*
*
*
*/

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
#include <errno.h>

#ifdef RUNONPC
# include <asm/msr.h>
#endif

#include "../include/elekIOPorts.h"

#define DEBUGLEVEL 0
#define DEBUGDEBUGMESSAGES 0

/*********************************************************************************************************/
/*                                                                                                       */
/* Function to init inbound UDP Sockets                                                                  */
/*                                                                                                       */
/*********************************************************************************************************/
int InitUDPInSocket(unsigned Port)
{

   struct sockaddr_in my_addr;    // my address information
   int                fdSocket;
   int                addr_len;

   // open debug udp socket
   if ((fdSocket = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
     {
        perror("socket");
        exit(1);
     }

   my_addr.sin_family = AF_INET;         // host byte order
   my_addr.sin_port = htons(Port);     // short, network byte order
   my_addr.sin_addr.s_addr = INADDR_ANY; // automatically fill with my IP
   memset(&(my_addr.sin_zero), '\0', 8); // zero the rest of the struct
   addr_len = sizeof(struct sockaddr);

   if (bind(fdSocket, (struct sockaddr *)&my_addr, addr_len) == -1)
     {
	perror("bind");
	exit(1);
     }

   return(fdSocket);

}
   /* InitUDPInSocket */

/*********************************************************************************************************/
/*                                                                                                       */
/* Function to init outbound UDP Sockets                                                                 */
/*                                                                                                       */
/*********************************************************************************************************/
int InitUDPOutSocket()
{
   const int iEnableOption = 1;

   struct sockaddr_in my_addr;    // my address information
   int                fdSocket;

   // open debug udp socket
   if ((fdSocket = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
     {
	perror("socket");
	exit(1);
     }
   // allow broadcast addresses
   if(setsockopt(fdSocket,SOL_SOCKET,SO_BROADCAST,&iEnableOption,sizeof(iEnableOption)) == -1)
     {
	perror("__function__: setsockopt() error:");
	exit(2);
     }

   return(fdSocket);

}
   /* InitUDPOutSocket */

/*********************************************************************************************************/
/*                                                                                                       */
/* Function to Send UDP Data, IP Adress given in Message Structure                                       */
/*                                                                                                       */
/*********************************************************************************************************/

int SendUDPData(struct MessagePortType *ptrMessagePort, unsigned nByte, void *msg)
{

   struct sockaddr_in their_addr;
   int numbytes;

   their_addr.sin_family = AF_INET;     // host byte order
   their_addr.sin_port = htons(ptrMessagePort->PortNumber); // short, network byte order
   their_addr.sin_addr.s_addr = inet_addr(ptrMessagePort->IPAddr);
   memset(&(their_addr.sin_zero), '\0', 8);  // zero the rest of the struct

   if ((numbytes=sendto(ptrMessagePort->fdSocket, msg, nByte, 0,
			(struct sockaddr *)&their_addr, sizeof(struct sockaddr_in))) == -1)
     {
     int lastError = errno;
	printf("\nproblem with sendto MsgPort: %d %s LINUX claims:",
	       ptrMessagePort->PortNumber,ptrMessagePort->PortName);
	printf("%s\n",strerror(lastError));
	perror("sendto");
	exit(1);
     }

#if DEBUGLEVEL>0
   printf("[%s] sent %d bytes to %s:%d\n", ptrMessagePort->PortName,numbytes, inet_ntoa(their_addr.sin_addr),ptrMessagePort->PortNumber);
#endif

}
   /* SendUDPData */

/*********************************************************************************************************/
/*                                                                                                       */
/* Function to Send UDP Data, IP Adress explicit given in IPAddr                                         */
/*                                                                                                       */
/*********************************************************************************************************/

int SendUDPDataToIP(struct MessagePortType *ptrMessagePort, char *IPAddr, unsigned nByte, void *msg)
{

   struct sockaddr_in their_addr;
   int numbytes;

   their_addr.sin_family = AF_INET;     // host byte order
   their_addr.sin_port = htons(ptrMessagePort->PortNumber); // short, network byte order
   their_addr.sin_addr.s_addr = inet_addr(IPAddr);
   memset(&(their_addr.sin_zero), '\0', 8);  // zero the rest of the struct

   if ((numbytes=sendto(ptrMessagePort->fdSocket, msg, nByte, 0,
			(struct sockaddr *)&their_addr, sizeof(struct sockaddr))) == -1)
     {
	perror("sendto");
	exit(1);
     }

#if DEBUGLEVEL>0
   printf("sent %d bytes to %s:%d\n", numbytes, inet_ntoa(their_addr.sin_addr),ptrMessagePort->PortNumber);
#endif

}
   /* SendUDPDatatoIP */

/*********************************************************************************************************/
/*                                                                                                       */
/* Function to Send String to UDP port IP Adress given in Message Structure                              */
/*                                                                                                       */
/*********************************************************************************************************/

int SendUDPMsg(struct MessagePortType *ptrMessagePort, void *msg)
{

   struct sockaddr_in their_addr;
   int numbytes;

   their_addr.sin_family = AF_INET;     // host byte order
   their_addr.sin_port = htons(ptrMessagePort->PortNumber); // short, network byte order
   their_addr.sin_addr.s_addr = inet_addr(ptrMessagePort->IPAddr);
   memset(&(their_addr.sin_zero), '\0', 8);  // zero the rest of the struct

   if (DEBUGDEBUGMESSAGES)
     printf("%s\n\r",(char*) msg);

   if ((numbytes=sendto(ptrMessagePort->fdSocket, msg, strlen(msg), 0,
			(struct sockaddr *)&their_addr, sizeof(struct sockaddr))) == -1)
     {
	perror("sendto");
	exit(1);
     }
   //   printf("sent %d bytes to %s\n", numbytes, inet_ntoa(their_addr.sin_addr));
   //
}
   /* SendUDPMsg */

/*********************************************************************************************************/
/*                                                                                                       */
/* Function to recieve Data, it will wait max 1 sec 
/*                                                                                                       */
/*********************************************************************************************************/

int RecieveUDPData(struct MessagePortType *ptrMessagePort, unsigned nByte, void *msg)
{

   struct sockaddr_in their_addr;
   int numbytes;
   socklen_t addr_len;
   fd_set fdsSelect;
   struct timeval select_timeout;         // timeout
   int fdMax;                             // max fd for select
   int ret;

   addr_len = sizeof(struct sockaddr);
   
   FD_ZERO(&fdsSelect);                            // prepare descriptor set and timeout for select
   select_timeout.tv_sec = UDP_SERVER_TIMEOUT;
   select_timeout.tv_usec = 0;
   FD_SET(ptrMessagePort->fdSocket,&fdsSelect);


   // printf("Start Select\n");
   fdMax=ptrMessagePort->fdSocket;
   ret=select(fdMax+1, &fdsSelect, NULL, NULL, &select_timeout);  // wait until incoming udp or Signal
   //   printf("select : %d\n",ret);
   
   if (ret>0) {
     if ((numbytes=recvfrom(ptrMessagePort->fdSocket, msg,nByte , 0,
			    (struct sockaddr *)&their_addr, &addr_len)) == -1)
       {
	 perror("ReceiveUDPData, recvfrom: ");
	 return(-2);
       }
     return(1);
   } else {     
     return(ret);
   }


#if DEBUGLEVEL>0
   printf("[%s] receive %d bytes from %s:%d\n", ptrMessagePort->PortName,numbytes, inet_ntoa(their_addr.sin_addr),ptrMessagePort->PortNumber);
#endif

}
   /* RecieveUDPData */


/*********************************************************************************************************/
/*                                                                                                       */
/* Function to recieve Data, it will wait until all data is recieved                                     */
/*                                                                                                       */
/*********************************************************************************************************/

int RecieveUDPDataWait4all(struct MessagePortType *ptrMessagePort, unsigned nByte, void *msg)
{

   struct sockaddr_in their_addr;
   int numbytes;
   socklen_t addr_len;

   addr_len = sizeof(struct sockaddr);

   if ((numbytes=recvfrom(ptrMessagePort->fdSocket, msg,nByte , MSG_WAITALL,
			  (struct sockaddr *)&their_addr, &addr_len)) == -1)
     {
	perror("ReceiveUDPData: ");
	return(-1);
     }

#if DEBUGLEVEL>0
   printf("[%s] receive %d bytes from %s:%d\n", ptrMessagePort->PortName,numbytes, inet_ntoa(their_addr.sin_addr),ptrMessagePort->PortNumber);
#endif

}
   /* RecieveUDPData */
