#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <SDL/SDL.h>
#include <string.h>
#include <math.h>

#include "AudioProvider.h"
#include "system.h"

#define MIX(sample, newsample) { int __i = (int)(sample) + (int)(newsample) / 2; if (__i < -0x7FFF) __i = -0x7FFF; if (__i > 0x7FFF) __i = 0x7FFF; sample = __i;}

TAudioProvider nullAudioProvider;
TAudioProvider* currentAudioProvider = &nullAudioProvider;
TAudioProvider* cleanupAudioProvider = NULL;

void OGGFileInfo::doPreloadStep()
{
    if (f)
    {
        int i = fread(buffer + loadedSize, 1, 8192, f);
        if (i <= 0)
        {
            fclose(f);
            f = NULL;
        }else{
            loadedSize += i;
        }
    }else{
        loadedSize = bufferSize;
    }
}

size_t OGG_read(void *ptr, size_t size, size_t nmemb, void *datasource)
{
    OGGFileInfo* fi = (OGGFileInfo*)datasource;
    int len = size*nmemb;
    
    if (fi->bufferPos + len > fi->bufferSize)
    {
        len = fi->bufferSize - fi->bufferPos;
    }
    while (fi->bufferPos + len > fi->loadedSize)
    {
        fi->doPreloadStep();
    }
    
    memcpy(ptr, fi->buffer + fi->bufferPos, len);
    fi->bufferPos += len;
    return len/size;
}
int OGG_seek(void *datasource, ogg_int64_t offset, int whence)
{
    int newPos = -1;
    OGGFileInfo* fi = (OGGFileInfo*)datasource;
    if (whence == SEEK_SET)
    {
        newPos = offset;
    }
    if (whence == SEEK_CUR)
    {
        newPos = fi->bufferPos + offset;
    }
    if (whence == SEEK_END)
    {
        newPos = fi->bufferSize + offset;
    }
    if (newPos < 0 || newPos >= fi->bufferSize)
        return -1;
    fi->bufferPos = newPos;
    return 0;
}
int OGG_close (void *datasource)
{
    OGGFileInfo* fi = (OGGFileInfo*)datasource;
    fi->bufferPos = 0;
    return 0;
}
long OGG_tell(void *datasource)
{
    OGGFileInfo* fi = (OGGFileInfo*)datasource;
    return fi->bufferPos;
}

ov_callbacks callbacks = {
    OGG_read,
    OGG_seek,
    OGG_close,
    OGG_tell
};

TOGGProvider::TOGGProvider(const char* file)
{
    int size = -1;
    valid = false;
    section = 0;
    pos = internPos = 0;
    mutex = createMutex();

    fi.buffer = NULL;
    fi.bufferSize = 0;
    
    fi.f = fopen(file, "rb");
    if (fi.f)
    {
        fseek(fi.f, 0, SEEK_END);
        fi.bufferSize = ftell(fi.f);
        fseek(fi.f, 0, SEEK_SET);

        if (fi.bufferSize <= 0)
        {
            fi.bufferSize = 0;
            fclose(fi.f);
            return;
        }

        fi.buffer = (unsigned char *) malloc(fi.bufferSize);
        if (fi.buffer == NULL)
        {
            fi.bufferSize = 0;
            fclose(fi.f);
            return;
        }
        
        fi.loadedSize = 0;
        fi.bufferPos = 0;
        int ret = ov_open_callbacks(&fi, &vf, NULL, 0, callbacks);
        if (ret > -1)
        {
            size = ov_pcm_total(&vf,-1);
            valid = 1;
        }
    }
}

int TOGGProvider::preloadDone()
{
    lockMutex(mutex);
    if (fi.bufferSize == 0)
    {
        unlockMutex(mutex);
        return 100;
    }
    int done = fi.loadedSize * 100 / fi.bufferSize;
    if (done == 100 && fi.loadedSize < fi.bufferSize)
    {
        unlockMutex(mutex);
        return 99;
    }
    unlockMutex(mutex);
    return done;
}

void TOGGProvider::doPreloadStep()
{
    lockMutex(mutex);
    fi.doPreloadStep();
    unlockMutex(mutex);
}

TOGGProvider::~TOGGProvider()
{
    if (valid)
        ov_clear(&vf);
    if (fi.buffer)
        free(fi.buffer);
    if (fi.f)
        fclose(fi.f);
        
    destroyMutex(mutex);
}
    
void TOGGProvider::seek(int offset)
{
    lockMutex(mutex);
    if (valid)
    {
        if (offset < 0)
            ov_time_seek(&vf, 0);
        else
            ov_time_seek(&vf, offset);
    }
    internPos = offset;
    pos = offset;
    unlockMutex(mutex);
}

int TOGGProvider::read(short* buffer, int samples)
{
    lockMutex(mutex);
    int done = 0;
    if (internPos < 0)
    {
        internPos += samples;
        if (internPos > 0)
            internPos = 0;
    }
    else
    {
        while(valid && done < (samples*4))
        {
            long ret = ov_read(&vf, ((char*)buffer) + done, (samples*4) - done, &section);
            if (ret == 0)
            {
              valid = 0;
              ov_clear(&vf);
            } else if (ret < 0) {
            } else {
                done += ret;
            }
        }
        internPos += done/4;
    }
    unlockMutex(mutex);
    return done/4;
}
    
