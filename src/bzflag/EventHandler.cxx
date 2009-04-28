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
#include "bzfio.h"
#include "GfxBlock.h"
#include "OpenGLPassState.h"


EventHandler eventHandler;


//============================================================================//
//============================================================================//

// it's easier to read with named booleans
static const int REQ_FULL_READ  = (1 << 0);
static const int REQ_GAME_CTRL  = (1 << 1);
static const int REQ_INPUT_CTRL = (1 << 2);
static const int REVERSED       = (1 << 3);
static const int REENTRANT      = (1 << 4);


void EventHandler::SetupEvent(const std::string& eName, EventClientList* list,
                              int orderType, LoopType loopType, int bits)
{
  assert(eventMap.find(eName) == eventMap.end());
  const bool reqFullRead  = ((bits & REQ_FULL_READ)  ? true : false);
  const bool reqGameCtrl  = ((bits & REQ_GAME_CTRL)  ? true : false);
  const bool reqInputCtrl = ((bits & REQ_INPUT_CTRL) ? true : false);
  const bool reversed     = ((bits & REVERSED)       ? true : false);
  const bool reentrant    = ((bits & REENTRANT)      ? true : false);

  list->set_order_type(orderType);
  list->set_reversed(reversed);

  eventMap[eName] = EventInfo(eName, list,
                              reqFullRead, reqGameCtrl, reqInputCtrl,
                              reversed, reentrant, loopType);
}


