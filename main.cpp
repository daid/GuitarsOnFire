#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>

#include "system.h"
#include "video.h"
#include "input.h"
#include "game.h"
#include "midi.h"
#include "audio.h"
#include "audioProvider.h"
#include "models.h"
#include "readdir.h"
#include "strfunc.h"
#include "stage.h"
#include "settings.h"

struct TFireParticle
{
    float x, y;
    float xd, yd, ad;
    float size;
    float alpha;
};
#define MAX_PARTICLE 5000
TFireParticle fireParticle[MAX_PARTICLE];

void drawFire()
{
    setZBuffer(0);
    
    for(int i=0;i<MAX_PARTICLE;i++)
    {
        if (fireParticle[i].alpha <= 0)
        {
            fireParticle[i].x = ((rand() % 200) - 100) * 0.1;
            fireParticle[i].y = -11;
            fireParticle[i].xd = ((rand() % 200) - 100) * 0.001;
            fireParticle[i].yd = ((rand() % 200) - 100) * 0.001;
            fireParticle[i].ad = ((rand() % 50) + 75) * 0.0001;
            fireParticle[i].alpha = ((rand() % 50)+50) * 0.01;
            fireParticle[i].size = ((rand() % 75)+50) * 0.01;
        }
        setDrawPos(fireParticle[i].x, fireParticle[i].y, -10);
        int color[3];
        if (fireParticle[i].alpha > 0.7)
        {
            color[0] = 255;
            color[1] = 255;
            color[2] = int(255 * (fireParticle[i].alpha - 0.7) * 3);
        }else if (fireParticle[i].alpha > 0.4)
        {
            color[0] = 255;
            color[1] = int(255 * (fireParticle[i].alpha - 0.4) * 3);
            color[2] = 0;
        }else{
            color[0] = 255;
            color[1] = 0;
            color[2] = 0;
        }
        drawMenuObject(2, fireParticle[i].size, color, int(255*fireParticle[i].alpha));

        fireParticle[i].x += fireParticle[i].xd;
        fireParticle[i].y += fireParticle[i].yd;
        fireParticle[i].alpha -= fireParticle[i].ad;
    }
    
    setZBuffer(1);
}

char difficultyName[MAX_DIFFICULTY][32] = {"Easy", "Normal", "Hard", "Expert"};
char instrumentName[MAX_INSTRUMENT][32] = {"Guitar", "Bass", "Drum"};

int loadSongData(TDirEntry* songEntry)
{
    TAudioProvider* nextAudioProvider;

    char path[1024];
    songEntry->getFullPath(path, 1024);
    char buffer[1024];
    nprintf(buffer, 1024, "%s/guitar.ogg", path);
    TAudioProvider* guitar = new TOGGProvider(buffer);
    nprintf(buffer, 1024, "%s/rhythm.ogg", path);
    TAudioProvider* rhythm = new TOGGProvider(buffer);
    nprintf(buffer, 1024, "%s/drums.ogg", path);
    TAudioProvider* drum = new TOGGProvider(buffer);
    nprintf(buffer, 1024, "%s/song.ogg", path);
    TAudioProvider* song = new TOGGProvider(buffer);
    nextAudioProvider = new TMixedAudioProvider(guitar, rhythm, drum, song);
    
    currentSongEntry = songEntry;
    track.load(path);
    
    setNewAudioProvider(nextAudioProvider);
    return 0;
}

#define PLAYER_MENU_DIFFICULTY  0x0001
#define PLAYER_MENU_INSTRUMENT  0x0002
#define PLAYER_MENU_TEAM        0x0004
#define PLAYER_MENU_ALL_OPTIONS 0x1000

