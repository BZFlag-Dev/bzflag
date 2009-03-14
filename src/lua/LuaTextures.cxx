
#include "common.h"

// interface header
#include "LuaTextures.h"

// system headers
#include <new>
#include <string.h>
#include <string>
using std::string;

// common headers
#include "bzfgl.h"
#include "bzfio.h"
#include "OpenGLGState.h"
#include "OpenGLTexture.h"
#include "OpenGLPassState.h"
#include "TextureManager.h"
#include "StateDatabase.h"

// local headers
#include "LuaInclude.h"
#include "LuaUtils.h"
#include "LuaHashString.h"
#include "LuaGLBuffers.h"


LuaTextureMgr luaTextureMgr;

const char* LuaTextureMgr::metaName = "Texture";

GLenum LuaTextureMgr::activeTexture = GL_TEXTURE0;



#define TEXMGR (TextureManager::instance())


//============================================================================//
//============================================================================//
//
//  LuaTexture
//

LuaTexture::LuaTexture()
{
}


LuaTexture::~LuaTexture()
{
}


//============================================================================//
//============================================================================//
//
//  LuaTextureRef
//

class LuaTextureRef : public LuaTexture
{
	public:
		LuaTextureRef(const std::string& name);
		~LuaTextureRef();

		GLuint GetTexID() const { return INVALID_GL_TEXTURE_ID; }

		bool Delete();

		bool Bind() const;
		bool IsValid() const { return bzTexID >= 0; }
		bool IsWritable() const { return false; }

		GLenum GetTarget() const;
		GLenum GetFormat() const;

		GLsizei GetSizeX() const;
		GLsizei GetSizeY() const;
		GLsizei GetSizeZ() const;

		GLint GetBorder() const;

		GLenum GetMinFilter() const;
		GLenum GetMagFilter() const;

		GLenum GetWrapS() const;
		GLenum GetWrapT() const;
		GLenum GetWrapR() const;

		GLfloat GetAniso() const;

	private:
		int bzTexID;
		const std::string name;
};


//============================================================================//

LuaTextureRef::LuaTextureRef(const std::string& _name) : name(_name)
{
	bzTexID = TEXMGR.getTextureID(name.c_str(), false);
}


LuaTextureRef::~LuaTextureRef()
{
	LuaLog(0, "deleting LuaTextureRef\n");
}


bool LuaTextureRef::Delete()
{
	if (bzTexID < 0) {
		return false;
	}
	bzTexID = -1;
	return true;
}


bool LuaTextureRef::Bind() const
{
	if (bzTexID < 0) {
		return false;
	}
	const ImageInfo& ii = TEXMGR.getInfo(bzTexID);
	if (ii.texture == NULL) {
		return false;
	}
	glEnable(GL_TEXTURE_2D);
	ii.texture->execute();
	return true;
}


GLenum LuaTextureRef::GetTarget() const
{
	if (bzTexID < 0) {
		return -1;
	}
	const ImageInfo& ii = TEXMGR.getInfo(bzTexID);
	if (ii.texture == NULL) {
		return -1;
	}
	return GL_TEXTURE_2D;
}


GLenum LuaTextureRef::GetFormat() const
{
	if (bzTexID < 0) {
		return -1;
	}
	const ImageInfo& ii = TEXMGR.getInfo(bzTexID);
	if (ii.texture == NULL) {
		return -1;
	}
	return ii.texture->getInternalFormat();
}


GLsizei LuaTextureRef::GetSizeX() const
{
	if (bzTexID < 0) {
		return -1;
	}
	const ImageInfo& ii = TEXMGR.getInfo(bzTexID);
	return ii.x;
}


GLsizei LuaTextureRef::GetSizeY() const
{
	if (bzTexID < 0) {
		return -1;
	}
	const ImageInfo& ii = TEXMGR.getInfo(bzTexID);
	return ii.y;
}


GLsizei LuaTextureRef::GetSizeZ() const
{
	if (bzTexID < 0) {
		return -1;
	}
	return 1;
}


GLint LuaTextureRef::GetBorder() const
{
	return 0;
}


GLenum LuaTextureRef::GetMinFilter() const
{
	if (bzTexID < 0) {
		return -1;
	}
	const ImageInfo& ii = TEXMGR.getInfo(bzTexID);
	if (ii.texture == NULL) {
		return -1;
	}
	return ii.texture->getMinFilter();
}


GLenum LuaTextureRef::GetMagFilter() const
{
	if (bzTexID < 0) {
		return -1;
	}
	const ImageInfo& ii = TEXMGR.getInfo(bzTexID);
	if (ii.texture == NULL) {
		return -1;
	}
	return ii.texture->getMagFilter();
}


