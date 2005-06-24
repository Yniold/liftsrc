/*
* $RCSfile: elekStatus.h,v $ last changed on $Date: 2005-06-24 12:34:13 $ by $Author: rudolf $
*
*
*/

#ifndef ELEKSTATUS_H
#define ELEKSTATUS_H

#define GROUP_DATASETDATA	(1<< 0)
#define GROUP_ADCDATA		(1<< 1)
#define GROUP_TIMEDATA		(1<< 2)
#define GROUP_CCDATA 		(1<< 3)
#define GROUP_TEMPDATA		(1<< 4)
#define GROUP_GPSDATA		(1<< 5)

// define which groups are shown on startup of elekStatus
unsigned int uiGroupFlags = GROUP_DATASETDATA | GROUP_ADCDATA | GROUP_TIMEDATA;
#endif
