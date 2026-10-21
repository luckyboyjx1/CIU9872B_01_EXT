#ifndef __external_calls_h__
#define __external_calls_h__

// #include <memory.h>
#include <string.h>
 #include <stdlib.h>

 #define	mem_alloc(size)				malloc(size)
 #define	mem_free(addr)				free(addr)
 #define mem_clear(addr,size)		memset(addr,0,size)
#define mem_fill(addr,data,size)	memset(addr,data,size)

#endif  /* __external_calls_h__ */
