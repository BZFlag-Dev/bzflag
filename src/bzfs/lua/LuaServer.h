#ifndef LUA_SERVER_H
#define LUA_SERVER_H


#include <string>

struct lua_State;


class LuaServer {
  public:
    static bool init(const std::string& scriptFile);
    static bool kill();

    static bool isActive();

    static void recvCommand(const std::string& command, int player);

    static lua_State* GetL(); // for local lua libraries
};


#endif // LUA_SERVER_H
