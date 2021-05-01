#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "audio.h"
#include "audioProvider.h"
#include "game.h"
#include "video.h"
#include "input.h"
#include "midi.h"
#include "models.h"
#include "stage.h"
#include "readdir.h"
#include "system.h"
#include "strfunc.h"

TPlayerInfo player[MAX_PLAYERS];
TTeamInfo team[MAX_TEAMS];
int showTime[4] = {2500, 2000, 1500, 1000};
int qualityUp[4] = {9,7,6,5};
int qualityDown[4] = {10,12,14,17};
int neckTexture = -1;
int effectTexture = -1;

int perPlayerQuality = 0;
int teamGame = 0;

struct TDisplayLoc
{
    float showX, showY;
    float left, right, bottom, top;
    float angle;
    
    float hudX;
    float hudScale;
};

TDisplayLoc displayLoc[15] = {
    //1 player
    {-2.0, -2.5, -1, 1, -1, 1, 45, -1, 1},
    //2 players
    {-2.1, -2.2, -0.6, 1.8, -1.3, 1.3, 45, -1, 0.5},
    {-2.1, -2.2, -1.8, 0.6, -1.3, 1.3, 45,  0, 0.5},
    //3 players
    {-2.2, -2.0, -0.6, 2.9, -1.5, 1.5, 50, -1.00, 0.33},
    {-2.2, -2.0, -1.8, 1.8, -1.5, 1.5, 50, -0.33, 0.33},
    {-2.2, -2.0, -2.9, 0.6, -1.5, 1.5, 50,  0.33, 0.33},
    //4 players
    {-3.7, -2.0, -0.6, 4.0, -2.3, 2.3, 65, -1.0, 0.25},
    {-3.7, -2.0, -1.7, 2.8, -2.3, 2.3, 65, -0.5, 0.25},
    {-3.7, -2.0, -2.8, 1.7, -2.3, 2.3, 65,  0.5, 0.25},
    {-3.7, -2.0, -4.0, 0.6, -2.3, 2.3, 65,  0.0, 0.25},
    //5 players
    {-3.5, -2.0, -0.6, 5.0, -2.5, 2.5, 65, -1.00, 0.20},
    {-3.5, -2.0, -1.7, 3.9, -2.5, 2.5, 65, -0.60, 0.20},
    {-3.5, -2.0, -2.8, 2.8, -2.5, 2.5, 65, -0.20, 0.20},
    {-3.5, -2.0, -3.9, 1.7, -2.5, 2.5, 65,  0.20, 0.20},
    {-3.5, -2.0, -5.0, 0.6, -2.5, 2.5, 65,  0.60, 0.20},
};

void hitNote(TNote* n, int p)
{
    currentAudioProvider->setEnabled(1, player[p].instrument);
    n->hit[p] = 50; //Not getting points for the first 50ms
    player[p].myPlayNote = n->next;
    player[p].score += 50 * player[p].multiplier;
    player[p].streak ++;
    if (player[p].streak > player[p].bestStreak)
        player[p].bestStreak = player[p].streak;
    if (player[p].streak > (10 * player[p].multiplier) - 1 && player[p].multiplier < 4)
        player[p].multiplier++;
    
    if ((player[p].streak % 50  ) == 0)
    {
        sprintf(player[p].effectText, "%i Streak!", player[p].streak);
        player[p].effectTime = getTime();
    }
    
    if (perPlayerQuality)
        player[p].quality += qualityUp[player[p].difficulty];
    if (player[p].quality > 1000)
        player[p].quality = 1000;
    if (teamGame)
        team[player[p].team].quality += qualityUp[player[p].difficulty];
    if (team[player[p].team].quality > 1000)
        team[player[p].team].quality = 1000;
    player[p].myHitNote = n;
}

