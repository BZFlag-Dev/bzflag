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
#include "LuaServer.h"

// system headers
#include <stdio.h>
#include <ctype.h>
#include <assert.h>
#include <string>
#include <vector>

// common headers
#include "bzfsAPI.h"
#include "bzfio.h"
#include "version.h"
#include "BzVFS.h"
#include "DirectoryNames.h"
#include "TextUtils.h"
#include "LuaHeader.h"

// bzfs headers
#include "bzfs/bzfs.h"
#include "bzfs/CmdLineOptions.h"

//local headers
#include "CallIns.h"
#include "CallOuts.h"
#include "Constants.h"
#include "MapObject.h"
#include "RawLink.h"
#include "SlashCmd.h"
// from libLuaGame
#include "LuaDouble.h"
#include "LuaExtras.h"
#include "LuaBitOps.h"
#include "LuaBZDB.h"
#include "LuaBzMaterial.h"
#include "LuaDynCol.h"
#include "LuaHTTP.h"
#include "LuaObstacle.h"
#include "LuaPack.h"
#include "LuaPhyDrv.h"
#include "LuaTexMat.h"
#include "LuaVector.h"
#include "LuaVFS.h"
#include "LuaZip.h"


static const char* sourceFile = "bzServer.lua";


//============================================================================//
//============================================================================//

static lua_State* L = NULL;

static bool CreateLuaState(const std::string& script);
static std::string EnvExpand(const std::string& path);


//============================================================================//
//============================================================================//

static std::string directory = "./";


//============================================================================//
//============================================================================//

class UpdateTick : public bz_EventHandler {
  private:
    enum QueuedCommand { NoCmd, ReloadCmd, DisableCmd };

  public:
    UpdateTick() : active(false), command(NoCmd) {}
    ~UpdateTick() {}

    void queueReload() {
      if (command != DisableCmd) {
        command = ReloadCmd;
      }
    }

    void queueDisable() {
      command = DisableCmd;
    }

    void activate() {
      if (!active) {
        bz_registerEvent(bz_eTickEvent, this);
        active = true;
      }
    }

    void deactivate() {
      if (active) {
        bz_removeEvent(bz_eTickEvent, this);
        active = false;
      }
    }

  private:
    void process(bz_EventData*) {
      switch (command) {
        case NoCmd: {
          break; // do nothing
        }
        case DisableCmd: {
          LuaServer::kill();
          break;
        }
        case ReloadCmd: {
          LuaServer::kill();
          LuaServer::init(clOptions->luaServerDir);
          break;
        }
      }
      command = NoCmd;
    }

  private:
    bool active;
    QueuedCommand command;
};

static UpdateTick updateTick;


//============================================================================//
//============================================================================//

static bool fileExists(const std::string& file) {
  FILE* f = fopen(file.c_str(), "r");
  if (f == NULL) {
    return false;
  }
  fclose(f);
  return true;
}


//============================================================================//
//============================================================================//

const std::string& LuaServer::getLuaDir() {
  return directory;
}


//============================================================================//
//============================================================================//

static void setupVFS() {
  // add the -luaserver directory as readable
  bzVFS.removeFS(BZVFS_LUA_SERVER);
  bzVFS.addFS(BZVFS_LUA_SERVER, directory);

  // add a writable cache dir
  const std::string writeDir = getCacheDirName() + "LuaServer";
  bzVFS.removeFS(BZVFS_LUA_SERVER_WRITE);
  bzVFS.addFS(BZVFS_LUA_SERVER_WRITE, writeDir);
  bzVFS.setFSWritable(BZVFS_LUA_SERVER_WRITE, true);
  BzVFS::createPathDirs("", BzVFS::cleanDirPath(writeDir));

  // add the -luaworld directory as read/write
  const std::string luaWorldDir = clOptions->luaWorldDir;
  if (!luaWorldDir.empty()) {
    // add the raw filesystem, not the docket
    bzVFS.removeFS(BZVFS_LUA_WORLD);
    if (bzVFS.addFS(BZVFS_LUA_WORLD, luaWorldDir)) {
      bzVFS.setFSWritable(BZVFS_LUA_WORLD, true);
    }
  }

  // add the -luarules directory as read/write
  const std::string luaRulesDir = clOptions->luaRulesDir;
  if (!luaRulesDir.empty()) {
    // add the raw filesystem, not the docket
    bzVFS.removeFS(BZVFS_LUA_RULES);
    if (bzVFS.addFS(BZVFS_LUA_RULES, luaRulesDir)) {
      bzVFS.setFSWritable(BZVFS_LUA_RULES, true);
    }
  }
}


