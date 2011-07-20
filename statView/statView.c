#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#define COMMANDLEN 256

int main (int argc, char* argv[])

{

    int      i;
    FILE     *gp1,*gp2,*gp3;
    char     dumstr[COMMANDLEN];
    
    

    snprintf (dumstr,COMMANDLEN,"gnuplot -geometry %s -title '%s'","360x308+0+346","PMT Channelview");      
    if (!(gp1=popen(dumstr,"w"))) {
	fprintf (stderr,"Could not call %s\n",dumstr);    
	exit (EXIT_FAILURE);
    }
    fprintf (gp1,"set xtics 0,%u\n",10);fflush(gp1);
    


    snprintf (dumstr,COMMANDLEN,"gnuplot -geometry %s -title '%s'","360x308+0+0","ADC Data");  
    if (!(gp2=popen(dumstr,"w"))) {
	fprintf (stderr,"Could not call %s\n",dumstr);    
	exit (EXIT_FAILURE);
    }
    fprintf (gp2,"set xtics 0,%u\n",10);fflush(gp2);
    

    
    
    
//    if (gp1) {
//	snprintf (dumstr,COMMANDLEN,"gawk -f %s %s > %s",p1_extract,status_tail,p1_datafile);
//	system (dumstr); /* This extracts the columns described in 'p1_extract' from 'status_tail'
//			    and creates the datafile for gnuplot from them                        */
//   }         
    

    while (1) {
	if (gp1) {fprintf (gp1,"load '%s'\n","chPMT.gpc");fflush(gp1);}      

	snprintf (dumstr,COMMANDLEN,"./extract > %s","ccADC_sub.txt");
	system (dumstr); 
	if (gp2) {fprintf (gp2,"load '%s'\n","ccADC.gpc");fflush(gp2);}      


	sleep (5);
    }
    
    if (gp1) pclose(gp1);
    if (gp2) pclose(gp2);

}
