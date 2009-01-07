
#include "common.h"

// implementation header
#include "LuaTextures.h"

// system headers
#include <string.h>
#include <string>
#include <set>
using std::string;
using std::set;

// common headers
#include "bzfgl.h"
#include "OpenGLGState.h"
#include "OpenGLTexture.h"
#include "TextureManager.h"
#include "StateDatabase.h"

// local headers
#include "LuaInclude.h"
#include "LuaHashString.h"


LuaTextureMgr luaTextureMgr;

const char* LuaTextureMgr::metaName = "Texture";


#define TEXMGR (TextureManager::instance())


/******************************************************************************/
/******************************************************************************/
//
//  LuaTexture
//

LuaTexture::LuaTexture()
{
	luaTextureMgr.InsertTexture(this);
}


LuaTexture::~LuaTexture()
{
	luaTextureMgr.RemoveTexture(this);
}


/******************************************************************************/
/******************************************************************************/
//
//  LuaTextureRef
//

class LuaTextureRef : public LuaTexture
{
	public:
		LuaTextureRef(const std::string& name);
		~LuaTextureRef() { printf(" deleting LuaTextureRef\n"); } // FIXME

		GLuint GetTexID() const { return INVALID_GL_TEXTURE_ID; }
		bool Bind() const;
		bool IsValid() const { return texNum >= 0; }
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
		int texNum;
		std::string name;
};


/******************************************************************************/

LuaTextureRef::LuaTextureRef(const std::string& _name) : name(_name)
{
	texNum = TEXMGR.getTextureID(name.c_str(), false);
}


bool LuaTextureRef::Bind() const
{
	if (texNum < 0) {
		return false;
	}
	const ImageInfo& ii = TEXMGR.getInfo(texNum);
	if (ii.texture == NULL) {
		return false;
	}
	glEnable(GL_TEXTURE_2D);
	ii.texture->execute();
	return true;
}


GLenum LuaTextureRef::GetTarget() const
{
	if (texNum < 0) {
		return -1;
	}
	const ImageInfo& ii = TEXMGR.getInfo(texNum);
	if (ii.texture == NULL) {
		return -1;
	}
	return GL_TEXTURE_2D;
}


GLenum LuaTextureRef::GetFormat() const
{
	if (texNum < 0) {
		return -1;
	}
	const ImageInfo& ii = TEXMGR.getInfo(texNum);
	if (ii.texture == NULL) {
		return -1;
	}
	return ii.texture->getInternalFormat();
}


GLsizei LuaTextureRef::GetSizeX() const
{
	if (texNum < 0) {
		return -1;
	}
	const ImageInfo& ii = TEXMGR.getInfo(texNum);
	return ii.x;
}


GLsizei LuaTextureRef::GetSizeY() const
{
	if (texNum < 0) {
		return -1;
	}
	const ImageInfo& ii = TEXMGR.getInfo(texNum);
	return ii.y;
}


