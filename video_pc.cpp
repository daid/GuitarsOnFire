#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <math.h>

#include <SDL/SDL.h>
#ifdef WIN32
#include <windows.h>
#include <SDL/SDL_syswm.h>
#endif
#include <GL/gl.h>

#include "pngu/pngu.h"

#include "system.h"
#include "video.h"
#include "dir.h"
#include "strfunc.h"
#include "models.h"

#ifndef GL_MULTISAMPLE_ARB
//Needed for AntiAlias, not used on the Wii
#define GL_MULTISAMPLE_ARB 0x809D
#endif
#ifndef MAX_SAMPLES_EXT
//Needed for AntiAlias, not used on the Wii
#define MAX_SAMPLES_EXT 0x8D57
#endif
#ifndef GL_CLAMP_TO_EDGE
//Needed to mimic the Wii texture mode as close as possible (GL_CLAMP generates slightly transparent borders)
#define GL_CLAMP_TO_EDGE 0x812F
#endif

int ScreenWidth;
int ScreenHeight;

class TTextureSet
{
private:
    GLuint* loadTextureList;
    int loadTextureCount;
    
public:
    TTextureSet()
    {
        loadTextureList = NULL;
        loadTextureCount = 0;
    }
    
    int loadTexture(const char* path)
    {
        IMGCTX ctx;
        PNGUPROP imgProp;
        void* texture_data;
        
        ctx = PNGU_SelectImageFromDevice(path);
        if (!ctx)
            return -1;
        
        if (PNGU_GetImageProperties(ctx, &imgProp) != PNGU_OK)
        {
            PNGU_ReleaseImageContext(ctx);
            return -1;
        }
        texture_data = malloc(imgProp.imgWidth * imgProp.imgHeight * 4);
        PNGU_DecodeToRGBA8(ctx, imgProp.imgWidth, imgProp.imgHeight, texture_data, 0, 255);
        PNGU_ReleaseImageContext(ctx);

        //Make a texture
        loadTextureCount++;
        loadTextureList = (GLuint*)realloc(loadTextureList, sizeof(GLuint) * loadTextureCount);
        glGenTextures(1, &loadTextureList[loadTextureCount-1]);
        glBindTexture(GL_TEXTURE_2D, loadTextureList[loadTextureCount-1]);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);//GL_CLAMP / GL_REPEAT
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);//GL_CLAMP / GL_REPEAT
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);//GL_NEAREST / GL_LINEAR
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); //GL_NEAREST / GL_LINEAR

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, imgProp.imgWidth, imgProp.imgHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, texture_data);
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

        free(texture_data);

        return loadTextureCount - 1;
    }
    
    int bindTexture(int num)
    {
        if (num < 0 || num >= loadTextureCount)
            return 0;
        glBindTexture(GL_TEXTURE_2D, loadTextureList[num]);
        return 1;
    }
    
    void freeTextures()
    {
        if (loadTextureCount > 0)
        {
            for(int i=0;i<loadTextureCount;i++)
            {
                glDeleteTextures(1, &loadTextureList[i]);
            }
            free(loadTextureList);
            loadTextureList = NULL;
            loadTextureCount = 0;
        }
    }
};

TTextureSet textureSet[TT_MAX];

void initVideoSys()
{
    const SDL_VideoInfo* info = NULL;
	int bpp = 0;
	int flags = 0;

	if ( SDL_Init( SDL_INIT_VIDEO | SDL_INIT_JOYSTICK | SDL_INIT_AUDIO | SDL_INIT_NOPARACHUTE ) < 0 )
	//if ( SDL_Init( SDL_INIT_VIDEO | SDL_INIT_JOYSTICK | SDL_INIT_AUDIO ) < 0 )
	{
		fprintf( stderr, "Video initialization failed: %s\n", SDL_GetError() );
		exit(-1);
	}
	atexit(SDL_Quit);

    info = SDL_GetVideoInfo();

    if ( !info )
    {
        fprintf( stderr, "Video query failed: %s\n", SDL_GetError() );
        exit(-1);
    }
#ifdef USE_FULLSCREEN
    ScreenWidth = info->current_w;
    ScreenHeight = info->current_h;
#endif

    bpp = info->vfmt->BitsPerPixel;

	SDL_GL_SetAttribute( SDL_GL_RED_SIZE, 8 );
	SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, 8 );
	SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, 8 );
	SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 16 );
	SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );
	/* Try to set AA. */
	//SDL_GL_SetAttribute( SDL_GL_MULTISAMPLEBUFFERS, 1);
    //SDL_GL_SetAttribute( SDL_GL_MULTISAMPLESAMPLES, 8);

    flags = SDL_OPENGL | SDL_GL_DOUBLEBUFFER | SDL_HWPALETTE | SDL_HWSURFACE;