int playerMenu(int flags)
{
    int difficultyOption[MAX_PLAYERS] = {0,0,0,0,0};
    int instrumentOption[MAX_PLAYERS] = {0,0,0,0,0};
    int teamOption[MAX_PLAYERS] = {0,0,0,0,0};
    int quit = 0;
    
    int difficultyMenuNum[MAX_INSTRUMENT][MAX_DIFFICULTY];
    int difficultyCount[MAX_INSTRUMENT] = {0, 0};
    
    int instrumentMenuNum[MAX_INSTRUMENT];
    int instrumentCount = 0;
    
    for(int i=0;i<MAX_INSTRUMENT;i++)
    {
        for(int d=MAX_DIFFICULTY-1;d>-1;d--)
        {
            if ((flags & PLAYER_MENU_ALL_OPTIONS) || track.difficulty[d].noteList[i]) {difficultyMenuNum[i][difficultyCount[i]++] = d;}
        }
        
        if (difficultyCount[i] > 0) instrumentMenuNum[instrumentCount++] = i;
    }
    
    for(int p=0;p<MAX_PLAYERS;p++)
    {
        for(int i=0;i<instrumentCount;i++)
        {
            if (instrumentMenuNum[i] == player[p].instrument)
                instrumentOption[p] = i;
            if (player[p].instrument < 0 && instrumentMenuNum[i] == 2 && getInputState(p) & INPUT_HAS_DRUMS)
                instrumentOption[p] = i;
        }
        for(int d=0;d<difficultyCount[instrumentMenuNum[instrumentOption[p]]];d++)
        {
            if (player[p].difficulty == difficultyMenuNum[instrumentMenuNum[instrumentOption[p]]][d])
            {
                difficultyOption[p] = d;
            }
        }
    }
    
    while(!quit)
    {
        drawFire();
        updateInputs(true);

        for(int i=0;i<MAX_PLAYERS;i++)
        {
            int buttons = getInputPress(i);
            if (player[i].playing)
            {
                if (player[i].instrument < 0 && (flags & PLAYER_MENU_INSTRUMENT))
                {
                    if (buttons & BUTTON_MENU_UP)
                    {
                        instrumentOption[i] = (instrumentOption[i] + instrumentCount - 1) % instrumentCount;
                    }
                    if (buttons & BUTTON_MENU_DOWN)
                    {
                        instrumentOption[i] = (instrumentOption[i] + 1) % instrumentCount;
                    }
                }
                else if (player[i].difficulty < 0 && (flags & PLAYER_MENU_DIFFICULTY))
                {
                    if (buttons & BUTTON_MENU_UP)
                    {
                        difficultyOption[i] = (difficultyOption[i] + difficultyCount[instrumentMenuNum[instrumentOption[i]]] - 1) % difficultyCount[instrumentMenuNum[instrumentOption[i]]];
                    }
                    if (buttons & BUTTON_MENU_DOWN)
                    {
                        difficultyOption[i] = (difficultyOption[i] + 1) % difficultyCount[instrumentMenuNum[instrumentOption[i]]];
                    }
                }
                else if (player[i].team < 0 && (flags & PLAYER_MENU_TEAM))
                {
                    if (buttons & BUTTON_MENU_UP)
                    {
                        teamOption[i] = (teamOption[i] + MAX_TEAMS - 1) % MAX_TEAMS;
                    }
                    if (buttons & BUTTON_MENU_DOWN)
                    {
                        teamOption[i] = (teamOption[i] + 1) % MAX_TEAMS;
                    }
                }
            }
            if (buttons & BUTTON_MENU_SELECT)
            {
                if (!player[i].playing)
                {
                    player[i].playing = 1;
                }else if (player[i].instrument < 0 && (flags & PLAYER_MENU_INSTRUMENT))
                {
                    player[i].instrument = instrumentMenuNum[instrumentOption[i]];
                    
                    if (difficultyOption[i] >= difficultyCount[instrumentMenuNum[instrumentOption[i]]])
                        difficultyOption[i] = difficultyCount[instrumentMenuNum[instrumentOption[i]]] - 1;
                }else if (player[i].difficulty < 0 && (flags & PLAYER_MENU_DIFFICULTY))
                {
                    player[i].difficulty = difficultyMenuNum[instrumentMenuNum[instrumentOption[i]]][difficultyOption[i]];
                }else if (player[i].team < 0 && (flags & PLAYER_MENU_TEAM))
                {
                    player[i].team = teamOption[i];
                }
            }
            if (buttons & BUTTON_MENU_CANCEL)
            {
                if (player[i].team >= 0 && (flags & PLAYER_MENU_TEAM))
                {
                    player[i].team = -1;
                }else if (player[i].difficulty >= 0 && (flags & PLAYER_MENU_DIFFICULTY))
                {
                    player[i].difficulty = -1;
                }else if (player[i].instrument >= 0 && (flags & PLAYER_MENU_INSTRUMENT))
                {
                    player[i].instrument = -1;
                }else if (player[i].playing)
                {
                    player[i].playing = 0;
                }else{
                    quit = 1;
                }
            }
        }

        int readyCount = 0;
        int readyToRock = 0;
        for(int i=0;i<MAX_PLAYERS;i++)
        {
            char buffer[32];
            sprintf(buffer, "Player %i", i+1);
            float x = -0.8+i*0.4;
            float y = 0;
            if (!player[i].playing)
            {
                drawTextCenter(buffer, x,y+0.1, 1.1);
                drawTextCenter("Press", x,y, 1.0);
                drawTextCenter("GREEN", x,y-0.06, 1.0);
                drawTextCenter("button", x,y-0.12, 1.0);
                drawTextCenter("to play", x,y-0.18, 1.0);
            }
            else if (player[i].instrument < 0 && (flags & PLAYER_MENU_INSTRUMENT))
            {
                setDrawPos(x,y-0.1*instrumentOption[i] + 0.02,-1);
                drawQuad(0.2, 0.04, keyColor[i], 255);

                drawTextCenter(buffer, x,y+0.1, 1.1);
                for(int d=0;d<instrumentCount;d++)
                {
                    drawTextCenter(instrumentName[instrumentMenuNum[d]], x, y-0.1f*d, 1.2);
                }
            }
            else if (player[i].difficulty < 0 && (flags & PLAYER_MENU_DIFFICULTY))
            {
                setDrawPos(x, y-0.1*difficultyOption[i] + 0.02, -1);
                drawQuad(0.2, 0.04, keyColor[i], 255);

                drawTextCenter(buffer, x,y+0.1, 1.1);
                for(int d=0;d<difficultyCount[instrumentMenuNum[instrumentOption[i]]];d++)
                {
                    drawTextCenter(difficultyName[difficultyMenuNum[instrumentMenuNum[instrumentOption[i]]][d]], x, y-0.1f*d, 1.2);
                }
            }
            else if (player[i].team < 0 && (flags & PLAYER_MENU_TEAM))
            {
                setDrawPos(x, y-0.1*teamOption[i] + 0.02, -1);
                drawQuad(0.2, 0.04, keyColor[i], 255);

                drawTextCenter(buffer, x,y+0.1, 1.1);
                for(int d=0;d<MAX_TEAMS;d++)
                {
                    if (d == 0)
                        drawTextCenter("Team 1", x, y-0.1f*d, 1.2);
                    else
                        drawTextCenter("Team 2", x, y-0.1f*d, 1.2);
                }
            }else{
                drawTextCenter(buffer, x,y+0.1, 1.1);
                drawTextCenter("Ready", x,y, 1.4);
                drawTextCenter(difficultyName[difficultyMenuNum[instrumentMenuNum[instrumentOption[i]]][difficultyOption[i]]], x,y-0.1, 1.2);
                drawTextCenter(instrumentName[instrumentMenuNum[instrumentOption[i]]], x,y-0.2, 1.2);
                
                setDrawPos(x * 2, y - 1.0, -2);
                drawMenuObject(0, 0.5, 255);
                
                readyCount ++;
            }
        }
        if (!teamGame && readyCount > 0)
            readyToRock = 1;
        if (teamGame && readyCount > 1)
            readyToRock = 1;
        if (readyToRock)
        {
            drawTextCenter("Press \x80 to rock!", 0, 0.35, 2.0);
        }
        
        int preload = currentAudioProvider->preloadDone();
        if (preload < 100)
        {
            currentAudioProvider->doPreloadStep();
        }

        if (!(flags & PLAYER_MENU_ALL_OPTIONS))
        {
            if (currentSongEntry->info->artist)
                drawTextCenter(currentSongEntry->info->artist, 0, 0.6, 2.3);
            drawTextCenter(currentSongEntry->info->name, 0, 0.5, 2.5);
        
            char buffer[32];
            sprintf(buffer, "Loading: %i%%", preload);
            drawText(buffer, -0.8,-0.9, 1.5);
        }
        drawTextCenter("Guitars On Fire", 0, 0.8, 3);
        flipBuffers();
        
        if (readyToRock && (getInputPress(-1) & BUTTON_MENU))
        {
            quit = 2;
        }
    }
    
    return quit == 2;
}

