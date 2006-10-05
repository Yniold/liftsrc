#include "butterfly.h"
#include <stdio.h>
#include <pthread.h>

const char progname[]="butterflytest";

uint16_t sGotoPosition;
int iTimer;

int main()
{
	ButterflyInit();
	printf("Entering endless loop!\n\r");
	
	while(true)
	{
		pthread_mutex_lock(&mButterflyMutex);
		printf("Valid: %01x\n\r",sButterflyThread.ucPositionValid);
		printf("CurrentPosition: %05d\n\r",sButterflyThread.sCurrentPosition);
		printf("TargetPosition(read): %05d\n\r",sButterflyThread.sTargetPositionRead);
		printf("TargetPosition(set): %05d\n\r",sButterflyThread.sTargetPositionSet);
		printf("Driver Status: %04x\n\r", sButterflyThread.sMotorControlWord);
		pthread_mutex_unlock(&mButterflyMutex);
		sleep(1);
		iTimer++;
		if(iTimer > 9)
		{
			iTimer = 0;
			pthread_mutex_lock(&mButterflyMutex);
			sGotoPosition += 200;
			if(sGotoPosition > (2500 / 4))
				sGotoPosition = 0;
			sButterflyThread.sTargetPositionSet = sGotoPosition;
			pthread_mutex_unlock(&mButterflyMutex);
		};
	};
};
