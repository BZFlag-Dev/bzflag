
#include "common.h"

// interface header
#include "LuaCallInCheck.h"

// local headers
#include "LuaInclude.h"


/******************************************************************************/
/******************************************************************************/

LuaCallInCheck::LuaCallInCheck(lua_State* _L, const char* name)
{
	L = _L;
	startTop = lua_gettop(L);
	funcName = name;
}


LuaCallInCheck::~LuaCallInCheck()
{
	const int endTop = lua_gettop(L);
	if (startTop != endTop) {
		printf("LuaCallInCheck mismatch for %s():  start = %i,  end = %i",
		       funcName, startTop, endTop);
	}
}


/******************************************************************************/
/******************************************************************************/

