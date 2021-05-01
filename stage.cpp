#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "audio.h"
#include "audioProvider.h"
#include "game.h"
#include "video.h"
#include "input.h"
#include "midi.h"
#include "models.h"
#include "stage.h"
#include "readdir.h"
#include "strfunc.h"
#include "system.h"

char luaSavedPathBuffer[1024] = "";
#ifndef GEKKO
//On PC check if the stage file changes, then reload the stage.
time_t fileTime;
#endif

extern "C" {
#include "lua_src/lua.h"
#include "lua_src/lauxlib.h"
#include "lua_src/lualib.h"
}

const char* getStagePath()
{
    return luaSavedPathBuffer;
}

lua_State* luaState = NULL;
char luaErrorBuffer[1024];

static int luaLoadTexture(lua_State *L)
{
    const char* file = luaL_checkstring(L, 1);
    char path[1024];
    
    nprintf(path, 1024, DATA_BASE "/stage/%s/%s", luaSavedPathBuffer, file);
    
    int num = loadTexture(path, TT_Stage);
    lua_pushinteger(L, num);
    return 1;
}

static int luaSetPos(lua_State *L)
{
    float x = luaL_checknumber(L, 1);
    float y = luaL_checknumber(L, 2);
    float z = luaL_checknumber(L, 3);
    float yaw = luaL_checknumber(L, 4);
    float pitch = luaL_checknumber(L, 5);
    float roll = luaL_checknumber(L, 6);
    
    setDrawPos(0,0,-10,  25, x, y, z, yaw, pitch, roll);
    return 0;
}
static int luaDrawQuad(lua_State *L)
{
    float width = luaL_checknumber(L, 1);
    float height = luaL_checknumber(L, 2);
    int color = luaL_checkint(L, 3);
    int alpha = luaL_checkint(L, 4);
    int col[3] = {color & 0xFF, (color >> 8) & 0xFF, (color >> 16) & 0xFF};
    drawQuad(width, height, col, alpha);
    return 0;
}
static int luaDrawTexture(lua_State *L)
{
    int texture = luaL_checkint(L, 1);
    float x = luaL_checknumber(L, 2);
    float y = luaL_checknumber(L, 3);
    float w = luaL_checknumber(L, 4);
    float h = luaL_checknumber(L, 5);
    float width = luaL_checknumber(L, 6);
    float height = luaL_checknumber(L, 7);
    int alpha = luaL_checkint(L, 8);
    drawTexturedQuad(TT_Stage, texture, x,y, w, h, width, height, alpha);
    return 0;
}
static int luaGetInput(lua_State *L)
{
    int num = luaL_optint(L, 1, -1);
    lua_pushnumber(L, getInputState(num));
    return 1;
}
static int luaGetInputPress(lua_State *L)
{
    int num = luaL_optint(L, 1, -1);
    lua_pushnumber(L, getInputPress(num));
    return 1;
}
static int luaGetInputAngle(lua_State *L)
{
    int num = luaL_optint(L, 1, -1);
    lua_pushnumber(L, getInputAngle(num));
    return 1;
}
static int luaGetInputTouch(lua_State *L)
{
    int num = luaL_optint(L, 1, -1);
    lua_pushinteger(L, getInputTouch(num));
    return 1;
}
static int luaGetInputWhammy(lua_State *L)
{
    int num = luaL_optint(L, 1, -1);
    lua_pushnumber(L, getInputWhammy(num));
    return 1;
}
static int luaGetTime(lua_State *L)
{
    int n1 = luaL_optinteger(L, 1, -1);
    int n2 = luaL_optinteger(L, 2, -1);
    if (n1 > -1 && n2 > -1)
        lua_pushnumber(L, (getTime() / n1) % n2);
    else if (n1 > -1)
        lua_pushnumber(L, getTime() % n1);
    else
        lua_pushnumber(L, getTime());
    return 1;
}
static int luaHasBit(lua_State *L)
{
    int num1 = luaL_checkinteger(L, 1);
    int num2 = luaL_checkinteger(L, 2);
    lua_pushboolean(L, num1 & num2);
    return 1;
}
static int luaGetPlayerInfo(lua_State *L)
{
    int num = luaL_checkinteger(L, 1);
    if (num < 0 || num >= MAX_PLAYERS)
        return 0;
    lua_newtable(L);
    lua_pushstring(L, "playing");
    lua_pushboolean(L, player[num].playing);
    lua_settable(L, -3);
    lua_pushstring(L, "score");
    lua_pushinteger(L, player[num].score);
    lua_settable(L, -3);
    lua_pushstring(L, "streak");
    lua_pushinteger(L, player[num].streak);
    lua_settable(L, -3);
    lua_pushstring(L, "bestStreak");
    lua_pushinteger(L, player[num].bestStreak);
    lua_settable(L, -3);
    lua_pushstring(L, "difficulty");
    lua_pushinteger(L, player[num].difficulty);
    lua_settable(L, -3);
    lua_pushstring(L, "quality");
    lua_pushinteger(L, player[num].quality);
    lua_settable(L, -3);
    lua_pushstring(L, "leftyFlip");
    lua_pushboolean(L, player[num].leftyFlip);
    lua_settable(L, -3);
    return 1;
}

