#ifndef LUA_GL_POINTERS_H
#define LUA_GL_POINTERS_H


#include "common.h"
#include "bzfgl.h"

struct lua_State;


class LuaGLPointers {
	public:

		static bool PushEntries(lua_State* L);

		static bool Enable(lua_State* L);
		static bool Reset(lua_State* L);

	private: // call-outs
		static void CheckActiveState(lua_State* L, const char* funcName);

	private: // call-outs
		static int ArrayElement(lua_State* L);
		static int DrawArrays(lua_State* L);
		static int DrawElements(lua_State* L);
		static int DrawRangeElements(lua_State* L);
		static int DrawArraysInstanced(lua_State* L);
		static int DrawElementsInstanced(lua_State* L);

		static int VertexPointer(lua_State* L);
		static int NormalPointer(lua_State* L);
		static int TexCoordPointer(lua_State* L);
		static int MultiTexCoordPointer(lua_State* L);
		static int ColorPointer(lua_State* L);
		static int EdgeFlagPointer(lua_State* L);

		static int GetPointerState(lua_State* L);
};


#endif // LUA_GL_POINTERS_H