void loadingScreen()
{
    int duck = rand() % 50; //Random ducky!
    int preload = 0;
    while(preload < 100)
    {
        drawFire();
        updateInputs(true);
        
        preload = currentAudioProvider->preloadDone();
        if (preload < 100)
        {
            while(preload == currentAudioProvider->preloadDone())
            {
                currentAudioProvider->doPreloadStep();
            }
        }
        char buffer[32];
        sprintf(buffer, "Loading: %i%%", preload);
        drawTextCenter(buffer, 0.0, 0.0, 3.0);
        for(int i=0;i<preload/10;i++)
        {
            setDrawPos(-5+i,-5,-8);
            drawMenuObject(i == duck ? 3 : 0, 1, 255);
        }
        setDrawPos(-5+preload/10,-5,-8);
        drawMenuObject((preload/10) == duck ? 3 : 0, 1, 28*(preload%10));
        flipBuffers();
    }
}

void resultScreen()
{
    float hit[MAX_PLAYERS];
    
    for(int p=0;p<MAX_PLAYERS;p++)
    {
        if (!player[p].playing)
            continue;
        int totalNotes = 0;
        int hitNotes = 0;
        for(TNote* n = track.difficulty[player[p].difficulty].noteList[player[p].instrument]; n; n = n->next)
        {
            totalNotes ++;
            if (n->hit[p])
                hitNotes ++;
        }
        hit[p] = float(hitNotes * 1000 / totalNotes) / 10;
    }
    
    int quit = 0;
    while(!quit)
    {
        updateInputs(true);
        int input = getInputPress(-1);
        if (input & BUTTON_MENU_SELECT)
            quit = 1;
        
        //        1234567 1234  123.1
        drawText(" Score  Streak Hit", -0.8, 0.5, 2);
        float y = 0.3f;
        for(int p=0;p<MAX_PLAYERS;p++)
        {
            if (!player[p].playing)
                continue;
            char buffer[128];
            if (player[p].quality > 0)
            {
                nprintf(buffer, 128, "%7i  %4i  %3.1f%%", player[p].score, player[p].bestStreak, hit[p]);
            }else{
                nprintf(buffer, 128, "Did not finish.");
            }
            drawText(buffer, -0.8, y, 2);
            y -= 0.15;
        }

        drawTextCenter("\201continue", 0,-0.9, 1.5);
        drawTextCenter("Guitars On Fire", 0, 0.8, 3);
        flipBuffers();
    }
}

void showErrorMessage(const char* message, const char* message2)
{
    while(1)
    {
        drawFire();
        updateInputs(true);
        
        int buttons = getInputPress(-1);
        if (buttons & BUTTON_MENU_SELECT)
        {
            return;
        }
        
        drawTextCenter(message, 0, 0, 2.5);
        drawTextCenter(message2, 0,-0.2, 2.5);

        drawTextCenter("Guitars On Fire", 0, 0.8, 3);
        flipBuffers();
    }
}

TDirEntry* randomSong()
{
    TDirList* list = getBaseDirList();
    int songCount = list->songCount();
    if (songCount == 0)
        return NULL;
    
    int num = rand() % songCount;
    return list->songNum(num);;
}

#define DIR_DISPLAY_COUNT 8
TDirEntry* selectSong(TDirEntry* prev)
{
    int option = 0;
    int menuOffset = 0;
    TDirList* list;
    list = getBaseDirList();
    if (prev)
    {
        if (prev->parent)
            list = prev->parent->getDirList();
        for(int i=0;i<list->entryCount;i++)
        {
            if (prev == list->entry[i])
                option = i;
        }
    }
    
    if (list->entryCount < 1)
    {
        showErrorMessage("No songs found...", "");
        return NULL;
    }
    
    while(1)
    {
        drawFire();
        updateInputs(true);
        
        int buttons = getInputPress(-1);
        if (buttons & BUTTON_MENU_UP)
        {
            option = (option + list->entryCount - 1) % list->entryCount;
            playCrunch();
        }
        if (buttons & BUTTON_MENU_DOWN)
        {
            option = (option + 1) % list->entryCount;
            playCrunch();
        }
        if (buttons & BUTTON_MENU)
        {
            return randomSong();
        }
        if (option < menuOffset)
        {
            menuOffset = option;
        }
        if (option >= menuOffset + DIR_DISPLAY_COUNT)
        {
            menuOffset = option - DIR_DISPLAY_COUNT + 1;
        }
        if (buttons & BUTTON_MENU_SELECT)
        {
            playIn();
            switch(list->entry[option]->type)
            {
            case DET_SongFoF:
                return list->entry[option];
            case DET_Folder:
                if (list->entry[option]->getDirList()->entryCount > 0)
                {
                    list = list->entry[option]->getDirList();
                    option = 0;
                }
                break;
            case DET_Unknown:
                break;
            }
        }
        if (buttons & BUTTON_MENU_CANCEL)
        {
            playOut();
            TDirEntry* parent = list->entry[0]->parent;
            //No parent to fall back to, so we are the top list. Drop from song select without a song.
            if (parent == NULL)
                return NULL;
            if (parent->parent)
            {
                //If the parent has a parent then use that list.
                list = parent->parent->getDirList();
            }else{
                //Else we have to use the base list.
                list = getBaseDirList();
            }
            option = 0;
            for(int i=0;i<list->entryCount;i++)
            {
                if (list->entry[i] == parent)
                    option = i;
            }
        }
        
        setDrawPos(0.0, 2.35f - (option-menuOffset) * 0.80f, -4);
	    drawQuad(4, 0.4, keyColor[0], 255);

        setDrawPos(-1.5, 1.175f - (option-menuOffset) * 0.40f, -2);
	    drawMenuObject(1, 0.4, 255);

        if (menuOffset > 0)
            drawText("More...",-0.85, 0.70, 1.0);
        if (menuOffset + DIR_DISPLAY_COUNT < list->entryCount)
            drawText("More...",-0.85,-0.95, 1.0);

	    for(int i=0;i<DIR_DISPLAY_COUNT;i++)
	    {
	        if (menuOffset+i < list->entryCount)
	        {
                if (list->entry[menuOffset+i]->type == DET_Folder)
                {
                    drawText(list->entry[menuOffset+i]->path, -0.65, 0.6f - 0.2f*i, 1.5);
                    drawText("\x90", -0.55, 0.51f - 0.2f*i, 1.5);
                }else if (list->entry[menuOffset+i]->info)
                {
                    switch(list->entry[menuOffset+i]->info->difficultyDone())
                    {
                    case 3://Expert
                        setDrawPos(-1.4, 0.88125f - (i) * 0.30f, -1.5);
                        drawMenuObject(1, 0.13, 255);
                        setDrawPos(-1.4, 0.88125f - (i) * 0.30f, -1.5, 0, 0,0,0, 180,0,0);
                        drawMenuObject(1, 0.13, 255);
                        break;
                    case 2://Hard
                        setDrawPos(-1.4, 0.88125f - (i) * 0.30f, -1.5);
                        drawMenuObject(1, 0.13, 255);
                        break;
                    case 1://Normal
                        setDrawPos(-1.4, 0.88125f - (i) * 0.30f, -1.5);
                        drawMenuObject(0, 0.13, 255);
                        break;
                    case 0://Easy
                        setDrawPos(-1.4, 0.88125f - (i) * 0.30f, -1.5);
                        drawMenuObject(3, 0.13, 255);
                        break;
                    }
                    
                    drawText(list->entry[menuOffset+i]->info->name, -0.65, 0.6f - 0.2f*i, 1.5);
                    if (list->entry[menuOffset+i]->info->artist)
                    {
                        drawText(list->entry[menuOffset+i]->info->artist, -0.55, 0.51f - 0.2f*i, 1.5);
                    }
                    if (list->entry[menuOffset+i]->info->unlockID)
                    {
                        drawTextRight(list->entry[menuOffset+i]->info->unlockID, 1.0, 0.51f - 0.2f*i, 1.5);
                    }
                }
	        }
	    }

        drawTextCenter("Guitars On Fire", 0, 0.8, 3);
        flipBuffers();
    }
}

