
#include "common.h"

// implementation header
#include "LuaDLists.h"

// system headers
#include <string>
#include <set>
using std::string;
using std::set;

// common headers
#include "bzfgl.h"
#include "OpenGLGState.h"
#include "OpenGLPassState.h"

// local headers
#include "LuaInclude.h"
#include "LuaHashString.h"


LuaDListMgr luaDListMgr;

const char* LuaDListMgr::metaName = "DList";


/******************************************************************************/
/******************************************************************************/
//
//  LuaDList
//

LuaDList::LuaDList(GLuint list)
: listID(list)
{
	OpenGLGState::registerContextInitializer(StaticFreeContext,
	                                         StaticInitContext, this);
}


LuaDList::~LuaDList()
{
	glDeleteLists(listID, 1);
	OpenGLGState::unregisterContextInitializer(StaticFreeContext,
	                                           StaticInitContext, this);
}


bool LuaDList::Call()
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


bool LuaDList::Free()
{
	if (listID == INVALID_GL_LIST_ID) {
		return false;
	}
	glDeleteLists(listID, 1);
	listID = INVALID_GL_LIST_ID;
	return true;
}


void LuaDList::InitContext()
{
}


void LuaDList::FreeContext()
{
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


/******************************************************************************/
/******************************************************************************/
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


void LuaDListMgr::Init()
{
	OpenGLGState::registerContextInitializer(StaticFreeContext,
	                                         StaticInitContext, NULL);
}


void LuaDListMgr::Free()
{
	OpenGLGState::unregisterContextInitializer(StaticFreeContext,
	                                           StaticInitContext, NULL);
}



/******************************************************************************/
/******************************************************************************/

inline LuaDList*& LuaDListMgr::CheckLuaDList(lua_State* L, int index)
{
	return *((LuaDList**)luaL_checkudata(L, index, metaName));
}


/******************************************************************************/
/******************************************************************************/

bool LuaDListMgr::CreateMetatable(lua_State* L)
{
  luaL_newmetatable(L, metaName);
  HSTR_PUSH_CFUNC(L, "__gc",    MetaGC);
  HSTR_PUSH_CFUNC(L, "__index", MetaIndex);
  lua_pop(L, 1);  
  return true;
}


int LuaDListMgr::MetaGC(lua_State* L)
{
	LuaDList*& list = CheckLuaDList(L, 1);
	delete list;
	list = NULL;
	return 0;
}


int LuaDListMgr::MetaIndex(lua_State* L)
{
	LuaDList* list = CheckLuaDList(L, 1);
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


/******************************************************************************/
/******************************************************************************/

int LuaDListMgr::CreateList(lua_State* L)
{
	const int args = lua_gettop(L); // number of arguments
	if ((args < 1) || !lua_isfunction(L, 1)) {
		luaL_error(L,
			"Incorrect arguments to gl.CreateList(func [, arg1, arg2, etc ...])");
	}

	// generate the list id
	const GLuint listID = glGenLists(1);
	if (listID == 0) {
		return 0;
	}

	// save the current state
	const bool origDrawingEnabled = OpenGLPassState::IsDrawingEnabled();
	OpenGLPassState::SetDrawingEnabled(true);

	// build the list with the specified lua call/args
	glNewList(listID, GL_COMPILE);
	const int error = lua_pcall(L, (args - 1), 0, 0);
	glEndList();

	if (error != 0) {
		glDeleteLists(listID, 1);
		LuaLog("gl.CreateList: error(%i) = %s", error, lua_tostring(L, -1));
		lua_pushnil(L);
	}
	else {
		LuaDList** listPtr =
			(LuaDList**)lua_newuserdata(L, sizeof(LuaDList*));
		luaL_getmetatable(L, metaName);
		lua_setmetatable(L, -2);

		*listPtr = new LuaDList(listID);
	}

	// restore the state
	OpenGLPassState::SetDrawingEnabled(origDrawingEnabled);

	return 1;
}


int LuaDListMgr::CallList(lua_State* L)
{
	LuaDList* list = CheckLuaDList(L, 1);
	if (list == NULL) {
		return 0;
	}

	lua_pushboolean(L, list->Call());

	return 1;
}


int LuaDListMgr::DeleteList(lua_State* L)
{
	if (OpenGLGState::isExecutingInitFuncs()) {
		luaL_error(L, "gl.DeleteList can not be used in GLInitContext");
	}
	if (lua_isnil(L, 1)) {
		return 0;
	}
	LuaDList*& list = CheckLuaDList(L, 1);
	delete list;
	list = NULL;
	return 0;
}


/******************************************************************************/
/******************************************************************************/

bool LuaDListMgr::InsertList(LuaDList* list)
{
	if (lists.find(list) != lists.end()) {
		return false;
	}
	lists.insert(list);
	return true;
}


bool LuaDListMgr::RemoveList(LuaDList* list)
{
	set<LuaDList*>::iterator it = lists.find(list);
	if (it == lists.end()) {
		return false;
	}
	lists.erase(it);
	return true;
}


void LuaDListMgr::InitContext()
{
}


void LuaDListMgr::FreeContext()
{
}


void LuaDListMgr::StaticInitContext(void* data)
{
	((LuaDListMgr*)data)->InitContext();
}


void LuaDListMgr::StaticFreeContext(void* data)
{
	((LuaDListMgr*)data)->FreeContext();
}


/******************************************************************************/
/******************************************************************************/
