#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <math.h>

#include <gccore.h>
#include <wiiuse/wpad.h>
#include <fat.h>
#include <ogc/pad.h>
#include <wiikeyboard/keyboard.h>

#include "pngu/pngu.h"

#include "video.h"
#include "strfunc.h"
#include "models.h"

#define DEFAULT_FIFO_SIZE	(1024*1024)

GXRModeObj *rmode = NULL;
void *frameBuffer[2] = { NULL, NULL};
int fb = 0;
Mtx view; // view and perspective matrices

class TTextureSet
{
private:
    int loadTextureCount;
    GXTexObj* loadTextureList;
    void** loadTextureData;
    
public:
    TTextureSet()
    {
        loadTextureList = NULL;
        loadTextureData = NULL;
        loadTextureCount = 0;
    }
    
    int loadTexture(const char* path)
    {
        IMGCTX ctx;
        PNGUPROP imgProp;
        
        ctx = PNGU_SelectImageFromDevice(path);
        if (!ctx)
            return -1;
        
        if (PNGU_GetImageProperties(ctx, &imgProp) != PNGU_OK)
        {
            PNGU_ReleaseImageContext(ctx);
            return -1;
        }
        //Make a texture
        loadTextureCount++;
        loadTextureList = (GXTexObj*)realloc(loadTextureList, sizeof(GXTexObj) * loadTextureCount);
        loadTextureData = (void**)realloc(loadTextureData, sizeof(void*) * loadTextureCount);

        loadTextureData[loadTextureCount-1] = memalign(32, imgProp.imgWidth * imgProp.imgHeight * 4);
        GX_InitTexObj (&loadTextureList[loadTextureCount-1], loadTextureData[loadTextureCount-1], imgProp.imgWidth, imgProp.imgHeight, GX_TF_RGBA8, GX_CLAMP, GX_CLAMP, GX_FALSE);
        PNGU_DecodeTo4x4RGBA8(ctx, imgProp.imgWidth, imgProp.imgHeight, loadTextureData[loadTextureCount-1], 255);
        PNGU_ReleaseImageContext(ctx);
        DCFlushRange(loadTextureData[loadTextureCount-1], imgProp.imgWidth * imgProp.imgHeight * 4);

        return loadTextureCount - 1;
    }
    
    int bindTexture(int num)
    {
        if (num < 0 || num >= loadTextureCount)
            return 0;
        GX_LoadTexObj(&loadTextureList[num], GX_TEXMAP0);
        return 1;
    }
    
    void freeTextures()
    {
        if (loadTextureCount > 0)
        {
            for(int i=0;i<loadTextureCount;i++)
            {
                free(loadTextureData[i]);
            }
            free(loadTextureData);
            free(loadTextureList);
            loadTextureList = NULL;
            loadTextureData = NULL;
            loadTextureCount = 0;
        }
    }
};

TTextureSet textureSet[TT_MAX];

