#ifndef SYSTEM_H
#define SYSTEM_H

/**
 * returns a time in miliseconds. Only relative to itself.
 */
int getTime();

void* createMutex();
void lockMutex(void* mutex);
void unlockMutex(void* mutex);
void destroyMutex(void* mutex);

#endif//SYSTEM_H
