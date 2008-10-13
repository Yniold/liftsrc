/*
 * $RCSfile: testccram.c,v $ last changed on $Date: 2008-10-13 16:03:46 $ by $Author: harder $
 *
 * $Log: testccram.c,v $
 * Revision 1.3  2008-10-13 16:03:46  harder
 * print results to file
 *
 * Revision 1.2  2008/10/07 20:25:36  harder
 * CC not in deb mode, wait msleep(1), reset counter
 *
 * Revision 1.1  2008/10/07 18:08:01  rudolf
 * added basic testsoft for CC2
 *
 *
 *
 *
 */

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <math.h>
#include <time.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>

#include "../include/elekGeneral.h"
#include "../include/elekIO.h"
#include "../include/elekIOPorts.h"

#include "testccram.h"

#undef DEBUG_CC_ADC

#define CC_BASE (0xD000)

// plain text name of each ADC channel
//
unsigned char aChannelDescription[8][32]=
{
   "PMT Threshold                 ",
     "ext. SYNC Threshold           ",
     "MCP1 Comperator Threshold     ",
     "+15 V DC Supply               ",
     "+5 V DC Supply                ",
     "+28 V DC Supply               ",
     "+3,3 V DC Supply              ",
     "+1,2 V Cyclone II Core Voltage"
};

// lookuptable needed because some ADC channels are connected in "5Volts-range-style" and some are not
//
unsigned char aChannelIs5VoltRange[8] =
{
   1, // PMT Thresh
     1, // Laser Thresh
     1, // MCP1 Comp Thresh
     0, // MCP2 Comp Thresh
     0, // +5V
     0, // +28V
     1, // +3,3V
     0  // +1,5 Core
};

// external voltage divider ratios
//
double dChannelScaleFactors[8]=
{
   1.00,
     1.00,
     1.00,
     7.80,
     3.00,
     16.00,
     1.00,
     1.00
};

int retval = 0;

// wrapper for nanosleep to continue in case we get an early wakeup
int msleep(unsigned long milisec)
{
  struct timespec req={0};
  time_t sec=(int)(milisec/1000);
  milisec=milisec-(sec*1000);
  req.tv_sec=sec;
  req.tv_nsec=milisec*1000000L;
  while(nanosleep(&req,&req)==-1)
    continue;
  return 1;
}

