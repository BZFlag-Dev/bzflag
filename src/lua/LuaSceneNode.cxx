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
#include "LuaSceneNode.h"

// system headers
#include <set>
#include <string>

// common headers
#include "LuaHeader.h"
#include "OpenGLGState.h"
#include "OpenGLPassState.h"
#include "SceneNode.h"
#include "SceneRenderer.h"
#include "SceneDatabase.h"

#include "clientbase/playing.h"
#include "clientbase/ClientFlag.h"
#include "clientbase/Roster.h"
#include "clientbase/World.h"


// local headers
#include "LuaGLPointers.h"
#include "LuaHandle.h"


class LuaSceneNode;

static std::set<LuaSceneNode*> validNodes;
static std::set<LuaSceneNode*> invalidNodes;

static const char* metaName = "SceneNode";


//============================================================================//
//============================================================================//
//
//  LuaRenderNode
//

class LuaRenderNode : public RenderNode
{
  public:
    LuaRenderNode(lua_State* LS, const LuaSceneNode* _sceneNode)
    : L(LS)
    , funcRef(LUA_NOREF)
    , dataRef(LUA_NOREF)
    , sceneNode(_sceneNode)
    , useArrays(false)
    , useTransform(false)
    {}
    ~LuaRenderNode() { Unref(); }

    void Unref();

    void render();
    void renderRadar() {}
    void renderShadow() {}
    const fvec3& getPosition() const;

    void executeTransform() const;

  public:
    lua_State* L;
    int funcRef;
    int dataRef;
    const LuaSceneNode* sceneNode;
    bool useArrays;
    bool useTransform;
};


void LuaRenderNode::Unref()
{
  if (funcRef != LUA_NOREF) {
    luaL_unref(L, LUA_REGISTRYINDEX, funcRef);
    funcRef = LUA_NOREF;
  }
  if (dataRef != LUA_NOREF) {
    luaL_unref(L, LUA_REGISTRYINDEX, dataRef);
    dataRef = LUA_NOREF;
  }
  L = NULL;
}


void LuaRenderNode::render()
{
  const bool useData = (dataRef != LUA_NOREF);

  lua_rawgeti(L, LUA_REGISTRYINDEX, funcRef);
  if (useData) {
    lua_rawgeti(L, LUA_REGISTRYINDEX, dataRef);
  }

  OpenGLPassState::EnableDrawWorldAlpha();
  if (useArrays) { LuaGLPointers::Enable(L); }
  if (useTransform) { executeTransform(); }

  const int status = lua_pcall(L, useData ? 1 : 0, 0, 0);

  if (useTransform) { glPopMatrix(); }
  if (useArrays) { LuaGLPointers::Reset(L); }
  OpenGLPassState::DisableDrawWorldAlpha();

  if (status != 0) {
    const char* errMsg = lua_tostring(L, -1);
    LuaLog(0, "LuaRenderNode::render() error:\n%s\n",
           errMsg ? errMsg : "<NULL>");
    lua_pop(L, 1);
  }
}


//============================================================================//
//============================================================================//
//
//  LuaSceneNode
//

class LuaSceneNode : public SceneNode
{
  public:
    enum TrackMode {
      TrackNone,
      TrackTank,
      TrackFlag,
      TrackShot
    };

  public:
    LuaSceneNode(lua_State* L);
    ~LuaSceneNode();

    void Unref() { renderNode.Unref(); }

  public:
    inline void publicSetSphere(const fvec4& s) { setSphere(s); }
    inline void publicSetRadius(const float  r) { setRadius(r); }
    inline void publicSetCenter(const fvec3& p) { setCenter(p); }

    void updatePosition();

    void addRenderNodes(SceneRenderer& renderer) {
      if (renderNode.funcRef != LUA_NOREF) {
        renderer.addRenderNode(&renderNode, &gstate);
      }
    }

  public:

  public:
    LuaRenderNode renderNode;
    OpenGLGState  gstate;
    bool          active;
    TrackMode     trackMode;
    int           trackTarget;
    float         radians;
};


//============================================================================//
//============================================================================//

const fvec3& LuaRenderNode::getPosition() const
{
  return sceneNode->getSphere().xyz();
}


void LuaRenderNode::executeTransform() const
{
  glPushMatrix();
  const fvec3& pos = getPosition();
  const float degs = sceneNode->radians * RAD2DEGf;
  glTranslatef(pos.x, pos.y, pos.z);
  if (degs != 0.0f) {
    glRotatef(degs, 0.0f, 0.0f, 1.0f);
  }
}


//============================================================================//
//============================================================================//

LuaSceneNode::LuaSceneNode(lua_State* L)
: renderNode(L, this)
, active(true)
, trackMode(TrackNone)
, trackTarget(-1)
, radians(0.0f)
{
  OpenGLGStateBuilder builder;
  builder.setNeedsSorting(true);
  gstate = builder.getState();
  validNodes.insert(this);
}


LuaSceneNode::~LuaSceneNode()
{
  validNodes.erase(this);
  invalidNodes.erase(this);
}


void LuaSceneNode::updatePosition()
{
  radians = 0.0f;

  switch (trackMode) {
    case TrackNone: {
      const fvec3& pos = getSphere().xyz();
      setCenter(pos);
      break;
    }
    case TrackTank: {
      const PlayerId playerID = (PlayerId) trackTarget;
      const Player* tank = lookupPlayer(playerID);
      if (tank == NULL) {
        break;
      }
      const fvec3& pos = tank->getPosition();
      setCenter(pos);
      radians = tank->getAngle();
      break;
    }
    case TrackFlag: {
      const int flagID = trackTarget;
      World* world = World::getWorld();
      if ((world == NULL) || (flagID < 0) || (flagID >= world->getMaxFlags())) {
        break;
      }
      const Flag& flag = world->getFlag(flagID);
      const fvec3& pos = flag.position;
      setCenter(pos);
    }
    case TrackShot: {
      const uint32_t fullID   = (uint32_t) trackTarget;
      const PlayerId playerID = (fullID >> 16);
      const uint16_t shotID   = fullID & 0xffff;
      const Player* player = lookupPlayer(playerID);
      if (player == NULL) {
        break;
      }
      const ShotPath* shot = player->getShot(shotID);
      if ((shot == NULL) || shot->isExpired()) {
        break;
      }
      const fvec3& pos = shot->getPosition();
      setCenter(pos);
    }
  }
}


//============================================================================//
//============================================================================//

bool LuaSceneNodeMgr::PushEntries(lua_State* L)
{
  CreateMetatable(L);
  PUSH_LUA_CFUNC(L, CreateSceneNode);
  PUSH_LUA_CFUNC(L, DeleteSceneNode);
  PUSH_LUA_CFUNC(L, IsSceneNode);
  PUSH_LUA_CFUNC(L, IsValidSceneNode);
  PUSH_LUA_CFUNC(L, SetSceneNodeFunc);
  PUSH_LUA_CFUNC(L, SetSceneNodeData);
  PUSH_LUA_CFUNC(L, SetSceneNodeActive);
  PUSH_LUA_CFUNC(L, SetSceneNodePosition);
  PUSH_LUA_CFUNC(L, SetSceneNodeRadius);
  PUSH_LUA_CFUNC(L, SetSceneNodeTracking);
  PUSH_LUA_CFUNC(L, SetSceneNodeUseArrays);
  PUSH_LUA_CFUNC(L, SetSceneNodeUseTransform);
  PUSH_LUA_CFUNC(L, GetSceneNodeInfo);
  PUSH_LUA_CFUNC(L, GetSceneNodePosition);
  PUSH_LUA_CFUNC(L, GetSceneNodeRadius);
  PUSH_LUA_CFUNC(L, GetSceneNodeRadians);
  return true;
}


