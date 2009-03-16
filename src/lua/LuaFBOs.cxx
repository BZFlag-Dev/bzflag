
#include "common.h"

// interface header
#include "LuaFBOs.h"

// system headers
#include <string>
#include <vector>
#include <map>
using std::string;
using std::vector;
using std::map;

// common headers
#include "bzfgl.h"
#include "bzfio.h"
#include "OpenGLGState.h"
#include "OpenGLPassState.h"

// local headers
#include "LuaInclude.h"

#include "LuaHandle.h"
#include "LuaHashString.h"
#include "LuaUtils.h"

#include "LuaOpenGL.h"
#include "LuaRBOs.h"
#include "LuaTextures.h"


const char* LuaFBOMgr::metaName = "FBO";

LuaFBOMgr luaFBOMgr;


//============================================================================//
//============================================================================//

LuaFBO::LuaFBO(const LuaFBOData& fboData)
: LuaFBOData(fboData)
{
	OpenGLGState::registerContextInitializer(StaticFreeContext,
	                                         StaticInitContext, this);
}


LuaFBO::~LuaFBO()
{
	FreeContext();
	OpenGLGState::unregisterContextInitializer(StaticFreeContext,
	                                           StaticInitContext, this);
}


void LuaFBO::Delete(lua_State* L)
{
	FreeContext();

	if (luaRef == LUA_NOREF) {
		return;
	}
	luaL_unref(L, LUA_REGISTRYINDEX, luaRef);
	luaRef = LUA_NOREF;
}


void LuaFBO::InitContext()
{
}


void LuaFBO::FreeContext()
{
	if (id == 0) {
		return;
	}
	glDeleteFramebuffers(1, &id);
	id = 0;
}


void LuaFBO::StaticInitContext(void* data)
{
	((LuaFBO*)data)->InitContext();
}


void LuaFBO::StaticFreeContext(void* data)
{
	((LuaFBO*)data)->FreeContext();
}


//============================================================================//
//============================================================================//

LuaFBOMgr::LuaFBOMgr()
{
}


LuaFBOMgr::~LuaFBOMgr()
{
}


//============================================================================//
//============================================================================//

bool LuaFBOMgr::PushEntries(lua_State* L)
{
	CreateMetatable(L);

	PUSH_LUA_CFUNC(L, CreateFBO);
	PUSH_LUA_CFUNC(L, DeleteFBO);
	PUSH_LUA_CFUNC(L, IsValidFBO);
	PUSH_LUA_CFUNC(L, ActiveFBO);
	PUSH_LUA_CFUNC(L, UnsafeSetFBO);
	PUSH_LUA_CFUNC(L, BlitFBO);

	return true;
}


bool LuaFBOMgr::CreateMetatable(lua_State* L)
{
	luaL_newmetatable(L, metaName);
	HSTR_PUSH_CFUNC(L,  "__gc",        MetaGC);
	HSTR_PUSH_CFUNC(L,  "__index",     MetaIndex);
	HSTR_PUSH_CFUNC(L,  "__newindex",  MetaNewindex);
	HSTR_PUSH_STRING(L, "__metatable", "no access");
	lua_pop(L, 1);
	return true;
}


//============================================================================//
//============================================================================//

const LuaFBO* LuaFBOMgr::TestLuaFBO(lua_State* L, int index)
{
	if (lua_getuserdataextra(L, index) != metaName) {
		return NULL;
	}
	return (LuaFBO*)lua_touserdata(L, index);
}


const LuaFBO* LuaFBOMgr::CheckLuaFBO(lua_State* L, int index)
{
	if (lua_getuserdataextra(L, index) != metaName) {
		luaL_argerror(L, index, "expected FBO");
	}
	return (LuaFBO*)lua_touserdata(L, index);
}


LuaFBO* LuaFBOMgr::GetLuaFBO(lua_State* L, int index)
{
	if (lua_getuserdataextra(L, index) != metaName) {
		luaL_argerror(L, index, "expected FBO");
	}
	return (LuaFBO*)lua_touserdata(L, index);
}


//============================================================================//
//============================================================================//

int LuaFBOMgr::MetaGC(lua_State* L)
{
	LuaFBO* fbo = GetLuaFBO(L, 1);
	fbo->Delete(L);
	fbo->~LuaFBO();
	return 0;
}


