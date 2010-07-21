/*
*
* $RCSfile: spectrometerServer.c,v $ last changed on $Date: 2007/10/25 13:59:18 $ by $Author: rudolf $
*
* $Log: spectrometerServer.c,v $
* Revision 1.6  2007/10/25 13:59:18  rudolf
* removed sleeps() again because plotting spectra will be done by reading status.spc
*
* Revision 1.5  2007/03/03 12:09:51  rudolf
* added two sleeps to allow for slow MATLAB to keep up with the packet rate
*
* Revision 1.4  2007-02-19 19:05:47  rudolf
* add statically linked binary for test on lift
*
* Revision 1.3  2007-02-18 20:36:39  rudolf
* changed UDP packet format, fixed wrong wavelength calculation
*
* Revision 1.2  2007-02-16 22:57:49  rudolf
* added missing LOG keyword
*
*
*
*/

// socket includes
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

// main includes
#include <string.h>
#include <stdio.h>
#include <usb.h>
#include <math.h>

#include "spectrometerServer.h"

// generic globals
unsigned char ucIsHR2000 = 0;
unsigned char ucFoundSpectrometer = 0;
int i = 0;
unsigned short pixel_value = 0;
char input_buffer[INPUT_BUFFER_SIZE];

// USB stuff
struct usb_bus *bus = 0;
struct usb_device *device = 0;
struct usb_dev_handle *hdev = 0;
int interface = 0;
int bytes_read = 0;
int uiUSBSpeed = 0; // used for HR4000 which supports 12MBit and 480Mbit

// HR2000 stuff
double dWaveLenCoeffsHR2000[4];
double dDiscreteLinesHR2000[2048];
unsigned short usWaveLenHR2000[2048];
char HR2000_CONFIG_EEPROM[18][16];

// HR4000 stuff
double dWaveLenCoeffsHR4000[4];
double dDiscreteLinesHR4000[3648];
unsigned short usWaveLenHR4000[3648];
char HR4000_CONFIG_EEPROM[18][16];

// UDP stuff
unsigned short usUDPBuffer[4096]; // == 8192 Bytes
int fdSocket;
int iStatus;

