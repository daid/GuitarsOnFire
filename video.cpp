#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "midi.h"
#include "readdir.h"
#include "video.h"
#include "models.h"
#include "input.h"

int fontTexture;
int objectsTexture;

void drawTextCenter(const char* Text, float X, float Y, float Size)
{
    drawText(Text, X - strlen(Text) * Size * 0.02, Y, Size);
}
void drawTextRight(const char* Text, float X, float Y, float Size)
{
    drawText(Text, X - strlen(Text) * Size * 0.04, Y, Size);
}

void drawText(const char* Text, float X, float Y, float Size)
{
    Size *= 0.05;
    
	X-=Size*0.2f - Size * 0.5f;
	Y-=Size*0.5f - Size * 1.0f;

	for (;*Text != 0;Text++)
	{
		float CX = (float)(((unsigned char)(*Text) - 32) % 16) / 16;
		float CY = (float)(((unsigned char)(*Text) - 32) / 16) / 8;
		
		setDrawPos(X, Y, -1);
		drawTexturedQuad(TT_System, fontTexture, CX, CY, 0.0625, 0.125, Size/2, Size, 255);

		X+=Size*0.8f;
	}
}

void drawTextRotated(const char* Text, float X, float Y, float Size, float roll)
{
    Size *= 0.05;
    
    float XD = cosf(roll / 180 * 3.14) * Size*0.8f;
    float YD = sinf(roll / 180 * 3.14) * Size*0.8f;
    
    float X2 = strlen(Text) * XD * -0.4;
    float Y2 = strlen(Text) * YD * -0.4;

	for (;*Text != 0;Text++)
	{
		float CX = (float)(((unsigned char)(*Text) - 32) % 16) / 16;
		float CY = (float)(((unsigned char)(*Text) - 32) / 16) / 8;
		
        setDrawPos(X, Y, -1, 0, X2, Y2, 0, 0, 0, roll);
		drawTexturedQuad(TT_System, fontTexture, CX, CY, 0.0625, 0.125, Size/2, Size, 255);

		X2+=XD;
		Y2+=YD;
	}
}

void drawMenuObject(int object, float size, int alpha)
{
    float x = float(object % 2) * 0.5f;
    float y = float(object / 2) * 0.5f;
    
    drawTexturedQuad(TT_System, objectsTexture, x, y, 0.5, 0.5, size, size, alpha);
}

void drawMenuObject(int object, float size, const int* color, int alpha)
{
    float x = float(object % 2) * 0.5f;
    float y = float(object / 2) * 0.5f;
    
    drawTexturedQuad(TT_System, objectsTexture, x, y, 0.5, 0.5, size, size, color, alpha);
}

void initVideo()
{
    initVideoSys();
    fontTexture = loadTexture(DATA_BASE "/gfx/font.png", TT_System);
    
    drawTextCenter("Guitars On Fire", 0, 0.8, 3);
    drawTextCenter("Loading...", 0, 0, 3);
    flipBuffers();
    drawTextCenter("Guitars On Fire", 0, 0.8, 3);
    drawTextCenter("Loading...", 0, 0, 3);
    flipBuffers();
    
    objectsTexture = loadTexture(DATA_BASE "/gfx/menu_objects.png", TT_System);
    
    defaultNoteModel = new T3DModel(DATA_BASE "/gfx/note.raw");
    defaultNotePullModel = new T3DModel(DATA_BASE "/gfx/note_pull.raw");
    defaultNoteBgModel = new T3DModel(DATA_BASE "/gfx/note_bg.raw");
}

void drawIRDrums()
{
#ifdef USE_IR_DRUMS
    struct ir_drums drums;
    getIrDrums(&drums);

    setDrawPos(-0.75, -0.8, -1);
    drawQuad(0.25, 0.2, drumColor[0], 50);
    setDrawPos(-0.25, -0.8, -1);
    drawQuad(0.25, 0.2, drumColor[1], 50);
    setDrawPos(0.25, -0.8, -1);
    drawQuad(0.25, 0.2, drumColor[2], 50);
    setDrawPos(0.75, -0.8, -1);
    drawQuad(0.25, 0.2, drumColor[3], 50);
    
    setZBuffer(0);
    for(int i=0;i<drums.num;i++)
    {
        if (drums.y[i] > 0.8)
        {
            int num = int(drums.x[i] * 4);
            if (num < 0 || num > 3)
                continue;
            setDrawPos(-0.75 + 0.5 * num, -0.8, -1);
            drawQuad(0.25, 0.2, drumColor[num], 100);
        }
        
        setDrawPos(drums.x[i] * 2 - 1, 1 - drums.y[i] * 2, -1);
        drawMenuObject(2, 0.1, 100);
    }
    setZBuffer(1);
#endif
}
