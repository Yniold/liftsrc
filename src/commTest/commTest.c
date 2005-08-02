/*
 * $RCSfile: commTest.c,v $ last changed on $Date: 2005-08-02 11:18:30 $ by $Author: rudolf $
 *
 * $Log: commTest.c,v $
 * Revision 1.1  2005-08-02 11:18:30  rudolf
 * added short test software for serial communications
 *
 * 
 *
 *
 */
 
#include "serial.h"
#include <stdio.h>

unsigned char prog[256] = "comTest";

char *progname = 0;
int verbose = 255;
long baud = 115200;

// main()

int main(int argc, char *argv[]) 
{
	char *progname = prog;
	int fd = -1;
	int rc = -1;

	if(argc < 3)
	{
		printf("%s serial COM port test software\n\r",argv[0]);
		printf("usage: %s serport text\n\r",argv[0]);
		printf("e.g. %s /dev/ttySO Testtext\n\r");
		exit(-1);
	};
	
	fd = serial_open(argv[1] , baud);
	if(fd == 1)
	{
		printf("Error opening %s !\n\r", argv[1]);
		exit(-1);
	};
	printf("Opened %s with %d BAUD!\n\r", argv[1], baud);
	printf("Filehandle is 0x%08X\n\r",fd);
	printf("Writing Testtext!\n\r");

	// write text
	int nBytes = 0;
	unsigned char *pText = argv[2];
	while(*pText)
	{
		nBytes++;
		pText++;
	};
	
	if(nBytes)
	{
		printf("Writing Text \"%s\"(%d bytes) to %s...\n\r",argv[2],nBytes,argv[1]);
		rc =write(fd, argv[2], nBytes);
	}
	else
	{
		close(fd);
		printf("Nothing to write!\n\r");
		exit(-1);
	};

	if(rc < 0)
		printf("Write failed\n\r");
	if(rc != nBytes)
	{
		printf("Write failed, written %d should have been %d!\n\r",rc,nBytes);
	};

	if(rc == nBytes)
	{
		printf("Wrote of %d bytes OK!\n\r",rc);
	};

 	serial_close(fd);

};
