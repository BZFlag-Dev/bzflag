#ifndef LUA_BZFS_H
#define LUA_BZFS_H


#include <string>


class LuaBZFS {
  public:
    static bool init(const std::string& scriptFile);
    static bool kill();
    static bool isActive();
    static void recvCommand(const std::string& command, int player);
};


#endif // LUA_BZFS_H
