
#include "common.h"

// implementation header
#include "LuaDynCol.h"

// system headers
#include <string>
using std::string;

// common headers
#include "DynamicColor.h"

// local headers
#include "LuaInclude.h"


/******************************************************************************/
/******************************************************************************/

bool LuaDynCol::PushEntries(lua_State* L)
{
	PUSH_LUA_CFUNC(L, GetDynColName);
	PUSH_LUA_CFUNC(L, GetDynCol);
	PUSH_LUA_CFUNC(L, SetDynCol);

	return true;
}


/******************************************************************************/
/******************************************************************************/

static inline const DynamicColor* ParseDynCol(lua_State* L, int index)
{
	const int dyncolIndex = luaL_checkint(L, index);
	return DYNCOLORMGR.getColor(dyncolIndex);
}


/******************************************************************************/
/******************************************************************************/

int LuaDynCol::GetDynColName(lua_State* L)
{
	const DynamicColor* dyncol = ParseDynCol(L, 1);
	if (dyncol == NULL) {
		return 0;
	}
	lua_pushstdstring(L, dyncol->getName());
	return 1;
}


int LuaDynCol::GetDynCol(lua_State* L)
{
	const DynamicColor* dyncol = ParseDynCol(L, 1);
	if (dyncol == NULL) {
		return 0;
	}
	const float* color = dyncol->getColor();
	lua_pushnumber(L, color[0]);
	lua_pushnumber(L, color[1]);
	lua_pushnumber(L, color[2]);
	lua_pushnumber(L, color[3]);
	return 4;
}


int LuaDynCol::SetDynCol(lua_State* L)
{
	DynamicColor* dyncol = const_cast<DynamicColor*>(ParseDynCol(L, 1));
	if (dyncol == NULL) {
		return 0;
	}
	const float* oldColor = dyncol->getColor();
	float newColor[4];
	newColor[0] = luaL_optfloat(L, 2, oldColor[0]);
	newColor[1] = luaL_optfloat(L, 3, oldColor[1]);
	newColor[2] = luaL_optfloat(L, 4, oldColor[2]);
	newColor[3] = luaL_optfloat(L, 5, oldColor[3]);
	dyncol->setColor(newColor);
	lua_pushboolean(L, true);
	return 1;
}


/******************************************************************************/
/******************************************************************************/
