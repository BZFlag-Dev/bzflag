#ifndef LUA_GL_QUERIES_H
#define LUA_GL_QUERIES_H


#include "common.h"

// common headers
#include "bzfgl.h"


struct lua_State;


class LuaGLQuery {
	public:
		LuaGLQuery(GLuint queryID, GLenum target);
		~LuaGLQuery();

		bool Delete();

		bool IsValid() const { return (id != 0); }

		GLuint GetID()     const { return id; }
		GLenum GetTarget() const { return target; }

	private:
		GLuint id;
		GLenum target;

	private:
		void InitContext();
		void FreeContext();

	private:
		static void StaticInitContext(void* data);
		static void StaticFreeContext(void* data);
};


class LuaGLQueryMgr {
	public:
		static bool PushEntries(lua_State* L);

		static const LuaGLQuery* TestLuaGLQuery(lua_State* L, int index);
		static const LuaGLQuery* CheckLuaGLQuery(lua_State* L, int index);

	public:
		static const char* metaName;

	private:
		static bool CreateMetatable(lua_State* L);
		static int MetaGC(lua_State* L);
		static int MetaIndex(lua_State* L);

		static LuaGLQuery* GetLuaGLQuery(lua_State* L, int index);
	
	private: // call-outs
		static int CreateQuery(lua_State* L);
		static int DeleteQuery(lua_State* L);
		static int RunQuery(lua_State* L);
		static int GetQuery(lua_State* L);
		static int BeginEndConditionalRender(lua_State* L);
};


#endif // LUA_GL_QUERIES_H
