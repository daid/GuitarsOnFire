#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "midi.h"
#include "readdir.h"
#include "settings.h"
#include "input.h"
#include "strfunc.h"

int useUSBdrive = 0;

void loadSettings()
{
    FILE* saveFile = fopen(DATA_BASE "/settings.ini", "rt");
    if (!saveFile)
        return;
    char buffer[1024];
    while(fgets(buffer, 1024, saveFile))
    {
        buffer[1023] = 0;
        char* num = strchr(buffer, ':');
        char* value = strchr(buffer, '=');
        if (!num || !value)
            continue;
        *num++ = 0;
        *value++ = 0;
        num = trim(num);
        value = trim(value);
        char* name = trim(buffer);
        int n = atoi(num)-1;
        
        if (strcasecmp(name, "leftyFlip") == 0)
        {
            if (n < 0 || n >= MAX_PLAYERS)
                continue;
            player[n].leftyFlip = atoi(value);
        }
        if (strcasecmp(name, "precisionMode") == 0)
        {
            if (n < 0 || n >= MAX_PLAYERS)
                continue;
            player[n].precisionMode = atoi(value);
        }
        if (strcasecmp(name, "keymap") == 0)
        {
            if (n < 0 || n >= KEYBOARD_KEY_COUNT)
                continue;
            setKeyboardKey(n, atoi(value));
        }
        if (strcasecmp(name, "useusbdrive") == 0)
        {
            useUSBdrive = atoi(value);
        }
    }
    fclose(saveFile);
}

void saveSettings()
{
    FILE* saveFile = fopen(DATA_BASE "/settings.ini", "wt");
    if (!saveFile)
        return;
    
    for(int i=0;i<5;i++)
    {
        fprintf(saveFile, "leftyFlip:%i = %i\n", i+1, player[i].leftyFlip);
    }
    for(int i=0;i<5;i++)
    {
        fprintf(saveFile, "precisionMode:%i = %i\n", i+1, player[i].precisionMode);
    }
    for(int i=0;i<KEYBOARD_KEY_COUNT;i++)
    {
        fprintf(saveFile, "keymap:%i = %i\n", i+1, getKeyboardKey(i));
    }    
    fprintf(saveFile, "useusbdrive:0 = %i\n", useUSBdrive);
    
    fclose(saveFile);
}
