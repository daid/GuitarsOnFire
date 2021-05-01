#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>

#include "game.h"
#include "video.h"
#include "input.h"
#include "models.h"
#include "midi.h"
#include "readdir.h"
#include "strfunc.h"
#include "stage.h"

TPlayerInfo player[MAX_PLAYERS];

#define DIR_DISPLAY_COUNT 8
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
        updateInputs(true);
        
        int buttons = getInputPress(-1);
        if (buttons & BUTTON_STRUM_UP)
        {
            option = (option + (stageCount) - 1) % (stageCount);
        }
        if (buttons & BUTTON_STRUM_DOWN)
        {
            option = (option + 1) % (stageCount);
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
            loadStage(stageList[option]->name);
            quit = 2;
        }
        if ((buttons & BUTTON_MENU_CANCEL) || (buttons & BUTTON_MENU))
        {
            quit = 1;
        }
        
        if (quit)
            break;

        setDrawPos(0.0, 2.35f - (option-menuOffset) * 0.80f, -4);
	    drawQuad(4, 0.4, keyColor[0], 255);

        if (menuOffset > 0)
            drawText("More...",-0.85, 0.70, 1.0);
        if (menuOffset + DIR_DISPLAY_COUNT < (stageCount))
            drawText("More...",-0.85,-0.95, 1.0);

	    for(int i=0;i<DIR_DISPLAY_COUNT;i++)
	    {
	        if (menuOffset+i < (stageCount))
	        {
	            if (stageList[menuOffset+i]->icon > -1)
	            {
                    setDrawPos(-1.35, 1.7625f - i * 0.60f, -3);
                    drawTexturedQuad(TT_Stage, stageList[menuOffset+i]->icon, 0,0, 1,1, 0.6, 0.3, 255);
	            }
	        }
	    }
	    for(int i=0;i<DIR_DISPLAY_COUNT;i++)
	    {
	        if (menuOffset+i < (stageCount))
	        {
	            if (stageList[menuOffset+i]->icon > -1)
	            {
                    drawText(stageList[menuOffset+i]->name, -0.25, 0.5f - 0.2f*i, 1.5);
	            }else{
                    drawText(stageList[menuOffset+i]->name, -0.65, 0.5f - 0.2f*i, 1.5);
	            }
	        }
	    }

        setDrawPos(-1.5, 1.175f - (option-menuOffset) * 0.40f, -2);
	    drawMenuObject(1, 0.4, 255);
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

int main(int argc,char **argv)
{
    initVideo();
    
    while(1)
    {
        if (!selectStage())
            return 0;
        
        player[0].playing = 1;
        
        while(1)
        {
            updateInputs(true);
            if (getInputPress(-1) & BUTTON_MENU)
                break;

            drawStage();
            flipBuffers();
        }
    }
}
