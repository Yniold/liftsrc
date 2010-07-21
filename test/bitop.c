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
  printf("%d %x %x\n",n,bitpos,word); 
  if (val==0) ret=word & ~bitpos;
  else ret=word | bitpos;

  return (ret);

} /* bitset */


int main () {

  printf("%x %x %x %x\n",bitset(0xffff,4,0),bitset(0,4,1),
	 bitset(0xaa55,4,0),bitset(0xaa55,4,1)); 
}
