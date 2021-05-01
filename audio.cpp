#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "midi.h"
#include "audioProvider.h"
#include "audio.h"
#include "readdir.h"

#define MIX(sample, newsample) { int __i = (int)(sample) + (int)(newsample) / 2; if (__i < -0x7FFF) __i = -0x7FFF; if (__i > 0x7FFF) __i = 0x7FFF; sample = __i;}

struct TSFX
{
    short* samples;
    int length;
    int playing;
};

TSFX errorSound[6];
TSFX crunchSound[3];
TSFX startSound;
TSFX suckSound[5];
TSFX inSound;
TSFX outSound;

void playError()
{
    errorSound[rand() % 6].playing = 0;
}

void playCrunch()
{
    crunchSound[rand() % 3].playing = 0;
}

void playStart()
{
    startSound.playing = 0;
}

void playSuck()
{
    suckSound[rand() % 5].playing = 0;
}

void playIn()
{
    inSound.playing = 0;
}

void playOut()
{
    outSound.playing = 0;
}

void playEffect(TSFX* sfx, short* stream, int len, int volumeDec)
{
    if (sfx->playing > -1)
    {
        for(int i=0;i<len;i++)
        {
            if (sfx->playing < sfx->length)
            {
                MIX(stream[i], sfx->samples[sfx->playing] / volumeDec);
                sfx->playing ++;
            }
        }
        if (sfx->playing == sfx->length)
        {
            sfx->playing = -1;
        }
    }
}

void audio_callback2(void *userdata, unsigned char *byteStream, int len)
{
    short* stream = (short*)byteStream;
    short buffer[len/2];
    int done = 0;

    if (cleanupAudioProvider)
    {
        if (cleanupAudioProvider != &nullAudioProvider)
        {
            delete cleanupAudioProvider;
        }
        cleanupAudioProvider = NULL;
    }

    //memset(stream, len, 0);

    for(int effect=0;effect<6;effect++)
    {
        playEffect(&errorSound[effect], stream, len/2, 6);
    }
    for(int effect=0;effect<3;effect++)
    {
        playEffect(&crunchSound[effect], stream, len/2, 4);
    }
    for(int effect=0;effect<5;effect++)
    {
        playEffect(&suckSound[effect], stream, len/2, 1);
    }
    playEffect(&startSound, stream, len/2, 2);
    playEffect(&inSound, stream, len/2, 4);
    playEffect(&outSound, stream, len/2, 4);

    if (currentAudioProvider)
    {
        done = currentAudioProvider->read(buffer, len/4);
        for(int i=0;i<done*2;i++)
        {
            MIX(stream[i], buffer[i]);
        }
    }
}

void audio_callback(void *userdata, unsigned char *byteStream, int len)
{
    audio_callback2(userdata, byteStream, len * 44100 / 48000);
    
    short* stream = (short*)byteStream;
    len /= 2;
    for(int i=len-2;i>-1;i-=2)
    {
        int j = (i * 44100 / 48000) & ~1;
        stream[i]   = stream[j];
        stream[i+1] = stream[j+1]; 
    }
}

void ReadSFX(const char* filename, TSFX* sfx)
{
    OggVorbis_File vf;

    sfx->samples = NULL;
    sfx->length = 0;
    sfx->playing = -1;

    FILE* f = fopen(filename, "rb");
    if (f)
    {
        int ret = ov_open(f, &vf, NULL, 0);
        if (ret > -1)
        {
            int current_section = 0;
            int done = 0;
            int ret;

            sfx->length = ov_pcm_total(&vf, -1) * 2;
            sfx->samples = (short*)malloc(sfx->length * 2);

            while((ret = ov_read(&vf, ((char*)sfx->samples) + done, sfx->length * 2 - done, &current_section)) > 0)
            {
                done += ret;
            }

            ov_clear(&vf);
        }else{
            fclose(f);
        }
    }
}

void initAudio()
{
    ReadSFX(DATA_BASE "/sfx/fiba1.ogg", &errorSound[0]);
    ReadSFX(DATA_BASE "/sfx/fiba2.ogg", &errorSound[1]);
    ReadSFX(DATA_BASE "/sfx/fiba3.ogg", &errorSound[2]);
    ReadSFX(DATA_BASE "/sfx/fiba4.ogg", &errorSound[3]);
    ReadSFX(DATA_BASE "/sfx/fiba5.ogg", &errorSound[4]);
    ReadSFX(DATA_BASE "/sfx/fiba6.ogg", &errorSound[5]);

    ReadSFX(DATA_BASE "/sfx/crunch1.ogg", &crunchSound[0]);
    ReadSFX(DATA_BASE "/sfx/crunch2.ogg", &crunchSound[1]);
    ReadSFX(DATA_BASE "/sfx/crunch3.ogg", &crunchSound[2]);
    
    ReadSFX(DATA_BASE "/sfx/jurgen1.ogg", &suckSound[0]);
    ReadSFX(DATA_BASE "/sfx/jurgen2.ogg", &suckSound[1]);
    ReadSFX(DATA_BASE "/sfx/jurgen3.ogg", &suckSound[2]);
    ReadSFX(DATA_BASE "/sfx/jurgen4.ogg", &suckSound[3]);
    ReadSFX(DATA_BASE "/sfx/jurgen5.ogg", &suckSound[4]);

    ReadSFX(DATA_BASE "/sfx/start.ogg", &startSound);

    ReadSFX(DATA_BASE "/sfx/in.ogg", &inSound);

    ReadSFX(DATA_BASE "/sfx/out.ogg", &outSound);

    initAudio_sys();
}