//============================================================================//
//============================================================================//

bool LuaServer::init(const std::string& cmdLine) {
  if (cmdLine.empty()) {
    return false;
  }

  bool dieHard = false;

  debugf(1, "loading LuaServer\n");

  if (L != NULL) {
    debugf(1, "LuaServer is already loaded\n");
    return false;
  }

  // dieHard check
  std::string scriptDir = cmdLine.c_str();
  if (scriptDir.size() > 8) {
    if (scriptDir.substr(0, 8) == "dieHard,") {
      dieHard = true;
      scriptDir = scriptDir.substr(8);
    }
  }

  // leading tilde => $HOME substitution
  if (!scriptDir.empty() && (scriptDir[0] == '~')) {
    scriptDir = "$HOME" + scriptDir.substr(1);
  }

  scriptDir = EnvExpand(scriptDir);
  if (!scriptDir.empty() && (scriptDir[scriptDir.size() - 1] != '/')) {
    scriptDir += '/';
  }

  const std::string scriptFile = scriptDir + sourceFile;
  if (!fileExists(scriptFile)) {
    debugf(1, "LuaServer: could not find %s in %s\n",
           sourceFile, scriptDir.c_str());
    if (dieHard) {
      exit(2);
    }
    return false;
  }

  directory = scriptDir;

  setupVFS();

  if (!CreateLuaState(scriptFile)) {
    if (dieHard) {
      exit(3);
    }
    return false;
  }

  updateTick.activate();

  return true;
}


//============================================================================//
//============================================================================//

bool LuaServer::kill() {
  updateTick.deactivate();

  if (L == NULL) {
    return false;
  }

  CallIns::Shutdown(); // send the call-in

  RawLink::CleanUp(L);
  SlashCmd::CleanUp(L);
  MapObject::CleanUp(L);
  CallIns::CleanUp(L);

  lua_close(L);
  L = NULL;

  return true;
}


//============================================================================//
//============================================================================//

bool LuaServer::isActive() {
  return (L != NULL);
}


//============================================================================//
//============================================================================//

lua_State* LuaServer::GetL() {
  return L;
}


//============================================================================//
//============================================================================//

void LuaServer::queueReload() {
  updateTick.queueReload();
}


void LuaServer::queueDisable() {
  updateTick.queueDisable();
}


//============================================================================//
//============================================================================//

void LuaServer::recvCommand(const std::string& cmdLine, int playerIndex) {
  std::vector<std::string> args = TextUtils::tokenize(cmdLine, " \t", 3);

  GameKeeper::Player* p = GameKeeper::Player::getPlayerByIndex(playerIndex);
  if (p == NULL) {
    return;
  }

  if (args[0] != "/luaserver") {
    return; // something is amiss, bail
  }

  if (args.size() < 2) {
    sendMessage(ServerPlayer, playerIndex,
                "/luaserver < status | disable | reload >");
    return;
  }

  if (args[1] == "status") {
    if (isActive()) {
      sendMessage(ServerPlayer, playerIndex, "LuaServer is enabled");
    }
    else {
      sendMessage(ServerPlayer, playerIndex, "LuaServer is disabled");
    }
    return;
  }

  if (args[1] == "disable") {
    if (!p->accessInfo.hasPerm(PlayerAccessInfo::luaServer)) {
      sendMessage(ServerPlayer, playerIndex,
                  "You do not have permission to control LuaServer");
      return;
    }
    if (isActive()) {
      kill();
      sendMessage(ServerPlayer, playerIndex, "LuaServer has been disabled");
    }
    else {
      sendMessage(ServerPlayer, playerIndex, "LuaServer is not loaded");
    }
    return;
  }

  if (args[1] == "reload") {
    if (!p->accessInfo.hasPerm(PlayerAccessInfo::luaServer)) {
      sendMessage(ServerPlayer, playerIndex,
                  "You do not have permission to control LuaServer");
      return;
    }
    const std::string srcDir =
      (args.size() > 2) ? args[2] : clOptions->luaServerDir;
    kill();
    if (init(srcDir)) {
      sendMessage(ServerPlayer, playerIndex, "LuaServer reload succeeded");
    }
    else {
      sendMessage(ServerPlayer, playerIndex, "LuaServer reload failed");
    }
    return;
  }

  if (L != NULL) {
    CallIns::RecvCommand(cmdLine, playerIndex);
  }

  return;
}


//============================================================================//
//============================================================================//

