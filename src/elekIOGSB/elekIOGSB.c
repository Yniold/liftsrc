#include "../include/elekIO.h"

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <string.h>
#include <unistd.h>

volatile struct GSBStatusType sGSBStatus;
struct GSBStatusType* pGSBStatus;

int main()
{
	struct timeval tvLocalTime;
    char c;
    int iShmHandle;
    key_t ShmKey; // is an int actually
    char *shm, *s;

	// shared memory unique key
    ShmKey = GSB_SHMKEY;

	// create a shared memory segment
    if ((iShmHandle = shmget(ShmKey, sizeof(struct GSBStatusType), IPC_CREAT | 0666)) < 0) 
    {
        perror("Error calling shmget()");
        exit(1);
    };

	// attach SHM to my process addressspace
    if ((shm = shmat(iShmHandle, NULL, 0)) == (char *) -1) 
    {
        perror("Error calling shmat()");
        exit(1);
    }

	// pointer now is a pointer to struct
	pGSBStatus = (struct GSBStatusType*) shm;
	
	// up to now, only update time struct
    while (1)
	{
		// get GMT Time
		gettimeofday(&tvLocalTime,0);
		memcpy((void*)&pGSBStatus->TimeOfDayGSB,(void*)&tvLocalTime,sizeof(struct timeval));
        sleep(1);
	};

    exit(0);
}


