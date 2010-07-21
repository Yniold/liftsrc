// ============================================
// GPS Testsoftware
// Headerfile
// ============================================

// $RCSfile: gps_main.h,v $ last changed on $Date: 2005/01/27 14:59:36 $ by $Author: rudolf $

// History:
//
// $Log: gps_main.h,v $
// Revision 1.1  2005/01/27 14:59:36  rudolf
// added files for GPS receiver connection
//
//

#define COMPILE_FOR_ARM

#ifndef GPS_MAIN_H
#define GPS_MAIN_H

// ==============
// Globals
// ==============

struct timeval HandlerTimeOld;
struct timeval HandlerTimeCurrent;
struct timeval Now;
struct timeval TimeWhenSelectResumed;
struct sigaction  SignalAction;
struct sigevent   SignalEvent;
sigset_t          SignalMask;

struct itimerval StatusTimer;
timer_t           StatusTimer_id;

#ifdef COMPILE_FOR_ARM
struct timeval select_timeout;
#elif
struct timespec pselect_timeout;   // timeout
#endif

// ==============
// Prototypes
// ==============

static inline void tv_sub(struct timeval* ptv1, struct timeval* ptv2);
void signalstatus(int signo);

#endif // GPS_MAIN_H
