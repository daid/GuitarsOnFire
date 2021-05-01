#ifndef READDIR_H
#define READDIR_H
/**
 * readdir provides functions to read a directory contents of songs.
 * it also provides a function to read the stage directory. And the
 * define of the DATA_BASE which is the directory containing all data
 * files.
 */

//Set the DATA_BASE define to the directory containing all data files.
#ifdef GEKKO
#define DATA_BASE "/guitarsonfire"
#else
#define DATA_BASE "."
#endif

class TDirBase;
class TDirList;
class TDirEntry;

enum EDirEntryType
{
    DET_Folder,
    DET_SongFoF,
    DET_Unknown
};

class TDirBase
{
public:
    const char* path;
    int usb;
    
    TDirBase(const char* nPath, int usb);
};

class TSongInfo
{
public:
    char* name;
    char* artist;
    char* unlockID;
    int delay;
    
    int highscore[MAX_INSTRUMENT][MAX_DIFFICULTY];
    
    TSongInfo(const char* path);
    ~TSongInfo();
    
    int difficultyDone();
    int saveHighscores();
    void highscoreFilename(char* buffer, int bufferSize);
};

class TDirEntry
{
public:
    char* path;
    
    enum EDirEntryType type;
    TDirBase* base;
    TDirEntry* parent;
    TDirList* list;
    TSongInfo* info;

    TDirEntry(TDirBase* base, TDirEntry* parent, const char* nPath);
    ~TDirEntry();

    int getFullPath(char* buffer, int bufferSize);
    TDirList* getDirList();
};

class TDirList
{
public:
    TDirEntry** entry;
    int entryCount;
    
    TDirList();
    ~TDirList();
    
    void sort();
    void addEntry(TDirEntry* entry);
    
    int songCount();
    TDirEntry* songNum(int num);
};

/**
 * read a directory and return the list of entries.
 *  if base and path are NULL then the default song directories are used.
 */
TDirList* getBaseDirList();

extern TDirEntry* currentSongEntry;

class TStageEntry
{
public:
    char* name;
    int icon;
    
    TStageEntry(const char* nName);
    ~TStageEntry();
};

/**
 * read the stage directory and return all stages found.
 */
TStageEntry** readStageDir(int* count);
/**
 * frees the result of readStageDir
 */
void freeStageDirContents(TStageEntry** dir, int count);

#endif//READDIR_H
