/*
 * $RCSfile: drivertester.c,v $ last changed on $Date: 2005-09-14 16:05:00 $ by $Author: rudolf $
 *
 * $Log: drivertester.c,v $
 * Revision 1.2  2005-09-14 16:05:00  rudolf
 * fix
 *
 *
 *
 */
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/time.h>

#include <fcntl.h>
#include <unistd.h>

#include "drivertester.h"

// plain text name of each ADC channel

unsigned char aChannelDescription[8][32]=
{
	"PMT Threshold                 ",
	"Laser Threshold               ",
	"MCP1 Comperator Threshold     ",
	"MCP2 Comperator Threshold     ",
	"+5Volts DC Supply             ",
	"+28Volts DC Supply            ",
	"+3,3Volts DC Supply           ",
	"+1,5Volts Cyclone Core Voltage"
};

// lookuptable needed because some ADC channels are connected in "5Volts-range-style" and some are not

unsigned char aChannelIs5VoltRange[8] =
{
	1, // PMT Thresh
	1, // Laser Thresh
	1, // MCP1 Comp Thresh
	1, // MCP2 Comp Thresh
	0, // +5V
	0, // +28V
	1, // +3,3V
	0  // +1,5 Core
};

// external voltage divider ratios

double dChannelScaleFactors[8]=
{
	1.00,
	1.00,
	1.00,
	1.00,
	3.00,
	16.00,
	1.00,
	1.00
};

int retval = 0;

int main()
{
	int iLoop = 0;
	int uiVentState = 0;
	fd = 0;
	
	printf("Trying to open /dev/serbus....\n\r");

	fd = open("/dev/serbus",O_RDWR);

	// check for errors
	if (fd < 0)
	{
		perror("Error during open() ");
		exit(1);
	};

	printf("Device opened, FD is %d\n\r",fd);

	// turn of debug messages

	printf("Turning off debug output from serbus driver via IOCTL\n\r");
	retval = ioctl(fd, SERBUS_IOCSDEBUGOFF);

	if(retval < 0)
		perror("Error during ioctl() ");

	// init cards
	
	ioctl(fd, SERBUS_IOCTWRITEWORD, 0xa404 + (0x00FF << 16)); // PWM for FlowController
	
	ioctl(fd, SERBUS_IOCTWRITEWORD, 0xa460 + (0x0690 << 16)); // VentCard#1 (12Volt)
	ioctl(fd, SERBUS_IOCTWRITEWORD, 0xa462 + (0x0690 << 16)); // VentCard#2 (12Volt)
	
	ioctl(fd, SERBUS_IOCTWRITEWORD, 0xa408 + (0x3fff << 16)); // VentCard#1 (all on)
	ioctl(fd, SERBUS_IOCTWRITEWORD, 0xa40A + (0x3fff << 16)); // VentCard#2 (all on)

	// init ADC 16bit #1
		
	ioctl(fd, SERBUS_IOCTWRITEWORD, 0xa490 + (0x0000 << 16));
	ioctl(fd, SERBUS_IOCTWRITEWORD, 0xa492 + (0x0001 << 16));
	ioctl(fd, SERBUS_IOCTWRITEWORD, 0xa494 + (0x0002 << 16));
	ioctl(fd, SERBUS_IOCTWRITEWORD, 0xa496 + (0x0003 << 16));
	ioctl(fd, SERBUS_IOCTWRITEWORD, 0xa498 + (0x0004 << 16));
	ioctl(fd, SERBUS_IOCTWRITEWORD, 0xa49A + (0x0005 << 16));
	ioctl(fd, SERBUS_IOCTWRITEWORD, 0xa49C + (0x0006 << 16));
	ioctl(fd, SERBUS_IOCTWRITEWORD, 0xa49E + (0x0007 << 16));
	
	// init ADC 16bit #2
		
	ioctl(fd, SERBUS_IOCTWRITEWORD, 0xa4B0 + (0x0000 << 16));
	ioctl(fd, SERBUS_IOCTWRITEWORD, 0xa4B2 + (0x0001 << 16));
	ioctl(fd, SERBUS_IOCTWRITEWORD, 0xa4B4 + (0x0002 << 16));
	ioctl(fd, SERBUS_IOCTWRITEWORD, 0xa4B6 + (0x0003 << 16));
	ioctl(fd, SERBUS_IOCTWRITEWORD, 0xa4B8 + (0x0004 << 16));
	ioctl(fd, SERBUS_IOCTWRITEWORD, 0xa4BA + (0x0005 << 16));
	ioctl(fd, SERBUS_IOCTWRITEWORD, 0xa4BC + (0x0006 << 16));
	ioctl(fd, SERBUS_IOCTWRITEWORD, 0xa4BE + (0x0007 << 16));
	
	// init ADC 24 Bit Card #1

	// clear ADC24 ram
	
	for(iLoop = 0; iLoop < 128; iLoop++)
		ioctl(fd, SERBUS_IOCTWRITEWORD, (iLoop*2+0xa500)+ ((0) << 16));
	
	// switch on DCDC on card	
	ioctl(fd, SERBUS_IOCTWRITEWORD, 0xa510 + (0x00F0 << 16));
	
	
	// do ADC readout

	while(1)
	{
		sleep(1);
		
		uiVentState = ioctl(fd, SERBUS_IOCHREADWORD, 0xA40A);
		uiVentState = uiVentState ^ (1<<13);
		ioctl(fd, SERBUS_IOCTWRITEWORD, 0xA40A + (uiVentState<<16));
		
		for(iLoop = 0; iLoop < 8; iLoop++)
		{
			retval = ioctl(fd, SERBUS_IOCHREADWORD, 0xA300 + 2*iLoop);
	
			if(retval < 0)
				perror("Error during ioctl() ");
	
			retval = retval & 0xFFF; // ADC data is 12bit, so drop MS nibble (channelnumber)
	
			printf("Reading ADC Channel #%d (%s): Value 0x%03X ",iLoop,aChannelDescription[iLoop],retval);
			printf("Voltage: %f\n\r", CalculateVoltage(aChannelIs5VoltRange[iLoop], retval, dChannelScaleFactors[iLoop]));
		};
	
		printf("\n\rTemperaturecard: Dumping FPGA Memory:\n\r");
		printf("=====================================\n\r");
		
		for(iLoop = 0; iLoop < 128; iLoop++)
		{
			retval = ioctl(fd, SERBUS_IOCHREADWORD, 0xB000 + 2*iLoop);
	
			if(retval < 0)
				perror("Error during ioctl() ");
	
			printf("0x%04X ",retval);
	
			if(iLoop % 8 == 7)
				printf("\n\r");
		};
		
		printf("\n\r24bit AIN8eXX: Dumping 24bit ADC Memory:\n\r");
		printf("==============================================\n\r");
		
		printf("Card Signature: ");
		
		for(iLoop = 0; iLoop < 8; iLoop++)
		{
			retval = ioctl(fd, SERBUS_IOCHREADWORD, 0xA500 + 2*iLoop);
			printf("%c%c",(char)(retval & 0x00FF),(char)(retval >> 8));
		};
		printf("\n");
		
		for(iLoop = 0; iLoop < 64; iLoop++)
		{
			retval = ioctl(fd, SERBUS_IOCHREADWORD, 0xa500 + 2*iLoop);
	
			if(retval < 0)
				perror("Error during ioctl() ");
	
			if(iLoop %8 == 0)
				printf("0x%04x: ",0xA500+2*iLoop);
				
			printf("0x%04X ",retval);
	
			if(iLoop % 8 == 7)
				printf("\n\r");
		};
		
		for(iLoop = 0; iLoop < 8; iLoop++)
		{
			printf("Channel#%d: %08xl\n\r", iLoop, ReadADC24Channel(iLoop));
		};
		
		printf("\n\r16bit AIN8XX #1: Dumping 16bit ADC Memory:\n\r");
		printf("==============================================\n\r");
		
		for(iLoop = 0; iLoop < 8; iLoop++)
		{
			retval = ioctl(fd, SERBUS_IOCHREADWORD, 0xa480 + 2*iLoop);
	
			if(retval < 0)
				perror("Error during ioctl() ");
	
			if(iLoop %8 == 0)
				printf("0x%04x: ",0xA480+2*iLoop);
				
			printf("%05d ",retval);
	
			if(iLoop % 8 == 7)
				printf("\n\r");
		};
	
		printf("\n\r16bit AIN8XX #2: Dumping 16bit ADC Memory:\n\r");
		printf("==============================================\n\r");
		
		for(iLoop = 0; iLoop < 8; iLoop++)
		{
			retval = ioctl(fd, SERBUS_IOCHREADWORD, 0xa4A0 + 2*iLoop);
	
			if(retval < 0)
				perror("Error during ioctl() ");
	
			if(iLoop %8 == 0)
				printf("0x%04x: ",0xA4A0+2*iLoop);
				
			printf("%05d ",retval);
	
			if(iLoop % 8 == 7)
				printf("\n\r");
		};
		
		printf("\n\rMFC Actual Readouts:\n\r");
		printf("==============================================\n\r");
		
		for(iLoop = 0; iLoop < 4; iLoop++)
		{
			retval = ioctl(fd, SERBUS_IOCHREADWORD, 0xa4C0 + 2*iLoop);
	
			if(retval < 0)
				perror("Error during ioctl() ");
	
			if(iLoop %4 == 0)
				printf("0x%04x: ",0xA4C0+2*iLoop);
				
			printf("%05d ",retval);
	
			if(iLoop % 4 == 3)
				printf("\n\r");
		};
	};
	
	// close
	printf("Closing Device...\n\r");
	close(fd);
};

