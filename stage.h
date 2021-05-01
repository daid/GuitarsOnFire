#ifndef STAGE_H
#define STAGE_H
/***
 * stage contains the background stage functionality, which is scriptable
 * with lua scripts.
 */

/**
 * load a new stage, discards the old stage and runs the new one.
 */
void loadStage(const char* path);
/**
 * draw the stage. (because of Z buffering, do this before you draw other stuff)
 */
void drawStage();

const char* getStagePath();

#endif//STAGE_H