int selectStage()
{
    int stageCount = 0;
    TStageEntry** stageList = readStageDir(&stageCount);

    if (stageCount < 1)
        return 0;

    freeTextures(TT_Stage);
    for(int i=0;i<stageCount;i++)
    {
        char buffer[1024];
        nprintf(buffer, 1024, DATA_BASE "/stage/%s/icon.png", stageList[i]->name);
        stageList[i]->icon = loadTexture(buffer, TT_Stage);
    }

    int option = 0;
    int quit = 0;
    int menuOffset = 0;
    
    while(!quit)
    {
        drawFire();
        updateInputs(true);
        
        int buttons = getInputPress(-1);
        if (buttons & BUTTON_MENU_UP)
        {
            option = (option + (stageCount + 1) - 1) % (stageCount + 1);
            playCrunch();
        }
        if (buttons & BUTTON_MENU_DOWN)
        {
            option = (option + 1) % (stageCount + 1);
            playCrunch();
        }
        if (option < menuOffset)
        {
            menuOffset = option;
        }
        if (option >= menuOffset + DIR_DISPLAY_COUNT)
        {
            menuOffset = option - DIR_DISPLAY_COUNT + 1;
        }
        if (buttons & BUTTON_MENU_SELECT)
        {
            playIn();
            if (option == 0)
            {
                loadStage(stageList[rand() % stageCount]->name);
            }else{
                loadStage(stageList[option-1]->name);
            }
            quit = 2;
        }
        if (buttons & BUTTON_MENU_CANCEL)
        {
            playOut();
            quit = 1;
        }
        
        if (quit)
            break;

        setDrawPos(0.0, 2.35f - (option-menuOffset) * 0.80f, -4);
	    drawQuad(4, 0.4, keyColor[0], 255);

        if (menuOffset > 0)
            drawText("More...",-0.85, 0.70, 1.0);
        if (menuOffset + DIR_DISPLAY_COUNT < (stageCount+1))
            drawText("More...",-0.85,-0.95, 1.0);

	    for(int i=0;i<DIR_DISPLAY_COUNT;i++)
	    {
	        if (menuOffset+i > 0 && menuOffset+i < (stageCount+1))
	        {
	            if (stageList[menuOffset+i-1]->icon > -1)
	            {
                    setDrawPos(-1.35, 1.7625f - i * 0.60f, -3);
                    drawTexturedQuad(TT_Stage, stageList[menuOffset+i-1]->icon, 0,0, 1,1, 0.6, 0.3, 255);
	            }
	        }
	    }
	    for(int i=0;i<DIR_DISPLAY_COUNT;i++)
	    {
	        if (menuOffset+i == 0)
	        {
	            drawText("Random Stage", -0.45, 0.5f - 0.2f*i, 2.5);
	        }

	        if (menuOffset+i > 0 && menuOffset+i < (stageCount+1))
	        {
	            if (stageList[menuOffset+i-1]->icon > -1)
	            {
                    drawText(stageList[menuOffset+i-1]->name, -0.25, 0.5f - 0.2f*i, 1.5);
	            }else{
                    drawText(stageList[menuOffset+i-1]->name, -0.65, 0.5f - 0.2f*i, 1.5);
	            }
	        }
	    }

        setDrawPos(-1.5, 1.175f - (option-menuOffset) * 0.40f, -2);
	    drawMenuObject(1, 0.4, 255);
	    
        int preload = currentAudioProvider->preloadDone();
        if (preload < 100)
        {
            currentAudioProvider->doPreloadStep();
        }
        /*
        char buffer[32];
        sprintf(buffer, "Loading: %i%%", preload);
        drawText(buffer, -0.8,-0.9, 1.1);
        */
	    drawTextCenter("Guitars On Fire", 0, 0.8, 3);
        flipBuffers();
    }
    
    freeStageDirContents(stageList, stageCount);
    
    return quit == 2;
}

