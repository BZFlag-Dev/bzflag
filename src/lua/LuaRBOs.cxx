
#include "common.h"

// implementation header
#include "LuaRBOs.h"

// system headers
#include <string>
#include <set>
using std::string;
using std::set;

// common headers
#include "bzfgl.h"
#include "bzfio.h"
#include "OpenGLGState.h"

// local headers
#include "LuaInclude.h"
#include "LuaHashString.h"
#include "LuaUtils.h"


const char* LuaRBOMgr::metaName = "RBO";

LuaRBOMgr luaRBOMgr;


/******************************************************************************/
/******************************************************************************/

LuaRBOMgr::LuaRBOMgr()
{
}


LuaRBOMgr::~LuaRBOMgr()
{
	set<LuaRBO*>::const_iterator it;
	for (it = rbos.begin(); it != rbos.end(); ++it) {
		const LuaRBO* rbo = *it;
		glDeleteRenderbuffersEXT(1, &rbo->id);
	}
}


/******************************************************************************/
/******************************************************************************/

bool LuaRBOMgr::PushEntries(lua_State* L)
{
	CreateMetatable(L);

	PUSH_LUA_CFUNC(L, CreateRBO);
	PUSH_LUA_CFUNC(L, DeleteRBO);

	return true;
}


bool LuaRBOMgr::CreateMetatable(lua_State* L)
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

const LuaRBO* LuaRBOMgr::GetLuaRBO(lua_State* L, int index)
{
	return (LuaRBO*)LuaUtils::TestUserData(L, index, metaName);
}


/******************************************************************************/
/******************************************************************************/

void LuaRBO::Init()
{
	id     = 0;
	target = GL_RENDERBUFFER_EXT;
	format = GL_RGBA;
	xsize  = 0;
	ysize  = 0;

	OpenGLGState::registerContextInitializer(StaticFreeContext,
	                                         StaticInitContext, this);
}


void LuaRBO::Free()
{
	OpenGLGState::unregisterContextInitializer(StaticFreeContext,
	                                           StaticInitContext, this);
	if (id == 0) {
		return;
	}

	glDeleteRenderbuffersEXT(1, &id);
	id = 0;
}


void LuaRBO::InitContext()
{
}


void LuaRBO::FreeContext()
{
	if (id != 0) {
		glDeleteRenderbuffersEXT(1, &id);
		id = 0;
	}
}


void LuaRBO::StaticInitContext(void* data)
{
	((LuaRBO*)data)->InitContext();
}


void LuaRBO::StaticFreeContext(void* data)
{
	((LuaRBO*)data)->FreeContext();
}


/******************************************************************************/
/******************************************************************************/

int LuaRBOMgr::meta_gc(lua_State* L)
{
	LuaRBO* rbo = (LuaRBO*)luaL_checkudata(L, 1, metaName);
	rbo->Free();
	return 0;
}


int LuaRBOMgr::meta_index(lua_State* L)
{
	const LuaRBO* rbo = (LuaRBO*)luaL_checkudata(L, 1, metaName);
	const string key = luaL_checkstring(L, 2);
	if (key == "valid") {
		lua_pushboolean(L, glIsRenderbufferEXT(rbo->id));
	}
	else if (key == "target") { lua_pushnumber(L, rbo->target); }
	else if (key == "format") { lua_pushnumber(L, rbo->format); }
	else if (key == "xsize")  { lua_pushnumber(L, rbo->xsize);  }
	else if (key == "ysize")  { lua_pushnumber(L, rbo->ysize);  }
	else {
		return 0;
	}
	return 1;
}


int LuaRBOMgr::meta_newindex(lua_State* /*L*/)
{
	return 0;
}


/******************************************************************************/
/******************************************************************************/

int LuaRBOMgr::CreateRBO(lua_State* L)
{
	LuaRBO rbo;
	rbo.Init();

	rbo.xsize = (GLsizei)luaL_checknumber(L, 1);
	rbo.ysize = (GLsizei)luaL_checknumber(L, 2);
	rbo.target = GL_RENDERBUFFER_EXT;
	rbo.format = GL_RGBA;

	const int table = 3;
	if (lua_istable(L, table)) {
		lua_getfield(L, table, "target");
		if (lua_israwnumber(L, -1)) {
			rbo.target = (GLenum)lua_toint(L, -1);
		}
		lua_pop(L, 1);
		lua_getfield(L, table, "format");
		if (lua_israwnumber(L, -1)) {
			rbo.format = (GLenum)lua_toint(L, -1);
		}
		lua_pop(L, 1);
	}

	glGenRenderbuffersEXT(1, &rbo.id);
	glBindRenderbufferEXT(rbo.target, rbo.id);

	// allocate the memory
	glRenderbufferStorageEXT(rbo.target, rbo.format, rbo.xsize, rbo.ysize);
	
	glBindRenderbufferEXT(rbo.target, 0);

	LuaRBO* rboPtr = (LuaRBO*)lua_newuserdata(L, sizeof(LuaRBO));
	*rboPtr = rbo;

	luaL_getmetatable(L, metaName);
	lua_setmetatable(L, -2);

	return 1;
}


int LuaRBOMgr::DeleteRBO(lua_State* L)
{
	if (lua_isnil(L, 1)) {
		return 0;
	}
	LuaRBO* rbo = (LuaRBO*)luaL_checkudata(L, 1, metaName);
	rbo->Free();
	return 0;
}


/******************************************************************************/
/******************************************************************************/
