#ifndef LUA_VFS_H
#define LUA_VFS_H

#include "common.h"


struct lua_State;


class LuaVFS {
	public:
		LuaVFS();
		~LuaVFS();
		static bool PushEntries(lua_State* L);

	private:
		static int FileExists(lua_State* L);
		static int FileSize(lua_State* L);
		static int ReadFile(lua_State* L);
		static int WriteFile(lua_State* L);
		static int AppendFile(lua_State* L);
		static int RemoveFile(lua_State* L);
		static int RenameFile(lua_State* L);
		static int Include(lua_State* L);
		static int CreateDir(lua_State* L);
		static int DirList(lua_State* L);
};


#endif // LUA_VFS_H
