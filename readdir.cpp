#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>

#include "strfunc.h"

#include "midi.h"
#include "readdir.h"
#include "settings.h"

#ifdef GEKKO
TDirBase* dirBaseList[] = {
    new TDirBase("sd:/guitarfun/songs", 0),
    new TDirBase("sd:/guitarsonfire/songs", 0),
    new TDirBase("sd:/apps/guitarfun/songs", 0),
    new TDirBase("sd:/apps/guitarsonfire/songs", 0),
    new TDirBase("usb:/songs", 1),
    new TDirBase("usb:/guitarfun/songs", 1),
    new TDirBase("usb:/guitarsonfire/songs", 1),
    new TDirBase("usb:/apps/guitarfun/songs", 1),
    new TDirBase("usb:/apps/guitarsonfire/songs", 1),
    NULL};
#else
TDirBase* dirBaseList[] = {
    new TDirBase("C:/Programming/Wii/FretsOnFire/songs", 1),
    new TDirBase("C:/Programming/Wii/GuitarsOnFire/songs", 0),
    NULL};
#endif
TDirList* baseDirList = NULL;
TDirList* baseDirListUSB = NULL;
TDirEntry* currentSongEntry = NULL;

TSongInfo::TSongInfo(const char* path)
{
    name = NULL;
    artist = NULL;
    unlockID = NULL;
    delay = 0;
    memset(highscore, 0, sizeof(highscore));

    struct stat statInfo;
    if (stat(path, &statInfo) < 0)
    {
        return;
    }
    if (S_ISDIR(statInfo.st_mode))
    {
        char buffer[1024];
        nprintf(buffer, 1024, "%s/song.ini", path);
        if (stat(buffer, &statInfo) == 0)
        {
            char* nPath = strrchr(path, '/');
            if (nPath)
            {
                nPath++;
                name = (char*)malloc(strlen(nPath)+1);
                strcpy(name, nPath);
            }else{
                name = (char*)malloc(strlen(path)+1);
                strcpy(name, path);
            }
            
            FILE* f = fopen(buffer, "rt");
            while(fgets(buffer, 1024, f))
            {
                char* key = buffer;
                char* value = strchr(buffer, '=');
                if (!value)
                    continue;
                *value = 0;
                value++;
                value = trim(value);
                key = trim(key);
                
                if (stricmp(key, "name") == 0)
                {
                    if (name)
                    {
                        free(name);
                    }
                    name = (char*)malloc(strlen(value)+1);
                    strcpy(name, value);
                }
                if (stricmp(key, "artist") == 0)
                {
                    if (artist)
                    {
                        free(artist);
                    }
                    artist = (char*)malloc(strlen(value)+1);
                    strcpy(artist, value);
                }
                if (stricmp(key, "unlock_id") == 0)
                {
                    if (unlockID)
                    {
                        free(unlockID);
                    }
                    unlockID = (char*)malloc(strlen(value)+1);
                    strcpy(unlockID, value);
                }
                if (stricmp(key, "delay") == 0)
                {
                    delay = atoi(value);
                }
            }
            fclose(f);

            highscoreFilename(buffer, 1024);
            f = fopen(buffer, "rb");
            if (f)
            {
                int num;
                fread(&num, sizeof(int), 1, f);
                switch(num)
                {
                case 1:
                    //Version 1 highscore list
                    for(int i=0;i<MAX_INSTRUMENT;i++)
                    {
                        for(int d=0;d<MAX_DIFFICULTY;d++)
                        {
                            fread(&num, sizeof(int), 1, f);
                            highscore[i][d] = num;
                        }
                    }
                }
                fclose(f);
            }
        }
    }
}

TSongInfo::~TSongInfo()
{
    if (name)
        free(name);
    if (artist)
        free(artist);
    if (unlockID)
        free(unlockID);
}

int TSongInfo::difficultyDone()
{
    for(int d=MAX_DIFFICULTY-1;d >= 0;d--)
    {
        for(int i=0;i<MAX_INSTRUMENT;i++)
        {
            if (highscore[i][d] > 0)
                return d;
        }
    }
    return -1;
}

