#ifndef MODELS_H
#define MODELS_H

#include "video.h"
/**
 * Some 3D models, created from the FretsOnFire models.
 * also contains some static color arrays.
 */

class T3DFace
{
public:
    float vertex[9];
};

class T3DModel
{
private:
    T3DFace* face;
    int faceCount;
    
public:
    T3DModel(const char* filename);
    ~T3DModel();
    
    int valid();
    void Draw(const int* color, const float* type, int alpha);
};

extern T3DModel* noteModel;
extern T3DModel* notePullModel;
extern T3DModel* noteBgModel;

extern T3DModel* defaultNoteModel;
extern T3DModel* defaultNotePullModel;
extern T3DModel* defaultNoteBgModel;

extern float noteModelType[4];

extern int keyColor[5][3];
extern int drumColor[5][3];
extern int noColor[3];
extern int grayColor[3];
extern int neckColor[2][3];

#endif//MODELS_H