GLenum LuaTextureRef::GetWrapS() const
{
	if (bzTexID < 0) {
		return -1;
	}
	const ImageInfo& ii = TEXMGR.getInfo(bzTexID);
	if (ii.texture == NULL) {
		return -1;
	}
	return ii.texture->getRepeat() ? GL_REPEAT : GL_CLAMP;
}


GLenum LuaTextureRef::GetWrapT() const
{
	if (bzTexID < 0) {
		return -1;
	}
	const ImageInfo& ii = TEXMGR.getInfo(bzTexID);
	if (ii.texture == NULL) {
		return -1;
	}
	return ii.texture->getRepeat() ? GL_REPEAT : GL_CLAMP;
}


GLenum LuaTextureRef::GetWrapR() const
{
	if (bzTexID < 0) {
		return -1;
	}
	const ImageInfo& ii = TEXMGR.getInfo(bzTexID);
	if (ii.texture == NULL) {
		return -1;
	}
	return ii.texture->getRepeat() ? GL_REPEAT : GL_CLAMP;
}


GLfloat LuaTextureRef::GetAniso() const
{
	if (bzTexID < 0) {
		return -1.0f;
	}
	return BZDB.eval("aniso");
}


//============================================================================//
//============================================================================//
//
//  LuaTextureObj
//

class LuaTextureObj : public LuaTexture
{
	public:
		LuaTextureObj(GLenum target, GLenum format,
		              GLsizei xsize, GLsizei ysize, GLsizei zsize, GLint border,
		              GLenum min_filter, GLenum mag_filter,
		              GLenum wrap_s, GLenum wrap_t, GLenum wrap_r,
		              GLfloat aniso);
		~LuaTextureObj();

		GLuint GetTexID() const { return texID; }

		bool Delete();

		bool Bind() const;
		bool IsValid() const { return texID != INVALID_GL_TEXTURE_ID; }
		bool IsWritable() const { return true; }

		GLenum GetTarget() const { return target; }
		GLenum GetFormat() const { return format; }

		GLsizei GetSizeX() const { return xsize; }
		GLsizei GetSizeY() const { return ysize; }
		GLsizei GetSizeZ() const { return zsize; }

		GLint GetBorder() const { return border; }

		GLenum GetMinFilter() const { return min_filter; }
		GLenum GetMagFilter() const { return mag_filter; }

		GLenum GetWrapS() const { return wrap_s; }
		GLenum GetWrapT() const { return wrap_t; }
		GLenum GetWrapR() const { return wrap_r; }

		GLfloat GetAniso() const { return aniso; }

	private:
		void InitContext();
		void FreeContext();

	public:
		static void StaticInitContext(void* data);
		static void StaticFreeContext(void* data);

	private:
		GLuint texID;

		GLenum target;
		GLenum format;

		GLsizei xsize;
		GLsizei ysize;
		GLsizei zsize;
		GLint border;

		GLenum min_filter;
		GLenum mag_filter;

		GLenum wrap_s;
		GLenum wrap_t;
		GLenum wrap_r;

		GLfloat aniso;

	public:
};


//============================================================================//

int LuaTextureMgr::GetPixelSize(GLenum format, GLenum type)
{
	int fSize = -1;
	switch (format) {
//		case GL_COLOR_INDEX:     { fSize = 1; break; } // not supported
		case GL_RED:             { fSize = 1; break; }
		case GL_GREEN:           { fSize = 1; break; }
		case GL_BLUE:            { fSize = 1; break; }
		case GL_ALPHA:           { fSize = 1; break; }
		case GL_LUMINANCE:       { fSize = 1; break; }
		case GL_LUMINANCE_ALPHA: { fSize = 2; break; }
		case GL_RGB:             { fSize = 3; break; }
		case GL_BGR:             { fSize = 3; break; }
		case GL_RGBA:            { fSize = 4; break; }
		case GL_BGRA:            { fSize = 4; break; }
		default: {
			return -1;
		}
	}

	int tSize = -1;
	switch (type) {
		case GL_BYTE:           { tSize = 1; break; }
		case GL_UNSIGNED_BYTE:  { tSize = 1; break; }
		case GL_BITMAP:         { tSize = 1; break; }
		case GL_SHORT:          { tSize = 2; break; }
		case GL_UNSIGNED_SHORT: { tSize = 2; break; }
		case GL_UNSIGNED_INT:   { tSize = 4; break; }
		case GL_INT:            { tSize = 4; break; }
		case GL_FLOAT:          { tSize = 4; break; }
		case GL_UNSIGNED_BYTE_3_3_2:
		case GL_UNSIGNED_BYTE_2_3_3_REV: {
			if ((format != GL_RGB) &&
			    (format != GL_BGR)) {
				return -1;
			}
			return 1;
		}
		case GL_UNSIGNED_SHORT_5_6_5:
		case GL_UNSIGNED_SHORT_5_6_5_REV: {
			if ((format != GL_RGB) &&
			    (format != GL_BGR)) {
				return -1;
			}
			return 2;
		}
		case GL_UNSIGNED_SHORT_4_4_4_4:
		case GL_UNSIGNED_SHORT_4_4_4_4_REV:
		case GL_UNSIGNED_SHORT_5_5_5_1:
		case GL_UNSIGNED_SHORT_1_5_5_5_REV:
		case GL_UNSIGNED_INT_8_8_8_8:
		case GL_UNSIGNED_INT_8_8_8_8_REV:
		case GL_UNSIGNED_INT_10_10_10_2:
		case GL_UNSIGNED_INT_2_10_10_10_REV: {
			if ((format != GL_RGBA) &&
			    (format != GL_BGRA)) {
				return -1;
			}
			return 2;
		}
		default: {
			return -1;
		}
	}
	return fSize * tSize;
}


