/* small program to test the communication with the SpectraPhysiscs Laser */
/* HH Dez 2004 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <termios.h>

#define BAUDRATE B9600
#define LASERDEVICE "/dev/ttyS1"
#define _POSIX_SOURCE 1

#define FALSE 0
#define TRUE 1

main () 
{
  int      fd,c,res;
  struct termios oldtio,newtio;
  char buf[255];

  fd=open(LASERDEVICE,O_RDWR | O_NOCTTY );
  if (fd<0) { perror(LASERDEVICE); exit(-1);




