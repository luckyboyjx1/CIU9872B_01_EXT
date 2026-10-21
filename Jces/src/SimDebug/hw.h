#ifndef HW_HW_H_
#define HW_HW_H_

//#include "hw_memory.h"
#include "pthread.h"
extern pthread_t javaDebugConnThreadId;
extern pthread_t callbackThreadId;
void *javaDebugConn (void *arg);

void hw_init(int argc, char* argv[]);

#endif /* HW_HW_SIM_H_ */
