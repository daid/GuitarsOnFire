#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <math.h>

#include <gccore.h>
#include <wiiuse/wpad.h>
#include <wiikeyboard/keyboard.h>
#include <ogc/pad.h>

#include "input.h"
#include "game.h"
#include "system.h"

int lastState[MAX_PLAYERS];
int keyboard[KEYBOARD_KEY_COUNT];
int orient[4];
float whammy[4];
int touch[4];
int keyMap[KEYBOARD_KEY_COUNT] = {KS_f1, KS_f2, KS_f3, KS_f4, KS_f5, KS_Up, KS_Down, KS_Return, KS_Shift_R, KS_Escape, KS_q, KS_w, KS_e, -1, KS_r, KS_space};
int rawKey = -1;
struct ir_drums input_drums;

int repeatTime = 0;

void updateInputs(int autoRepeat)
{
    struct expansion_t exp;
    
    rawKey = -1;
    for(int i=0;i<MAX_PLAYERS;i++)
    {
        lastState[i] = getInputState(i);
    }
    PAD_ScanPads();
    WPAD_ScanPads();
    for(int i=0;i<4;i++)
    {
        //Calculate the orientation ourselfs, as we don't get the right info from libwiiuse
        struct gforce_t force;
        WPAD_GForce(i, &force);
        if (force.x > 1) force.x = 1;
        if (force.x <-1) force.x =-1;
        if (force.y > 1) force.y = 1;
        if (force.y <-1) force.y =-1;
        orient[i] = atan2f(force.x, force.y) * 180 / 3.1415926535897932384626433832795;
        
        WPAD_Expansion(i, &exp);
        switch(exp.type)
        {
        case EXP_GUITAR_HERO_3:
            whammy[i] = exp.gh3.whammy_bar;
            touch[i] = exp.gh3.touch_bar;
            break;
        }
    }

    keyboard_event ke;
    while (KEYBOARD_GetEvent(&ke))
    {
        if (ke.type == KEYBOARD_RELEASED || ke.type == KEYBOARD_PRESSED)
        {
            if (ke.type == KEYBOARD_PRESSED)
            {
                rawKey = ke.symbol;
            }
            for(int i=0; i<KEYBOARD_KEY_COUNT;i++)
            {
                if (ke.symbol == keyMap[i])
                    keyboard[i] = ke.type == KEYBOARD_PRESSED;
            }
        }
    }
    
    if (WPAD_ButtonsDown(0) & WPAD_BUTTON_HOME) exit(0);
    
    if (autoRepeat)
    {
        if (getInputPress(-1) & BUTTON_ANY)
        {
            repeatTime = getTime() + AUTO_REPEAT_TIME_FIRST;
        }
        if (!(getInputState(-1) & BUTTON_ANY))
        {
            repeatTime = 0;
        }
        if (repeatTime > 0 && repeatTime - getTime() < 0)
        {
            for(int i=0;i<MAX_PLAYERS;i++)
            {
                lastState[i] &=~BUTTON_ANY;
            }
            repeatTime = getTime() + AUTO_REPEAT_TIME_REPEAT;
        }
    }
    
#ifdef USE_IR_DRUMS
    struct ir_t ir;
    input_drums.num = 0;
    for(int i=0;i<4;i++)
    {
        WPAD_Expansion(i, &exp);
        if (exp.type != EXP_NONE)
            continue;
        WPAD_IR(i, &ir);
        if (ir.num_dots > 0)
        {
            input_drums.num = 1;
            input_drums.x[0] = float(ir.dot[0].rx) / 1024.0f;
            input_drums.y[0] = 1 - (float(ir.dot[0].ry) / 768.0f);
            if (ir.num_dots > 1)
            {
                input_drums.num = 2;
                input_drums.x[1] = float(ir.dot[1].rx) / 1024.0f;
                input_drums.y[1] = 1 - (float(ir.dot[1].ry) / 768.0f);
            }
            break;
        }
    }
#endif
}

int getIrDrums(struct ir_drums* drums)
{
    drums->num = 0;
#ifdef USE_IR_DRUMS
    memcpy(drums, &input_drums, sizeof(struct ir_drums));
#endif
    return drums->num;
}

int getInputPress(int player)
{
    if (player == -1)
        return getInputPress(0) | getInputPress(1) | getInputPress(2) | getInputPress(3) | getInputPress(4);
    if (player < 0 || player >= MAX_PLAYERS)
        return 0;
    return getInputState(player) & ~lastState[player];
}

