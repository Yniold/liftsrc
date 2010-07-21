
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <time.h>

#include "../include/elekIOPorts.h"
#include "../include/elekIO.h"


int main(int argc, char *argv[])
{
    extern int errno;
   
    int i;
    struct elekStatusType StatusFeld;
    uint8_t *pStatus;


    printf("gesamt %d \n", sizeof( struct elekStatusType));
      printf("timeval %d \n", sizeof( struct timeval));                  /* time of data */
      printf("CounterCard %d \n", sizeof( struct CounterCardType));          /* Counter Card for Ref, OH and HO2 */
      printf("EtalonData %d \n", sizeof( struct EtalonDataType));           /* All Etalonstepper data */
      printf("ADC %d \n", sizeof( struct ADCCardType)*MAX_ADC_CARD); /* ADC Card */
      printf("MFC %d \n", sizeof( struct MFCCardType)*MAX_MFC_CARD); /* MFC Card */
      printf("Valve %d \n", sizeof( struct ValveCardType)*MAX_VALVE_CARD);  /* Valve Card */
      printf("DCDC4 %d \n", sizeof( struct DCDC4CardType)*MAX_DCDC4_CARD); /* Valve Card */
      printf("TempSens %d \n", sizeof( struct TempSensorCardType)*MAX_TEMP_SENSOR_CARD); /* Temperature Sensor Card */
      printf("Flags %d \n", sizeof( struct InstrumentFlagsType));       /* Instrument flags*/
 

      pStatus=(uint8_t*)&StatusFeld;
    
      for (i=0;i>sizeof(StatusFeld);i++)
	*pStatus++=i % 256;

      

}
