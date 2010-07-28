#include "../include/elekIO.h"
#include "cgigateway.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/time.h>
#include <unistd.h>
#include <string.h>

#include "time.h"

#define MAXBUFFER 		(1024)
struct GSBStatusType * pGSBStatus;

 ePages ePageToDisplay = E_UNKNOWN;

int main()
{
    int iShmHandle;
    key_t ShmKey;
    char *shm, *s, *pQuery;
	int iLoop;
	char aBuffer[MAXBUFFER];

	pQuery = getenv("QUERY_STRING");
	if(pQuery == NULL)
    {
		printf("Content-type: text/html\n\n") ;
		printf("<html>\n") ;
		printf("<head><title>GSB Webinterface</title></head>\n") ;
  		printf("<meta http-equiv=\"Refresh\" content=\"2; url=http://%s/cgi-bin/cgigateway.cgi?page=status\"/>\n",getenv("SERVER_NAME"));
		printf("</head\n") ;
		printf("<body>\n") ;
		printf("<h1>Error: wrong or none parameters passed to CGI program</h1>\n") ;
		printf("<p>Please follow this <a href=\"http://%s/cgi-bin/cgigateway.cgi?page=index\"> link</a>!</p>\n",getenv("SERVER_NAME"));
		printf("Redirecting you in 2 seconds.\n");
		printf("</body>\n") ;
		printf("</html>\n") ;
		exit(0) ;
    }
	else 
	if(sscanf(pQuery,"page=%s",aBuffer)!=1)
    {
		printf("Content-type: text/html\n\n") ;
		printf("<html>\n") ;
		printf("<head><title>GSB Webinterface</title></head>\n") ;
		printf("<body>\n") ;
		printf("<h1>Error: sscanf() failed</h1>\n") ;
		printf("Wrong parameters passed?");
		printf("</body>\n") ;
		printf("</html>\n") ;
		exit(0) ;
    }
    else
    {
    	if(strstr(aBuffer,"status") != NULL)
    		ePageToDisplay = E_PAGE_STATUS;
		else    	
    	if(strstr(aBuffer,"debug") != NULL)
    		ePageToDisplay = E_PAGE_DEBUG;
    	else
    		ePageToDisplay = E_UNKNOWN;
    };

	// helpers for time functions
	struct tm sBrokenUpTime;
    time_t    Seconds;

	// ID for the shared memory segment
    ShmKey = GSB_SHMKEY;

	// search segment and get a handle to it
    if ((iShmHandle = shmget(ShmKey, sizeof(struct GSBStatusType), 0666)) < 0) 
    {
		printf("Content-type: text/html\n\n") ;
		printf("<html>\n") ;
		printf("<head><title>GSB Webinterface</title></head>\n") ;
		printf("<body>\n") ;
		printf("<h1>Error: Can't find shared memory key!</h1>\n") ;
		printf("Most probably elekIOGSB process is not running.");
		printf("</body>\n") ;
		printf("</html>\n") ;
		exit(0) ;
    }

	// attach shm segment to my address-space
    if ((shm = shmat(iShmHandle, NULL, 0)) == (char *) -1) 
    {
		printf("Content-type: text/html\n\n") ;
		printf("<html>\n") ;
		printf("<head><title>GSB Webinterface</title></head>\n") ;
		printf("<body>\n") ;
		printf("<h1>Error: Attaching SHM failed!</h1>\n") ;
		printf("</body>\n") ;
		printf("</html>\n") ;
		exit(0) ;
    }
	
	// pointer points to our shared structure now
	pGSBStatus = (struct GSBStatusType*) shm;
	s = shm;
	
	// Time
	// get secs since 1.1.1970
	Seconds=pGSBStatus->TimeOfDayGSB.tv_sec;
	gmtime_r(&Seconds,&sBrokenUpTime);

	if(ePageToDisplay == E_PAGE_DEBUG)
	{
		printf("Content-type: text/html\n\n") ;
		
		printf("<html>\n") ;
		printf("<head><title>GSB Webinterface</title></head>\n") ;
		printf("<body>\n") ;
		printf("<h1>Debug Page</h1>\n") ;
		printf("SharedMem dump:<br>");

		for(iLoop = 0; iLoop < sizeof(struct GSBStatusType); iLoop++)
		{
			unsigned char cCurrent = *s;
			printf("%02X&#32;",cCurrent);
			s++;
			if(((iLoop % 8) == 7) && (iLoop))
				printf("<br>\n");
		};
		printf("<br>\n");
		printf("SERVER_NAME is: '%s'<br>",getenv("SERVER_NAME"));
		printf("Parameter String passed was '%s'<br>\n",aBuffer);
		printf("Structure size in shared memory is %d bytes.<br>\n",sizeof(struct GSBStatusType));

		printf("<a href=\"cgigateway.cgi?page=status\">Statuspage</a><br>\n");
		printf("<a href=\"cgigateway.cgi?page=debug\">Debugpage</a><br>\n");
	
		printf("GSB-Date is: %02d.%02d.%04d (JD: %03d), %02d:%02d:%02d.%03ld GMT\n",\
		sBrokenUpTime.tm_mday, sBrokenUpTime.tm_mon+1, sBrokenUpTime.tm_year+1900, sBrokenUpTime.tm_yday+1,\
		sBrokenUpTime.tm_hour,sBrokenUpTime.tm_min, sBrokenUpTime.tm_sec,pGSBStatus->TimeOfDayGSB.tv_usec/1000);

		printf("</body>\n") ;
		printf("</html>\n") ;

		exit(0) ;
	}
	else
	if(ePageToDisplay == E_PAGE_STATUS)
	{
		printf("Content-type: text/html\n\n") ;
		
		printf("<html>\n") ;
		printf("<head><title>GSB Webinterface</title></head>\n") ;
		printf("<body>\n") ;
		printf("<h1>Status Page</h1>\n") ;
		
		// edit fields & submit buttons for MFC Setpoint1
		printf("<FORM ACTION=\"http://%s/cgi-bin/cgigateway.cgi\" METHOD=\"get\"> <DIV>Set Flow MFC1:\n",getenv("SERVER_NAME"));
		printf("<INPUT NAME=\"page\" TYPE=\"hidden\" VALUE=\"status\">\n");
		printf("<INPUT NAME=\"cmd\" TYPE=\"hidden\" VALUE=\"setflow\">\n");
	
		printf("<INPUT NAME=\"setflow1\" SIZE=\"5\" MAXLENGTH=\"5\">sccm\n");
		printf("<INPUT TYPE=\"SUBMIT\" VALUE=\"Set\"></DIV></FORM><br>\n");	

		// edit fields & submit buttons for MFC Setpoint2
		printf("<FORM ACTION=\"http://%s/cgi-bin/cgigateway.cgi\" METHOD=\"get\"> <DIV>Set Flow MFC2:\n",getenv("SERVER_NAME"));
		printf("<INPUT NAME=\"page\" TYPE=\"hidden\" VALUE=\"status\">\n");
		printf("<INPUT NAME=\"cmd\" TYPE=\"hidden\" VALUE=\"setflow\">\n");
	
		printf("<INPUT NAME=\"setflow2\" SIZE=\"5\" MAXLENGTH=\"5\">sccm\n");
		printf("<INPUT TYPE=\"SUBMIT\" VALUE=\"Set\"></DIV></FORM><br>\n");	

		// edit fields & submit buttons for MFC Setpoint3
		printf("<FORM ACTION=\"http://%s/cgi-bin/cgigateway.cgi\" METHOD=\"get\"> <DIV>Set Flow MFC3:\n",getenv("SERVER_NAME"));
		printf("<INPUT NAME=\"page\" TYPE=\"hidden\" VALUE=\"status\">\n");
		printf("<INPUT NAME=\"cmd\" TYPE=\"hidden\" VALUE=\"setflow\">\n");
	
		printf("<INPUT NAME=\"setflow3\" SIZE=\"5\" MAXLENGTH=\"5\">sccm\n");
		printf("<INPUT TYPE=\"SUBMIT\" VALUE=\"Set\"></DIV></FORM><br>\n");	

		// check boxes for Valve1
		printf("<FORM ACTION=\"http://%s/cgi-bin/cgigateway.cgi\" METHOD=\"get\">\n",getenv("SERVER_NAME"));
		printf("<INPUT NAME=\"page\" TYPE=\"hidden\" VALUE=\"status\">\n");
		printf("<INPUT NAME=\"cmd\" TYPE=\"hidden\" VALUE=\"setvalve\">\n");
	
		printf("Valve 1:<INPUT NAME=\"valve1\" TYPE=\"checkbox\" VALUE=\"on\"><br>\n");
		printf("Valve 2:<INPUT NAME=\"valve2\" TYPE=\"checkbox\" VALUE=\"on\"><br>\n");
		printf("Valve 3:<INPUT NAME=\"valve3\" TYPE=\"checkbox\" VALUE=\"on\"><br>\n");
		printf("Valve 4:<INPUT NAME=\"valve4\" TYPE=\"checkbox\" VALUE=\"on\"><br>\n");
		printf("Valve 5:<INPUT NAME=\"valve5\" TYPE=\"checkbox\" VALUE=\"on\"><br>\n");
		printf("<INPUT TYPE=\"SUBMIT\" VALUE=\"Set Valves\"></FORM><br>\n");	

		printf("<a href=\"cgigateway.cgi?page=status\">Statuspage</a><br>\n");
		printf("<a href=\"cgigateway.cgi?page=debug\">Debugpage</a><br>\n");

		printf("GSB-Date is: %02d.%02d.%04d (JD: %03d), %02d:%02d:%02d.%03ld GMT\n",\
		sBrokenUpTime.tm_mday, sBrokenUpTime.tm_mon+1, sBrokenUpTime.tm_year+1900, sBrokenUpTime.tm_yday+1,\
		sBrokenUpTime.tm_hour,sBrokenUpTime.tm_min, sBrokenUpTime.tm_sec,pGSBStatus->TimeOfDayGSB.tv_usec/1000);
		
		printf("</body>\n") ;
		printf("</html>\n") ;

		exit(0) ;
	}
	else
	if(ePageToDisplay == E_UNKNOWN)
    {
		printf("Content-type: text/html\n\n") ;
		printf("<html>\n") ;
		printf("<head><title>GSB Webinterface</title></head>\n") ;
		printf("<body>\n") ;
		printf("<h1>Unknown Page!</h1>\n") ;
		printf("Something strange happened...");
		printf("</body>\n") ;
		printf("</html>\n") ;
		exit(0) ;
    }
	
}