void LuaSceneNodeMgr::AddSceneNodes(SceneDatabase& scene)
{
  std::set<LuaSceneNode*>::const_iterator it;
  for (it = validNodes.begin(); it != validNodes.end(); ++it) {
    LuaSceneNode* sceneNode = *it;
    if ((sceneNode->active) &&
        (sceneNode->renderNode.funcRef != LUA_NOREF)) {
      sceneNode->updatePosition();
      scene.addDynamicNode((SceneNode*)sceneNode);
    }
  }
  ClearSceneNodes();
}


void LuaSceneNodeMgr::ClearSceneNodes()
{
  std::set<LuaSceneNode*>::const_iterator it, nextIt;
  for (it = invalidNodes.begin(); it != invalidNodes.end(); it = nextIt) {
    nextIt = it;
    nextIt++;
    delete *it;
  }
  if (!invalidNodes.empty()) {
    LuaLog(0, "WARNING: LuaSceneNodeMgr::ClearSceneNodes()"
              " invalidNodes are not empty\n");
  }
  invalidNodes.clear();
}


//============================================================================//
//============================================================================//

bool LuaSceneNodeMgr::CreateMetatable(lua_State* L)
{
  luaL_newmetatable(L, metaName);

  luaset_strfunc(L, "__gc",  DeleteSceneNode);

  lua_pushliteral(L, "__index");
  lua_newtable(L);
  luaset_strfunc(L, "Delete",          DeleteSceneNode);
  luaset_strfunc(L, "IsValid",         IsValidSceneNode);
  luaset_strfunc(L, "SetFunc",         SetSceneNodeFunc);
  luaset_strfunc(L, "SetData",         SetSceneNodeData);
  luaset_strfunc(L, "SetActive",       SetSceneNodeActive);
  luaset_strfunc(L, "SetPosition",     SetSceneNodePosition);
  luaset_strfunc(L, "SetRadius",       SetSceneNodeRadius);
  luaset_strfunc(L, "SetTracking",     SetSceneNodeTracking);
  luaset_strfunc(L, "SetUseArrays",    SetSceneNodeUseArrays);
  luaset_strfunc(L, "SetUseTransform", SetSceneNodeUseTransform);
  luaset_strfunc(L, "GetInfo",         GetSceneNodeInfo);
  luaset_strfunc(L, "GetPosition",     GetSceneNodePosition);
  luaset_strfunc(L, "GetRadius",       GetSceneNodeRadius);
  luaset_strfunc(L, "GetRadians",      GetSceneNodeRadians);
  lua_rawset(L, -3);

  luaset_strstr(L, "__metatable", "no access");

  lua_pop(L, 1);
  return true;
}


//============================================================================//
//============================================================================//

static LuaSceneNode* CheckLuaSceneNode(lua_State* L, int index)
{
  if (lua_getuserdataextra(L, index) != metaName) {
    luaL_argerror(L, index, "expected a SceneNode");
  }
  LuaSceneNode** ptr = (LuaSceneNode**) lua_touserdata(L, index);
  return *ptr;
}


//============================================================================//
//============================================================================//

int LuaSceneNodeMgr::CreateSceneNode(lua_State* L)
{
  LuaSceneNode* sceneNode = new LuaSceneNode(L);
  LuaSceneNode** ptr =
    (LuaSceneNode**) lua_newuserdata(L, sizeof(LuaSceneNode*));
  lua_setuserdataextra(L, -1, (void*)metaName);
  luaL_getmetatable(L, metaName);
  lua_setmetatable(L, -2);
  *ptr = sceneNode;
  return 1;
}


int LuaSceneNodeMgr::DeleteSceneNode(lua_State* L)
{
  if (lua_getuserdataextra(L, 1) != metaName) {
    luaL_argerror(L, 1, "expected a SceneNode");
  }
  LuaSceneNode** ptr = (LuaSceneNode**) lua_touserdata(L, 1);
  LuaSceneNode* sceneNode = *ptr;
  if (sceneNode == NULL) {
    return 0;
  }
  sceneNode->Unref();
  validNodes.erase(sceneNode);
  invalidNodes.insert(sceneNode);
  *ptr = NULL;
  return 0;
}


