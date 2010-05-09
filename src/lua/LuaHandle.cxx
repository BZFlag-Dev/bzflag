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
#include "LuaHandle.h"

// system headers
#include <assert.h>
#include <string>
#include <vector>
using std::string;
using std::vector;

// common headers
#include "bzfio.h"
#include "BzVFS.h"
#include "EventHandler.h"

// local headers
#include "LuaHeader.h"

#include "LuaClientOrder.h"
#include "LuaCallInCheck.h"
#include "LuaCallInDB.h"
#include "LuaUtils.h"
#include "LuaExtras.h"

// local lua library headers
#include "LuaBitOps.h"
#include "LuaBZDB.h"
#include "LuaBzMaterial.h"
#include "LuaCallOuts.h"
#include "LuaConsole.h"
#include "LuaGameConst.h"
#include "LuaGLConst.h"
#include "LuaControl.h"
#include "LuaDouble.h"
#include "LuaDynCol.h"
#include "LuaExtras.h"
#include "LuaKeySyms.h"
#include "LuaObstacle.h"
#include "LuaOpenGL.h"
#include "LuaPack.h"
#include "LuaPhyDrv.h"
#include "LuaSceneNode.h"
#include "LuaScream.h"
#include "LuaServerPing.h"
#include "LuaSpatial.h"
#include "LuaTexMat.h"
#include "LuaHTTP.h"
#include "LuaVector.h"
#include "LuaVFS.h"
#include "LuaZip.h"


bool LuaHandle::devMode = false;


//============================================================================//
//============================================================================//

LuaHandle::LuaHandle(const string& _name, int16_t _scriptID,
		     int gameState, int drawWorld, int drawScreen,
		     bool _fullRead, bool _gameCtrl, bool _inputCtrl)
: EventClient(_name, _scriptID,
	      gameState, drawWorld, drawScreen,
	      _fullRead, _gameCtrl, _inputCtrl)
, requestReload  (false)
, requestDisable (false)
{
  assert(BZ_LUA_EXTRASPACE >= sizeof(LuaExtraSpace));

  L = lua_open();
  if (L == NULL) {
    return;
  }

  L2ES(L)->handle = this;

  SetupValidCallIns();

  AddBasicCalls();

  lua_pushvalue(L, LUA_GLOBALSINDEX);
  LuaExtras::PushEntries(L);
  lua_pop(L, 1);

  if (devMode) {
    lua_register(L, "dump",    LuaExtras::dump);
    lua_register(L, "listing", LuaExtras::listing);
  }
}


LuaHandle::~LuaHandle()
{
  eventHandler.RemoveClient(this);

  // free the lua state
  KillLua();

/*
  string msg = GetName();
  if (!requestMessage.empty()) {
    msg += ": " + requestMessage;
  }
  LuaLog(1, "Disabled %s", msg.c_str());
*/
}


void LuaHandle::KillLua()
{
  if (L != NULL) {
    lua_close(L);
  }
  L = NULL;
}


void LuaHandle::SetupValidCallIns()
{
  validCallIns.clear();

  // setup the validCallIns set
  const LuaCallInDB::InfoMap& ciInfoMap = luaCallInDB.GetInfoMap();
  LuaCallInDB::InfoMap::const_iterator it;
  for (it = ciInfoMap.begin(); it != ciInfoMap.end(); ++it) {
    const LuaCallInDB::CallInInfo& ciInfo = it->second;
    if (!ciInfo.singleScript.empty() &&
	(ciInfo.singleScript != GetName())) {
      continue;
    }
    if ((ciInfo.reqFullRead  && !HasFullRead()) ||
	(ciInfo.reqGameCtrl  && !HasGameCtrl()) ||
	(ciInfo.reqInputCtrl && !HasInputCtrl())) {
      continue;
    }
    validCallIns.insert(luaCallInDB.GetCode(ciInfo.name));
  }
}


//============================================================================//
//============================================================================//

static void CheckEqualStack(const LuaHandle* lh, lua_State* L, int top,
			    const char* tableName)
{
  if (top != lua_gettop(L)) {
    string msg = __FUNCTION__;
    msg += " : " + lh->GetName() + " : ";
    msg += tableName;
    LuaLog(0, "ERROR: %s has an unequal stack\n", msg.c_str());
  }
}