static bool CreateLuaState(const std::string& script) {
  static LuaVfsModes vfsModes;
  vfsModes.readDefault  = BZVFS_LUA_SERVER;
  vfsModes.readAllowed  = BZVFS_ALL;
  vfsModes.writeDefault = BZVFS_LUA_SERVER_WRITE;
  vfsModes.writeAllowed = BZVFS_ALL;

  assert(LUAI_EXTRASPACE >= sizeof(LuaExtraSpace));

  if (L != NULL) {
    return false;
  }

  L = luaL_newstate();

  LuaExtraSpace* LHH = L2ES(L);
  LHH->handle    = NULL;
  LHH->handlePtr = NULL;
  LHH->vfsModes  = &vfsModes;
  LHH->bzdbReadCheck  = NULL;
  LHH->bzdbWriteCheck = NULL;

  luaL_openlibs(L);

  // adjust package.path and package.cpath
  const std::string lualib = directory + "lualib/";
  const std::string path   = lualib + "?.lua";
  const std::string cpath  = lualib + "?.so;" + lualib + "?.dll";
  lua_getglobal(L, "package");
  lua_pushstdstring(L, path);  lua_setfield(L, -2, "path");
  lua_pushstdstring(L, cpath); lua_setfield(L, -2, "cpath");
  lua_pop(L, 1);

  // add the global error function (traceback)
  lua_getglobal(L, "debug");
  lua_getfield(L, -1, "traceback");
  lua_seterrorfunc(L, luaL_ref(L, LUA_REGISTRYINDEX));

  lua_pushliteral(L, "script");
  lua_newtable(L); {
    CallIns::PushEntries(L);
  }
  lua_rawset(L, LUA_GLOBALSINDEX);

  lua_pushvalue(L, LUA_GLOBALSINDEX);
  LuaDouble::PushEntries(L);
  LuaExtras::PushEntries(L);
  lua_pop(L, 1);

  lua_getglobal(L, "math");
  LuaVector::PushEntries(L);
  LuaBitOps::PushEntries(L);
  lua_pop(L, 1);

  lua_pushliteral(L, "bz");
  lua_newtable(L); {
    CallOuts::PushEntries(L);
    MapObject::PushEntries(L);
    SlashCmd::PushEntries(L);
    RawLink::PushEntries(L);
    LuaObstacle::PushEntries(L);
    LuaPack::PushEntries(L);
    LuaBzMaterial::PushEntries(L);
    LuaDynCol::PushEntries(L);
    LuaTexMat::PushEntries(L);
    LuaPhyDrv::PushEntries(L);
    LuaZip::PushEntries(L);
  }
  lua_rawset(L, LUA_GLOBALSINDEX);

  lua_pushliteral(L, "bzdb");
  lua_newtable(L); {
    LuaBZDB::PushEntries(L);
  }
  lua_rawset(L, LUA_GLOBALSINDEX);

  lua_pushliteral(L, "url");
  lua_newtable(L); {
    LuaHTTPMgr::PushEntries(L);
  }
  lua_rawset(L, LUA_GLOBALSINDEX);

  lua_pushliteral(L, "vfs");
  lua_newtable(L); {
    LuaVFS::PushEntries(L);
  }
  lua_rawset(L, LUA_GLOBALSINDEX);

  lua_pushliteral(L, "BZ");
  lua_newtable(L); {
    Constants::PushEntries(L);
  }
  lua_rawset(L, LUA_GLOBALSINDEX);

  if (luaL_dofile(L, script.c_str()) != 0) {
    debugf(0, "lua init error: %s\n", lua_tostring(L, -1));
    lua_pop(L, 1);
    return false;
  }

  return true;
}


//============================================================================//
//============================================================================//

static std::string EnvExpand(const std::string& path) {
  std::string::size_type pos = path.find('$');
  if (pos == std::string::npos) {
    return path;
  }

  if (path[pos + 1] == '$') { // allow $$ escapes
    return path.substr(0, pos + 1) + EnvExpand(path.substr(pos + 2));
  }

  const char* b = path.c_str(); // beginning of string
  const char* s = b + pos + 1;  // start of the key name
  const char* e = s;      // end   of the key Name
  while ((*e != 0) && (isalnum(*e) || (*e == '_'))) {
    e++;
  }

  const std::string head = path.substr(0, pos);
  const std::string key  = path.substr(pos + 1, e - s);
  const std::string tail = path.substr(e - b);

  const char* env = getenv(key.c_str());
  const std::string value = (env == NULL) ? "" : env;

  return head + value + EnvExpand(tail);
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
