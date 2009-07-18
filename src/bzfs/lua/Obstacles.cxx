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

// interface header
#include "Obstacles.h"

// system headers
#include <string>
#include <vector>
using std::string;
using std::vector;

// common headers
#include "ArcObstacle.h"
#include "BaseBuilding.h"
#include "BoxBuilding.h"
#include "ConeObstacle.h"
#include "MeshObstacle.h"
#include "Obstacle.h"
#include "ObstacleMgr.h"
#include "PyramidBuilding.h"
#include "SphereObstacle.h"
#include "WallObstacle.h"
#include "LinkManager.h"
#include "PhysicsDriver.h"
#include "CollisionManager.h"

// local headers
#include "LuaHeader.h"


//============================================================================//
//============================================================================//

bool LuaObstacle::PushEntries(lua_State* L)
{
  PUSH_LUA_CFUNC(L, GetObstacleList);
  PUSH_LUA_CFUNC(L, GetObstaclesInBox);

  PUSH_LUA_CFUNC(L, GetObstacleName);
  PUSH_LUA_CFUNC(L, GetObstacleType);

  PUSH_LUA_CFUNC(L, GetObstacleDriveThrough);
  PUSH_LUA_CFUNC(L, GetObstacleShootThrough);
  PUSH_LUA_CFUNC(L, GetObstacleRicochet);

  PUSH_LUA_CFUNC(L, GetObstaclePosition);
  PUSH_LUA_CFUNC(L, GetObstacleSize);
  PUSH_LUA_CFUNC(L, GetObstacleRotation);

  PUSH_LUA_CFUNC(L, GetObstacleFlatTop);
  PUSH_LUA_CFUNC(L, GetObstacleExtents);
  PUSH_LUA_CFUNC(L, GetObstacleTeam);
  PUSH_LUA_CFUNC(L, GetObstacleFlipZ);

  PUSH_LUA_CFUNC(L, GetMeshFaceCount);
  PUSH_LUA_CFUNC(L, GetMeshFace);

  PUSH_LUA_CFUNC(L, GetFaceMesh);
  PUSH_LUA_CFUNC(L, GetFaceVertCount);
  PUSH_LUA_CFUNC(L, GetFaceVerts);
  PUSH_LUA_CFUNC(L, GetFaceNorms);
  PUSH_LUA_CFUNC(L, GetFaceTxcds);
  PUSH_LUA_CFUNC(L, GetFacePlane);
  PUSH_LUA_CFUNC(L, GetFacePhyDrv);
  PUSH_LUA_CFUNC(L, GetFaceSmoothBounce);
  PUSH_LUA_CFUNC(L, GetFaceLinkName);

  PUSH_LUA_CFUNC(L, GetLinkSrcIDs);
  PUSH_LUA_CFUNC(L, GetLinkDstIDs);
  PUSH_LUA_CFUNC(L, GetLinkSrcName);
  PUSH_LUA_CFUNC(L, GetLinkDstName);
  PUSH_LUA_CFUNC(L, GetLinkSrcFace);
  PUSH_LUA_CFUNC(L, GetLinkDstFace);
  PUSH_LUA_CFUNC(L, GetLinkDestinations);

  PUSH_LUA_CFUNC(L, GetPhyDrvID);
  PUSH_LUA_CFUNC(L, GetPhyDrvName);

  return true;
}


//============================================================================//
//============================================================================//

static int GetTypeFromName(const string& name)
{
       if (name == "wall")   { return wallType;   }
  else if (name == "box")    { return boxType;    }
  else if (name == "pyr")    { return pyrType;    }
  else if (name == "base")   { return baseType;   }
  else if (name == "mesh")   { return meshType;   }
  else if (name == "tele")   { return teleType;   }
  else if (name == "arc")    { return arcType;    }
  else if (name == "cone")   { return coneType;   }
  else if (name == "sphere") { return sphereType; }
  else if (name == "face")   { return faceType;   }
  return -1;
}


//============================================================================//
//============================================================================//
//
//  Utility routines
//

static inline const Obstacle* ParseObstacle(lua_State* L, int index)
{
  const uint32_t obsID = luaL_checkint(L, index);
  return OBSTACLEMGR.getObstacleFromID(obsID);
}


