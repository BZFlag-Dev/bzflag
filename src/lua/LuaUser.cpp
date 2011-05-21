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
#include "LuaUser.h"

// system headers
#include <cctype>
#include <string>
#include <set>
using std::string;
using std::set;

// common headers
#include "BzVFS.h"
#include "EventHandler.h"
#include "StateDatabase.h"

// bzflag headers
#include "bzflag/Downloads.h"

// local headers
#include "LuaClientOrder.h"
#include "LuaHeader.h"
#include "LuaUtils.h"


LuaUser* luaUser = NULL;

static const char* sourceFile = "bzUser.lua";


//============================================================================//
//============================================================================//

void LuaUser::LoadHandler() {
  if (luaUser) {
    return;
  }

  if (!BZDB.isTrue("luaUser")) {
    return;
  }

  const string& dir = BZDB.get("luaUserDir");
  if (dir.empty() || (dir[0] == '!')) {
    return;
  }

  new LuaUser();

  if (luaUser->L == NULL) {
    delete luaUser;
  }
}


void LuaUser::FreeHandler() {
  delete luaUser;
}


//============================================================================//
//============================================================================//

LuaUser::LuaUser()
  : LuaHandle("LuaUser",
              LUA_USER_SCRIPT_ID,
              LUA_USER_GAME_ORDER,
              LUA_USER_DRAW_WORLD_ORDER,
              LUA_USER_DRAW_SCREEN_ORDER,
              false, false, true) { // handle perms
  static LuaVfsModes vfsModes;
  vfsModes.readDefault  = BZVFS_LUA_USER BZVFS_LUA_USER_WRITE BZVFS_MEDIA;
  vfsModes.readAllowed  = BZVFS_LUA_USER BZVFS_LUA_USER_WRITE BZVFS_MEDIA
                          BZVFS_LUA_READ BZVFS_LUA_WRITE;
  vfsModes.writeDefault = BZVFS_LUA_USER_WRITE;
  vfsModes.writeAllowed = BZVFS_LUA_USER_WRITE;

  luaUser = this;

  if (L == NULL) {
    return;
  }

  // setup the handle header pointers
  LuaExtraSpace* LHH = L2ES(L);
  LHH->handle = this;
  LHH->handlePtr = (LuaHandle**)&luaUser;
  LHH->vfsModes = &vfsModes;
  LHH->bzdbReadCheck  = NULL;
  LHH->bzdbWriteCheck = LuaUtils::ClientWriteCheck;

  if (!SetupEnvironment()) {
    KillLua();
    return;
  }

  const string sourceCode = LoadSourceCode(sourceFile, BZVFS_LUA_USER);
  if (sourceCode.empty()) {
    KillLua();
    return;
  }

  // register for call-ins
  eventHandler.AddClient(this);

  if (!ExecSourceCode(sourceCode, sourceFile)) {
    KillLua();
    return;
  }
}


LuaUser::~LuaUser() {
  if (L != NULL) {
    Shutdown();
    KillLua();
  }
  luaUser = NULL;
}


//============================================================================//
//============================================================================//


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=2 tabstop=8