LuaTextureObj::LuaTextureObj(GLenum _target, GLenum _format,
		                         GLsizei _xsize, GLsizei _ysize, GLsizei _zsize,
		                         GLint _border,
		                         GLenum _min_filter, GLenum _mag_filter,
		                         GLenum _wrap_s, GLenum _wrap_t, GLenum _wrap_r,
		                         GLfloat _aniso)
: texID(INVALID_GL_TEXTURE_ID)
, target(_target)
, format(_format)
, xsize(_xsize)
, ysize(_ysize)
, zsize(_zsize)
, border(_border)
, min_filter(_min_filter)
, mag_filter(_mag_filter)
, wrap_s(_wrap_s)
, wrap_t(_wrap_t)
, wrap_r(_wrap_r)
, aniso(_aniso)
{
	OpenGLGState::registerContextInitializer(StaticFreeContext,
	                                         StaticInitContext, this);

	if (target == GL_TEXTURE_BUFFER_ARB) {
		// just generate a texture ID
		glGenTextures(1, &texID);
		return;		
	}

/*
	if (data != NULL) {
		const int pixelSize = GetPixelSize(dataFormat, dataType);
		if (pixelSize < 0) {
			luaL_error(L, "unknown data format / type");
		}
		if (dataSize < (pixelSize * xsize * ysize * zsize)) {
			luaL_error(L, "not enough data");
		}
	}
*/
		
	if (!OpenGLPassState::PushAttrib(GL_TEXTURE_BIT)) {
		return; // exceeded the attrib stack depth
	}

	glGenTextures(1, &texID);
	glBindTexture(target, texID);

	// FIXME -- useless -- parse for user data
	GLenum dataFormat = GL_RGBA;
	GLenum dataType   = GL_UNSIGNED_BYTE;
	if ((format == GL_DEPTH_COMPONENT) ||
	    (format == GL_DEPTH_COMPONENT16) ||
	    (format == GL_DEPTH_COMPONENT24) ||
	    (format == GL_DEPTH_COMPONENT32)) {
		dataFormat = GL_DEPTH_COMPONENT;
		dataType = GL_FLOAT;
	}


	glGetError(); // clear current error
	glTexImage2D(target, 0, format, 
	             xsize, ysize, border,
	             dataFormat, dataType, NULL);
	const GLenum err = glGetError();
	if (err != GL_NO_ERROR) {
		glDeleteTextures(1, &texID);
		texID = INVALID_GL_TEXTURE_ID;
		OpenGLPassState::PopAttrib();
		return;
	}

	glTexParameteri(target, GL_TEXTURE_WRAP_S, wrap_s);
	glTexParameteri(target, GL_TEXTURE_WRAP_T, wrap_t);
	glTexParameteri(target, GL_TEXTURE_WRAP_R, wrap_r);
	glTexParameteri(target, GL_TEXTURE_MIN_FILTER, min_filter);
	glTexParameteri(target, GL_TEXTURE_MAG_FILTER, mag_filter);
	if (GLEW_VERSION_1_4) {
		glTexParameteri(target, GL_TEXTURE_COMPARE_MODE_ARB, GL_NONE);
	}

	if ((aniso != 0.0f) && GLEW_EXT_texture_filter_anisotropic) {
		static GLfloat maxAniso = -1.0f;
		if (maxAniso == -1.0f) {
			glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAniso);
		}
		const GLfloat realAniso = std::max(1.0f, std::min(maxAniso, aniso));
		glTexParameterf(target, GL_TEXTURE_MAX_ANISOTROPY_EXT, realAniso);
	}

	OpenGLPassState::PopAttrib();
}