int ConfigureKey(int keyNum, const char* name)
{
    int key = -1;
    while(key == -1)
    {
        drawFire();
        updateInputs(true);
        key = getRawKeyPress();
        
	    drawTextCenter("Press key for:", 0, 0.4, 2);
	    drawTextCenter(name, 0, 0.2, 2);
	    drawTextCenter("Guitars On Fire", 0, 0.8, 3);
        flipBuffers();
    }
    setKeyboardKey(keyNum, key);
    return 0;
}

void ConfigureKeyboard()
{
    ConfigureKey(KEYBOARD_KEY_FRET(0), "Green Fret");
    ConfigureKey(KEYBOARD_KEY_FRET(1), "Red Fret");
    ConfigureKey(KEYBOARD_KEY_FRET(2), "Yellow Fret");
    ConfigureKey(KEYBOARD_KEY_FRET(3), "Blue Fret");
    ConfigureKey(KEYBOARD_KEY_FRET(4), "Orange Fret");

    ConfigureKey(KEYBOARD_KEY_STRUM_UP, "Strum Up");
    ConfigureKey(KEYBOARD_KEY_STRUM_DOWN, "Strum Down");

    ConfigureKey(KEYBOARD_KEY_STRUM_UP_ALT, "Strum Up Alternative");
    ConfigureKey(KEYBOARD_KEY_STRUM_DOWN_ALT, "Strum Down Alternative");

    ConfigureKey(KEYBOARD_KEY_DRUM_BASS, "Drums - Bass");
    ConfigureKey(KEYBOARD_KEY_DRUM_PAD(0), "Drums - Red pad");
    ConfigureKey(KEYBOARD_KEY_DRUM_PAD(1), "Drums - Yellow cymbal");
    ConfigureKey(KEYBOARD_KEY_DRUM_PAD(2), "Drums - Blue pad");
    //ConfigureKey(KEYBOARD_KEY_DRUM_PAD(3), "Drums - Orange cymbal");
    ConfigureKey(KEYBOARD_KEY_DRUM_PAD(4), "Drums - Green pad");

    ConfigureKey(KEYBOARD_KEY_MENU, "Menu (\x80)");
}

#define SETTING_MENU_OPTION_COUNT 5
void settingsMenu()
{
    int menuPos = 0;
    int quit = 0;
    while(!quit)
    {
        drawFire();
        updateInputs(true);
        
        int buttons = getInputPress(-1);
        if (buttons & BUTTON_MENU_UP)
        {
            menuPos = (menuPos + SETTING_MENU_OPTION_COUNT - 1) % SETTING_MENU_OPTION_COUNT;
            playCrunch();
        }
        if (buttons & BUTTON_MENU_DOWN)
        {
            menuPos = (menuPos + 1) % SETTING_MENU_OPTION_COUNT;
            playCrunch();
        }
        if (menuPos < 0)
        {
            menuPos = SETTING_MENU_OPTION_COUNT - 1;
        }
        if (menuPos >= SETTING_MENU_OPTION_COUNT)
        {
            menuPos = 0;
        }
        if (buttons & BUTTON_MENU_SELECT)
        {
            switch(menuPos)
            {
            case 0:
                playIn();
                for(int i=0;i<5;i++)
                {
                    if (getInputPress(i) & BUTTON_MENU_SELECT)
                    {
                        player[i].leftyFlip = !player[i].leftyFlip;
                    }
                }
                break;
            case 1:
                playIn();
                ConfigureKeyboard();
                break;
            case 2:
                useUSBdrive = !useUSBdrive;
                break;
            case 3:
                playIn();
                for(int i=0;i<5;i++)
                {
                    if (getInputPress(i) & BUTTON_MENU_SELECT)
                    {
                        player[i].precisionMode = !player[i].precisionMode;
                    }
                }
                break;
            case 4:
                playOut();
                quit = 1;
                break;
            }
        }
        if (buttons & BUTTON_MENU_CANCEL)
        {
            playOut();
            quit = 1;
        }
        
        if (quit)
            break;

        setDrawPos(0.0, 1.75f - menuPos * 0.80f, -4);
	    drawQuad(4, 0.2, keyColor[0], 255);
	    
        setDrawPos(-1.3, 0.875f - menuPos * 0.40f, -2);
	    drawMenuObject(1, 0.4, 255);
	    
	    drawText("Lefty flip", -0.5, 0.4, 2);
	    for(int i=0;i<5;i++)
	    {
	        if (player[i].leftyFlip)
                drawText("\x91", -0.5 + 0.08 * (13 + i), 0.4, 2);
	        else
                drawText("\x92", -0.5 + 0.08 * (13 + i), 0.4, 2);
	    }
	    drawText("Configure kb keys", -0.5, 0.2, 2);
	    if (useUSBdrive)
            drawText("Use USB drive \x91", -0.5, 0.0, 2);
        else
            drawText("Use USB drive \x92", -0.5, 0.0, 2);
	    drawText("HiPrecision", -0.5,-0.2, 2);
	    for(int i=0;i<5;i++)
	    {
	        if (player[i].precisionMode)
                drawText("\x91", -0.5 + 0.08 * (13 + i),-0.2, 2);
	        else
                drawText("\x92", -0.5 + 0.08 * (13 + i),-0.2, 2);
	    }
	    drawText("Quit", -0.5,-0.4, 2);

	    drawTextCenter("Guitars On Fire", 0, 0.8, 3);
        flipBuffers();
    }
    
    saveSettings();
}

void audioSeekToStart()
{
    int min = 2000;
    for(int i=0;i<MAX_INSTRUMENT;i++)
    {
        for(int d=0;d<MAX_DIFFICULTY;d++)
        {
            if (track.difficulty[d].noteList[i])
            {
                if (min > track.difficulty[d].noteList[i]->time)
                {
                    min = track.difficulty[d].noteList[i]->time;
                }
            }
        }
    }
    currentAudioProvider->seek(int(float(min - 3000) / 1000 * 44100));
}