void misNote(int p)
{
    currentAudioProvider->setEnabled(0, player[p].instrument);
    player[p].streak = 0;
    player[p].multiplier = 1;
    if (perPlayerQuality && player[p].quality > 0)
    {
        player[p].quality -= qualityDown[player[p].difficulty];
        if (player[p].quality <= 0)
        {
            player[p].quality = 0;
            playSuck();
        }
    }
    if (teamGame && team[player[p].team].quality > 0)
    {
        team[player[p].team].quality -= qualityDown[player[p].difficulty];
        if (team[player[p].team].quality <= 0)
        {
            team[player[p].team].quality = 0;
            for(int i=0;i<MAX_PLAYERS;i++)
            {
                if (player[p].team == player[i].team)
                    player[i].quality = 0;
            }
            playSuck();
        }
    }
}

void drawEffect(int effect, float size, int alpha)
{
    float x = float(effect % 4) * 0.25f;
    float y = float(effect / 4) * 0.25f;
    
    drawTexturedQuad(TT_Neck, effectTexture, x, y, 0.25, 0.25, size, size, alpha);
}

void drawEffect(int effect, float size, const int* color, int alpha)
{
    float x = float(effect % 4) * 0.25f;
    float y = float(effect / 4) * 0.25f;
    
    drawTexturedQuad(TT_Neck, effectTexture, x, y, 0.25, 0.25, size, size, color, alpha);
}

void drawEffect(int effect, float sizeX, float sizeY, const int* color, int alpha)
{
    float x = float(effect % 4) * 0.25f;
    float y = float(effect / 4) * 0.25f;
    
    drawTexturedQuad(TT_Neck, effectTexture, x, y, 0.25, 0.25, sizeX, sizeY, color, alpha);
}