int main(int argc, char **argv)
{
  int iLoop = 0;
  int iIntTime = 1;
  FILE *fp;
  char strFname[80]="Gandalf.asc";

  if (argc>0) 
    { 
      iIntTime=atoi(argv[1]);
      if (argc>1)
	strncpy(strFname,argv[2],80);
    };
  printf("Integrating %d seconds. Using filename %s\n",iIntTime, strFname);

  if (elkInit())
    {
      // grant IO access
      printf("Error: failed to grant IO access rights\n");
      exit(EXIT_FAILURE);
    }

   if(elkReadData(CC_BASE + 2* OFF_CC2_SIG) != CC2_SIGNATURE)
     {
	printf("Error: failed to read CC2 signature for CC2@ 0x%04X \n", CC_BASE);
	printf("Check DIP switch or call 357...\n");
	exit(-1);
     }
   else
     {
	printf("found a Counter Card 2 with Revision %04d @0x%04X\n", elkReadData(CC_BASE + 2*OFF_CC2_REV),CC_BASE);

	for(iLoop = 0; iLoop < 8; iLoop++)
	  {
	     double dValue;
	     retval = elkReadData(CC_BASE + 2* (OFF_CC2_ADC0+iLoop));
	     retval = retval & 0xfff;
	     printf("Raw: 0x%04X: ",retval);
	     dValue = CalculateVoltage(aChannelIs5VoltRange[iLoop],retval,dChannelScaleFactors[iLoop]);

	     printf("%s: %04.2f\n",aChannelDescription[iLoop],dValue);
	  };

     };
   printf("Setting Pulse width to 500 ns\n");
   elkWriteData(CC_BASE + 2 * OFF_CC2_PULSEW, 0x7a);

   printf("Setting Pause width to 500 ns\n");
   elkWriteData(CC_BASE + 2 * OFF_CC2_PAUSEW, 0x7b);

   printf("Setting Count Delay to 4  ns\n");
   elkWriteData(CC_BASE + 2 * OFF_CC2_CNTDEL, 2);

   printf("Triggering Counter Copy Cycle\n");
   elkWriteData(CC_BASE + 2 * OFF_CC2_CTRL, CTRL_STARTCOPY);

   printf("\n\rCyclone II CC: Dumping FPGA Memory:\n\r");
   printf("====================================\n\r");

   for(iLoop = 0; iLoop < 256; iLoop++)
     {
       retval = elkReadData(CC_BASE + 2*iLoop);
       
       printf("0x%04X ",retval);
       
       if(iLoop % 8 == 7)
	 printf("\n\r");
     };
   sleep(1);
   
   while(1)
     {
       printf("Starting Shiftreg Debugmode and copy cycle\n");
       elkWriteData(CC_BASE + 2 * OFF_CC2_CTRL, 0x0001);
       msleep(1); // wait 1ms before reading memory

       printf("\n\rCyclone II CC: Dumping FPGA Memory:\n\r");
       printf("====================================\n\r");
       
       for(iLoop = 0; iLoop < 256; iLoop++)
	 {
	   retval = elkReadData(CC_BASE + 2*iLoop);
	   
	   printf("0x%04X ",retval);
	   
	   if(iLoop % 10 == 7)
	     printf("\n\r");
	 };
       printf("Reset and continue...\n");
//       elkWriteData(CC_BASE + 2 * OFF_CC2_CTRL, 0x8000);

       msleep(1000*iIntTime);
       
       fp=fopen(strFname,"a+");
       for(iLoop = 0; iLoop < 256; iLoop++)
	 {
	   retval = elkReadData(CC_BASE + 2*iLoop);
	   
	   fprintf(fp,"0%d ",retval);
	   
	   //	   if(iLoop % 10 == 7)
//	     fprintf(fp,"\n");
	 };
       fprintf(fp,"\n");
       fclose(fp);       
     };
   /*
	printf("\n\rCyclone II CC: Writing Testpattern:\n\r");
	printf("\n\rAddress equals Content\n\r");
	printf("====================================\n\r");

	for(iLoop = 0; iLoop < 256; iLoop++)
	  {
	     elkWriteData(CC_BASE + 2*iLoop, iLoop);
	  };
	printf("\n\rCyclone II CC: Dumping FPGA Memory:\n\r");
	printf("====================================\n\r");

	for(iLoop = 0; iLoop < 256; iLoop++)
	  {
	     retval = elkReadData(CC_BASE + 2*iLoop);

	     printf("0x%04X ",retval);

	     if(iLoop % 8 == 7)
	       printf("\n\r");
	  };

	sleep(5);
	printf("\n\rCyclone II CC: Writing Testpattern:\n\r");
	printf("\n\r0x55AA:\n\r");
	printf("====================================\n\r");

	for(iLoop = 0; iLoop < 256; iLoop++)
	  {
	     elkWriteData(CC_BASE + 2*iLoop, 0x55AA);
	  };

	sleep(5);
	printf("\n\rCyclone II CC: Dumping FPGA Memory:\n\r");
	printf("====================================\n\r");

	for(iLoop = 0; iLoop < 256; iLoop++)
	  {
	     retval = elkReadData(CC_BASE + 2*iLoop);

	     printf("0x%04X ",retval);

	     if(iLoop % 8 == 7)
	       printf("\n\r");
	  };
	sleep(5);

	printf("\n\rCyclone II CC: Writing Testpattern:\n\r");
	printf("\n\r0xAA55:\n\r");
	printf("====================================\n\r");

	for(iLoop = 0; iLoop < 256; iLoop++)
	  {
	     elkWriteData(CC_BASE + 2*iLoop, 0xAA55);
	  };

	sleep(5);
	printf("\n\rCyclone II CC: Dumping FPGA Memory:\n\r");
	printf("====================================\n\r");

	for(iLoop = 0; iLoop < 256; iLoop++)
	  {
	     retval = elkReadData(CC_BASE + 2*iLoop);

	     printf("0x%04X ",retval);

	     if(iLoop % 8 == 7)
	       printf("\n\r");
	  };
	sleep(5);
     };
*/
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
