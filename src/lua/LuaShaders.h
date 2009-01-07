#ifndef LUA_SHADERS_H
#define LUA_SHADERS_H

#include "common.h"

#include <string>
#include <vector>
#include <set>

#include "bzfgl.h"


struct lua_State;


class LuaShader {
	public:
		struct Object {
			Object(GLuint _id, GLenum _type) : id(_id), type(_type) {}
			GLuint id;
			GLenum type;
		};

	public:
		LuaShader(GLuint progID, const std::vector<Object>& objects);
		~LuaShader();

		inline bool IsValid() const { return (progID != 0); }
		inline GLuint GetProgID() const { return progID; }

	private:
		GLuint progID;
		std::vector<Object> objects;

	private:
		void InitContext();
		void FreeContext();
		static void StaticInitContext(void* data);
		static void StaticFreeContext(void* data);
};


class LuaShaderMgr {
	public:
		bool InsertShader(LuaShader*);
		bool RemoveShader(LuaShader*);

	private:
		std::set<LuaShader*> shaders;

	public:
		static bool PushEntries(lua_State* L);

		static void Init();
		static void Free();

	public:
		static const char* metaName;

	private:
		static int activeShaderDepth;

	private:
		static bool CreateMetatable(lua_State* L);
		static int MetaGC(lua_State* L);
		static int MetaIndex(lua_State* L);

		static LuaShader*& CheckLuaShader(lua_State* L, int index);

	private: // call-outs
		static int CreateShader(lua_State* L);
		static int DeleteShader(lua_State* L);
		static int UseShader(lua_State* L);
		static int ActiveShader(lua_State* L);

		static int GetActiveUniforms(lua_State* L);
		static int GetUniformLocation(lua_State* L);
		static int Uniform(lua_State* L);
		static int UniformInt(lua_State* L);
		static int UniformMatrix(lua_State* L);

		static int SetShaderParameter(lua_State* );

		static int GetShaderLog(lua_State* L);

	private:
		void InitContext();
		void FreeContext();
		static void StaticInitContext(void* data);
		static void StaticFreeContext(void* data);
};

#endif // LUA_SHADERS_H
