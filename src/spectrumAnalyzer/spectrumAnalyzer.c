/*
*
* $RCSfile: spectrumAnalyzer.c,v $ last changed on $Date: 2005-07-10 15:38:25 $ by $Author: rudolf $
*
* $Log: spectrumAnalyzer.c,v $
* Revision 1.5  2005-07-10 15:38:25  rudolf
* changes for min max scaling
*
* Revision 1.4  2005/07/10 15:18:58  rudolf
* added basic support for plotting a spectrum via directFB
*
* Revision 1.3  2005/07/09 19:29:22  rudolf
* fixed 480mbit mode, added basic evaluation of wavelength and counts
*
* Revision 1.2  2005/07/06 21:08:47  rudolf
* more work on spectrometer, HR4000 decides between HIGH and FULLSPEED mode, implemented correct handling for both modes
*
* Revision 1.1  2005/07/06 16:38:42  rudolf
* added initial WIP version of spectrometer software
*
*
*
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <usb.h>
#include <directfb.h>
#include "spectrumAnalyzer.h"

// DFB globals

static IDirectFB *dfb = NULL;
static IDirectFBSurface *primary = NULL;
static int screen_width  = 0;
static int screen_height = 0;

// globals
int verbose = 0;
unsigned char ucCalibrationData[18][17];
uint16_t uiSpectralData[3840];
double dSpectralWavelengths[3840];
double dCoeffs[4];
unsigned char *pSpectralBuffer = NULL;
int iByteCount = 0;
int iNumberOfPackets = 0;

// USB globals
int uiUSBSpeed = SPEED_HIGH;		// default FULLSPEED, but will be read from device
unsigned char ucBuffer[8192];		// datasets are 7616 bytes max.
void *ptrVoid;
int uiNumBusses = 0;
int uiNumDevices =0;
int iRet = 0;

struct usb_bus *usb_busses;		// saves pointer to first bus
struct usb_bus *bus;					// bus structure
struct usb_device *dev;				// device structure

struct usb_bus *specBus;			// the bus structure the spectrograph is attached to
struct usb_device *specDevice;	// the device structure of the spectrograph
usb_dev_handle *specHandle;		// the handle of the spectrograph

// main	
int main(int argc, char *argv[])
{
	
	// evaluate commandline
	
	if (argc > 1 && !strcmp(argv[1], "-v"))
		verbose = 1;
	DFBCHECK (DirectFBInit (&argc, &argv));

	usb_init();
	
	// find busses
	
	uiNumBusses = usb_find_busses();
	if(uiNumBusses)
	{
		printf("Number of USB busses found: %02d\n\r",uiNumBusses);
	}
	else
	{
		printf("No USB busses found. Exiting...\n\r");
		exit(-1);
	};
	
	// find devices
	uiNumDevices = usb_find_devices();
	
	if(uiNumDevices)
	{
		printf("Number of USB devices found: %02d\n\r",uiNumDevices);
	}
	else
	{
		printf("No USB devices found. Exiting...\n\r");
		exit(-1);
	};
	
	// save pointer to first bus structure
	usb_busses = usb_get_busses();
	
	// print info about each bus and the attached devices
	for (bus = usb_busses; bus; bus = bus->next) 
	{
		if (bus->root_dev && !verbose)
			print_device(bus->root_dev, 0);
		else 
		{
			struct usb_device *dev;
	
			for (dev = bus->devices; dev; dev = dev->next)
				print_device(dev, 0);
		}
	}
	
	specBus = NULL;
	specDevice = NULL;
	specHandle = 0;
	
	// Locate the spectrometer on the busses
	for (bus = usb_busses; bus; bus = bus->next) 
	{
		for (dev = bus->devices; dev; dev = dev->next)
		{
			if((dev->descriptor.idVendor == VEN_ID_OCEAN_OPTICS) &&
			   (dev->descriptor.idProduct == PROD_ID_HR4000))
			{
				printf("Found Ocean Optics HR4000 as device# %02d\n\r",dev->devnum);
				specBus = bus;
				specDevice = dev;
			};
			
			if((dev->descriptor.idVendor == VEN_ID_OCEAN_OPTICS) &&
			   (dev->descriptor.idProduct == PROD_ID_HR2000))
			{
				printf("Found Ocean Optics HR2000 as device# %02d\n\r",dev->devnum);
				specBus = bus;
				specDevice = dev;

			};
		};
	};
	// check if the structures are valid
	// then try to open the devic	
	if(specDevice)
	{
		specHandle = usb_open(specDevice);
		if(specHandle < 0)
		{
			printf("Open device failed\n\r");
			usb_close(specHandle);
			exit(-1);
		}
		else
		{
			printf("Device opened!\n\r");
		};
		
		iRet = usb_claim_interface(specHandle,0); // claim interface 0 from driver (we only have 1 config);
		
		if(iRet < 0)
		{
			printf("Can't claim interface from driver\n\r");	
			usb_close(specHandle);
			exit(-1);
		}
		else
		{
			printf("Claimed interface #0 from driver\n\r");	
		}
		
		
		iRet = usb_set_configuration(specHandle,1); // set config 1 active (we only have 1 config);
		
		if(iRet < 0)
		{
			printf("Can't set configuration\n\r");	
			usb_close(specHandle);
			exit(-1);
		}
		else
		{
			printf("Set configuration\n\r");	
		}
		//******************************************************************************
		// from here on we can talk to the spectrograph via usb in/out bulk transfers
		//******************************************************************************
		usb_clear_halt(specHandle, EP1_OUT_ADDR);
		usb_clear_halt(specHandle, EP2_IN_ADDR);
		usb_clear_halt(specHandle, EP6_IN_ADDR);
		usb_clear_halt(specHandle, EP1_IN_ADDR);

		HR4000_Init();
		HR4000_SetTriggerMode(0);
		HR4000_ReadPCBTemp();
		HR4000_ReadCalibrationData();
//		HR4000_SetPowerdownMode();
//		HR4000_Query_Status();
		HR4000_SetIntegrationTime(100000);
//		HR4000_Query_Status();
		HR4000_AquireSpectrum();
				
		// cleanup
		usb_release_interface(specHandle,0);
		usb_close(specHandle);
	}
	else
	{
		printf("No Spectrometer found! Exiting...\n\r");
	};
	return -1;
}


// Helper functions

void print_endpoint(struct usb_endpoint_descriptor *endpoint)
{
  printf("      bEndpointAddress: %02xh\n", endpoint->bEndpointAddress);
  printf("      bmAttributes:     %02xh\n", endpoint->bmAttributes);
  printf("      wMaxPacketSize:   %d\n", endpoint->wMaxPacketSize);
  printf("      bInterval:        %d\n", endpoint->bInterval);
  printf("      bRefresh:         %d\n", endpoint->bRefresh);
  printf("      bSynchAddress:    %d\n", endpoint->bSynchAddress);
}

void print_altsetting(struct usb_interface_descriptor *interface)
{
  int i;

  printf("    bInterfaceNumber:   %d\n", interface->bInterfaceNumber);
  printf("    bAlternateSetting:  %d\n", interface->bAlternateSetting);
  printf("    bNumEndpoints:      %d\n", interface->bNumEndpoints);
  printf("    bInterfaceClass:    %d\n", interface->bInterfaceClass);
  printf("    bInterfaceSubClass: %d\n", interface->bInterfaceSubClass);
  printf("    bInterfaceProtocol: %d\n", interface->bInterfaceProtocol);
  printf("    iInterface:         %d\n", interface->iInterface);

  for (i = 0; i < interface->bNumEndpoints; i++)
    print_endpoint(&interface->endpoint[i]);
}

void print_interface(struct usb_interface *interface)
{
  int i;

  for (i = 0; i < interface->num_altsetting; i++)
    print_altsetting(&interface->altsetting[i]);
}

void print_configuration(struct usb_config_descriptor *config)
{
  int i;

  printf("  wTotalLength:         %d\n", config->wTotalLength);
  printf("  bNumInterfaces:       %d\n", config->bNumInterfaces);
  printf("  bConfigurationValue:  %d\n", config->bConfigurationValue);
  printf("  iConfiguration:       %d\n", config->iConfiguration);
  printf("  bmAttributes:         %02xh\n", config->bmAttributes);
  printf("  MaxPower:             %d\n", config->MaxPower);

  for (i = 0; i < config->bNumInterfaces; i++)
    print_interface(&config->interface[i]);
}

int print_device(struct usb_device *dev, int level)
{
  usb_dev_handle *udev;
  char description[256];
  char string[256];
  int ret, i;

  udev = usb_open(dev);
  if (udev) {
    if (dev->descriptor.iManufacturer) {
      ret = usb_get_string_simple(udev, dev->descriptor.iManufacturer, string, sizeof(string));
      if (ret > 0)
        snprintf(description, sizeof(description), "%s - ", string);
      else
        snprintf(description, sizeof(description), "%04X - ",
                 dev->descriptor.idVendor);
    } else
      snprintf(description, sizeof(description), "%04X - ",
               dev->descriptor.idVendor);

    if (dev->descriptor.iProduct) {
      ret = usb_get_string_simple(udev, dev->descriptor.iProduct, string, sizeof(string));
      if (ret > 0)
        snprintf(description + strlen(description), sizeof(description) -
                 strlen(description), "%s", string);
      else
        snprintf(description + strlen(description), sizeof(description) -
                 strlen(description), "%04X", dev->descriptor.idProduct);
    } else
      snprintf(description + strlen(description), sizeof(description) -
               strlen(description), "%04X", dev->descriptor.idProduct);

  } else
    snprintf(description, sizeof(description), "%04X - %04X",
             dev->descriptor.idVendor, dev->descriptor.idProduct);

  printf("%.*sDev #%d: %s\n", level * 2, "                    ", dev->devnum,
         description);

  if (udev && verbose) {
    if (dev->descriptor.iSerialNumber) {
      ret = usb_get_string_simple(udev, dev->descriptor.iSerialNumber, string, sizeof(string));
      if (ret > 0)
        printf("%.*s  - Serial Number: %s\n", level * 2,
               "                    ", string);
    }
  }

  if (udev)
    usb_close(udev);

  if (verbose) {
    if (!dev->config) {
      printf("  Couldn't retrieve descriptors\n");
      return 0;
    }

    for (i = 0; i < dev->descriptor.bNumConfigurations; i++)
      print_configuration(&dev->config[i]);
  } else {
    for (i = 0; i < dev->num_children; i++)
      print_device(dev->children[i], level + 1);
  }

  return 0;
  
}

// ***************************************************************************
// * HR 4000 functions below
// ***************************************************************************

//***********************************
//* query and dump the current status
//***********************************

void HR4000_Query_Status()
{	
		// *******************
		// * query status CMD
		// *******************
		
		ucBuffer[0] = CMD_QUERY_STATUS;
		iRet = usb_bulk_write(specHandle, EP1_OUT_ADDR, (char*)ucBuffer, 1, USB_TIMEOUT);
		if(iRet < 0)
		{
			printf("CMD_QUERY_STATUS failed!\n\r");
			// cleanup
			usb_release_interface(specHandle,0);
			usb_close(specHandle);
			exit(-1);
		};
		
		iRet = usb_bulk_read(specHandle,EP1_IN_ADDR, (char *)ucBuffer, sizeof(ucBuffer), USB_TIMEOUT);
		if(iRet>0)
		{
			printf("\n\r");
			printf("Number of Pixels: %04d\n\r",(int)ucBuffer[0]+(int)(ucBuffer[1]<<8));
			printf("Integration Time: %08d\n\r",
					(int)(ucBuffer[2])+
					(int)(ucBuffer[3]<<8)+
					(int)(ucBuffer[4]<<16)+
					(int)(ucBuffer[5]<<24));
					
			printf("Lamp enabled: %s\n\r",ucBuffer[6]==0?"off":"on");
			printf("Trigger mode: %08d\n\r",ucBuffer[7]);
			printf("Aquisition status: %08d\n\r",ucBuffer[8]);
			printf("Packets in spectra: %04d\n\r",ucBuffer[9]);
			printf("Power state: %s\n\r",ucBuffer[10]==0?"powered down":"active");
			printf("Packets loaded to Endpoint: %04d\n\r",ucBuffer[11]);
			printf("USB Speed: %s\n\r",ucBuffer[14]==0?"full speed (12Mbit)":"high speed (480MBit)");
			
			if(ucBuffer[14] == 0)
				uiUSBSpeed = SPEED_FULL;
			else
				uiUSBSpeed = SPEED_HIGH;
		};
}

// *************
// * init CMD
// *************
void HR4000_Init()
{		
		ucBuffer[0] = CMD_INIT;
		iRet = usb_bulk_write(specHandle, EP1_OUT_ADDR, (char*)ucBuffer, 1, USB_TIMEOUT);
		if(iRet < 0)
		{
			printf("Initializing Spectrograph failed!\n\r");
			// cleanup
			usb_release_interface(specHandle,0);
			usb_close(specHandle);
			exit(-1);
		}
		else
		{
			printf("Init Command sent!\n\r");
		};
};

// ***********************
// * set integration time
// ***********************

void HR4000_SetIntegrationTime(unsigned int uiMicroSeconds)
{
		ucBuffer[0] = CMD_SET_INT_TIME;
		ucBuffer[1] = (unsigned char)(uiMicroSeconds & 0xFF);
		ucBuffer[2] = (unsigned char)((uiMicroSeconds>>8) & 0xFF);
		ucBuffer[3] = (unsigned char)((uiMicroSeconds>>16) & 0xFF);
		ucBuffer[4] = (unsigned char)((uiMicroSeconds>>24) & 0xFF);
		
		iRet = usb_bulk_write(specHandle, EP1_OUT_ADDR, (char*)ucBuffer, 5, USB_TIMEOUT);
		if(iRet < 0)
		{
			printf("Set integration time failed!\n\r");
			// cleanup
			usb_release_interface(specHandle,0);
			usb_close(specHandle);
			exit(-1);
		}
		else
		{
			printf("Set integration time to %04.3f ms!\n\r",(float)(uiMicroSeconds/1000));
		};
};

// ***********************
// * set trigger mode
// ***********************

void HR4000_SetTriggerMode(unsigned char ucMode)
{
		
		ucBuffer[0] = CMD_SET_TRIGGER;
		ucBuffer[1] = ucMode & 0x03;
		ucBuffer[2] = 0x00;
		
		iRet = usb_bulk_write(specHandle, EP1_OUT_ADDR, (char*)ucBuffer, 3, USB_TIMEOUT);
		if(iRet < 0)
		{
			printf("Set trigger mode failed!\n\r");
			// cleanup
			usb_release_interface(specHandle,0);
			usb_close(specHandle);
			exit(-1);
		}
		else
		{
			printf("Set trigger mode!\n\r");
		};
};	

// *********************
// * read PCB temp
// *********************

void HR4000_ReadPCBTemp()
{	
		ucBuffer[0] = CMD_READ_PCB_TEMP;
		iRet = usb_bulk_write(specHandle, EP1_OUT_ADDR, (char*)ucBuffer, 1, USB_TIMEOUT);
		if(iRet < 0)
		{
			printf("READ TEMP failed!\n\r");
			// cleanup
			usb_release_interface(specHandle,0);
			usb_close(specHandle);
			exit(-1);
		}
		else
		{
			printf("Reading board temperature!\n\r");
		};
		
		iRet = usb_bulk_read(specHandle,EP1_IN_ADDR, (char *)ucBuffer, sizeof(ucBuffer), USB_TIMEOUT);
		if(iRet>0 && (ucBuffer[0] == 0x08))
		{			
			unsigned int iTemp = (unsigned int)(ucBuffer[2]<<8)+
										(unsigned int)(ucBuffer[1]);
			
			printf("Board Temperature is: %f\n\r",0.003906*(double)(iTemp));
		}
;}

// ************************************************************************
// * read calibration data, serial number etc and store in external file
// ************************************************************************

void HR4000_ReadCalibrationData(void)
{		
		int iDataByte = 0;
		
		printf("Dump of calibration data:\n\r");
		printf("=========================\n\r");
		
		for(iDataByte=0;iDataByte < 17;iDataByte++)
		{	
			ucBuffer[0] = CMD_QUERY_INFO;
			ucBuffer[1] = iDataByte; // "Data Byte" documentation page 13
			
			iRet = usb_bulk_write(specHandle, EP1_OUT_ADDR, (char*)ucBuffer, 2, USB_TIMEOUT);
			if(iRet < 0)
			{
				printf("CMD_QUERY_INFO failed!\n\r");
				// cleanup
				usb_release_interface(specHandle,0);
				usb_close(specHandle);
				exit(-1);
			};
						
			iRet = usb_bulk_read(specHandle,EP1_IN_ADDR, (char *)&ucCalibrationData[iDataByte][0], 19, USB_TIMEOUT);
			if(iRet>0)
			{
				int iLoop = 0;
				for(iLoop = 0; iLoop < iRet; iLoop++)
					printf("%02x ",ucCalibrationData[iDataByte][iLoop]);
				printf("\n\r");
				
				for(iLoop = 2; iLoop < iRet; iLoop++)
					printf("%c",ucCalibrationData[iDataByte][iLoop]);
				printf("\n\r");
			}
		}
		
		// write cal data to file
		
		FILE *fdFile;
		if ((fdFile=fopen("./calib_data.bin","w"))==NULL)
		{
			printf("Can't open file for writing!\n\r");
		}
		else
		{
			iRet = fwrite(ucCalibrationData,1,sizeof(ucCalibrationData),fdFile);
			if(iRet != sizeof(ucCalibrationData))
			{
				printf("Write cal data failed!\n\r");
				fclose(fdFile);
			}
			else
			{
				printf("Write cal data succeeded!\n\r");
				fclose(fdFile);
			}
									
		}	
		int iLoop = 0;
		for(iLoop = 1; iLoop < 5; iLoop++)
		{	
			float fValue = 0;
			unsigned char* pBuffer = &ucCalibrationData[iLoop][2];
			int iRet = sscanf(pBuffer, "%f", &fValue);
			
			if(iRet >0)
				dCoeffs[iLoop-1] = (double) fValue;
			else
				dCoeffs[iLoop-1] = 0;
		} 
		for(iLoop = 0; iLoop < 4; iLoop++)
		{
			printf("Calibration Coefficient #%02d is %f\n\r", iLoop, dCoeffs[iLoop]);
		};
			
};

// *********************
// * set powerdown mode
// *********************

void HR4000_SetPowerdownMode()
{				
	ucBuffer[0] = CMD_SET_SHUTDOWN;
	ucBuffer[1] = 0x01;	// power up electronics (manual page 12)
	ucBuffer[2] = 0x00;
	
	iRet = usb_bulk_write(specHandle, EP1_OUT_ADDR, (char*)ucBuffer, 3, USB_TIMEOUT);
	if(iRet < 0)
	{
		printf("Set Powerdown Mode failed!\n\r");
		// cleanup
		usb_release_interface(specHandle,0);
		usb_close(specHandle);
		exit(-1);
	}
	else
	{
		printf("Set Powerdown Mode sent!\n\r");
	};
}		

		// **************************************************************
		// * read spectral data
		// **************************************************************
		// * in full speed mode the data are sent via EP2_IN
		// * 120 packets of 64 bytes each plus a sync packet of 1 byte 
		// **************************************************************

void HR4000_AquireSpectrum()
{
	if(uiUSBSpeed == SPEED_FULL)
	{
		printf("Aquiring spectral data in FULL SPEED mode (12mbit)\n\r");			
		ucBuffer[0] = CMD_REQUEST_SPECTRA;
		iRet = usb_bulk_write(specHandle, EP1_OUT_ADDR, (char*)ucBuffer, 1, USB_TIMEOUT);
		if(iRet < 0)
		{
			printf("CMD_REQUEST_SPECTRA failed!\n\r");
			// cleanup
			usb_release_interface(specHandle,0);
			usb_close(specHandle);
			exit(-1);
		}
		else
		{
			printf("CMD_REQUEST_SPECTRA Command sent!\n\r");
		};
		
		iNumberOfPackets = 0;
		pSpectralBuffer = (unsigned char*)(&uiSpectralData);
		iByteCount = 0;
		
		while(1)
		{
			iRet = usb_bulk_read(specHandle,EP2_IN_ADDR, (char *)ucBuffer, 64, USB_TIMEOUT);
			
			if(iRet>1)
			{
				printf("Got %d bytes from HR4000.\n\r",iRet);
				int iLoop = 0;
				for(iLoop = 0; iLoop < iRet; iLoop++)
				{
					*pSpectralBuffer = ucBuffer[iLoop];
					pSpectralBuffer++;
					iByteCount++;
					printf("%02x ",ucBuffer[iLoop]);
				}; 
				printf("\n\r");
			};
				
			if(iRet==64)
				iNumberOfPackets++;
				
			if(iRet==1 && ucBuffer[0] == 0x69)
			{
				printf("Got Sync after %03d packets, read %04d bytes total.\n\r", 
					iNumberOfPackets,
					iByteCount);
				break;
			};
		};
	}
	else
	// **************************************************************
	// * in high speed mode the first 2K of data are sent via EP6_IN
	// * e.g. packets #00-#03 and then switched to EP2_IN where
	// * packets#04-#14 are sent. 
	// * packet #15 is a sync packet
	// **************************************************************
	{
		printf("Aquiring spectral data in HIGH SPEED mode (480mbit)\n\r");
		ucBuffer[0] = CMD_REQUEST_SPECTRA;
		iRet = usb_bulk_write(specHandle, EP1_OUT_ADDR, (char*)ucBuffer, 1, USB_TIMEOUT);
		if(iRet < 0)
		{
			printf("CMD_REQUEST_SPECTRA failed!\n\r");
			// cleanup
			usb_release_interface(specHandle,0);
			usb_close(specHandle);
			exit(-1);
		}
		else
		{
			printf("CMD_REQUEST_SPECTRA Command sent!\n\r");
		};
		
		iByteCount = 0;
		iNumberOfPackets = 0;
		pSpectralBuffer = (unsigned char*)(&uiSpectralData);
		
		
		while(1)
		{
			// check which endpoint to use
			if(iNumberOfPackets < 4)
				iRet = usb_bulk_read(specHandle,EP6_IN_ADDR, (char *)ucBuffer, 512, USB_TIMEOUT);
			else
				iRet = usb_bulk_read(specHandle,EP2_IN_ADDR, (char *)ucBuffer, 512, USB_TIMEOUT);

			// check if we got data, and it's not a sync packet
			if(iRet>1)
			{
				if(iNumberOfPackets < 4)
					printf("Got %d bytes from HR4000 on EP6_IN, packet #%04d\n\r",iRet,iNumberOfPackets);
				else
					printf("Got %d bytes from HR4000 on EP2_IN, packet #%04d\n\r",iRet,iNumberOfPackets);
					
				int iLoop = 0;
				for(iLoop = 0; iLoop < iRet; iLoop++)
				{
					*pSpectralBuffer = ucBuffer[iLoop];
					pSpectralBuffer++;
					iByteCount++;
					printf("%02x ",ucBuffer[iLoop]);
				}; 
				printf("\n\r");
			};
			
			// it was a data packet, so increment packet count
			if(iRet==512)
				iNumberOfPackets++;
			
			// if packetsize is 1 and the byte is 0x69, it's a sync packet	
			if(iRet==1 && ucBuffer[0] == 0x69)
			{
				printf("Got Sync after %03d packets, read %04d bytes total.\n\r", 
					iNumberOfPackets,
					iByteCount);
				break;
			};
		
		};
	}
	if(iByteCount==sizeof(uiSpectralData))
	{
		printf("Spectral size matches !\n\r");
		int iLoop = 0;
		int iRelevantPixels = 0;
		int iStartOfSpectrum = 0;
		
		for(iLoop=0; iLoop < (sizeof(uiSpectralData)>>1);iLoop++)
		{
			dSpectralWavelengths[iLoop] = (dCoeffs[0])+
													((double)iLoop * dCoeffs[1]) +
													((double)iLoop * (double)iLoop * dCoeffs[2])+
													((double)iLoop * (double)iLoop * (double)iLoop * dCoeffs[3]);

			if((dSpectralWavelengths[iLoop] >= 610.00) && (dSpectralWavelengths[iLoop] < 620.00))
			{
				if(iStartOfSpectrum == 0)		// memorize the pixel which is the first > 610nm
					iStartOfSpectrum = iLoop;
			
				iRelevantPixels++;
				printf("Wavelength for dataset point #%04d is %3.9f nm , readout is %05d\n\r",
					iLoop, 
					dSpectralWavelengths[iLoop],
					uiSpectralData[iLoop]); 
			}
		};
		printf("%04d Pixels are in the range from 610 to 620nm\n\r",iRelevantPixels);
		printf("Start of Spectrum is at: %04d\n\r",iStartOfSpectrum);
		
		DFBSurfaceDescription dsc;
		DFBCHECK (DirectFBCreate (&dfb));
		DFBCHECK (dfb->SetCooperativeLevel (dfb, DFSCL_FULLSCREEN));
		dsc.flags = DSDESC_CAPS;
		dsc.caps  = DSCAPS_PRIMARY | DSCAPS_FLIPPING;
		DFBCHECK (dfb->CreateSurface( dfb, &dsc, &primary ));
		DFBCHECK (primary->GetSize (primary, &screen_width, &screen_height));
		DFBCHECK (primary->FillRectangle (primary, 0, 0, screen_width, screen_height)); // en schworzen bildschirm
		DFBCHECK (primary->SetColor (primary, 0x80, 0x80, 0xff, 0xff));
		

		int iStartOfLineX = 0;
		int iStartOfLineY = 0;
		int iEndOfLineX = 0;
		int iEndOfLineY = 0;
		int iMaxValue = 0;
		int iMinValue = 65536;
		int iDeltaY = 0;
		// find minima and maxima
		for(iLoop = 0; iLoop < (iRelevantPixels-1); iLoop++)
		{
		if(uiSpectralData[iLoop+iStartOfSpectrum] < iMinValue)
			iMinValue = uiSpectralData[iLoop+iStartOfSpectrum];
		if(uiSpectralData[iLoop+iStartOfSpectrum] > iMaxValue)
			iMaxValue = uiSpectralData[iLoop+iStartOfSpectrum];
		}

		printf("Min: %05d\n\r", iMinValue);
		printf("Max: %05d\n\r", iMaxValue);
		
		iDeltaY = iMaxValue - iMinValue;
		printf("Delta is: %05d\n\r", iDeltaY);

		int iPixelPerDigitX = screen_width / iRelevantPixels;
		printf("%05d pixels per dataset for X axis\n\r", iPixelPerDigitX);
				
		int iPixelPerCountY = screen_height / iDeltaY;
		printf("%05d pixels per count for Y axis\n\r", iPixelPerCountY);
		
		return;
		
		DFBCHECK (primary->DrawLine (primary, 0, screen_height-1, screen_width - 1, screen_height-1)); // plot X Axis
		DFBCHECK (primary->DrawLine (primary, 0, 0 , 0 , screen_height-1)); // plot Y Axis

		for(iLoop = 0; iLoop < (iRelevantPixels-1); iLoop++)
		{
			iStartOfLineX = iLoop * iPixelPerDigitX;
			iStartOfLineY = (screen_height-1) - ((uiSpectralData[iLoop+iStartOfSpectrum]-iMinValue)*iPixelPerCountY);
			iEndOfLineX = (iLoop+1) * iPixelPerDigitX;
			iEndOfLineY = (screen_height-1) - ((uiSpectralData[iLoop+iStartOfSpectrum+1]-iMinValue)*iPixelPerCountY);
			DFBCHECK (primary->DrawLine (primary, iStartOfLineX, iStartOfLineY , iEndOfLineX , iEndOfLineY)); //plot data
		}
		DFBCHECK (primary->Flip (primary, NULL, 0));
		sleep (5);
		primary->Release( primary );
		dfb->Release( dfb );
	}
	else
	{
		printf("Spectral size is wrong!\n\r");
	};
};		