GLsizei LuaTextureRef::GetSizeZ() const
{
	if (texNum < 0) {
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
	if (texNum < 0) {
		return -1;
	}
	const ImageInfo& ii = TEXMGR.getInfo(texNum);
	if (ii.texture == NULL) {
		return -1;
	}
	return ii.texture->getMinFilter();
}


GLenum LuaTextureRef::GetMagFilter() const
{
	if (texNum < 0) {
		return -1;
	}
	const ImageInfo& ii = TEXMGR.getInfo(texNum);
	if (ii.texture == NULL) {
		return -1;
	}
	return ii.texture->getMagFilter();
}


GLenum LuaTextureRef::GetWrapS() const
{
	if (texNum < 0) {
		return -1;
	}
	const ImageInfo& ii = TEXMGR.getInfo(texNum);
	if (ii.texture == NULL) {
		return -1;
	}
	return ii.texture->getRepeat() ? GL_REPEAT : GL_CLAMP;
}


GLenum LuaTextureRef::GetWrapT() const
{
	if (texNum < 0) {
		return -1;
	}
	const ImageInfo& ii = TEXMGR.getInfo(texNum);
	if (ii.texture == NULL) {
		return -1;
	}
	return ii.texture->getRepeat() ? GL_REPEAT : GL_CLAMP;
}


GLenum LuaTextureRef::GetWrapR() const
{
	if (texNum < 0) {
		return -1;
	}
	const ImageInfo& ii = TEXMGR.getInfo(texNum);
	if (ii.texture == NULL) {
		return -1;
	}
	return ii.texture->getRepeat() ? GL_REPEAT : GL_CLAMP;
}


GLfloat LuaTextureRef::GetAniso() const
{
	if (texNum < 0) {
		return -1.0f;
	}
	return BZDB.evalInt("aniso");
}


/******************************************************************************/
/******************************************************************************/
//
//  LuaTextureObj
//

class LuaTextureObj : public LuaTexture
{
	public:
		LuaTextureObj(GLenum target, GLenum format,
		              GLsizei xsize, GLsizei ysize, GLint border,
		              GLenum min_filter, GLenum mag_filter,
		              GLenum wrap_s, GLenum wrap_t, GLenum wrap_r,
		              GLfloat aniso);
		~LuaTextureObj();

		GLuint GetTexID() const { return texID; }

		bool Bind() const;
		bool IsValid() const { return texID != INVALID_GL_TEXTURE_ID; }
		bool IsWritable() const { return true; }

		GLenum GetTarget() const { return target; }
		GLenum GetFormat() const { return format; }

		GLsizei GetSizeX() const { return xsize; }
		GLsizei GetSizeY() const { return ysize; }
		GLsizei GetSizeZ() const { return 1; }

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
		GLint border;

		GLenum min_filter;
		GLenum mag_filter;

		GLenum wrap_s;
		GLenum wrap_t;
		GLenum wrap_r;

		GLfloat aniso;

	public:
};


/******************************************************************************/

LuaTextureObj::LuaTextureObj(GLenum _target, GLenum _format,
		                         GLsizei _xsize, GLsizei _ysize, GLint _border,
		                         GLenum _min_filter, GLenum _mag_filter,
		                         GLenum _wrap_s, GLenum _wrap_t, GLenum _wrap_r,
		                         GLfloat _aniso)
: texID(INVALID_GL_TEXTURE_ID)
, target(_target)
, format(_format)
, xsize(_xsize)
, ysize(_ysize)
, border(_border)
, min_filter(_min_filter)
, mag_filter(_mag_filter)
, wrap_s(_wrap_s)
, wrap_t(_wrap_t)
, wrap_r(_wrap_r)
, aniso(_aniso)
{
	GLint currentBinding;
	glGetIntegerv(GL_TEXTURE_BINDING_2D, &currentBinding); // FIXME -- target

	glGenTextures(1, &texID);
	glBindTexture(target, texID);

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
		glBindTexture(GL_TEXTURE_2D, currentBinding);
		texID = INVALID_GL_TEXTURE_ID;
	}

	glTexParameteri(target, GL_TEXTURE_WRAP_S, wrap_s);
	glTexParameteri(target, GL_TEXTURE_WRAP_T, wrap_t);
	glTexParameteri(target, GL_TEXTURE_WRAP_R, wrap_r);
	glTexParameteri(target, GL_TEXTURE_MIN_FILTER, min_filter);
	glTexParameteri(target, GL_TEXTURE_MAG_FILTER, mag_filter);
	glTexParameteri(target, GL_TEXTURE_COMPARE_MODE_ARB, GL_NONE);

	if ((aniso != 0.0f) && GLEW_EXT_texture_filter_anisotropic) {
		static GLfloat maxAniso = -1.0f;
		if (maxAniso == -1.0f) {
			glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAniso);
		}
		const GLfloat realAniso = std::max(1.0f, std::min(maxAniso, aniso));
		glTexParameterf(target, GL_TEXTURE_MAX_ANISOTROPY_EXT, realAniso);
	}

	glBindTexture(GL_TEXTURE_2D, currentBinding); // revert the current binding

	OpenGLGState::registerContextInitializer(StaticFreeContext,
	                                         StaticInitContext, this);
}