bool LuaHandle::PushLib(const char* name, bool (*entriesFunc)(lua_State*))
{
  const int top = lua_gettop(L);
  lua_pushstring(L, name);
  lua_rawget(L, -2);
  if (lua_istable(L, -1)) {
    const bool success = entriesFunc(L);
    lua_pop(L, 1);
    CheckEqualStack(this, L, top, name);
    return success;
  }
  lua_pop(L, 1);

  // make a new table
  lua_pushstring(L, name);
  lua_newtable(L);
  const bool success = entriesFunc(L);
  if (!success) {
    lua_pop(L, 2);
    CheckEqualStack(this, L, top, name);
    return false;
  }
  lua_rawset(L, -3);
  CheckEqualStack(this, L, top, name);
  return true;
}


string LuaHandle::LoadSourceCode(const string& sourceFile,
				 const string& sourceModes)
{
  string modes = sourceModes;
  if (devMode) {
    modes = string(BZVFS_LUA_USER) + modes;
  }
  string code;
  if (!bzVFS.readFile(sourceFile, modes, code)) {
    LuaLog(0, "FAILED to load  '%s'  with  '%s'\n",
	   sourceFile.c_str(), modes.c_str());
    return "";
  }
  return code;
}


bool LuaHandle::ExecSourceCode(const string& code, const string& label)
{
  int error = luaL_loadbuffer(L, code.c_str(), code.size(), label.c_str());

  if (error != 0) {
    LuaLog(0, "Lua LoadCode loadbuffer error = %i, %s, %s\n",
	   error, GetName().c_str(), lua_tostring(L, -1));
    lua_pop(L, 1);
    return false;
  }

  error = lua_pcall(L, 0, 0, 0);

  if (error != 0) {
    LuaLog(0, "Lua LoadCode pcall error(%i), %s, %s\n",
	   error, GetName().c_str(), lua_tostring(L, -1));
    lua_pop(L, 1);
    return false;
  }

  return true;
}


//============================================================================//
//============================================================================//

void LuaHandle::CheckStack()
{
  const int top = lua_gettop(L);
  if (top != 0) {
    LuaLog(0, "WARNING: %s stack check: top = %i\n", GetName().c_str(), top);
    lua_settop(L, 0);
  }
}


//============================================================================//
//============================================================================//

bool LuaHandle::CanUseCallIn(int code) const
{
  return (validCallIns.find(code) != validCallIns.end());
}


bool LuaHandle::CanUseCallIn(const string& ciName) const
{
  const int code = luaCallInDB.GetCode(luaCallInDB.GetCallInName(ciName));
  if (code == 0)  {
    return false;
  }
  return (validCallIns.find(code) != validCallIns.end());
}


bool LuaHandle::UpdateCallIn(const string& ciName, bool state)
{
  const string& eventName = luaCallInDB.GetEventName(ciName);
  if (state) {
    return eventHandler.InsertEvent(this, eventName);
  } else {
    return eventHandler.RemoveEvent(this, eventName);
  }
}


//============================================================================//
//============================================================================//

static void AddCallInError(lua_State* L, const string& funcName)
{
  // error string is on the top of the stack
  lua_checkstack(L, 4);

  lua_pushliteral(L, "ERRORS");
  lua_rawget(L, LUA_GLOBALSINDEX);
  if (!lua_istable(L, -1)) {
    lua_pop(L, 1);
    lua_createtable(L, 0, 0);
    lua_pushliteral(L, "ERRORS");
    lua_pushvalue(L, -2); // make a copy of {ERRORS}
    lua_rawset(L, LUA_GLOBALSINDEX);
  }

  const size_t len = lua_objlen(L, -1);
  if (len >= 1024) {
    lua_pop(L, 2);
    return;
  }

  lua_createtable(L, 0, 2); {
    lua_pushliteral(L, "func");
    lua_pushstdstring(L, funcName);
    lua_rawset(L, -3);
    lua_pushliteral(L, "error");
    lua_pushvalue(L, -4);
    lua_rawset(L, -3);
  }
  lua_rawseti(L, -2, len + 1);

  lua_pop(L, 2); // also pop the message
}


bool LuaHandle::RunCallIn(int ciCode, int inArgs, int outArgs)
{
  int error;

  error = lua_pcall(L, inArgs, outArgs, 0);

  if (error != 0) {
    // log the error
    const string* ciName = luaCallInDB.GetName(ciCode);
    const char* ciNameStr = ciName ? ciName->c_str() : "UNKNOWN";
    LuaLog(0, "%s::RunCallIn: error = %i, %s, %s\n",
	   GetName().c_str(), error, ciNameStr, lua_tostring(L, -1));
    // move the error string into CALLIN_ERRORS
    AddCallInError(L, ciNameStr);
    return false;
  }

  return true;
}


//============================================================================//