int LuaFBOMgr::MetaIndex(lua_State* L)
{
	const LuaFBO* fbo = CheckLuaFBO(L, 1);
	if (fbo->luaRef == LUA_NOREF) {
		return 0;
	}

	// read the value from the ref table
	lua_rawgeti(L, LUA_REGISTRYINDEX, fbo->luaRef);
	lua_pushvalue(L, 2);
	lua_rawget(L, -2);

	return 1;
}


int LuaFBOMgr::MetaNewindex(lua_State* L)
{
	LuaFBO* fbo = GetLuaFBO(L, 1);
	if (fbo->luaRef == LUA_NOREF) {
		return 0;
	}

	if (lua_israwstring(L, 2)) {
		const string key = lua_tostring(L, 2);
		const GLenum type = ParseAttachment(key);
		if (type != 0) {
			GLint currentFBO;
			glGetIntegerv(GL_FRAMEBUFFER_BINDING, &currentFBO);
			glBindFramebuffer(fbo->target, fbo->id);
			ApplyAttachment(L, 3, fbo, type);
			glBindFramebuffer(fbo->target, currentFBO);
		}
		else if (key == "drawBuffers") {
			GLint currentFBO;
			glGetIntegerv(GL_FRAMEBUFFER_BINDING, &currentFBO);
			glBindFramebuffer(fbo->target, fbo->id);
			ApplyDrawBuffers(L, 3);
			glBindFramebuffer(fbo->target, currentFBO);
		}
		else if (key == "readBuffer") {
			GLint currentFBO;
			glGetIntegerv(GL_FRAMEBUFFER_BINDING, &currentFBO);
			glBindFramebuffer(fbo->target, fbo->id);
			if (lua_israwnumber(L, 3)) {
				const GLenum buffer = (GLenum)lua_tointeger(L, 3);
				glReadBuffer(buffer);
			}
			glBindFramebuffer(fbo->target, currentFBO);
		}
		else if (key == "target") {
			return 0;// fbo->target = (GLenum)luaL_checkint(L, 3);
		}
	}

	// set the key/value in the ref table
	lua_rawgeti(L, LUA_REGISTRYINDEX, fbo->luaRef);
	lua_pushvalue(L, 2);
	lua_pushvalue(L, 3);
	lua_rawset(L, -3);

	return 0;
}


//============================================================================//
//============================================================================//

static GLenum GetBindingEnum(GLenum target)
{
	switch (target) {
		case GL_FRAMEBUFFER:      { return GL_FRAMEBUFFER_BINDING;      }
		case GL_DRAW_FRAMEBUFFER: { return GL_DRAW_FRAMEBUFFER_BINDING; }
		case GL_READ_FRAMEBUFFER: { return GL_READ_FRAMEBUFFER_BINDING; }
		default: {
			return 0;
		}
	}
}


//============================================================================//
//============================================================================//

GLenum LuaFBOMgr::ParseAttachment(const string& name)
{
	static map<string, GLenum> attachMap;
	if (attachMap.empty()) {
		attachMap["depth"]   = GL_DEPTH_ATTACHMENT;
		attachMap["stencil"] = GL_STENCIL_ATTACHMENT;
		attachMap["color0"]  = GL_COLOR_ATTACHMENT0;
		attachMap["color1"]  = GL_COLOR_ATTACHMENT1;
		attachMap["color2"]  = GL_COLOR_ATTACHMENT2;
		attachMap["color3"]  = GL_COLOR_ATTACHMENT3;
		attachMap["color4"]  = GL_COLOR_ATTACHMENT4;
		attachMap["color5"]  = GL_COLOR_ATTACHMENT5;
		attachMap["color6"]  = GL_COLOR_ATTACHMENT6;
		attachMap["color7"]  = GL_COLOR_ATTACHMENT7;
		attachMap["color8"]  = GL_COLOR_ATTACHMENT8;
		attachMap["color9"]  = GL_COLOR_ATTACHMENT9;
		attachMap["color10"] = GL_COLOR_ATTACHMENT10;
		attachMap["color11"] = GL_COLOR_ATTACHMENT11;
		attachMap["color12"] = GL_COLOR_ATTACHMENT12;
		attachMap["color13"] = GL_COLOR_ATTACHMENT13;
		attachMap["color14"] = GL_COLOR_ATTACHMENT14;
		attachMap["color15"] = GL_COLOR_ATTACHMENT15;
	}
	map<string, GLenum>::const_iterator it = attachMap.find(name);
	if (it != attachMap.end()) {
		return it->second;
	}
	return 0;
}


