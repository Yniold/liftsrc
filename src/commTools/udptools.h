/*
* $RCSfile: udptools.h,v $ last changed on $Date: 2007-03-04 13:42:57 $ by $Author: rudolf $
*
* $Log: udptools.h,v $
* Revision 1.1  2007-03-04 13:42:57  rudolf
* added headerfile
*
*
*
*
*/
#ifndef UDPTOOLS_H
#define UDPTOOLS_H
#include "../include/elekIOPorts.h"

extern int SendUDPMsg(struct MessagePortType *ptrMessagePort, void *msg);
extern int InitUDPInSocket(unsigned Port);

#endif