bool LuaHandle::PushCallIn(int ciCode)
{
  lua_rawgeti(L, LUA_CALLINSINDEX, ciCode);
  if (lua_isfunction(L, -1)) {
    return true;
  }
  const string* name = luaCallInDB.GetName(ciCode);
  printf("Failed to get: %i %s\n", ciCode, name ? name->c_str() : "UNKNOWN");
  lua_pop(L, 1);
  return false;
}


//============================================================================//
//============================================================================//
//
//  Call-ins
//

//============================================================================//
//============================================================================//

bool LuaHandle::AddBasicCalls()
{
  lua_newtable(L); {
    luaset_strfunc(L, "Reload",	ScriptReload);
    luaset_strfunc(L, "Disable",       ScriptDisable);

    luaset_strfunc(L, "GetID",	 ScriptGetID);
    luaset_strfunc(L, "GetName",       ScriptGetName);

    luaset_strfunc(L, "HasFullRead",   ScriptHasFullRead);
    luaset_strfunc(L, "HasGameCtrl",   ScriptHasGameCtrl);
    luaset_strfunc(L, "HasInputCtrl",  ScriptHasInputCtrl);

    luaset_strfunc(L, "GetCallInInfo", ScriptGetCallInInfo);
    luaset_strfunc(L, "CanUseCallIn",  ScriptCanUseCallIn);
    luaset_strfunc(L, "SetCallIn",     ScriptSetCallIn);

    luaset_strfunc(L, "GetDevMode",    ScriptGetDevMode);
    luaset_strfunc(L, "GetGLOBALS",    ScriptGetGLOBALS);
    luaset_strfunc(L, "GetCALLINS",    ScriptGetCALLINS);
    luaset_strfunc(L, "GetREGISTRY",   ScriptGetREGISTRY);

    luaset_strfunc(L, "PrintPointer",  ScriptPrintPointer);
    luaset_strfunc(L, "PrintGCInfo",   ScriptPrintGCInfo);

    lua_pushliteral(L, "ID");
    lua_newtable(L); {
      LuaSetDualPair(L, "LuaUser",  LUA_USER_SCRIPT_ID);
      LuaSetDualPair(L, "LuaBzOrg", LUA_BZORG_SCRIPT_ID);
      LuaSetDualPair(L, "LuaWorld", LUA_WORLD_SCRIPT_ID);
      LuaSetDualPair(L, "LuaRules", LUA_RULES_SCRIPT_ID);
    }
    lua_rawset(L, -3);
  }
  lua_setglobal(L, "script");

  return true;
}


//============================================================================//

int LuaHandle::ScriptDisable(lua_State* L)
{
  const int args = lua_gettop(L);
  if ((args >= 1) && lua_israwstring(L, 1)) {
    L2H(L)->requestMessage = lua_tostring(L, 1);
  }
  L2H(L)->requestDisable = true;
  return 0;
}


int LuaHandle::ScriptReload(lua_State* L)
{
  const int args = lua_gettop(L);
  if ((args >= 1) && lua_israwstring(L, 1)) {
    L2H(L)->requestMessage = lua_tostring(L, 1);
  }
  L2H(L)->requestReload = true;
  return 0;
}


//============================================================================//

int LuaHandle::ScriptPrintPointer(lua_State* L)
{
  const string prefix = luaL_optstring(L, 2, "PrintPointer: ");
  LuaLog(0, "%s%p\n", prefix.c_str(), lua_topointer(L, 1));
  return 0;
}


int LuaHandle::ScriptPrintGCInfo(lua_State* L)
{
  LuaLog(0, "GCInfo: %.3f MBytes\n", (float)lua_getgccount(L) / 1024.0f);
  return 0;
}


//============================================================================//

int LuaHandle::ScriptGetID(lua_State* L)
{
  if (lua_gettop(L) == 0) {
    lua_pushinteger(L, L2H(L)->GetScriptID());
  }
  else if (lua_israwstring(L, 1)) {
    const string key = lua_tostring(L, 1);
    if (key == "LuaUser") {
      lua_pushinteger(L, LUA_USER_SCRIPT_ID);
    }
    else if (key == "LuaBzOrg") {
      lua_pushinteger(L, LUA_BZORG_SCRIPT_ID);
    }
    else if (key == "LuaWorld") {
      lua_pushinteger(L, LUA_WORLD_SCRIPT_ID);
    }
    else if (key == "LuaRules") {
      lua_pushinteger(L, LUA_RULES_SCRIPT_ID);
    }
    else {
      return luaL_pushnil(L);
    }
  }
  else {
    luaL_error(L, "invalid argument");
  }
  return 1;
}