void TSongInfo::highscoreFilename(char* buffer, int bufferSize)
{
    char filenameBuffer[bufferSize];
    if (artist && name && unlockID)
        nprintf(filenameBuffer, bufferSize, "highscore_%s_%s_%s.dat", artist, name, unlockID);
    else if (artist && name)
        nprintf(filenameBuffer, bufferSize, "highscore_%s_%s.dat", artist, name);
    else
        nprintf(filenameBuffer, bufferSize, "highscore_%s.dat", name);
    for(char* c=filenameBuffer;*c;c++)
    {
        if (*c < 33) *c = '_';
        if (*c > 126) *c = '_';
        if (*c == '/') *c = '_';
        if (*c == '\\') *c = '_';
        if (*c == '/') *c = '_';
        if (*c == ':') *c = '_';
        if (*c == '*') *c = '_';
        if (*c == '?') *c = '_';
        if (*c == '"') *c = '_';
        if (*c == '<') *c = '_';
        if (*c == '>') *c = '_';
        if (*c == '|') *c = '_';
    }
    nprintf(buffer, bufferSize, DATA_BASE "/%s", filenameBuffer);
}

int TSongInfo::saveHighscores()
{
    char buffer[1024];
    highscoreFilename(buffer, 1024);
    FILE* f = fopen(buffer, "w");
    if (f)
    {
        int num;
        num = 1; //Version number
        fwrite(&num, sizeof(int), 1, f);
        for(int i=0;i<MAX_INSTRUMENT;i++)
        {
            for(int d=0;d<MAX_DIFFICULTY;d++)
            {
                num = highscore[i][d];
                fwrite(&num, sizeof(int), 1, f);
            }
        }
        fclose(f);
        return 0;
    }
    return 1;
}

TDirBase::TDirBase(const char* nPath, int nUsb)
{
    usb = nUsb;
    path = nPath;
}

TDirEntry::TDirEntry(TDirBase* nBase, TDirEntry* nParent, const char* nPath)
{
    base = nBase;
    parent = nParent;
    path = (char*)malloc(strlen(nPath)+1);
    strcpy(path, nPath);
    type = DET_Unknown;
    info = NULL;
    list = NULL;
    
    if (strcmp(path, "tutorial") == 0)
        return;

    char buffer[1024];
    getFullPath(buffer, 1024);
    struct stat statInfo;
    if (stat(buffer, &statInfo) < 0)
    {
        return;
    }
    if (S_ISDIR(statInfo.st_mode))
    {
        char fileBuffer[1024];
        nprintf(fileBuffer, 1024, "%s/song.ini", buffer);
        if (stat(fileBuffer, &statInfo) == 0)
        {
            type = DET_SongFoF;
            info = new TSongInfo(buffer);
        }else{
            type = DET_Folder;
        }
    }
}
TDirEntry::~TDirEntry()
{
    free(path);
    if (info)
        delete info;
}
int TDirEntry::getFullPath(char* buffer, int len)
{
    int i = 0;
    if (base)
        i = nprintf(buffer, len, "%s", base->path);
    else
        i = parent->getFullPath(buffer, len);
    i += nprintf(buffer + i, len - i, "/%s", path);
    return i;
}

void readAddDir(TDirBase* base, TDirEntry* parent, const char* path, TDirList* list)
{
    DIR* dir = opendir(path);
    if (!dir)
        return;
    
    struct dirent* file;
    while((file = readdir(dir)))
    {
        if (file->d_name[0] == '.')
            continue;
        TDirEntry* nEntry = new TDirEntry(base, parent, file->d_name);
        if (nEntry->type == DET_Unknown)
        {
            delete nEntry;
        }else{
            list->addEntry(nEntry);
        }
    }
    closedir(dir);
}

TDirList* TDirEntry::getDirList()
{
    if (list)
        return list;
    list = new TDirList();
    char buffer[1024];
    getFullPath(buffer, 1024);
    readAddDir(NULL, this, buffer, list);
    list->sort();
    return list;
}

