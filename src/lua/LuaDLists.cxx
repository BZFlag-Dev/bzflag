
#include "common.h"

// interface header
#include "LuaDLists.h"

// system headers
#include <string>
using std::string;

// common headers
#include "bzfgl.h"
#include "OpenGLGState.h"
#include "OpenGLPassState.h"

// local headers
#include "LuaInclude.h"
#include "LuaHashString.h"


LuaDListMgr luaDListMgr;

const char* LuaDListMgr::metaName = "DList";


//============================================================================//
//============================================================================//
//
//  LuaDList
//

LuaDList::LuaDList(GLuint id)
: listID(id)
, maxAttribDepth(0)
, minAttribDepth(0)
, exitAttribDepth(0)
{
	OpenGLGState::registerContextInitializer(StaticFreeContext,
																					 StaticInitContext, this);
}


LuaDList::~LuaDList()
{
	FreeContext();
	OpenGLGState::unregisterContextInitializer(StaticFreeContext,
																						 StaticInitContext, this);
}


bool LuaDList::Call() const
{
	if (listID == INVALID_GL_LIST_ID) {
		return false;
	}
	glCallList(listID);
	return true;
}


bool LuaDList::IsValid() const 
{
	return (listID != INVALID_GL_LIST_ID);
}


bool LuaDList::Delete()
{
	if (listID == INVALID_GL_LIST_ID) {
		return false;
	}
	FreeContext();
	return true;
}


void LuaDList::InitContext()
{
}


void LuaDList::FreeContext()
{
	if (listID == INVALID_GL_LIST_ID) {
		return;
	}
	glDeleteLists(listID, 1);
	listID = INVALID_GL_LIST_ID;
}


void LuaDList::StaticInitContext(void* data)
{
	((LuaDList*)data)->InitContext();
}


void LuaDList::StaticFreeContext(void* data)
{
	((LuaDList*)data)->FreeContext();
}


//============================================================================//
//============================================================================//
//
//  LuaDListMgr
//

bool LuaDListMgr::PushEntries(lua_State* L)
{
	CreateMetatable(L);

	PUSH_LUA_CFUNC(L, CreateList);
	PUSH_LUA_CFUNC(L, CallList);
	PUSH_LUA_CFUNC(L, DeleteList);

	return true;
}


//============================================================================//
//============================================================================//

const LuaDList* LuaDListMgr::TestLuaDList(lua_State* L, int index)
{
	if (lua_getuserdataextra(L, index) != metaName) {
		return NULL;
	}
	return (LuaDList*)lua_touserdata(L, index);
}


const LuaDList* LuaDListMgr::CheckLuaDList(lua_State* L, int index)
{
	if (lua_getuserdataextra(L, index) != metaName) {
		luaL_argerror(L, index, "expected DList");
	}
	return (LuaDList*)lua_touserdata(L, index);
}


LuaDList* LuaDListMgr::GetLuaDList(lua_State* L, int index)
{
	if (lua_getuserdataextra(L, index) != metaName) {
		luaL_argerror(L, index, "expected DList");
	}
	return (LuaDList*)lua_touserdata(L, index);
}


//============================================================================//
//============================================================================//

bool LuaDListMgr::CreateMetatable(lua_State* L)
{
	luaL_newmetatable(L, metaName);
	HSTR_PUSH_CFUNC(L,  "__gc",    MetaGC);
	HSTR_PUSH_CFUNC(L,  "__index", MetaIndex);
	HSTR_PUSH_STRING(L, "__metatable", "no access");
	lua_pop(L, 1);  
	return true;
}


int LuaDListMgr::MetaGC(lua_State* L)
{
	LuaDList* list = GetLuaDList(L, 1);
	list->~LuaDList();
	return 0;
}


int LuaDListMgr::MetaIndex(lua_State* L)
{
	const LuaDList* list = CheckLuaDList(L, 1);
	if (list == NULL) {
		return 0;
	}

	if (!lua_israwstring(L, 2)) {
		return 0;
	}

	const string key = lua_tostring(L, 2);
	if (key == "valid") {
		lua_pushboolean(L, list->IsValid());
		return 1;
	}

	return 0;
}


//============================================================================//
//============================================================================//

int LuaDListMgr::CreateList(lua_State* L)
{
	const int args = lua_gettop(L); // number of arguments
	if ((args < 1) || !lua_isfunction(L, 1)) {
		luaL_error(L, "Incorrect arguments to gl.CreateList(func, ...)");
	}

	// notify and check with OpenGLPassState
	if (!OpenGLPassState::NewList()) {
		luaL_error(L, "gl.CreateList() recursion");
	}

	// generate the list id
	const GLuint listID = glGenLists(1);
	if (listID == 0) {
		OpenGLPassState::EndList();
		return 0;
	}

	// build the list with the specified lua call/args
	glNewList(listID, GL_COMPILE);
	const int error = lua_pcall(L, (args - 1), 0, 0);
	glEndList();

	if (error != 0) {
		glDeleteLists(listID, 1);
		LuaLog(1, "gl.CreateList: error(%i) = %s", error, lua_tostring(L, -1));
		lua_pushnil(L);
	}
	else {
		void* udData = lua_newuserdata(L, sizeof(LuaDList));
		new(udData) LuaDList(listID);
		lua_setuserdataextra(L, -1, (void*)metaName);
		luaL_getmetatable(L, metaName);
		lua_setmetatable(L, -2);
	}

	// restore the state
	OpenGLPassState::EndList();

	return 1;
}


int LuaDListMgr::CallList(lua_State* L)
{
	const LuaDList* list = CheckLuaDList(L, 1);
	if (list == NULL) {
		return 0;
	}

	lua_pushboolean(L, list->Call());

	return 1;
}


int LuaDListMgr::DeleteList(lua_State* L)
{
	if (OpenGLGState::isExecutingInitFuncs()) {
		luaL_error(L, "gl.DeleteList can not be used in GLReload");
	}
	if (lua_isnil(L, 1)) {
		return 0;
	}
	LuaDList* list = GetLuaDList(L, 1);
	list->Delete();
	return 0;
}


//============================================================================//
//============================================================================//