static inline const MeshFace* ParseMeshFace(lua_State* L, int index)
{
  const Obstacle* face = ParseObstacle(L, index);
  if (face == NULL) {
    return NULL;
  }
  if (face->getTypeID() != faceType) {
    return NULL;
  }
  return (const MeshFace*)face;
}


static bool PushObstacleList(lua_State* L, int type, int& index)
{
  const GroupDefinition& world = OBSTACLEMGR.getWorld();
  if ((type < 0) || (type >= ObstacleTypeCount)) {
    return false;
  }
  const ObstacleList& obsList = world.getList(type);
  const size_t count = obsList.size();
  for (size_t i = 0; i < count; i++) {
    index++;
    lua_pushdouble(L, obsList[i]->getGUID());
    lua_rawseti(L, -2, index);
  }

  return true;
}


//============================================================================//
//============================================================================//
//
//  Obstacle lists
//

int LuaObstacle::GetObstacleList(lua_State* L)
{
  lua_settop(L, 1);
  lua_newtable(L);
  const int table = 1;

  if (!lua_istable(L, table)) {
    int index = 0;
    for (int type = 0; type < ObstacleTypeCount; type++) {
      PushObstacleList(L, type, index);
    }
  }
  else {
    lua_newtable(L);
    const int newTable = lua_gettop(L);
    for (int i = 1; lua_checkgeti(L, table, i); lua_pop(L, 1), i++) {
      int type = -1;
      if (lua_israwnumber(L, -1)) {
        type = lua_toint(L, -1);
      }
      else if (lua_israwstring(L, -1)) {
        type = GetTypeFromName(lua_tostring(L, -1));
      }
      if (type >= 0) {
        lua_pushinteger(L, type);
        lua_newtable(L);
        int index = 0;
        PushObstacleList(L, type, index);
        lua_rawset(L, newTable);
      }
    }
  }

  return 1;
}


int LuaObstacle::GetObstaclesInBox(lua_State* L)
{
  Extents exts;
  exts.mins.x = luaL_checkfloat(L, 1);
  exts.mins.y = luaL_checkfloat(L, 2);
  exts.mins.z = luaL_checkfloat(L, 3);
  exts.maxs.x = luaL_checkfloat(L, 4);
  exts.maxs.y = luaL_checkfloat(L, 5);
  exts.maxs.z = luaL_checkfloat(L, 6);
  const ObsList* obsList = COLLISIONMGR.axisBoxTest(exts);
  lua_createtable(L, obsList->count, 0);
  for (int i = 0; i < obsList->count; i++) {
    lua_pushdouble(L, obsList->list[i]->getGUID());
    lua_rawseti(L, -2, i + 1);
  }
  return 1;
}


//============================================================================//
//============================================================================//
//
//  Obstacle properties
//

int LuaObstacle::GetObstacleName(lua_State* L)
{
  const Obstacle* obs = ParseObstacle(L, 1);
  if (obs == NULL) {
    return 0;
  }
  lua_pushstdstring(L, obs->getName());
  return 1;
}


int LuaObstacle::GetObstacleType(lua_State* L)
{
  const Obstacle* obs = ParseObstacle(L, 1);
  if (obs == NULL) {
    return 0;
  }
  lua_pushinteger(L, obs->getTypeID());
  return 1;
}


int LuaObstacle::GetObstacleDriveThrough(lua_State* L)
{
  const Obstacle* obs = ParseObstacle(L, 1);
  if (obs == NULL) {
    return 0;
  }
  lua_pushboolean(L, obs->isDriveThrough());
  return 1;
}


int LuaObstacle::GetObstacleShootThrough(lua_State* L)
{
  const Obstacle* obs = ParseObstacle(L, 1);
  if (obs == NULL) {
    return 0;
  }
  lua_pushboolean(L, obs->isShootThrough());
  return 1;
}


int LuaObstacle::GetObstacleRicochet(lua_State* L)
{
  const Obstacle* obs = ParseObstacle(L, 1);
  if (obs == NULL) {
    return 0;
  }
  lua_pushboolean(L, obs->canRicochet());
  return 1;
}


