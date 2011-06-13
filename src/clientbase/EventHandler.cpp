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

#include "common.h"

// implementation header
#include "EventHandler.h"

// system headers
#include <assert.h>
#include <algorithm>

// common headers
#include "clientbase/GfxBlock.h"
#include "common/bzfio.h"
#include "ogl/OpenGLPassState.h"


EventHandler eventHandler;


//============================================================================//
//============================================================================//

// it's easier to read with named booleans
static const int REQ_FULL_READ  = (1 << 0);
static const int REQ_GAME_CTRL  = (1 << 1);
static const int REQ_INPUT_CTRL = (1 << 2);
static const int REVERSED       = (1 << 3);
static const int REENTRANT      = (1 << 4);


void EventHandler::SetupEvent(const std::string& eName, EventClientList* ecList,
                              int orderType, LoopType loopType, int bits) {
  assert(eventMap.find(eName) == eventMap.end());
  const bool reqFullRead  = ((bits & REQ_FULL_READ)  ? true : false);
  const bool reqGameCtrl  = ((bits & REQ_GAME_CTRL)  ? true : false);
  const bool reqInputCtrl = ((bits & REQ_INPUT_CTRL) ? true : false);
  const bool reversed     = ((bits & REVERSED)       ? true : false);
  const bool reentrant    = ((bits & REENTRANT)      ? true : false);

  ecList->set_order_type(orderType);
  ecList->set_reversed(reversed);

  eventMap[eName] = EventInfo(eName, ecList,
                              reqFullRead, reqGameCtrl, reqInputCtrl,
                              reversed, reentrant, loopType);
}


