/*
* $RCSfile: elekIOTest.c,v $ last changed on $Date: 2007/09/11 13:28:06 $ by $Author: rudolf $
*
* $Log: elekIOTest.c,v $
* Revision 1.2  2007/09/11 13:28:06  rudolf
* added AT96 test software
*
* Revision 1.1  2007/02/13 17:19:25  martinez
* added AT96 Test program
*
* Revision 1.4  2006-09-04 11:40:36  rudolf
* Fixed warnings for GCC 4.03
*
* Revision 1.3  2005/04/21 15:01:58  rudolf
* made elekIO work with ARM controller. Added routines to open device driver for read and write to the backplane
*
* Revision 1.2  2005/04/21 14:11:42  rudolf
* added revision history field
*
*
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/io.h>
#include <stdint.h>

#include <ctype.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/time.h>

#include <fcntl.h>
#include <unistd.h>

#include "../include/elekIO.h"

#define TESTADR   0xa400
#define TESTWORD  0x55aa
#define LOOPCOUNT 10000

#define DEBUGLEVEL 10
#define RUNONPC 

#ifdef RUNONARM
	#define SERBUS_IOCSDEBUGON    0x40047301
	#define SERBUS_IOCSDEBUGOFF   0x40047302
	#define SERBUS_IOCTWRITEWORD  0x40047303
	#define SERBUS_IOCHREADWORD   0x40047304
#endif

#ifdef RUNONPC
//=====================================================================================================
// PC Version

int elkInit (void) {

     int ret;

    ret=ioperm(ELK_BASE, 10, 1);
    return (ret);
}
//=====================================================================================================
#endif

#ifdef RUNONARM
//=====================================================================================================
// ARM9 version
int fd = 0;

int elkInit (void)
{
	printf("Trying to open /dev/serbus...\n\r");
    fd = open("/dev/serbus",O_RDWR);

	if(fd < 0)
	{
		printf("Failed...\n\r");
    	return (1);
	}
	else
	{
		printf("Success!\n\r");
		ioctl(fd, SERBUS_IOCSDEBUGOFF);
		return (0);
	}
}
//=====================================================================================================
#endif

#ifdef RUNONARM
//=====================================================================================================
// ARM9 Version
int elkExit (void)
{
	close(fd);
	return (0);
}
//=====================================================================================================
#endif

#ifdef RUNONPC
int elkExit (void) {

    int ret;

    ret=ioperm(ELK_BASE, 10, 0);                // we don't need it anymore
    return (ret);
}
//=====================================================================================================
#endif

#ifdef RUNONPC
int elkWriteData(uint16_t Adress, uint16_t Data)
{

    unsigned Counter=ELK_TIMEOUT;
    uint16_t ElkQToDo=1;
    uint16_t ElkQReady;
    uint16_t ElkQReadyNew;
    uint16_t ElkQStatus;
    uint16_t ElkReadData;
    int ret;

#if (DEBUGLEVEL > 0)
    printf("Write adr %x to port %x and data %x to adr %x\n",Adress,ELK_ADR,Data,ELK_DATA);
#endif

    outw(Adress, ELK_ADR);
    outw(Data, ELK_DATA);
   sleep(1);
} // end elkWriteData(uint16_t Adress, uint16_t Data)

//=====================================================================================================
#endif

#ifdef RUNONPC
int elkReadData(uint16_t Adress) {

    unsigned Counter=ELK_TIMEOUT;
    uint16_t ElkQToDo=1;
    uint16_t ElkQReady;
    uint16_t ElkQReadyNew;
    uint16_t ElkQStatus;
    uint16_t ElkReadData;
    uint16_t ElkReadAdress;

    int ret;
    
    Adress=Adress | (uint16_t) 0x0001;            // LSB of Adress for ReadCommand has to be 1 

    ElkReadData = (uint16_t) 0;                  // init ElkReadData to something

//    outw(Adress, ELK_ADR);                      // Adress we want the data from
//    outw(ElkReadData, ELK_DATA);                       // write to Dataport to submit request

//    ElkReadAdress = inw(ELK_ADR);                         // get the address
    ElkReadData   = inw(ELK_DATA);                        // read on data and increase Tailptr

//	printf("Problem in ELKQueue, expected Adress %x, Read Address %x \n",Adress,ElkReadAdress);


    return(ElkReadData);


} // end elkReadData(uint16_t Adress, uint16_t Data)

//======================================================================================================
#endif

#ifdef RUNONARM
int elkWriteData(uint16_t Adress, uint16_t Data)
{
	return (ioctl(fd, SERBUS_IOCTWRITEWORD, (unsigned long)(Adress) + (unsigned long)(Data<<16)));
}

int elkReadData(uint16_t Adress)
{
	return (ioctl(fd, SERBUS_IOCHREADWORD, (unsigned long)(Adress)));
}

#endif

//======================================================================================================

int main ()
{

  int ret;
  int i;

  printf("Test program for AT96 backplane\n");

  printf(" init Linux rights..");
  ret=elkInit();
  printf(" returns : 0x%04x \n",ret);

  for (i=0;i<LOOPCOUNT ;i++) {
    printf(" write 0x%04x to address 0x%04x\n",TESTWORD,TESTADR);
    ret=elkWriteData(TESTADR, TESTWORD);

     printf(" read from address 0x%04x\n",TESTADR);
    ret=elkReadData(TESTADR);
    
     printf(" got: 0x%04x\n",ret);
  } /* for */


  printf(" exit ..");
  ret=elkExit();
  printf(" returns : 0x%x \n",ret);



} /* main */
    




