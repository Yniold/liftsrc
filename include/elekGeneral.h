/*
* $RCSfile: elekGeneral.h,v $ last changed on $Date: 2005/04/22 10:52:52 $ by $Author: rudolf $
*
* $Log: elekGeneral.h,v $
* Revision 1.2  2005/04/22 10:52:52  rudolf
* added extra path for files in RAMDISK (e.g. for status.bin)
*
*
*/

#define MAX_FILENAME_LEN 128
#define GENERIC_BUF_LEN 256

#define DATAPATH   "./data"
#define RAMDISKPATH "/lift/ramdisk"

#ifndef bool
typedef unsigned char bool;
#define FALSE (unsigned char) 0
#define TRUE  (unsigned char) 1
#endif

