/***************************************************************************
   main.c  -  description
   -------------------
   begin                : Wed Oct 22 12:24:09 CEST 2003  by H.Harder
   email                : harder@mpch-mainz.mpg.de
***************************************************************************/

#define VERSION  "0.1"

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "instserver.h"
#include "../include/cfgopts.h"
#include "protos.h"


/***************************************************************************************************************/
/* some unness global variables */
/***************************************************************************************************************/

// list of accepted clients with max possible right of each client
struct ClientRightAscType ptrClientRightAscList[]= {
    {  "141.5.1.178","FF-","Hartwig"  },
    {  "141.5.1.183","FF-","Monica"  },
    {  "141.5.1.229","FF-","Markus"  },
    {  "141.5.1.196","FF-","LIFT"  },
    {  "127.0.0.1","FF-","localhost"  },
    
    {  NULL,NULL,NULL  }                       // keep NULL is marker for end of list
};


// list of device commands
struct InterprFktType ParseDeviceFkt[] = {
    {  "motor",         DoMotor,     TCP_CMD_DEV_MOTOR},
    {  "valve",         DoValve,     TCP_CMD_DEV_VALVE}

};




/***************************************************************************************************************/
/* Function which is called to execute the Motor Command
 * returns socketfd of the listening port on success, else -1
 */
/***************************************************************************************************************/
int DoMotor(struct ClientMessageType *ptrClientMessage) {

    int ret;

    printf("%d Motor : <%s>\n",ptrClientMessage->MessageNum,ptrClientMessage->Arg);

    return(ret);
} // DoMotor

/***************************************************************************************************************/
/* Function which is called to execute the Valve Commands
 * returns socketfd of the listening port on success, else -1
 */
/***************************************************************************************************************/
int DoValve(struct ClientMessageType *ptrClientMessage) {

    int ret;

    printf("%d Valve : <%s>\n",ptrClientMessage->MessageNum,ptrClientMessage->Arg);

    return(ret);

} // DoValve


/***************************************************************************************************************/
/* Function which is called by bsearch to compare two funtion names
 * 
 */
/***************************************************************************************************************/
int FindFkt( struct InterprFktType *ptr1, struct InterprFktType *ptr2 ) {
   return ( strcasecmp( ptr1->FName, ptr2->FName) );
} // FindFkt

/***************************************************************************************************************/
/* Parse MEssage, parse incoming message and generate the appropiate port call 
 * message format :
 * MsgID Device Subdev par
 */
/***************************************************************************************************************/

