#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <math.h>

#include <SDL/SDL.h>
#ifdef WIN32
#include <windows.h>
#include <SDL/SDL_syswm.h>
#endif

#include "input.h"
#include "game.h"
#include "system.h"

int key[1024];
int keypress[1024];
int rawKey = -1;

int keyMap[KEYBOARD_KEY_COUNT] = {SDLK_F1, SDLK_F2, SDLK_F3, SDLK_F4, SDLK_F5, SDLK_UP, SDLK_DOWN, SDLK_RETURN, SDLK_RSHIFT, SDLK_ESCAPE, SDLK_q, SDLK_w, SDLK_e, -1, SDLK_r, SDLK_SPACE};

void updateInputs(int autoRepeat)
{
    rawKey = -1;
    
    memset(keypress, 0, sizeof(keypress));
    if (autoRepeat)
    {
        int time = getTime();
        for(int i=0;i<1024;i++)
        {
            if (key[i] > 0 && key[i] - time < 0)
            {
                keypress[i] = 1;
                key[i] = getTime() + AUTO_REPEAT_TIME_REPEAT;
            }
        }
    }
    
    SDL_Event Event;
    while ( SDL_PollEvent( &Event ) )
    {
        switch (Event.type)
        {
        case SDL_QUIT:
            exit(0);
            break;
        case SDL_KEYDOWN:
            if (Event.key.keysym.sym < 1024)
            {
                key[Event.key.keysym.sym] = getTime() + AUTO_REPEAT_TIME_FIRST;
                keypress[Event.key.keysym.sym] = 1;
                
                rawKey = Event.key.keysym.sym;
            }
            break;
        case SDL_KEYUP:
            if (Event.key.keysym.sym < 1024)
            {
                key[Event.key.keysym.sym] = 0;
            }
            break;
        }
    }
}

int getInputState(int player)
{
    int buttons = 0;

    switch(player)
    {
    case -1:
    case 0:
        buttons |= INPUT_HAS_GUITAR;
        if (key[keyMap[0]]) buttons |= BUTTON_FRET(0);
        if (key[keyMap[1]]) buttons |= BUTTON_FRET(1);
        if (key[keyMap[2]]) buttons |= BUTTON_FRET(2);
        if (key[keyMap[3]]) buttons |= BUTTON_FRET(3);
        if (key[keyMap[4]]) buttons |= BUTTON_FRET(4);
        if (key[keyMap[5]]) buttons |= BUTTON_STRUM_UP;
        if (key[keyMap[6]]) buttons |= BUTTON_STRUM_DOWN;
        if (key[keyMap[7]]) buttons |= BUTTON_STRUM_UP;
        if (key[keyMap[8]]) buttons |= BUTTON_STRUM_DOWN;
        if (key[keyMap[9]]) buttons |= BUTTON_MENU;
        break;
    case 1:
        buttons |= INPUT_HAS_DRUMS;
        if (key[SDLK_1]) buttons |= BUTTON_DRUM_PAD(0);
        if (key[SDLK_2]) buttons |= BUTTON_DRUM_PAD(1);
        if (key[SDLK_3]) buttons |= BUTTON_DRUM_PAD(2);
        if (key[SDLK_4]) buttons |= BUTTON_DRUM_PAD(3);
        if (key[SDLK_5]) buttons |= BUTTON_DRUM_PAD(4);
        if (key[SDLK_SPACE]) buttons |= BUTTON_DRUM_BASS;
        //if (keypress[SDLK_]) buttons |= BUTTON_MENU;
        break;
    }
    return buttons;
}

int getInputPress(int player)
{
    if (player == -1)
        return getInputPress(0) | getInputPress(1) | getInputPress(2) | getInputPress(3) | getInputPress(4);
    
    int buttons = 0;

    switch(player)
    {
    case 0:
        buttons |= INPUT_HAS_GUITAR;
        if (keypress[keyMap[0]]) buttons |= BUTTON_FRET(0);
        if (keypress[keyMap[1]]) buttons |= BUTTON_FRET(1);
        if (keypress[keyMap[2]]) buttons |= BUTTON_FRET(2);
        if (keypress[keyMap[3]]) buttons |= BUTTON_FRET(3);
        if (keypress[keyMap[4]]) buttons |= BUTTON_FRET(4);
        if (keypress[keyMap[5]]) buttons |= BUTTON_STRUM_UP;
        if (keypress[keyMap[6]]) buttons |= BUTTON_STRUM_DOWN;
        if (keypress[keyMap[7]]) buttons |= BUTTON_STRUM_UP;
        if (keypress[keyMap[8]]) buttons |= BUTTON_STRUM_DOWN;
        if (keypress[keyMap[9]]) buttons |= BUTTON_MENU;
        break;
    case 1:
        buttons |= INPUT_HAS_DRUMS;
        if (keypress[SDLK_1]) buttons |= BUTTON_DRUM_PAD(0) | BUTTON_DRUM_JOY_UP;
        if (keypress[SDLK_2]) buttons |= BUTTON_DRUM_PAD(1);
        if (keypress[SDLK_3]) buttons |= BUTTON_DRUM_PAD(2) | BUTTON_DRUM_JOY_DOWN;
        if (keypress[SDLK_4]) buttons |= BUTTON_DRUM_PAD(3);
        if (keypress[SDLK_5]) buttons |= BUTTON_DRUM_PAD(4);
        if (keypress[SDLK_SPACE]) buttons |= BUTTON_DRUM_BASS;
        //if (keypress[SDLK_]) buttons |= BUTTON_MENU;
        break;
    default:
        if (keypress[SDLK_z]) buttons |= BUTTON_FRET(0);
        if (keypress[SDLK_SPACE]) buttons |= BUTTON_FRET(0);
        break;
    }
    return buttons;
}

int getInputAngle(int player)
{
    return key[SDLK_SLASH] ? 140 : 100;
}

float getInputWhammy(int player)
{
    return key[SDLK_BACKSLASH] ? 1 : 0;
}

int getInputTouch(int player)
{
    if (player != 0)
        return 0;
    
    return 0x1000 | (key[SDLK_q] ? 0x01 : 0) | (key[SDLK_w] ? 0x02 : 0) | (key[SDLK_e] ? 0x04 : 0) | (key[SDLK_r] ? 0x08 : 0) | (key[SDLK_t] ? 0x10 : 0);
}

int getRawKeyPress()
{
    return rawKey;
}

void setKeyboardKey(int keyNum, int rawKey)
{
    if (keyNum < 0 || keyNum >= KEYBOARD_KEY_COUNT)
        return;
    keyMap[keyNum] = rawKey;
}

int getKeyboardKey(int keyNum)
{
    if (keyNum < 0 || keyNum >= KEYBOARD_KEY_COUNT)
        return 0;
    return keyMap[keyNum];
}

int getIrDrums(struct ir_drums* drums)
{
    drums->num = 0;
#ifdef USE_IR_DRUMS
    int x, y;
    int button = SDL_GetMouseState(&x, &y);
    drums->x[0] = float(x) / (576.0f  * 4.0 / 3.0);
    drums->y[0] = float(y) / 576.0f;
    drums->num = 1;
    if (button)
    {
        drums->x[1] = drums->x[0];
        drums->y[1] = drums->y[0];
        drums->num = 2;
    }
#endif
    return drums->num;
}
