// ===================================================================
// Serial interface
// ===================================================================
// implements serial_open() and serial_close()
// will handle terminal settings correctly
// (Parity, binary mode, stopbits, etc....)
//
// set *progname to something meaningful (e.g. name of main program)
// set int verboselevel to control debug messages
// ===================================================================
//
// $RCSfile: serial.c,v $ last changed on $Date: 2006-08-30 15:56:30 $ by $Author: rudolf $
//
// History:
//
// $Log: serial.c,v $
// Revision 1.1  2006-08-30 15:56:30  rudolf
// started writing new elekIOcalib based on elekIOserv
//
// Revision 1.1  2005/01/27 14:59:36  rudolf
// added files for GPS receiver connection
//
//

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/time.h>

#include <fcntl.h>
#include <termios.h>
#include <unistd.h>

extern char *progname;
extern int verbose;

// mapping

struct baud_mapping {
  long baud;
  speed_t speed;
};

// Baudrate lookup table

static struct baud_mapping baud_lookup_table [] = {
  { 1200,   B1200 },
  { 2400,   B2400 },
  { 4800,   B4800 },
  { 9600,   B9600 },
  { 19200,  B19200 },
  { 38400,  B38400 },
  { 57600,  B57600 },
  { 115200, B115200 },
  { 230400, B230400 },
  { 0,      0 }
};

static speed_t serial_baud_lookup(long baud)
{
  struct baud_mapping *map = baud_lookup_table;

  while (map->baud) {
    if (map->baud == baud)
      return map->speed;
    map++;
  }

  fprintf(stderr, "%s: serial_baud_lookup(): unknown baud rate: %ld", 
          progname, baud);
  exit(1);
}

static int serial_setattr(int fd, long baud)
{
  int rc;
  struct termios termios;
  speed_t speed = serial_baud_lookup (baud);
  
  if (!isatty(fd))
    return -1;
  
  // initialize terminal modes

  rc = tcgetattr(fd, &termios);
  if (rc < 0) {
    fprintf(stderr, "%s: serial_setattr(): tcgetattr() failed, %s", 
            progname, strerror(errno));
    return -errno;
  }

  termios.c_iflag = 0;
  termios.c_oflag = 0;
  termios.c_cflag = 0;
  termios.c_cflag |=   (CS8 | CREAD | CLOCAL);
  termios.c_lflag = 0;
  termios.c_cc[VMIN]  = 1;
  termios.c_cc[VTIME] = 0;

  cfsetospeed(&termios, speed);
  cfsetispeed(&termios, speed);
  
  rc = tcsetattr(fd, TCSANOW, &termios);
  if (rc < 0) {
    fprintf(stderr, "%s: serial_setattr(): tcsetattr() failed, %s", 
            progname, strerror(errno));
    return -errno;
  }

  return 0;
}


int serial_open(char * port, int baud)
{
  int rc;
  int fd;

  // open the serial port

  fd = open(port, O_RDWR | O_NOCTTY | O_NONBLOCK);
  if (fd < 0) {
    fprintf(stderr, "%s: serial_open(): can't open device \"%s\": %s\n",
            progname, port, strerror(errno));
    exit(1);
  }

   // set serial line attributes

  rc = serial_setattr(fd, baud);
  if (rc) {
    fprintf(stderr,
            "%s: serial_open(): can't set attributes for device \"%s\"\n",
            progname, port);
    exit(1);
  }

  return fd;
}


void serial_close(int fd)
{
  // FIXME: Should really restore the terminal to original state here.

  close(fd);
}