LuaTextureObj::~LuaTextureObj()
{
	FreeContext();
	OpenGLGState::unregisterContextInitializer(StaticFreeContext,
	                                           StaticInitContext, this);
	LuaLog(0, "deleting LuaTextureObj\n");
}


bool LuaTextureObj::Delete()
{
	if (texID == INVALID_GL_TEXTURE_ID) {
		return false;
	}
	FreeContext();
	return true;
}


bool LuaTextureObj::Bind() const
{
	if (texID == INVALID_GL_TEXTURE_ID) {
		return false;
	}
	glBindTexture(target, texID);
	return true;
}


void LuaTextureObj::InitContext()
{
}


void LuaTextureObj::FreeContext()
{
	if (texID == INVALID_GL_TEXTURE_ID) {
		return;
	}
	glDeleteTextures(1, &texID);
	texID = INVALID_GL_TEXTURE_ID;
}


void LuaTextureObj::StaticInitContext(void* data)
{
	((LuaTextureObj*)data)->InitContext();
}


void LuaTextureObj::StaticFreeContext(void* data)
{
	((LuaTextureObj*)data)->FreeContext();
}


//============================================================================//
//============================================================================//
//
//  LuaTextureMgr
//

bool LuaTextureMgr::PushEntries(lua_State* L)
{
	CreateMetatable(L);

	PUSH_LUA_CFUNC(L, RefTexture);
	PUSH_LUA_CFUNC(L, CreateTexture);
	PUSH_LUA_CFUNC(L, DeleteTexture);

	PUSH_LUA_CFUNC(L, Texture);
	PUSH_LUA_CFUNC(L, TexEnv);
	PUSH_LUA_CFUNC(L, TexGen);
	PUSH_LUA_CFUNC(L, TexParameter);

	if (glActiveTexture != NULL) {
		PUSH_LUA_CFUNC(L, ActiveTexture);
		PUSH_LUA_CFUNC(L, MultiTexture);
		PUSH_LUA_CFUNC(L, MultiTexEnv);
		PUSH_LUA_CFUNC(L, MultiTexGen);
	}

	PUSH_LUA_CFUNC(L, CopyToTexture);

	if (glGenerateMipmapEXT) {
		PUSH_LUA_CFUNC(L, GenerateMipMap);
	}

	if (GLEW_ARB_texture_buffer_object) {
		PUSH_LUA_CFUNC(L, TexBuffer);
	}

	return true;
}


//============================================================================//
//============================================================================//

const LuaTexture* LuaTextureMgr::TestLuaTexture(lua_State* L, int index)
{
	if (lua_getuserdataextra(L, index) != metaName) {
		return NULL;
	}
	return (LuaTexture*)lua_touserdata(L, index);
}


const LuaTexture* LuaTextureMgr::CheckLuaTexture(lua_State* L, int index)
{
	if (lua_getuserdataextra(L, index) != metaName) {
		luaL_argerror(L, index, "expected Texture");
	}
	return (LuaTexture*)lua_touserdata(L, index);
}


LuaTexture* LuaTextureMgr::GetLuaTexture(lua_State* L, int index)
{
	if (lua_getuserdataextra(L, index) != metaName) {
		luaL_argerror(L, index, "expected Texture");
	}
	return (LuaTexture*)lua_touserdata(L, index);
}


//============================================================================//
//============================================================================//

int LuaTextureMgr::RefTexture(lua_State* L)
{
	const char* name = luaL_checkstring(L, 1);

	void* udData = lua_newuserdata(L, sizeof(LuaTextureRef));
	new(udData) LuaTextureRef(name);

	lua_setuserdataextra(L, -1, (void*)metaName);
	luaL_getmetatable(L, metaName);
	lua_setmetatable(L, -2);

	return 1;
}


