#ifndef LUA_GLBuffers_H
#define LUA_GLBuffers_H

#include "common.h"

#include <set>

#include "bzfgl.h"


struct lua_State;


class LuaGLBufferData {
	public:
		LuaGLBufferData()
		: id(0)
		, target(GL_ARRAY_BUFFER)
		, usage(GL_STATIC_READ)
		, size(0)
		, indexMax(0)
		, indexType(0)
		, indexTypeSize(0)
		{}
		virtual ~LuaGLBufferData() {};

	public:
		GLuint id;
		GLenum target;
		GLenum usage;
		GLsizei size;

		// these are assigned when the target is GL_ELEMENT_ARRAY_BUFFER
		GLuint indexMax;
		GLenum indexType;
		GLuint indexTypeSize;
};


class LuaGLBuffer : public LuaGLBufferData {
	public:
		LuaGLBuffer(const LuaGLBufferData& bufData);
		~LuaGLBuffer();

		void Delete();

		bool IsValid()     const { return id != 0; }
		bool IsIndexData() const { return target == GL_ELEMENT_ARRAY_BUFFER; }

	private:
		void InitContext();
		void FreeContext();
		static void StaticInitContext(void* data);
		static void StaticFreeContext(void* data);
};


class LuaGLBufferMgr {
	public:
		LuaGLBufferMgr();
		~LuaGLBufferMgr();

		static bool PushEntries(lua_State* L);

		static const LuaGLBuffer* TestLuaGLBuffer(lua_State* L, int index);
		static const LuaGLBuffer* CheckLuaGLBuffer(lua_State* L, int index);

		static int GetTypeSize(GLenum type);
		static int GetIndexTypeSize(GLenum type);
		static int CalcMaxElement(GLenum type, int bytes, const void* indices);

	public:
		static const char* metaName;
		static const int maxDataSize;

	private:
		static bool CreateMetatable(lua_State* L);
		static int MetaGC(lua_State* L);
		static int MetaIndex(lua_State* L);
		static int MetaNewindex(lua_State* L);

		static LuaGLBuffer* GetLuaGLBuffer(lua_State* L, int index);

		static const void* ParseArgs(lua_State* L, int index,
		                             LuaGLBufferData& bufData);
		static const void* ParseTable(lua_State* L, int index,
		                              LuaGLBufferData& bufData);
		static void IndexCheck(lua_State* L,
		                       LuaGLBufferData& bufData, const void* data);
		static void SetBufferData(const LuaGLBufferData& buffer, const void* data);

	private: // call-outs
		static int CreateBuffer(lua_State* L);
		static int DeleteBuffer(lua_State* L);
		static int BufferData(lua_State* L);
		static int BufferSubData(lua_State* L);
		static int GetBufferSubData(lua_State* L);

		static int TexBuffer(lua_State* L);

		static int UniformBuffer(lua_State* L);
};


extern LuaGLBufferMgr luaGLBufferMgr;


#endif // LUA_GLBuffers_H
