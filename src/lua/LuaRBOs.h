#ifndef LUA_RBOS_H
#define LUA_RBOS_H

#include "common.h"

#include <set>

#include "bzfgl.h"


struct lua_State;


struct LuaRBO {
	LuaRBO() : id(0), target(0), format(0), xsize(0), ysize(0) {}
	void Init();
	void Free();

	void InitContext();
	void FreeContext();
	static void StaticInitContext(void* data);
	static void StaticFreeContext(void* data);

	GLuint id;
	GLenum target;
	GLenum format;
	GLsizei xsize;
	GLsizei ysize;
};


class LuaRBOMgr {
	public:
		LuaRBOMgr();
		~LuaRBOMgr();

		static bool PushEntries(lua_State* L);

		static const LuaRBO* GetLuaRBO(lua_State* L, int index);

		static const char* metaName;

	private:
		std::set<LuaRBO*> rbos;

	private: // helpers
		static bool CreateMetatable(lua_State* L);

	private: // metatable methods
		static int meta_gc(lua_State* L);
		static int meta_index(lua_State* L);
		static int meta_newindex(lua_State* L);

	private:
		static int CreateRBO(lua_State* L);
		static int DeleteRBO(lua_State* L);
};


extern LuaRBOMgr luaRBOMgr;


#endif // LUA_RBOS_H