int direntry_cmp(TDirEntry* q, TDirEntry* y)
{
    if (q->info && y->info && q->info->unlockID && y->info->unlockID)
    {
        if (strcmp(q->info->unlockID, y->info->unlockID) == 0)
        {
            return strcmp(q->path, y->path);
        }
        return strcmp(q->info->unlockID, y->info->unlockID);
    }
    if (q->info && q->info->unlockID)
        return 0;
    if (y->info && y->info->unlockID)
        return 1;
    return strcmp(q->path, y->path);
}

TDirList::TDirList()
{
    entry = NULL;
    entryCount = 0;
}

TDirList::~TDirList()
{
    for(int i=0;i<entryCount;i++)
    {
        delete entry[i];
    }
    free(entry);
}

void TDirList::addEntry(TDirEntry* newEntry)
{
    entry = (TDirEntry**)realloc(entry, sizeof(TDirEntry*) * (entryCount + 1));
    entry[entryCount] = newEntry;
    entryCount++;
}

void TDirList::sort()
{
    //Sort the list (silly sort?)
    for(int i=0;i<(entryCount-1);i++)
    {
        if (direntry_cmp(entry[i], entry[i+1]) > 0)
        {
            TDirEntry* tmp = entry[i];
            entry[i] = entry[i+1];
            entry[i+1] = tmp;
            if (i > 0)
                i -= 2;
        }
    }
}

int TDirList::songCount()
{
    int count = 0;
    for(int i=0;i<entryCount;i++)
    {
        switch(entry[i]->type)
        {
        case DET_Folder:
            count += entry[i]->getDirList()->songCount();
            break;
        case DET_SongFoF:
            count++;
            break;
        case DET_Unknown:
            break;
        }
    }
    return count;
}

TDirEntry* TDirList::songNum(int num)
{
    for(int i=0;i<entryCount;i++)
    {
        switch(entry[i]->type)
        {
        case DET_Folder:
            {
                TDirEntry* e = entry[i]->getDirList()->songNum(num);
                if (e)
                    return e;
                num -= entry[i]->getDirList()->songCount();
            }
            break;
        case DET_SongFoF:
            if (num == 0)
                return entry[i];
            num--;
            break;
        case DET_Unknown:
            break;
        }
    }
    return NULL;
}

TDirList* getBaseDirList()
{
    if (useUSBdrive && baseDirListUSB)
        return baseDirListUSB;
    if (!useUSBdrive && baseDirList)
        return baseDirList;
    
    TDirList* newList = new TDirList();
    for(TDirBase** base = dirBaseList; *base; base++)
    {
        if ((*base)->usb)
        {
            if (useUSBdrive)
                readAddDir(*base, NULL, (*base)->path, newList);
        }else{
            readAddDir(*base, NULL, (*base)->path, newList);
        }
    }
    newList->sort();
    
    if (useUSBdrive)
    {
        baseDirListUSB = newList;
    }else{
        baseDirList = newList;
    }
    return newList;
}

/******************************************************************/

TStageEntry::TStageEntry(const char* nName)
{
    name = (char*)malloc(strlen(nName)+1);
    strcpy(name, nName);
    icon = -1;
}

TStageEntry::~TStageEntry()
{
    free(name);
}

TStageEntry** readStageDir(int* count)
{
    TStageEntry** list = NULL;
    int listSize = 0;
    
    DIR* dir = opendir(DATA_BASE "/stage");
    if (!dir)
    {
        *count = 0;
        return list;
    }
    
    struct dirent* file;
    while((file = readdir(dir)))
    {
        if (file->d_name[0] == '.')
            continue;
        TStageEntry* nEntry = new TStageEntry(file->d_name);
        list = (TStageEntry**)realloc(list, sizeof(TStageEntry*) * (listSize+1));
        list[listSize] = nEntry;
        listSize++;
    }
    closedir(dir);
    
    //Sort the list (silly sort?)
    for(int i=0;i<(listSize-1);i++)
    {
        if (strcmp(list[i]->name, list[i+1]->name) > 0)
        {
            TStageEntry* tmp = list[i];
            list[i] = list[i+1];
            list[i+1] = tmp;
            if (i > 0)
                i -= 2;
        }
    }
    *count = listSize;
    return list;
}

void freeStageDirContents(TStageEntry** dir, int count)
{
    for(int i=0;i<count;i++)
        delete dir[i];
    free(dir);
}
