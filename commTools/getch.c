/*
* $RCSfile: getch.c,v $ last changed on $Date: 2006/09/04 11:53:29 $ by $Author: rudolf $
*
* $Log: getch.c,v $
* Revision 1.2  2006/09/04 11:53:29  rudolf
* Fixed warnings for GCC 4.03, added newline and CVS revision info
*
*
*
*/
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

static struct termios new_io;
static struct termios old_io;

/* Funktion schaltet das Terminal in den cbreak-Modus:        */
/* Kontrollflag ECHO und ICANON auf 0 setzen                  */
/* Steuerzeichen: Leseoperation liefert 1 Byte VMIN=1 VTIME=1 */
int cbreak(int fd) {
   /*Sichern unseres Terminals*/
   if((tcgetattr(fd, &old_io)) == -1)
      return -1;
   new_io = old_io;
   /*Wir verändern jetzt die Flags für   den cbreak-Modus*/
   new_io.c_lflag = new_io.c_lflag & ~(ECHO|ICANON);
   new_io.c_cc[VMIN] = 0;
   new_io.c_cc[VTIME]= 0;

   /*Jetzt setzen wir den cbreak-Modus*/
   if((tcsetattr(fd, TCSAFLUSH, &new_io)) == -1)
      return -1;
   return 1;
}

int restoreinput(void){
   /*Alten Terminal-Modus wiederherstellen*/
   tcsetattr(STDIN_FILENO, TCSANOW, &old_io);
};

int getch(void) {
   int c;

   c = getchar();
   return c;
}
