/* bzflag
 * Copyright (c) 1993 - 2008 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef	LUA_CLIENT_SCRIPTS_H
#define LUA_CLIENT_SCRIPTS_H

#include "common.h"

#include <string>


namespace LuaClientScripts {

  void LuaUserFreeHandler();
  void LuaUserLoadHandler();
  void LuaUserUpdate();
  bool LuaUserIsActive();
  bool LuaUserCommand(const std::string& cmd);

  void LuaWorldFreeHandler();
  void LuaWorldLoadHandler();
  void LuaWorldUpdate();
  bool LuaWorldIsActive();
  bool LuaWorldCommand(const std::string& cmd);

  void LuaOpenGLInit();
  void LuaOpenGLFree();

  bool GetDevMode();
  void SetDevMode(bool value);

}


#endif // LUA_CLIENT_SCRIPTS_H

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
