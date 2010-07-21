/*
* $RCSfile: bitop.c,v $ last changed on $Date: 2005/04/21 14:08:18 $ by $Author: rudolf $
*
* $Log: bitop.c,v $
* Revision 1.3  2005/04/21 14:08:18  rudolf
* added revision history field
*
*
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#ifdef RUNONPC
#include <asm/msr.h>
#endif

/**********************************************************************/
/* bitset function to set bit N in WORD to VAL                        */
/**********************************************************************/

int bitset(int word, int n, int val) {

  int ret;
  int bitpos;

  bitpos=1<<n;
  if (val==0) ret=word & ~bitpos;
  else ret=word | bitpos;

  return (ret);

} /* bitset */


