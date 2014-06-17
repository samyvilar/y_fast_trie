
#ifndef __TIMED__
#define __TIMED__

#include <time.h>

#define timed(func) ({clock_t __start__, __end__; __start__ = clock(); func(); __end__ = clock(); ((double)(__end__ - __start__))/CLOCKS_PER_SEC;})

#endif