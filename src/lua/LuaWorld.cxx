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
#include "LuaWorld.h"

// system headers
#include <string>
#include <set>
#include <cctype>
using std::string;
using std::set;

// common headers
#include "BzVFS.h"
#include "EventHandler.h"
#include "StateDatabase.h"

// bzflag headers
#include "bzflag/Downloads.h"
#include "clientbase/World.h"

// local headers
#include "LuaClientOrder.h"
#include "LuaHeader.h"
#include "LuaUtils.h"
#include "LuaBZDB.h"


LuaWorld* luaWorld = NULL;

static const char* sourceFile = "bzWorld.lua";


//============================================================================//
//============================================================================//

void LuaWorld::LoadHandler()
{
  if (luaWorld) {
    return;
  }

  const World* world = World::getWorld();
  if (world == NULL) {
    return;
  }

  if (!BZDB.isTrue("luaWorld") && !world->luaWorldRequired()) {
    return;
  }

  new LuaWorld();

  if (luaWorld->L == NULL) {
    delete luaWorld;
  }
}


void LuaWorld::FreeHandler()
{
  delete luaWorld;
}


//============================================================================//
//============================================================================//

static bool BzdbReadCheck(const std::string& name)
{
  return (name != "password");
}


static bool BzdbWriteCheck(const std::string& name)
{
  if (name == "password") {
    return false;
  }
  return (BZDB.getPermission(name) == StateDatabase::ReadWrite);
}


//============================================================================//
//============================================================================//

LuaWorld::LuaWorld()
: LuaHandle("LuaWorld",
	    LUA_WORLD_SCRIPT_ID,
	    LUA_WORLD_GAME_ORDER,
	    LUA_WORLD_DRAW_WORLD_ORDER,
	    LUA_WORLD_DRAW_SCREEN_ORDER,
	    true, World::getWorld()->luaWorldRequired(), true)  // handle perms
{
  static LuaVfsModes vfsModes;
  vfsModes.readDefault  = BZVFS_LUA_WORLD BZVFS_LUA_WORLD_WRITE BZVFS_BASIC;
  vfsModes.readAllowed  = BZVFS_ALL;
  vfsModes.writeDefault = BZVFS_LUA_WORLD_WRITE;
  vfsModes.writeAllowed = BZVFS_LUA_WORLD_WRITE;
  if (devMode) {
    vfsModes.readDefault = BZVFS_LUA_USER
			   BZVFS_LUA_WORLD BZVFS_LUA_WORLD_WRITE BZVFS_BASIC;

  }

  luaWorld = this;

  if (L == NULL) {
    return;
  }

  // setup the handle pointer
  LuaExtraSpace* LHH = L2ES(L);
  LHH->handle = this;
  LHH->handlePtr = (LuaHandle**)&luaWorld;
  LHH->vfsModes = &vfsModes;
  LHH->bzdbReadCheck  = BzdbReadCheck;
  LHH->bzdbWriteCheck = BzdbWriteCheck;

  if (!SetupEnvironment()) {
    KillLua();
    return;
  }

  const string sourceCode = LoadSourceCode(sourceFile, BZVFS_LUA_WORLD);
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


LuaWorld::~LuaWorld()
{
  if (L != NULL) {
    Shutdown();
    KillLua();
  }
  luaWorld = NULL;
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
