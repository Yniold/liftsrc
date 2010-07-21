/***************************************************************************
   logerror.c  -  description
   -------------------
   begin                : Wed Oct 22 12:24:09 CEST 2003  by H.Harder
   email                : harder@mpch-mainz.mpg.de

responsible for errorlogging
provides:
void LogError (char *Message)
***************************************************************************/

#include <stdio.h>
#include <stdlib.h>

/**************************************************************************/
/* LogError writes Logmessage to stdout
 *
 */
/***************************************************************************/

void LogError (char *Message) {

    fprintf(stderr,"%s\n",Message);
    

} // LogError
