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
#include <asm/msr.h>

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