void loadStage(const char* path)
{
    char luaPathBuffer[1024];
    struct stat statBuf;
    
    luaErrorBuffer[0] = 0;

    if (luaState)
        lua_close(luaState);
    freeTextures(TT_Stage);

    nprintf(luaSavedPathBuffer, 1024, "%s", path);

    nprintf(luaPathBuffer, 1024, DATA_BASE "/stage/%s/stage.lua", path);
    if (stat(luaPathBuffer, &statBuf))
        return;
    
#ifndef GEKKO
    fileTime = statBuf.st_mtime;
#endif
    
    luaState = lua_open();
    luaL_openlibs(luaState);
    lua_register(luaState, "loadTexture", luaLoadTexture);
    lua_register(luaState, "setPos", luaSetPos);
    lua_register(luaState, "drawQuad", luaDrawQuad);
    lua_register(luaState, "drawTexture", luaDrawTexture);
    lua_register(luaState, "getTime", luaGetTime);
    lua_register(luaState, "getInput", luaGetInput);
    lua_register(luaState, "getInputPress", luaGetInputPress);
    lua_register(luaState, "getInputAngle", luaGetInputAngle);
    lua_register(luaState, "getInputWhammy", luaGetInputWhammy);
    lua_register(luaState, "getInputTouch", luaGetInputTouch);
    lua_register(luaState, "getPlayerInfo", luaGetPlayerInfo);
    lua_register(luaState, "hasBit", luaHasBit);
    
    if (luaL_dofile(luaState, luaPathBuffer))
    {
        nprintf(luaErrorBuffer, 1024, "%s", lua_tostring(luaState, -1), 1024);
        lua_close(luaState);
        luaState = NULL;
    }
}

void drawStage()
{
#ifndef GEKKO
    struct stat statBuf;
    char luaPathBuffer[1024];
    nprintf(luaPathBuffer, 1024, DATA_BASE "/stage/%s/stage.lua", luaSavedPathBuffer);
    if (!stat(luaPathBuffer, &statBuf) && fileTime != statBuf.st_mtime)
    {
        loadStage(luaSavedPathBuffer);
    }
#endif
    
    
    /** Draw the stage */
    setDefaultFrustum();
    if (!luaState)
    {
        drawText(luaErrorBuffer, -1, 0, 1.0);
        if (strlen(luaErrorBuffer) > 50)
            drawText(luaErrorBuffer + 50, -1,-0.1, 1.0);
        if (strlen(luaErrorBuffer) > 100)
            drawText(luaErrorBuffer + 100, -1,-0.2, 1.0);
        if (strlen(luaErrorBuffer) > 150)
            drawText(luaErrorBuffer + 150, -1,-0.3, 1.0);
        return;
    }
    
    //Call the "draw" lua function
    lua_getglobal(luaState, "draw");
    if (lua_pcall(luaState, 0, 0, 0))
    {
        nprintf(luaErrorBuffer, 1024, "%s", lua_tostring(luaState, -1), 1024);
        lua_close(luaState);
        luaState = NULL;
    }
}
