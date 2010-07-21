/*
*
* $RCSfile: spectrometerServer.h,v $ last changed on $Date: 2007/02/16 22:57:49 $ by $Author: rudolf $
*
* $Log: spectrometerServer.h,v $
* Revision 1.2  2007/02/16 22:57:49  rudolf
* added missing LOG keyword
*
*
*/

#ifndef SPECTROMETERSERVER_H
#define SPECTROMETERSERVER_H

// UDP Adressing stuff

#define RECEIVERIP "127.0.0.1"
#define UDPPORTNUMBER 4711

// USB specific defines

#define OOI_VENDOR_ID 0x2457
#define OOI_HR4000_ID 0x1012
#define OOI_HR2000_ID 0x100A
#define USB_BULK_TIMEOUT 500

// generic defines

#define INPUT_BUFFER_SIZE 8192
#define false (0)
#define true (!false)

// DEBUGGING
// 
//#define PRINT_UDPBUF

// functions

int HR2000_GetSpectrum(struct usb_dev_handle *hdev);
void HR2000_Run(void);

int HR4000_Query_Status(void);
void HR4000_Run(void);
int HR4000_GetSpectrum(struct usb_dev_handle *hdev);

// static command defines
// HR2000

char HR2000_INIT[] = { 0x01 };
char HR2000_REQUEST_SPECTRUM[] = { 0x09 };
char HR2000_SETTRIGGER_SOFTWARE[] =
{
   0x0a,0x01,0x00
};
char HR2000_SETTRIGGER_FREERUN[] =
{
   0x0a,0x00,0x00
};

char HR2000_SET_INT100[] =
{
   0x02, 0x64, 0x00
};

// HR4000
#define SPEED_FULL 0 // HR4000 supports 12Mbit and 480Mbit
#define SPEED_HIGH 1

char HR4000_INIT[] = { 0x01 };
char HR4000_REQUEST_SPECTRUM[] = { 0x09 };
char HR4000_QUERY_STATUS[] = { 0xFE };

char HR4000_SET_INT100[] =
{
   0x02, 0xA0, 0x86, 0x01, 0x00 // 1000000 us => 100ms => 0x186A0
};
char HR4000_SETTRIGGER_FREERUN[] =
{
   0x0a,0x00,0x00
};
char HR4000_POWERUP[] =
{
   0x04,0xFF,0xFF
};

   
#endif
