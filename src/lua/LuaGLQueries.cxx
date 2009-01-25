
#include "common.h"

// implementation header
#include "LuaGLQueries.h"

// system headers
#include <new>
#include <string>
using std::string;

// common headers
#include "bzfgl.h"
#include "OpenGLGState.h"
#include "OpenGLPassState.h"

// local headers
#include "LuaInclude.h"
#include "LuaHashString.h"
#include "LuaOpenGL.h"


LuaGLQueryMgr luaGLQueryMgr;

const char* LuaGLQueryMgr::metaName = "GLQuery";


/******************************************************************************/
/******************************************************************************/
//
//  LuaGLQuery
//

LuaGLQuery::LuaGLQuery(GLuint _id, GLenum _target)
: id(_id)
, target(_target)
{
	OpenGLGState::registerContextInitializer(StaticFreeContext,
	                                         StaticInitContext, this);
}


LuaGLQuery::~LuaGLQuery()
{
	FreeContext();
	OpenGLGState::unregisterContextInitializer(StaticFreeContext,
	                                           StaticInitContext, this);
}


bool LuaGLQuery::Delete()
{
	if (id == 0) {
		return false;
	}
	glDeleteQueries(1, &id);
	id = 0;
	return true;
}


void LuaGLQuery::InitContext()
{
}


void LuaGLQuery::FreeContext()
{
	if (id == 0) {
		return;
	}
	glDeleteQueries(1, &id);
	id = 0;
}


void LuaGLQuery::StaticInitContext(void* data)
{
	((LuaGLQuery*)data)->InitContext();
}


void LuaGLQuery::StaticFreeContext(void* data)
{
	((LuaGLQuery*)data)->FreeContext();
}


/******************************************************************************/
/******************************************************************************/
//
//  LuaGLQueryMgr
//

bool LuaGLQueryMgr::PushEntries(lua_State* L)
{
	CreateMetatable(L);

	PUSH_LUA_CFUNC(L, CreateQuery);
	PUSH_LUA_CFUNC(L, DeleteQuery);
	PUSH_LUA_CFUNC(L, RunQuery);
	PUSH_LUA_CFUNC(L, GetQuery);
	if (glBeginConditionalRender) {
		PUSH_LUA_CFUNC(L, BeginEndConditionalRender);
	}

	return true;
}


/******************************************************************************/
/******************************************************************************/

const LuaGLQuery* LuaGLQueryMgr::TestLuaGLQuery(lua_State* L, int index)
{
	if (lua_getuserdataextra(L, index) != metaName) {
		return NULL;
	}
	return (LuaGLQuery*)lua_touserdata(L, index);
}


const LuaGLQuery* LuaGLQueryMgr::CheckLuaGLQuery(lua_State* L, int index)
{
	if (lua_getuserdataextra(L, index) != metaName) {
		luaL_argerror(L, index, "expected GLQuery");
	}
	return (LuaGLQuery*)lua_touserdata(L, index);
}


LuaGLQuery* LuaGLQueryMgr::GetLuaGLQuery(lua_State* L, int index)
{
	if (lua_getuserdataextra(L, index) != metaName) {
		luaL_argerror(L, index, "expected GLQuery");
	}
	return (LuaGLQuery*)lua_touserdata(L, index);
}


/******************************************************************************/
/******************************************************************************/

bool LuaGLQueryMgr::CreateMetatable(lua_State* L)
{
	luaL_newmetatable(L, metaName);
	HSTR_PUSH_CFUNC(L,  "__gc",    MetaGC);
	HSTR_PUSH_CFUNC(L,  "__index", MetaIndex);
	HSTR_PUSH_STRING(L, "__metatable", "no access");
	lua_pop(L, 1);  
	return true;
}


int LuaGLQueryMgr::MetaGC(lua_State* L)
{
	LuaGLQuery* query = GetLuaGLQuery(L, 1);
	query->~LuaGLQuery();
	return 0;
}


