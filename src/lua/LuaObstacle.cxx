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
#include "LuaObstacle.h"

// system headers
#include <assert.h>
#include <string>
#include <vector>
using std::string;
using std::vector;

// common headers
#include "ArcObstacle.h"
#include "BaseBuilding.h"
#include "BoxBuilding.h"
#include "ConeObstacle.h"
#include "MeshDrawInfo.h"
#include "MeshObstacle.h"
#include "Obstacle.h"
#include "ObstacleMgr.h"
#include "PyramidBuilding.h"
#include "SphereObstacle.h"
#include "Teleporter.h"
#include "WallObstacle.h"
#include "WorldText.h"
#include "CollisionManager.h"
#include "LinkManager.h"
#include "Extents.h"

// local headers
#include "LuaHeader.h"
#include "LuaUtils.h"
#include "LuaDouble.h"


//============================================================================//
//============================================================================//

bool LuaObstacle::PushEntries(lua_State* L)
{
  PUSH_LUA_CFUNC(L, GetObstacleList);
  PUSH_LUA_CFUNC(L, GetObstacleName);
  PUSH_LUA_CFUNC(L, GetObstacleType);
  PUSH_LUA_CFUNC(L, GetObstacleTypeID);

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
  PUSH_LUA_CFUNC(L, GetObstacleBorder);

  PUSH_LUA_CFUNC(L, GetMeshFaceCount);
  PUSH_LUA_CFUNC(L, GetMeshFace);

  PUSH_LUA_CFUNC(L, GetFaceMesh);
  PUSH_LUA_CFUNC(L, GetFaceElementCount);
  PUSH_LUA_CFUNC(L, GetFaceVerts);
  PUSH_LUA_CFUNC(L, GetFaceNorms);
  PUSH_LUA_CFUNC(L, GetFaceTxcds);
  PUSH_LUA_CFUNC(L, GetFacePlane);
  PUSH_LUA_CFUNC(L, GetFacePhyDrv);
  PUSH_LUA_CFUNC(L, GetFaceMaterial);
  PUSH_LUA_CFUNC(L, GetFaceSmoothBounce);

  PUSH_LUA_CFUNC(L, GetFaceBaseTeam);
  PUSH_LUA_CFUNC(L, GetFaceLinkName);
  PUSH_LUA_CFUNC(L, GetFaceLinkSrcID);
  PUSH_LUA_CFUNC(L, GetFaceLinkSrcAttribs);
  PUSH_LUA_CFUNC(L, GetFaceLinkSrcGeometry);
  PUSH_LUA_CFUNC(L, GetFaceLinkDstGeometry);
  PUSH_LUA_CFUNC(L, GetFaceZoneParams);

  PUSH_LUA_CFUNC(L, GetLinkSrcIDs);
  PUSH_LUA_CFUNC(L, GetLinkDstIDs);
  PUSH_LUA_CFUNC(L, GetLinkSrcName);
  PUSH_LUA_CFUNC(L, GetLinkDstName);
  PUSH_LUA_CFUNC(L, GetLinkSrcFace);
  PUSH_LUA_CFUNC(L, GetLinkDstFace);
  PUSH_LUA_CFUNC(L, GetLinkDestinations);

  PUSH_LUA_CFUNC(L, HasMeshDrawInfo);
  PUSH_LUA_CFUNC(L, GetMeshDrawInfo);

  PUSH_LUA_CFUNC(L, GetWorldTextCount);
  PUSH_LUA_CFUNC(L, GetWorldTextName);
  PUSH_LUA_CFUNC(L, GetWorldTextData);
  PUSH_LUA_CFUNC(L, GetWorldTextVarName);
  PUSH_LUA_CFUNC(L, GetWorldTextFont);
  PUSH_LUA_CFUNC(L, GetWorldTextFontSize);
  PUSH_LUA_CFUNC(L, GetWorldTextLineSpace);
  PUSH_LUA_CFUNC(L, GetWorldTextJustify);
  PUSH_LUA_CFUNC(L, GetWorldTextFixedWidth);
  PUSH_LUA_CFUNC(L, GetWorldTextLengthPerPixel);
  PUSH_LUA_CFUNC(L, GetWorldTextBillboard);
  PUSH_LUA_CFUNC(L, GetWorldTextMaterial);
  PUSH_LUA_CFUNC(L, GetWorldTextTransform);

  PUSH_LUA_CFUNC(L, GetObstaclesInBox);
  PUSH_LUA_CFUNC(L, ObstacleBoxTest);
  PUSH_LUA_CFUNC(L, ObstacleRayTest);

  return true;
}


//============================================================================//
//============================================================================//
//
// -- copied from MeshSceneNodeGenerator
//