int LuaHandle::ScriptGetName(lua_State* L)
{
  lua_pushstring(L, L2H(L)->GetName().c_str());
  return 1;
}


int LuaHandle::ScriptHasFullRead(lua_State* L)
{
  lua_pushboolean(L, L2H(L)->HasFullRead());
  return 1;
}


int LuaHandle::ScriptHasGameCtrl(lua_State* L)
{
  lua_pushboolean(L, L2H(L)->HasGameCtrl());
  return 1;
}


int LuaHandle::ScriptHasInputCtrl(lua_State* L)
{
  lua_pushboolean(L, L2H(L)->HasInputCtrl());
  return 1;
}


int LuaHandle::ScriptGetDevMode(lua_State* L)
{
  lua_pushboolean(L, devMode);
  return 1;
}


int LuaHandle::ScriptGetGLOBALS(lua_State* L)
{
  if (!devMode) {
    return luaL_pushnil(L);
  }
  lua_pushvalue(L, LUA_GLOBALSINDEX);
  return 1;
}


int LuaHandle::ScriptGetCALLINS(lua_State* L)
{
  if (!devMode) {
    return luaL_pushnil(L);
  }
  lua_pushvalue(L, LUA_CALLINSINDEX);
  return 1;
}


int LuaHandle::ScriptGetREGISTRY(lua_State* L)
{
  if (!devMode) {
    return luaL_pushnil(L);
  }
  lua_pushvalue(L, LUA_REGISTRYINDEX);
  return 1;
}


//============================================================================//

static void PushCallInInfo(lua_State* L, const LuaCallInDB::CallInInfo& ciInfo)
{
  lua_newtable(L);
  luaset_strint   (L, "code",	 ciInfo.code);
  luaset_strbool  (L, "reqFullRead",  ciInfo.reqFullRead);
  luaset_strbool  (L, "reqInputCtrl", ciInfo.reqInputCtrl);
  luaset_strbool  (L, "reversed",     ciInfo.reversed);
  luaset_strbool  (L, "reentrant",    ciInfo.reentrant);
  luaset_strstr(L, "loopType",     ciInfo.loopType);
  if (LuaHandle::GetDevMode()) {
    lua_pushliteral(L, "func");
    lua_rawgeti(L, LUA_CALLINSINDEX, ciInfo.code);
    if (!lua_isfunction(L, -1)) {
      lua_pop(L, 1);
      lua_pushboolean(L, false);
    }
    lua_rawset(L, -3);
  }
}


int LuaHandle::ScriptGetCallInInfo(lua_State* L)
{
  vector<string> list;
  eventHandler.GetEventList(list);

  const LuaCallInDB::InfoMap& ciInfoMap = luaCallInDB.GetInfoMap();
  LuaCallInDB::InfoMap::const_iterator it;

  if (lua_israwstring(L, 1)) {
    const string ciName = lua_tostring(L, 1);
    const bool wantAll = lua_isboolean(L, 2) && lua_tobool(L, 2);
    if (wantAll || L2H(L)->CanUseCallIn(ciName)) {
      it = ciInfoMap.find(ciName);
      if (it != ciInfoMap.end()) {
	PushCallInInfo(L, it->second);
      }
    } else {
      return luaL_pushnil(L);
    }
  }
  else {
    const bool wantAll = lua_isboolean(L, 1) && lua_tobool(L, 1);
    lua_createtable(L, 0, ciInfoMap.size());
    for (it = ciInfoMap.begin(); it != ciInfoMap.end(); ++it) {
      const LuaCallInDB::CallInInfo& ciInfo = it->second;
      if (wantAll || L2H(L)->CanUseCallIn(ciInfo.name)) {
	lua_pushstring(L, ciInfo.name.c_str());
	PushCallInInfo(L, ciInfo);
	lua_rawset(L, -3);
      }
    }
  }

  return 1;
}


int LuaHandle::ScriptCanUseCallIn(lua_State* L)
{
  const string ciName = lua_tostring(L, 1);
  lua_pushboolean(L, L2H(L)->CanUseCallIn(ciName));
  return 1;
}


