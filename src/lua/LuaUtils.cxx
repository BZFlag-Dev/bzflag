
#include "common.h"

// interface header
#include "LuaUtils.h"

// system headers
#include <assert.h>
#include <string>
#include <set>
#include <cctype>
using std::string;
using std::set;

// common headers
#include "TextUtils.h"
#include "bzfio.h"


/******************************************************************************/
/******************************************************************************/

void LuaLog(int level, const std::string& msg) // FIXME
{
	logDebugMessage(level, msg);
}


void LuaLog(int level, const char* fmt, ...) // FIXME
{
	va_list ap;
	va_start(ap, fmt);
	logDebugMessageArgs(level, fmt, ap);
	va_end(ap);
}


/******************************************************************************/
/******************************************************************************/

static       int copyDepth = 0;
static const int maxCopyDepth = 256;


static bool CopyPushData(lua_State* dst, lua_State* src, int index);
static bool CopyPushTable(lua_State* dst, lua_State* src, int index);


static inline int PosLuaIndex(lua_State* src, int index)
{
	if (index > 0) {
		return index;
	} else {
		return (lua_gettop(src) + index + 1);
	}
}


static bool CopyPushData(lua_State* dst, lua_State* src, int index)
{
	const int type = lua_type(src, index);
	switch (type) {
		case LUA_TBOOLEAN: {
			lua_pushboolean(dst, lua_tobool(src, index));
			break;
		}
		case LUA_TNUMBER: {
			lua_pushnumber(dst, lua_tonumber(src, index));
			break;
		}
		case LUA_TSTRING: {
			size_t len;
			const char* data = lua_tolstring(src, index, &len);
			lua_pushlstring(dst, data, len);
			break;
		}
		case LUA_TTABLE: {
			CopyPushTable(dst, src, index);
			break;
		}
		default: {
			lua_pushnil(dst); // unhandled type
			return false;
		}
	}
	return true;
}


static bool CopyPushTable(lua_State* dst, lua_State* src, int index)
{
	if (copyDepth > maxCopyDepth) {
		lua_pushnil(dst); // push something
		return false;
	}
	lua_checkstack(dst, 3);
	copyDepth++;
	lua_newtable(dst);
	const int table = PosLuaIndex(src, index);
	for (lua_pushnil(src); lua_next(src, table) != 0; lua_pop(src, 1)) {
		CopyPushData(dst, src, -2); // copy the key
		CopyPushData(dst, src, -1); // copy the value
		lua_rawset(dst, -3);
	}
	copyDepth--;

	return true;
}


int LuaUtils::CopyData(lua_State* dst, lua_State* src, int count)
{
	const int srcTop = lua_gettop(src);
	const int dstTop = lua_gettop(dst);
	if (srcTop < count) {
		return 0;
	}
	lua_checkstack(dst, count + 2);

	copyDepth = 0;

	const int startIndex = (srcTop - count + 1);
	const int endIndex   = srcTop;
	for (int i = startIndex; i <= endIndex; i++) {
		CopyPushData(dst, src, i);
	}
	lua_settop(dst, dstTop + count);

	return count;
}


/******************************************************************************/
/******************************************************************************/

static void PushCurrentFunc(lua_State* L, const char* caller)
{
	// get the current function
	lua_Debug ar;
	if (lua_getstack(L, 1, &ar) == 0) {
		luaL_error(L, "%s() lua_getstack() error", caller);
	}
	if (lua_getinfo(L, "f", &ar) == 0) {
		luaL_error(L, "%s() lua_getinfo() error", caller);
	}
	if (!lua_isfunction(L, -1)) {
		luaL_error(L, "%s() invalid current function", caller);
	}
}


static void PushFunctionEnv(lua_State* L, const char* caller, int funcIndex)
{
	lua_getfenv(L, funcIndex);
	lua_pushliteral(L, "__fenv");
	lua_rawget(L, -2);
	if (lua_isnil(L, -1)) {
		lua_pop(L, 1); // there is no fenv proxy
	} else {
		lua_remove(L, -2); // remove the orig table, leave the proxy
	}

	if (!lua_istable(L, -1)) {
		luaL_error(L, "%s() invalid fenv", caller);
	}
}


void LuaUtils::PushCurrentFuncEnv(lua_State* L, const char* caller)
{
	PushCurrentFunc(L, caller);
	PushFunctionEnv(L, caller, -1);
	lua_remove(L, -2); // remove the function
}


/******************************************************************************/
/******************************************************************************/
//
//  FormatArgs()
//

/* FIXME
bool LuaUtils::FormatArgs(lua_State* L, bool expandTables,
												  std::vector<std::string>& result, const char* caller)
{
}
*/
																						

/******************************************************************************/
/******************************************************************************/

// copied from lua/src/lauxlib.cpp:luaL_checkudata()
void* LuaUtils::TestUserData(lua_State* L, int index, const string& type)
{
	const char* tname = type.c_str();
	void *p = lua_touserdata(L, index);
	if (p != NULL) {                               // value is a userdata?
		if (lua_getmetatable(L, index)) {            // does it have a metatable?
			lua_getfield(L, LUA_REGISTRYINDEX, tname); // get correct metatable
			if (lua_rawequal(L, -1, -2)) {             // the correct mt?
				lua_pop(L, 2);                           // remove both metatables
				return p;
			}
		}
	}
	return NULL;
}