int LuaObstacle::GetObstaclePosition(lua_State* L)
{
  const Obstacle* obs = ParseObstacle(L, 1);
  if (obs == NULL) {
    return 0;
  }
  const fvec3& pos = obs->getPosition();
  lua_pushfloat(L, pos.x);
  lua_pushfloat(L, pos.y);
  lua_pushfloat(L, pos.z);
  return 3;
}


int LuaObstacle::GetObstacleSize(lua_State* L)
{
  const Obstacle* obs = ParseObstacle(L, 1);
  if (obs == NULL) {
    return 0;
  }
  const fvec3& size = obs->getSize();
  lua_pushfloat(L, size.x);
  lua_pushfloat(L, size.y);
  lua_pushfloat(L, size.z);
  return 3;
}


int LuaObstacle::GetObstacleRotation(lua_State* L)
{
  const Obstacle* obs = ParseObstacle(L, 1);
  if (obs == NULL) {
    return 0;
  }
  lua_pushfloat(L, obs->getRotation());
  return 1;
}




int LuaObstacle::GetObstacleFlatTop(lua_State* L)
{
  const Obstacle* obs = ParseObstacle(L, 1);
  if (obs == NULL) {
    return 0;
  }
  lua_pushboolean(L, obs->isFlatTop());
  return 1;
}


int LuaObstacle::GetObstacleExtents(lua_State* L)
{
  const Obstacle* obs = ParseObstacle(L, 1);
  if (obs == NULL) {
    return 0;
  }
  const Extents& exts = obs->getExtents();
  lua_pushfloat(L, exts.mins.x);
  lua_pushfloat(L, exts.mins.y);
  lua_pushfloat(L, exts.mins.z);
  lua_pushfloat(L, exts.maxs.x);
  lua_pushfloat(L, exts.maxs.y);
  lua_pushfloat(L, exts.maxs.z);
  return 6;
}


int LuaObstacle::GetObstacleTeam(lua_State* L)
{
  const Obstacle* obs = ParseObstacle(L, 1);
  if (obs == NULL) {
    return 0;
  }
  const int baseTeam = obs->getBaseTeam();
  if (baseTeam < 0) {
    return 0;
  }
  lua_pushinteger(L, baseTeam);
  return 1;
}


int LuaObstacle::GetObstacleFlipZ(lua_State* L)
{
  const Obstacle* obs = ParseObstacle(L, 1);
  if (obs == NULL) {
    return 0;
  }
  lua_pushboolean(L, obs->getZFlip());
  return 1;
}


//============================================================================//
//============================================================================//
//
//  MeshFaces
//

int LuaObstacle::GetMeshFaceCount(lua_State* L)
{
  const Obstacle* obs = ParseObstacle(L, 1);
  if (obs->getTypeID() != meshType) {
    return 0;
  }

  const MeshObstacle* mesh = (const MeshObstacle*)obs;
  lua_pushinteger(L, mesh->getFaceCount());

  return 1;
}


int LuaObstacle::GetMeshFace(lua_State* L)
{
  const Obstacle* obs = ParseObstacle(L, 1);
  if (obs->getTypeID() != meshType) {
    return 0;
  }
  const MeshObstacle* mesh = (const MeshObstacle*)obs;

  const int faceID = luaL_checkint(L, 2) - 1;
  if ((faceID < 0) || (faceID >= mesh->getFaceCount())) { 
    return 0;
  }
  const MeshFace* face = mesh->getFace(faceID);

  lua_pushdouble(L, face->getGUID());

  return 1;
}


int LuaObstacle::GetFaceMesh(lua_State* L)
{
  const MeshFace* face = ParseMeshFace(L, 1);
  if (face == NULL) {
    return 0;
  }
  const MeshObstacle* mesh = face->getMesh();
  if (mesh == NULL) {
    lua_pushboolean(L, false);
    return 1;
  }
  lua_pushdouble(L, mesh->getGUID());
  return 1;
}


int LuaObstacle::GetFaceVertCount(lua_State* L)
{
  const MeshFace* face = ParseMeshFace(L, 1);
  if (face == NULL) {
    return 0;
  }
  const int elements = face->getVertexCount();
  lua_pushinteger(L, elements);
  return 1;
}


