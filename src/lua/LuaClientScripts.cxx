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

#include "common.h"

// implementation header
#include "LuaClientScripts.h"

// system headers
#include <string.h>
#include <string>
using std::string;

// common headers
#include "TextUtils.h"
#include "StateDatabase.h"

// bzflag headers
#include "../bzflag/playing.h"

// local headers
#include "LuaUser.h"
#include "LuaWorld.h"
#include "LuaOpenGL.h"

/******************************************************************************/
/******************************************************************************/

void LuaClientScripts::LuaOpenGLInit() { LuaOpenGL::Init(); }
void LuaClientScripts::LuaOpenGLFree() { LuaOpenGL::Free(); }

void LuaClientScripts::LuaUserFreeHandler()  { LuaUser::FreeHandler();  }
void LuaClientScripts::LuaWorldFreeHandler() { LuaWorld::FreeHandler(); }

void LuaClientScripts::LuaUserLoadHandler()  { LuaUser::LoadHandler();  }
void LuaClientScripts::LuaWorldLoadHandler() { LuaWorld::LoadHandler(); }

bool LuaClientScripts::LuaUserIsActive()  { return (luaUser != NULL);  }
bool LuaClientScripts::LuaWorldIsActive() { return (luaWorld != NULL); }

bool LuaClientScripts::GetDevMode()         { return LuaHandle::GetDevMode(); }
void LuaClientScripts::SetDevMode(bool val) { LuaHandle::SetDevMode(val);     }


/******************************************************************************/
/******************************************************************************/

void LuaClientScripts::LuaUserUpdate()
{
  if (luaUser == NULL) {
    return;
  }
  else if (luaUser->RequestReload()) {
    string reason = luaUser->RequestMessage();
    if (!reason.empty()) { reason = ": " + reason; }

    LuaUser::FreeHandler();
    LuaUser::LoadHandler();

    if (luaUser != NULL) {
      addMessage(NULL, "LuaUser reloaded" + reason);
    } else {
      addMessage(NULL, "LuaUser reload failed" + reason);
    }
  }
  else if (luaUser->RequestDisable()) {
    string reason = luaUser->RequestMessage();
    if (!reason.empty()) { reason = ": " + reason; }

    LuaUser::FreeHandler();

    addMessage(NULL, "LuaUser disabled" + reason);
  }
}


void LuaClientScripts::LuaWorldUpdate()
{
  if (luaWorld == NULL) {
    return;
  }
  else if (luaWorld->RequestReload()) {
    string reason = luaWorld->RequestMessage();
    if (!reason.empty()) { reason = ": " + reason; }

    LuaWorld::FreeHandler();
    LuaWorld::LoadHandler();

    if (luaWorld != NULL) {
      addMessage(NULL, "LuaWorld reloaded" + reason);
    } else {
      addMessage(NULL, "LuaWorld reload failed" + reason);
    }
  }
  else if (luaWorld->RequestDisable()) {
    string reason = luaWorld->RequestMessage();
    if (!reason.empty()) { reason = ": " + reason; }

    LuaWorld::FreeHandler();

    addMessage(NULL, "LuaWorld disabled" + reason);
  }
}


/******************************************************************************/
/******************************************************************************/

bool LuaClientScripts::LuaUserCommand(const std::string& cmdLine)
{
  const string prefix = "luauser";
  const char* c = cmdLine.c_str();
  if (strncmp(c, prefix.c_str(), prefix.size()) != 0) {
    return false;
  }
  c = TextUtils::skipWhitespace(c + prefix.size());

  const string cmd = c;
  if (cmd == "reload") {
    LuaUser::FreeHandler();
    if (BZDB.isTrue(StateDatabase::BZDB_FORBIDLUAUSER)) {
      addMessage(NULL, "This server forbids LuaUser scripts");
      return false;
    } else {
      LuaUser::LoadHandler();
    }
    LuaUser::LoadHandler();
  }
  else if (cmd == "disable") {
    LuaUser::FreeHandler();
  }
  else if (cmd == "status") {
    if (luaUser != NULL) {
      addMessage(NULL, "LuaUser is enabled");
    } else {
      addMessage(NULL, "LuaUser is disabled");
    }
  }
  else if (luaUser != NULL) {
    luaUser->RecvCommand(c);
  }
  else {
    return false;
  }

  return true;
}


bool LuaClientScripts::LuaWorldCommand(const std::string& cmdLine)
{
  const string prefix = "luaworld";
  const char* c = cmdLine.c_str();
  if (strncmp(c, prefix.c_str(), prefix.size()) != 0) {
    return false;
  }
  c = TextUtils::skipWhitespace(c + prefix.size());

  const string cmd = c;
  if (cmd == "reload") {
    LuaWorld::FreeHandler();
    LuaWorld::LoadHandler();
  }
  else if (cmd == "disable") {
    if (BZDB.isTrue("_forceLuaWorld")) {
      return false;
    }
    LuaWorld::FreeHandler();
  }
  else if (cmd == "status") {
    if (luaWorld != NULL) {
      addMessage(NULL, "LuaWorld is enabled");
    } else {
      addMessage(NULL, "LuaWorld is disabled");
    }
  }
  else if (luaWorld != NULL) {
    luaWorld->RecvCommand(c);
  }
  else {
    return false;
  }

  return true; 
}

/******************************************************************************/
/******************************************************************************/


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
