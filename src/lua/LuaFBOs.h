#ifndef LUA_FBOS_H
#define LUA_FBOS_H


#include "common.h"
#include <set>
#include <string>

// common headers
#include "bzfgl.h"


struct lua_State;


struct LuaFBO {
	void Init(lua_State* L);
	void Free(lua_State* L);

	void InitContext();
	void FreeContext();
	static void StaticInitContext(void* data);
	static void StaticFreeContext(void* data);

	GLuint id;
	GLenum target;
	int luaRef;
	GLsizei xsize;
	GLsizei ysize;
};


class LuaFBOMgr {
	public:
		LuaFBOMgr();
		~LuaFBOMgr();

		const LuaFBO* GetLuaFBO(lua_State* L, int index);

	public:
		static bool PushEntries(lua_State* L);

		static GLenum ParseAttachment(const std::string& name);

		static const char* metaName;

	private:
		std::set<LuaFBO*> fbos;

	private: // helpers
		static bool CreateMetatable(lua_State* L);
		static bool AttachObject(lua_State* L, int index,
		                         LuaFBO* fbo, GLenum attachID,
		                         GLenum attachTarget = 0,
		                         GLenum attachLevel  = 0);
		static bool ApplyAttachment(lua_State* L, int index,
		                            LuaFBO* fbo, GLenum attachID);
		static bool ApplyDrawBuffers(lua_State* L, int index);

	private: // metatable methods
		static int meta_gc(lua_State* L);
		static int meta_index(lua_State* L);
		static int meta_newindex(lua_State* L);

	private: // call-outs
		static int CreateFBO(lua_State* L);
		static int DeleteFBO(lua_State* L);
		static int IsValidFBO(lua_State* L);
		static int ActiveFBO(lua_State* L);
		static int UnsafeSetFBO(lua_State* L); // unsafe
		static int BlitFBO(lua_State* L);
};


extern LuaFBOMgr luaFBOMgr;


#endif // LUA_FBOS_H
