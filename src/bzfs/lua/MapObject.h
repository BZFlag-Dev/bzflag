#ifndef MAP_OBJECT_H
#define MAP_OBJECT_H

struct lua_State;

namespace MapObject {
  bool PushEntries(lua_State* L);
  bool CleanUp(lua_State* L);
}

#endif // MAP_OBJECT_H