int LuaObstacle::GetFaceVerts(lua_State* L)
{
  const MeshFace* face = ParseMeshFace(L, 1);
  if (face == NULL) {
    return 0;
  }
  const int elements = face->getVertexCount();
  lua_createtable(L, elements, 0);
  for (int i = 0; i < elements; i++) {
    const fvec3& vec = face->getVertex(i);
    lua_createtable(L, 3, 0);
    lua_pushfloat(L, vec.x); lua_rawseti(L, -2, 1);
    lua_pushfloat(L, vec.y); lua_rawseti(L, -2, 2);
    lua_pushfloat(L, vec.z); lua_rawseti(L, -2, 3);
    lua_rawseti(L, -2, i + 1);
  }
  return 1;
}


int LuaObstacle::GetFaceNorms(lua_State* L)
{
  const MeshFace* face = ParseMeshFace(L, 1);
  if (face == NULL) {
    return 0;
  }
  if (!face->useNormals()) {
    return 0;
  }
  const int elements = face->getVertexCount();
  lua_createtable(L, elements, 0);
  for (int i = 0; i < elements; i++) {
    const fvec3& vec = face->getNormal(i);
    lua_createtable(L, 3, 0);
    lua_pushfloat(L, vec.x); lua_rawseti(L, -2, 1);
    lua_pushfloat(L, vec.y); lua_rawseti(L, -2, 2);
    lua_pushfloat(L, vec.z); lua_rawseti(L, -2, 3);
    lua_rawseti(L, -2, i + 1);
  }
  return 1;
}


int LuaObstacle::GetFaceTxcds(lua_State* L)
{
  const MeshFace* face = ParseMeshFace(L, 1);
  if (face == NULL) {
    return 0;
  }

  // defined texcoords
  if (face->useTexcoords()) {
    const int elements = face->getVertexCount();
    lua_createtable(L, elements, 0);
    for (int i = 0; i < elements; i++) {
      const fvec2& vec = face->getTexcoord(i);
      lua_createtable(L, 2, 0);
      lua_pushfloat(L, vec.s); lua_rawseti(L, -2, 1);
      lua_pushfloat(L, vec.t); lua_rawseti(L, -2, 2);
      lua_rawseti(L, -2, i + 1);
    }
    return 1;
  }

  // generated texcoords
  if (!lua_isboolean(L, 3) || lua_tobool(L, 3)) {
    const int elements = face->getVertexCount();
    vector<fvec3> vertArray;
    for (int i = 0; i < elements; i++) {
      vertArray.push_back(face->getVertex(i));
    }
    vector<fvec2> txcdArray;
    const fvec2& autoScale = face->getMaterial()->getTextureAutoScale(0);
    if (!MeshObstacle::makeTexcoords(autoScale,
                                     face->getPlane(), vertArray, txcdArray)) {
      return 0;
    }
    lua_createtable(L, elements, 0);
    for (int i = 0; i < elements; i++) {
      const fvec2& vec = txcdArray[i];
      lua_createtable(L, 2, 0);
      lua_pushfloat(L, vec.s); lua_rawseti(L, -2, 1);
      lua_pushfloat(L, vec.t); lua_rawseti(L, -2, 2);
      lua_rawseti(L, -2, i + 1);
    }
    return 1;
  }

  return 0;
}


int LuaObstacle::GetFacePlane(lua_State* L)
{
  const MeshFace* face = ParseMeshFace(L, 1);
  if (face == NULL) {
    return 0;
  }
  const fvec4& vec = face->getPlane();
  lua_pushfloat(L, vec.x);
  lua_pushfloat(L, vec.y);
  lua_pushfloat(L, vec.z);
  lua_pushfloat(L, vec.w);
  return 4;
}


int LuaObstacle::GetFacePhyDrv(lua_State* L)
{
  const MeshFace* face = ParseMeshFace(L, 1);
  if (face == NULL) {
    return 0;
  }
  lua_pushinteger(L, face->getPhysicsDriver());
  return 1;
}


int LuaObstacle::GetFaceSmoothBounce(lua_State* L)
{
  const MeshFace* face = ParseMeshFace(L, 1);
  if (face == NULL) {
    return 0;
  }
  lua_pushboolean(L, face->isSmoothBounce());
  return 1;
}