void initVideoSys()
{
	f32 yscale;
	u32 xfbHeight;
	void *gpfifo = NULL;
	GXColor background = {0, 0, 0, 0xff};
	guVector cam = {0.0F, 0.0F, 0.0F},
			up = {0.0F, 1.0F, 0.0F},
		  look = {0.0F, 0.0F, -1.0F};

    USB_Initialize();
    USBStorage_Initialize();

    //Also init other subsystems (video is done first)
	VIDEO_Init();
	rmode = VIDEO_GetPreferredMode(NULL);

	// allocate the fifo buffer
	gpfifo = memalign(32,DEFAULT_FIFO_SIZE);
	memset(gpfifo,0,DEFAULT_FIFO_SIZE);

	// allocate 2 framebuffers for double buffering
	frameBuffer[0] = SYS_AllocateFramebuffer(rmode);
	frameBuffer[1] = SYS_AllocateFramebuffer(rmode);

	// configure video
	VIDEO_Configure(rmode);
	VIDEO_SetNextFramebuffer(frameBuffer[fb]);
	VIDEO_Flush();
	VIDEO_WaitVSync();
	if(rmode->viTVMode&VI_NON_INTERLACE) VIDEO_WaitVSync();

	fb ^= 1;

	// init the flipper
	GX_Init(gpfifo,DEFAULT_FIFO_SIZE);
 
	// clears the bg to color and clears the z buffer
	GX_SetCopyClear(background, 0x00ffffff);
 
	// other gx setup
	GX_SetViewport(0,0,rmode->fbWidth,rmode->efbHeight,0,1);
	yscale = GX_GetYScaleFactor(rmode->efbHeight,rmode->xfbHeight);
	xfbHeight = GX_SetDispCopyYScale(yscale);
	GX_SetScissor(0,0,rmode->fbWidth,rmode->efbHeight);
	GX_SetDispCopySrc(0,0,rmode->fbWidth,rmode->efbHeight);
	GX_SetDispCopyDst(rmode->fbWidth,xfbHeight);
	GX_SetCopyFilter(rmode->aa,rmode->sample_pattern,GX_TRUE,rmode->vfilter);
	GX_SetFieldMode(rmode->field_rendering,((rmode->viHeight==2*rmode->xfbHeight)?GX_ENABLE:GX_DISABLE));
 
	if (rmode->aa)
        GX_SetPixelFmt(GX_PF_RGB565_Z16, GX_ZC_LINEAR);
    else
        GX_SetPixelFmt(GX_PF_RGB8_Z24, GX_ZC_LINEAR);

	GX_SetCullMode(GX_CULL_NONE);
	GX_CopyDisp(frameBuffer[fb],GX_TRUE);
	GX_SetDispCopyGamma(GX_GM_1_0);

	// setup the vertex attribute table
	// describes the data
	// args: vat location 0-7, type of data, data format, size, scale
	// so for ex. in the first call we are sending position data with
	// 3 values X,Y,Z of size F32. scale sets the number of fractional
	// bits for non float data.
	GX_ClearVtxDesc();
	GX_SetVtxDesc(GX_VA_POS, GX_DIRECT);
	GX_SetVtxDesc(GX_VA_CLR0, GX_DIRECT);
	GX_SetVtxDesc(GX_VA_TEX0, GX_DIRECT);

	GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
	GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
	GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);

	// set number of rasterized color channels
	GX_SetNumChans(1);

	//set number of textures to generate?
	GX_SetNumTexGens(1);

	// setup texture coordinate generation
	// args: texcoord slot 0-7, matrix type, source to generate texture coordinates from, matrix to use
	GX_SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY);

    GX_InvVtxCache();
	GX_InvalidateTexAll();

	// setup our camera at the origin
	// looking down the -z axis with y up
	guLookAt(view, &cam, &up, &look);
 
    GX_SetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_CLEAR);
    GX_SetAlphaUpdate(GX_TRUE);

    setDefaultFrustum();

	KEYBOARD_Init(NULL);
    fatInitDefault();
	WPAD_Init();
	WPAD_SetDataFormat(WPAD_CHAN_ALL, WPAD_FMT_BTNS_ACC_IR);
	PAD_Init();
}

void flipBuffers()
{
    static int first_frame = 1;
    
    GX_SetColorUpdate(GX_TRUE);
    GX_CopyDisp(frameBuffer[fb],GX_TRUE);

    GX_DrawDone();

    VIDEO_SetNextFramebuffer(frameBuffer[fb]);
    if(first_frame) {
        first_frame = 0;
        VIDEO_SetBlack(FALSE);
    }
    VIDEO_Flush();
    VIDEO_WaitVSync();
    fb ^= 1;
}

void setDefaultFrustum()
{
    Mtx44 perspective;
    //float w = rmode->viWidth;
    //float h = rmode->viHeight;
	//guPerspective(perspective, 90, (f32)w/h, 0.1f, 300.0f);
	guFrustum(perspective, 1, -1, -1, 1, 1.0f, 300.0f);
	GX_LoadProjectionMtx(perspective, GX_PERSPECTIVE);
}

void setFrustum(float left, float right, float bottom, float top)
{
    Mtx44 perspective;
	guFrustum(perspective, top, bottom, left, right, 1.0f, 300.0f);
	GX_LoadProjectionMtx(perspective, GX_PERSPECTIVE);
}

