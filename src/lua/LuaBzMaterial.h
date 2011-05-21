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

#ifndef LUA_BZMATERIAL_H
#define LUA_BZMATERIAL_H


#include <string>

struct lua_State;


class LuaBzMaterial {
  public:
    typedef bool (*BlendParser)(const std::string&,
                                unsigned int&, unsigned int&);
  public:
    static bool PushEntries(lua_State* L);

    static void SetBlendParser(BlendParser blendParser);

  private: // call-outs
    static int GetMaterialList(lua_State* L);

    static int GetMaterialID(lua_State* L);
    static int GetMaterialName(lua_State* L);
    static int GetMaterialOrder(lua_State* L);

    static int GetMaterialAmbient(lua_State* L);
    static int GetMaterialDiffuse(lua_State* L);
    static int GetMaterialSpecular(lua_State* L);
    static int GetMaterialEmission(lua_State* L);
    static int GetMaterialShininess(lua_State* L);

    static int GetMaterialDynCol(lua_State* L);
    static int GetMaterialOccluder(lua_State* L);
    static int GetMaterialGroupAlpha(lua_State* L);
    static int GetMaterialNoLighting(lua_State* L);
    static int GetMaterialNoRadar(lua_State* L);
    static int GetMaterialNoShadowCast(lua_State* L);
    static int GetMaterialNoShadowRecv(lua_State* L);
    static int GetMaterialNoCulling(lua_State* L);
    static int GetMaterialNoSorting(lua_State* L);
    static int GetMaterialAlphaThresh(lua_State* L);
    static int GetMaterialFlatShade(lua_State* L);
    static int GetMaterialBlending(lua_State* L);

    static int GetMaterialTextureCount(lua_State* L);
    static int GetMaterialTexture(lua_State* L);
    static int GetMaterialTextureInfo(lua_State* L);

    static int GetMaterialShaderCount(lua_State* L);
    static int GetMaterialShader(lua_State* L);

    static int GetMaterialTable(lua_State* L);

  private:
    static BlendParser blendParser;
};


#endif // LUA_BZMATERIAL_H


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=2 tabstop=8 expandtab