static bool makeTexcoords(const fvec2& autoScale,
			  const fvec4& plane,
			  const vector<fvec3>& vertices,
			  vector<fvec2>& texcoords)
{
  const float defScale = 1.0f / 8.0f;
  const float sScale = (autoScale.s == 0.0f) ? defScale : 1.0f / autoScale.s;
  const float tScale = (autoScale.t == 0.0f) ? defScale : 1.0f / autoScale.t;

  fvec3 x = fvec3(vertices[1]) - fvec3(vertices[0]);
  fvec3 y = fvec3::cross(plane.xyz(), x);

  if (!fvec3::normalize(x) ||
      !fvec3::normalize(y)) {
    return false;
  }

  const bool horizontal = fabsf(plane[2]) > 0.999f;

  const size_t count = vertices.size();
  for (size_t i = 0; i < count; i++) {
    const fvec3& v = vertices[i];
    const fvec3 delta = fvec3(v) - vertices[0];
    const fvec2 nh = fvec2(plane.x, plane.y).normalize();
    const float vs = 1.0f / sqrtf(1.0f - (plane.z * plane.z));

    if (sScale < 0.0f) {
      texcoords[i].s = -sScale * fvec3::dot(delta, x);
    }
    else {
      if (horizontal) {
	texcoords[i].s = sScale * v.x;
      } else {
	texcoords[i].s = sScale * ((nh.x * v.y) - (nh.y * v.x));
      }
    }

    if (tScale < 0.0f) {
      texcoords[i].t = -tScale * fvec3::dot(delta, y);
    }
    else {
      if (horizontal) {
	texcoords[i].t = tScale * v.y;
      } else {
	texcoords[i].t = tScale * (v.z * vs);
      }
    }
  }
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
  return -1;
}


//============================================================================//
//============================================================================//
//
//  Utility routines
//

static void PushObstacleGUID(lua_State* L, uint32_t guid)
{
  assert(sizeof(void*) >= sizeof(uint32_t));

  void* ptr = 0;
  *((uint32_t*)&ptr) = guid;
  lua_pushlightuserdata(L, ptr);
}


static inline const Obstacle* ParseObstacle(lua_State* L, int index)
{
  uint32_t obsID;

  switch (lua_type(L, index)) {
    case LUA_TLIGHTUSERDATA: {
      const void* ptr = (void*)lua_topointer(L, index);
      obsID = *((uint32_t*)&ptr);
      break;
    }
    case LUA_TUSERDATA: {
      const double* d = LuaDouble::TestDouble(L, index);
      if (d != NULL) {
	obsID = (uint32_t) *d;
      } else {
	return NULL;
      }
      break;
    }
    default: {
      return NULL;
    }
  }

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


static inline WorldText* ParseWorldText(lua_State* L, int index)
{
  const int textID = luaL_checkint(L, index);
  const std::vector<WorldText*>& texts = OBSTACLEMGR.getTexts();
  if ((textID < 0) || (textID >= (int)texts.size())) {
    return NULL;
  }
  return texts[textID];
}


static bool isContainer(const Obstacle* obs)
{
  switch (obs->getTypeID()) {
    case teleType:
    case arcType:
    case coneType:
    case sphereType: {
      return true;
    }
    default: {
      return false;
    }
  }
}


static bool PushObstacleList(lua_State* L, int&index,
			     int type, bool containers)
{
  const GroupDefinition& world = OBSTACLEMGR.getWorld();
  if ((type < 0) || (type >= ObstacleTypeCount)) {
    return false;
  }
  const ObstacleList& obsList = world.getList(type);
  const size_t count = obsList.size();
  for (size_t i = 0; i < count; i++) {
    if (containers || !isContainer(obsList[i])) {
      index++;
      PushObstacleGUID(L, obsList[i]->getGUID());
      lua_rawseti(L, -2, index);
    }
  }

  return true;
}


//============================================================================//
//============================================================================//
//
//  Basic obstacle queries
//

int LuaObstacle::GetObstacleList(lua_State* L)
{
  lua_settop(L, 1);
  lua_newtable(L);
  const int table = 1;

  if (!lua_istable(L, table)) {
    const bool containers = lua_isboolean(L, 1) && lua_toboolean(L, 1);
    int index = 0;
    for (int type = 0; type < ObstacleTypeCount; type++) {
      PushObstacleList(L, index, type, containers);
    }
  }
  else {
    const bool containers = lua_isboolean(L, 2) && lua_toboolean(L, 2);
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
	PushObstacleList(L, index, type, containers);
	lua_rawset(L, newTable);
      }
    }
  }

  return 1;
}


int LuaObstacle::GetObstacleName(lua_State* L)
{
  const Obstacle* obs = ParseObstacle(L, 1);
  if (obs == NULL) {
    return luaL_pushnil(L);
  }
  lua_pushstdstring(L, obs->getName());
  return 1;
}


int LuaObstacle::GetObstacleType(lua_State* L)
{
  const Obstacle* obs = ParseObstacle(L, 1);
  if (obs == NULL) {
    return luaL_pushnil(L);
  }
  lua_pushinteger(L, obs->getTypeID());
  return 1;
}


int LuaObstacle::GetObstacleTypeID(lua_State* L)
{
  const Obstacle* obs = ParseObstacle(L, 1);
  if (obs == NULL) {
    return luaL_pushnil(L);
  }
  lua_pushinteger(L, obs->getListID());
  return 1;
}


