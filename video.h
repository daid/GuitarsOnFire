#ifndef VIDEO_H
#define VIDEO_H

void initVideo();//General video initializer
void drawText(const char* Text, float X, float Y, float Size);
void drawTextCenter(const char* Text, float X, float Y, float Size);
void drawTextRight(const char* Text, float X, float Y, float Size);
void drawTextRotated(const char* Text, float X, float Y, float Size, float roll);

void initVideoSys();//Video initializer per system type (PC/Wii)

void setDefaultFrustum();
void setFrustum(float left, float right, float bottom, float top);

void setZBuffer(int enable);

void setDrawPos(float x, float y, float z);
void setDrawPos(float x, float y, float z, float pitch, float x2, float y2, float z2, float yaw2, float pitch2, float roll2);

void drawQuad(float width, float height, const int* color, int alpha);
void drawQuadFade(float width, float height, const int* color, int alpha1, int alpha2);
void drawMenuObject(int object, float size, int alpha);
void drawMenuObject(int object, float size, const int* color, int alpha);

void drawIRDrums();
void flipBuffers();

enum ETextureType
{
    TT_System,
    TT_Stage,
    TT_Neck,
    
    TT_MAX
};

int loadTexture(const char* path, ETextureType type);
void freeTextures(ETextureType type);
void drawTexturedQuad(ETextureType type, int texture, float x, float y, float w, float h, float sizeX, float sizeY, int alpha);
void drawTexturedQuad(ETextureType type, int texture, float x, float y, float w, float h, float sizeX, float sizeY, const int* color, int alpha);
void drawTexturedQuadFade(ETextureType type, int texture, float x, float y, float w, float h, float sizeX, float sizeY, int alpha1, int alpha2);

#endif//VIDEO_H
