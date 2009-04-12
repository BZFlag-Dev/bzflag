
#include "common.h"

// interface header
#include "LuaDynCol.h"

// system headers
#include <string>
using std::string;

// common headers
#include "DynamicColor.h"

// local headers
#include "LuaInclude.h"


//============================================================================//
//============================================================================//

bool LuaDynCol::PushEntries(lua_State* L)
{
	PUSH_LUA_CFUNC(L, GetDynColName);
	PUSH_LUA_CFUNC(L, GetDynCol);
	PUSH_LUA_CFUNC(L, SetDynCol);

	return true;
}


//============================================================================//
//============================================================================//

static inline const DynamicColor* ParseDynCol(lua_State* L, int index)
{
	const int dyncolIndex = luaL_checkint(L, index);
	return DYNCOLORMGR.getColor(dyncolIndex);
}


//============================================================================//
//============================================================================//

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
	lua_pushfvec4(L, dyncol->getColor());
	return 4;
}


int LuaDynCol::SetDynCol(lua_State* L)
{
	DynamicColor* dyncol = const_cast<DynamicColor*>(ParseDynCol(L, 1));
	if (dyncol == NULL) {
		return 0;
	}
	const fvec4& oldColor = dyncol->getColor();
	const fvec4 newColor = luaL_optfvec4(L, 2, oldColor);
	dyncol->setColor(newColor);
	lua_pushboolean(L, true);
	return 1;
}


//============================================================================//
//============================================================================//
