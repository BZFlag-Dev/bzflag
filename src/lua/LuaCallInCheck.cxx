
#include "common.h"

// interface header
#include "LuaCallInCheck.h"

// common headers
#include "bzfio.h"

// local headers
#include "LuaInclude.h"


//============================================================================//
//============================================================================//

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
		LuaLog(0,
			"LuaCallInCheck mismatch for %s():  start = %i,  end = %i\n",
			funcName, startTop, endTop
		);
	}
}


//============================================================================//
//============================================================================//