#define SETUP_EVENT(name, orderType, loopType, bits) \
  SetupEvent(#name, &list ## name, orderType, loopType, bits)


//============================================================================//
//============================================================================//

EventHandler::EventHandler()
{
  const int ScriptIDOrder   = EventClient::ScriptIDOrder;
  const int GameStateOrder  = EventClient::GameStateOrder;
  const int DrawWorldOrder  = EventClient::DrawWorldOrder;
  const int DrawScreenOrder = EventClient::DrawScreenOrder;

  SETUP_EVENT(Update,            ScriptIDOrder,   Basic,      0);

  SETUP_EVENT(BZDBChange,        GameStateOrder,  Basic,     REENTRANT);

  SETUP_EVENT(CommandFallback,   DrawScreenOrder, FirstTrue,    0);

  SETUP_EVENT(RecvChatMsg,       ScriptIDOrder,   FirstTrue,      0);
  SETUP_EVENT(RecvLuaData,       ScriptIDOrder,   FirstTrue,      0);

  SETUP_EVENT(ServerJoined,      GameStateOrder,  Basic,     0);
  SETUP_EVENT(ServerParted,      GameStateOrder,  Basic,     0);

  SETUP_EVENT(PlayerAdded,       GameStateOrder,  Basic,     0);
  SETUP_EVENT(PlayerRemoved,     GameStateOrder,  Basic,     0);
  SETUP_EVENT(PlayerSpawned,     GameStateOrder,  Basic,     0);
  SETUP_EVENT(PlayerKilled,      GameStateOrder,  Basic,     0);
  SETUP_EVENT(PlayerJumped,      GameStateOrder,  Basic,     REQ_FULL_READ);
  SETUP_EVENT(PlayerLanded,      GameStateOrder,  Basic,     REQ_FULL_READ);
  SETUP_EVENT(PlayerTeleported,  GameStateOrder,  Basic,     REQ_FULL_READ);
  SETUP_EVENT(PlayerTeamChange,  GameStateOrder,  Basic,     0);
  SETUP_EVENT(PlayerScoreChange, GameStateOrder,  Basic,     0);

  SETUP_EVENT(ShotAdded,         GameStateOrder,  Basic,     REQ_FULL_READ);
  SETUP_EVENT(ShotRemoved,       GameStateOrder,  Basic,     REQ_FULL_READ);
  SETUP_EVENT(ShotRicochet,      GameStateOrder,  Basic,     REQ_FULL_READ);
  SETUP_EVENT(ShotTeleported,    GameStateOrder,  Basic,     REQ_FULL_READ);

  SETUP_EVENT(FlagAdded,         GameStateOrder,  Basic,     0);
  SETUP_EVENT(FlagRemoved,       GameStateOrder,  Basic,     0);
  SETUP_EVENT(FlagGrabbed,       GameStateOrder,  Basic,     0);
  SETUP_EVENT(FlagDropped,       GameStateOrder,  Basic,     0);
  SETUP_EVENT(FlagCaptured,      GameStateOrder,  Basic,     0);
  SETUP_EVENT(FlagTransferred,   GameStateOrder,  Basic,     0);

  SETUP_EVENT(GLResize,          DrawScreenOrder, Basic,    0);
  SETUP_EVENT(GLContextInit,     DrawScreenOrder, Basic,    0);
  SETUP_EVENT(GLContextFree,     DrawScreenOrder, Basic,    0);
  SETUP_EVENT(GLUnmapped,        DrawScreenOrder, Basic,    0);

  SETUP_EVENT(DrawGenesis,       DrawScreenOrder, Basic, REVERSED);
  SETUP_EVENT(DrawWorldStart,    DrawWorldOrder,  Basic, REVERSED);
  SETUP_EVENT(DrawWorld,         DrawWorldOrder,  Basic, REVERSED);
  SETUP_EVENT(DrawWorldAlpha,    DrawWorldOrder,  Basic, REVERSED);
  SETUP_EVENT(DrawWorldShadow,   DrawWorldOrder,  Basic, REVERSED);
  SETUP_EVENT(DrawScreenStart,   DrawScreenOrder, Basic, REVERSED);
  SETUP_EVENT(DrawScreen,        DrawScreenOrder, Basic, REVERSED);
  SETUP_EVENT(DrawRadar,         DrawScreenOrder, Basic, REVERSED);

  SETUP_EVENT(KeyPress,          DrawScreenOrder, TakenContinue, REQ_INPUT_CTRL);
  SETUP_EVENT(KeyRelease,        DrawScreenOrder, TakenContinue, REQ_INPUT_CTRL);
  SETUP_EVENT(MousePress,        DrawScreenOrder, TakenContinue, REQ_INPUT_CTRL);
  SETUP_EVENT(MouseMove,         DrawScreenOrder, TakenContinue, REQ_INPUT_CTRL);
  SETUP_EVENT(MouseRelease,      DrawScreenOrder, TakenContinue, REQ_INPUT_CTRL);
  SETUP_EVENT(MouseWheel,        DrawScreenOrder, TakenContinue, REQ_INPUT_CTRL);
  SETUP_EVENT(IsAbove,           DrawScreenOrder, FirstTrue,     REQ_INPUT_CTRL);
  SETUP_EVENT(GetTooltip,        DrawScreenOrder, FirstString,   REQ_INPUT_CTRL);

  SETUP_EVENT(WordComplete,      DrawScreenOrder, Special,        REQ_INPUT_CTRL);

  SETUP_EVENT(ForbidSpawn,       GameStateOrder,  FirstTrue,    REQ_GAME_CTRL);
  SETUP_EVENT(ForbidJump,        GameStateOrder,  FirstTrue,    REQ_GAME_CTRL);
  SETUP_EVENT(ForbidShot,        GameStateOrder,  FirstTrue,    REQ_GAME_CTRL);
  SETUP_EVENT(ForbidShotLock,    GameStateOrder,  FirstTrue,    REQ_GAME_CTRL);
  SETUP_EVENT(ForbidFlagDrop,    GameStateOrder,  FirstTrue,    REQ_GAME_CTRL);
}


EventHandler::~EventHandler()
{
}


//============================================================================//
//============================================================================//

void EventHandler::AddClient(EventClient* ec)
{
  if (ec->GetName().empty()) {
    logDebugMessage(0, "EventClients must have valid names");
    return;
  }

  if (!clientList.insert(ec)) {
    logDebugMessage(0, "Duplicate EventClient name: %s\n");
    return;
  }
  clientSet.insert(ec);
}


void EventHandler::RemoveClient(EventClient* ec)
{
  clientList.remove(ec);
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

void EventHandler::GetEventList(std::vector<std::string>& list) const
{
  list.clear();
  EventMap::const_iterator it;
  for (it = eventMap.begin(); it != eventMap.end(); ++it) {
    list.push_back(it->first);
  }
}


const EventHandler::EventInfo*
  EventHandler::GetEventInfo(const std::string& eName) const
{
  EventMap::const_iterator it = eventMap.find(eName);
  return (it != eventMap.end()) ? &(it->second) : NULL;
}


bool EventHandler::IsKnown(const std::string& eName) const
{
  return GetEventInfo(eName) != NULL;
}


bool EventHandler::IsManaged(const std::string& eName) const
{
  const EventInfo* ei = GetEventInfo(eName);
  return (ei != NULL) && (ei->GetList() != NULL);
}


bool EventHandler::IsReversed(const std::string& eName) const
{
  const EventInfo* ei = GetEventInfo(eName);
  return (ei != NULL) && ei->IsReversed();
}


bool EventHandler::IsReentrant(const std::string& eName) const
{
  const EventInfo* ei = GetEventInfo(eName);
  return (ei != NULL) && ei->IsReentrant();
}


bool EventHandler::ReqFullRead(const std::string& eName) const
{
  const EventInfo* ei = GetEventInfo(eName);
  return (ei != NULL) && ei->ReqFullRead();
}


bool EventHandler::ReqGameCtrl(const std::string& eName) const
{
  const EventInfo* ei = GetEventInfo(eName);
  return (ei != NULL) && ei->ReqGameCtrl();
}


bool EventHandler::ReqInputCtrl(const std::string& eName) const
{
  const EventInfo* ei = GetEventInfo(eName);
  return (ei != NULL) && ei->ReqInputCtrl();
}


//============================================================================//
//============================================================================//

bool EventHandler::CanUseEvent(EventClient* ec, const EventInfo& ei) const
{
  if ((ei.ReqFullRead()  && !ec->HasFullRead()) ||
      (ei.ReqGameCtrl()  && !ec->HasGameCtrl()) ||
      (ei.ReqInputCtrl() && !ec->HasInputCtrl())) {
    return false;
  }
  return true;
}


bool EventHandler::InsertEvent(EventClient* ec, const std::string& ciName)
{
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


bool EventHandler::RemoveEvent(EventClient* ec, const std::string& ciName)
{
  if (clientSet.find(ec) == clientSet.end()) {
    return false;
  }
  EventMap::iterator it = eventMap.find(ciName);
  if ((it == eventMap.end()) || (it->second.GetList() == NULL)) {
    return false;
  }
  return it->second.GetList()->remove(ec);
}


//============================================================================//
//============================================================================//

const char* EventHandler::GetLoopTypeName(LoopType type) const
{
  switch (type) {
    case Basic:         { return "BASIC";          }
    case Special:       { return "SPECIAL";        }
    case FirstTrue:     { return "FIRST_TRUE";     }
    case FirstFalse:    { return "FIRST_FALSE";    }
    case FirstNumber:   { return "FIRST_NUMBER";   }
    case FirstString:   { return "FIRST_STRING";   }
    case BooleanOR:     { return "BOOLEAN_OR";     }
    case TakenContinue: { return "TAKEN_CONTINUE"; }
  }
  return NULL;
}


//============================================================================//
//============================================================================//

void EventHandler::RecvLuaData(int srcPlayerID, int srcScriptID,
                               int dstPlayerID, int dstScriptID,
                               int status, const std::string& data)
{
  EventClientList& list = listRecvLuaData;
  if (list.empty()) { return; }
  size_t i = 0;
  EventClient* ec;
  for (list.start(i); list.next(i, ec); /* no-op */) {
    if ((dstScriptID == 0) || (dstScriptID == ec->GetScriptID())) {
      ec->RecvLuaData(srcPlayerID, srcScriptID,
                      dstPlayerID, dstScriptID,
                      status, data);
    }
  }
  list.finish();
}


//============================================================================//
//============================================================================//

#define DRAW_CALLIN(name)                 \
  void EventHandler:: name ()             \
  {                                       \
    EventClientList& list = list ## name; \
    if (list.empty()) { return; }         \
                                          \
    OpenGLPassState::Enable ## name ();   \
                                          \
    EventClient* ec = NULL;               \
    size_t i = 0;                         \
    list.start(i);                        \
    list.next(i, ec);                     \
    ec-> name ();                         \
                                          \
    while (list.next(i, ec)) {            \
      OpenGLPassState::Reset ## name ();  \
      ec-> name ();                       \
    }                                     \
    list.finish();                        \
                                          \
    OpenGLPassState::Disable ## name ();  \
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

bool EventHandler::CommandFallback(const std::string& cmd)
{
  EventClientList& list = listCommandFallback;
  if (list.empty()) { return false; }
  size_t i = 0;
  EventClient* ec;
  for (list.start(i); list.next(i, ec); /* no-op */) {
    if (ec->CommandFallback(cmd)) {
      list.finish();
      return true;
    }
  }
  list.finish();
  return false;
}


bool EventHandler::KeyPress(bool taken, int key, bool isRepeat)
{
  EventClientList& list = listKeyPress;
  if (list.empty()) { return false; }
  size_t i = 0;
  EventClient* ec;
  for (list.start(i); list.next(i, ec); /* no-op */) {
    taken = ec->KeyPress(taken, key, isRepeat) || taken;
  }
  list.finish();
  return taken;
}


bool EventHandler::KeyRelease(bool taken, int key)
{
  EventClientList& list = listKeyRelease;
  if (list.empty()) { return false; }
  size_t i = 0;
  EventClient* ec;
  for (list.start(i); list.next(i, ec); /* no-op */) {
    taken = ec->KeyRelease(taken, key) || taken;
  }
  list.finish();
  return taken;
}


bool EventHandler::MousePress(bool taken, int x, int y, int button)
{
  EventClientList& list = listMousePress;
  if (list.empty()) { return false; }
  size_t i = 0;
  EventClient* ec;
  for (list.start(i); list.next(i, ec); /* no-op */) {
    taken = ec->MousePress(taken, x, y, button) || taken;
  }
  list.finish();
  return taken;
}


bool EventHandler::MouseRelease(bool taken, int x, int y, int button)
{
  EventClientList& list = listMouseRelease;
  if (list.empty()) { return false; }
  size_t i = 0;
  EventClient* ec;
  for (list.start(i); list.next(i, ec); /* no-op */) {
    taken = ec->MouseRelease(taken, x, y, button) || taken;
  }
  list.finish();
  return taken;
}


bool EventHandler::MouseMove(bool taken, int x, int y)
{
  EventClientList& list = listMouseMove;
  if (list.empty()) { return false; }
  size_t i = 0;
  EventClient* ec;
  for (list.start(i); list.next(i, ec); /* no-op */) {
    taken = ec->MouseMove(taken, x, y) || taken;
  }
  list.finish();
  return taken;
}


bool EventHandler::MouseWheel(bool taken, float value)
{
  EventClientList& list = listMouseWheel;
  if (list.empty()) { return false; }
  size_t i = 0;
  EventClient* ec;
  for (list.start(i); list.next(i, ec); /* no-op */) {
    taken = ec->MouseWheel(taken, value) || taken;
  }
  list.finish();
  return taken;
}


bool EventHandler::IsAbove(int x, int y)
{
  EventClientList& list = listIsAbove;
  if (list.empty()) { return false; }
  size_t i = 0;
  EventClient* ec;
  for (list.start(i); list.next(i, ec); /* no-op */) {
    if (ec->IsAbove(x, y)) {
      list.finish();
      return true;
    }
  }
  list.finish();
  return false;
}


std::string EventHandler::GetTooltip(int x, int y)
{
  EventClientList& list = listGetTooltip;
  if (list.empty()) { return ""; }
  size_t i = 0;
  EventClient* ec;
  for (list.start(i); list.next(i, ec); /* no-op */) {
    const std::string tt = ec->GetTooltip(x, y);
    if (!tt.empty()) {
      list.finish();
      return tt;
    }
  }
  list.finish();
  return "";
}


void EventHandler::WordComplete(const std::string& line, std::set<std::string>& partials)
{
  EventClientList& list = listWordComplete;
  if (list.empty()) { return; }
  size_t i = 0;
  EventClient* ec;
  for (list.start(i); list.next(i, ec); /* no-op */) {
    std::set<std::string> safePartials;
    ec->WordComplete(line, safePartials);
    std::set<std::string>::const_iterator it;
    for (it = safePartials.begin(); it != safePartials.end(); ++it) {
      partials.insert(*it); // NOTE: empty strings are valid
    }
  }
  list.finish();
}


//============================================================================//
//============================================================================//

bool EventHandler::ForbidSpawn()
{
  EventClientList& list = listForbidSpawn;
  if (list.empty()) { return false; }
  size_t i = 0;
  EventClient* ec;
  for (list.start(i); list.next(i, ec); /* no-op */) {
    if (ec->ForbidSpawn()) {
      list.finish();
      return true;
    }
  }
  list.finish();
  return false;
}


bool EventHandler::ForbidJump()
{
  EventClientList& list = listForbidJump;
  if (list.empty()) { return false; }
  size_t i = 0;
  EventClient* ec;
  for (list.start(i); list.next(i, ec); /* no-op */) {
    if (ec->ForbidJump()) {
      list.finish();
      return true;
    }
  }
  list.finish();
  return false;
}


bool EventHandler::ForbidShot()
{
  EventClientList& list = listForbidShot;
  if (list.empty()) { return false; }
  size_t i = 0;
  EventClient* ec;
  for (list.start(i); list.next(i, ec); /* no-op */) {
    if (ec->ForbidShot()) {
      list.finish();
      return true;
    }
  }
  list.finish();
  return false;
}


bool EventHandler::ForbidShotLock(const Player& player)
{
  EventClientList& list = listForbidShotLock;
  if (list.empty()) { return false; }
  size_t i = 0;
  EventClient* ec;
  for (list.start(i); list.next(i, ec); /* no-op */) {
    if (ec->ForbidShotLock(player)) {
      list.finish();
      return true;
    }
  }
  list.finish();
  return false;
}


bool EventHandler::ForbidFlagDrop()
{
  EventClientList& list = listForbidFlagDrop;
  if (list.empty()) { return false; }
  size_t i = 0;
  EventClient* ec;
  for (list.start(i); list.next(i, ec); /* no-op */) {
    if (ec->ForbidFlagDrop()) {
      list.finish();
      return true;
    }
  }
  list.finish();
  return false;
}


//============================================================================//
//============================================================================//
