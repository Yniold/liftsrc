// ============================================
// GPS Receiver Testsoftware
// simple wrapper program around NMEA parser
// ============================================

// $RCSfile: gps_main.c,v $ last changed on $Date: 2005-01-27 14:59:36 $ by $Author: rudolf $

// History:
//
// $Log: gps_main.c,v $
// Revision 1.1  2005-01-27 14:59:36  rudolf
// added files for GPS receiver connection
//
//

#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <signal.h>
#include <errno.h>
#include <sched.h>

#include "gps_main.h"
#include "serial.h"
#include "NMEAParser.h"

#define SIGNAL_STATUS SIGALRM

// ========================
// Globals
// ========================

unsigned char port[256] = "/dev/ttyS1";	// serial device used for the incoming GPS data
unsigned char pDataBuffer[1024];

char aProgramName[] = "GPS";		// needed for serial.c 's debug output
char *progname = (char *) aProgramName;		// export pointer to debug string

int verbose = 255;					// be very verbose
long baud = 9600;					// serial baudrate
volatile char ucDataReadyFlag = 0;	// Data ready flag
int fd = -1;						// file descriptor for serial communication
char ucPortOpened = 0;				// flag for the timer routine wether port is available or not


//===================================
//main ()
//===================================

int main()
{

	// register action handler (timer callback)
	sigfillset(&SignalAction.sa_mask);
	SignalAction.sa_flags = 0;
	SignalAction.sa_handler = signalstatus;
	sigaction(SIGNAL_STATUS, &SignalAction, NULL);

	// setup timer

	memset(&SignalEvent, 0, sizeof(SignalEvent));
	SignalEvent.sigev_notify = SIGEV_SIGNAL;
	SignalEvent.sigev_signo = SIGNAL_STATUS;
	SignalEvent.sigev_value.sival_int = 0;

	// Start timer

	StatusTimer.it_interval.tv_sec =   0;
	StatusTimer.it_interval.tv_usec =  90000;
	StatusTimer.it_value = StatusTimer.it_interval;

	int ret = 0;

	ret = setitimer(ITIMER_REAL, &StatusTimer, NULL);
	if (ret < 0)
	{
		perror("settimer");
		return -1;
	}


	NMEAParserInit();

	fd = serial_open(port , baud);
	if(fd == 1)
	{
		printf("Error opening %s !\n\r", port);
		exit(-1);
	};
	printf("Opened %s with %d BAUD!\n\r", port, baud);
	ucPortOpened = 1;

	while(1)
	{
		while(!ucDataReadyFlag);
		printf("UTC Time is %02d:%02d:%02d\n",ucGGAHour, ucGGAMinute, ucGGASecond);
		printf("Latitude is %f, Longitude is %f\n", dGGALatitude, dGGALongitude);

	//
		printf("Quality: %d\n",ucGGAGPSQuality);
	// 0 = fix not available, 1 = GPS sps mode, 2 = Differential GPS, SPS mode, fix valid, 3 = GPS PPS mode, fix valid
	//
		printf("Number of Satellites: %d\n", ucGGANumOfSatsInUse);
		printf("HDOP: %f\n",dGGAHDOP);
		printf("Altitude MSL: %f\n\n", dGGAAltitude);
		ucDataReadyFlag = FALSE;
	//int dwGGACount;					//
	//int nGGAOldVSpeedSeconds;			//
	//double dGGAOldVSpeedAlt;			//
	//double dGGAVertSpeed;				//
	};
 	serial_close(fd);

};

// ==================================================
// Timerfunction actually called each 100ms or so....
// ==================================================

void signalstatus(int signo)
{
	extern struct timeval HandlerTimeOld;
	extern struct timeval HandlerTimeCurrent;
	extern struct timeval Now;
	int iBytesRead = 0;

	gettimeofday(&Now, NULL);
	HandlerTimeCurrent = Now;
	tv_sub(&HandlerTimeCurrent,&HandlerTimeOld);
	HandlerTimeOld = Now;
	// printf("Time difference between calls: %ds %dus\n\r", HandlerTimeCurrent.tv_sec,HandlerTimeCurrent.tv_usec);

	if(ucPortOpened)	// check if main() has opened the port already
	{
		iBytesRead = read(fd, pDataBuffer, 1024);	// nonblocking (!)

		if(iBytesRead)
			ParseBuffer(pDataBuffer,iBytesRead);	// feed some characters to the parser
  };
}

// ===========================================================
// small inline helper function
// needed to calculate time between two signal handler calls
// ===========================================================

static inline void
tv_sub(struct timeval* ptv1, struct timeval* ptv2)
{
    ptv1->tv_sec -= ptv2->tv_sec;
    ptv1->tv_usec -= ptv2->tv_usec;
    while (ptv1->tv_usec < 0)
    {
        ptv1->tv_sec--;
        ptv1->tv_usec += 1000*1000;
    }
}