int LuaSceneNodeMgr::IsSceneNode(lua_State* L)
{
  lua_pushboolean(L, lua_getuserdataextra(L, 1) == metaName);
  return 1;
}


int LuaSceneNodeMgr::IsValidSceneNode(lua_State* L)
{
  if (lua_getuserdataextra(L, 1) != metaName) {
    lua_pushboolean(L, false);
  }
  else {
    LuaSceneNode** ptr = (LuaSceneNode**) lua_touserdata(L, 1);
    lua_pushboolean(L, *ptr != NULL);
  }
  return 1;
}


int LuaSceneNodeMgr::SetSceneNodeFunc(lua_State* L)
{
  LuaSceneNode* sceneNode = CheckLuaSceneNode(L, 1);

  const int type = lua_type(L, 2);
  if ((type != LUA_TNIL) && (type != LUA_TFUNCTION)) {
    luaL_error(L, "bad function type");
  }

  if (sceneNode == NULL) {
    lua_settop(L, 1);
    return 1;
  }

  int& ref = sceneNode->renderNode.funcRef;
  if (ref != LUA_NOREF) {
    luaL_unref(L, LUA_REGISTRYINDEX, ref);
    ref = LUA_NOREF;
  }
  if (type == LUA_TFUNCTION) {
    lua_settop(L, 2);
    ref = luaL_ref(L, LUA_REGISTRYINDEX);
  }
  lua_settop(L, 1);
  return 1;
}


int LuaSceneNodeMgr::SetSceneNodeData(lua_State* L)
{
  LuaSceneNode* sceneNode = CheckLuaSceneNode(L, 1);

  luaL_checkany(L, 2);

  if (sceneNode == NULL) {
    lua_settop(L, 1);
    return 1;
  }

  int& ref = sceneNode->renderNode.dataRef;
  if (ref != LUA_NOREF) {
    luaL_unref(L, LUA_REGISTRYINDEX, ref);
    ref = LUA_NOREF;
  }

  if (!lua_isnil(L, 2)) {
    lua_settop(L, 2);
    ref = luaL_ref(L, LUA_REGISTRYINDEX);
  }

  lua_settop(L, 1);
  return 1;
}


int LuaSceneNodeMgr::SetSceneNodeActive(lua_State* L)
{
  LuaSceneNode* sceneNode = CheckLuaSceneNode(L, 1);
  if (sceneNode) {
    sceneNode->active = lua_tobool(L, 2);
  }
  lua_settop(L, 1);
  return 1;
}


int LuaSceneNodeMgr::SetSceneNodePosition(lua_State* L)
{
  LuaSceneNode* sceneNode = CheckLuaSceneNode(L, 1);
  const fvec3 pos(luaL_checkfloat(L, 2),
                  luaL_checkfloat(L, 3),
                  luaL_checkfloat(L, 4));
  if (sceneNode) {
    sceneNode->publicSetCenter(pos);
  }
  lua_settop(L, 1);
  return 1;
}


int LuaSceneNodeMgr::SetSceneNodeRadius(lua_State* L)
{
  LuaSceneNode* sceneNode = CheckLuaSceneNode(L, 1);
  const float rad = luaL_checkfloat(L, 2);
  if (sceneNode) {
    sceneNode->publicSetRadius(rad * rad);
  }
  lua_settop(L, 1);
  return 1;
}


int LuaSceneNodeMgr::SetSceneNodeUseArrays(lua_State* L)
{
  LuaSceneNode* sceneNode = CheckLuaSceneNode(L, 1);
  if (sceneNode) {
    sceneNode->renderNode.useArrays = lua_tobool(L, 2);
  }
  lua_settop(L, 1);
  return 1;
}


