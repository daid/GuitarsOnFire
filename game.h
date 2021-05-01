#ifndef MAIN_H
#define MAIN_H

#define MAX_PLAYERS 5
#define MAX_TEAMS 2

/** the actual TNote implementation is in 'midi' */
class TNote;

class TPlayerInfo
{
public:
    int score;      //Total score points
    int streak;     //Streak, resets to 0 when misses.
    int bestStreak; 
    int multiplier; //Multiplier, 1,2,3,4
    int difficulty; //Difficultiy level 0,1,2,3
    int quality;    //Currenty play 'quality' AKA rock meter. 0-1000
    int playing;    //If we are playing (0,1)
    int showTime;   //Total time to show in the 'neck' in ms.
    int displayLocation; //Location where this player should be displayed. Depends on the number of players and which player this is.
    int instrument;
    int leftyFlip;
    int precisionMode;
    int playAsDrums;
    int drumCountDown;
    int drumHitMask;
    int team;
    
    char effectText[32];
    int effectTime;
    
    TNote* myShowNote;  //First note that should be shown, for performance reasons
    TNote* myPlayNote;  //Current note that should be played now, or later.
    TNote* myHitNote;   //Currently holding this note down.
    
    void reset()
    {
        multiplier = 1;
        score = 0;
        streak = 0;
        bestStreak = 0;
        multiplier = 1;
        difficulty = -1;
        instrument = -1;
        quality = 0;
        playing = 0;
        displayLocation = 0;
        
        playAsDrums = 0;
        drumCountDown = 0;
        drumHitMask = 0;
        team = -1;
        
        myShowNote = NULL;
        myPlayNote = NULL;
        myHitNote = NULL;
        
        effectTime = -1;
    }
};
class TTeamInfo
{
public:
    int score;
    int quality;
    
    void reset()
    {
        score = 0;
        quality = 500;
    }
};

/**
 * give everything access to the player status.
 */
extern TPlayerInfo player[MAX_PLAYERS];
extern TTeamInfo team[MAX_TEAMS];
extern int perPlayerQuality;
extern int teamGame;

/**
 * play the game. The player[].playing,.difficulty and
 * .instrument values need to be set before starting.
 * and everything has to be reset.
 */
int playGame();

#endif//MAIN_H

