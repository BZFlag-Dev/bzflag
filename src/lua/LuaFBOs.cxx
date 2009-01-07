
#include "common.h"

// implementation header
#include "LuaFBOs.h"

// system headers
#include <string>
#include <vector>
#include <set>
#include <map>
using std::string;
using std::vector;
using std::set;
using std::map;

// common headers
#include "bzfgl.h"
#include "bzfio.h"
#include "OpenGLGState.h"

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


/******************************************************************************/
/******************************************************************************/

LuaFBOMgr::LuaFBOMgr()
{
}


LuaFBOMgr::~LuaFBOMgr()
{
	set<LuaFBO*>::const_iterator it;
	for (it = fbos.begin(); it != fbos.end(); ++it) {
		const LuaFBO* fbo = *it;
		glDeleteFramebuffersEXT(1, &fbo->id);
	}
}


/******************************************************************************/
/******************************************************************************/

bool LuaFBOMgr::PushEntries(lua_State* L)
{
	CreateMetatable(L);

	PUSH_LUA_CFUNC(L, CreateFBO);
	PUSH_LUA_CFUNC(L, DeleteFBO);
	PUSH_LUA_CFUNC(L, IsValidFBO);
	PUSH_LUA_CFUNC(L, ActiveFBO);
	PUSH_LUA_CFUNC(L, UnsafeSetFBO);
	if (GLEW_EXT_framebuffer_blit) {
		PUSH_LUA_CFUNC(L, BlitFBO);
	}

	return true;
}


bool LuaFBOMgr::CreateMetatable(lua_State* L)
{
	luaL_newmetatable(L, metaName);
	HSTR_PUSH_CFUNC(L, "__gc",        meta_gc);
	HSTR_PUSH_CFUNC(L, "__index",     meta_index);
	HSTR_PUSH_CFUNC(L, "__newindex",  meta_newindex);
	lua_pop(L, 1);
	return true;
}


/******************************************************************************/
/******************************************************************************/

const LuaFBO* LuaFBOMgr::GetLuaFBO(lua_State* L, int index)
{
	return (LuaFBO*)LuaUtils::TestUserData(L, index, metaName);
}


/******************************************************************************/
/******************************************************************************/

void LuaFBO::Init(lua_State* /*L*/)
{
	id     = 0;
	target = GL_FRAMEBUFFER_EXT;
	luaRef = LUA_NOREF;
	OpenGLGState::registerContextInitializer(StaticFreeContext,
	                                         StaticInitContext, this);
}


void LuaFBO::Free(lua_State* L)
{
	OpenGLGState::unregisterContextInitializer(StaticFreeContext,
	                                           StaticInitContext, this);

	if (luaRef == LUA_NOREF) {
		return;
	}
	luaL_unref(L, LUA_REGISTRYINDEX, luaRef);
	luaRef = LUA_NOREF;

	glDeleteFramebuffersEXT(1, &id);
	id = 0;
}


void LuaFBO::InitContext()
{
}


void LuaFBO::FreeContext()
{
	if (id != 0) {
		glDeleteFramebuffersEXT(1, &id);
		id = 0;
	}
}


void LuaFBO::StaticInitContext(void* data)
{
	((LuaFBO*)data)->InitContext();
}


void LuaFBO::StaticFreeContext(void* data)
{
	((LuaFBO*)data)->FreeContext();
}


/******************************************************************************/
/******************************************************************************/

int LuaFBOMgr::meta_gc(lua_State* L)
{
	LuaFBO* fbo = (LuaFBO*)luaL_checkudata(L, 1, metaName);
	fbo->Free(L);
	return 0;
}


int LuaFBOMgr::meta_index(lua_State* L)
{
	const LuaFBO* fbo = (LuaFBO*)luaL_checkudata(L, 1, metaName);
	if (fbo->luaRef == LUA_NOREF) {
		return 0;
	}

	// read the value from the ref table
	lua_rawgeti(L, LUA_REGISTRYINDEX, fbo->luaRef);
	lua_pushvalue(L, 2);
	lua_rawget(L, -2);

	return 1;
}


