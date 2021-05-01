#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <SDL/SDL.h>
#include <string.h>

#include "audio.h"

void initAudio_sys()
{
    SDL_AudioSpec desired, obtained;
    desired.freq = 48000;//44100;
    desired.format= AUDIO_S16SYS;
    desired.channels = 2;
    desired.samples = 512;
    desired.callback = audio_callback;
    desired.userdata = NULL;

    if (SDL_OpenAudio(&desired, &obtained))
    {
        fprintf(stderr, "Failed to open audio: %s\n", SDL_GetError());
        exit(-1);
    }
    if (desired.freq != obtained.freq)
        exit(-1);
    if (desired.format != obtained.format)
        exit(-1);
    if (desired.channels != obtained.channels)
        exit(-1);

    SDL_PauseAudio(0);
}
