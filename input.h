#ifndef INPUT_H
#define INPUT_H
/***
 * input contains the prototypes for all controller input related
 * functions.
 */


/* masks for the getInput and getInputPress functions */
#define INPUT_HAS_GUITAR    (0x00000001)
#define INPUT_HAS_DRUMS     (0x00000002)
#define INPUT_HAS_CLASSIC   (0x00000004)
#define INPUT_HAS_ANY_CON   (0x0000000f)

//Guitar keys
#define BUTTON_FRET(n)      (1 << ((n) + 4))
#define BUTTON_FRET_ANY     (0x000001f0)
#define BUTTON_STRUM        (0x00003000)
#define BUTTON_STRUM_UP     (0x00001000)
#define BUTTON_STRUM_DOWN   (0x00002000)
#define BUTTON_MENU         (0x00004000)
//Drum keys
#define BUTTON_DRUM_BASS     (0x00010000)
#define BUTTON_DRUM_PAD(n)   (1 << ((n) + 17))
#define BUTTON_DRUM_PAD_ANY  (0x003E0000)
#define BUTTON_DRUM_JOY_UP   (0x00400000)
#define BUTTON_DRUM_JOY_DOWN (0x00800000)
//Combined menu keys
#define BUTTON_MENU_UP      (BUTTON_STRUM_UP | BUTTON_DRUM_JOY_UP)
#define BUTTON_MENU_DOWN    (BUTTON_STRUM_DOWN | BUTTON_DRUM_JOY_DOWN)
#define BUTTON_MENU_SELECT  (BUTTON_FRET(0) | BUTTON_DRUM_PAD(1))
#define BUTTON_MENU_CANCEL  (BUTTON_FRET(1) | BUTTON_DRUM_PAD(4))

#define BUTTON_ANY          (0x00fffff0)

/* Indexes for the setKeyboardKey function */
#define KEYBOARD_KEY_FRET(n)        (n)
#define KEYBOARD_KEY_STRUM_UP       (5)
#define KEYBOARD_KEY_STRUM_DOWN     (6)
#define KEYBOARD_KEY_STRUM_UP_ALT   (7)
#define KEYBOARD_KEY_STRUM_DOWN_ALT (8)
#define KEYBOARD_KEY_MENU           (9)
#define KEYBOARD_KEY_DRUM_PAD(n)    (10 + (n))
#define KEYBOARD_KEY_DRUM_BASS      (15)
#define KEYBOARD_KEY_COUNT          (16)

#define AUTO_REPEAT_TIME_FIRST 500
#define AUTO_REPEAT_TIME_REPEAT 80

struct ir_drums
{
    int num;
    float x[2];
    float y[2];
};

/**
 * update the input values, read new keys/whammy/touch bars.
 * updates the 'pressed' states.
 */
void updateInputs(int autoRepeat);

/**
 * Gets the keys hold at the moment up 'updateInputs'
 */
int getInputState(int player);
/**
 * Gets which keys have been pressed between the last updateInputs and the one before that.
 * In other words, gets key presses. Calling it multiple times won't change the result, only updateInputs will change the result.
 */
int getInputPress(int player);

/**
 * return the angle at which the guitar is hold. 90 deg is about normal when standing with the guitar in your hand. 180 is aiming up.
 *  range -180 to 180
 */
int getInputAngle(int player);

/**
 * returns the possition of the whammy bar.
 * range: 0-1, where 1 is pressed down. (Some guitars return 0.1 even in 'rested' possition)
 */
float getInputWhammy(int player);
/**
 * returns the state of the touchbar. See 'wiiuse.h' for the masks.
 */
int getInputTouch(int player);

/**
 * returns the raw number of the last key pressed after updateInputs. Or -1 if no key was pressed.
 */
int getRawKeyPress();
/**
 * sets a keyboard map key.
 */
void setKeyboardKey(int keyNum, int rawKey);
/**
 * gets a keyboard map key.
 */
int getKeyboardKey(int keyNum);
/**
 * gets the position of the 2 IR leds (attach IR leds to the end of 2 sticks and you can drum :D)
 */
int getIrDrums(struct ir_drums* drums);

#endif//INPUT_H