#define SETUP_EVENT(name, orderType, loopType, bits) \
  SetupEvent(#name, &list ## name, orderType, loopType, bits)


//============================================================================//
//============================================================================//

EventHandler::EventHandler() {
  const int ScriptIDOrder   = EventClient::ScriptIDOrder;
  const int GameStateOrder  = EventClient::GameStateOrder;
  const int DrawWorldOrder  = EventClient::DrawWorldOrder;
  const int DrawScreenOrder = EventClient::DrawScreenOrder;

  SETUP_EVENT(Update,            ScriptIDOrder,   Basic, 0);

  SETUP_EVENT(BZDBChange,        GameStateOrder,  Basic, REENTRANT);

  SETUP_EVENT(CommandFallback,   DrawScreenOrder, FirstTrue, 0);

  SETUP_EVENT(RecvChatMsg,       ScriptIDOrder,   FirstTrue, 0);
  SETUP_EVENT(RecvLuaData,       ScriptIDOrder,   FirstTrue, 0);

  SETUP_EVENT(ServerJoined,      GameStateOrder,  Basic, 0);
  SETUP_EVENT(ServerParted,      GameStateOrder,  Basic, 0);

  SETUP_EVENT(PlayerAdded,       GameStateOrder,  Basic, 0);
  SETUP_EVENT(PlayerRemoved,     GameStateOrder,  Basic, 0);
  SETUP_EVENT(PlayerSpawned,     GameStateOrder,  Basic, 0);
  SETUP_EVENT(PlayerKilled,      GameStateOrder,  Basic, 0);
  SETUP_EVENT(PlayerJumped,      GameStateOrder,  Basic, REQ_FULL_READ);
  SETUP_EVENT(PlayerLanded,      GameStateOrder,  Basic, REQ_FULL_READ);
  SETUP_EVENT(PlayerTeleported,  GameStateOrder,  Basic, REQ_FULL_READ);
  SETUP_EVENT(PlayerTeamChange,  GameStateOrder,  Basic, 0);
  SETUP_EVENT(PlayerScoreChange, GameStateOrder,  Basic, 0);

  SETUP_EVENT(ShotAdded,         GameStateOrder,  Basic, REQ_FULL_READ);
  SETUP_EVENT(ShotRemoved,       GameStateOrder,  Basic, REQ_FULL_READ);
  SETUP_EVENT(ShotRicochet,      GameStateOrder,  Basic, REQ_FULL_READ);
  SETUP_EVENT(ShotTeleported,    GameStateOrder,  Basic, REQ_FULL_READ);

  SETUP_EVENT(FlagAdded,         GameStateOrder,  Basic, 0);
  SETUP_EVENT(FlagRemoved,       GameStateOrder,  Basic, 0);
  SETUP_EVENT(FlagGrabbed,       GameStateOrder,  Basic, 0);
  SETUP_EVENT(FlagDropped,       GameStateOrder,  Basic, 0);
  SETUP_EVENT(FlagCaptured,      GameStateOrder,  Basic, 0);
  SETUP_EVENT(FlagTransferred,   GameStateOrder,  Basic, 0);

  SETUP_EVENT(GLResize,          ScriptIDOrder,   Basic, 0);
  SETUP_EVENT(GLContextInit,     ScriptIDOrder,   Basic, 0);
  SETUP_EVENT(GLContextFree,     ScriptIDOrder,   Basic, 0);
  SETUP_EVENT(GLUnmapped,        ScriptIDOrder,   Basic, 0);

  SETUP_EVENT(DrawGenesis,       DrawScreenOrder, Basic, REVERSED);
  SETUP_EVENT(DrawWorldStart,    DrawWorldOrder,  Basic, REVERSED);
  SETUP_EVENT(DrawWorld,         DrawWorldOrder,  Basic, REVERSED);
  SETUP_EVENT(DrawWorldAlpha,    DrawWorldOrder,  Basic, REVERSED);
  SETUP_EVENT(DrawWorldShadow,   DrawWorldOrder,  Basic, REVERSED);
  SETUP_EVENT(DrawScreenStart,   DrawScreenOrder, Basic, REVERSED);
  SETUP_EVENT(DrawScreen,        DrawScreenOrder, Basic, REVERSED);
  SETUP_EVENT(DrawRadar,         DrawScreenOrder, Basic, REVERSED);

  SETUP_EVENT(KeyPress,          ScriptIDOrder,   BooleanOR,   REQ_INPUT_CTRL);
  SETUP_EVENT(KeyRelease,        ScriptIDOrder,   BooleanOR,   REQ_INPUT_CTRL);
  SETUP_EVENT(UnicodeText,       ScriptIDOrder,   BooleanOR,   REQ_INPUT_CTRL);
  SETUP_EVENT(MousePress,        ScriptIDOrder,   BooleanOR,   REQ_INPUT_CTRL);
  SETUP_EVENT(MouseMove,         ScriptIDOrder,   BooleanOR,   REQ_INPUT_CTRL);
  SETUP_EVENT(MouseRelease,      ScriptIDOrder,   BooleanOR,   REQ_INPUT_CTRL);
  SETUP_EVENT(MouseWheel,        ScriptIDOrder,   BooleanOR,   REQ_INPUT_CTRL);
  SETUP_EVENT(IsAbove,           ScriptIDOrder,   FirstTrue,   REQ_INPUT_CTRL);
  SETUP_EVENT(GetTooltip,        ScriptIDOrder,   FirstString, REQ_INPUT_CTRL);
  SETUP_EVENT(WordComplete,      ScriptIDOrder,   Special,     REQ_INPUT_CTRL);

  SETUP_EVENT(ForbidSpawn,       GameStateOrder,  FirstTrue, REQ_GAME_CTRL);
  SETUP_EVENT(ForbidJump,        GameStateOrder,  FirstTrue, REQ_GAME_CTRL);
  SETUP_EVENT(ForbidFlagDrop,    GameStateOrder,  FirstTrue, REQ_GAME_CTRL);
  SETUP_EVENT(ForbidShot,        GameStateOrder,  FirstTrue, REQ_GAME_CTRL);
  SETUP_EVENT(ForbidShotLock,    GameStateOrder,  FirstTrue, REQ_GAME_CTRL);
  SETUP_EVENT(ForbidShotHit,     GameStateOrder,  FirstTrue, REQ_GAME_CTRL);
}


EventHandler::~EventHandler() {
}


//============================================================================//
//============================================================================//

void EventHandler::Purify() {
  if (!dirtyLists.empty()) {
    debugf(3, "EventHandler::Purify() purifying %i list\n",
           (int)dirtyLists.size());
  }
  std::set<EventClientList*>::iterator it;
  for (it = dirtyLists.begin(); it != dirtyLists.end(); ++it) {
    (*it)->purify();
  }
  dirtyLists.clear();
}


void EventHandler::AddClient(EventClient* ec) {
  if (ec->GetName().empty()) {
    debugf(0, "EventClients must have valid names");
    return;
  }

  if (!clientList.insert(ec)) {
    debugf(0, "Duplicate EventClient name: %s\n");
    return;
  }
  clientSet.insert(ec);
}


void EventHandler::RemoveClient(EventClient* ec) {
  clientList.remove(ec);
  clientList.purify();
  clientSet.erase(ec);

  EventMap::const_iterator it;
  for (it = eventMap.begin(); it != eventMap.end(); ++it) {
    const EventInfo& ei = it->second;
    if (ei.IsManaged() && (ei.GetList() != NULL)) {
      ei.GetList()->remove(ec);
    }
  }

  GfxBlockMgr::removeClient(ec);

//???  RemoveClientActions(ec, syncedActions);
//???  RemoveClientActions(ec, unsyncedActions);
}


//============================================================================//
//============================================================================//

void EventHandler::GetEventList(std::vector<std::string>& eventList) const {
  eventList.clear();
  EventMap::const_iterator it;
  for (it = eventMap.begin(); it != eventMap.end(); ++it) {
    eventList.push_back(it->first);
  }
}


const EventHandler::EventInfo*
EventHandler::GetEventInfo(const std::string& eName) const {
  EventMap::const_iterator it = eventMap.find(eName);
  return (it != eventMap.end()) ? &(it->second) : NULL;
}


bool EventHandler::IsKnown(const std::string& eName) const {
  return GetEventInfo(eName) != NULL;
}


bool EventHandler::IsManaged(const std::string& eName) const {
  const EventInfo* ei = GetEventInfo(eName);
  return (ei != NULL) && (ei->GetList() != NULL);
}


bool EventHandler::IsReversed(const std::string& eName) const {
  const EventInfo* ei = GetEventInfo(eName);
  return (ei != NULL) && ei->IsReversed();
}


bool EventHandler::IsReentrant(const std::string& eName) const {
  const EventInfo* ei = GetEventInfo(eName);
  return (ei != NULL) && ei->IsReentrant();
}


bool EventHandler::ReqFullRead(const std::string& eName) const {
  const EventInfo* ei = GetEventInfo(eName);
  return (ei != NULL) && ei->ReqFullRead();
}


bool EventHandler::ReqGameCtrl(const std::string& eName) const {
  const EventInfo* ei = GetEventInfo(eName);
  return (ei != NULL) && ei->ReqGameCtrl();
}


bool EventHandler::ReqInputCtrl(const std::string& eName) const {
  const EventInfo* ei = GetEventInfo(eName);
  return (ei != NULL) && ei->ReqInputCtrl();
}


//============================================================================//
//============================================================================//

bool EventHandler::CanUseEvent(EventClient* ec, const EventInfo& ei) const {
  if ((ei.ReqFullRead()  && !ec->HasFullRead()) ||
      (ei.ReqGameCtrl()  && !ec->HasGameCtrl()) ||
      (ei.ReqInputCtrl() && !ec->HasInputCtrl())) {
    return false;
  }
  return true;
}


bool EventHandler::InsertEvent(EventClient* ec, const std::string& ciName) {
  if (clientSet.find(ec) == clientSet.end()) {
    return false;
  }
  EventMap::iterator it = eventMap.find(ciName);
  if (it == eventMap.end()) {
    return false;
  }
  const EventInfo& ei = it->second;
  EventClientList* ecList = ei.GetList();
  if (ecList == NULL) {
    return false;
  }
  if (!CanUseEvent(ec, ei)) {
    return false;
  }
  return ecList->insert(ec);
}


bool EventHandler::RemoveEvent(EventClient* ec, const std::string& ciName) {
  if (clientSet.find(ec) == clientSet.end()) {
    return false;
  }
  EventMap::iterator it = eventMap.find(ciName);
  if ((it == eventMap.end()) || (it->second.GetList() == NULL)) {
    return false;
  }
  EventClientList* ecList = it->second.GetList();
  if (ecList->remove(ec)) {
    dirtyLists.insert(ecList);
    return true;
  }
  return false;
}


//============================================================================//
//============================================================================//

const char* EventHandler::GetLoopTypeName(LoopType type) const {
  switch (type) {
    case Basic:       { return "BASIC";        }
    case Special:     { return "SPECIAL";      }
    case FirstTrue:   { return "FIRST_TRUE";   }
    case FirstFalse:  { return "FIRST_FALSE";  }
    case FirstNumber: { return "FIRST_NUMBER"; }
    case FirstString: { return "FIRST_STRING"; }
    case BooleanOR:   { return "BOOLEAN_OR";   }
  }
  return NULL;
}


//============================================================================//
//============================================================================//

void EventHandler::RecvLuaData(int srcPlayerID, int srcScriptID,
                               int dstPlayerID, int dstScriptID,
                               int status, const std::string& data) {
  EventClientList& ecList = listRecvLuaData;
  EventClientList::const_iterator it;
  for (it = ecList.begin(); it != ecList.end(); ++it) {
    EventClient* ec = *it;
    if ((dstScriptID == 0) || (dstScriptID == ec->GetScriptID())) {
      ec->RecvLuaData(srcPlayerID, srcScriptID,
                      dstPlayerID, dstScriptID,
                      status, data);
    }
  }
}


//============================================================================//
//============================================================================//

#define DRAW_CALLIN(name) \
  void EventHandler:: name () \
  { \
    EventClientList& ecList = list ## name; \
    if (ecList.empty()) { return; } \
    EventClientList::const_iterator it = ecList.begin(); \
    \
    OpenGLPassState::Enable ## name (); \
    (*it)-> name (); \
    \
    for (/* noop */; it != ecList.end(); ++it) { \
      OpenGLPassState::Reset ## name (); \
      (*it)-> name (); \
    } \
    \
    OpenGLPassState::Disable ## name (); \
  }

DRAW_CALLIN(DrawGenesis)
DRAW_CALLIN(DrawWorldStart)
DRAW_CALLIN(DrawWorld)
DRAW_CALLIN(DrawWorldAlpha)
DRAW_CALLIN(DrawWorldShadow)
DRAW_CALLIN(DrawScreenStart)
DRAW_CALLIN(DrawScreen)
DRAW_CALLIN(DrawRadar)


//============================================================================//
//============================================================================//

bool EventHandler::CommandFallback(const std::string& cmd) {
  EventClientList& ecList = listCommandFallback;
  EventClientList::const_iterator it;
  for (it = ecList.begin(); it != ecList.end(); ++it) {
    if ((*it)->CommandFallback(cmd)) {
      return true;
    }
  }
  return false;
}


bool EventHandler::KeyPress(bool taken, int key, int mods) {
  EventClientList& ecList = listKeyPress;
  EventClientList::const_iterator it;
  for (it = ecList.begin(); it != ecList.end(); ++it) {
    taken = (*it)->KeyPress(taken, key, mods) || taken;
  }
  return taken;
}


bool EventHandler::KeyRelease(bool taken, int key, int mods) {
  EventClientList& ecList = listKeyRelease;
  EventClientList::const_iterator it;
  for (it = ecList.begin(); it != ecList.end(); ++it) {
    taken = (*it)->KeyRelease(taken, key, mods) || taken;
  }
  return taken;
}


bool EventHandler::UnicodeText(bool taken, uint32_t unicode) {
  EventClientList& ecList = listUnicodeText;
  EventClientList::const_iterator it;
  for (it = ecList.begin(); it != ecList.end(); ++it) {
    taken = (*it)->UnicodeText(taken, unicode) || taken;
  }
  return taken;
}


bool EventHandler::MousePress(bool taken, int x, int y, int button) {
  EventClientList& ecList = listMousePress;
  EventClientList::const_iterator it;
  for (it = ecList.begin(); it != ecList.end(); ++it) {
    taken = (*it)->MousePress(taken, x, y, button) || taken;
  }
  return taken;
}


bool EventHandler::MouseRelease(bool taken, int x, int y, int button) {
  EventClientList& ecList = listMouseRelease;
  EventClientList::const_iterator it;
  for (it = ecList.begin(); it != ecList.end(); ++it) {
    taken = (*it)->MouseRelease(taken, x, y, button) || taken;
  }
  return taken;
}


bool EventHandler::MouseMove(bool taken, int x, int y) {
  EventClientList& ecList = listMouseMove;
  EventClientList::const_iterator it;
  for (it = ecList.begin(); it != ecList.end(); ++it) {
    taken = (*it)->MouseMove(taken, x, y) || taken;
  }
  return taken;
}


bool EventHandler::MouseWheel(bool taken, float value) {
  EventClientList& ecList = listMouseWheel;
  EventClientList::const_iterator it;
  for (it = ecList.begin(); it != ecList.end(); ++it) {
    taken = (*it)->MouseWheel(taken, value) || taken;
  }
  return taken;
}


bool EventHandler::IsAbove(int x, int y) {
  EventClientList& ecList = listIsAbove;
  EventClientList::const_iterator it;
  for (it = ecList.begin(); it != ecList.end(); ++it) {
    if ((*it)->IsAbove(x, y)) {
      return true;
    }
  }
  return false;
}


std::string EventHandler::GetTooltip(int x, int y) {
  EventClientList& ecList = listGetTooltip;
  EventClientList::const_iterator it;
  for (it = ecList.begin(); it != ecList.end(); ++it) {
    const std::string tt = (*it)->GetTooltip(x, y);
    if (!tt.empty()) {
      return tt;
    }
  }
  return "";
}


void EventHandler::WordComplete(const std::string& line, std::set<std::string>& partials) {
  EventClientList& ecList = listWordComplete;
  EventClientList::const_iterator it;
  for (it = ecList.begin(); it != ecList.end(); ++it) {
    std::set<std::string> safePartials;
    (*it)->WordComplete(line, safePartials);
    std::set<std::string>::const_iterator sit;
    for (sit = safePartials.begin(); sit != safePartials.end(); ++sit) {
      partials.insert(*sit); // NOTE: empty strings are valid
    }
  }
}


//============================================================================//
//============================================================================//

bool EventHandler::ForbidSpawn() {
  EventClientList& ecList = listForbidSpawn;
  EventClientList::const_iterator it;
  for (it = ecList.begin(); it != ecList.end(); ++it) {
    if ((*it)->ForbidSpawn()) {
      return true;
    }
  }
  return false;
}


bool EventHandler::ForbidJump() {
  EventClientList& ecList = listForbidJump;
  EventClientList::const_iterator it;
  for (it = ecList.begin(); it != ecList.end(); ++it) {
    if ((*it)->ForbidJump()) {
      return true;
    }
  }
  return false;
}


bool EventHandler::ForbidFlagDrop() {
  EventClientList& ecList = listForbidFlagDrop;
  EventClientList::const_iterator it;
  for (it = ecList.begin(); it != ecList.end(); ++it) {
    if ((*it)->ForbidFlagDrop()) {
      return true;
    }
  }
  return false;
}


bool EventHandler::ForbidShot() {
  EventClientList& ecList = listForbidShot;
  EventClientList::const_iterator it;
  for (it = ecList.begin(); it != ecList.end(); ++it) {
    if ((*it)->ForbidShot()) {
      return true;
    }
  }
  return false;
}


bool EventHandler::ForbidShotLock(const Player& player) {
  EventClientList& ecList = listForbidShotLock;
  EventClientList::const_iterator it;
  for (it = ecList.begin(); it != ecList.end(); ++it) {
    if ((*it)->ForbidShotLock(player)) {
      return true;
    }
  }
  return false;
}


bool EventHandler::ForbidShotHit(const Player& player,
                                 const ShotPath& shot, const fvec3& pos) {
  EventClientList& ecList = listForbidShotHit;
  EventClientList::const_iterator it;
  for (it = ecList.begin(); it != ecList.end(); ++it) {
    if ((*it)->ForbidShotHit(player, shot, pos)) {
      return true;
    }
  }
  return false;
}


//============================================================================//
//============================================================================//