int LuaTextureMgr::CreateTexture(lua_State* L)
{
	const GLsizei xsize = (GLsizei)luaL_checknumber(L, 1);
	const GLsizei ysize = (GLsizei)luaL_checknumber(L, 2);
	const GLsizei zsize = 1;

	GLint   border = 0;
	GLenum  target = GL_TEXTURE_2D;
	GLenum  format = GL_RGBA8;
	GLenum  min_filter = GL_LINEAR;
	GLenum  mag_filter = GL_LINEAR;
	GLenum  wrap_s = GL_REPEAT;
	GLenum  wrap_t = GL_REPEAT;
	GLenum  wrap_r = GL_REPEAT;
	GLfloat aniso = 0.0f;

	if (lua_istable(L, 3)) {
		const int table = 3;
		for (lua_pushnil(L); lua_next(L, table) != 0; lua_pop(L, 1)) {
			if (lua_israwstring(L, -2)) {
				const string key = lua_tostring(L, -2);
				if (lua_israwnumber(L, -1)) {
					if (key == "target") {
						target = (GLenum)lua_tointeger(L, -1);
					} else if (key == "format") {
						format = (GLint)lua_tointeger(L, -1);
					} else if (key == "min_filter") {
						min_filter = (GLenum)lua_tointeger(L, -1);
					} else if (key == "mag_filter") {
						mag_filter = (GLenum)lua_tointeger(L, -1);
					} else if (key == "wrap_s") {
						wrap_s = (GLenum)lua_tointeger(L, -1);
					} else if (key == "wrap_t") {
						wrap_t = (GLenum)lua_tointeger(L, -1);
					} else if (key == "wrap_r") {
						wrap_r = (GLenum)lua_tointeger(L, -1);
					} else if (key == "aniso") {
						aniso = (GLfloat)lua_tonumber(L, -1);
					}
				}
				else if (lua_isboolean(L, -1)) {
					if (key == "border") {
						border = lua_tobool(L, -1) ? 1 : 0;
					}
				}
			}
		}
	}

	void* udData = lua_newuserdata(L, sizeof(LuaTextureObj));
	LuaTextureObj* texObj = new(udData) LuaTextureObj(target, format,
	                                                xsize, ysize, zsize, border,
	                                                min_filter, mag_filter,
	                                                wrap_s, wrap_t, wrap_r,
	                                                aniso);
	lua_setuserdataextra(L, -1, (void*)metaName);
	luaL_getmetatable(L, metaName);
	lua_setmetatable(L, -2);

	if (!texObj->IsValid()) {
		return 0;
	}

	return 1;
}


//============================================================================//
//============================================================================//

bool LuaTextureMgr::CreateMetatable(lua_State* L)
{
	luaL_newmetatable(L, metaName);
	HSTR_PUSH_CFUNC(L,  "__gc",    MetaGC);
	HSTR_PUSH_CFUNC(L,  "__index", MetaIndex);
	HSTR_PUSH_STRING(L, "__metatable", "no access");
	lua_pop(L, 1);  
	return true;
}


int LuaTextureMgr::MetaGC(lua_State* L)
{
	LuaTexture* texture = GetLuaTexture(L, 1);
	texture->~LuaTexture();
	return 0;
}


int LuaTextureMgr::MetaIndex(lua_State* L)
{
	const LuaTexture* texture = CheckLuaTexture(L, 1);
	if (texture == NULL) {
		return 0;
	}

	if (!lua_israwstring(L, 2)) {
		return 0;
	}

	const string key = lua_tostring(L, 2);
	if (key == "valid") { lua_pushboolean(L, texture->IsValid()); }
	else if (key == "target") { lua_pushinteger(L, texture->GetTarget()); }
	else if (key == "format") { lua_pushinteger(L, texture->GetFormat()); }
	else if (key == "xsize")  { lua_pushinteger(L, texture->GetSizeX());  }
	else if (key == "ysize")  { lua_pushinteger(L, texture->GetSizeY());  }
	else if (key == "zsize")  { lua_pushinteger(L, texture->GetSizeZ());  }
	else if (key == "border") { lua_pushinteger(L, texture->GetBorder()); }
	else if (key == "aniso")  { lua_pushnumber(L,  texture->GetAniso());  }
	else if (key == "wrapS")  { lua_pushinteger(L, texture->GetWrapS());  }
	else if (key == "wrapT")  { lua_pushinteger(L, texture->GetWrapT());  }
	else if (key == "wrapR")  { lua_pushinteger(L, texture->GetWrapR());  }
	else if (key == "minFilter") { lua_pushinteger(L, texture->GetMinFilter()); }
	else if (key == "magFilter") { lua_pushinteger(L, texture->GetMinFilter()); }
	else {
		return 0;
	}
	return 1;
}


//============================================================================//
//============================================================================//

int LuaTextureMgr::DeleteTexture(lua_State* L)
{
	if (OpenGLGState::isExecutingInitFuncs()) {
		luaL_error(L, "gl.DeleteTexture can not be used in GLReload");
	}
	if (lua_isnil(L, 1)) {
		return 0;
	}
	LuaTexture* texture = GetLuaTexture(L, 1);
	texture->Delete();
	return 0;
}


