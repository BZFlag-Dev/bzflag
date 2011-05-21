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
#include "LuaClientScripts.h"

// system headers
#include <string.h>
#include <string>
using std::string;

// common headers
#include "OpenGLUtils.h"
#include "TextUtils.h"
#include "SceneDatabase.h"
#include "StateDatabase.h"

// bzflag headers
#include "bzflag/guiplaying.h"
#include "bzflag/Downloads.h"
#include "clientbase/playing.h"
#include "clientbase/World.h"

// local headers
#include "LuaCallInDB.h"
#include "LuaOpenGL.h"
#include "LuaBZDB.h"
#include "LuaBzMaterial.h"
#include "LuaHTTP.h"
#include "LuaSceneNode.h"
#include "LuaServerPing.h"

// LuaHandle headers
#include "LuaUser.h"
#include "LuaBzOrg.h"
#include "LuaWorld.h"
#include "LuaRules.h"


//============================================================================//
//============================================================================//

void LuaClientScripts::Init() {
  luaCallInDB.Init();

  LuaOpenGL::Init();

  LuaHTTPMgr::SetAccessList(Downloads::instance().getAccessList());

  LuaBzMaterial::SetBlendParser(parseBlendFactors);
}


void LuaClientScripts::Free() {
  LuaUserFreeHandler();
  LuaBzOrgFreeHandler();
  LuaWorldFreeHandler();
  LuaRulesFreeHandler();

  LuaOpenGL::Free();

  LuaSceneNodeMgr::ClearSceneNodes();
}


void LuaClientScripts::Update() {
  LuaSceneNodeMgr::ClearSceneNodes();
  LuaClientScripts::LuaUserUpdate();
  LuaClientScripts::LuaBzOrgUpdate();
  LuaClientScripts::LuaWorldUpdate();
  LuaClientScripts::LuaRulesUpdate();
}


//============================================================================//
//============================================================================//

void LuaClientScripts::AddSceneNodes(SceneDatabase& scene) {
  LuaSceneNodeMgr::AddSceneNodes(scene);
}


//============================================================================//
//============================================================================//

void LuaClientScripts::LuaUserFreeHandler()  { LuaUser::FreeHandler();  }
void LuaClientScripts::LuaBzOrgFreeHandler() { LuaBzOrg::FreeHandler(); }
void LuaClientScripts::LuaWorldFreeHandler() { LuaWorld::FreeHandler(); }
void LuaClientScripts::LuaRulesFreeHandler() { LuaRules::FreeHandler(); }

void LuaClientScripts::LuaUserLoadHandler()  { LuaUser::LoadHandler();  }
void LuaClientScripts::LuaBzOrgLoadHandler() { LuaBzOrg::LoadHandler(); }
void LuaClientScripts::LuaWorldLoadHandler() { LuaWorld::LoadHandler(); }
void LuaClientScripts::LuaRulesLoadHandler() { LuaRules::LoadHandler(); }

bool LuaClientScripts::LuaUserIsActive()  { return (luaUser  != NULL);   }
bool LuaClientScripts::LuaBzOrgIsActive() { return LuaBzOrg::IsActive(); }
bool LuaClientScripts::LuaWorldIsActive() { return (luaWorld != NULL);   }
bool LuaClientScripts::LuaRulesIsActive() { return (luaRules != NULL);   }

bool LuaClientScripts::GetDevMode() { return LuaHandle::GetDevMode(); }
void LuaClientScripts::SetDevMode(bool value) { LuaHandle::SetDevMode(value); }


//============================================================================//
//============================================================================//

void LuaClientScripts::LuaUserUpdate() {
  LuaServerPingMgr::Update(); // NOTE: doesn't really belong here

  if (luaUser == NULL) {
    return;
  }
  else if (luaUser->RequestReload()) {
    string reason = luaUser->RequestMessage();
    if (!reason.empty()) { reason = ": " + reason; }

    LuaUser::FreeHandler();
    LuaUser::LoadHandler();

    if (luaUser != NULL) {
      showMessage("LuaUser reloaded" + reason);
    }
    else {
      showMessage("LuaUser reload failed" + reason);
    }
  }
  else if (luaUser->RequestDisable()) {
    string reason = luaUser->RequestMessage();
    if (!reason.empty()) { reason = ": " + reason; }

    LuaUser::FreeHandler();

    showMessage("LuaUser disabled" + reason);
  }
}


