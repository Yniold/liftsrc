// ============================================
// Posix compatible serial interface
// Headerfile
// ============================================

// $RCSfile: serial.h,v $ last changed on $Date: 2005/01/27 14:59:36 $ by $Author: rudolf $

// History:
//
// $Log: serial.h,v $
// Revision 1.1  2005/01/27 14:59:36  rudolf
// added files for GPS receiver connection
//
//

#ifndef SERIAL_H
#define SERIAL_H

extern int serial_open(char * port, int baud);
extern void serial_close(int fd);

#endif // SERIAL_H
