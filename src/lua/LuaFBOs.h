#ifndef LUA_FBOS_H
#define LUA_FBOS_H


#include "common.h"

// system headers
#include <string>

// common headers
#include "bzfgl.h"

// local headers
#include "LuaInclude.h"


struct lua_State;


class LuaFBOData {
	public:
		LuaFBOData()
		: id(0)
		, target(GL_FRAMEBUFFER)
		, luaRef(LUA_NOREF)
		, xsize(0)
		, ysize(0)
		{}
		virtual ~LuaFBOData() {};

	public:
		GLuint id;
		GLenum target;
		int luaRef;
		GLsizei xsize;
		GLsizei ysize;
};


class LuaFBO : public LuaFBOData {
	public:
		LuaFBO(const LuaFBOData& fboData);
		~LuaFBO();

		void Delete(lua_State* L);

		bool IsValid() { return (id != 0) && (luaRef != LUA_NOREF); }

	private:
		void InitContext();
		void FreeContext();
		static void StaticInitContext(void* data);
		static void StaticFreeContext(void* data);
};


class LuaFBOMgr {
	public:
		LuaFBOMgr();
		~LuaFBOMgr();

	public:
		static bool PushEntries(lua_State* L);

		static GLenum ParseAttachment(const std::string& name);

		static const char* metaName;

		static const LuaFBO* TestLuaFBO(lua_State* L, int index);
		static const LuaFBO* CheckLuaFBO(lua_State* L, int index);

	private: // helpers
		static bool CreateMetatable(lua_State* L);
		static bool AttachObject(lua_State* L, int index,
		                         LuaFBO* fbo, GLenum attachID,
		                         GLenum attachTarget = 0,
		                         GLenum attachLevel  = 0);
		static bool ApplyAttachment(lua_State* L, int index,
		                            LuaFBO* fbo, GLenum attachID);
		static bool ApplyDrawBuffers(lua_State* L, int index);

		static LuaFBO* GetLuaFBO(lua_State* L, int index);

	private: // metatable methods
		static int MetaGC(lua_State* L);
		static int MetaIndex(lua_State* L);
		static int MetaNewindex(lua_State* L);

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