void LuaClientScripts::LuaBzOrgUpdate() {
  if (luaBzOrg == NULL) {
    return;
  }
  else if (luaBzOrg->RequestReload()) {
    string reason = luaBzOrg->RequestMessage();
    if (!reason.empty()) { reason = ": " + reason; }

    LuaBzOrg::FreeHandler();
    LuaBzOrg::LoadHandler();

    if (LuaBzOrg::IsActive()) {
      showMessage("LuaBzOrg reloading" + reason);
    }
    else {
      showMessage("LuaBzOrg reload failed" + reason);
    }
  }
  else if (luaBzOrg->RequestDisable()) {
    string reason = luaBzOrg->RequestMessage();
    if (!reason.empty()) { reason = ": " + reason; }

    LuaBzOrg::FreeHandler();

    showMessage("LuaBzOrg disabled" + reason);
  }
}


void LuaClientScripts::LuaWorldUpdate() {
  if (luaWorld == NULL) {
    return;
  }
  else if (luaWorld->RequestReload()) {
    string reason = luaWorld->RequestMessage();
    if (!reason.empty()) { reason = ": " + reason; }

    LuaWorld::FreeHandler();
    LuaWorld::LoadHandler();

    if (luaWorld != NULL) {
      showMessage("LuaWorld reloaded" + reason);
    }
    else {
      showMessage("LuaWorld reload failed" + reason);
    }
  }
  else if (luaWorld->RequestDisable()) {
    string reason = luaWorld->RequestMessage();
    if (!reason.empty()) { reason = ": " + reason; }

    LuaWorld::FreeHandler();

    showMessage("LuaWorld disabled" + reason);
  }
}


void LuaClientScripts::LuaRulesUpdate() {
  if (luaRules == NULL) {
    return;
  }
  else if (luaRules->RequestReload()) {
    string reason = luaRules->RequestMessage();
    if (!reason.empty()) { reason = ": " + reason; }

    LuaRules::FreeHandler();
    LuaRules::LoadHandler();

    if (luaRules != NULL) {
      showMessage("LuaRules reloaded" + reason);
    }
    else {
      showMessage("LuaRules reload failed" + reason);
    }
  }
  else if (luaRules->RequestDisable()) {
    string reason = luaRules->RequestMessage();
    if (!reason.empty()) { reason = ": " + reason; }

    LuaRules::FreeHandler();

    showMessage("LuaRules disabled" + reason);
  }
}


//============================================================================//
//============================================================================//

bool LuaClientScripts::LuaUserCommand(const std::string& cmdLine) {
  const string prefix = "luauser";
  const char* c = cmdLine.c_str();
  if (strncmp(c, prefix.c_str(), prefix.size()) != 0) {
    return false;
  }
  c = TextUtils::skipWhitespace(c + prefix.size());

  const string cmd = c;
  if (cmd == "reload") {
    LuaUser::FreeHandler();
    if (BZDB.isTrue("_forbidLuaUser")) {
      showMessage("This server forbids LuaUser scripts");
      return false;
    }
    else {
      LuaUser::LoadHandler();
    }
  }
  else if (cmd == "disable") {
    const bool active = (luaUser != NULL);
    LuaUser::FreeHandler();
    if (active) {
      showMessage("LuaUser disabled");
    }
  }
  else if (cmd == "status") {
    if (luaUser != NULL) {
      showMessage("LuaUser is enabled");
    }
    else {
      showMessage("LuaUser is disabled");
    }
  }
  else if (cmd == "") {
    addMessage(NULL,
               "/luauser < status | reload | disable | custom_command ... >");
  }
  else if (luaUser != NULL) {
    return luaUser->RecvCommand(c);
  }
  else {
    return false;
  }

  return true;
}