LuaTextureObj::~LuaTextureObj()
{
	printf("deleting LuaTextureObj\n"); // FIXME
	FreeContext();
	OpenGLGState::unregisterContextInitializer(StaticFreeContext,
	                                           StaticInitContext, this);
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


/******************************************************************************/
/******************************************************************************/
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
	PUSH_LUA_CFUNC(L, TexParameter);
	if (glGenerateMipmapEXT) {
		PUSH_LUA_CFUNC(L, GenerateMipMap);
	}
	PUSH_LUA_CFUNC(L, CopyToTexture);

	return true;
}


void LuaTextureMgr::Init()
{
	OpenGLGState::registerContextInitializer(StaticFreeContext,
	                                         StaticInitContext, NULL);
}


void LuaTextureMgr::Free()
{
	OpenGLGState::unregisterContextInitializer(StaticFreeContext,
	                                           StaticInitContext, NULL);
}


int LuaTextureMgr::RefTexture(lua_State* L)
{
	const char* name = luaL_checkstring(L, 1);

	LuaTextureRef** texPtr =
		(LuaTextureRef**)lua_newuserdata(L, sizeof(LuaTextureRef*));
	luaL_getmetatable(L, metaName);
	lua_setmetatable(L, -2);

	*texPtr = new LuaTextureRef(name);

	return 1;
}


int LuaTextureMgr::CreateTexture(lua_State* L)
{
	GLsizei xsize = (GLsizei)luaL_checknumber(L, 1);
	GLsizei ysize = (GLsizei)luaL_checknumber(L, 2);

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
						target = (GLenum)lua_tonumber(L, -1);
					} else if (key == "format") {
						format = (GLint)lua_tonumber(L, -1);
					} else if (key == "min_filter") {
						min_filter = (GLenum)lua_tonumber(L, -1);
					} else if (key == "mag_filter") {
						mag_filter = (GLenum)lua_tonumber(L, -1);
					} else if (key == "wrap_s") {
						wrap_s = (GLenum)lua_tonumber(L, -1);
					} else if (key == "wrap_t") {
						wrap_t = (GLenum)lua_tonumber(L, -1);
					} else if (key == "wrap_r") {
						wrap_r = (GLenum)lua_tonumber(L, -1);
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

	
	LuaTextureObj** texPtr =
		(LuaTextureObj**)lua_newuserdata(L, sizeof(LuaTextureObj*));
	luaL_getmetatable(L, metaName);
	lua_setmetatable(L, -2);

	*texPtr = new LuaTextureObj(target, format,
	                            xsize, ysize, border,
	                            min_filter, mag_filter,
	                            wrap_s, wrap_t, wrap_r,
	                            aniso);

	return 1;
}


/******************************************************************************/
/******************************************************************************/

inline LuaTexture*& LuaTextureMgr::CheckLuaTexture(lua_State* L, int index)
{
	return *((LuaTexture**)luaL_checkudata(L, index, metaName));
}


/******************************************************************************/
/******************************************************************************/

bool LuaTextureMgr::CreateMetatable(lua_State* L)
{
  luaL_newmetatable(L, metaName);
  HSTR_PUSH_CFUNC(L, "__gc",    MetaGC);
  HSTR_PUSH_CFUNC(L, "__index", MetaIndex);
  lua_pop(L, 1);  
  return true;
}


int LuaTextureMgr::MetaGC(lua_State* L)
{
	LuaTexture*& texture = CheckLuaTexture(L, 1);
	delete texture;
	texture = NULL;
	return 0;
}