void resetPlayers()
{
    for(int i=0;i<MAX_PLAYERS;i++)
    {
        player[i].reset();
        if (getInputState(i) & (INPUT_HAS_GUITAR | INPUT_HAS_DRUMS))
        {
            player[i].playing = 1;
        }
    }
}

void quickPlay()
{
    perPlayerQuality = 1;
    teamGame = 0;
    
SongSelect:
    TDirEntry* song = selectSong(currentSongEntry);
    if (!song)
        return;
    if (loadSongData(song))
        return;

StageSelect:
    if (!selectStage())
        goto SongSelect;
    
    resetPlayers();

    if (!playerMenu(PLAYER_MENU_DIFFICULTY | PLAYER_MENU_INSTRUMENT))
        goto StageSelect;
    
    loadingScreen();
    
    audioSeekToStart();
    if (!playGame())
    {
        int saveHighscore = 0;
        for(int i=0;i<MAX_PLAYERS;i++)
        {
            //player finished!
            if (player[i].playing && player[i].quality > 0)
            {
                if (player[i].score > currentSongEntry->info->highscore[player[i].instrument][player[i].difficulty])
                {
                    currentSongEntry->info->highscore[player[i].instrument][player[i].difficulty] = player[i].score;
                    saveHighscore = 1;
                }
            }
            if (saveHighscore)
            {
                currentSongEntry->info->saveHighscores();
            }
        }
        
        resultScreen();
    }
}

void practicePlay()
{
    perPlayerQuality = 0;
    teamGame = 0;
    
SongSelect:
    TDirEntry* song = selectSong(currentSongEntry);
    if (!song)
        return;
    if (loadSongData(song))
        return;

StageSelect:
    if (!selectStage())
        goto SongSelect;
    
    resetPlayers();

    if (!playerMenu(PLAYER_MENU_DIFFICULTY | PLAYER_MENU_INSTRUMENT))
        goto StageSelect;
    
    loadingScreen();
    
    audioSeekToStart();
    if (!playGame())
    {
        resultScreen();
    }
}

void versusGame()
{
    perPlayerQuality = 0;
    teamGame = 1;
    
SongSelect:
    TDirEntry* song = selectSong(currentSongEntry);
    if (!song)
        return;
    if (loadSongData(song))
        return;

StageSelect:
    if (!selectStage())
        goto SongSelect;
    
    resetPlayers();

    if (!playerMenu(PLAYER_MENU_DIFFICULTY | PLAYER_MENU_INSTRUMENT | PLAYER_MENU_TEAM))
        goto StageSelect;
    
    loadingScreen();
    
    audioSeekToStart();
    if (!playGame())
    {
        resultScreen();
    }
}

void lastmanStandingGame()
{
    perPlayerQuality = 1;
    teamGame = 0;
    
StageSelect:
    if (!selectStage())
        return;
    
    resetPlayers();

    if (!playerMenu(PLAYER_MENU_ALL_OPTIONS | PLAYER_MENU_DIFFICULTY | PLAYER_MENU_INSTRUMENT))
        goto StageSelect;

    while(1)
    {
        int songOk = 0;
        while(!songOk)
        {
            songOk = 1;
            TDirEntry* song = randomSong();
            if (!song)
                return;
            if (loadSongData(song))
                return;
            for(int i=0;i<MAX_PLAYERS;i++)
            {
                if (player[i].playing && player[i].instrument > -1 && player[i].difficulty > -1)
                {
                    if (!track.difficulty[player[i].difficulty].noteList[player[i].instrument])
                    {
                        songOk = 0;
                        break;
                    }
                }
            }
        }
        
        loadingScreen();
        
        audioSeekToStart();
        if (playGame())
        {
            //Canceled game
            return;
        }
        int active = 0;
        for(int i=0;i<MAX_PLAYERS;i++)
        {
            if (player[i].playing && player[i].quality > 0)
                active++;
        }
        if (active == 1)
        {
            //We have a winner!
            return;
        }
        if (active > 0)
        {
            //Atleast 2 people where left, eliminate the rest (with 0 players left, do another round with the same players)
            for(int i=0;i<MAX_PLAYERS;i++)
            {
                if (player[i].playing && player[i].quality < 1)
                    player[i].playing = 0;
            }
        }
    }
}

const char* textLines[] = {
    "ABCDE",
    "FGHIJ",
    "KLMNO",
    "PQRST",
    "UVWXY",
    "Z_ .,",
    "abcde",
    "fghij",
    "klmno",
    "pqrst",
    "uvwxy",
    "z?!+-"};

int enterText(char* inputBuffer, int intputBufferSize, const char* description)
{
    int line = 0;
    while(1)
    {
        drawFire();
        updateInputs(1);
        
        int buttons = getInputPress(-1);
        if (buttons & BUTTON_MENU_UP)
        {
            line = (line + 12) % 13;
            playCrunch();
        }
        if (buttons & BUTTON_MENU_DOWN)
        {
            line = (line + 1) % 13;
            playCrunch();
        }
        if (line < 12)
        {
            for(int i=0;i<5;i++)
            {
                if (buttons & BUTTON_FRET(i))
                {
                    playIn();
                    char tmpBuffer[intputBufferSize];
                    nprintf(tmpBuffer, intputBufferSize, "%s%c", inputBuffer, textLines[line][i]);
                    memcpy(inputBuffer, tmpBuffer, intputBufferSize);
                }
            }

            for(int i=0;i<5;i++)
            {
                if (line < 6)
                    setDrawPos(-1.52 + 0.30*i, 0.9f - 0.4*line, -2);
                else
                    setDrawPos( 0.27 + 0.30*i, 0.9f - 0.4*(line-6), -2);
                drawQuad(0.15, 0.15, keyColor[i], 255);
            }
        }else{
            if (buttons & BUTTON_FRET(0))
            {
                playIn();
                return 1;
            }
            if (buttons & BUTTON_FRET(1))
            {
                playOut();
                if (inputBuffer[0] != 0)
                {
                    inputBuffer[strlen(inputBuffer)-1] = 0;
                }
            }
            
            setDrawPos(0.57,-1.6f, -2);
            drawQuad(0.50, 0.15, keyColor[0], 255);
            
            setDrawPos(1.37,-1.6f, -2);
            drawQuad(0.30, 0.15, keyColor[1], 255);
        }
        
        drawTextCenter(description, 0.0, 0.85, 2);
        char buffer[128];
        if (getTime() % 700 < 350)
            nprintf(buffer, 128, ":%s_", inputBuffer);
        else
            nprintf(buffer, 128, ":%s", inputBuffer);
        drawText(buffer,-0.6, 0.6, 3);
        for(int i=0;i<6;i++)
        {
            for(int j=0;j<5;j++)
            {
                nprintf(buffer, 128, "%c", textLines[i][j]);
                drawText(buffer, -0.8 + 0.15 * j, 0.4 - 0.2 * i, 2.5);

                nprintf(buffer, 128, "%c", textLines[i+6][j]);
                drawText(buffer, 0.1 + 0.15 * j, 0.4 - 0.2 * i, 2.5);
            }
        }
        drawText("Done <-", 0.1,-0.85, 2.5);
        
        flipBuffers();
    }
}