bool LuaTextureMgr::ParseTexture(lua_State* L, int index)
{
	// enable/disable for GL_TEXTURE_2D
	if (lua_isboolean(L, index)) {
		if (lua_tobool(L, index)) {
			glEnable(GL_TEXTURE_2D);
		} else {
			glDisable(GL_TEXTURE_2D);
		}
		return true;
	}

	// named engine textures
	if (lua_israwstring(L, index)) {
		const char* name = lua_tostring(L, index);
		const int bzTexID = TEXMGR.getTextureID(name, false);
		if (bzTexID < 0) {
			glDisable(GL_TEXTURE_2D);
			return false;
		}
		const ImageInfo& ii = TEXMGR.getInfo(bzTexID);
		if ((ii.texture == NULL) || !ii.texture->execute()) {
			glDisable(GL_TEXTURE_2D);
			return false;
		}

		glEnable(GL_TEXTURE_2D);
		return true;
	}

	// lua texture (ref or obj)
	const LuaTexture* texture = CheckLuaTexture(L, index);
	if ((texture == NULL) || !texture->IsValid()) {
		return false;
	}

	// enable/disable for lua texture's target
	const int stateIndex = index + 1;
	if (lua_isboolean(L, stateIndex)) {
		if (lua_tobool(L, stateIndex)) {
			glEnable(texture->GetTarget());
		} else {
			glDisable(texture->GetTarget());
		}
		return true;
	}

	// bind a lua texture
	if (!texture->Bind()) {
		glDisable(texture->GetTarget());
		return false;
	}

	glEnable(texture->GetTarget());
	return true;
}


int LuaTextureMgr::Texture(lua_State* L)
{
	LuaOpenGL::CheckDrawingEnabled(L, __FUNCTION__);

	lua_pushboolean(L, ParseTexture(L, 1));

	return 1;
}


int LuaTextureMgr::MultiTexture(lua_State* L)
{
	LuaOpenGL::CheckDrawingEnabled(L, __FUNCTION__);

	const int texNum = luaL_checkint(L, 1);
	if ((texNum < 0) || (texNum >= MAX_LUA_TEXTURE_UNITS)) {
		luaL_error(L, "Bad texture unit passed to gl.MultiTexture()");
		return 0;
	}
	GLenum texUnit = GL_TEXTURE0 + texNum;

	if (texUnit != activeTexture) { glActiveTexture(texUnit); }

	const bool success = ParseTexture(L, 2);

	if (texUnit != activeTexture) { glActiveTexture(activeTexture); }

	lua_pushboolean(L, success);

	return 1;
}


int LuaTextureMgr::TexParameter(lua_State* L)
{
	//LuaOpenGL::CheckDrawingEnabled(L, __FUNCTION__);

	const LuaTexture* texture = CheckLuaTexture(L, 1);
	if (!texture->IsWritable()) {
		return 0;
	}

	const GLenum target = luaL_optint(L, 2, texture->GetTarget());
	const GLenum pname  = luaL_checkint(L, 3);

	const int maxParams = 64;
	float array[maxParams];
	const int params = lua_gettop(L) - 3;
	if (params > maxParams) {
		return 0;
	}
	memset(array, 0, sizeof(array));
	for (int i = 0; i < params; i++) {
		array[i] = luaL_checkfloat(L, i + 4);
	}

	const GLenum texTarget = texture->GetTarget();

	GLuint oldTexID;
	glGetIntegerv(texTarget, (GLint*)&oldTexID);

	if (!texture->Bind()) {
		return 0;
	}

	glTexParameterfv(target, pname, array);

	glBindTexture(texTarget, oldTexID);

	return 0;
}


int LuaTextureMgr::GenerateMipMap(lua_State* L)
{
	//LuaOpenGL::CheckDrawingEnabled(L, __FUNCTION__);

	const LuaTexture* texture = CheckLuaTexture(L, 1);
	if (!texture->IsWritable()) {
		return 0;
	}
	const GLenum texTarget = texture->GetTarget();

	GLuint oldTexID;
	glGetIntegerv(texTarget, (GLint*)&oldTexID);

	if (!texture->Bind()) {
		return 0;
	}

	glGenerateMipmapEXT(texTarget);

	glBindTexture(texTarget, oldTexID);

	return 0;
}


int LuaTextureMgr::CopyToTexture(lua_State* L)
{
	//LuaOpenGL::CheckDrawingEnabled(L, __FUNCTION__);

	const LuaTexture* texture = CheckLuaTexture(L, 1);
	if (!texture->IsWritable()) {
		return 0;
	}
	const GLenum texTarget = texture->GetTarget();

	GLuint oldTexID;
	glGetIntegerv(texTarget, (GLint*)&oldTexID);

	if (!texture->Bind()) {
		lua_pushboolean(L, false);
		return 1;
	}

	const GLint xoff =   (GLint)luaL_checkint(L, 2);
	const GLint yoff =   (GLint)luaL_checkint(L, 3);
	const GLint    x =   (GLint)luaL_checkint(L, 4);
	const GLint    y =   (GLint)luaL_checkint(L, 5);
	const GLsizei  w = (GLsizei)luaL_checkint(L, 6);
	const GLsizei  h = (GLsizei)luaL_checkint(L, 7);
	const GLenum target = (GLenum)luaL_optint(L, 8, texTarget);
	const GLenum level  = (GLenum)luaL_optint(L, 9, 0);

	glGetError(); // clear the error
	glCopyTexSubImage2D(target, level, xoff, yoff, x, y, w, h);
	lua_pushboolean(L, glGetError() == GL_NO_ERROR);

	glBindTexture(target, oldTexID);

	return 1;
}