bool TOGGProvider::reachedEnd()
{
    return !valid;
}
const int TOGGProvider::position()
{
    return pos;
}
void TOGGProvider::updatePosition()
{
    pos = internPos * 10 / 441;
}
/*
TMP3Provider::TMP3Provider(const char* file)
{
    int  channels = 0, encoding = 0;
    long rate = 0;
    int size;
    
    valid = false;
    
    mh = mpg123_new(NULL, NULL);
    mpg123_open(mh, file);
    mpg123_getformat(mh, &rate, &channels, &encoding);
    mpg123_format_none(mh);
    mpg123_format(mh, 44100, 2, MPG123_ENC_SIGNED_16);
    mpg123_getformat(mh, &rate, &channels, &encoding);

    size = mpg123_length(mh);
    valid = 1;
    pos = internPos = 0;
    
    rescale = 44100 / rate;
}
TMP3Provider::~TMP3Provider()
{
    if (valid)
    {
        mpg123_close(mh);
        mpg123_delete(mh);
    }
}

void TMP3Provider::seek(int offset)
{
    if (valid)
        mpg123_seek(mh, offset, SEEK_SET);
    internPos = offset;
    pos = offset;
}
int TMP3Provider::read(short* buffer, int samples)
{
    if (!valid)
        return 0;
    
    size_t done = 0;
    if (internPos < 0)
    {
        internPos += samples;
        if (internPos > 0)
            internPos = 0;
    }
    else
    {
        if (mpg123_read(mh, (unsigned char*)buffer, samples * 4 / rescale, &done) != MPG123_OK)
        {
            //On end or error, stop playing.
            valid = false;
            mpg123_close(mh);
            mpg123_delete(mh);
        }else{
            if (rescale == 2)
            {
                for(int i=done/2-1;i>-1;i-=2)
                {
                    buffer[i*2+0] = buffer[i];
                    buffer[i*2+1] = buffer[i+1];
                    buffer[i*2+2] = buffer[i];
                    buffer[i*2+3] = buffer[i+1];
                }
                done *= rescale;
            }
        }
        internPos += done/4;
    }
    return done/4;
}

bool TMP3Provider::reachedEnd()
{
    return !valid;
}
int TMP3Provider::position()
{
    return pos;
}
void TMP3Provider::updatePosition()
{
    pos = internPos;
}
*/
TMixedAudioProvider::TMixedAudioProvider(TAudioProvider* main, TAudioProvider* sub, TAudioProvider* drum, TAudioProvider* base)
{
    pause = 1;
    enabled = 0xf;
    this->main = main;
    this->sub = sub;
    this->drum = drum;
    this->base = base;
}
TMixedAudioProvider::~TMixedAudioProvider()
{
    delete main;
    delete sub;
    delete drum;
    delete base;
}

void TMixedAudioProvider::seek(int offset)
{
    main->seek(offset);
    sub->seek(offset);
    drum->seek(offset);
    base->seek(offset);
}

int TMixedAudioProvider::read(short* buffer, int samples)
{
    static short* internBuffer = NULL;
    static int internBufferSize = 0;
    
    if (pause)
    {
        return 0;
    }
    
    if (internBufferSize != samples * 4)
    {
        internBuffer = (short*)malloc(samples * 4);
        internBufferSize = samples * 4;
    }
    
    int baseSamples = base->read(buffer, samples);
    memset(buffer + baseSamples * 2, 0, (samples - baseSamples) * 4);
    
    int mainSamples = main->read(internBuffer, samples);
    if (enabled & 1 || (baseSamples == 0))
    {
        for(int i=0;i<mainSamples*2;i++)
        {
            MIX(buffer[i], internBuffer[i]);
        }
        if (mainSamples > baseSamples)
            baseSamples = mainSamples;
    }
    int subSamples = sub->read(internBuffer, samples);
    if (enabled & 2)
    {
        for(int i=0;i<subSamples*2;i++)
        {
            MIX(buffer[i], internBuffer[i]);
        }
        if (subSamples > baseSamples)
            baseSamples = subSamples;
    }
    int drumSamples = drum->read(internBuffer, samples);
    if (enabled & 4)
    {
        for(int i=0;i<drumSamples*2;i++)
        {
            MIX(buffer[i], internBuffer[i]);
        }
        if (drumSamples > baseSamples)
            baseSamples = drumSamples;
    }
    
    return baseSamples;
}

bool TMixedAudioProvider::reachedEnd()
{
    return main->reachedEnd();
}
const int TMixedAudioProvider::position()
{
    return main->position();
}
void TMixedAudioProvider::updatePosition()
{
    main->updatePosition();
    sub->updatePosition();
    base->updatePosition();
    drum->updatePosition();
}

int TMixedAudioProvider::preloadDone()
{
    int mainDone = main->preloadDone();
    int subDone = sub->preloadDone();
    int drumDone = drum->preloadDone();
    int baseDone = base->preloadDone();
    
    if (drumDone < mainDone && drumDone < subDone && drumDone < baseDone)
        return drumDone;
    if (mainDone < subDone && mainDone < baseDone)
        return mainDone;
    if (subDone < baseDone)
        return subDone;
    return baseDone;
}

void TMixedAudioProvider::doPreloadStep()
{
    main->doPreloadStep();
    sub->doPreloadStep();
    drum->doPreloadStep();
    base->doPreloadStep();
}

void TMixedAudioProvider::setEnabled(int enabled, int type)
{
    if (enabled)
    {
        this->enabled |= (1 << type);
    }else{
        this->enabled &=~(1 << type);
    }
}

void TMixedAudioProvider::setPause(int pause)
{
    this->pause = pause;
}

void setNewAudioProvider(TAudioProvider* newProv)
{
    if (newProv == currentAudioProvider)
        return;
    //Correctly swap the old and new audio provider so the audio callback can interrupt at any moment.
    TAudioProvider* old = currentAudioProvider;
    currentAudioProvider = newProv;
    cleanupAudioProvider = old;
}