/******************************************************************************/
/******************************************************************************/
//
//  LowerKeys()
//

static int lowerKeysTable = 0;


static bool LowerKeysCheck(lua_State* L, int table)
{
	bool used = false;
	lua_pushvalue(L, table);
	lua_rawget(L, lowerKeysTable);
	if (lua_isnil(L, -1)) {
		used = false;
		lua_pushvalue(L, table);
		lua_pushboolean(L, true);
		lua_rawset(L, lowerKeysTable);
	}
	lua_pop(L, 1);
	return used;
}


static bool LowerKeysReal(lua_State* L, int depth)
{
	lua_checkstack(L, 4);

	const int table = lua_gettop(L);
	if (LowerKeysCheck(L, table)) {
		return true;
	}

	// a new table for changed values
	const int changed = table + 1;
	lua_newtable(L);

	for (lua_pushnil(L); lua_next(L, table) != 0; lua_pop(L, 1)) {
		if (lua_istable(L, -1)) {
			LowerKeysReal(L, depth + 1);
		}
		if (lua_israwstring(L, -2)) {
			const string rawKey = lua_tostring(L, -2);
			const string lowerKey = TextUtils::tolower(rawKey);
			if (rawKey != lowerKey) {
				// removed the mixed case entry
				lua_pushvalue(L, -2); // the key
				lua_pushnil(L);
				lua_rawset(L, table);
				// does the lower case key alread exist in the table?
				lua_pushstring(L, lowerKey.c_str());
				lua_rawget(L, table);
				if (lua_isnil(L, -1)) {
					// lower case does not exist, add it to the changed table
					lua_pushstring(L, lowerKey.c_str());
					lua_pushvalue(L, -3); // the value
					lua_rawset(L, changed);
				}
				lua_pop(L, 1);
			}
		}
	}

	// copy the changed values into the table
	for (lua_pushnil(L); lua_next(L, changed) != 0; lua_pop(L, 1)) {
		lua_pushvalue(L, -2); // copy the key to the top
		lua_pushvalue(L, -2); // copy the value to the top
		lua_rawset(L, table);		
	}

	lua_pop(L, 1); // pop the changed table

	return true;
}


bool LuaUtils::LowerKeys(lua_State* L, int table)
{
	if (!lua_istable(L, table)) {
		return false;
	}

	// table of processed tables
	lowerKeysTable = lua_gettop(L) + 1;
	lua_checkstack(L, 4);
	lua_newtable(L);

	lua_pushvalue(L, table); // push the table onto the top of the stack

	LowerKeysReal(L, 0);

	lua_pop(L, 2); // the lowered table, and the check table

	return true;
}


/******************************************************************************/
/******************************************************************************/

void LuaUtils::PrintStack(lua_State* L)
{
	const int top = lua_gettop(L);
	for (int i = 1; i <= top; i++) {
		printf("\t%i: type = %s (%p)\n", i, luaL_typename(L, i), lua_topointer(L, i));
		const int type = lua_type(L, i);
		if (type == LUA_TSTRING) {
			printf("\t\t%s\n", lua_tostring(L, i));
		} else if (type == LUA_TNUMBER) {
			printf("\t\t%f\n", lua_tonumber(L, i));
		} else if (type == LUA_TBOOLEAN) {
			printf("\t\t%s\n", lua_tobool(L, i) ? "true" : "false");
		} else {
			printf("\n");
		}
	}
}


/******************************************************************************/
/******************************************************************************/

int LuaUtils::Print(lua_State* L)
{
	// copied from lua/src/lib/lbaselib.c
	string msg = "";
	const int args = lua_gettop(L); // number of arguments

	lua_getglobal(L, "tostring");

	for (int i = 1; i <= args; i++) {
		const char *s;
		lua_pushvalue(L, -1);     // function to be called
		lua_pushvalue(L, i);      // value to print
		lua_call(L, 1, 1);
		s = lua_tostring(L, -1);  // get result
		if (s == NULL) {
			return luaL_error(L, "`tostring' must return a string to `print'");
		}
		if (i > 1) {
			msg += ", ";
		}
		msg += s;
		lua_pop(L, 1);            // pop result
	}
	LuaLog(0, msg);

	if ((args != 1) || !lua_istable(L, 1)) {
		return 0;
	}

	// print solo tables (array style)
	msg = "TABLE: ";
	bool first = true;
	const int table = 1;
	for (lua_pushnil(L); lua_next(L, table) != 0; lua_pop(L, 1)) {
		if (lua_israwnumber(L, -2)) {  // only numeric keys
			const char *s;
			lua_pushvalue(L, -3);     // function to be called
			lua_pushvalue(L, -2	);    // value to print
			lua_call(L, 1, 1);
			s = lua_tostring(L, -1);  // get result
			if (s == NULL) {
				return luaL_error(L, "`tostring' must return a string to `print'");
			}
			if (!first) {
				msg += ", ";
			}
			msg += s;
			first = false;
			lua_pop(L, 1);            // pop result
		}
	}
	LuaLog(0, msg);

	return 0;
}


/******************************************************************************/
/******************************************************************************/