//============================================================================//
//============================================================================//

bool LuaFBOMgr::AttachObject(lua_State* L, int index,
		                         LuaFBO* fbo, GLenum attachID,
		                         GLenum attachTarget, GLenum attachLevel)
{
	if (lua_isnil(L, index)) {
		// nil object
		glFramebufferTexture2D(fbo->target, attachID, GL_TEXTURE_2D, 0, 0);
		glFramebufferRenderbuffer(fbo->target, attachID, GL_RENDERBUFFER, 0);
		return true;
	}

	const LuaTexture* tex = LuaTextureMgr::TestLuaTexture(L, index);
	if (tex != NULL) {
		if (!tex->IsValid() || !tex->IsWritable()) {
			return false;
		}
		if (attachTarget == 0) {
			attachTarget = tex->GetTarget();
		}
		glFramebufferTexture2D(fbo->target, attachID, attachTarget,
		                       tex->GetTexID(), attachLevel);
		fbo->xsize = tex->GetSizeX();
		fbo->ysize = tex->GetSizeY();
		return true;
	}

	const LuaRBO* rbo = LuaRBOMgr::TestLuaRBO(L, index);
	if (rbo != NULL) {
		if (attachTarget == 0) {
			attachTarget = rbo->target;
		}
		glFramebufferRenderbuffer(fbo->target, attachID, attachTarget, rbo->id);
		fbo->xsize = rbo->xsize;
		fbo->ysize = rbo->ysize;
		return true;
	}

	return false;
}


bool LuaFBOMgr::ApplyAttachment(lua_State* L, int index,
                                LuaFBO* fbo, const GLenum attachID)
{
	if (attachID == 0) {
		return false;
	}

	if (!lua_istable(L, index)) {
		return AttachObject(L, index, fbo, attachID);
	}

	const int table = (index < 0) ? index : (lua_gettop(L) + index + 1);

	GLenum target = 0;
	GLint  level  = 0;

	lua_rawgeti(L, table, 2);
	if (lua_israwnumber(L, -1)) { target = (GLenum)lua_tointeger(L, -1); }
	lua_pop(L, 1);

	lua_rawgeti(L, table, 3);
	if (lua_israwnumber(L, -1)) { level = (GLint)lua_tointeger(L, -1); }
	lua_pop(L, 1);

	lua_rawgeti(L, table, 1);
	const bool success = AttachObject(L, -1, fbo, attachID, target, level);
	lua_pop(L, 1);

	return success;
}


bool LuaFBOMgr::ApplyDrawBuffers(lua_State* L, int index)
{
	if (lua_israwnumber(L, index)) {
		const GLenum buffer = (GLenum)lua_tointeger(L, index);
		glDrawBuffer(buffer);
		return true;
	}
	else if (lua_istable(L, index) && GLEW_ARB_draw_buffers) {
		const int table = (index > 0) ? index : (lua_gettop(L) + index + 1);

		vector<GLenum> buffers;
		for (int i = 1; lua_checkgeti(L, table, i) != 0; lua_pop(L, 1), i++) {
			const GLenum buffer = (GLenum)luaL_checkint(L, -1);
			buffers.push_back(buffer);
		}

		GLenum* bufArray = new GLenum[buffers.size()];
		for (int d = 0; d < (int)buffers.size(); d++) {
			bufArray[d] = buffers[d];
		}

		glDrawBuffers(buffers.size(), bufArray);

		delete[] bufArray;

		return true;
	}
	return false;
}


//============================================================================//
//============================================================================//

