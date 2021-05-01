#include <stdio.h>
#include <stdlib.h>

#include "models.h"
#include "strfunc.h"

T3DModel* defaultNoteModel = NULL;
T3DModel* defaultNotePullModel = NULL;
T3DModel* defaultNoteBgModel = NULL;

T3DModel* noteModel = NULL;
T3DModel* notePullModel = NULL;
T3DModel* noteBgModel = NULL;

float noteModelType[4] = {0.2, 7.0, 0.4, 0.5};

int keyColor[5][3] = {{25,255,25},{255,25,25},{255,255,25},{64,64,255},{255,140,25}};
int drumColor[5][3] = {{255,25,25},{255,255,25},{64,64,255},{25,255,25},{255,255,255}};
int noColor[3] = {0,0,0};
int grayColor[3] = {127,127,127};
int neckColor[2][3] = {{25,25,25},{100,50,25}};

#define MODEL_SCALE 0.8f
T3DModel::T3DModel(const char* filename)
{
    faceCount = 0;
    face = NULL;
    
    FILE* f = fopen(filename, "r");
    if (!f)
        return;
    char buffer[1024];
    float x1,y1,z1, x2,y2,z2, x3,y3,z3, x4,y4,z4;
    while(fgets(buffer, 1024, f))
    {
        int ret = sscanf(buffer, "%f %f %f %f %f %f %f %f %f %f %f %f", &x1,&y1,&z1, &x2,&y2,&z2, &x3,&y3,&z3, &x4,&y4,&z4);
        if (ret == 12)
        {
            face = (T3DFace*)realloc(face, sizeof(T3DFace) * (faceCount+1));
            face[faceCount].vertex[0] = x1*MODEL_SCALE;
            face[faceCount].vertex[1] = y1*MODEL_SCALE;
            face[faceCount].vertex[2] = z1*MODEL_SCALE;
            face[faceCount].vertex[3] = x2*MODEL_SCALE;
            face[faceCount].vertex[4] = y2*MODEL_SCALE;
            face[faceCount].vertex[5] = z2*MODEL_SCALE;
            face[faceCount].vertex[6] = x3*MODEL_SCALE;
            face[faceCount].vertex[7] = y3*MODEL_SCALE;
            face[faceCount].vertex[8] = z3*MODEL_SCALE;
            faceCount++;
            
            face = (T3DFace*)realloc(face, sizeof(T3DFace) * (faceCount+1));
            face[faceCount].vertex[0] = x1*MODEL_SCALE;
            face[faceCount].vertex[1] = y1*MODEL_SCALE;
            face[faceCount].vertex[2] = z1*MODEL_SCALE;
            face[faceCount].vertex[3] = x3*MODEL_SCALE;
            face[faceCount].vertex[4] = y3*MODEL_SCALE;
            face[faceCount].vertex[5] = z3*MODEL_SCALE;
            face[faceCount].vertex[6] = x4*MODEL_SCALE;
            face[faceCount].vertex[7] = y4*MODEL_SCALE;
            face[faceCount].vertex[8] = z4*MODEL_SCALE;
            faceCount++;
        }else if (ret > 8)
        {
            face = (T3DFace*)realloc(face, sizeof(T3DFace) * (faceCount+1));
            face[faceCount].vertex[0] = x1*MODEL_SCALE;
            face[faceCount].vertex[1] = y1*MODEL_SCALE;
            face[faceCount].vertex[2] = z1*MODEL_SCALE;
            face[faceCount].vertex[3] = x2*MODEL_SCALE;
            face[faceCount].vertex[4] = y2*MODEL_SCALE;
            face[faceCount].vertex[5] = z2*MODEL_SCALE;
            face[faceCount].vertex[6] = x3*MODEL_SCALE;
            face[faceCount].vertex[7] = y3*MODEL_SCALE;
            face[faceCount].vertex[8] = z3*MODEL_SCALE;
            faceCount++;
        }
    }
    fclose(f);
}

T3DModel::~T3DModel()
{
    free(face);
}

int T3DModel::valid()
{
    return faceCount > 0;
}
