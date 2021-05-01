#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <math.h>

#include <gccore.h>
#include <ogc/lwp_watchdog.h>

#include "system.h"

int getTime()
{
    return (gettime() / TB_TIMER_CLOCK) & 0x7FFFFFFF;
}

void* createMutex()
{
    mutex_t* mutex = (mutex_t*)malloc(sizeof(mutex_t));
    LWP_MutexInit(mutex, true);
    return (void*)mutex;
}

void lockMutex(void* pmutex)
{
    mutex_t* mutex = (mutex_t*)pmutex;
    LWP_MutexLock(*mutex);
}

void unlockMutex(void* pmutex)
{
    mutex_t* mutex = (mutex_t*)pmutex;
    LWP_MutexUnlock(*mutex);
}

void destroyMutex(void* pmutex)
{
    mutex_t* mutex = (mutex_t*)pmutex;
    
    LWP_MutexDestroy(*mutex);
    free(mutex);
}
