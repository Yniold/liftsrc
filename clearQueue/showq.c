// program to display Queue of AT96 Converter
// HH, Jul 03

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <asm/io.h>

#define DEBUGLEVEL 1

#define ELK_TIMEOUT (unsigned) 0x1000
#define ELK_BASE (unsigned)0x200
#define ELK_ADR  ELK_BASE
#define ELK_DATA (ELK_BASE + (unsigned) 2)
#define ELK_TODO (ELK_BASE + (unsigned) 4)
#define ELK_QSIZE 4


#define STAT_TIMEOUT 0
#define STAT_OK      1


int elkReadQueue() {

    unsigned Counter=0;
    unsigned QAdr;
    unsigned QData;
    unsigned ElkQPtr;
    unsigned ElkQBasePtr;
    unsigned    ElkQStatusBegin;
    unsigned ElkQStatusEnd;

    ElkQStatusBegin = inw(ELK_TODO);
    ElkQBasePtr=ElkQStatusBegin & 0x000f;  // get the current pointer so we know where we are
    ElkQPtr=ElkQBasePtr;

    do {
	QAdr=inw(ELK_ADR);
	QData=inw(ELK_DATA);
	printf("%04x - %04x. %04x %04x\n",ElkQBasePtr,ElkQPtr,QAdr,QData);
	ElkQPtr=inw(ELK_TODO) & 0x000f;
	Counter++;
    } while (ElkQPtr!=ElkQBasePtr);

    ElkQStatusEnd = inw(ELK_TODO);
    

    printf("QSize : %d QSTatStart %04x QStatEnd %04x\n",Counter,ElkQStatusBegin,ElkQStatusEnd);
    return(Counter);
    
} // end elkReadQueue

//=====================================================================================================

int main (int argc, char *argv[])
{
    int ret;
    int remove;
    int i;
    int QData;
    char ch;
   
    if (ioperm(ELK_BASE, 10, 1)) {                // grant IO access
	printf("Error: failed to grant IO access rights\n");
	exit(EXIT_FAILURE);
    }
    
    ret=elkReadQueue();

    if (argc>1) remove=strtod(argv[1],NULL);
 
    for (i=0;i<remove;i++) {
	QData=inw(ELK_DATA);
    }
    
    ret=elkReadQueue();


    ioperm(ELK_BASE, 10, 0);                // we don't need it anymore
	    
    exit(EXIT_SUCCESS);

}


    
    
