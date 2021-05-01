#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <math.h>

#include <SDL/SDL.h>

#include "system.h"

int getTime()
{
    return SDL_GetTicks() & 0x7FFFFFFF;
}

void* createMutex()
{
    return (void*)SDL_CreateMutex();
}

void lockMutex(void* mutex)
{
    SDL_mutexP((SDL_mutex *)mutex);
}

void unlockMutex(void* mutex)
{
    SDL_mutexV((SDL_mutex *)mutex);
}

void destroyMutex(void* mutex)
{
    SDL_DestroyMutex((SDL_mutex *)mutex);
}