int ParseMsg(char *ptrLine,                                            // buffer of msg
	     unsigned nbytes,                                       // number of bytes in msg
	     struct ActiveClientStructType *ptrActiveClientStruct   // Active Client struct
    )  
{
    int j;
    int ret;
    char *ptrParse;
    char buf[MAX_MSG_LEN];
    char *ptrToken;
    struct ClientMessageType ClientMessage;
    struct InterprFktType FindDevice;
    struct InterprFktType *FoundDevice;

    if (!ptrLine) {                                               // we don't process NULL ptr  
	return(-1);
    } /* endif */
    
    
    strncpy(buf,ptrLine,MAX_MSG_LEN);                             // save the original string 

 /*    printf("<"); */
/*     for(j=0; j<nbytes; j++)   */
/*    	if (buf[j]>32) printf("%c",buf[j]); */
/*    	else printf("\\%02x",buf[j]); */
/*     printf(">\n"); */
    
    
//    strncpy(tokenbuf,ptrLine,MAX_MSG_LEN);
    
    if ('\0'==*buf) {                                            // we don't handle empty strings 
	return (0);   // successfull operation of NOP 
    } /* endif */
    
// terminate string at CRLF 13 10

    ptrParse=buf;
    while (*ptrParse!=13 && *ptrParse!=10 && *ptrParse ) ptrParse++;  
    // ptrParse is now on the first occurence of 0 || 13 || 10 
    *ptrParse='\0';                                             // terminate here
    // seperate the first token, should be message number 
    if (NULL!=(ptrParse=strtok_r(buf, MSG_DELIM, (char**) &ptrToken))) {
	// try to convert message number
	ClientMessage.MessageNum=(unsigned) atoi(ptrParse);
//	printf("Message Num : %d\n",ClientMessage.MessageNum);
	if (0==ClientMessage.MessageNum) {                       // got fooled, not valid
	    LogError("unvalid MessageNum");
	    return(FALSE);
	}
	// get next token which is the device name
	if (NULL!=(ptrParse=strtok_r(NULL, MSG_DELIM, (char**) &ptrToken))) { 
//	    printf("Device : %s\n",ptrParse);
	    if('\0'==*ptrParse) { // empty device name
		LogError("empty DeviceName");
		return(FALSE);
	    }
	    // lets identify the Device
	    strncpy(FindDevice.FName,ptrParse,MAX_FN_NAME_LEN);
	    if (NULL!=(FoundDevice=bsearch(&FindDevice, ParseDeviceFkt, TCP_CMD_COUNT, 
					   sizeof(struct InterprFktType), (func)FindFkt))) {
//		printf("Found Device %s\n",FoundDevice->FName);
		// prepare Message structure
		strncpy(ClientMessage.Arg, ptrToken, MAX_MSG_LEN);
		ret=(FoundDevice->func)(&ClientMessage);
		
	    } else {
		/* no device given */
		LogError("can't find Devicename");
		return(FALSE);	    
	    } /* endif */
	} else {
	    /* no device given */
	    LogError("no Devicename");
	    return(FALSE);	    
	} /* endif */
	
	
    } /* endif */

 /*    printf("<"); */
/*     while (ptrParse) { */
/* 	printf("%s.",ptrParse); */
/* 	ptrParse=strtok_r(NULL, MSG_DELIM, (char**) &ptrToken);		 */
/*     } // while */	
//    printf(">\n");
       
    return(TRUE);
} // ParseMsg






/***************************************************************************************************************/
/* Function to setup the listening socket
 * returns socketfd of the listening port on success, else -1
 */
