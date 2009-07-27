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

#ifndef LUA_OBSTACLE_H
#define LUA_OBSTACLE_H

struct lua_State;

class LuaObstacle {
  public:
    static bool PushEntries(lua_State* L);

  private: // call-outs
    static int GetObstacleList(lua_State* L);
    static int GetObstaclesInBox(lua_State* L);

    static int GetObstacleName(lua_State* L);
    static int GetObstacleType(lua_State* L);

    static int GetObstacleDriveThrough(lua_State* L);
    static int GetObstacleShootThrough(lua_State* L);
    static int GetObstacleRicochet(lua_State* L);

    static int GetObstaclePosition(lua_State* L);
    static int GetObstacleSize(lua_State* L);
    static int GetObstacleRotation(lua_State* L);

    static int GetObstacleFlatTop(lua_State* L);
    static int GetObstacleExtents(lua_State* L);
    static int GetObstacleTeam(lua_State* L);
    static int GetObstacleFlipZ(lua_State* L);

    static int GetMeshFaceCount(lua_State* L);
    static int GetMeshFace(lua_State* L);

    static int GetFaceMesh(lua_State* L);
    static int GetFaceVertCount(lua_State* L);
    static int GetFaceVerts(lua_State* L);
    static int GetFaceNorms(lua_State* L);
    static int GetFaceTxcds(lua_State* L);
    static int GetFacePlane(lua_State* L);
    static int GetFacePhyDrv(lua_State* L);
    static int GetFaceSmoothBounce(lua_State* L);
    static int GetFaceLinkName(lua_State* L);

    static int GetLinkSrcIDs(lua_State* L);
    static int GetLinkDstIDs(lua_State* L);
    static int GetLinkSrcName(lua_State* L);
    static int GetLinkDstName(lua_State* L);
    static int GetLinkSrcFace(lua_State* L);
    static int GetLinkDstFace(lua_State* L);
    static int GetLinkDestinations(lua_State* L);

    static int GetPhyDrvID(lua_State* L);
    static int GetPhyDrvName(lua_State* L);

    static int ObstacleRayTime(lua_State* L);
    static int ObstacleBoxTest(lua_State* L);
};


#endif // LUA_OBSTACLE_H


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