//===============================================================================================================
// function to convert 16bit ADC readout into floating point voltage (upper 4 bits of uiADCCounts have to be 0)
//===============================================================================================================

double CalculateVoltage(unsigned char ucIs5VoltRange, unsigned int uiADCCounts, double ScaleFactor)
{
	double dVoltage = 0;
	double dDivisor = 4096;

	if(ucIs5VoltRange)
	{
		dVoltage = ScaleFactor * 5.0 * (((double)uiADCCounts)/ dDivisor);
	}
	else
	{

		dVoltage = ScaleFactor * 2.5 * (((double)uiADCCounts) / dDivisor);
	};
	return dVoltage;
};

long ReadADC24Channel(int iChannelNumber)
{
	int iChannelOffset = 0;
	int iValue = 0;
	
	// sanity checks
	
	if((iChannelNumber < 0) || (iChannelNumber > 7))
	{
		perror("invalid channel!\n");
		exit(-1);
	};
	
	iChannelOffset = (iChannelNumber << 2);
	
	iValue = ioctl(fd, SERBUS_IOCHREADWORD, 0xA520 + iChannelOffset);
	iValue &= 0xFFFF;
	
	// check if negative
	
	if(iValue & 0x8000)
	{
		return((long)(((ioctl(fd, SERBUS_IOCHREADWORD, 0xA520 + iChannelOffset+2))<<16)+\
		iValue + 0x8000));
	}
	else
	{
		return((long)(((ioctl(fd, SERBUS_IOCHREADWORD, 0xA520 + iChannelOffset+2))<<16)+\
		iValue));
	};
}