int LuaObstacle::GetObstacleDriveThrough(lua_State* L)
{
  const Obstacle* obs = ParseObstacle(L, 1);
  if (obs == NULL) {
    return luaL_pushnil(L);
  }
  lua_pushboolean(L, obs->isDriveThrough());
  return 1;
}


int LuaObstacle::GetObstacleShootThrough(lua_State* L)
{
  const Obstacle* obs = ParseObstacle(L, 1);
  if (obs == NULL) {
    return luaL_pushnil(L);
  }
  lua_pushboolean(L, obs->isShootThrough());
  return 1;
}


int LuaObstacle::GetObstacleRicochet(lua_State* L)
{
  const Obstacle* obs = ParseObstacle(L, 1);
  if (obs == NULL) {
    return luaL_pushnil(L);
  }
  lua_pushboolean(L, obs->canRicochet());
  return 1;
}


int LuaObstacle::GetObstaclePosition(lua_State* L)
{
  const Obstacle* obs = ParseObstacle(L, 1);
  if (obs == NULL) {
    return luaL_pushnil(L);
  }
  lua_pushfvec3(L, obs->getPosition());
  return 3;
}


int LuaObstacle::GetObstacleSize(lua_State* L)
{
  const Obstacle* obs = ParseObstacle(L, 1);
  if (obs == NULL) {
    return luaL_pushnil(L);
  }
  lua_pushfvec3(L, obs->getSize());
  return 3;
}


int LuaObstacle::GetObstacleRotation(lua_State* L)
{
  const Obstacle* obs = ParseObstacle(L, 1);
  if (obs == NULL) {
    return luaL_pushnil(L);
  }
  lua_pushfloat(L, obs->getRotation());
  return 1;
}




int LuaObstacle::GetObstacleFlatTop(lua_State* L)
{
  const Obstacle* obs = ParseObstacle(L, 1);
  if (obs == NULL) {
    return luaL_pushnil(L);
  }
  lua_pushboolean(L, obs->isFlatTop());
  return 1;
}


int LuaObstacle::GetObstacleExtents(lua_State* L)
{
  const Obstacle* obs = ParseObstacle(L, 1);
  if (obs == NULL) {
    return luaL_pushnil(L);
  }
  const Extents& exts = obs->getExtents();
  lua_pushfvec3(L, exts.mins);
  lua_pushfvec3(L, exts.maxs);
  return 6;
}


int LuaObstacle::GetObstacleTeam(lua_State* L)
{
  const Obstacle* obs = ParseObstacle(L, 1);
  if (obs == NULL) {
    return luaL_pushnil(L);
  }
  const int baseTeam = obs->getBaseTeam();
  if (baseTeam < 0) {
    return luaL_pushnil(L);
  }
  lua_pushinteger(L, baseTeam);
  return 1;
}


int LuaObstacle::GetObstacleFlipZ(lua_State* L)
{
  const Obstacle* obs = ParseObstacle(L, 1);
  if (obs == NULL) {
    return luaL_pushnil(L);
  }
  lua_pushboolean(L, obs->getZFlip());
  return 1;
}


int LuaObstacle::GetObstacleBorder(lua_State* L)
{
  const Obstacle* obs = ParseObstacle(L, 1);
  if (obs == NULL) {
    return luaL_pushnil(L);
  }
  if (obs->getTypeID() != teleType) {
    return luaL_pushnil(L);
  }
  const Teleporter* tele = (Teleporter*)obs;
  lua_pushfloat(L, tele->getBorder());
  return 1;
}


//============================================================================//
//============================================================================//
//
//  Meshes and faces
//

int LuaObstacle::GetMeshFaceCount(lua_State* L)
{
  const Obstacle* obs = ParseObstacle(L, 1);

  if (obs->getTypeID() != meshType) {
    return luaL_pushnil(L);
  }

  const MeshObstacle* mesh = (const MeshObstacle*)obs;
  lua_pushinteger(L, mesh->getFaceCount());

  return 1;
}


int LuaObstacle::GetMeshFace(lua_State* L)
{
  const Obstacle* obs = ParseObstacle(L, 1);

  if (obs->getTypeID() != meshType) {
    return luaL_pushnil(L);
  }

  const MeshObstacle* mesh = (const MeshObstacle*)obs;

  const int faceID = luaL_checkint(L, 2) - 1;
  if ((faceID < 0) || (faceID >= mesh->getFaceCount())) {
    return luaL_pushnil(L);
  }

  PushObstacleGUID(L, mesh->getFace(faceID)->getGUID());
  return 1;
}


int LuaObstacle::GetFaceMesh(lua_State* L)
{
  const MeshFace* face = ParseMeshFace(L, 1);
  if (face == NULL) {
    return luaL_pushnil(L);
  }
  const MeshObstacle* mesh = face->getMesh();
  if (mesh == NULL) {
    lua_pushboolean(L, false);
    return 1;
  }
  PushObstacleGUID(L, mesh->getGUID());
  return 1;
}