int getInputState(int p)
{
    if (p == -1)
        return getInputState(0) | getInputState(1) | getInputState(2) | getInputState(3) | getInputState(4);
    int buttons = 0;
    struct expansion_t exp;
    
    if (p < 4)
    {
        WPAD_Expansion(p, &exp);
        switch(exp.type)
        {
        case EXP_GUITAR_HERO_3:
            buttons |= INPUT_HAS_GUITAR;
            if (exp.gh3.btns & GUITAR_HERO_3_BUTTON_STRUM_UP)
                buttons |= BUTTON_STRUM_UP;
            if (exp.gh3.btns & GUITAR_HERO_3_BUTTON_STRUM_DOWN)
                buttons |= BUTTON_STRUM_DOWN;
            if (exp.gh3.btns & GUITAR_HERO_3_BUTTON_GREEN)
                buttons |= BUTTON_FRET(0);
            if (exp.gh3.btns & GUITAR_HERO_3_BUTTON_RED)
                buttons |= BUTTON_FRET(1);
            if (exp.gh3.btns & GUITAR_HERO_3_BUTTON_YELLOW)
                buttons |= BUTTON_FRET(2);
            if (exp.gh3.btns & GUITAR_HERO_3_BUTTON_BLUE)
                buttons |= BUTTON_FRET(3);
            if (exp.gh3.btns & GUITAR_HERO_3_BUTTON_ORANGE)
                buttons |= BUTTON_FRET(4);
            if (exp.gh3.btns & GUITAR_HERO_3_BUTTON_PLUS)
                buttons |= BUTTON_MENU;
            break;
        case EXP_GUITAR_HERO_DRUMS:
            buttons |= INPUT_HAS_DRUMS;
            //if (exp.ghdrums.btns & (GUITAR_HERO_DRUM_PLUS | GUITAR_HERO_DRUM_MINUS))
            //    buttons |= BUTTON_MENU;
            if (exp.ghdrums.btns & GUITAR_HERO_DRUM_BASS)
                buttons |= BUTTON_DRUM_BASS;
            if (exp.ghdrums.btns & GUITAR_HERO_DRUM_PAD_RED)
                buttons |= BUTTON_DRUM_PAD(0);
            if (exp.ghdrums.btns & GUITAR_HERO_DRUM_CYMBAL_YELLOW)
                buttons |= BUTTON_DRUM_PAD(1);
            if (exp.ghdrums.btns & GUITAR_HERO_DRUM_PAD_BLUE)
                buttons |= BUTTON_DRUM_PAD(2);
            if (exp.ghdrums.btns & GUITAR_HERO_DRUM_CYMBAL_ORANGE)
                buttons |= BUTTON_DRUM_PAD(3);
            if (exp.ghdrums.btns & GUITAR_HERO_DRUM_PAD_GREEN)
                buttons |= BUTTON_DRUM_PAD(4);
            if (exp.ghdrums.js.mag > 0.5)
            {
                if (exp.ghdrums.js.ang < 45 || exp.ghdrums.js.ang > 315)
                    buttons |= BUTTON_DRUM_JOY_UP;
                if (exp.ghdrums.js.ang > 135 && exp.ghdrums.js.ang < 225)
                    buttons |= BUTTON_DRUM_JOY_DOWN;
            }
            break;
        case EXP_CLASSIC:
            buttons |= INPUT_HAS_CLASSIC;
            if (exp.classic.btns & CLASSIC_CTRL_BUTTON_A)
                buttons |= BUTTON_FRET(0);
            if (exp.classic.btns & CLASSIC_CTRL_BUTTON_X)
                buttons |= BUTTON_FRET(1);
            if (exp.classic.btns & CLASSIC_CTRL_BUTTON_Y)
                buttons |= BUTTON_FRET(2);
            if (exp.classic.btns & CLASSIC_CTRL_BUTTON_PLUS)
                buttons |= BUTTON_FRET(3);
            if (exp.classic.btns & CLASSIC_CTRL_BUTTON_FULL_R)
                buttons |= BUTTON_FRET(4);
            if (exp.classic.btns & (CLASSIC_CTRL_BUTTON_HOME | CLASSIC_CTRL_BUTTON_MINUS))
                buttons |= BUTTON_MENU;
            if (exp.classic.ljs.mag > 0.5)
            {
                if (exp.classic.ljs.ang > 315 || exp.classic.ljs.ang < 45)
                    buttons |= BUTTON_STRUM_UP;
                if (exp.classic.ljs.ang < 225 && exp.classic.ljs.ang > 135)
                    buttons |= BUTTON_STRUM_DOWN;
            }
            break;
        }
        
        if (!(buttons & INPUT_HAS_ANY_CON))
        {
            //No suitable controller found, try to use gamecube controller. (Which can be a PS2 guitar trough PS2->GC converter, so match that mapping)
            if (PAD_ButtonsHeld(p) & PAD_TRIGGER_R)
                buttons |= BUTTON_FRET(0);
            if (PAD_ButtonsHeld(p) & PAD_BUTTON_X)
                buttons |= BUTTON_FRET(1);
            if (PAD_ButtonsHeld(p) & PAD_BUTTON_Y)
                buttons |= BUTTON_FRET(2);
            if (PAD_ButtonsHeld(p) & PAD_BUTTON_A)
                buttons |= BUTTON_FRET(3);
            if (PAD_ButtonsHeld(p) & PAD_BUTTON_B)
                buttons |= BUTTON_FRET(4);
            if (PAD_ButtonsHeld(p) & PAD_BUTTON_UP)
                buttons |= BUTTON_STRUM_UP;
            if (PAD_ButtonsHeld(p) & PAD_BUTTON_DOWN)
                buttons |= BUTTON_STRUM_DOWN;
            if (PAD_ButtonsHeld(p) & PAD_BUTTON_START)
                buttons |= BUTTON_MENU;
        }
    }else{
        if (keyboard[KEYBOARD_KEY_FRET(0)])
            buttons |= BUTTON_FRET(0);
        if (keyboard[KEYBOARD_KEY_FRET(1)])
            buttons |= BUTTON_FRET(1);
        if (keyboard[KEYBOARD_KEY_FRET(2)])
            buttons |= BUTTON_FRET(2);
        if (keyboard[KEYBOARD_KEY_FRET(3)])
            buttons |= BUTTON_FRET(3);
        if (keyboard[KEYBOARD_KEY_FRET(4)])
            buttons |= BUTTON_FRET(4);
        if (keyboard[KEYBOARD_KEY_STRUM_UP] || keyboard[KEYBOARD_KEY_STRUM_UP_ALT])
            buttons |= BUTTON_STRUM_UP | BUTTON_DRUM_JOY_UP;
        if (keyboard[KEYBOARD_KEY_STRUM_DOWN] || keyboard[KEYBOARD_KEY_STRUM_DOWN_ALT])
            buttons |= BUTTON_STRUM_DOWN | BUTTON_DRUM_JOY_DOWN;
        if (keyboard[KEYBOARD_KEY_MENU])
            buttons |= BUTTON_MENU;
        if (player[p].playAsDrums)
        {
            //Only do this when playing as drums, so we don't act on drum keys in the menus.
            if (keyboard[KEYBOARD_KEY_DRUM_PAD(0)])
                buttons |= BUTTON_DRUM_PAD(0);
            if (keyboard[KEYBOARD_KEY_DRUM_PAD(1)])
                buttons |= BUTTON_DRUM_PAD(1);
            if (keyboard[KEYBOARD_KEY_DRUM_PAD(2)])
                buttons |= BUTTON_DRUM_PAD(2);
            if (keyboard[KEYBOARD_KEY_DRUM_PAD(3)])
                buttons |= BUTTON_DRUM_PAD(3);
            if (keyboard[KEYBOARD_KEY_DRUM_PAD(4)])
                buttons |= BUTTON_DRUM_PAD(4);
            if (keyboard[KEYBOARD_KEY_DRUM_BASS])
                buttons |= BUTTON_DRUM_BASS;
        }
#ifdef USE_IR_DRUMS
        for(int i=0;i<input_drums.num;i++)
        {
            if (input_drums.y[i] > 0.8)
            {
                int num = int(input_drums.x[i] * 4);
                if (num < 0 || num > 3)
                    continue;
                buttons |= BUTTON_DRUM_PAD(num);
            }
        }
#endif
    }
    
    return buttons;
}

int getInputAngle(int player)
{
    if (player >= 0 && player < 4)
        return orient[player];
    return 0;
}

float getInputWhammy(int player)
{
    if (player < 0 || player > 3)
        return 0;
    return whammy[player];
}

int getInputTouch(int player)
{
    if (player < 0 || player > 3)
        return 0;
    return touch[player];
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
