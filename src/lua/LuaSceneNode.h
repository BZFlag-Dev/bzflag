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

#ifndef LUA_SCENENODE_H
#define LUA_SCENENODE_H

struct lua_State;
class SceneDatabase;


class LuaSceneNodeMgr {
  public:
    static bool PushEntries(lua_State* L);

    static void AddSceneNodes(SceneDatabase&);
    static void ClearSceneNodes();

  private:
    static bool CreateMetatable(lua_State* L);

  private: // call-outs
    static int CreateSceneNode(lua_State* L);
    static int DeleteSceneNode(lua_State* L);

    static int IsSceneNode(lua_State* L);
    static int IsValidSceneNode(lua_State* L);

    static int SetSceneNodeFunc(lua_State* L);
    static int SetSceneNodeData(lua_State* L);
    static int SetSceneNodeActive(lua_State* L);
    static int SetSceneNodePosition(lua_State* L);
    static int SetSceneNodeRadius(lua_State* L);
    static int SetSceneNodeTracking(lua_State* L);
    static int SetSceneNodeUseArrays(lua_State* L);
    static int SetSceneNodeUseTransform(lua_State* L);

    static int GetSceneNodeInfo(lua_State* L);
    static int GetSceneNodePosition(lua_State* L);
    static int GetSceneNodeRadius(lua_State* L);
    static int GetSceneNodeRadians(lua_State* L);
};


#endif // LUA_SCENENODE_H


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
