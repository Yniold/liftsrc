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

#define DEBUGLEVEL 10
// PC Version
//
int elkInit (void)
{

   int ret;

   ret=ioperm(ELK_BASE, 50, 1);
   return (ret);
}
//=====================================================================================================
int elkExit (void)
{

   int ret;

   ret=ioperm(ELK_BASE, 50, 0);                // we don't need it anymore
   return (ret);
}
//=====================================================================================================

int elkWriteData(uint16_t Adress, uint16_t Data)
{
#if (DEBUGLEVEL > 0)
   printf("Write Data 0x%04x to port 0x%04x\n",Data,Adress);
#endif

   outw(Data, Adress);
}
// end elkWriteData(uint16_t Adress, uint16_t Data)
//
//=====================================================================================================

int elkReadData(uint16_t Adress)
{
   int ret;
   uint16_t ElkReadData;

   ElkReadData   = inw(Adress);   
   return(ElkReadData);
}
//======================================================================================================

int main(int argc, char *argv[])
{

   int ret;
   int i;
   int ArgCount;
   int Addr;
   int Value;
   if(argc < 2)
     {
	printf("AT96 Test. Usage 'test (w/r) adress data\n'");
	exit(-1);
     };
   printf("Test program for AT96 backplane\n");

   printf(" init Linux rights..");
   ret=elkInit();
   printf(" returns : 0x%04x \n",ret);
   ArgCount=1; // next argument

   switch(argv[ArgCount++][0])
     {
      case 'r':
	Addr=strtol(argv[ArgCount],NULL,0);
	ArgCount++;
	Value=elkReadData((uint16_t) Addr);
	printf("Read %04x(%05d) : %04x(%05d)\n",Addr,Addr, Value, Value);
	printf("%d\n",Value);
	break;

      case 'w':
	if(argc>3) // better check here before messing around with argv[], hartwig :)
	  {
	     Addr=strtol(argv[ArgCount],NULL,0);
	     ArgCount++;
	     Value=strtol(argv[ArgCount],NULL,0);
	     ArgCount++;
	     ret=elkWriteData((uint16_t) Addr,(uint16_t)Value);
//	     printf("Wrote %4x=%4x(%5d) : %4x(%5d)\n",ret,Addr,Addr,Value,Value);
	     break;
	  }

      default:
	printf("Unknown Command\n");
	printf(" exit ..");
	ret=elkExit();
	printf(" returns : 0x%x \n",ret);
     }
   exit(0);
};
   /* main */

