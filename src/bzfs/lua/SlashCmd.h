#ifndef SLASH_CMD_H
#define SLASH_CMD_H

struct lua_State;

namespace SlashCmd {
  bool PushEntries(lua_State* L);
  bool CleanUp(lua_State* L);
}

#endif // SLASH_CMD_H
