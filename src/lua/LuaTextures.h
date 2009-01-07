#ifndef LUA_TEXTURES_H
#define LUA_TEXTURES_H

#include "common.h"

// system headers
#include <string>
#include <set>

// common headers
#include "bzfgl.h"

// local headers
#include "LuaOpenGL.h"


struct lua_State;


static const int MAX_TEXTURE_UNITS = 32;


/******************************************************************************/

class LuaTexture
{
	public:
		LuaTexture();
		virtual ~LuaTexture();

		virtual GLuint GetTexID() const = 0;

		virtual bool Bind() const = 0;
		virtual bool IsValid() const = 0;
		virtual bool IsWritable() const = 0;

		virtual GLenum GetTarget() const = 0;
		virtual GLenum GetFormat() const = 0;

		virtual GLsizei GetSizeX() const = 0;
		virtual GLsizei GetSizeY() const = 0;
		virtual GLsizei GetSizeZ() const = 0;

		virtual GLint GetBorder() const = 0;

		virtual GLenum GetMinFilter() const = 0;
		virtual GLenum GetMagFilter() const = 0;

		virtual GLenum GetWrapS() const = 0;
		virtual GLenum GetWrapT() const = 0;
		virtual GLenum GetWrapR() const = 0;

		virtual GLfloat GetAniso() const = 0;
};


/******************************************************************************/

class LuaTextureMgr {
	public:
		bool InsertTexture(LuaTexture*);
		bool RemoveTexture(LuaTexture*);

	private:
		std::set<LuaTexture*> textures;

	public:
		static bool PushEntries(lua_State* L);

		static void Init();
		static void Free();

	public:
		static const char* metaName;

	private:
		static bool CreateMetatable(lua_State* L);
		static int MetaGC(lua_State* L);
		static int MetaIndex(lua_State* L);

		static LuaTexture*& CheckLuaTexture(lua_State* L, int index);

	private: // call-outs
		static int RefTexture(lua_State* L);
		static int CreateTexture(lua_State* L);
		static int DeleteTexture(lua_State* L);
		static int Texture(lua_State* L);
		static int TexParameter(lua_State* L);
		static int GenerateMipMap(lua_State* L);
		static int CopyToTexture(lua_State* L);

	private:
		void InitContext();
		void FreeContext();
		static void StaticInitContext(void* data);
		static void StaticFreeContext(void* data);
};


extern LuaTextureMgr luaTextureMgr;


/******************************************************************************/

#endif // LUA_TEXTURES_H
