#ifndef CALLINS_H_H
#define CALLINS_H_H

struct lua_State;

struct bz_CustomMapObjectInfo;

namespace CallIns {
  bool PushEntries(lua_State* L);
  bool Shutdown(lua_State* L);
}

#endif // CALLINS_H_H