int LuaFBOMgr::CreateFBO(lua_State* L)
{
	LuaFBOData fboData;

	const int table = 1;

	if (lua_istable(L, table)) {
		lua_getfield(L, table, "target");
		if (lua_israwnumber(L, -1)) {
			fboData.target = (GLenum)lua_tointeger(L, -1);
		} else {
			lua_pop(L, 1);
		}
	}

	const GLenum bindTarget = GetBindingEnum(fboData.target);
	if (bindTarget == 0) {
		return 0;
	}

	// maintain a lua table to hold RBO references
	lua_newtable(L);
	fboData.luaRef = luaL_ref(L, LUA_REGISTRYINDEX);
	if (fboData.luaRef == LUA_NOREF) {
		return 0;
	}

	GLint currentFBO;
	glGetIntegerv(bindTarget, &currentFBO);

	glGenFramebuffers(1, &fboData.id);
	glBindFramebuffer(fboData.target, fboData.id);

	void* udData = lua_newuserdata(L, sizeof(LuaFBO));
	LuaFBO* fbo = new(udData) LuaFBO(fboData);

	lua_setuserdataextra(L, -1, (void*)metaName);
	luaL_getmetatable(L, metaName);
	lua_setmetatable(L, -2);

	// parse the initialization table
	if (lua_istable(L, table)) {
		for (lua_pushnil(L); lua_next(L, table) != 0; lua_pop(L, 1)) {
			if (lua_israwstring(L, -2)) {
				const string key = lua_tostring(L, -2);
				const GLenum type = ParseAttachment(key);
				if (type != 0) {
					ApplyAttachment(L, -1, fbo, type);
				}
				else if (key == "drawBuffers") {
					ApplyDrawBuffers(L, -1);
				}
			}
		}
	}

	// revert to the old fbo
	glBindFramebuffer(GL_FRAMEBUFFER, currentFBO);

	return 1;
}


int LuaFBOMgr::DeleteFBO(lua_State* L)
{
	if (OpenGLGState::isExecutingInitFuncs()) {
		luaL_error(L, "gl.DeleteFBO can not be used in GLReload");
	}
	if (lua_isnil(L, 1)) {
		return 0;
	}
	LuaFBO* fbo = GetLuaFBO(L, 1);
	fbo->Delete(L);
	return 0;
}


int LuaFBOMgr::IsValidFBO(lua_State* L)
{
	if (lua_isnil(L, 1) || !lua_isuserdata(L, 1)) {
		lua_pushboolean(L, false);
		return 1;
	}
	const LuaFBO* fbo = CheckLuaFBO(L, 1);
	if ((fbo->id == 0) || (fbo->luaRef == LUA_NOREF)) {
		lua_pushboolean(L, false);
		return 1;
	}

	const GLenum target = (GLenum)luaL_optint(L, 2, fbo->target);
	const GLenum bindTarget = GetBindingEnum(target);
	if (bindTarget == 0) {
		lua_pushboolean(L, false);
		return 1;
	}

	GLint currentFBO;
	glGetIntegerv(bindTarget, &currentFBO);
	glBindFramebuffer(target, fbo->id);
	const GLenum status = glCheckFramebufferStatus(target);
	glBindFramebuffer(target, currentFBO);

	const bool valid = (status == GL_FRAMEBUFFER_COMPLETE);
	lua_pushboolean(L, valid);
	lua_pushinteger(L, status);
	return 2;
}


int LuaFBOMgr::ActiveFBO(lua_State* L)
{
	LuaOpenGL::CheckDrawingEnabled(L, __FUNCTION__);

	const LuaFBO* fbo = CheckLuaFBO(L, 1);
	if (fbo->id == 0) {
		return 0;
	}

	int funcIndex = 2;

	// target and matrix manipulation options
	GLenum target = fbo->target;
	if (lua_israwnumber(L, funcIndex)) {
		target = (GLenum)lua_tointeger(L, funcIndex);
		funcIndex++;
	}
	bool identities = false;
	if (lua_isboolean(L, funcIndex)) {
		identities = lua_tobool(L, funcIndex);
		funcIndex++;
	}

	if (!lua_isfunction(L, funcIndex)) {
		luaL_error(L, "Incorrect arguments to gl.ActiveFBO()");
	}
	const int args = lua_gettop(L);

	const GLenum bindTarget = GetBindingEnum(target);
	if (bindTarget == 0) {
		return 0;
	}

	if (!OpenGLPassState::PushAttrib(GL_VIEWPORT_BIT)) {
		luaL_error(L, "attrib stack overflow");
	}

	glViewport(0, 0, fbo->xsize, fbo->ysize);
	if (identities) {
		glMatrixMode(GL_PROJECTION); glPushMatrix(); glLoadIdentity();
		glMatrixMode(GL_MODELVIEW);  glPushMatrix(); glLoadIdentity();
	}
	GLint currentFBO;
	glGetIntegerv(bindTarget, &currentFBO);
	glBindFramebuffer(target, fbo->id);

	const int error = lua_pcall(L, (args - funcIndex), 0, 0);

	glBindFramebuffer(target, currentFBO);
	if (identities) {
		glMatrixMode(GL_PROJECTION); glPopMatrix();
		glMatrixMode(GL_MODELVIEW);  glPopMatrix();
	}

	if (!OpenGLPassState::PopAttrib()) {
		luaL_error(L, "attrib stack underflow");
	}

	if (error != 0) {
		LuaLog(1, "gl.ActiveFBO: error(%i) = %s", error, lua_tostring(L, -1));
		lua_error(L);
	}

	return 0;
}