class TTournamentPlayerInfo
{
public:
    char name[8];
    
    TTournamentPlayerInfo()
    {
        name[0] = 0;
    }
};

class TTournamentRound
{
private:
    int depth;
    int fillDir;
    int winner;
    TTournamentRound* child[2];
public:
    int playerNum[2];
    
    TTournamentRound(int nDepth)
    {
        depth = nDepth;
        child[0] = NULL;
        child[1] = NULL;
        playerNum[0] = -1;
        playerNum[1] = -1;
        fillDir = 0;
        winner = -1;
    }
    ~TTournamentRound()
    {
        if (child[0])
            delete child[0];
        if (child[1])
            delete child[1];
    }
    
    void addPlayer(int num)
    {
        if (!child[0])
        {
            if (playerNum[0] == -1)
            {
                playerNum[0] = num;
                return;
            }
            if (playerNum[1] == -1)
            {
                playerNum[1] = num;
                return;
            }
            child[0] = new TTournamentRound(depth+1);
            child[0]->addPlayer(playerNum[0]);
            child[0]->addPlayer(num);
            
            playerNum[0] = -1;
            return;
        }
        if (!child[1])
        {
            child[1] = new TTournamentRound(depth+1);
            child[1]->addPlayer(playerNum[1]);
            child[1]->addPlayer(num);
            
            playerNum[1] = -1;
            return;
        }
        if (fillDir == 0)
        {
            child[0]->addPlayer(num);
            fillDir = 1;
            return;
        }
        if (fillDir == 1)
        {
            child[1]->addPlayer(num);
            fillDir = 0;
            return;
        }
    }
    
    int getHeight()
    {
        if (!child[0])
            return 1;
        if (!child[1])
            return child[0]->getHeight();
        return child[0]->getHeight() + child[1]->getHeight();
    }
    
    void draw(float x, float y, TTournamentPlayerInfo* playerList)
    {
        int h[2] = {1, 1};
        for(int i=0;i<2;i++)
        {
            if (child[i])
            {
                h[i] = child[i]->getHeight();
            }
        }
        if (child[0] && child[1])
        {
            child[0]->draw(x + 0.4, y + h[0] * 0.15, playerList);
            child[1]->draw(x + 0.4, y - h[1] * 0.15, playerList);
        }else if (child[0])
        {
            child[0]->draw(x + 0.4, y + 0.075, playerList);
        }
        if (winner == 0)
        {
            setDrawPos(x*2 + 0.28, y*2 + 0.19, -2);
            drawQuad(0.32,0.08, keyColor[0], 255);
            setDrawPos(x*2 + 0.28, y*2 - 0.06, -2);
            drawQuad(0.32,0.08, keyColor[1], 255);
        }
        if (winner == 1)
        {
            setDrawPos(x*2 + 0.28, y*2 + 0.19, -2);
            drawQuad(0.32,0.08, keyColor[1], 255);
            setDrawPos(x*2 + 0.28, y*2 - 0.06, -2);
            drawQuad(0.32,0.08, keyColor[0], 255);
        }
        if (playerNum[0] < 0)
            drawText("????", x, y+0.06, 1.5);
        else
            drawText(playerList[playerNum[0]].name, x, y+0.06, 1.5);
        drawText(" vs ", x, y, 1.5);
        if (playerNum[1] < 0)
            drawText("????", x, y-0.06, 1.5);
        else
            drawText(playerList[playerNum[1]].name, x, y-0.06, 1.5);
    }
    
    TTournamentRound* getNextRound()
    {
        if (winner > -1)
            return NULL;
        TTournamentRound* r[2] = {NULL, NULL};
        if (child[0] && child[0]->winner < 0)
            r[0] = child[0]->getNextRound();
        if (child[1] && child[1]->winner < 0)
            r[1] = child[1]->getNextRound();

        if (!r[0] && playerNum[0] < 0)
            playerNum[0] = child[0]->playerNum[child[0]->winner];
        if (!r[1] && playerNum[1] < 0)
            playerNum[1] = child[1]->playerNum[child[1]->winner];
        
        if (!r[0] && !r[1])
        {
            return this;
        }
        if (r[0] && !r[1])
            return r[0];
        if (!r[0] && r[1])
            return r[1];
        if (r[1]->depth > r[0]->depth)
            return r[1];
        return r[0];
    }
    
    void win(int player)
    {
        winner = player;
    }
};

