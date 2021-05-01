#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdarg.h>

#include "strfunc.h"

int nprintf(char * buf, size_t len, const char * fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    int ret = vsnprintf(buf, len, fmt, args);
    va_end(args);
    buf[len-1] = 0;
    
    return ret;
}

char* trim(char* buf)
{
    while(*buf < 33 && *buf)
        buf++;
    int i = strlen(buf);
    while(i > 0 && buf[i-1] < 33)
    {
        buf[i-1] = 0;
        i--;
    }
    return buf;
}