int LuaFBOMgr::UnsafeSetFBO(lua_State* L)
{
	//LuaOpenGL::CheckDrawingEnabled(L, __FUNCTION__);

	if (lua_isboolean(L, 1) && !lua_tobool(L, 1)) {
		const GLenum target = (GLenum)luaL_optnumber(L, 2, GL_FRAMEBUFFER);
		glBindFramebuffer(target, 0);
		return 0;
	}

	const LuaFBO* fbo = CheckLuaFBO(L, 1);
	if (fbo->id == 0) {
		return 0;
	}
	const GLenum target = (GLenum)luaL_optint(L, 2, fbo->target);
	glBindFramebuffer(target, fbo->id);
	return 0;
}


//============================================================================//

int LuaFBOMgr::BlitFBO(lua_State* L)
{
	if (lua_israwnumber(L, 1)) {
		const GLint x0Src = (GLint)luaL_checknumber(L, 1);
		const GLint y0Src = (GLint)luaL_checknumber(L, 2);
		const GLint x1Src = (GLint)luaL_checknumber(L, 3);
		const GLint y1Src = (GLint)luaL_checknumber(L, 4);

		const GLint x0Dst = (GLint)luaL_checknumber(L, 5);
		const GLint y0Dst = (GLint)luaL_checknumber(L, 6);
		const GLint x1Dst = (GLint)luaL_checknumber(L, 7);
		const GLint y1Dst = (GLint)luaL_checknumber(L, 8);

		const GLbitfield mask = (GLbitfield)luaL_optint(L, 9, GL_COLOR_BUFFER_BIT);
		const GLenum filter = (GLenum)luaL_optint(L, 10, GL_NEAREST);

		glBlitFramebuffer(x0Src, y0Src, x1Src, y1Src,
											x0Dst, y0Dst, x1Dst, y1Dst,
											mask, filter);
	}
	else {
		const LuaFBO* fboSrc = CheckLuaFBO(L, 1);
		if (fboSrc->id == 0) { return 0; }
		const GLint x0Src = (GLint)luaL_checknumber(L, 2);
		const GLint y0Src = (GLint)luaL_checknumber(L, 3);
		const GLint x1Src = (GLint)luaL_checknumber(L, 4);
		const GLint y1Src = (GLint)luaL_checknumber(L, 5);

		const LuaFBO* fboDst = CheckLuaFBO(L, 1);
		if (fboDst->id == 0) { return 0; }
		const GLint x0Dst = (GLint)luaL_checknumber(L, 7);
		const GLint y0Dst = (GLint)luaL_checknumber(L, 8);
		const GLint x1Dst = (GLint)luaL_checknumber(L, 9);
		const GLint y1Dst = (GLint)luaL_checknumber(L, 10);

		const GLbitfield mask = (GLbitfield)luaL_optint(L, 11, GL_COLOR_BUFFER_BIT);
		const GLenum filter = (GLenum)luaL_optint(L, 12, GL_NEAREST);

		GLint currentFBO;
		glGetIntegerv(GL_FRAMEBUFFER_BINDING, &currentFBO);

		glBindFramebuffer(GL_READ_FRAMEBUFFER, fboSrc->id);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fboDst->id);

		glBlitFramebuffer(x0Src, y0Src, x1Src, y1Src,
											x0Dst, y0Dst, x1Dst, y1Dst,
											mask, filter);

		glBindFramebuffer(GL_FRAMEBUFFER, currentFBO);
	}

	return 0;
}


//============================================================================//
//============================================================================//