int playGame()
{
    freeTextures(TT_Neck);
    
    {
        char buffer[1024];
        nprintf(buffer, 1024, DATA_BASE "/stage/%s/neck.png", getStagePath());
        neckTexture = loadTexture(buffer, TT_Neck);
        if (neckTexture < 0)
            neckTexture = loadTexture(DATA_BASE "/gfx/neck.png", TT_Neck);
        
        nprintf(buffer, 1024, DATA_BASE "/stage/%s/neck_effects.png", getStagePath());
        effectTexture = loadTexture(buffer, TT_Neck);
        if (effectTexture < 0)
            effectTexture = loadTexture(DATA_BASE "/gfx/neck_effects.png", TT_Neck);
        
        nprintf(buffer, 1024, DATA_BASE "/stage/%s/note.raw", getStagePath());
        noteModel = new T3DModel(buffer);
        if (!noteModel->valid())
            noteModel = defaultNoteModel;
        nprintf(buffer, 1024, DATA_BASE "/stage/%s/note_pull.raw", getStagePath());
        notePullModel = new T3DModel(buffer);
        if (!notePullModel->valid())
            notePullModel = defaultNotePullModel;
        nprintf(buffer, 1024, DATA_BASE "/stage/%s/note_bg.raw", getStagePath());
        noteBgModel = new T3DModel(buffer);
        if (!noteBgModel->valid())
            noteBgModel = defaultNoteBgModel;
    }
    
    int playerCount = 0;
    for(int p=0;p<MAX_PLAYERS;p++)
    {
        if (!player[p].playing || player[p].difficulty < 0 || player[p].instrument < 0 || (teamGame && player[p].team < 0))
        {
            player[p].playing = 0;
            player[p].instrument = -1;
            player[p].difficulty = -1;
            player[p].team = -1;
            continue;
        }
        player[p].playAsDrums = 0;
        if (getInputState(p) & INPUT_HAS_DRUMS)
            player[p].playAsDrums = 1;
        else if (p > 3 && player[p].instrument == 2) //keyboard players as drums.
            player[p].playAsDrums = 1;
        playerCount++;
    }
    
    int displayLocCnt = 0;
    switch(playerCount)
    {
    case 0: return 0;
    case 1: displayLocCnt = 0; break;
    case 2: displayLocCnt = 1; break;
    case 3: displayLocCnt = 3; break;
    case 4: displayLocCnt = 6; break;
    case 5: displayLocCnt = 10; break;
    }
    for(int p=0;p<MAX_PLAYERS;p++)
    {
        if (!player[p].playing)
            continue;
        if (!teamGame || player[p].team == 0)
            player[p].displayLocation = displayLocCnt++;
    }
    for(int p=0;p<MAX_PLAYERS;p++)
    {
        if (!player[p].playing)
            continue;
        if (teamGame && player[p].team != 0)
            player[p].displayLocation = displayLocCnt++;
    }
    for(int p=0;p<MAX_PLAYERS;p++)
    {
        if (!player[p].playing)
            continue;
        player[p].myShowNote = track.difficulty[player[p].difficulty].noteList[player[p].instrument];
        player[p].myPlayNote = player[p].myShowNote;
        player[p].showTime = showTime[player[p].difficulty];
        player[p].quality = 500;
        
        for(TNote* n = player[p].myShowNote; n; n = n->next)
        {
            n->hit[p] = 0;
        }
    }
    for(int i=0;i<MAX_TEAMS;i++)
    {
        team[i].reset();
    }
    
    playStart();
    currentAudioProvider->setPause(0);
    
    int fade = getTime();
    int fadeIn = 1;
    int fadeOut = 0;
    int quit = 0;
    int menu = -1;
    while(!quit)
    {
        int playingCount = 0;
        updateInputs(false);
        currentAudioProvider->updatePosition();
        
        /** Do the game logic */
        for(int p=0;p<MAX_PLAYERS;p++)
        {
            if (!player[p].playing)
                continue;
            if (player[p].quality > 0)
                playingCount++;

            if (menu > -1)
            {
                //Menu open, no game logic.
                int newInput = getInputPress(p);
                if (newInput & BUTTON_MENU_UP)
                {
                    menu = (menu + 1) % 2;
                }
                if (newInput & BUTTON_MENU_DOWN)
                {
                    menu = (menu + 1) % 2;
                }
                if (newInput & BUTTON_MENU_SELECT)
                {
                    switch(menu)
                    {
                    case 0://Resume
                        menu = -1;
                        currentAudioProvider->setPause(0);
                        break;
                    case 1://Quit
                        menu = -1;
                        currentAudioProvider->setPause(0);
                        quit = 2;
                        break;
                    }
                }
                if (newInput & BUTTON_MENU_CANCEL)
                {
                    menu = -1;
                    currentAudioProvider->setPause(0);
                }
            }else{
                //No menu, game logic here.
                int input = getInputState(p);
                int newInput = getInputPress(p);
                
                if (newInput & BUTTON_MENU)
                {
                    menu = 0;
                    currentAudioProvider->setPause(1);
                }
                
                if (player[p].myShowNote && player[p].myShowNote->time + player[p].myShowNote->length - currentAudioProvider->position() < -400)
                {
                    player[p].myShowNote = player[p].myShowNote->next;
                }

                if (!player[p].playAsDrums)
                {
                    int keyMask = 0;
                    
                    for(int i=0;i<5;i++)
                    {
                        if (input & BUTTON_FRET(i))
                        {
                            keyMask |= (1 << i);
                        }
                    }
                    
                    if (player[p].quality > 0 && newInput & BUTTON_STRUM)
                    {
                        int hit = 0;
                        TNote* n;
                        for(n = player[p].myPlayNote; n && n->time - currentAudioProvider->position() < player[p].showTime * (player[p].precisionMode ? 0.05 : 0.1); n = n->next)
                        {
                            if (n->match(keyMask))
                            {
                                if (n != player[p].myPlayNote)
                                {
                                    //skipped a note.
                                    misNote(p);
                                }
                                hitNote(n, p);
                                hit = 1;
                                break;
                            }
                        }
                        if (!hit)
                        {
                            //Might just match a 'pull' note which has already fired.
                            for(TNote* n = player[p].myShowNote; n && n->time - currentAudioProvider->position() < player[p].showTime * (player[p].precisionMode ? 0.05 : 0.1); n = n->next)
                            {
                                if (n->type == NT_Pull && n->match(keyMask) && n->time - currentAudioProvider->position() > player[p].showTime * -(player[p].precisionMode ? 0.05 : 0.1))
                                {
                                    hit = 1;
                                }
                            }
                            
                            //MISS!
                            if (!hit)
                            {
                                playError();
                                player[p].myHitNote = NULL;
                                misNote(p);
                            }
                        }
                    }
                    
                    //play hammer-on/pull-off notes
                    if (player[p].myPlayNote && player[p].streak > 0 && player[p].myPlayNote->type == NT_Pull && player[p].myPlayNote->time - currentAudioProvider->position() < player[p].showTime * (player[p].precisionMode ? 0.05 : 0.1))
                    {
                        if (player[p].myPlayNote->match(keyMask))
                        {
                            hitNote(player[p].myPlayNote, p);
                        }
                    }
                    
                    //discard old notes that are beyond playing
                    while(player[p].myPlayNote && player[p].myPlayNote->time - currentAudioProvider->position() < player[p].showTime * -(player[p].precisionMode ? 0.05 : 0.1))
                    {
                        if (player[p].quality > 0 && !player[p].myPlayNote->hit[p])
                        {
                            //Missed!
                            misNote(p);
                        }
                        player[p].myPlayNote = player[p].myPlayNote->next;
                    }
                    
                    if (player[p].quality > 0 && player[p].myHitNote && player[p].myHitNote->match(keyMask) && player[p].myHitNote->time + player[p].myHitNote->length - currentAudioProvider->position() > player[p].showTime * -(player[p].precisionMode ? 0.05 : 0.1))
                    {
                        //Still holding note
                        if (currentAudioProvider->position() - player[p].myHitNote->time > player[p].myHitNote->hit[p])
                        {
                            int score = ((currentAudioProvider->position() - player[p].myHitNote->time) - player[p].myHitNote->hit[p]) / 15;
                            player[p].score += score * player[p].multiplier;
                            player[p].myHitNote->hit[p] += score * 15;
                        }
                    }else{
                        player[p].myHitNote = NULL;
                    }
                }else{
                    //Play as drums
                    if (player[p].quality > 0 && (newInput & (BUTTON_DRUM_BASS | BUTTON_DRUM_PAD_ANY)))
                    {
                        //Small 'add up hit' to count for small time difference in the hits.
                        player[p].drumCountDown = 2;
                        player[p].drumHitMask |= newInput;
                    }
                    
                    if (player[p].drumCountDown > 0 && (--player[p].drumCountDown) == 0)
                    {
                        int hit = 0;
                        TNote* n;
                        for(n = player[p].myPlayNote; n && n->time - currentAudioProvider->position() < player[p].showTime * (player[p].precisionMode ? 0.05 : 0.1); n = n->next)
                        {
                            if (n->matchDrums(player[p].drumHitMask))
                            {
                                if (n != player[p].myPlayNote)
                                {
                                    //skipped a note.
                                    misNote(p);
                                }
                                hitNote(n, p);
                                hit = 1;
                                break;
                            }
                        }
                        if (!hit)
                        {
                            //MISS!
                            if (!hit)
                            {
                                playError();
                                player[p].myHitNote = NULL;
                                misNote(p);
                            }
                        }
                        player[p].drumHitMask = 0;
                    }
                    
                    //discard old notes that are beyond playing
                    while(player[p].myPlayNote && player[p].myPlayNote->time - currentAudioProvider->position() < player[p].showTime * -(player[p].precisionMode ? 0.05 : 0.1))
                    {
                        if (player[p].quality > 0 && !player[p].myPlayNote->hit[p])
                        {
                            //Missed!
                            misNote(p);
                        }
                        player[p].myPlayNote = player[p].myPlayNote->next;
                    }
                    
                    if (player[p].quality > 0 && player[p].myHitNote && player[p].myHitNote->time - currentAudioProvider->position() > player[p].showTime * -(player[p].precisionMode ? 0.05 : 0.1))
                    {
                        //Note not yet passed window, so keep it referenced so we see the playing effect.
                    }else{
                        player[p].myHitNote = NULL;
                    }
                }
            }
        }
        if ((currentAudioProvider->reachedEnd() || playingCount < 1) && !fadeOut)
        {
            if (playingCount < 1)
            {
                currentAudioProvider->setPause(1);
                playOut();
            }
            fadeOut = 1;
            fade = getTime();
        }
        
        if (fadeOut && (getTime() - fade) > 1500)
            quit = 1;
        if (fadeIn && (getTime() - fade) > 1000)
            fadeIn = 0;
        
        /** Do the drawing */
        
        /** Draw the stage */
        drawStage();
        
        /** Draw the necks */
        for(int p=0;p<MAX_PLAYERS;p++)
        {
            if (!player[p].playing)
                continue;
            
            TDisplayLoc* dLoc = &displayLoc[player[p].displayLocation];
            float showY = dLoc->showX;
            float showZ = dLoc->showY;
            float angle = dLoc->angle;
            float neckOffset = fmod(float(currentAudioProvider->position() + 10000) / float(player[p].showTime) * 3, 1);
            setFrustum(dLoc->left, dLoc->right, dLoc->bottom, dLoc->top);
            
            if (fadeIn)
            {
                angle -= (1000 - (getTime() - fade)) * 0.07;
                showY -= (1000 - (getTime() - fade)) * 0.01;
            }
            if (fadeOut)
            {
                angle -= (getTime() - fade) * 0.07;
                showY -= (getTime() - fade) * 0.01;
            }
            
            int input = getInputState(p);
            int keyMask = 0;
            for(int i=0;i<5;i++)
            {
                if (input & BUTTON_FRET(i))
                {
                    keyMask |= (1 << i);
                }
            }
            
            //Draw the neck
            if (player[p].quality > 0 && player[p].quality < 256)
            {
                int alpha = (255 - player[p].quality);
                int anim = getTime() % 512;
                if (anim < 256)
                    alpha = alpha * anim / 255;
                else
                    alpha = alpha * (511-anim) / 255;
                
                setDrawPos(0, showY, showZ, angle, 0, -0.02, -1, 0,90,0);
                drawQuad(1, 3, keyColor[1], alpha);
                setDrawPos(0, showY, showZ, angle, 0, -0.02, -5, 0,90,0);
                drawQuadFade(1, 1, keyColor[1], alpha, 0);
            }
            
            setDrawPos(0,showY,showZ, angle,  0,-0.01,-6+neckOffset, 0,90,0);
            drawTexturedQuadFade(TT_Neck, neckTexture, 0,0, 1,neckOffset, 1,neckOffset, int(255 * neckOffset), 0);
            setDrawPos(0,showY,showZ, angle,  0,-0.01,-5+neckOffset, 0,90,0);
            drawTexturedQuadFade(TT_Neck, neckTexture, 0, neckOffset, 1,1-neckOffset, 1,1-neckOffset, 255, int(255 * neckOffset));
            
            setDrawPos(0,showY,showZ,  angle,  0,-0.01,-4+neckOffset, 0,90,0);
            drawTexturedQuad(TT_Neck, neckTexture, 0,0, 1,neckOffset, 1,neckOffset, 255);
            for(int i=0;i<3;i++)
            {
                setDrawPos(0,showY,showZ,  angle,  0,-0.01,-3+i*2+neckOffset*2, 0,90,0);
                drawTexturedQuad(TT_Neck, neckTexture, 0,0, 1,1, 1,1, 255);
            }
            
            //Draw beat lines (this does not look correct yet IMHO)
            /*
            float timePerBeat = 60.0 / track.beatsPerMinute * 1000;
            float lineOffset = fmod(float(currentAudioProvider->position() + 1000) / timePerBeat * 2, 1);
            for(float f = lineOffset * -timePerBeat; f < float(player[p].showTime) * 3; f += timePerBeat)
            {
                setDrawPos(0,showY,showZ,  angle,  0, 0.0, -f / float(player[p].showTime) * 3, 0,90,0);
                drawQuad(1,0.03, neckColor[0], 255);
            }
            */
            
            if (!player[p].playAsDrums)
            {
                //Draw the keys
                for(int i=0;i<5;i++)
                {
                    if (player[p].leftyFlip)
                        setDrawPos(0,showY,showZ, angle, 0.8f - i * 0.4,-0.005, 0, 0, 90, 0);
                    else
                        setDrawPos(0,showY,showZ, angle, i * 0.4 - 0.8f,-0.005, 0, 0, 90, 0);
                    if (keyMask & (1 << i))
                    {
                        drawEffect(7, 0.2, keyColor[i], 255);
                    }else{
                        drawEffect(6, 0.2, keyColor[i], 172);
                    }
                }

                for(TNote* n = player[p].myShowNote; n && n->time - currentAudioProvider->position() < player[p].showTime; n = n->next)
                {
                    for(int i=0;i<5;i++)
                    {
                        if (n->mask & (1 << i))
                        {
                            float drawX = i * 0.4 - 0.8f;
                            if (player[p].leftyFlip)
                                drawX = -drawX;
                            int* color = keyColor[i];
                            float drawPos = -float(n->time - currentAudioProvider->position()) / player[p].showTime * 6;
                            int alpha = 255;
                            if (drawPos < -4)
                            {
                                alpha = int((drawPos + 6) * 127);
                            }
                            if (player[p].quality < 1)
                            {
                                color = grayColor;
                            }
                            
                            if (n->time - currentAudioProvider->position() < player[p].showTime * -0.05 && n != player[p].myHitNote)
                            {
                                color = grayColor;
                            }
                            
                            if (!(n->hit[p]))
                            {
                                setDrawPos(0,showY,showZ, angle, drawX,0.01,drawPos, 0, 0,0);
                                noteBgModel->Draw(noColor, noteModelType, alpha);
                                if (n->type == NT_Pull)
                                    notePullModel->Draw(color, noteModelType, alpha);
                                else
                                    noteModel->Draw(color, noteModelType, alpha);
                            }
                            
                            int len = n->length;
                            if (n->time + len - currentAudioProvider->position() > player[p].showTime)
                            {
                                len = player[p].showTime - (n->time - currentAudioProvider->position());
                            }
                            if (n->length > 150)
                            {
                                setDrawPos(0,showY,showZ, angle, drawX, 0, -float(n->time + len/2 - currentAudioProvider->position()) / player[p].showTime * 6, 0,90, 0);
                                drawQuad(0.03, float(len/2) / player[p].showTime * 6, color, alpha);
                                if (n == player[p].myHitNote)
                                {
                                    setDrawPos(0,showY,showZ, angle, drawX+0.06, 0, -float(n->time + len/2 - currentAudioProvider->position()) / player[p].showTime * 6, 90,90, 0);
                                    drawQuadFade(float(len/2) / player[p].showTime * 6, 0.03, color, 0, alpha);
                                    
                                    setDrawPos(0,showY,showZ, angle, drawX-0.06, 0, -float(n->time + len/2 - currentAudioProvider->position()) / player[p].showTime * 6, 90,90, 0);
                                    drawQuadFade(float(len/2) / player[p].showTime * 6, 0.03, color, alpha, 0);
                                }
                            }
                        }
                    }
                }
            }else{
                //playAsDrums
                
                //Draw the pads
                for(int i=0;i<4;i++)
                {
                    setDrawPos(0,showY,showZ, angle, i * 0.4 - 0.6f,-0.005, 0, 0, 90, 0);
                    drawEffect(6, 0.2, drumColor[i], 172);
                }

                for(TNote* n = player[p].myShowNote; n && n->time - currentAudioProvider->position() < player[p].showTime; n = n->next)
                {
                    float drawPos = -float(n->time - currentAudioProvider->position()) / player[p].showTime * 6;
                    int alpha = 255;
                    if (drawPos < -4)
                    {
                        alpha = int((drawPos + 6) * 127);
                    }
                    
                    //(mask & 1) == bass
                    if (n->mask & 1)
                    {
                        int* color = drumColor[4];
                        if (player[p].quality < 1)
                        {
                            color = grayColor;
                        }
                        
                        if (n->time - currentAudioProvider->position() < player[p].showTime * -0.05 && n != player[p].myHitNote)
                        {
                            color = grayColor;
                        }
                        
                        if (!(n->hit[p]))
                        {
                            setDrawPos(0,showY,showZ, angle, 0,0.01,drawPos, 0, 90, 0);
                            drawEffect(5, 1, 0.1, color, 255);
                        }
                    }
                    for(int i=0;i<4;i++)
                    {
                        if (n->mask & (1 << (i+1)))
                        {
                            float drawX = i * 0.4 - 0.6f;
                            int* color = drumColor[i];
                            if (player[p].quality < 1)
                            {
                                color = grayColor;
                            }
                            
                            if (n->time - currentAudioProvider->position() < player[p].showTime * -0.05 && n != player[p].myHitNote)
                            {
                                color = grayColor;
                            }
                            
                            if (!(n->hit[p]))
                            {
                                setDrawPos(0,showY,showZ, angle, drawX,0.01,drawPos, 0, 0,0);
                                noteBgModel->Draw(noColor, noteModelType, alpha);
                                noteModel->Draw(color, noteModelType, alpha);
                            }
                        }
                    }
                }
            }
            
            setZBuffer(false);
            //Draw rockMeter AKA quality
            if (perPlayerQuality)
            {
                setDrawPos(-1.2, showY, showZ, angle, 0,-0.012,-0.8f, 0, 90, 0);
                drawEffect(8, 0.26, 255);
                setDrawPos(-1.2, showY, showZ, angle, 0,-0.011,-1.2f, 0, 90, 0);
                drawEffect(9, 0.26, 255);
                setDrawPos(-1.2, showY, showZ, angle, 0,-0.010,-1.6f, 0, 90, 0);
                drawEffect(10, 0.26, 255);
                if (player[p].quality > 666)
                {
                    setDrawPos(-1.2, showY, showZ, angle, 0,-0.009,-1.6f, 0, 90, 0);
                    drawEffect(14, 0.26, 255);
                }else if (player[p].quality > 333)
                {
                    setDrawPos(-1.2, showY, showZ, angle, 0,-0.009,-1.2f, 0, 90, 0);
                    drawEffect(13, 0.26, 255);
                }else if (player[p].quality > 0)
                {
                    setDrawPos(-1.2, showY, showZ, angle, 0,-0.009,-0.8f, 0, 90, 0);
                    drawEffect(12, 0.26, 255);
                }

                setDrawPos(-1.15, showY, showZ, angle, 0, 0.008,-0.620f - 0.0011f * player[p].quality, 0, 90, 0);
                drawEffect(11, 0.17, 255);
            }
            
            //Streak meter
            for(int i=0;i<10;i++)
            {
                setDrawPos(1.1, showY, showZ, angle, 0,-0.01 + 0.001*i,-0.5f - 0.12*i, 0,90,0);
                drawEffect(4, 0.1, 255);
                if (player[p].streak >= 30 || player[p].streak % 10 > i)
                {
                    setDrawPos(1.1, showY, showZ, angle, 0,-0.0095 + 0.001*i,-0.5f - 0.12*i, 0,90,0);
                    drawEffect(5, 0.1, 255);
                }
            }
            if (player[p].multiplier > 1)
            {
                setDrawPos(1.2, showY, showZ, angle, 0,0.20,-1.7f, 0, -angle, 0);
                drawEffect(player[p].multiplier - 1 , 0.4, 255);
            }

            //Draw "playing" effect
            if (player[p].myHitNote)
            {
                if (!player[p].playAsDrums)
                {
                    for(int i=0;i<5;i++)
                    {
                        if (player[p].myHitNote->mask & (1 << i))
                        {
                            if (player[p].leftyFlip)
                                setDrawPos(0.8f*0.75f-i*0.4f*0.75f, showY*0.75, showZ*0.75, 0, 0,0,0, 0,0, rand() % 360);
                            else
                                setDrawPos(i*0.4f*0.75f-0.8f*0.75f, showY*0.75, showZ*0.75, 0, 0,0,0, 0,0, rand() % 360);
                            drawEffect(0, 0.3, 255);
                        }
                    }
                }else{
                    for(int i=0;i<4;i++)
                    {
                        if (player[p].myHitNote->mask & (1 << (i+1)))
                        {   //Pad effects
                            setDrawPos(i*0.4f*0.75f-0.6f*0.75f, showY*0.75, showZ*0.75, 0, 0,0,0, 0,0, rand() % 360);
                            drawEffect(0, 0.3, 255);
                        }else if (player[p].myHitNote->mask & 1)
                        {   //Bass effect
                            setDrawPos(i*0.4f*0.75f-0.6f*0.75f, showY*0.75, showZ*0.75, 0, 0,0,0, 0,0, rand() % 360);
                            drawEffect(0, 0.18, 255);
                        }
                    }
                }
            }
            setZBuffer(true);
        }
        
        /** Draw HUD */
        setDefaultFrustum();

        for(int p=0;p<MAX_PLAYERS;p++)
        {
            if (!player[p].playing)
                continue;
            
            TDisplayLoc* dLoc = &displayLoc[player[p].displayLocation];

            char buffer[128];
            sprintf(buffer, "%i", player[p].score);
            drawText(buffer, dLoc->hudX + 0.1, 0.75, 1.4);
            
            if (player[p].effectTime != -1)
            {
                int time = getTime() - player[p].effectTime;
                if (time < 1000)
                {
                    drawTextRotated(player[p].effectText, dLoc->hudX + 1 * dLoc->hudScale, 0, 3, logf(time) / 2.71 * 360.0f + 140);
                }else if (time < 3000)
                {
                    drawTextRotated(player[p].effectText, dLoc->hudX + 1 * dLoc->hudScale, 0, 3 - (time - 1000) * 0.0015, logf(time) / 2.71 * 360.0f + 140);
                }
                else
                    player[p].effectTime = -1;
            }
        }
        if (teamGame)
        {
            float showX = -2.8;
            float showY = -1;
            float showZ = -3.0;
            float angle = 90;
            for(int i=0;i<MAX_TEAMS;i++)
            {
                if (i == 0)
                    showX = -2.8;
                else
                    showX =  2.8;
                setDrawPos(showX, showY, showZ, angle, 0,-0.012,-0.8f, 0, 90, 0);
                drawEffect(8, 0.26, 255);
                setDrawPos(showX, showY, showZ, angle, 0,-0.011,-1.2f, 0, 90, 0);
                drawEffect(9, 0.26, 255);
                setDrawPos(showX, showY, showZ, angle, 0,-0.010,-1.6f, 0, 90, 0);
                drawEffect(10, 0.26, 255);
                if (team[i].quality > 666)
                {
                    setDrawPos(showX, showY, showZ, angle, 0,-0.009,-1.6f, 0, 90, 0);
                    drawEffect(14, 0.26, 255);
                }else if (team[i].quality > 333)
                {
                    setDrawPos(showX, showY, showZ, angle, 0,-0.009,-1.2f, 0, 90, 0);
                    drawEffect(13, 0.26, 255);
                }else if (team[i].quality > 0)
                {
                    setDrawPos(showX, showY, showZ, angle, 0,-0.009,-0.8f, 0, 90, 0);
                    drawEffect(12, 0.26, 255);
                }

                setDrawPos(showX + 0.05, showY, showZ, angle, 0, 0.008,-0.620f - 0.0011f * team[i].quality, 0, 90, 0);
                drawEffect(11, 0.17, 255);
            }
        }
        
        if (menu > -1)
        {
            //Make the rest dark.
            setZBuffer(0);
            setDrawPos(0, 0, -2.1);
            drawQuad(3,3, noColor, 128);
            setZBuffer(1);
            
            drawTextCenter("PAUSE", 0.0, 0.8, 4.0);

            setDrawPos(0.0,0.475f - menu * 0.40f,-2);
            drawQuad(2, 0.1, keyColor[0], 255);
            
            setDrawPos(-0.75,0.35625f - menu * 0.30f,-1.5);
            drawMenuObject(1, 0.3, 255);
            
            drawText("Resume", -0.4, 0.2, 2);
            drawText("Quit", -0.4, 0.0, 2);
        }
        
        drawIRDrums();
        flipBuffers();
    }
    
    for(int p=4;p<MAX_PLAYERS;p++)
    {
        //Reset playAsDrums for keyboard players, so the menus don't act on drum keys.
        player[p].playAsDrums = 0;
    }

    if (noteModel != defaultNoteModel)
        delete noteModel;
    if (notePullModel != defaultNotePullModel)
        delete notePullModel;
    if (noteBgModel != defaultNoteBgModel)
        delete noteBgModel;
    
    return quit == 2;
}
