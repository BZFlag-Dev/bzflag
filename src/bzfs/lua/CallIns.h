#ifndef CALLINS_H_H
#define CALLINS_H_H

struct lua_State;

struct bz_CustomMapObjectInfo;

namespace CallIns {
  bool PushEntries(lua_State* L);
  bool CleanUp(lua_State* L);

  bool Shutdown(); // lua plugin custom call-in
}

#endif // CALLINS_H_H
