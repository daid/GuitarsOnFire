#ifndef AUDIO_PROVIDER_H
#define AUDIO_PROVIDER_H
/***
 * audioProviders are C++ objects that can be used to read streams of data.
 *  currently only the OGG audio provider is used, and one to mix multiple providers into a single one.
 *  the mp3 provider works, but is not required.
 */

#include "libtremor/ivorbiscodec.h"
#include "libtremor/ivorbisfile.h"

//#include "libmpg123/mpg123.h"

class TAudioProvider
{
public:
    TAudioProvider() {}
    virtual ~TAudioProvider() {}
    
    virtual void seek(int offset) {}
    virtual int read(short* buffer, int samples) {return 0;}
    
    virtual bool reachedEnd() {return false;}
    virtual const int position() {return 0;}
    virtual void updatePosition() {}
    
    virtual int preloadDone() { return 100;}
    virtual void doPreloadStep() {}
    
    virtual void setEnabled(int enabled, int type) {}
    virtual void setPause(int pause) {}
};

class OGGFileInfo
{
public:
    unsigned char* buffer;
    int bufferSize;
    int bufferPos;
    
    int loadedSize;
    FILE* f;
    
    void doPreloadStep();
};

class TOGGProvider : public TAudioProvider
{
protected:
    OGGFileInfo fi;
    bool valid;
    OggVorbis_File vf;
    int section;
    int internPos;
    int pos;
    
    void* mutex;
public:
    TOGGProvider(const char* file);
    virtual ~TOGGProvider();
    
    virtual void seek(int offset);
    virtual int read(short* buffer, int samples);
    
    virtual bool reachedEnd();
    virtual const int position();
    virtual void updatePosition();

    virtual int preloadDone();
    virtual void doPreloadStep();
};
/*
class TMP3Provider : public TAudioProvider
{
private:
    bool valid;
    mpg123_handle *mh;
    int internPos;
    int pos;
    int rescale;
public:
    TMP3Provider(const char* file);
    virtual ~TMP3Provider();
    
    virtual void seek(int offset);
    virtual int read(short* buffer, int samples);
    
    virtual bool reachedEnd();
    virtual int position();
    virtual void updatePosition();
};
*/
class TMixedAudioProvider : public TAudioProvider
{
private:
    TAudioProvider* main;
    TAudioProvider* sub;
    TAudioProvider* drum;
    TAudioProvider* base;
    
    int enabled;
    int pause;
public:
    TMixedAudioProvider(TAudioProvider* main, TAudioProvider* sub, TAudioProvider* drum, TAudioProvider* base);
    virtual ~TMixedAudioProvider();
    
    virtual void seek(int offset);
    virtual int read(short* buffer, int samples);
    
    virtual bool reachedEnd();
    virtual const int position();
    virtual void updatePosition();

    virtual int preloadDone();
    virtual void doPreloadStep();
    
    virtual void setEnabled(int enabled, int type);
    virtual void setPause(int pause);
};

extern TAudioProvider nullAudioProvider;
extern TAudioProvider* currentAudioProvider;
extern TAudioProvider* cleanupAudioProvider;

void setNewAudioProvider(TAudioProvider* newProv);

#endif//AUDIO_PROVIDER_H
