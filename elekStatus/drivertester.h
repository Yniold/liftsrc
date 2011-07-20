#ifndef __drivertester_h__
#define __drivertester_h__

#define SERBUS_IOCSDEBUGON 		0x40047301
#define SERBUS_IOCSDEBUGOFF 	0x40047302
#define SERBUS_IOCTWRITEWORD	0x40047303
#define SERBUS_IOCHREADWORD		0x40047304

double CalculateVoltage(unsigned char ucIs5VoltRange, unsigned int uiADCCounts, double ScaleFactor);

#endif