void setZBuffer(int enable)
{
    if (enable)
    {
        GX_SetZMode(GX_TRUE, GX_LEQUAL, GX_TRUE);
    }else{
        GX_SetZMode(GX_FALSE, GX_LEQUAL, GX_FALSE);
    }
}

void drawQuad(float width, float height, const int* color, int alpha)
{
    GX_SetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
	GX_SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORDNULL, GX_TEXMAP_NULL, GX_COLOR0A0);

    GX_Begin(GX_QUADS, GX_VTXFMT0, 4);
    GX_Position3f32(-width, -height,0); GX_Color4u8(color[0], color[1], color[2], alpha); GX_TexCoord2f32(0.0f,0.0f);
    GX_Position3f32( width, -height,0); GX_Color4u8(color[0], color[1], color[2], alpha); GX_TexCoord2f32(0.0f,0.0f);
    GX_Position3f32( width,  height,0); GX_Color4u8(color[0], color[1], color[2], alpha); GX_TexCoord2f32(0.0f,0.0f);
    GX_Position3f32(-width,  height,0); GX_Color4u8(color[0], color[1], color[2], alpha); GX_TexCoord2f32(0.0f,0.0f);
    GX_End();
}

void drawQuadFade(float width, float height, const int* color, int alpha1, int alpha2)
{
    GX_SetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
	GX_SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORDNULL, GX_TEXMAP_NULL, GX_COLOR0A0);

    GX_Begin(GX_QUADS, GX_VTXFMT0, 4);
    GX_Position3f32(-width, -height,0); GX_Color4u8(color[0], color[1], color[2], alpha2); GX_TexCoord2f32(0.0f,0.0f);
    GX_Position3f32( width, -height,0); GX_Color4u8(color[0], color[1], color[2], alpha2); GX_TexCoord2f32(0.0f,0.0f);
    GX_Position3f32( width,  height,0); GX_Color4u8(color[0], color[1], color[2], alpha1); GX_TexCoord2f32(0.0f,0.0f);
    GX_Position3f32(-width,  height,0); GX_Color4u8(color[0], color[1], color[2], alpha1); GX_TexCoord2f32(0.0f,0.0f);
    GX_End();
}

void drawTexturedQuad(ETextureType type, int texture, float x, float y, float w, float h, float sizeX, float sizeY, int alpha)
{
    GX_SetTevOp(GX_TEVSTAGE0,GX_MODULATE);
    GX_SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);

    if (!textureSet[type].bindTexture(texture))
        return;

    GX_Begin(GX_QUADS, GX_VTXFMT0, 4);
    GX_Position3f32(-sizeX, sizeY, 0);
    GX_Color4u8(255,255,255, alpha);
    GX_TexCoord2f32(x,y);
    
    GX_Position3f32( sizeX, sizeY, 0);
    GX_Color4u8(255,255,255, alpha);
    GX_TexCoord2f32(x+w,y);
    
    GX_Position3f32( sizeX,-sizeY, 0);
    GX_Color4u8(255,255,255, alpha);
    GX_TexCoord2f32(x+w,y+h);
    
    GX_Position3f32(-sizeX,-sizeY, 0);
    GX_Color4u8(255,255,255, alpha);
    GX_TexCoord2f32(x,y+h);
    GX_End();
}

void drawTexturedQuad(ETextureType type, int texture, float x, float y, float w, float h, float sizeX, float sizeY, const int* color, int alpha)
{
    GX_SetTevOp(GX_TEVSTAGE0,GX_MODULATE);
    GX_SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);

    if (!textureSet[type].bindTexture(texture))
        return;

    GX_Begin(GX_QUADS, GX_VTXFMT0, 4);
    GX_Position3f32(-sizeX, sizeY, 0);
    GX_Color4u8(color[0],color[1],color[2], alpha);
    GX_TexCoord2f32(x,y);
    
    GX_Position3f32( sizeX, sizeY, 0);
    GX_Color4u8(color[0],color[1],color[2], alpha);
    GX_TexCoord2f32(x+w,y);
    
    GX_Position3f32( sizeX,-sizeY, 0);
    GX_Color4u8(color[0],color[1],color[2], alpha);
    GX_TexCoord2f32(x+w,y+h);
    
    GX_Position3f32(-sizeX,-sizeY, 0);
    GX_Color4u8(color[0],color[1],color[2], alpha);
    GX_TexCoord2f32(x,y+h);
    GX_End();
}

