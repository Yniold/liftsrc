/*
*
* $RCSfile: spectrumAnalyzer.h,v $ last changed on $Date: 2005-07-06 21:08:47 $ by $Author: rudolf $
*
* $Log: spectrumAnalyzer.h,v $
* Revision 1.2  2005-07-06 21:08:47  rudolf
* more work on spectrometer, HR4000 decides between HIGH and FULLSPEED mode, implemented correct handling for both modes
*
* Revision 1.1  2005/07/06 16:38:42  rudolf
* added initial WIP version of spectrometer software
*
*
*
*/

#ifndef SPECTRUMANALYZER_H 
#define SPECTRUMANALYZER_H 

#define VEN_ID_OCEAN_OPTICS 0x2457
#define PROD_ID_HR4000 0x1012
#define PROD_ID_HR2000 0x100A

// FIXME: read actual adresses from the descriptors

#define EP1_OUT_ADDR				(0x01)
#define EP2_IN_ADDR				(0x82)
#define EP6_IN_ADDR				(0x86)
#define EP1_IN_ADDR				(0x81)

// CMDs from datasheet HR2000

#define CMD_INIT 					(0x01)
#define CMD_SET_INT_TIME 		(0x02) 
#define CMD_SET_STR_ENA			(0x03)
#define CMD_RESERVED 			(0x04)
#define CMD_QUERY_INFO			(0x05)
#define CMD_WRITE_INFO			(0x06)
#define CMD_WRITE_SERIAL		(0x07)
#define CMD_READ_SERIAL			(0x08)
#define CMD_REQUEST_SPECTRA	(0x09)
#define CMD_SET_TRIGGER			(0x0a)
#define CMD_QUERY_NUM_PLUG		(0x0b)
#define CMD_QUERY_PLUG_ID		(0x0c)
#define CMD_DETECT_PLUG			(0x0d)

// only the HR4000 seems to know following commands

#define CMD_SET_SHUTDOWN 		(0x04)
#define CMD_READ_PCB_TEMP		(0x6C)
#define CMD_QUERY_STATUS		(0xFE) 

#define SPEED_FULL 				(0x00)
#define SPEED_HIGH				(0x01)

void print_endpoint(struct usb_endpoint_descriptor *endpoint);
void print_altsetting(struct usb_interface_descriptor *interface);
void print_interface(struct usb_interface *interface);
void print_configuration(struct usb_config_descriptor *config);
int print_device(struct usb_device *dev, int level);

void HR4000_Query_Status(void);
#endif
