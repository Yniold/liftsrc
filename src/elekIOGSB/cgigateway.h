#ifndef CGIGATEWAY_H
#define CGIGATEWAY_H

typedef enum {E_PAGE_STATUS,E_PAGE_DEBUG,E_UNKNOWN} ePages;

#ifdef RUNONPC

// newer 2.6 kernel don't provide the header file for rtscll() any longer,
// so we define it here by ourselves

#define rdtsc(low,high) \
__asm__ __volatile__("rdtsc" : "=a" (low), "=d" (high))

#define rdtscl(low) \
__asm__ __volatile__("rdtsc" : "=a" (low) : : "edx")

#define rdtscll(val) \
__asm__ __volatile__("rdtsc" : "=A" (val))

#endif
extern int SendUDPData(struct MessagePortType *ptrMessagePort, unsigned nByte, void *msg);
extern int SendUDPMsg(struct MessagePortType *ptrMessagePort, void *msg);
extern int InitUDPOutSocket();
extern int InitUDPInSocket();
extern int SendUDPDataToIP(struct MessagePortType *ptrMessagePort, char *IPAddr, unsigned nByte, void *msg);

extern int SetStatusCommand(uint16_t MsgType, uint16_t Addr, int64_t Value);
   
#endif
