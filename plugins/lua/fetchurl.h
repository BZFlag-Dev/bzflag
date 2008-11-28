#ifndef FETCH_URL_H
#define FETCH_URL_H

struct lua_State;

namespace FetchURL {
  bool PushEntries(lua_State* L);
  bool CleanUp(lua_State* L);
}

#endif // FETCH_URL_H
