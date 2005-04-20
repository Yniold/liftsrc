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

#include "../include/elekIOPorts.h"

#define DEBUGLEVEL 0

/*********************************************************************************************************/
/*                                                                                                       */
/* Function to init inbound UDP Sockets                                                                  */
/*                                                                                                       */
/*********************************************************************************************************/
int InitUDPInSocket(unsigned Port) {

    struct sockaddr_in my_addr;    // my address information
    int                fdSocket;
    int                addr_len;


    // open debug udp socket 
    if ((fdSocket = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }

    my_addr.sin_family = AF_INET;         // host byte order
    my_addr.sin_port = htons(Port);     // short, network byte order
    my_addr.sin_addr.s_addr = INADDR_ANY; // automatically fill with my IP
    memset(&(my_addr.sin_zero), '\0', 8); // zero the rest of the struct
    addr_len = sizeof(struct sockaddr);
    
    if (bind(fdSocket, (struct sockaddr *)&my_addr, addr_len) == -1) {
        perror("bind");
        exit(1);
    }

    return(fdSocket);

} /* InitUDPInSocket */

/*********************************************************************************************************/
/*                                                                                                       */
/* Function to init outbound UDP Sockets                                                                 */
/*                                                                                                       */
/*********************************************************************************************************/
int InitUDPOutSocket() {

    struct sockaddr_in my_addr;    // my address information
    int                fdSocket;

    // open debug udp socket 
    if ((fdSocket = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }
    return(fdSocket);

} /* InitUDPOutSocket */



/*********************************************************************************************************/
/*                                                                                                       */
/* Function to Send UDP Data, IP Adress given in Message Structure                                       */
/*                                                                                                       */
/*********************************************************************************************************/

int SendUDPData(struct MessagePortType *ptrMessagePort, unsigned nByte, void *msg) {

    struct sockaddr_in their_addr;
    int numbytes;

    their_addr.sin_family = AF_INET;     // host byte order
    their_addr.sin_port = htons(ptrMessagePort->PortNumber); // short, network byte order
    their_addr.sin_addr.s_addr = inet_addr(ptrMessagePort->IPAddr);
    memset(&(their_addr.sin_zero), '\0', 8);  // zero the rest of the struct
       
    if ((numbytes=sendto(ptrMessagePort->fdSocket, msg, nByte, 0,
		     (struct sockaddr *)&their_addr, sizeof(struct sockaddr))) == -1) {
        printf("problem with sendto MsgPort: %d %s ",ptrMessagePort->PortNumber,ptrMessagePort->PortName);
        perror("sendto");
        exit(1);
    }

#if DEBUGLEVEL>0
   printf("sent %d bytes to %s:%d\n", numbytes, inet_ntoa(their_addr.sin_addr),ptrMessagePort->PortNumber);
#endif

} /* SendUDPData */


/*********************************************************************************************************/
/*                                                                                                       */
/* Function to Send UDP Data, IP Adress explicit given in IPAddr                                         */
/*                                                                                                       */
/*********************************************************************************************************/

int SendUDPDataToIP(struct MessagePortType *ptrMessagePort, char *IPAddr, unsigned nByte, void *msg) {

    struct sockaddr_in their_addr;
    int numbytes;

    their_addr.sin_family = AF_INET;     // host byte order
    their_addr.sin_port = htons(ptrMessagePort->PortNumber); // short, network byte order
    their_addr.sin_addr.s_addr = inet_addr(IPAddr);
    memset(&(their_addr.sin_zero), '\0', 8);  // zero the rest of the struct
    
    if ((numbytes=sendto(ptrMessagePort->fdSocket, msg, nByte, 0,
		     (struct sockaddr *)&their_addr, sizeof(struct sockaddr))) == -1) {
        perror("sendto");
        exit(1);
    }

#if DEBUGLEVEL>0
   printf("sent %d bytes to %s:%d\n", numbytes, inet_ntoa(their_addr.sin_addr),ptrMessagePort->PortNumber);
#endif

} /* SendUDPDatatoIP */

/*********************************************************************************************************/
/*                                                                                                       */
/* Function to Send String to UDP port IP Adress given in Message Structure                              */
/*                                                                                                       */
/*********************************************************************************************************/

int SendUDPMsg(struct MessagePortType *ptrMessagePort, void *msg) {

    struct sockaddr_in their_addr;
    int numbytes;

    their_addr.sin_family = AF_INET;     // host byte order
    their_addr.sin_port = htons(ptrMessagePort->PortNumber); // short, network byte order
    their_addr.sin_addr.s_addr = inet_addr(ptrMessagePort->IPAddr);
    memset(&(their_addr.sin_zero), '\0', 8);  // zero the rest of the struct
    
    if ((numbytes=sendto(ptrMessagePort->fdSocket, msg, strlen(msg), 0,
		     (struct sockaddr *)&their_addr, sizeof(struct sockaddr))) == -1) {
        perror("sendto");
        exit(1);
    }
//   printf("sent %d bytes to %s\n", numbytes, inet_ntoa(their_addr.sin_addr));

} /* SendUDPMsg */

/*********************************************************************************************************/
/*                                                                                                       */
/* Function to recieve Data, it will wait until all data is recieved                                     */
/*                                                                                                       */
/*********************************************************************************************************/

int RecieveUDPData(struct MessagePortType *ptrMessagePort, unsigned nByte, void *msg) {

    struct sockaddr_in their_addr;
    int numbytes;
    int addr_len;

    addr_len = sizeof(struct sockaddr);

    if ((numbytes=recvfrom(ptrMessagePort->fdSocket, msg,nByte , MSG_WAITALL,
			   (struct sockaddr *)&their_addr, &addr_len)) == -1) {
	perror("recvfrom");
	return(-1);
    }
    
#if DEBUGLEVEL>0
   printf("recieve %d bytes to %s:%d\n", numbytes, inet_ntoa(their_addr.sin_addr),ptrMessagePort->PortNumber);
#endif

} /* RecieveUDPData */