int LuaFBOMgr::meta_newindex(lua_State* L)
{
	LuaFBO* fbo = (LuaFBO*)luaL_checkudata(L, 1, metaName);
	if (fbo->luaRef == LUA_NOREF) {
		return 0;
	}

	if (lua_israwstring(L, 2)) {
		const string key = lua_tostring(L, 2);
		const GLenum type = ParseAttachment(key);
		if (type != 0) {
			GLint currentFBO;
			glGetIntegerv(GL_FRAMEBUFFER_BINDING_EXT, &currentFBO);
			glBindFramebufferEXT(fbo->target, fbo->id);
			ApplyAttachment(L, 3, fbo, type);
			glBindFramebufferEXT(fbo->target, currentFBO);
		}
		else if (key == "drawbuffers") {
			GLint currentFBO;
			glGetIntegerv(GL_FRAMEBUFFER_BINDING_EXT, &currentFBO);
			glBindFramebufferEXT(fbo->target, fbo->id);
			ApplyDrawBuffers(L, 3);
			glBindFramebufferEXT(fbo->target, currentFBO);
		}
		else if (key == "readbuffer") {
			GLint currentFBO;
			glGetIntegerv(GL_FRAMEBUFFER_BINDING_EXT, &currentFBO);
			glBindFramebufferEXT(fbo->target, fbo->id);
			if (lua_israwnumber(L, 3)) {
				const GLenum buffer = (GLenum)lua_tonumber(L, 3);
				glReadBuffer(buffer);
			}
			glBindFramebufferEXT(fbo->target, currentFBO);
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


/******************************************************************************/
/******************************************************************************/

static GLenum GetBindingEnum(GLenum target)
{
	switch (target) {
		case GL_FRAMEBUFFER_EXT:      { return GL_FRAMEBUFFER_BINDING_EXT;      }
		case GL_DRAW_FRAMEBUFFER_EXT: { return GL_DRAW_FRAMEBUFFER_BINDING_EXT; }
		case GL_READ_FRAMEBUFFER_EXT: { return GL_READ_FRAMEBUFFER_BINDING_EXT; }
		default: {
			return 0;
		}
	}
}


/******************************************************************************/
/******************************************************************************/

GLenum LuaFBOMgr::ParseAttachment(const string& name)
{
	static map<string, GLenum> attachMap;
	if (attachMap.empty()) {
		attachMap["depth"]   = GL_DEPTH_ATTACHMENT_EXT; 
		attachMap["stencil"] = GL_STENCIL_ATTACHMENT_EXT;
		attachMap["color0"]  = GL_COLOR_ATTACHMENT0_EXT;
		attachMap["color1"]  = GL_COLOR_ATTACHMENT1_EXT;
		attachMap["color2"]  = GL_COLOR_ATTACHMENT2_EXT;
		attachMap["color3"]  = GL_COLOR_ATTACHMENT3_EXT;
		attachMap["color4"]  = GL_COLOR_ATTACHMENT4_EXT;
		attachMap["color5"]  = GL_COLOR_ATTACHMENT5_EXT;
		attachMap["color6"]  = GL_COLOR_ATTACHMENT6_EXT;
		attachMap["color7"]  = GL_COLOR_ATTACHMENT7_EXT;
		attachMap["color8"]  = GL_COLOR_ATTACHMENT8_EXT;
		attachMap["color9"]  = GL_COLOR_ATTACHMENT9_EXT;
		attachMap["color10"] = GL_COLOR_ATTACHMENT10_EXT;
		attachMap["color11"] = GL_COLOR_ATTACHMENT11_EXT;
		attachMap["color12"] = GL_COLOR_ATTACHMENT12_EXT;
		attachMap["color13"] = GL_COLOR_ATTACHMENT13_EXT;
		attachMap["color14"] = GL_COLOR_ATTACHMENT14_EXT;
		attachMap["color15"] = GL_COLOR_ATTACHMENT15_EXT;
	}
	map<string, GLenum>::const_iterator it = attachMap.find(name);
	if (it != attachMap.end()) {
		return it->second;
	}
	return 0;
}


/******************************************************************************/
/******************************************************************************/

bool LuaFBOMgr::AttachObject(lua_State* L, int index,
		                         LuaFBO* fbo, GLenum attachID,
		                         GLenum attachTarget, GLenum attachLevel)
{
	if (lua_isnil(L, index)) {
		// nil object
		glFramebufferTexture2DEXT(fbo->target, attachID,
		                          GL_TEXTURE_2D, 0, 0);
		glFramebufferRenderbufferEXT(fbo->target, attachID,
		                             GL_RENDERBUFFER_EXT, 0);
		return true;
	}

	LuaTexture** texPtr = (LuaTexture**)LuaUtils::TestUserData(L, index, LuaTextureMgr::metaName);
	if (texPtr != NULL) {
		const LuaTexture* tex = *texPtr;
		if ((tex == NULL) || !tex->IsWritable()) {
			return false;
		}
		if (attachTarget == 0) {
			attachTarget = tex->GetTarget();
		} 
		glFramebufferTexture2DEXT(fbo->target, attachID, attachTarget,
		                          tex->GetTexID(), attachLevel);
		fbo->xsize = tex->GetSizeX();
		fbo->ysize = tex->GetSizeY();
		return true;
	}

	const LuaRBO* rbo =
		(LuaRBO*)LuaUtils::TestUserData(L, index, LuaRBOMgr::metaName);
	if (rbo != NULL) {
		if (attachTarget == 0) {
			attachTarget = rbo->target;
		} 
		glFramebufferRenderbufferEXT(fbo->target,
																 attachID, attachTarget, rbo->id);
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
	if (lua_israwnumber(L, -1)) { target = (GLenum)lua_tonumber(L, -1); }
	lua_pop(L, 1);

	lua_rawgeti(L, table, 3);
	if (lua_israwnumber(L, -1)) { level = (GLint)lua_tonumber(L, -1); }
	lua_pop(L, 1);

	lua_rawgeti(L, table, 1);
	const bool success = AttachObject(L, -1, fbo, attachID, target, level);
	lua_pop(L, 1);

	return success;
}


bool LuaFBOMgr::ApplyDrawBuffers(lua_State* L, int index)
{
	if (lua_israwnumber(L, index)) {
		const GLenum buffer = (GLenum)lua_tonumber(L, index);
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

		glDrawBuffersARB(buffers.size(), bufArray);

		delete[] bufArray;

		return true;
	}
	return false;
}


/******************************************************************************/
/******************************************************************************/

int LuaFBOMgr::CreateFBO(lua_State* L)
{
	LuaFBO fbo;
	fbo.Init(L);

	const int table = 1;
/*
	if (lua_istable(L, table)) {
		lua_getfield(L, table, "target");
		if (lua_israwnumber(L, -1)) {
			fbo.target = (GLenum)lua_tonumber(L, -1);
		} else {
			lua_pop(L, 1);
		}
	}
*/
	const GLenum bindTarget = GetBindingEnum(fbo.target);
	if (bindTarget == 0) {
		return 0;
	}

	// maintain a lua table to hold RBO references
 	lua_newtable(L);
	fbo.luaRef = luaL_ref(L, LUA_REGISTRYINDEX);
	if (fbo.luaRef == LUA_NOREF) {
		return 0;
	}

	GLint currentFBO;
	glGetIntegerv(bindTarget, &currentFBO);

	glGenFramebuffersEXT(1, &fbo.id);
	glBindFramebufferEXT(fbo.target, fbo.id);


	LuaFBO* fboPtr = (LuaFBO*)lua_newuserdata(L, sizeof(LuaFBO));
	*fboPtr = fbo;

	luaL_getmetatable(L, metaName);
	lua_setmetatable(L, -2);

	// parse the initialization table
	if (lua_istable(L, table)) {
		for (lua_pushnil(L); lua_next(L, table) != 0; lua_pop(L, 1)) {
			if (lua_israwstring(L, -2)) {
				const string key = lua_tostring(L, -2);
				const GLenum type = ParseAttachment(key);
				if (type != 0) {
					ApplyAttachment(L, -1, fboPtr, type);
				}
				else if (key == "drawbuffers") {
					ApplyDrawBuffers(L, -1);
				}
			}
		}
	}

	// revert to the old fbo
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, currentFBO);

	return 1;
}


int LuaFBOMgr::DeleteFBO(lua_State* L)
{
	if (lua_isnil(L, 1)) {
		return 0;
	}
	LuaFBO* fbo = (LuaFBO*)luaL_checkudata(L, 1, metaName);
	fbo->Free(L);
	return 0;
}


int LuaFBOMgr::IsValidFBO(lua_State* L)
{
	if (lua_isnil(L, 1) || !lua_isuserdata(L, 1)) {
		lua_pushboolean(L, false);
		return 1;
	}
	LuaFBO* fbo = (LuaFBO*)luaL_checkudata(L, 1, metaName);
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
	glBindFramebufferEXT(target, fbo->id);
	const GLenum status = glCheckFramebufferStatusEXT(target);
	glBindFramebufferEXT(target, currentFBO);

	const bool valid = (status == GL_FRAMEBUFFER_COMPLETE_EXT);
	lua_pushboolean(L, valid);
	lua_pushnumber(L, status);
	return 2;
}


int LuaFBOMgr::ActiveFBO(lua_State* L)
{
	LuaOpenGL::CheckDrawingEnabled(L, __FUNCTION__);
	
	LuaFBO* fbo = (LuaFBO*)luaL_checkudata(L, 1, metaName);
	if (fbo->id == 0) {
		return 0;
	}

	int funcIndex = 2;

	// target and matrix manipulation options
	GLenum target = fbo->target;
	if (lua_israwnumber(L, funcIndex)) {
		target = (GLenum)lua_tonumber(L, funcIndex);
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

	glPushAttrib(GL_VIEWPORT_BIT);
	glViewport(0, 0, fbo->xsize, fbo->ysize);
	if (identities) {
		glMatrixMode(GL_PROJECTION); glPushMatrix(); glLoadIdentity();
		glMatrixMode(GL_MODELVIEW);  glPushMatrix(); glLoadIdentity();
	}
	GLint currentFBO;
	glGetIntegerv(bindTarget, &currentFBO);
	glBindFramebufferEXT(target, fbo->id);

	const int error = lua_pcall(L, (args - funcIndex), 0, 0);

	glBindFramebufferEXT(target, currentFBO);
	if (identities) {
		glMatrixMode(GL_PROJECTION); glPopMatrix();
		glMatrixMode(GL_MODELVIEW);  glPopMatrix();
	}
	glPopAttrib();

	if (error != 0) {
		LuaLog("gl.ActiveFBO: error(%i) = %s", error, lua_tostring(L, -1));
		lua_error(L);
	}

	return 0;
}


int LuaFBOMgr::UnsafeSetFBO(lua_State* L)
{
	//LuaOpenGL::CheckDrawingEnabled(L, __FUNCTION__);

	if (lua_isboolean(L, 1) && !lua_tobool(L, 1)) {
		const GLenum target = (GLenum)luaL_optnumber(L, 2, GL_FRAMEBUFFER_EXT);
		glBindFramebufferEXT(target, 0);
		return 0;
	}
		
	LuaFBO* fbo = (LuaFBO*)luaL_checkudata(L, 1, metaName);
	if (fbo->id == 0) {
		return 0;
	}
	const GLenum target = (GLenum)luaL_optnumber(L, 2, fbo->target);
	glBindFramebufferEXT(target, fbo->id);
	return 0;
}


/******************************************************************************/

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

		glBlitFramebufferEXT(x0Src, y0Src, x1Src, y1Src,
												 x0Dst, y0Dst, x1Dst, y1Dst,
												 mask, filter);
	}
	else {
		LuaFBO* fboSrc = (LuaFBO*)luaL_checkudata(L, 1, metaName);
		if (fboSrc->id == 0) { return 0; }
		const GLint x0Src = (GLint)luaL_checknumber(L, 2);
		const GLint y0Src = (GLint)luaL_checknumber(L, 3);
		const GLint x1Src = (GLint)luaL_checknumber(L, 4);
		const GLint y1Src = (GLint)luaL_checknumber(L, 5);

		LuaFBO* fboDst = (LuaFBO*)luaL_checkudata(L, 6, metaName);
		if (fboDst->id == 0) { return 0; }
		const GLint x0Dst = (GLint)luaL_checknumber(L, 7);
		const GLint y0Dst = (GLint)luaL_checknumber(L, 8);
		const GLint x1Dst = (GLint)luaL_checknumber(L, 9);
		const GLint y1Dst = (GLint)luaL_checknumber(L, 10);

		const GLbitfield mask = (GLbitfield)luaL_optint(L, 11, GL_COLOR_BUFFER_BIT);
		const GLenum filter = (GLenum)luaL_optint(L, 12, GL_NEAREST);

		GLint currentFBO;
		glGetIntegerv(GL_FRAMEBUFFER_BINDING_EXT, &currentFBO);
		
		glBindFramebufferEXT(GL_READ_FRAMEBUFFER_EXT, fboSrc->id);
		glBindFramebufferEXT(GL_DRAW_FRAMEBUFFER_EXT, fboDst->id);

		glBlitFramebufferEXT(x0Src, y0Src, x1Src, y1Src,
												 x0Dst, y0Dst, x1Dst, y1Dst,
												 mask, filter);

		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, currentFBO);
	}

	return 0;
}


/******************************************************************************/
/******************************************************************************/
