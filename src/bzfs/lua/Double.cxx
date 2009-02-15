
#include "common.h"

// interface header
#include "Double.h"

// system headers
#include <math.h>
#include <string>
using std::string;

// local headers
#include "LuaHeader.h"


const char* LuaDouble::metaName = "Double";


/******************************************************************************/
/******************************************************************************/

bool LuaDouble::IsDouble(lua_State* L, int index)
{
	return (lua_getuserdataextra(L, index) == metaName);
}


double* LuaDouble::TestDouble(lua_State* L, int index)
{
	if (lua_getuserdataextra(L, index) != metaName) {
		return NULL;
	}
	double* doublePtr = (double*)lua_touserdata(L, index);
	return doublePtr;
}


double* LuaDouble::TestNumber(lua_State* L, int index)
{
	static double value = 0.0f;
	if (lua_israwnumber(L, index)) {
		value = (double)lua_tonumber(L, 1);
		return &value;
	}
	if (lua_getuserdataextra(L, index) != metaName) {
		return NULL;
	}
	double* doublePtr = (double*)lua_touserdata(L, index);
	return doublePtr;
}


double LuaDouble::CheckDouble(lua_State* L, int index)
{
	if (lua_getuserdataextra(L, index) != LuaDouble::metaName) {
		luaL_argerror(L, index, "expected a Double");
	}
	const double* doublePtr = (double*)lua_touserdata(L, index);
	return *doublePtr;
}


double LuaDouble::CheckNumber(lua_State* L, int index)
{
	if (lua_israwnumber(L, index)) {
		return (double)lua_tonumber(L, index);
	}
	return CheckDouble(L, index);
}


/******************************************************************************/
/******************************************************************************/

int LuaDouble::MetaIndex(lua_State* L)
{
	const double d1 = CheckDouble(L, 1);
	const string key = luaL_checkstring(L, 2);
	if (key == "number") {
		lua_pushnumber(L, (float)d1);
		return 1;
	}
	else if (key == "string") {
		return MetaToString(L);
	}
	return 0;
}


int LuaDouble::MetaToString(lua_State* L)
{
	const double d1 = CheckNumber(L, 1);
	char buf[128];
	snprintf(buf, sizeof(buf), "%.14g", d1);
	lua_pushstring(L, buf);
	return 1;
}


int LuaDouble::MetaADD(lua_State* L)
{
	const double d1 = CheckNumber(L, 1);
	const double d2 = CheckNumber(L, 2);
	CreateDouble(L, d1 + d2);
	return 1;
}


int LuaDouble::MetaSUB(lua_State* L)
{
	const double d1 = CheckNumber(L, 1);
	const double d2 = CheckNumber(L, 2);
	CreateDouble(L, d1 - d2);
	return 1;
}


int LuaDouble::MetaMUL(lua_State* L)
{
	const double d1 = CheckNumber(L, 1);
	const double d2 = CheckNumber(L, 2);
	CreateDouble(L, d1 * d2);
	return 1;
}


int LuaDouble::MetaDIV(lua_State* L)
{
	const double d1 = CheckNumber(L, 1);
	const double d2 = CheckNumber(L, 2);
	CreateDouble(L, d1 / d2);
	return 1;
}


int LuaDouble::MetaMOD(lua_State* L)
{
	const double d1 = CheckNumber(L, 1);
	const double d2 = CheckNumber(L, 2);
	CreateDouble(L, fmod(d1, d2));
	return 1;
}


int LuaDouble::MetaPOW(lua_State* L)
{
	const double d1 = CheckNumber(L, 1);
	const double d2 = CheckNumber(L, 2);
	CreateDouble(L, pow(d1, d2));
	return 1;
}


int LuaDouble::MetaUNM(lua_State* L)
{
	const double d1 = CheckNumber(L, 1);
	CreateDouble(L, -d1);
	return 1;
}


int LuaDouble::MetaEQ(lua_State* L)
{
	const double d1 = CheckNumber(L, 1);
	const double d2 = CheckNumber(L, 2);
	lua_pushboolean(L, d1 == d2);
	return 1;
}


int LuaDouble::MetaLT(lua_State* L)
{
	const double d1 = CheckNumber(L, 1);
	const double d2 = CheckNumber(L, 2);
	lua_pushboolean(L, d1 < d2);
	return 1;
}


int LuaDouble::MetaLE(lua_State* L)
{
	const double d1 = CheckNumber(L, 1);
	const double d2 = CheckNumber(L, 2);
	lua_pushboolean(L, d1 <= d2);
	return 1;
}


/******************************************************************************/
/******************************************************************************/

static void PushNamedFunction(lua_State* L, const char* name,
                              int (*func)(lua_State*))
{
	lua_pushstring(L, name);
	lua_pushcfunction(L, func);
	lua_rawset(L, -3);
}


bool LuaDouble::CreateMetatable(lua_State* L)
{
	luaL_newmetatable(L, metaName);

	PushNamedFunction(L, "__index",    MetaIndex);
	PushNamedFunction(L, "__tostring", MetaToString);

	PushNamedFunction(L, "__add",  MetaADD);
	PushNamedFunction(L, "__sub",  MetaSUB);
	PushNamedFunction(L, "__mul",  MetaMUL);
	PushNamedFunction(L, "__div",  MetaDIV);
	PushNamedFunction(L, "__mod",  MetaMOD);
	PushNamedFunction(L, "__pow",  MetaPOW);
	PushNamedFunction(L, "__unm",  MetaUNM);
	PushNamedFunction(L, "__eq",   MetaEQ);
	PushNamedFunction(L, "__lt",   MetaLT);
	PushNamedFunction(L, "__le",   MetaLE);

	lua_pop(L, 1);

	return true;
}

/******************************************************************************/
/******************************************************************************/

int LuaDouble::CreateDouble(lua_State* L)
{
	double value = 0.0f;
	if (lua_israwnumber(L, 1)) {
		value = (double)lua_tonumber(L, 1);
	}
	else if (lua_israwstring(L, 1)) {
		const char* start = lua_tostring(L, 1);
		char* end;
		value = strtod(start, &end);
		if (start == end) {
			luaL_argerror(L, 1, "invalid numeric string");
		}
	}
	else {
		value = CheckDouble(L, 1);
	}

	double* doublePtr = (double*)lua_newuserdata(L, sizeof(double));
	*doublePtr = value;

	lua_setuserdataextra(L, -1, (void*)metaName);
	luaL_getmetatable(L, metaName);
	lua_setmetatable(L, -2);

	return 1;
}


int LuaDouble::CreateDouble(lua_State* L, double value)
{
	double* doublePtr = (double*)lua_newuserdata(L, sizeof(double));
	*doublePtr = value;

	lua_setuserdataextra(L, -1, (void*)metaName);
	luaL_getmetatable(L, metaName);
	lua_setmetatable(L, -2);

	return 1;
}


/******************************************************************************/
/******************************************************************************/

bool LuaDouble::PushEntries(lua_State* L)
{
	CreateMetatable(L);

	PushNamedFunction(L, "double", CreateDouble);

	return true;
}


/******************************************************************************/
/******************************************************************************/