#ifdef USE_FULLSCREEN
    flags |= SDL_FULLSCREEN;
#endif

    const SDL_VideoInfo* videoInfo = SDL_GetVideoInfo();
    if (videoInfo)
    {
#ifdef USE_FULLSCREEN
        ScreenWidth = videoInfo->current_w;
        ScreenHeight = videoInfo->current_h;
#else
        ScreenHeight = 576;
        ScreenWidth = ScreenHeight * 4 / 3;
        //ScreenWidth = ScreenHeight * 16 / 9;
#endif
    }

    if ( SDL_SetVideoMode( ScreenWidth, ScreenHeight, bpp, flags ) == 0 )
    {
        //Video can fail because of AA on linux (windows just ignores if no AA is there), so retry without.
        SDL_GL_SetAttribute( SDL_GL_MULTISAMPLEBUFFERS, 0);
        SDL_GL_SetAttribute( SDL_GL_MULTISAMPLESAMPLES, 0);

        if ( SDL_SetVideoMode( ScreenWidth, ScreenHeight, bpp, flags ) == 0 )
        {
            fprintf( stderr, "Video mode set failed: %s\n", SDL_GetError() );
            exit(-1);
        }
    }

	SDL_ShowCursor(SDL_DISABLE);

    // Enable AA
	glEnable(GL_MULTISAMPLE_ARB);

	/* Our shading model--Gouraud (smooth). */
	glShadeModel( GL_SMOOTH );

	/* Culling. */
	glCullFace( GL_BACK );
	glFrontFace( GL_CCW );
	glDisable( GL_CULL_FACE );
	glEnable( GL_DEPTH_TEST );
	glDepthFunc(GL_LEQUAL);

	//Blending
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	glClearColor( 0, 0, 0, 0 );

    glViewport(0, 0, ScreenWidth, ScreenHeight);
    
    setDefaultFrustum();
}

void initVideoStage2()
{
}

