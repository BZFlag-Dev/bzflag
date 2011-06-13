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
#include "LuaRules.h"

// system headers
#include <string>
#include <cctype>

// common headers
#include "clientbase/EventHandler.h"
#include "common/BzVFS.h"
#include "common/StateDatabase.h"

// bzflag headers
#include "bzflag/Downloads.h"
#include "clientbase/World.h"

// local headers
#include "LuaBZDB.h"
#include "LuaClientOrder.h"
#include "LuaHeader.h"
#include "LuaUtils.h"


LuaRules* luaRules = NULL;

static const char* sourceFile = "bzRules.lua";


//============================================================================//
//============================================================================//

void LuaRules::LoadHandler() {
  if (luaRules) {
    return;
  }

  const World* world = World::getWorld();
  if (world == NULL) {
    return;
  }

  if (!world->luaRulesScript()) {
    return;
  }

  new LuaRules();

  if (luaRules->L == NULL) {
    delete luaRules;
  }
}


void LuaRules::FreeHandler() {
  delete luaRules;
}


//============================================================================//
//============================================================================//

static bool BzdbReadCheck(const std::string& name) {
  return ((BZDB.isSet(name)) &&
          (BZDB.getPermission(name) == StateDatabase::Server));
}


static bool BzdbWriteCheck(const std::string& /*name*/) {
  return false;
}


//============================================================================//
//============================================================================//

LuaRules::LuaRules()
  : LuaHandle("LuaRules",
              LUA_RULES_SCRIPT_ID,
              LUA_RULES_GAME_ORDER,
              LUA_RULES_DRAW_WORLD_ORDER,
              LUA_RULES_DRAW_SCREEN_ORDER,
              true, true, true) { // handle perms
  static LuaVfsModes vfsModes;
  vfsModes.readDefault  = BZVFS_LUA_RULES;
  vfsModes.readAllowed  = BZVFS_LUA_RULES;
  vfsModes.writeDefault = BZVFS_LUA_RULES_WRITE;
  vfsModes.writeAllowed = BZVFS_LUA_RULES_WRITE;
  if (devMode) {
    vfsModes.readDefault = BZVFS_LUA_USER BZVFS_LUA_RULES;
    vfsModes.readAllowed = BZVFS_LUA_USER BZVFS_LUA_RULES;
  }

  luaRules = this;

  if (L == NULL) {
    return;
  }

  // setup the handle pointer
  LuaExtraSpace* LHH = L2ES(L);
  LHH->handle = this;
  LHH->handlePtr = (LuaHandle**)&luaRules;
  LHH->vfsModes = &vfsModes;
  LHH->bzdbReadCheck  = BzdbReadCheck;
  LHH->bzdbWriteCheck = BzdbWriteCheck;

  if (!SetupEnvironment()) {
    KillLua();
    return;
  }

  const std::string sourceCode = LoadSourceCode(sourceFile, BZVFS_LUA_RULES);
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


LuaRules::~LuaRules() {
  if (L != NULL) {
    Shutdown();
    KillLua();
  }
  luaRules = NULL;
}


//============================================================================//
//============================================================================//


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=2 tabstop=8 expandtab
