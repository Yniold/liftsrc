/*
 * $RCSfile: drivertester.h,v $ last changed on $Date: 2005/09/14 16:03:44 $ by $Author: rudolf $
 *
 * $Log: drivertester.h,v $
 * Revision 1.1  2005/09/14 16:03:44  rudolf
 * added driver test tool to CVS
 *
 *
 *
 */
#ifndef __drivertester_h__
#define __drivertester_h__

#define SERBUS_IOCSDEBUGON 		0x40047301
#define SERBUS_IOCSDEBUGOFF 	0x40047302
#define SERBUS_IOCTWRITEWORD	0x40047303
#define SERBUS_IOCHREADWORD		0x40047304

double CalculateVoltage(unsigned char ucIs5VoltRange, unsigned int uiADCCounts, double ScaleFactor);
long ReadADC24Channel(int iChannelNumber);

int fd;

#endif

