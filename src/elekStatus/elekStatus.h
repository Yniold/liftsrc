/*
* $RCSfile: elekStatus.h,v $ last changed on $Date: 2007-08-07 12:35:09 $ by $Author: rudolf $
* $Log: elekStatus.h,v $
* Revision 1.11  2007-08-07 12:35:09  rudolf
* preparations for recording spectral data as well
*
* Revision 1.10  2007-03-09 13:09:21  rudolf
* added new port for aux data, recording of aux data and dumping of aux data
*
* Revision 1.9  2007-02-21 22:57:57  rudolf
* added new group for LICOR in status printing
*
* Revision 1.8  2006-10-06 10:19:45  rudolf
* added butterfly structure as new group
*
* Revision 1.7  2005/06/29 12:46:18  rudolf
* preparation to display mask in status
*
* Revision 1.6  2005/06/26 16:50:17  rudolf
* added Valve display (HH)
*
* Revision 1.5  2005/06/25 19:20:18  rudolf
* removed debug output, added small helpscreen invokeable by pressing [H]
*
* Revision 1.4  2005/06/25 14:54:49  rudolf
* added missing RCS keyword in header
*
*
*/

#ifndef ELEKSTATUS_H
#define ELEKSTATUS_H

#define GROUP_DATASETDATA	(1<< 0)
#define GROUP_ADCDATA		(1<< 1)
#define GROUP_TIMEDATA		(1<< 2)
#define GROUP_CCDATA 		(1<< 3)
#define GROUP_CCMASK 		(1<< 4)
#define GROUP_TEMPDATA		(1<< 5)
#define GROUP_GPSDATA		(1<< 6)
#define GROUP_ETALONDATA	(1<< 7)
#define GROUP_VALVEDATA	        (1<< 8)
#define GROUP_BUTTERFLYDATA	(1<< 9)

#define GROUP_CALIBDATA         (1<<10)
#define GROUP_AUXDATA           (1<<11)
#define GROUP_SPECTRADATA       (1<<12)
// GLOBALS

// our keyboard scan routine
void EvaluateKeyboard(void);
void ShowHelp(void);

#endif