int LuaSceneNodeMgr::SetSceneNodeUseTransform(lua_State* L)
{
  LuaSceneNode* sceneNode = CheckLuaSceneNode(L, 1);
  if (sceneNode) {
    sceneNode->renderNode.useTransform = lua_tobool(L, 2);
  }
  lua_settop(L, 1);
  return 1;
}


int LuaSceneNodeMgr::SetSceneNodeTracking(lua_State* L)
{
  LuaSceneNode* sceneNode = CheckLuaSceneNode(L, 1);
  if (sceneNode == NULL) {
    lua_settop(L, 1);
    return 1;
  }

  if (lua_isnil(L, 2)) {
    sceneNode->trackMode = LuaSceneNode::TrackNone;
    sceneNode->trackTarget = -1;
    lua_settop(L, 1);
    return 1;
  }

  if (!L2H(L)->HasFullRead()) {
    luaL_error(L, "this script can not use positional tracking");
  }

  const std::string mode = luaL_checkstring(L, 2);
  if (mode == "none") {
    sceneNode->trackMode = LuaSceneNode::TrackNone;
    sceneNode->trackTarget = -1;
  } else if (mode == "tank") {
    sceneNode->trackMode = LuaSceneNode::TrackTank;
    sceneNode->trackTarget = luaL_checkint(L, 3);
  } else if (mode == "flag") {
    sceneNode->trackMode = LuaSceneNode::TrackFlag;
    sceneNode->trackTarget = luaL_checkint(L, 3);
  } else if (mode == "shot") {
    sceneNode->trackMode = LuaSceneNode::TrackShot;
    sceneNode->trackTarget = luaL_checkint(L, 3);
  } else {
    luaL_error(L, "bad tracking type");
  }

  lua_settop(L, 1);
  return 1;
}


int LuaSceneNodeMgr::GetSceneNodeInfo(lua_State* L)
{
  LuaSceneNode* sceneNode = CheckLuaSceneNode(L, 1);
  if (sceneNode == NULL) {
    lua_pushnil(L);
    return 1;
  }

  lua_pushboolean(L, sceneNode->active);
  lua_pushboolean(L, sceneNode->renderNode.useTransform);
  lua_pushboolean(L, sceneNode->renderNode.useArrays);

  switch (sceneNode->trackMode) {
    case LuaSceneNode::TrackNone: { lua_pushstdstring(L, "none"); }
    case LuaSceneNode::TrackTank: { lua_pushstdstring(L, "tank"); }
    case LuaSceneNode::TrackFlag: { lua_pushstdstring(L, "flag"); }
    case LuaSceneNode::TrackShot: { lua_pushstdstring(L, "shot"); }
  }
  lua_pushint(L, sceneNode->trackTarget);

  return 5;
}


int LuaSceneNodeMgr::GetSceneNodePosition(lua_State* L)
{
  LuaSceneNode* sceneNode = CheckLuaSceneNode(L, 1);
  if (sceneNode == NULL) {
    lua_pushnil(L);
    return 1;
  }
  const fvec4& s = sceneNode->getSphere();
  lua_pushfloat(L, s.x);
  lua_pushfloat(L, s.y);
  lua_pushfloat(L, s.z);
  return 3;
}


int LuaSceneNodeMgr::GetSceneNodeRadius(lua_State* L)
{
  LuaSceneNode* sceneNode = CheckLuaSceneNode(L, 1);
  if (sceneNode == NULL) {
    lua_pushnil(L);
    return 1;
  }
  lua_pushfloat(L, sqrtf(sceneNode->getSphere().w));
  return 1;
}


int LuaSceneNodeMgr::GetSceneNodeRadians(lua_State* L)
{
  LuaSceneNode* sceneNode = CheckLuaSceneNode(L, 1);
  if (sceneNode == NULL) {
    lua_pushnil(L);
    return 1;
  }
  lua_pushfloat(L, sceneNode->radians);
  return 1;
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
