#ifndef RAW_LINK_H
#define RAW_LINK_H

struct lua_State;

namespace RawLink {
  bool PushEntries(lua_State* L);
  bool CleanUp(lua_State* L);
}

#endif // RAW_LINK_H
