#include "LuaInterface.h"
#include "../Lua/lua.hpp"

ost::LuaInterface::LuaInterface()
    : m_pLuaState(luaL_newstate())
{
    luaL_openlibs(m_pLuaState);
}

ost::LuaInterface::~LuaInterface()
{
    lua_close(m_pLuaState);
}

void ost::LuaInterface::DoString(const char* words)
{
    // Do a Lua string
    luaL_dostring(m_pLuaState, words);
}

void ost::LuaInterface::DoFile(const char* path)
{
    luaL_dofile(m_pLuaState, path);
}
