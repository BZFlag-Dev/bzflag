/* bzflag
 * Copyright (c) 1993-2010 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */


#include "common.h"

// interface header
#include "LuaCallInCheck.h"

// common headers
#include "bzfio.h"

// local headers
#include "LuaHeader.h"


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



// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