int LuaTextureMgr::ActiveTexture(lua_State* L)
{
	LuaOpenGL::CheckDrawingEnabled(L, __FUNCTION__);

	const int args = lua_gettop(L); // number of arguments
	if ((args < 2) || !lua_israwnumber(L, 1) || !lua_isfunction(L, 2)) {
		luaL_error(L, "Incorrect arguments to gl.ActiveTexture(number, func, ...)");
	}
	const int texNum = lua_toint(L, 1);
	if ((texNum < 0) || (texNum >= MAX_LUA_TEXTURE_UNITS)) {
		luaL_error(L, "Bad texture unit passed to gl.ActiveTexture()");
		return 0;
	}
	const GLenum texUnit = texNum + GL_TEXTURE0;

	const GLenum oldTexUnit = activeTexture;

	// call the function
	activeTexture = texUnit;
	glActiveTexture(texUnit);
	const int error = lua_pcall(L, (args - 2), 0, 0);
	glActiveTexture(oldTexUnit);
	activeTexture = oldTexUnit;

	if (error != 0) {
		LuaLog(1, "gl.ActiveTexture: error(%i) = %s", error, lua_tostring(L, -1));
		lua_error(L);
	}
	return 0;
}


int LuaTextureMgr::TexEnv(lua_State* L)
{
	LuaOpenGL::CheckDrawingEnabled(L, __FUNCTION__);

	const GLenum target = (GLenum)luaL_checknumber(L, 1);
	const GLenum pname  = (GLenum)luaL_checknumber(L, 2);

	const int args = lua_gettop(L); // number of arguments
	if (args == 3) {
		const GLfloat value = (GLfloat)luaL_checknumber(L, 3);
		glTexEnvf(target, pname, value);
	}
	else if (args == 6) {
		GLfloat array[4];
		array[0] = luaL_optnumber(L, 3, 0.0f);
		array[1] = luaL_optnumber(L, 4, 0.0f);
		array[2] = luaL_optnumber(L, 5, 0.0f);
		array[3] = luaL_optnumber(L, 6, 0.0f);
		glTexEnvfv(target, pname, array);
	}
	else {
		luaL_error(L, "Incorrect arguments to gl.TexEnv()");
	}

	return 0;
}


int LuaTextureMgr::MultiTexEnv(lua_State* L)
{
	LuaOpenGL::CheckDrawingEnabled(L, __FUNCTION__);
	const int texNum    =    luaL_checkint(L, 1);
	const GLenum target = (GLenum)luaL_checknumber(L, 2);
	const GLenum pname  = (GLenum)luaL_checknumber(L, 3);

	if ((texNum < 0) || (texNum >= MAX_LUA_TEXTURE_UNITS)) {
		luaL_error(L, "Bad texture unit passed to gl.MultiTexEnv()");
	}

	const int args = lua_gettop(L); // number of arguments
	if (args == 4) {
		const GLfloat value = (GLfloat)luaL_checknumber(L, 4);
		glActiveTexture(GL_TEXTURE0 + texNum);
		glTexEnvf(target, pname, value);
		glActiveTexture(GL_TEXTURE0);
	}
	else if (args == 7) {
		GLfloat array[4];
		array[0] = luaL_optnumber(L, 4, 0.0f);
		array[1] = luaL_optnumber(L, 5, 0.0f);
		array[2] = luaL_optnumber(L, 6, 0.0f);
		array[3] = luaL_optnumber(L, 7, 0.0f);
		glActiveTexture(GL_TEXTURE0 + texNum);
		glTexEnvfv(target, pname, array);
		glActiveTexture(GL_TEXTURE0);
	}
	else {
		luaL_error(L, "Incorrect arguments to gl.MultiTexEnv()");
	}

	return 0;
}


static void SetTexGenState(GLenum target, bool state)
{
	if ((target >= GL_S) && (target <= GL_Q)) {
		const GLenum pname = GL_TEXTURE_GEN_S + (target - GL_S);
		if (state) {
			glEnable(pname);
		} else {
			glDisable(pname);
		}
	}
}