int LuaGLQueryMgr::MetaIndex(lua_State* L)
{
	const LuaGLQuery* query = CheckLuaGLQuery(L, 1);
	if (query == NULL) {
		return 0;
	}

	if (!lua_israwstring(L, 2)) {
		return 0;
	}

	const string key = lua_tostring(L, 2);
	if (key == "valid") {
		lua_pushboolean(L, query->IsValid());
		return 1;
	}

	return 0;
}


/******************************************************************************/
/******************************************************************************/

int LuaGLQueryMgr::CreateQuery(lua_State* L)
{
	// generate the query id
	GLuint queryID;
	glGenQueries(1, &queryID);
	if (queryID == 0) {
		return 0;
	}

	GLenum target = (GLenum)luaL_optint(L, 1, GL_SAMPLES_PASSED);

	void* udData = lua_newuserdata(L, sizeof(LuaGLQuery));
	new(udData) LuaGLQuery(queryID, target);

	luaL_getmetatable(L, metaName);
	lua_setmetatable(L, -2);
	lua_setuserdataextra(L, -1, (void*)metaName);

	return 1;
}


int LuaGLQueryMgr::DeleteQuery(lua_State* L)
{
	if (OpenGLGState::isExecutingInitFuncs()) {
		luaL_error(L, "gl.DeleteQuery can not be used in GLReload");
	}
	if (lua_isnil(L, 1)) {
		return 0;
	}
	LuaGLQuery* query = GetLuaGLQuery(L, 1);
	query->Delete();
	return 0;
}


int LuaGLQueryMgr::RunQuery(lua_State* L)
{
	static bool running = false;

	if (running) {
		luaL_error(L, "not re-entrant");
	}

	const LuaGLQuery* query = CheckLuaGLQuery(L, 1);
	if (!query->IsValid()) {
		luaL_error(L, "invalid query object");
	}

	int funcArg = 2;

	GLenum target = query->GetTarget();
	if (lua_israwnumber(L, funcArg)) {
		target = (GLenum)lua_toint(L, funcArg);
		funcArg++;
	}

	if (!lua_isfunction(L, funcArg)) {
		luaL_error(L, "expecting a function");
	}
	const int args = lua_gettop(L); // number of arguments

	running = true;
	glBeginQuery(target, query->GetID());
	const int error = lua_pcall(L, (args - funcArg), 0, 0);
	glEndQuery(target);
	running = false;

	if (error != 0) {
		LuaLog(1, "gl.RunQuery: error(%i) = %s", error, lua_tostring(L, -1));
		lua_error(L);
	}

	return 0;
}


int LuaGLQueryMgr::GetQuery(lua_State* L)
{
	const LuaGLQuery* query = CheckLuaGLQuery(L, 1);
	if ((query == NULL) || !query->IsValid()) {
		luaL_error(L, "invalid query object");
	}

	GLuint count;
	glGetQueryObjectuiv(query->GetID(), GL_QUERY_RESULT, &count);

	lua_pushinteger(L, count);

	return 1;
}


int LuaGLQueryMgr::BeginEndConditionalRender(lua_State* L)
{
	LuaOpenGL::CheckDrawingEnabled(L, __FUNCTION__);

	const LuaGLQuery* query = CheckLuaGLQuery(L, 1);
	if (query == NULL) {
		return 0;
	}

	const GLenum mode = (GLenum)luaL_checkint(L, 2);

	if (!lua_isfunction(L, 3)) {
		luaL_error(L, "expecting a function");
	}

	glBeginConditionalRender(query->GetID(), mode);
	const int args = lua_gettop(L);
	const int error = lua_pcall(L, (args - 3), 0, 0);
	glEndConditionalRender();

	if (error != 0) {
		LuaLog(1, "gl.BeginEndConditionRender: error(%i) = %s",
		       error, lua_tostring(L, -1));
		lua_error(L);
	}

	return 0;
}


/******************************************************************************/
/******************************************************************************/
