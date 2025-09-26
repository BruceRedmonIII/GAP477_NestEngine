#pragma once
struct lua_State;
namespace ost
{
    class LuaInterface
    {

        lua_State* m_pLuaState;
    public:
        LuaInterface();
        ~LuaInterface();
        LuaInterface(const LuaInterface& copy) = delete;
        LuaInterface& operator=(const LuaInterface& copy) = delete;
        void DoString(const char* words);
        void DoFile(const char* path);
    };
}