int LuaTextureMgr::TexGen(lua_State* L)
{
	LuaOpenGL::CheckDrawingEnabled(L, __FUNCTION__);

	const GLenum target = (GLenum)luaL_checknumber(L, 1);

	const int args = lua_gettop(L); // number of arguments
	if ((args == 2) && lua_isboolean(L, 2)) {
		const bool state = lua_tobool(L, 2);
		SetTexGenState(target, state);
		return 0;
	}

	const GLenum pname  = (GLenum)luaL_checknumber(L, 2);

	if (args == 3) {
		const GLfloat value = (GLfloat)luaL_checknumber(L, 3);
		glTexGenf(target, pname, value);
		SetTexGenState(target, true);
	}
	else if (args == 6) {
		GLfloat array[4];
		array[0] = luaL_optnumber(L, 3, 0.0f);
		array[1] = luaL_optnumber(L, 4, 0.0f);
		array[2] = luaL_optnumber(L, 5, 0.0f);
		array[3] = luaL_optnumber(L, 6, 0.0f);
		glTexGenfv(target, pname, array);
		SetTexGenState(target, true);
	}
	else {
		luaL_error(L, "Incorrect arguments to gl.TexGen()");
	}

	return 0;
}


int LuaTextureMgr::MultiTexGen(lua_State* L)
{
	LuaOpenGL::CheckDrawingEnabled(L, __FUNCTION__);

	const int texNum = luaL_checkint(L, 1);
	if ((texNum < 0) || (texNum >= MAX_LUA_TEXTURE_UNITS)) {
		luaL_error(L, "Bad texture unit passed to gl.MultiTexGen()");
	}

	const GLenum target = (GLenum)luaL_checknumber(L, 2);

	const int args = lua_gettop(L); // number of arguments
	if ((args == 3) && lua_isboolean(L, 3)) {
		const bool state = lua_tobool(L, 3);
		glActiveTexture(GL_TEXTURE0 + texNum);
		SetTexGenState(target, state);
		glActiveTexture(GL_TEXTURE0);
		return 0;
	}

	const GLenum pname  = (GLenum)luaL_checknumber(L, 3);

	if (args == 4) {
		const GLfloat value = (GLfloat)luaL_checknumber(L, 4);
		glActiveTexture(GL_TEXTURE0 + texNum);
		glTexGenf(target, pname, value);
		SetTexGenState(target, true);
		glActiveTexture(GL_TEXTURE0);
	}
	else if (args == 7) {
		GLfloat array[4];
		array[0] = luaL_optnumber(L, 4, 0.0f);
		array[1] = luaL_optnumber(L, 5, 0.0f);
		array[2] = luaL_optnumber(L, 6, 0.0f);
		array[3] = luaL_optnumber(L, 7, 0.0f);
		glActiveTexture(GL_TEXTURE0 + texNum);
		glTexGenfv(target, pname, array);
		SetTexGenState(target, true);
		glActiveTexture(GL_TEXTURE0);
	}
	else {
		luaL_error(L, "Incorrect arguments to gl.MultiTexGen()");
	}

	return 0;
}


//============================================================================//

int LuaTextureMgr::TexBuffer(lua_State* L)
{
	if (lua_isboolean(L, 1) && !lua_tobool(L, 1)) {
		const GLenum target = luaL_optint(L, 2, GL_TEXTURE_BUFFER_ARB);
		glTexBufferARB(target, 0, 0);
		lua_pushboolean(L, true);
		return 1;
	}

	// the texture
	const LuaTexture* tex = CheckLuaTexture(L, 1);
	if ((tex == NULL) ||
	    !tex->IsValid() || !tex->IsWritable() ||
	    (tex->GetTarget() != GL_TEXTURE_BUFFER_ARB)) {
		return 0;
	}

	// disconnect the buffer object from this texture
	if (lua_isboolean(L, 2) && !lua_tobool(L, 2)) {
		glTexBufferARB(GL_TEXTURE_BUFFER_ARB, tex->GetTexID(), 0);
		lua_pushboolean(L, true);
		return 1;
	}

	// the format
	const GLenum format = (GLenum)luaL_optint(L, 2, tex->GetFormat());

	// the buffer
	const LuaGLBuffer* buf = LuaGLBufferMgr::CheckLuaGLBuffer(L, 3);
	if (!buf->IsValid()) {
		return 0;
	}

	if (!OpenGLPassState::PushAttrib(GL_TEXTURE_BIT)) {
		return 0; // exceeded the attrib stack depth
	}

	if (!tex->Bind()) {
		lua_pushboolean(L, false);
		OpenGLPassState::PopAttrib();
		return 1;
	}

	glGetError();
	glTexBufferARB(tex->GetTarget(), format, buf->id);
	const GLenum errgl = glGetError();

	if (errgl != GL_NO_ERROR) {
		lua_pushboolean(L, false);
		lua_pushinteger(L, errgl);
		OpenGLPassState::PopAttrib();
		return 2;
	}

	lua_pushboolean(L, true);
	OpenGLPassState::PopAttrib();
	return 1;
}


//============================================================================//
//============================================================================//
