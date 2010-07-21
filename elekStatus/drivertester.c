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
	int fd = 0;
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

	// do ADC readout

	int iLoop = 0;

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