void tournamentGame()
{
    //Check if we have enough instruments connected
    int p1num = -1;
    int p2num = -1;
    for(int i=0;i<MAX_PLAYERS;i++)
    {
        if (p1num == -1 && getInputState(i) & INPUT_HAS_ANY_CON)
            p1num = i;
        else if (p2num == -1 && getInputState(i) & INPUT_HAS_ANY_CON)
            p2num = i;
    }
    if (p1num == -1 || p2num == -1)
    {
        showErrorMessage("Tournament needs", "2 controllers");
        return;
    }
    
    //Get number of players
    static int numberOfPlayers = 2;
	while(1)
	{
        drawFire();
	    updateInputs(true);

        int buttons = getInputPress(-1);
        if (buttons & BUTTON_MENU_UP)
        {
            if (numberOfPlayers > 2)
                numberOfPlayers--;
            playCrunch();
        }
        if (buttons & BUTTON_MENU_DOWN)
        {
            numberOfPlayers++;
            playCrunch();
        }
        if (buttons & BUTTON_MENU_SELECT)
        {
            playIn();
            break;
        }
        if (buttons & BUTTON_MENU_CANCEL)
        {
            playOut();
            return;
        }

	    setDrawPos(0.0, 0.4f, -4);
	    drawQuad(4, 0.5, keyColor[0], 255);
	    
	    drawTextCenter("Number of players:", 0.0, 0.4, 2);
	    char buffer[16];
	    nprintf(buffer, 16, "%i", numberOfPlayers);
	    drawTextCenter(buffer, 0.0, 0.0, 5);

	    drawTextCenter("Guitars On Fire", 0, 0.8, 3);
	    flipBuffers();
	}
    //Enter name per player
    TTournamentPlayerInfo playerInfo[numberOfPlayers];
    for(int i=0;i<numberOfPlayers;i++)
    {
        char buffer[64];
        nprintf(buffer, 64, "Enter name for player %i", i+1);
        //enterText(playerInfo[i].name, 8, buffer);
        nprintf(playerInfo[i].name, 8, "Play%i", i+1);
    }
    //Create the rounds
    TTournamentRound finalRound(0);
    for(int i=0;i<numberOfPlayers;i++)
    {
        finalRound.addPlayer(i);
    }
    //Play the rounds
    for(TTournamentRound* nextRound = finalRound.getNextRound(); nextRound; nextRound = finalRound.getNextRound())
    {
        //Show the tournament tree, and wait for both players to be ready.
        int ready = 0;
        while(ready != 0x03)
        {
            drawFire();
            updateInputs(true);

            int buttons = getInputPress(p1num);
            if ((buttons & BUTTON_MENU_SELECT) && !(ready & 0x01))
            {
                playIn();
                ready |= 0x01;
            }
            buttons = getInputPress(p2num);
            if ((buttons & BUTTON_MENU_SELECT) && !(ready & 0x02))
            {
                playIn();
                ready |= 0x02;
            }
            
            finalRound.draw(-0.8, 0.0, playerInfo);

            char buffer[64];
            drawText("Next round is:", -0.8,-0.8, 1.8);
            nprintf(buffer, 64, "%s%c vs %s%c", playerInfo[nextRound->playerNum[0]].name, (ready & 1) ? 0x91 : 0x92, playerInfo[nextRound->playerNum[1]].name, (ready & 2) ? 0x91 : 0x92);
            drawText(buffer, -0.8,-0.87, 1.8);

            drawTextCenter("Guitars On Fire", 0, 0.8, 3);
            flipBuffers();
        }
        
        nextRound->win(rand() & 1);
    }
}

void advancedGameMenu()
{
    int menuPos = 0;
	while(1)
	{
        drawFire();
	    updateInputs(true);

        int buttons = getInputPress(-1);
        if (buttons & BUTTON_MENU_UP)
        {
            menuPos = (menuPos + 4) % 5;
            playCrunch();
        }
        if (buttons & BUTTON_MENU_DOWN)
        {
            menuPos = (menuPos + 1) % 5;
            playCrunch();
        }
        if (buttons & BUTTON_MENU_SELECT)
        {
            playIn();
            switch(menuPos)
            {
            case 0:
                versusGame();
                break;
            case 1:
                lastmanStandingGame();
                break;
            case 2:
                //tournamentGame();
                break;
            case 3:
                break;
            case 4:
                return;
            }
        }
        if (buttons & BUTTON_MENU_CANCEL)
        {
            playOut();
            return;
        }

	    setDrawPos(0.0, 1.75f - menuPos * 0.80f, -4);
	    drawQuad(4, 0.2, keyColor[0], 255);
	    
        setDrawPos(-1.0, 0.875f - menuPos * 0.40f, -2);
	    drawMenuObject(1, 0.4, 255);
	    
	    drawText("Versus", -0.4, 0.4, 2);
	    drawText("Last man standing", -0.4, 0.2, 2);
	    //drawText("Tournament", -0.4, 0.0, 2);
	    drawText("?", -0.4, 0.0, 2);
	    drawText("?", -0.4,-0.2, 2);
	    drawText("Quit", -0.4,-0.4, 2);

	    drawTextCenter("Guitars On Fire", 0, 0.8, 3);
	    flipBuffers();
	}
}

int main(int argc,char **argv)
{
    initVideo();
    initAudio();
    
    loadSettings();
    
    int menuPos = 0;
	while(1)
	{
        drawFire();
	    updateInputs(true);

        int buttons = getInputPress(-1);
        if (buttons & BUTTON_MENU_UP)
        {
            menuPos = (menuPos + 4) % 5;
            playCrunch();
        }
        if (buttons & BUTTON_MENU_DOWN)
        {
            menuPos = (menuPos + 1) % 5;
            playCrunch();
        }
        if (buttons & BUTTON_MENU_SELECT)
        {
            playIn();
            switch(menuPos)
            {
            case 0:
                quickPlay();
                break;
            case 1:
                advancedGameMenu();
                break;
            case 2:
                practicePlay();
                break;
            case 3:
                settingsMenu();
                break;
            case 4:
                exit(0);
                break;
            }
        }

	    setDrawPos(0.0, 1.75f - menuPos * 0.80f, -4);
	    drawQuad(4, 0.2, keyColor[0], 255);
	    
        setDrawPos(-1.0, 0.875f - menuPos * 0.40f, -2);
	    drawMenuObject(1, 0.4, 255);
	    
	    drawText("Quick Play", -0.4, 0.4, 2);
	    drawText("Advanced Game", -0.4, 0.2, 2);
	    drawText("Practice", -0.4, 0.0, 2);
	    drawText("Settings", -0.4,-0.2, 2);
	    drawText("Quit", -0.4,-0.4, 2);
	    
	    drawTextCenter("Guitars On Fire", 0, 0.8, 3);
	    drawIRDrums();
	    flipBuffers();
	}
}
