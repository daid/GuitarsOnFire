#ifndef AUDIO_H
#define AUDIO_H
/***
 * audio contains the audio related functions.
 *  note that audio makes use of the audioProvider for music.
 */

/**
 * initialize the audio subsystem, load sample files.
 */
void initAudio();
/**
 * initialize the audio subsystem for the target system. Called from initAudio
 */
void initAudio_sys();

/**
 * The audio callback, called when new samples are needed. (matches the libSDL prototype)
 */
void audio_callback(void *userdata, unsigned char *byteStream, int len);

/**
 * play functions for some sound effects.
 */
void playCrunch();
void playError();
void playStart();
void playSuck();
void playIn();
void playOut();

#endif//AUDIO_H