int LuaHandle::ScriptSetCallIn(lua_State* L)
{
  const string ciName = luaL_checkstring(L, 1);

  const int ciCode = luaCallInDB.GetCode(ciName);
  if (ciCode == 0) {
    if (devMode) {
      LuaLog(1, "Request to update an Unknown call-in (%s)\n", ciName.c_str());
    }
    return luaL_pushnil(L);
  }

  if (!L2H(L)->CanUseCallIn(ciName)) {
    return luaL_pushnil(L);
  }

  lua_settop(L, 2);
  const bool haveFunc = lua_isfunction(L, 2);

  lua_checkstack(L, 4);

  lua_pushvalue(L, 2); // make a copy
  lua_rawseti(L, LUA_CALLINSINDEX, ciCode);

  lua_pushstring(L, ciName.c_str());
  lua_pushvalue(L, 2); // make a copy
  lua_rawset(L, LUA_CALLINSINDEX);

  const string& eventName = luaCallInDB.GetEventName(ciName);
  if (eventHandler.IsManaged(eventName)) {
    lua_pushboolean(L, L2H(L)->UpdateCallIn(ciName, haveFunc));
  } else {
    lua_pushboolean(L, true);
  }

  return 1;
}


//============================================================================//
//============================================================================//

#define LUA_OPEN_LIB(L, lib) \
  lua_pushcfunction((L), (lib)); \
  lua_pcall((L), 0, 0, 0);


bool LuaHandle::SetupEnvironment()
{
  // load the standard libraries
  LUA_OPEN_LIB(L, luaopen_base);
  LUA_OPEN_LIB(L, luaopen_math);
  LUA_OPEN_LIB(L, luaopen_table);
  LUA_OPEN_LIB(L, luaopen_string);
  LUA_OPEN_LIB(L, luaopen_lpeg);
  if (devMode) {
    LUA_OPEN_LIB(L, luaopen_debug);
  }
  //
  // disabled libraries  {io}, {os}, and {package}
  // NOTE: if {io} is added, disable io.popen()
  //				 ^^^^^^^^^^
  //LUA_OPEN_LIB(L, luaopen_io);
  //LUA_OPEN_LIB(L, luaopen_os);
  //LUA_OPEN_LIB(L, luaopen_package);

  // disable some global functions
  // (these use stdio calls to access file data)
  lua_pushnil(L); lua_setglobal(L, "dofile");
  lua_pushnil(L); lua_setglobal(L, "loadfile");

  // push the bzflag additions
  lua_pushvalue(L, LUA_GLOBALSINDEX); {
    // into the global table
    if (!LuaExtras::PushEntries(L) ||
	!LuaDouble::PushEntries(L)) {
      lua_pop(L, 1);
      return false;
    }
    // into sub-tables (using PushLib())
    if (!PushLib("math",   LuaBitOps::PushEntries)       ||
	!PushLib("math",   LuaVector::PushEntries)       ||
	!PushLib("http",   LuaHTTPMgr::PushEntries)      ||
	!PushLib("vfs",    LuaVFS::PushEntries)          ||
	!PushLib("bzdb",   LuaBZDB::PushEntries)         ||
	!PushLib("script", LuaScream::PushEntries)       ||
	!PushLib("gl",     LuaOpenGL::PushEntries)       ||
	!PushLib("GL",     LuaGLConst::PushEntries)      ||
	!PushLib("bz",     LuaZip::PushEntries)          ||
	!PushLib("bz",     LuaPack::PushEntries)         ||
	!PushLib("bz",     LuaCallOuts::PushEntries)     ||
	!PushLib("bz",     LuaConsole::PushEntries)      ||
	!PushLib("bz",     LuaSceneNodeMgr::PushEntries) ||
	!PushLib("BZ",     LuaKeySyms::PushEntries)      ||
	!PushLib("BZ",     LuaGameConst::PushEntries))    {
      lua_pop(L, 1);
      return false;
    }
    if (HasFullRead()) {
      if (!PushLib("bz", LuaBzMaterial::PushEntries) ||
	  !PushLib("bz", LuaDynCol::PushEntries)     ||
	  !PushLib("bz", LuaTexMat::PushEntries)     ||
	  !PushLib("bz", LuaPhyDrv::PushEntries)     ||
	  !PushLib("bz", LuaSpatial::PushEntries)    ||
	  !PushLib("bz", LuaObstacle::PushEntries)) {
	lua_pop(L, 1);
	return false;
      }
    }
    if (HasInputCtrl()) {
      if (!PushLib("control", LuaControl::PushEntries)) {
	return false;
      }
    }
    if (GetName() == "LuaBzOrg") {
      if (!PushLib("bz", LuaServerPingMgr::PushEntries)) {
	return false;
      }
    }
  }
  lua_pop(L, 1); // LUA_GLOBALSINDEX

  // setup the error function
  lua_getglobal(L, "traceback");
  lua_seterrorfunc(L, luaL_ref(L, LUA_REGISTRYINDEX));

  return true;
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
