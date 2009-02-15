#ifndef CALLINS_H
#define CALLINS_H

#include <string>

struct lua_State;

struct bz_CustomMapObjectInfo;

namespace CallIns {
  bool PushEntries(lua_State* L);
  bool CleanUp(lua_State* L);

  // lua plugin custom call-ins
  bool Shutdown();
  bool RecvCommand(const std::string& cmd);
}

#endif // CALLINS_H