bool LuaClientScripts::LuaBzOrgCommand(const std::string& cmdLine) {
  const string prefix = "luabzorg";
  const char* c = cmdLine.c_str();
  if (strncmp(c, prefix.c_str(), prefix.size()) != 0) {
    return false;
  }
  c = TextUtils::skipWhitespace(c + prefix.size());

  const string cmd = c;
  if (cmd == "reload") {
    LuaBzOrg::FreeHandler();
    if (BZDB.isTrue("_forbidLuaBzOrg")) {
      showMessage("This server forbids LuaBzOrg scripts");
      return false;
    }
    else {
      LuaBzOrg::LoadHandler();
    }
  }
  else if (cmd == "disable") {
    const bool active = LuaBzOrg::IsActive();
    LuaBzOrg::FreeHandler();
    if (active) {
      showMessage("LuaBzOrg disabled");
    }
  }
  else if (cmd == "status") {
    if (luaBzOrg != NULL) {
      showMessage("LuaBzOrg is enabled");
    }
    else if (LuaBzOrg::IsActive()) {
      showMessage("LuaBzOrg is loading");
    }
    else {
      showMessage("LuaBzOrg is disabled");
    }
  }
  else if (cmd == "") {
    addMessage(NULL,
               "/luabzorg < status | reload | disable | custom_command ... >");
  }
  else if (luaBzOrg != NULL) {
    return luaBzOrg->RecvCommand(c);
  }
  else {
    return false;
  }

  return true;
}


bool LuaClientScripts::LuaWorldCommand(const std::string& cmdLine) {
  const World* world = World::getWorld();
  if (world == NULL) {
    return false;
  }

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
    const bool active = (luaWorld != NULL);
    LuaWorld::FreeHandler();
    if (active) {
      showMessage("LuaWorld disabled");
    }
  }
  else if (cmd == "status") {
    if (luaWorld != NULL) {
      showMessage("LuaWorld is enabled");
    }
    else {
      showMessage("LuaWorld is disabled");
    }
  }
  else if (cmd == "") {
    addMessage(NULL,
               "/luaworld < status | reload | disable | custom_command ... >");
  }
  else if (luaWorld != NULL) {
    return luaWorld->RecvCommand(c);
  }
  else {
    return false;
  }

  return true;
}


bool LuaClientScripts::LuaRulesCommand(const std::string& cmdLine) {
  const World* world = World::getWorld();
  if (world == NULL) {
    return false;
  }

  const string prefix = "luarules";
  const char* c = cmdLine.c_str();
  if (strncmp(c, prefix.c_str(), prefix.size()) != 0) {
    return false;
  }
  c = TextUtils::skipWhitespace(c + prefix.size());

  const string cmd = c;
  if (cmd == "reload") {
    if (!GetDevMode()) {
      showMessage("LuaRules is a required script");
      return false;
    }
    LuaRules::FreeHandler();
    LuaRules::LoadHandler();
  }
  else if (cmd == "disable") {
    if (!GetDevMode()) {
      showMessage("LuaRules is a required script");
      return false;
    }
    const bool active = (luaRules != NULL);
    LuaRules::FreeHandler();
    if (active) {
      showMessage("LuaRules disabled");
    }
  }
  else if (cmd == "status") {
    if (luaRules != NULL) {
      showMessage("LuaRules is enabled");
    }
    else {
      showMessage("LuaRules is disabled");
    }
  }
  else if (cmd == "") {
    addMessage(NULL,
               "/luarules < status | reload | disable | custom_command ... >");
  }
  else if (luaRules != NULL) {
    if (!GetDevMode()) {
      showMessage("LuaRules can not receive commands");
      return false;
    }
    return luaRules->RecvCommand(c);
  }
  else {
    return false;
  }

  return true;
}


//============================================================================//
//============================================================================//

void LuaClientScripts::LuaUserUpdateForbidden() {
  if (luaUser == NULL) {
    return;
  }
  if (BZDB.isTrue("_forbidLuaUser")) {
    LuaUser::FreeHandler();
    showMessage("This server forbids LuaUser scripts");
    return;
  }
}


void LuaClientScripts::LuaBzOrgUpdateForbidden() {
  if (!LuaBzOrg::IsActive()) {
    return;
  }
  if (BZDB.isTrue("_forbidLuaBzOrg")) {
    LuaBzOrg::FreeHandler();
    showMessage("This server forbids LuaBzOrg scripts");
    return;
  }
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
