/* bzflag
 * Copyright (c) 1993 - 2009 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef CALLINS_H
#define CALLINS_H

#include <string>

struct lua_State;

struct bz_CustomMapObjectInfo;

namespace CallIns {
  bool PushEntries(lua_State* L);
  bool CleanUp(lua_State* L);

  // lua plugin custom call-ins
  bool Shutdown();
  bool RecvCommand(const std::string& cmd, int playerIndex);
}

#endif // CALLINS_H

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