void flipBuffers()
{
    SDL_GL_SwapBuffers();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void setDefaultFrustum()
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glFrustum(-1, 1, -1, 1, 1, 10240);
    /* We don't want to modify the projection matrix. */
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void setFrustum(float left, float right, float bottom, float top)
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glFrustum(left, right, bottom, top, 1.0f, 10240);
    /* We don't want to modify the projection matrix. */
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void drawQuad(float width, float height, const int* color, int alpha)
{
    glDisable(GL_TEXTURE_2D);

    glColor4ub(color[0], color[1], color[2], alpha);
    glBegin(GL_QUADS);
    glVertex3f(-width, -height,0);
    glVertex3f( width, -height,0);
    glVertex3f( width,  height,0);
    glVertex3f(-width,  height,0);
    glEnd();
}

void drawQuadFade(float width, float height, const int* color, int alpha1, int alpha2)
{
    glDisable(GL_TEXTURE_2D);

    glBegin(GL_QUADS);
    glColor4ub(color[0], color[1], color[2], alpha2);
    glVertex3f(-width, -height,0);
    glColor4ub(color[0], color[1], color[2], alpha2);
    glVertex3f( width, -height,0);
    glColor4ub(color[0], color[1], color[2], alpha1);
    glVertex3f( width,  height,0);
    glColor4ub(color[0], color[1], color[2], alpha1);
    glVertex3f(-width,  height,0);
    glEnd();
}

void drawTexturedQuad(ETextureType type, int texture, float x, float y, float w, float h, float sizeX, float sizeY, int alpha)
{
    glEnable(GL_TEXTURE_2D);
    if (!textureSet[type].bindTexture(texture))
        return;

    glColor4ub(255,255,255,alpha);
    glBegin(GL_QUADS);
    glTexCoord2f(x,y); glVertex3f(-sizeX, sizeY, 0);
    glTexCoord2f(x+w,y); glVertex3f( sizeX, sizeY, 0);
    glTexCoord2f(x+w,y+h); glVertex3f( sizeX,-sizeY, 0);
    glTexCoord2f(x,y+h); glVertex3f(-sizeX,-sizeY, 0);
    glEnd();
}

void drawTexturedQuad(ETextureType type, int texture, float x, float y, float w, float h, float sizeX, float sizeY, const int* color, int alpha)
{
    glEnable(GL_TEXTURE_2D);
    if (!textureSet[type].bindTexture(texture))
        return;

    glColor4ub(color[0],color[1],color[2],alpha);
    glBegin(GL_QUADS);
    glTexCoord2f(x,y); glVertex3f(-sizeX, sizeY, 0);
    glTexCoord2f(x+w,y); glVertex3f( sizeX, sizeY, 0);
    glTexCoord2f(x+w,y+h); glVertex3f( sizeX,-sizeY, 0);
    glTexCoord2f(x,y+h); glVertex3f(-sizeX,-sizeY, 0);
    glEnd();
}

void drawTexturedQuadFade(ETextureType type, int texture, float x, float y, float w, float h, float sizeX, float sizeY, int alpha1, int alpha2)
{
    glEnable(GL_TEXTURE_2D);
    if (!textureSet[type].bindTexture(texture))
        return;

    glBegin(GL_QUADS);
    glColor4ub(255,255,255,alpha1);
    glTexCoord2f(x,y); glVertex3f(-sizeX, sizeY, 0);
    glColor4ub(255,255,255,alpha1);
    glTexCoord2f(x+w,y); glVertex3f( sizeX, sizeY, 0);
    glColor4ub(255,255,255,alpha2);
    glTexCoord2f(x+w,y+h); glVertex3f( sizeX,-sizeY, 0);
    glColor4ub(255,255,255,alpha2);
    glTexCoord2f(x,y+h); glVertex3f(-sizeX,-sizeY, 0);
    glEnd();
}

void setDrawPos(float x, float y, float z)
{
    glLoadIdentity();
    glTranslatef(x,y,z);
}

void setDrawPos(float x, float y, float z, float pitch, float x2, float y2, float z2, float yaw2, float pitch2, float roll2)
{
    glLoadIdentity();
    glTranslatef(x,y,z);
    glRotatef(pitch, 1,0,0);
    glTranslatef(x2,y2,z2);
    glRotatef(yaw2, 0,1,0);
    glRotatef(pitch2, 1,0,0);
    glRotatef(roll2, 0,0,1);
}

void setZBuffer(int enable)
{
    if (enable)
    {
        glEnable(GL_DEPTH_TEST);
        glDepthMask(GL_TRUE);
    }else{
        glDisable(GL_DEPTH_TEST);
        glDepthMask(GL_FALSE);
    }
}

int loadTexture(const char* path, ETextureType type)
{
    return textureSet[type].loadTexture(path);
}

void freeTextures(ETextureType type)
{
    textureSet[type].freeTextures();
}

void T3DModel::Draw(const int* color, const float* type, int alpha)
{
    int i;
    float fl;
    
    glDisable(GL_TEXTURE_2D);

    glColor3ub(255,255,255);
    glBegin(GL_TRIANGLES);
    for(i=0;i<faceCount;i++)
    {
        T3DFace* f = &face[i];
#define DO_VERTEX(x) \
        fl = f->vertex[x*3+0]*type[0] + f->vertex[x*3+1]*type[1] - f->vertex[x*3+2]*type[2] + type[3]; \
        if (fl > 1.0f) fl = 1.0f; \
        glColor4ub((GLubyte)(fl * color[0]), (GLubyte)(fl * color[1]), (GLubyte)(fl * color[2]), alpha); \
        glVertex3f(f->vertex[x*3+0], f->vertex[x*3+1], f->vertex[x*3+2]);
        DO_VERTEX(0);
        DO_VERTEX(1);
        DO_VERTEX(2);
#undef DO_VERTEX
    }
    glEnd();
}