// main()
int main()
{
   printf("Spectrometer Server $Id: spectrometerServer.c,v 1.6 2007/10/25 13:59:18 rudolf Exp $ for i386\n\r");
   printf("Initialising UDP socket for sending to port %05d locally:", UDPPORTNUMBER);
   // open debug udp socket
   if ((fdSocket = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
     {
        perror(" failed\n\r");
        exit(1);
     }
   printf("success\n\r");
   usb_init();
   printf("Searching for devices...\n\r");

   while(1)
     {

	ucFoundSpectrometer = false;

	// search busses until finally someone plugs in a spectrometer
	//
	while(!ucFoundSpectrometer)
	  {

	     usb_find_busses();
	     usb_find_devices();

	     for(bus = usb_busses; bus; bus = bus->next)
	       {
		  for(device = bus->devices; device; device = device->next)
		    {
		       if(0 == device->descriptor.idVendor
			  && 0 == device->descriptor.idProduct)
			 continue;

		       if(OOI_VENDOR_ID == device->descriptor.idVendor
			  && OOI_HR4000_ID == device->descriptor.idProduct)
			 {
			    printf("Found a HR4000 on bus %s\n", bus->dirname);
			    ucIsHR2000 = false;
			    ucFoundSpectrometer = true;
			    break;
			 };

		       if(OOI_VENDOR_ID == device->descriptor.idVendor
			  && OOI_HR2000_ID == device->descriptor.idProduct)
			 {
			    printf("Found a HR2000 on bus: %s\n", bus->dirname);
			    ucIsHR2000 = true;
			    ucFoundSpectrometer = true;
			    break;
			 };
		    }
		  if(0 != device)
		    break;
	       }
	     sleep(1);
	  }; /* search for a valid device */
	if(ucIsHR2000 == true)
	  HR2000_Run();

	if(ucIsHR2000 == false)
	  HR4000_Run();
	printf("Searching for devices...\n\r");

     };/*while(1)*/
}; /*main*/

void HR2000_Run(void)
{
   if(!(hdev = usb_open(device)))
     {
	perror("usb_open() failed");
     }
   interface = device->config->interface->altsetting->bInterfaceNumber;
   if(usb_claim_interface(hdev, interface) != 0)
     {
	perror("usb_claim_interface() failed! Are you root ?");
	return;
     }
   sleep(1);
   // clear endpoints
   usb_clear_halt(hdev, 0x82); // EP2 OUT
   usb_clear_halt(hdev, 0x02); // EP2 IN
   usb_clear_halt(hdev, 0x87); // EP7 OUT

   // init HR2000 (will aquire a spectrum automatically)
   usb_bulk_write(hdev, 0x02, HR2000_INIT, sizeof(HR2000_INIT),USB_BULK_TIMEOUT);

   // read EEPROM Data
   printf("HR2000: Reading EEPROM...\n\r");
   for(i = 0; i< 17; i++)
     {
	char aTempBuffer[2];
	aTempBuffer[0] = 0x05; // read EEPROM CMD
	aTempBuffer[1] = i;

	usb_bulk_write(hdev, 0x02, aTempBuffer,sizeof(aTempBuffer),USB_BULK_TIMEOUT);
	usb_bulk_read(hdev, 0x87, input_buffer,18,USB_BULK_TIMEOUT);
	memcpy(&HR2000_CONFIG_EEPROM[i][0],input_buffer+2,16);
	if((i>0) && (i<5))
	  sscanf(HR2000_CONFIG_EEPROM[i],"%lf",&dWaveLenCoeffsHR2000[i-1]);
     }

   // Wavelength is a polynomal of 4th order
   // calculate the wavelength for each pixel value
   //
   printf("HR2000: Calculating wavelengths...");
   for(i=0;i<2048;i++)
     {
	dDiscreteLinesHR2000[i] = dWaveLenCoeffsHR2000[3]*pow((double)(i),3.0f) + dWaveLenCoeffsHR2000[2]*pow((double)(i),2.0f) + dWaveLenCoeffsHR2000[1]*(double)(i) + dWaveLenCoeffsHR2000[0];
	usWaveLenHR2000[i] = (unsigned short)(dDiscreteLinesHR2000[i]*50.0f);
     };

   printf("Range %04.2f - %04.2f nm\n\r",dDiscreteLinesHR2000[0],dDiscreteLinesHR2000[2047]);

   // set integration time to 100ms
   printf("HR2000: Set integration time to 100ms\n\r");
   usb_bulk_write(hdev, 0x02, HR2000_SET_INT100, sizeof(HR2000_SET_INT100), USB_BULK_TIMEOUT);

   // set trigger mode to software
   printf("HR2000: Set trigger mode to freerunning\n\r");
   usb_bulk_write(hdev, 0x02, HR2000_SETTRIGGER_FREERUN, sizeof(HR2000_SETTRIGGER_FREERUN), USB_BULK_TIMEOUT);

   HR2000_GetSpectrum((struct usb_dev_handle *) hdev);

   while(1)
     {
	iStatus = HR2000_GetSpectrum((struct usb_dev_handle *) hdev);
	if(iStatus == -3)
	  {
	     printf("HR2000: Spectrometer disconnected.\n\r");
	     break;
	  }

	if(iStatus < 0)
	  {
	     // clear endpoints
	     usb_clear_halt(hdev, 0x82); // EP2 OUT
	     usb_clear_halt(hdev, 0x02); // EP2 IN
	     usb_clear_halt(hdev, 0x87); // EP7 OUT

	     // HR2000 should be in sync and ready to run now
	     usb_bulk_write(hdev, 0x02,HR2000_REQUEST_SPECTRUM, sizeof(HR2000_REQUEST_SPECTRUM), USB_BULK_TIMEOUT);
	  };
     };
   printf("HR2000: Releasing interface...\n\r");
   usb_release_interface(hdev,interface);
   usb_close(hdev);
};

// ==================================================================================================================
// Handle HR4000 dependentstuff here
// HR4000 supports 12mbit and 480mbit transfers, so there must be 2 seperate
// routines for reading spectral data
// ==================================================================================================================
//
void HR4000_Run(void)
{
   if(!(hdev = usb_open(device)))
     {
	perror("usb_open() failed");
     }
   interface = device->config->interface->altsetting->bInterfaceNumber;
   if(usb_claim_interface(hdev, interface) != 0)
     {
	perror("usb_claim_interface() failed! Are you root ?");
	return;
     }
   sleep(1);
   usb_clear_halt(hdev, 0x01); // EP1 OUT (Commands to HR4000)
   usb_clear_halt(hdev, 0x81); // EP1 IN (reply to commands)
   usb_clear_halt(hdev, 0x82); // EP2 IN (spectra)
   usb_clear_halt(hdev, 0x86); // EP6 IN (spectra)

   // Init HR4000
   printf("HR4000: Init...\n\r");
   usb_bulk_write(hdev, 0x01, HR4000_INIT, sizeof(HR4000_INIT),USB_BULK_TIMEOUT);

   sleep(1);

   // read EEPROM Data
   printf("HR4000: Reading EEPROM...\n\r");
   for(i = 0; i< 17; i++)
     {
	char aTempBuffer[2];
	aTempBuffer[0] = 0x05; // read EEPROM CMD
	aTempBuffer[1] = i;

	usb_bulk_write(hdev, 0x01, aTempBuffer,sizeof(aTempBuffer),USB_BULK_TIMEOUT);
	bytes_read = usb_bulk_read(hdev, 0x81, input_buffer,18,USB_BULK_TIMEOUT);

	if(bytes_read == -19)
	  {
	     printf("HR4000: Spectrometer disconnected.\n\r");
	     usb_release_interface(hdev,interface);
	     usb_close(hdev);
	     return;
	  };

	if(bytes_read < 0)
	  {
	     printf("Read EEPROM failed at EEPROM row: %05d\n\r",i);
	  };

	//	printf("EEPROM: %02d -> %s\n\r",i,input_buffer+2);
	memcpy(&HR4000_CONFIG_EEPROM[i][0],input_buffer+2,16);
	if((i>0) && (i<5))
	  sscanf(HR4000_CONFIG_EEPROM[i],"%lf",&dWaveLenCoeffsHR4000[i-1]);
//	printf("Coeff: %d -> %e\n\r",i, dWaveLenCoeffsHR4000[i-1]);
     }

   // Wavelength is a polynomal of 4th order
   // calculate the wavelength for each pixel value
   //
   printf("HR4000: Calculating wavelengths...");
   for(i=0;i<3648;i++)
     {
	dDiscreteLinesHR4000[i] = dWaveLenCoeffsHR4000[3]*pow((double)(i),3.0f) + dWaveLenCoeffsHR4000[2]*pow((double)(i),2.0f) + dWaveLenCoeffsHR4000[1]*(double)(i) + dWaveLenCoeffsHR4000[0];
	usWaveLenHR4000[i] = (unsigned short)(dDiscreteLinesHR4000[i]*50.0f);
	//	printf("Line %05d is at %d nm\n\r",i,(int)usWaveLenHR4000[i]);
     };
   printf("Range %04.4f - %04.4f nm\n\r",dDiscreteLinesHR4000[0],dDiscreteLinesHR4000[3647]);

   sleep(1);
   // set integration time to 100ms
   printf("HR4000: Set integration time to 100ms\n\r");
   usb_bulk_write(hdev, 0x01, HR4000_SET_INT100, sizeof(HR4000_SET_INT100), USB_BULK_TIMEOUT);

   sleep(1);
   printf("HR4000: determining USB speed\n\r");
   if(HR4000_Query_Status() == -3)
     {
	printf("HR4000: Spectrometer disconnected.\n\r");
	usb_release_interface(hdev,interface);
	usb_close(hdev);
	return;
     }

   sleep(1);
   iStatus = 0;

   // HR4000 should be in sync and ready to run now
   usb_bulk_write(hdev, 0x01,HR4000_REQUEST_SPECTRUM, sizeof(HR4000_REQUEST_SPECTRUM), USB_BULK_TIMEOUT);
   sleep(1);

   while(1)
     {
	iStatus = HR4000_GetSpectrum((struct usb_dev_handle *) hdev);
	if(iStatus == -3)
	  {
	     printf("HR4000: Spectrometer disconnected.\n\r");
	     break;
	  }

	if(iStatus < 0)
	  {
	     usb_clear_halt(hdev, 0x01); // EP1 OUT (Commands to HR4000)
	     usb_clear_halt(hdev, 0x81); // EP1 IN (reply to commands)
	     usb_clear_halt(hdev, 0x82); // EP2 IN (spectra)
	     usb_clear_halt(hdev, 0x86); // EP6 IN (spectra)

	     // HR4000 should be in sync and ready to run now
	     usb_bulk_write(hdev, 0x01,HR4000_REQUEST_SPECTRUM, sizeof(HR4000_REQUEST_SPECTRUM), USB_BULK_TIMEOUT);
	  };
	// HR4000 should be in sync and ready to run now
	usb_bulk_write(hdev, 0x01,HR4000_REQUEST_SPECTRUM, sizeof(HR4000_REQUEST_SPECTRUM), USB_BULK_TIMEOUT);
     };

   printf("HR4000: Releasing interface...\n\r");
   usb_release_interface(hdev,interface);
   usb_close(hdev);

}

int HR2000_GetSpectrum(struct usb_dev_handle *hdev)
{
   // get spectrum
   bytes_read = usb_bulk_read(hdev, 0x82, input_buffer,
			      INPUT_BUFFER_SIZE, USB_BULK_TIMEOUT);

   // disconnect
   if(bytes_read == -19)
     {
	return (-3);
     };

   // wrong packet size
   if(bytes_read != 4097)
     {
	return (-1);
     }

   // wrong sync
   if(input_buffer[bytes_read - 1] != 0x69)
     {
	printf("Did not get sync byte.  Giving up.\n");
	return (-2);
     }
   // =========================
   // HR 2000: first UDP packet
   // =========================

   // first short of buffer contains packet number and maxpackets
   usUDPBuffer[0] = ((unsigned short)(1)) | ((unsigned short)(2 << 8));
   // scaling for y axis
   usUDPBuffer[1] = (unsigned short)4096;
   // Packet Offset
   usUDPBuffer[2] = 0;
   // Max Payload length
   usUDPBuffer[3] = sizeof(usWaveLenHR2000) + (2048*2); // Wavelengths plus two bytes for each sample

   unsigned short *pXValue = &usUDPBuffer[4];
   unsigned short *pYValue = &usUDPBuffer[5];

   unsigned short *pBuffer = (unsigned short *)&input_buffer;

   int iIndex = 0;
   int i = 0;
   int size = 0;
   for(i=0; i<2046; i++)
     {
	pixel_value = ((unsigned short)(input_buffer[iIndex] & 0x00FF) | ((unsigned short)((input_buffer[iIndex+64]) <<8) & 0xFF00));

#ifdef PRINT_PIXEL
	printf("Pixel %d value is %d\n", i, (int)pixel_value);
#endif
	iIndex++;
	if(iIndex%64 == 0)
	  iIndex += 64;

	// copy interleaved to UDP Buffer
	*pXValue = usWaveLenHR2000[i];
	pXValue += 2;
	*pYValue = pixel_value;
	pYValue += 2;
	size +=4;
     }
   printf("HR2000: Sent Spectral Data from offset %05d (Packet %02d of %02d, size %05d of %05d bytes total) to %s on port %05d\n\r",\
   (int)usUDPBuffer[2],\
   (int)(usUDPBuffer[0] & 0x00FF),\
   (int)((usUDPBuffer[0] >> 8) & 0x00FF),\
   (int)(size+8),\
   (int)usUDPBuffer[3],\
   RECEIVERIP,\
   UDPPORTNUMBER);

   // ==========================
   // HR 2000: second UDP packet
   // ==========================
 
   SendUDPDataToIP(RECEIVERIP,size+8, usUDPBuffer);
   // first short of buffer contains packet number and maxpackets
   usUDPBuffer[0] = ((unsigned short)(2)) | ((unsigned short)(2 << 8));
   // scaling for y axis
   usUDPBuffer[1] = (unsigned short)4096;
   // Packet Offset
   usUDPBuffer[2] = 4*2046;
   // Max Payload length
   usUDPBuffer[3] = sizeof(usWaveLenHR2000) + (2048*2); // Wavelengths plus two bytes for each sample

   pXValue = &usUDPBuffer[4];
   pYValue = &usUDPBuffer[5];
   size = 0;
   
   for(i = 2046; i < 2048; i++)
     {
	pixel_value = ((unsigned short)(input_buffer[iIndex] & 0x00FF) | ((unsigned short)((input_buffer[iIndex+64]) <<8) & 0xFF00));

#ifdef PRINT_PIXEL
	printf("Pixel %d value is %d\n", i, (int)pixel_value);
#endif
	iIndex++;
	if(iIndex%64 == 0)
	  iIndex += 64;

	// copy interleaved to UDP Buffer
	*pXValue = usWaveLenHR2000[i];
	pXValue += 2;
	*pYValue = pixel_value;
	pYValue += 2;
	size += 4;
     }
   printf("HR2000: Sent Spectral Data from offset %05d (Packet %02d of %02d, size %05d of %05d bytes total) to %s on port %05d\n\r",\
   (int)usUDPBuffer[2],\
   (int)(usUDPBuffer[0] & 0x00FF),\
   (int)((usUDPBuffer[0] >> 8) & 0x00FF),\
   (int)(size+8),\
   (int)usUDPBuffer[3],\
   RECEIVERIP,\
   UDPPORTNUMBER);

   SendUDPDataToIP(RECEIVERIP,size+8, usUDPBuffer);
   return 0;
}

// ==================================================================================================================
// GetSpectrum:
// HR4000 supports 12mbit and 480mbit transfers, so there must be 2 seperate
// routines for reading spectral data
// ==================================================================================================================
//
int HR4000_GetSpectrum(struct usb_dev_handle *hdev)
{
   if(uiUSBSpeed == SPEED_HIGH)
     {

	// get spectrum from EP6
	bytes_read = usb_bulk_read(hdev, 0x86, input_buffer,
				   2048, USB_BULK_TIMEOUT);

	// disconnect
	if(bytes_read == -19)
	  {
	     return (-3);
	  };

	// wrong packet size
	if(bytes_read != 2048)
	  {
	     return (-1);
	  }

	// get spectrum from EP2
	bytes_read = usb_bulk_read(hdev, 0x82, input_buffer+2048,
				   ((512*11)+1), USB_BULK_TIMEOUT);

	// disconnect
	if(bytes_read == -19)
	  {
	     return (-3);
	  };

	// wrong packet size
	if(bytes_read != ((512*11)+1))
	  {
	     return (-1);
	  }

     }
   else
     {

	// get spectrum
	bytes_read = usb_bulk_read(hdev, 0x82, input_buffer,
				   ((120*64)+1), USB_BULK_TIMEOUT);

	// disconnect
	if(bytes_read == -19)
	  {
	     return (-3);
	  };

	// wrong packet size
	if(bytes_read != ((120*64)+1))
	  {
	     return (-1);
	  }
	// check for sync byte
        if(input_buffer[bytes_read-1] != 0x69)
	  {
	     return (-2);
	  };

     }

   // first short of buffer contains packet number and maxpackets
   usUDPBuffer[0] = ((unsigned short)(1)) | ((unsigned short)(2 << 8));
   // scaling for y axis
   usUDPBuffer[1] = (unsigned short)16384;
   // Packet Offset
   usUDPBuffer[2] = 0;
   // Max Payload length
   usUDPBuffer[3] = sizeof(usWaveLenHR4000) + (3648*2); // Wavelengths plus two bytes for each sample

   unsigned short *pXValue = &usUDPBuffer[4];
   unsigned short *pYValue = &usUDPBuffer[5];

   unsigned short *pBuffer = (unsigned short *)&input_buffer;
   int i = 0;
   int size = 0;
   for(i=0; i<2046; i++)
     {
	pixel_value = (*pBuffer);

	if(pixel_value > 8192)
	  pixel_value = pixel_value -8192;
	  else
	  if(pixel_value < 8192)
	    pixel_value = pixel_value + 8192;

	pBuffer++;

	*pXValue = usWaveLenHR4000[i];
	pXValue += 2;
	*pYValue = pixel_value;
	pYValue += 2;
	size +=4;
     }
   printf("HR4000: Sent Spectral Data from offset %05d (Packet %02d of %02d, size %05d of %05d bytes total) to %s on port %05d\n\r",\
   (int)usUDPBuffer[2],\
   (int)(usUDPBuffer[0] & 0x00FF),\
   (int)((usUDPBuffer[0] >> 8) & 0x00FF),\
   (int)(size+8),\
   (int)usUDPBuffer[3],\
   RECEIVERIP,\
   UDPPORTNUMBER);
   SendUDPDataToIP(RECEIVERIP,size+8, usUDPBuffer);
//   sleep(1);
   // ======================
   // assemble second buffer
   // ======================
   //
   // first short of buffer contains packet number and maxpackets
   usUDPBuffer[0] = ((unsigned short)(2)) | ((unsigned short)(2 << 8));
   // scaling for y axis
   usUDPBuffer[1] = (unsigned short)16384;
   // Packet Offset
   usUDPBuffer[2] = (4*2046);
   // Max Payload length
   usUDPBuffer[3] = sizeof(usWaveLenHR4000) + (3648*2); // Wavelengths plus two bytes for each sample

   // first short of buffer contains packet number and maxpackets
   usUDPBuffer[0] = ((unsigned short)(2)) | ((unsigned short)(2 << 8));
   // scaling for y axis
   usUDPBuffer[1] = (unsigned short)16384;

   pXValue = (unsigned short*)&usUDPBuffer[4];
   pYValue = (unsigned short*)&usUDPBuffer[5];
   pBuffer = (unsigned short *)&input_buffer;
   pBuffer += 2044;

   i = 0;
   size = 0;
   for(i=2044; i<3648; i++)
     {
	pixel_value = (*pBuffer);

	if(pixel_value > 8192)
	  pixel_value = pixel_value -8192;
	  else
	  if(pixel_value < 8192)
	    pixel_value = pixel_value + 8192;

	pBuffer++;

	*pXValue = usWaveLenHR4000[i];
	pXValue += 2;
	*pYValue = pixel_value;
	pYValue += 2;
	size +=4;
     }

   printf("HR4000: Sent Spectral Data from offset %05d (Packet %02d of %02d, size %05d of %05d bytes total) to %s on port %05d\n\r",\
   (int)usUDPBuffer[2],\
   (int)(usUDPBuffer[0] & 0x00FF),\
   (int)((usUDPBuffer[0] >> 8) & 0x00FF),\
   (int)(size+8),\
   (int)usUDPBuffer[3],\
   RECEIVERIP,\
   UDPPORTNUMBER);
   SendUDPDataToIP(RECEIVERIP,size+8, usUDPBuffer);
//   sleep(1);
   return 0;
}
//***********************************
//* query and dump the current status
//***********************************

int HR4000_Query_Status(void)
{
   unsigned char ucBuffer[32];

   // *******************
   // * query status CMD
   // *******************
   //
   bytes_read = usb_bulk_write(hdev, 0x01, HR4000_QUERY_STATUS, sizeof(HR4000_QUERY_STATUS), USB_BULK_TIMEOUT);
   if(bytes_read < 0)
     {
	if(bytes_read == -19)
	  return (-3);
     };

   bytes_read = usb_bulk_read(hdev,0x81, (char *)ucBuffer, sizeof(ucBuffer), USB_BULK_TIMEOUT);
   if(bytes_read>0)
     {
	/*
	printf("\n\r");
	printf("Number of Pixels: %04d\n\r",(int)ucBuffer[0]+(int)(ucBuffer[1]<<8));
	printf("Integration Time: %08d\n\r",
	       (unsigned int)(ucBuffer[2]      & 0x000000FF) |
	       (unsigned int)((ucBuffer[3]<<8) & 0x0000FF00) |
	       (unsigned int)((ucBuffer[4]<<16)& 0x00FF0000) |
	       (unsigned int)((ucBuffer[5]<<24)& 0xFF000000));

	printf("Integration Time: %02x %02x %02x %02x\n\r",
	       ucBuffer[2],
	       ucBuffer[3],
	       ucBuffer[4],
	       ucBuffer[5]);

	printf("Lamp enabled: %s\n\r",ucBuffer[6]==0?"off":"on");
	printf("Trigger mode: %08d\n\r",ucBuffer[7]);
	printf("Aquisition status: %08d\n\r",ucBuffer[8]);
	printf("Packets in spectra: %04d\n\r",ucBuffer[9]);
	printf("Power state: %s\n\r",ucBuffer[10]==0?"powered down":"active");
	printf("Packets loaded to Endpoint: %04d\n\r",ucBuffer[11]);
	printf("USB Speed: %s\n\r",ucBuffer[14]==0?"full speed (12Mbit)":"high speed (480MBit)");
	*/

	// we need to do at least the USB speed detection
	if(ucBuffer[14] == 0)
	  uiUSBSpeed = SPEED_FULL;
	else
	  uiUSBSpeed = SPEED_HIGH;
     };
}

/*********************************************************************************************************/
/*                                                                                                       */
/* Function to Send UDP Data, IP Adress explicit given in IPAddr                                         */
/*                                                                                                       */
/*********************************************************************************************************/

int SendUDPDataToIP(char *IPAddr, unsigned nByte, void *msg)
{

   struct sockaddr_in their_addr;
   int numbytes;

   their_addr.sin_family = AF_INET;     // host byte order
   their_addr.sin_port = htons(UDPPORTNUMBER); // short, network byte order
   their_addr.sin_addr.s_addr = inet_addr(IPAddr);
   memset(&(their_addr.sin_zero), '\0', 8);  // zero the rest of the struct

   if ((numbytes=sendto(fdSocket, msg, nByte, 0,
			(struct sockaddr *)&their_addr, sizeof(struct sockaddr))) == -1)
     {
        perror("sendto");
        exit(1);
     }

}
/* SendUDPDatatoIP */