int LuaObstacle::GetFaceElementCount(lua_State* L)
{
  const MeshFace* face = ParseMeshFace(L, 1);
  if (face == NULL) {
    return luaL_pushnil(L);
  }
  const int elements = face->getVertexCount();
  lua_pushinteger(L, elements);
  return 1;
}


int LuaObstacle::GetFaceVerts(lua_State* L)
{
  const MeshFace* face = ParseMeshFace(L, 1);
  if (face == NULL) {
    return luaL_pushnil(L);
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
    return luaL_pushnil(L);
  }
  if (!face->useNormals()) {
    return luaL_pushnil(L);
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
    return luaL_pushnil(L);
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

  // do not generate texcoords if asked not to
  if (lua_isboolean(L, 3) || lua_tobool(L, 3)) {
    lua_pushboolean(L, false);
    return 1;
  }

  // generate texcoords
  const int elements = face->getVertexCount();
  vector<fvec3> vertArray;
  for (int i = 0; i < elements; i++) {
    vertArray.push_back(face->getVertex(i));
  }
  const fvec2& autoScale = face->getMaterial()->getTextureAutoScale(0);
  vector<fvec2> txcdArray;
  txcdArray.resize(elements);
  if (!makeTexcoords(autoScale, face->getPlane(), vertArray, txcdArray)) {
    return luaL_pushnil(L);
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


int LuaObstacle::GetFacePlane(lua_State* L)
{
  const MeshFace* face = ParseMeshFace(L, 1);
  if (face == NULL) {
    return luaL_pushnil(L);
  }
  const fvec4& vec = face->getPlane();
  lua_createtable(L, 4, 0);
  lua_pushfloat(L, vec.x); lua_rawseti(L, -2, 1);
  lua_pushfloat(L, vec.y); lua_rawseti(L, -2, 2);
  lua_pushfloat(L, vec.z); lua_rawseti(L, -2, 3);
  lua_pushfloat(L, vec.w); lua_rawseti(L, -2, 4);
  return 1;
}


int LuaObstacle::GetFacePhyDrv(lua_State* L)
{
  const MeshFace* face = ParseMeshFace(L, 1);
  if (face == NULL) {
    return luaL_pushnil(L);
  }

  const int phydrv = face->getPhysicsDriver();
  if (phydrv < 0) {
    lua_pushboolean(L, false);
    return 1;
  }

  lua_pushinteger(L, phydrv);;
  return 1;
}


int LuaObstacle::GetFaceMaterial(lua_State* L)
{
  const MeshFace* face = ParseMeshFace(L, 1);
  if (face == NULL) {
    return luaL_pushnil(L);
  }
  lua_pushinteger(L, face->getMaterial()->getID());
  return 1;
}


int LuaObstacle::GetFaceSmoothBounce(lua_State* L)
{
  const MeshFace* face = ParseMeshFace(L, 1);
  if (face == NULL) {
    return luaL_pushnil(L);
  }
  lua_pushboolean(L, face->isSmoothBounce());
  return 1;
}


//============================================================================//
//============================================================================//
//
//  Special MeshFace properties
//

int LuaObstacle::GetFaceBaseTeam(lua_State* L)
{
  const MeshFace* face = ParseMeshFace(L, 1);
  if (face == NULL) {
    return luaL_pushnil(L);
  }
  const int team = face->getBaseTeam();
  if (team < 0) {
    return luaL_pushnil(L);
  }
  lua_pushinteger(L, team);
  return 1;
}


int LuaObstacle::GetFaceLinkName(lua_State* L)
{
  const MeshFace* face = ParseMeshFace(L, 1);
  if (face == NULL) {
    return luaL_pushnil(L);
  }
  if (!face->isLinkFace()) {
    return luaL_pushnil(L);
  }
  lua_pushstdstring(L, face->getLinkName());
  return 1;
}


int LuaObstacle::GetFaceLinkSrcID(lua_State* L)
{
  const MeshFace* face = ParseMeshFace(L, 1);
  if (face == NULL) {
    return luaL_pushnil(L);
  }
  const int linkSrcID = linkManager.getLinkSrcID(face);
  if (linkSrcID < 0) {
    return luaL_pushnil(L);
  }
  lua_pushinteger(L, linkSrcID);
  return 1;
}


int LuaObstacle::GetFaceLinkSrcAttribs(lua_State* L)
{
  const MeshFace* face = ParseMeshFace(L, 1);
  if (face == NULL) {
    return luaL_pushnil(L);
  }
  if (!face->isLinkSrc()) {
    return luaL_pushnil(L);
  }

  lua_createtable(L, 0, 7);
  luaset_strbool(L, "touch",    face->linkSrcTouch());
  luaset_strbool(L, "rebound",  face->linkSrcRebound());
  luaset_strbool(L, "noGlow",   face->linkSrcNoGlow());
  luaset_strbool(L, "noSound",  face->linkSrcNoSound());
  luaset_strbool(L, "noEffect", face->linkSrcNoEffect());

  const MeshFace::SpecialData* sd = face->getSpecialData();
  luaset_strstr(L, "shotFailText", sd->linkSrcShotFailText);
  luaset_strstr(L, "tankFailText", sd->linkSrcTankFailText);

  return 1;
}


static void PushFVec3Table(lua_State* L, const std::string& n, const fvec3& v)
{
  lua_pushstdstring(L, n);
  lua_newtable(L);
  luaset_strnum(L, "x", v.x);
  luaset_strnum(L, "y", v.y);
  luaset_strnum(L, "z", v.z);
  lua_rawset(L, -3);
}


static int PushLinkGeometry(lua_State* L, const MeshFace::LinkGeometry& geo)
{
  lua_createtable(L, 0, 15);
  PushFVec3Table(L, "center", geo.center);
  PushFVec3Table(L, "sDir", geo.sDir);
  PushFVec3Table(L, "tDir", geo.tDir);
  PushFVec3Table(L, "pDir", geo.pDir);
  luaset_strnum(L, "sScale", geo.sScale);
  luaset_strnum(L, "tScale", geo.tScale);
  luaset_strnum(L, "pScale", geo.pScale);
  luaset_strnum(L, "angle", geo.angle);
  luaset_strint(L, "centerIndex", geo.centerIndex);
  luaset_strint(L, "sDirIndex", geo.sDirIndex);
  luaset_strint(L, "tDirIndex", geo.tDirIndex);
  luaset_strint(L, "pDirIndex", geo.pDirIndex);
  luaset_strbool(L, "autoSscale", geo.bits & MeshFace::LinkAutoSscale);
  luaset_strbool(L, "autoTscale", geo.bits & MeshFace::LinkAutoTscale);
  luaset_strbool(L, "autoPscale", geo.bits & MeshFace::LinkAutoPscale);
  return 1;
}


int LuaObstacle::GetFaceLinkSrcGeometry(lua_State* L)
{
  const MeshFace* face = ParseMeshFace(L, 1);
  if (face == NULL) {
    return luaL_pushnil(L);
  }
  if (!face->isLinkSrc()) {
    return luaL_pushnil(L);
  }
  return PushLinkGeometry(L, face->getSpecialData()->linkSrcGeo);
}


int LuaObstacle::GetFaceLinkDstGeometry(lua_State* L)
{
  const MeshFace* face = ParseMeshFace(L, 1);
  if (face == NULL) {
    return luaL_pushnil(L);
  }
  if (!face->isLinkDst()) {
    return luaL_pushnil(L);
  }
  return PushLinkGeometry(L, face->getSpecialData()->linkDstGeo);
}


int LuaObstacle::GetFaceZoneParams(lua_State* L)
{
  const MeshFace* face = ParseMeshFace(L, 1);
  if (face == NULL) {
    return luaL_pushnil(L);
  }
  if (!face->isSpecial()) {
    return luaL_pushnil(L);
  }
  const vector<string>& zoneParams = face->getSpecialData()->zoneParams;
  if (zoneParams.empty()) {
    lua_pushboolean(L, false);
    return 1;
  }
  lua_createtable(L, zoneParams.size(), 0);
  for (size_t i = 0; i < zoneParams.size(); i++) {
    lua_pushstdstring(L, zoneParams[i]);
    lua_rawseti(L, -2, i + 1);
  }
  return 1;
}


//============================================================================//
//============================================================================//
//
//  links
//

int LuaObstacle::GetLinkSrcIDs(lua_State* L)
{
  const std::string srcName = luaL_checkstring(L, 1);

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
  const std::string dstName = luaL_checkstring(L, 1);

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
    return luaL_pushnil(L);
  }
  lua_pushstdstring(L, face->getLinkName());
  return 1;
}


int LuaObstacle::GetLinkDstName(lua_State* L)
{
  const int linkDstID = luaL_checkint(L, 1);
  const MeshFace* face = linkManager.getLinkDstFace(linkDstID);
  if (face == NULL) {
    return luaL_pushnil(L);
  }
  lua_pushstdstring(L, face->getLinkName());
  return 1;
}


int LuaObstacle::GetLinkSrcFace(lua_State* L)
{
  const int linkSrcID = luaL_checkint(L, 1);
  const MeshFace* face = linkManager.getLinkSrcFace(linkSrcID);
  if (face == NULL) {
    return luaL_pushnil(L);
  }
  PushObstacleGUID(L, face->getGUID());
  return 1;
}


int LuaObstacle::GetLinkDstFace(lua_State* L)
{
  const int linkDstID = luaL_checkint(L, 1);
  const MeshFace* face = linkManager.getLinkDstFace(linkDstID);
  if (face == NULL) {
    return luaL_pushnil(L);
  }
  PushObstacleGUID(L, face->getGUID());
  return 1;
}


int LuaObstacle::GetLinkDestinations(lua_State* L)
{
  L = L;
  const int linkSrcID = luaL_checkint(L, 1);
  const MeshFace* srcFace = linkManager.getLinkSrcFace(linkSrcID);
  if (srcFace == NULL) {
    return luaL_pushnil(L);
  }
  const LinkManager::LinkMap& linkMap = linkManager.getLinkMap();
  LinkManager::LinkMap::const_iterator it = linkMap.find(srcFace);
  if (it == linkMap.end()) {
    return luaL_pushnil(L);
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
//  DrawInfo
//

static bool drawInfoArrayStrings = false;


static void PushDrawCmd(lua_State* L, const DrawCmd& cmd)
{
  lua_newtable(L);

  luaset_strint(L, "mode", cmd.drawMode);

  lua_pushliteral(L, "indices");
  if (drawInfoArrayStrings) {
    lua_createtable(L, 0, 2);
    if (cmd.indexType == DrawCmd::DrawIndexUShort) {
      luaset_strstr(L,   "type", "UNSIGNED_SHORT");
      lua_pushliteral(L, "data");
      lua_pushlstring(L, (const char*) cmd.indices,
			 cmd.count * sizeof(unsigned short));
      lua_rawset(L, -3);
    }
    else if (cmd.indexType == DrawCmd::DrawIndexUInt) {
      luaset_strstr(L,   "type", "UNSIGNED_INT");
      lua_pushliteral(L, "data");
      lua_pushlstring(L, (const char*) cmd.indices,
		      cmd.count * sizeof(unsigned int));
      lua_rawset(L, -3);
    }
  }
  else {
    lua_createtable(L, cmd.count, 0);
    if (cmd.indexType == DrawCmd::DrawIndexUShort) {
      unsigned short* array = (unsigned short*)cmd.indices;
      for (int i = 0; i < cmd.count; i++) {
	lua_pushinteger(L, array[i]);
	lua_rawseti(L, -2, i + 1);
      }
    }
    else if (cmd.indexType == DrawCmd::DrawIndexUInt) {
      unsigned int* array = (unsigned int*)cmd.indices;
      for (int i = 0; i < cmd.count; i++) {
	lua_pushinteger(L, array[i]);
	lua_rawseti(L, -2, i + 1);
      }
    }
  }
  lua_rawset(L, -3);
}


static void PushDrawSet(lua_State* L, const DrawSet& set)
{
  lua_newtable(L);

  luaset_strint(L, "material", set.material->getID());
  luaset_strbool(L, "wantList", set.wantList);

  lua_pushliteral(L, "sphere");
  lua_createtable(L, 0, 4);
  luaset_strnum(L, "x",  set.sphere.x);
  luaset_strnum(L, "y",  set.sphere.y);
  luaset_strnum(L, "z",  set.sphere.z);
  luaset_strnum(L, "r2", set.sphere.w);
  lua_rawset(L, -3);

  lua_pushliteral(L, "commands");
  lua_createtable(L, set.count, 0);
  for (int i = 0; i < set.count; i++) {
    PushDrawCmd(L, set.cmds[i]);
    lua_rawseti(L, -2, i + 1);
  }
  lua_rawset(L, -3);
}


static void PushDrawLod(lua_State* L, const DrawLod& lod)
{
  lua_newtable(L);

  luaset_strnum(L, "lengthPerPixel", lod.lengthPerPixel);

  lua_pushliteral(L, "lods");
  lua_createtable(L, lod.count, 0);
  for (int i = 0; i < lod.count; i++) {
    PushDrawSet(L, lod.sets[i]);
    lua_rawseti(L, -2, i + 1);
  }
  lua_rawset(L, -3);
}


static void PushTransform(lua_State* L, const MeshTransform::Tool& tool)
{
  lua_newtable(L);

  luaset_strbool(L, "inverted", tool.isInverted());
  luaset_strbool(L, "skewed",   tool.isSkewed());

  lua_pushliteral(L, "matrix");
  const float* matrix = tool.getMatrix();
  lua_createtable(L, 16, 0);
  for (int i = 0; i < 16; i++) {
    lua_pushfloat(L, matrix[i]);
    lua_rawseti(L, -2, i + 1);
  }
  lua_rawset(L, -3);
}


static void PushDrawInfo(lua_State* L, const MeshDrawInfo& info)
{
  lua_newtable(L);

  luaset_strstr(L, "name", info.getName());

  const AnimationInfo* anim = info.getAnimationInfo();
  if (anim != NULL) {
    luaset_strnum(L, "angvel", anim->angvel);
  }

  lua_pushliteral(L, "sphere");
  const fvec4& sphere = info.getSphere();
  lua_createtable(L, 0, 4);
  luaset_strnum(L, "x",  sphere.x);
  luaset_strnum(L, "y",  sphere.y);
  luaset_strnum(L, "z",  sphere.z);
  luaset_strnum(L, "r2", sphere.w);
  lua_rawset(L, -3);

  lua_pushliteral(L, "extents");
  const Extents& exts = info.getExtents();
  lua_createtable(L, 0, 6);
  luaset_strnum(L, "minx", exts.mins.x);
  luaset_strnum(L, "miny", exts.mins.y);
  luaset_strnum(L, "minz", exts.mins.z);
  luaset_strnum(L, "maxx", exts.maxs.x);
  luaset_strnum(L, "maxy", exts.maxs.y);
  luaset_strnum(L, "maxz", exts.maxs.z);
  lua_rawset(L, -3);

  const MeshTransform::Tool* tool = info.getTransformTool();
  if (tool != NULL) {
    lua_pushliteral(L, "transform");
    PushTransform(L, *tool);
    lua_rawset(L, -3);
  }

  const int cornerCount = info.getCornerCount();

  lua_pushliteral(L, "verts");
  const fvec3* verts = info.getVertices();
  if (drawInfoArrayStrings) {
    lua_pushlstring(L, (const char*) verts, cornerCount * sizeof(fvec3));
  }
  else {
    lua_createtable(L, cornerCount, 0);
    for (int i = 0; i < cornerCount; i++) {
      lua_createtable(L, 3, 0);
      lua_pushfloat(L, verts[i].x); lua_rawseti(L, -2, 1);
      lua_pushfloat(L, verts[i].y); lua_rawseti(L, -2, 2);
      lua_pushfloat(L, verts[i].z); lua_rawseti(L, -2, 3);
      lua_rawseti(L, -2, i + 1);
    }
  }
  lua_rawset(L, -3);

  lua_pushliteral(L, "norms");
  const fvec3* norms = info.getNormals();
  if (drawInfoArrayStrings) {
    lua_pushlstring(L, (const char*) norms, cornerCount * sizeof(fvec3));
  }
  else {
    lua_createtable(L, cornerCount, 0);
    for (int i = 0; i < cornerCount; i++) {
      lua_createtable(L, 3, 0);
      lua_pushfloat(L, norms[i].x); lua_rawseti(L, -2, 1);
      lua_pushfloat(L, norms[i].y); lua_rawseti(L, -2, 2);
      lua_pushfloat(L, norms[i].z); lua_rawseti(L, -2, 3);
      lua_rawseti(L, -2, i + 1);
    }
  }
  lua_rawset(L, -3);

  lua_pushliteral(L, "txcds");
  const fvec2* txcds = info.getTexcoords();
  if (drawInfoArrayStrings) {
    lua_pushlstring(L, (const char*) txcds, cornerCount * sizeof(fvec2));
  }
  else {
    lua_createtable(L, cornerCount, 0);
    for (int i = 0; i < cornerCount; i++) {
      lua_createtable(L, 2, 0);
      lua_pushfloat(L, txcds[i].s); lua_rawseti(L, -2, 1);
      lua_pushfloat(L, txcds[i].t); lua_rawseti(L, -2, 2);
      lua_rawseti(L, -2, i + 1);
    }
  }
  lua_rawset(L, -3);

  lua_pushliteral(L, "drawLods");
  const DrawLod* drawLods = info.getDrawLods();
  lua_createtable(L, info.getLodCount(), 0);
  for (int i = 0; i < info.getLodCount(); i++) {
    PushDrawLod(L, drawLods[i]);
    lua_rawseti(L, -2, i + 1);
  }
  lua_rawset(L, -3);

  lua_pushliteral(L, "radarLods");
  const DrawLod* radarLods = info.getRadarLods();
  lua_createtable(L, info.getRadarCount(), 0);
  for (int i = 0; i < info.getRadarCount(); i++) {
    PushDrawLod(L, radarLods[i]);
    lua_rawseti(L, -2, i + 1);
  }
  lua_rawset(L, -3);
}


int LuaObstacle::HasMeshDrawInfo(lua_State* L)
{
  const Obstacle* obs = ParseObstacle(L, 1);
  if (obs == NULL) {
    return luaL_pushnil(L);
  }
  if (obs->getTypeID() != meshType) {
    return luaL_pushnil(L);
  }
  const MeshObstacle* mesh = (const MeshObstacle*)obs;
  const MeshDrawInfo* drawInfo = mesh->getDrawInfo();
  lua_pushboolean(L, (drawInfo != NULL) && drawInfo->isValid());
  return 1;
}


int LuaObstacle::GetMeshDrawInfo(lua_State* L)
{
  const Obstacle* obs = ParseObstacle(L, 1);
  if (obs == NULL) {
    return luaL_pushnil(L);
  }
  if (obs->getTypeID() != meshType) {
    return luaL_pushnil(L);
  }
  const MeshObstacle* mesh = (const MeshObstacle*)obs;
  const MeshDrawInfo* drawInfo = mesh->getDrawInfo();
  if ((drawInfo == NULL) || !drawInfo->isValid()) {
    return luaL_pushnil(L);
  }
  drawInfoArrayStrings = lua_isboolean(L, 2) && lua_toboolean(L, 2);
  PushDrawInfo(L, *drawInfo);
  return 1;
}

//============================================================================//
//============================================================================//

int LuaObstacle::GetWorldTextCount(lua_State* L)
{
  lua_pushinteger(L, OBSTACLEMGR.getTexts().size());
  return 1;
}


int LuaObstacle::GetWorldTextName(lua_State* L)
{
  const WorldText* text = ParseWorldText(L, 1);
  lua_pushstdstring(L, text->name);
  return 1;
}


int LuaObstacle::GetWorldTextData(lua_State* L)
{
  const WorldText* text = ParseWorldText(L, 1);
  if (text->useBZDB) {
    lua_pushboolean(L, false);
  } else {
    lua_pushstdstring(L, text->data);
  }
  return 1;
}


int LuaObstacle::GetWorldTextVarName(lua_State* L)
{
  const WorldText* text = ParseWorldText(L, 1);
  if (!text->useBZDB) {
    lua_pushboolean(L, false);
  } else {
    lua_pushstdstring(L, text->data);
  }
  return 1;
}


int LuaObstacle::GetWorldTextFont(lua_State* L)
{
  const WorldText* text = ParseWorldText(L, 1);
  lua_pushstdstring(L, text->font);
  return 1;
}


int LuaObstacle::GetWorldTextFontSize(lua_State* L)
{
  const WorldText* text = ParseWorldText(L, 1);
  lua_pushfloat(L, text->fontSize);
  return 1;
}


int LuaObstacle::GetWorldTextLineSpace(lua_State* L)
{
  const WorldText* text = ParseWorldText(L, 1);
  lua_pushfloat(L, text->lineSpace);
  return 1;
}


int LuaObstacle::GetWorldTextJustify(lua_State* L)
{
  const WorldText* text = ParseWorldText(L, 1);
  lua_pushfloat(L, text->justify);
  return 1;
}


int LuaObstacle::GetWorldTextFixedWidth(lua_State* L)
{
  const WorldText* text = ParseWorldText(L, 1);
  lua_pushfloat(L, text->fixedWidth);
  return 1;
}


int LuaObstacle::GetWorldTextLengthPerPixel(lua_State* L)
{
  const WorldText* text = ParseWorldText(L, 1);
  lua_pushfloat(L, text->lengthPerPixel);
  return 1;
}


int LuaObstacle::GetWorldTextBillboard(lua_State* L)
{
  const WorldText* text = ParseWorldText(L, 1);
  lua_pushboolean(L, text->billboard);
  return 1;
}


int LuaObstacle::GetWorldTextMaterial(lua_State* L)
{
  const WorldText* text = ParseWorldText(L, 1);
  lua_pushinteger(L, text->bzMaterial->getID());
  return 1;
}


int LuaObstacle::GetWorldTextTransform(lua_State* L)
{
  const WorldText* text = ParseWorldText(L, 1);
  const MeshTransform& xform = text->xform;
  const MeshTransform::Tool tool(xform);
  const float* matrix = tool.getMatrix();
  for (int i = 0; i < 16; i++) {
    lua_pushfloat(L, matrix[i]);
  }
  return 16;
}


//============================================================================//
//============================================================================//

int LuaObstacle::GetObstaclesInBox(lua_State* L)
{
  const fvec3 pos     = luaL_checkfvec3(L, 2);
  const fvec3 size    = luaL_checkfvec3(L, 5);
  const float radians = luaL_optfloat(L, 8, 0.0f);

  Extents exts;
  if (radians == 0.0f) {
    exts.mins = pos - fvec3(size.x, size.y, 0.0f);
    exts.maxs = pos + fvec3(size.x, size.y, size.z);
  }
  else {
    fvec2 pv(+size.x, +size.y);
    fvec2 nv(+size.x, -size.y);
    pv.rotate(radians);
    nv.rotate(radians);
    pv = fvec2(fabsf(pv.x), fabsf(pv.y));
    nv = fvec2(fabsf(nv.x), fabsf(nv.y));
    const float sx = (pv.x > nv.x) ? pv.x : nv.x;
    const float sy = (pv.y > nv.y) ? pv.y : nv.y;
    exts.mins = fvec3(pos.x - sx, pos.y - sy, pos.z),
    exts.maxs = fvec3(pos.x + sx, pos.y + sy, pos.z + size.z);
  }

  const ObsList* obsList = COLLISIONMGR.axisBoxTest(exts);
  const int listCount = obsList->count;

  lua_createtable(L, listCount, 0);
  int index = 0;
  for (int i = 0; i < listCount; i++) {
    const Obstacle* obs = obsList->list[i];
    if (obs->inBox(pos, radians, size.x, size.y, size.z)) {
      index++;
      PushObstacleGUID(L, obs->getGUID());
      lua_rawseti(L, -2, index);
    }
  }

  return 1;
}


int LuaObstacle::ObstacleBoxTest(lua_State* L)
{
  const Obstacle* obs = ParseObstacle(L, 1);
  const fvec3 pos     = luaL_checkfvec3(L, 2);
  const fvec3 size    = luaL_checkfvec3(L, 5);
  const float radians = luaL_optfloat(L, 8, 0.0f);
  lua_pushboolean(L, obs->inBox(pos, radians, size.x, size.y, size.z));
  return 1;
}


int LuaObstacle::ObstacleRayTest(lua_State* L)
{
  const Obstacle* obs = ParseObstacle(L, 1);
  const fvec3 pos = luaL_checkfvec3(L, 2);
  const fvec3 vel = luaL_checkfvec3(L, 5);
  const float rayTime = obs->intersect(Ray(pos, vel));
  if (rayTime < 0.0f) {
    lua_pushboolean(L, false);
  } else {
    lua_pushfloat(L, rayTime);
  }
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