/***************************************************************************************************************/
int InitTCPListener(int portnum) {
    
    struct sockaddr_in myaddr;      // server address
    int listener;     // listening socket descriptor
    int yes=1;        // for setsockopt() SO_REUSEADDR, below
    
    
    // create listener socket
    if ((listener = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }
    
    // reuse sockets fast
    if (setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
	perror("setsockopt");
	exit(EXIT_FAILURE);
    }
    
    // bind
    bzero(&myaddr, sizeof(myaddr));
    myaddr.sin_family = AF_INET;
    myaddr.sin_addr.s_addr = INADDR_ANY;
    myaddr.sin_port = htons(portnum);
    if (bind(listener, (struct sockaddr *)&myaddr, sizeof(myaddr)) == -1) {
        perror("bind");
        exit(EXIT_FAILURE);
    }
    
    // listen
    if (listen(listener, 10) == -1) {
        perror("listen");
        exit(EXIT_FAILURE);
    }            
    
    return(listener);
    
} // InitTCPListen

/***************************************************************************************************************/
/* translates the ascii IP adress into the numeric version for speedy checking */
/***************************************************************************************************************/
void InitClientAccessList(struct ClientRightAscType *ptrClientRightAscList, 
			  struct ClientListType **pptrClientRightList) {
    
    
    struct ClientRightAscType *AsciiList;
    struct ClientListType *BinList;
    
    // assign some memory
    (*pptrClientRightList)=(struct ClientListType*) malloc(
	sizeof(struct ClientListType)*
	sizeof(ptrClientRightAscList)/
	sizeof(struct ClientRightAscType));
    if (!(*pptrClientRightList)) {
	perror("malloc rightlist");
	exit(EXIT_FAILURE);
    }
    
    AsciiList=ptrClientRightAscList;
    BinList=*pptrClientRightList;
    while (AsciiList->ClientIP) {
     	BinList->ClientID=0;
	BinList->ClientAddr.sin_addr.s_addr=htonl(inet_network(AsciiList->ClientIP));
	BinList->ClientRights.SetAll=1;	
	BinList->ClientRights.SetValve=1;
	BinList->ClientRights.SetLaser=1;
	strncpy(BinList->ClientName,AsciiList->ClientName,MAX_CLIENT_NAME);
	BinList++; AsciiList++;
    }
    BinList->ClientID=0;
    BinList->ClientAddr.sin_addr.s_addr=0;
    BinList->ClientRights.SetAll=0;	
    BinList->ClientRights.SetValve=0;
    BinList->ClientRights.SetLaser=0;
    strncpy(BinList->ClientName,"unvalid",MAX_CLIENT_NAME);
    
} //InitClientAccessList

/***************************************************************************************************************/
/* translates the ascii IP adress into the numeric version for speedy checking */
/***************************************************************************************************************/
void ReleaseClientAccessList(struct ClientListType *ptrClientRightList) {
    
    free(ptrClientRightList);
    
}



/***************************************************************************************************************/
/* searches remoteAdr in ClientRightList and returns pointer from ClientRightlist element or NULL if not present*/
/***************************************************************************************************************/
struct ClientListType *ValidateAddress(struct sockaddr_in *ptrRemoteAddr, 
				       struct ClientListType *ptrClientRightList) {
    
    struct ClientListType *ptrClient;
    
    ptrClient=ptrClientRightList;
    // walk through list until we find it or end of list (NULL) 
    while ( (ptrClient->ClientAddr.sin_addr.s_addr!=0) &   
	    (ptrClient->ClientAddr.sin_addr.s_addr!=ptrRemoteAddr->sin_addr.s_addr) ) {
	ptrClient++;
    }
    if (ptrClient->ClientAddr.sin_addr.s_addr)
	return(ptrClient);
    else
	return(NULL);
    
} //ValidateAddress


// C++ does has it advantages !!!

/***************************************************************************************************************/
/* Init Active Client List                                                                                     */
/*                                                                                                             */
/***************************************************************************************************************/

int InitActiveClientStruct(struct ActiveClientStructType *ActiveClientStruct) {
    
    int ret=1;
    
// allocate memory for active client array
    ActiveClientStruct->ptrActiveClientArray=(struct ClientListType*) malloc(MAX_CLIENT_NUMBER * 
									     sizeof(struct ClientListType));
    if (!ActiveClientStruct->ptrActiveClientArray) {
	perror("MemAlloc for ActiveClientArray");
	ret=0;
    }    
// mem for Client pointer list
    ActiveClientStruct->ptrActiveClientList=(struct ClientListType*) malloc(MAX_CLIENT_NUMBER * 
									    sizeof(struct ClientListType*));
    if (!ActiveClientStruct->ptrActiveClientList) {
	perror("MemAlloc for ActiveClientList");
	ret=0;
    }
    
    ActiveClientStruct->ptrComBuf=(char*) malloc(TCP_CLIENT_BUFSIZE);   
    if (!ActiveClientStruct->ptrComBuf) {
	perror("MemAlloc for ComBuf");
	ret=0;
    }
    
    ActiveClientStruct->NumActiveClient=0;
    return (ret);
} // InitActiveClientList


/***************************************************************************************************************/
/* release Active Client List                                                                                  */
/* to free all allocated memory                                                                                */
/***************************************************************************************************************/
void ReleaseActiveClientStruct(struct ActiveClientStructType *ActiveClientStruct) {
    
    free(ActiveClientStruct->ptrActiveClientArray);
    free(ActiveClientStruct->ptrActiveClientList);
    free(ActiveClientStruct->ptrComBuf);
    
    
} // releaseActiveClientStruct

/***************************************************************************************************************/
/* adds a Client to the Active Client List                                                                     */
/* ListNumber=(pointer to Active Client List, new Candidate, fd from socket                                    */
/***************************************************************************************************************/
int AddClientToList(struct ActiveClientStructType *ptrActiveClientStruct, 
		    struct ClientListType *ptrNewClient,
		    int newfd) {
    
    struct ClientListType *ptrActiveClient;
    int i;
    int ret;
    
    // first check if we have a space left
    if (ptrActiveClientStruct->NumActiveClient<MAX_CLIENT_NUMBER) { // we do have a free slot
	ptrActiveClient=ptrActiveClientStruct->ptrActiveClientArray; // start searching at the beginning of the list
	i=0;
	while ( (i<MAX_CLIENT_NUMBER) & (ptrActiveClient->ClientID>0) ) {  // search for next free slot 
	    i++; ptrActiveClient++;
	}
	if (i<MAX_CLIENT_NUMBER) { // found empty slot
	    
	    memcpy(ptrActiveClient, ptrNewClient,sizeof(struct ClientListType));   // copy structure
	    ptrActiveClient->ClientID=newfd;                                       // assign ID
	    // ptrActiveClientStruct->ptrActiveClientList[i]=ptrActiveClient;         // store access pointer in array
	    ptrActiveClientStruct->NumActiveClient++;                              // increase number of connected clients
	    
	    ret=i;
	} else {
	    ret=TCP_ERR_CLIENT_LIST_FULL;	
	}
    } else {
	ret=TCP_ERR_CLIENT_LIST_FULL;	
    }
    return (ret);
    
} // AddClientToStruct

/***************************************************************************************************************/
/* removes a Client from the Active Client List                                                                */
/* error=(pointer to Active Client List, fd from closed  socket                                                */
/***************************************************************************************************************/
int RemoveClientFromList(struct ActiveClientStructType *ptrActiveClientStruct, 
			 int fd) {
    
    struct ClientListType *ptrActiveClient;
    unsigned i;
    int ret;
    
    ptrActiveClient=ptrActiveClientStruct->ptrActiveClientArray;   // start searching at the beginning of the list
    i=0;
    while ( (i<MAX_CLIENT_NUMBER) & (ptrActiveClient->ClientID!=fd) ) {
	i++; ptrActiveClient++;
    }
    if (i<MAX_CLIENT_NUMBER) { // found client in list	
	ptrActiveClient->ClientID=0;                                       // release ID
	ptrActiveClientStruct->NumActiveClient--;                          // decrease number of connected clients
	ret=i;                                                             // return number found
    } else {
	ret=TCP_ERR_CLIENT_UNKNOWN;	
    }
    
    return (ret);
    
} // RemoveClientFromStruct





/***************************************************************************************************************/
/* MAIN */
/***************************************************************************************************************/

int main(int argc, char *argv[])
{
    
    fd_set master;                  // master file descriptor list
    fd_set read_fds;                // temp file descriptor list for select()
    struct sockaddr_in remoteaddr;  // client address
    unsigned ServerPort;            // portnumber of server 
    int fdmax,fdmax_used;           // maximum file descriptor number and helper to find new max 
    int listener;                   // listening socket descriptor
    int newfd;                      // newly accept()ed socket descriptor
    char buf[256];                  // buffer for client data
    int nbytes;
    int addrlen;
    int i, j;
    int NewClient;                  // NewClient ID
    struct timeval timeout;         // timeout 
    int ret; 
    struct ClientListType *ptrNewClient;              // ptr to new & Active Client;
    struct ClientListType *ptrClientRightList;                          // array which contains rights of reg.Clients      
    struct ActiveClientStructType ActiveClientStruct;                       // struct which contains array of active cl.
    
    
    // some command line parsing
    
    if (argc>1) {  // first argument might be a port assignment
	if (*argv[1]=='?') { // ask for help
	    printf("Usage : %s PortNum\n",argv[0]);
	    exit(EXIT_SUCCESS);
	} else {
	    ServerPort=atol(argv[1]);
	}
    } else {
	ServerPort=TCP_CLIENT_PORT;
    }
    
    //  Init Client Access list
    InitClientAccessList(ptrClientRightAscList, &ptrClientRightList);
    
    // Init Active Client List
    InitActiveClientStruct(&ActiveClientStruct);
    
    
    printf("Setup instrument server version %s on port %d ...",VERSION, ServerPort);
    listener=InitTCPListener(ServerPort);
    if (listener<1) {
	perror("TCPListener");
    }
    printf("success\n");
    
    // setup master fd
    FD_ZERO(&master);              // clear the master and temp sets
    FD_ZERO(&read_fds);
    FD_SET(listener, &master);     // add the listener to the master set
    
    // keep track of the biggest file descriptor
    
    fdmax = listener; // so far, it's this one

    for(;;) { // main loop

        // setup fd's timeout and wait for incoming request
        read_fds = master;
        timeout.tv_sec=TCP_CLIENT_WDG;
        timeout.tv_usec=0;
        ret=select(fdmax+1, &read_fds, NULL, NULL, &timeout);
        
        if (ret == -1) { // select error we dare not to handle
            perror("select");
            exit(EXIT_FAILURE);
        }
	
        if (ret) { // lets see what we got
	    for(i = 0; i <= fdmax; i++) {
		if (FD_ISSET(i, &read_fds)) {                                // new msg on i. socket ...
		    if (i == listener) {                                     // listener -> new connection request
			addrlen = sizeof(remoteaddr);			
			if ((newfd = accept(listener, (struct sockaddr *)&remoteaddr,           // first we have to accept
					    &addrlen)) == -1) {
			    perror("accept");
			} else {
			    //Valid client ?
			    if ( ((ptrNewClient=ValidateAddress(&remoteaddr,ptrClientRightList))!=NULL) ) {
				//add new client to active list
				if ((NewClient=AddClientToList(&ActiveClientStruct,ptrNewClient,newfd))<0) { // couldn't add Client to list;
				    printf("no available sockets for fd %d\n",newfd);
				    close(newfd); // close connection
				} else {          // give a positive msg to the log and keep track of maxfd
				    printf("accepted new connection from %s\n",
					   ActiveClientStruct.ptrActiveClientArray[NewClient].ClientName);
				    FD_SET(newfd, &master); // add to master set  
				    if (newfd > fdmax) {    // keep trackof the maximum
					fdmax = newfd;
				    }
				} // endif AddClient
			    } else {   // couldn't find applicant in internal rights list, kick him out
				close(newfd);
				printf("deny access from %s closing..\n",inet_ntoa(remoteaddr.sin_addr));
			    } // endif ValidateAddress
			    
			} // endif accept
		    } else {                                                // handle data from a clients
			if ((nbytes = recv(i, buf, sizeof(buf), 0)) <= 0) { // got error or connection closed by client
			    if (nbytes == 0) {                              // connection closed
				ret=RemoveClientFromList(&ActiveClientStruct,i);
				printf("instrumentserver: %s on socket %d hung up\n", 
				       ActiveClientStruct.ptrActiveClientArray[ret].ClientName, i);
			    } else {
				perror("recv");
			    } // EndIf recv
			    close(i); // bye!
			    FD_CLR(i, &master); // remove from master set
			} else { // we got some data from a client
			    
			    // display whats in decimal for first
			    
			    ParseMsg(buf,nbytes,&ActiveClientStruct);
			    
			    for(j = 0; j <= fdmax; j++) {
				// send to everyone
				if (FD_ISSET(j, &master)) {
				    // except the listener and sender
				    if (j != listener && j != i) {
					if (send(j, buf, nbytes, 0) == -1) {
					    perror("send");
					} //endif send					
				    }  // sender and listener
				} // someone in master list
			    } //endfor all clients
			}
		    }
		}
	    }
	} else {
	    for (i = 0; i <= fdmax; i++) {
		if (FD_ISSET(i,&master)) {
		    printf("+");
		    fdmax_used=i;                               // find out what the highest used fd is
		} else {
		    printf(".");
		}
	    }
	    printf("\n");
	    fdmax=fdmax_used;                                  // garbage collection on max fdnum
	}
    } // for main loop
    
    
// release used memory
    
    ReleaseActiveClientStruct(&ActiveClientStruct);            // relase memory of ActiveClientStruct
    ReleaseClientAccessList(ptrClientRightList);               // relase memory of ClientRightList
    
    return EXIT_SUCCESS;
}


