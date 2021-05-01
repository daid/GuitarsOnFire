#ifndef MIDI_H
#define MIDI_H
/***
 * midi read routine, reads a midi file and extracts the notes out of it.
 *  uses code from Guitarfun, but that's abstracted from the interface.
 */

/**
 * difficulties go from expert to easy (beginner could be build from easy).
 * Instruments are: guitar, bass, drum
 */
#define MAX_DIFFICULTY 4
#define MAX_INSTRUMENT 3

#include "game.h"
#include "readdir.h"

enum ENoteType
{
    NT_Single,	//A single note is a simple note that you have to strum.
    NT_Coord,	//A coord are multiple notes on the same time frame that you have to strum.
    NT_Pull		//A pull is a single note that you don't have to strum in case you hit the previous note right. But you are allowed to strum it.
};

class TNote
{
public:
    int mask;
    int time;
    int length;
    ENoteType type;
    
    int hit[MAX_PLAYERS];
    
    int match(int keyMask);
    int matchDrums(int keyMask);
    
    TNote* next;
};

class TDifficulty
{
public:
    TNote* noteList[MAX_INSTRUMENT];
    
    TDifficulty();
    
    void reset();
};

class TTrackInfo
{
public:
    float beatsPerMinute;

    TDifficulty difficulty[MAX_DIFFICULTY];

    TTrackInfo();
    
    void reset();
    
    void load(const char* filename);
};

/**
 * there is always 1 single track. Use the track.load function to load
 * different midi files.
 */
extern TTrackInfo track;

#endif//MIDI_H