int LuaObstacle::GetFaceLinkName(lua_State* L)
{
  const MeshFace* face = ParseMeshFace(L, 1);
  if (face == NULL) {
    return 0;
  }
  const std::string linkName = face->getLinkName();
  if (linkName.empty()) {
    return 0;
  }
  lua_pushstdstring(L, linkName);
  return 1;
}


//============================================================================//
//============================================================================//
//
//  links
//

int LuaObstacle::GetLinkSrcIDs(lua_State* L)
{
  const string srcName = luaL_checkstring(L, 1);

  lua_newtable(L);
  const LinkManager::FaceVec& linkSrcs = linkManager.getLinkSrcs();
  for (size_t i = 0; i < linkSrcs.size(); i++) {
    const MeshFace* face = linkSrcs[i];
    if (face->getLinkName() == srcName) {
      lua_pushinteger(L, i);
      lua_rawseti(L, -2, i + 1);
    }
  }
  return 1;
}


int LuaObstacle::GetLinkDstIDs(lua_State* L)
{
  const string dstName = luaL_checkstring(L, 1);

  lua_newtable(L);
  const LinkManager::DstDataVec& linkDsts = linkManager.getLinkDsts();
  for (size_t i = 0; i < linkDsts.size(); i++) {
    const MeshFace* face = linkDsts[i].face;
    if (face->getLinkName() == dstName) {
      lua_pushinteger(L, i);
      lua_rawseti(L, -2, i + 1);
    }
  }
  return 1;
}


int LuaObstacle::GetLinkSrcName(lua_State* L)
{
  const int linkSrcID = luaL_checkint(L, 1);
  const MeshFace* face = linkManager.getLinkSrcFace(linkSrcID);
  if (face == NULL) {
    return 0;
  }
  lua_pushstdstring(L, face->getLinkName());
  return 1;
}


int LuaObstacle::GetLinkDstName(lua_State* L)
{
  const int linkDstID = luaL_checkint(L, 1);
  const MeshFace* face = linkManager.getLinkDstFace(linkDstID);
  if (face == NULL) {
    return 0;
  }
  lua_pushstdstring(L, face->getLinkName());
  return 1;
}


int LuaObstacle::GetLinkSrcFace(lua_State* L)
{
  const int linkSrcID = luaL_checkint(L, 1);
  const MeshFace* face = linkManager.getLinkSrcFace(linkSrcID);
  if (face == NULL) {
    return 0;
  }
  lua_pushdouble(L, face->getGUID());
  return 1;
}


int LuaObstacle::GetLinkDstFace(lua_State* L)
{
  const int linkDstID = luaL_checkint(L, 1);
  const MeshFace* face = linkManager.getLinkDstFace(linkDstID);
  if (face == NULL) {
    return 0;
  }
  lua_pushdouble(L, face->getGUID());
  return 1;
}


int LuaObstacle::GetLinkDestinations(lua_State* L)
{
  L = L;
  const int linkSrcID = luaL_checkint(L, 1);
  const MeshFace* srcFace = linkManager.getLinkSrcFace(linkSrcID);
  if (srcFace == NULL) {
    return 0;
  }
  const LinkManager::LinkMap& linkMap = linkManager.getLinkMap();
  LinkManager::LinkMap::const_iterator it = linkMap.find(srcFace);
  if (it == linkMap.end()) {
    return 0;
  }
  const LinkManager::IntVec& dstIDs = it->second.dstIDs;
  lua_createtable(L, dstIDs.size(), 0);
  for (size_t i = 0; i < dstIDs.size(); i++) {
    lua_pushinteger(L, dstIDs[i]);
    lua_rawseti(L, -2, i + 1);
  }
  return 1;
}


//============================================================================//
//============================================================================//
//
//  physics drivers
//

int LuaObstacle::GetPhyDrvID(lua_State* L)
{
  const char* name = luaL_checkstring(L, 1);
  const int id = PHYDRVMGR.findDriver(name);
  if (id < 0) {
    return 0;
  }
  lua_pushinteger(L, id);
  return 1;
}


int LuaObstacle::GetPhyDrvName(lua_State* L)
{
  const int id = luaL_checkint(L, 1);
  const PhysicsDriver* phydrv = PHYDRVMGR.getDriver(id);
  if (phydrv == NULL) {
    return 0;
  }
  lua_pushstdstring(L, phydrv->getName());
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
