#include <stdio.h>
#include <stdlib.h>


int TestChannelMask(int *Mask, int TimeSlot) {

   int Cell;
   int BitNo;
   int Bit;
   
   Cell=(int) (TimeSlot/16);
   BitNo=1<<(TimeSlot % 16);
   
   Bit=BitNo==(Mask[Cell] & BitNo);
   
   return(Bit);

} /* TestChannelMask */


int main () {

   int Mask[10];
   
   int i;
   
   
//   for (i=0;i<10;i++) Mask[i]=0x001;
   
//   Mask[4]=0xffff;
   
//   for(i=0;i<160; i++) printf("%d",TestChannelMask(Mask,i));
     
//   printf("\n");


  for (i=0;i<50;i++) printf("%d:%d   ",i,(int)i/15);
}