int LuaTextureMgr::MetaIndex(lua_State* L)
{
	LuaTexture* texture = CheckLuaTexture(L, 1);
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
	else if (key == "aniso")  { lua_pushnumber(L, texture->GetAniso());   }
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


/******************************************************************************/
/******************************************************************************/

int LuaTextureMgr::DeleteTexture(lua_State* L)
{
	if (lua_isnil(L, 1)) {
		return 0;
	}
	LuaTexture*& texture = CheckLuaTexture(L, 1);
	delete texture;
	texture = NULL;
	return 0;
}


int LuaTextureMgr::Texture(lua_State* L) // FIXME -- multitex / boolean control
{
	LuaOpenGL::CheckDrawingEnabled(L, __FUNCTION__);

	bool success = false;

	int texIndex = 1;
	GLenum texUnit = GL_TEXTURE0;
	if (lua_israwnumber(L, 1)) {
		const int texNum = lua_toint(L, 1);
		if ((texNum < 0) || (texNum >= MAX_TEXTURE_UNITS)) {
			luaL_error(L, "Bad texture unit passed to gl.BindTexture()");
			return 0;
		}
		texUnit = GL_TEXTURE0 + texNum;
		if (texUnit != GL_TEXTURE0) {
			glActiveTexture(texUnit);
		}
		texIndex++;
	}

	if (lua_isboolean(L, texIndex)) {
		// FIXME - target mode?
		if (lua_tobool(L, texIndex)) {
			glEnable(GL_TEXTURE_2D);
		} else {
			glDisable(GL_TEXTURE_2D);
		}
		success = true;
	}
	else if (lua_israwstring(L, texIndex)) {
		const string name = lua_tostring(L, texIndex);
		const int texNum = TEXMGR.getTextureID(name.c_str(), false);
		if (texNum >= 0) {
			const ImageInfo& ii = TEXMGR.getInfo(texNum);
			if (ii.texture != NULL) {
				if (ii.texture->execute()) {
					glEnable(GL_TEXTURE_2D);
					lua_pushboolean(L, true);
					success = true;
				}
			}
		}
	}
	else {
		LuaTexture* texture = CheckLuaTexture(L, texIndex);
		if (texture != NULL) {
			if (lua_isboolean(L, texIndex + 1)) {
				if (!lua_tobool(L, texIndex + 1)) {
					if (texture->IsValid()) {
						glDisable(texture->GetTarget());
						success = true;
					}
				}
			}
			else {
				if (texture->Bind()) {
					glEnable(texture->GetTarget());
					success = true;
				}
			}
		}
	}

	if (texUnit != GL_TEXTURE0) {
		glActiveTexture(GL_TEXTURE0);
	}

	lua_pushboolean(L, success);

	return 1;
}


int LuaTextureMgr::TexParameter(lua_State* L)
{
	//LuaOpenGL::CheckDrawingEnabled(L, __FUNCTION__);

	LuaTexture* texture = CheckLuaTexture(L, 1);
	if (texture == NULL) {
		return 0;
	}
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

	LuaTexture* texture = CheckLuaTexture(L, 1);
	if (texture == NULL) {
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

	LuaTexture* texture = CheckLuaTexture(L, 1);
	if ((texture == NULL) || !texture->IsWritable()) {
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


/******************************************************************************/

bool LuaTextureMgr::InsertTexture(LuaTexture* texture)
{
	if (textures.find(texture) != textures.end()) {
		return false;
	}
	textures.insert(texture);
	return true;
}


bool LuaTextureMgr::RemoveTexture(LuaTexture* texture)
{
	set<LuaTexture*>::iterator it = textures.find(texture);
	if (it == textures.end()) {
		return false;
	}
	textures.erase(it);
	return true;
}


void LuaTextureMgr::InitContext()
{
}


void LuaTextureMgr::FreeContext()
{
}


void LuaTextureMgr::StaticInitContext(void* data)
{
	((LuaTextureMgr*)data)->InitContext();
}


void LuaTextureMgr::StaticFreeContext(void* data)
{
	((LuaTextureMgr*)data)->FreeContext();
}


/******************************************************************************/
/******************************************************************************/