void drawTexturedQuadFade(ETextureType type, int texture, float x, float y, float w, float h, float sizeX, float sizeY, int alpha1, int alpha2)
{
    GX_SetTevOp(GX_TEVSTAGE0,GX_MODULATE);
    GX_SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);

    if (!textureSet[type].bindTexture(texture))
        return;

    GX_Begin(GX_QUADS, GX_VTXFMT0, 4);
    GX_Position3f32(-sizeX, sizeY, 0);
    GX_Color4u8(255,255,255, alpha1);
    GX_TexCoord2f32(x,y);
    
    GX_Position3f32( sizeX, sizeY, 0);
    GX_Color4u8(255,255,255, alpha1);
    GX_TexCoord2f32(x+w,y);
    
    GX_Position3f32( sizeX,-sizeY, 0);
    GX_Color4u8(255,255,255, alpha2);
    GX_TexCoord2f32(x+w,y+h);
    
    GX_Position3f32(-sizeX,-sizeY, 0);
    GX_Color4u8(255,255,255, alpha2);
    GX_TexCoord2f32(x,y+h);
    GX_End();
}

guVector pitchAxis = {1,0,0};
guVector yawAxis = {0,1,0};
guVector rollAxis = {0,0,1};

void setDrawPos(float x, float y, float z)
{
    Mtx model, modelview;
    
    guMtxIdentity(model);
    guMtxTransApply(model, model, x, y, z);
    guMtxConcat(view,model,modelview);
    // load the modelview matrix into matrix memory
    GX_LoadPosMtxImm(modelview, GX_PNMTX0);
}

void setDrawPos(float x, float y, float z, float pitch, float x2, float y2, float z2, float yaw2, float pitch2, float roll2)
{
    Mtx model, modelview;
    Mtx model2, model3;
    Mtx mrx, mry,mrz;
    
    guMtxIdentity(model);
    guMtxIdentity(model2);
    guMtxRotAxisDeg(model, &pitchAxis, pitch);
    guMtxTransApply(model, model, x, y, z);
    guMtxRotAxisDeg(mrx, &yawAxis, yaw2);
    guMtxRotAxisDeg(mry, &pitchAxis, pitch2);
    guMtxRotAxisDeg(mrz, &rollAxis, roll2);
    guMtxConcat(mrx,mry,mry);
    guMtxConcat(mry,mrz,model2);
    guMtxTransApply(model2, model2, x2, y2, z2);
    guMtxConcat(model,model2,model3);
    guMtxConcat(view,model3,modelview);
    // load the modelview matrix into matrix memory
    GX_LoadPosMtxImm(modelview, GX_PNMTX0);
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
    
	GX_SetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
	GX_SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORDNULL, GX_TEXMAP_NULL, GX_COLOR0A0);

    GX_Begin(GX_TRIANGLES, GX_VTXFMT0, faceCount * 3);
    for(i=0;i<faceCount;i++)
    {
        T3DFace* f = &face[i];
#define DO_VERTEX(x) \
        fl = f->vertex[x*3+0]*type[0] + f->vertex[x*3+1]*type[1] - f->vertex[x*3+2]*type[2] + type[3]; \
        if (fl > 1.0f) fl = 1.0f; \
        GX_Position3f32(f->vertex[x*3+0], f->vertex[x*3+1], f->vertex[x*3+2]); \
        GX_Color4u8(fl * color[0], fl * color[1], fl * color[2], alpha); \
        GX_TexCoord2f32(0.0f,0.0f);
        DO_VERTEX(0);
        DO_VERTEX(1);
        DO_VERTEX(2);
#undef DO_VERTEX
    }
    GX_End();
}
