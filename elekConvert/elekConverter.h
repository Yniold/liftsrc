/*
* $RCSfile: elekConverter.h,v $ last changed on $Date: 2005/09/07 09:32:50 $ by $Author: rudolf $
* $Log: elekConverter.h,v $
* Revision 1.1  2005/09/07 09:32:50  rudolf
* added a small tool for reading a binary dataset. Based on elekStatus
*
*
*
*/

#ifndef ELEKCONVERTER_H
#define ELEKCONVERTER_H

#define GROUP_DATASETDATA	(1<< 0)
#define GROUP_ADCDATA		(1<< 1)
#define GROUP_TIMEDATA		(1<< 2)
#define GROUP_CCDATA 		(1<< 3)
#define GROUP_CCMASK 		(1<< 4)
#define GROUP_TEMPDATA		(1<< 5)
#define GROUP_GPSDATA		(1<< 6)
#define GROUP_ETALONDATA	(1<< 7)
#define GROUP_VALVEDATA	        (1<< 8)

// GLOBALS

// define which groups are shown on startup of elekStatus
unsigned int uiGroupFlags = GROUP_TIMEDATA | GROUP_GPSDATA;

// number of status counts total
long StatusCount=0;     

// our keyboard scan routine
void EvaluateKeyboard(void);
void ShowHelp(void);

#endif
