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
#include "LuaBzMaterial.h"

// system headers
#include <string>
using std::string;

// common headers
#include "BzMaterial.h"
#include "DynamicColor.h"
#include "TextureMatrix.h"

// local headers
#include "LuaHeader.h"
#include "LuaUtils.h"


LuaBzMaterial::BlendParser LuaBzMaterial::blendParser = NULL;


//============================================================================//
//============================================================================//

bool LuaBzMaterial::PushEntries(lua_State* L)
{
  PUSH_LUA_CFUNC(L, GetMaterialList);

  PUSH_LUA_CFUNC(L, GetMaterialID);
  PUSH_LUA_CFUNC(L, GetMaterialName);
  PUSH_LUA_CFUNC(L, GetMaterialOrder);

  PUSH_LUA_CFUNC(L, GetMaterialAmbient);
  PUSH_LUA_CFUNC(L, GetMaterialDiffuse);
  PUSH_LUA_CFUNC(L, GetMaterialSpecular);
  PUSH_LUA_CFUNC(L, GetMaterialEmission);
  PUSH_LUA_CFUNC(L, GetMaterialShininess);

  PUSH_LUA_CFUNC(L, GetMaterialDynCol);
  PUSH_LUA_CFUNC(L, GetMaterialOccluder);
  PUSH_LUA_CFUNC(L, GetMaterialGroupAlpha);
  PUSH_LUA_CFUNC(L, GetMaterialNoLighting);
  PUSH_LUA_CFUNC(L, GetMaterialNoRadar);
  PUSH_LUA_CFUNC(L, GetMaterialNoShadowCast);
  PUSH_LUA_CFUNC(L, GetMaterialNoShadowRecv);
  PUSH_LUA_CFUNC(L, GetMaterialNoCulling);
  PUSH_LUA_CFUNC(L, GetMaterialNoSorting);
  PUSH_LUA_CFUNC(L, GetMaterialAlphaThresh);
  PUSH_LUA_CFUNC(L, GetMaterialFlatShade);
  PUSH_LUA_CFUNC(L, GetMaterialBlending);

  PUSH_LUA_CFUNC(L, GetMaterialTextureCount);
  PUSH_LUA_CFUNC(L, GetMaterialTexture);
  PUSH_LUA_CFUNC(L, GetMaterialTextureInfo);

  PUSH_LUA_CFUNC(L, GetMaterialShaderCount);
  PUSH_LUA_CFUNC(L, GetMaterialShader);

  PUSH_LUA_CFUNC(L, GetMaterialTable);

  return true;
}


void LuaBzMaterial::SetBlendParser(BlendParser parser)
{
  blendParser = parser;
}


//============================================================================//
//============================================================================//

static const BzMaterial* ParseBzMat(lua_State* L, int index)
{
  switch (lua_type(L, index)) {
    case LUA_TNUMBER: {
      const int matIndex = luaL_checkint(L, index);
      if ((matIndex < 0) || (matIndex >= MATERIALMGR.getCount())) {
	return NULL;
      }
      return MATERIALMGR.getMaterial(matIndex);
    }
    case LUA_TSTRING: {
      const string matName = lua_tostring(L, index);
      return MATERIALMGR.findMaterial(matName);
    }
    default: {
      luaL_error(L, "expected number or string");
      return NULL;
    }
  }
}


static void PushColor(lua_State* L, const float* color)
{
  lua_pushfloat(L, color[0]);
  lua_pushfloat(L, color[1]);
  lua_pushfloat(L, color[2]);
  lua_pushfloat(L, color[3]);
}


//============================================================================//
//============================================================================//

int LuaBzMaterial::GetMaterialList(lua_State* L)
{
  const int count = MATERIALMGR.getCount();
  lua_createtable(L, count, 0);
  for (int i = 0; i < count; i++) {
    lua_pushint(L, i);
    lua_rawseti(L, -2, i + 1);
  }
  return 1;
}


int LuaBzMaterial::GetMaterialID(lua_State* L)
{
  const BzMaterial* mat = NULL;
  if (lua_israwnumber(L, 1)) {
    const int matID = lua_toint(L, 1);
    if ((matID >= 0) && (matID < MATERIALMGR.getCount())) {
      mat = MATERIALMGR.getMaterial(matID);
    }
  }
  else if (lua_israwstring(L, 1)) {
    const string matName = lua_tostring(L, 1);
    mat = MATERIALMGR.findMaterial(matName);
  }
  else {
    return luaL_pushnil(L);
  }

  if (mat == NULL) {
    return luaL_pushnil(L);
  }

  lua_pushinteger(L, mat->getID());
  return 1;
}


//============================================================================//

#define MAT_PARAMETER_FUNC(name, getFunc, pushFunc, count) \
  int LuaBzMaterial::GetMaterial ## name(lua_State* L)     \
  {							\
    const BzMaterial* mat = ParseBzMat(L, 1);	      \
    if (mat == NULL) {				     \
      lua_pushnil(L);				      \
      return 1;					    \
    }						      \
    pushFunc(L, mat->get ## getFunc());		    \
    return count;					  \
  }

MAT_PARAMETER_FUNC(Name,	 Name,	   lua_pushstdstring, 1)
MAT_PARAMETER_FUNC(Order,	Order,	  lua_pushinteger,   1)
MAT_PARAMETER_FUNC(Ambient,      Ambient,	PushColor,	 4)
MAT_PARAMETER_FUNC(Diffuse,      Diffuse,	PushColor,	 4)
MAT_PARAMETER_FUNC(Emission,     Emission,       PushColor,	 4)
MAT_PARAMETER_FUNC(Specular,     Specular,       PushColor,	 4)
MAT_PARAMETER_FUNC(Shininess,    Shininess,      lua_pushfloat,     1)
MAT_PARAMETER_FUNC(AlphaThresh,  AlphaThreshold, lua_pushfloat,     1)
MAT_PARAMETER_FUNC(Occluder,     Occluder,       lua_pushboolean,   1)
MAT_PARAMETER_FUNC(GroupAlpha,   GroupAlpha,     lua_pushboolean,   1)
MAT_PARAMETER_FUNC(NoLighting,   NoLighting,     lua_pushboolean,   1)
MAT_PARAMETER_FUNC(NoRadar,      NoRadar,	lua_pushboolean,   1)
MAT_PARAMETER_FUNC(NoShadowCast, NoShadowCast,   lua_pushboolean,   1)
MAT_PARAMETER_FUNC(NoShadowRecv, NoShadowRecv,   lua_pushboolean,   1)
MAT_PARAMETER_FUNC(NoCulling,    NoCulling,      lua_pushboolean,   1)
MAT_PARAMETER_FUNC(NoSorting,    NoSorting,      lua_pushboolean,   1)
MAT_PARAMETER_FUNC(FlatShade,    FlatShade,      lua_pushboolean,   1)


int LuaBzMaterial::GetMaterialBlending(lua_State* L)
{
  const BzMaterial* mat = ParseBzMat(L, 1);
  if (mat == NULL) {
    return luaL_pushnil(L);
  }
  const std::string& blending = mat->getBlendFactors();
  if (blending.empty()) {
    lua_pushboolean(L, false);
    lua_pushstring(L, "");
    return 2;
  }
  unsigned int src, dst;
  if (blendParser == NULL) {
    lua_pushstdstring(L, blending);
    return 1;
  }
  if (!blendParser(blending, src, dst)) {
    lua_pushboolean(L, false);
    lua_pushstdstring(L, blending);
    return 2;
  }
  lua_pushinteger(L, src);
  lua_pushinteger(L, dst);
  lua_pushstdstring(L, blending);
  return 3;
}



//============================================================================//

int LuaBzMaterial::GetMaterialDynCol(lua_State* L)
{
  const BzMaterial* mat = ParseBzMat(L, 1);
  if (mat == NULL) {
    return luaL_pushnil(L);
  }
  const int dyncol = mat->getDynamicColor();
  if (dyncol >= 0) {
    lua_pushinteger(L, dyncol);
  } else {
    lua_pushboolean(L, false);
  }
  return 1;
}


int LuaBzMaterial::GetMaterialTextureCount(lua_State* L)
{
  const BzMaterial* mat = ParseBzMat(L, 1);
  if (mat == NULL) {
    return luaL_pushnil(L);
  }
  lua_pushinteger(L, mat->getTextureCount());
  return 1;
}


int LuaBzMaterial::GetMaterialTexture(lua_State* L)
{
  const BzMaterial* mat = ParseBzMat(L, 1);
  if (mat == NULL) {
    return luaL_pushnil(L);
  }
  const int index = luaL_optint(L, 2, 1) - 1;
  if ((index < 0) || (index >= mat->getTextureCount())) {
    return luaL_pushnil(L);
  }
  lua_pushstdstring(L, mat->getTexture(index));
  return 1;
}


int LuaBzMaterial::GetMaterialTextureInfo(lua_State* L)
{
  const BzMaterial* mat = ParseBzMat(L, 1);
  if (mat == NULL) {
    return luaL_pushnil(L);
  }
  const int index = luaL_optint(L, 2, 1) - 1;
  if ((index < 0) || (index >= mat->getTextureCount())) {
    return luaL_pushnil(L);
  }
  lua_newtable(L);
  luaset_strstr(L,  "name",      mat->getTexture(index));
  luaset_strstr(L,  "local",     mat->getTextureLocal(index));
  luaset_strint(L,  "envMode",   mat->getCombineMode(index));
  luaset_strbool(L, "useAlpha",  mat->getUseTextureAlpha(index));
  luaset_strbool(L, "useColor",  mat->getUseColorOnTexture(index));
  luaset_strbool(L, "sphereMap", mat->getUseSphereMap(index));
  const int texmat = mat->getTextureMatrix(index);
  if (texmat >= 0) {
    luaset_strint(L,  "texmat", texmat);
  } else {
    luaset_strbool(L, "texmat", false);
  }
  return 1;
}


int LuaBzMaterial::GetMaterialShaderCount(lua_State* L)
{
  const BzMaterial* mat = ParseBzMat(L, 1);
  if (mat == NULL) {
    return luaL_pushnil(L);
  }
  lua_pushinteger(L, mat->getShaderCount());
  return 1;
}


int LuaBzMaterial::GetMaterialShader(lua_State* L)
{
  const BzMaterial* mat = ParseBzMat(L, 1);
  if (mat == NULL) {
    return luaL_pushnil(L);
  }
  const int index = luaL_optint(L, 2, 1) - 1;
  if ((index < 0) || (index >= mat->getShaderCount())) {
    return luaL_pushnil(L);
  }
  lua_pushstdstring(L, mat->getShader(index));
  return 1;
}


//============================================================================//
//============================================================================//
//
//  GetMaterialTable()
//

static void PushColorTable(lua_State* L,
			   const std::string& name, const fvec4& color)
{
  lua_pushstdstring(L, name);
  lua_createtable(L, 4, 0);
  lua_pushfloat(L, color[0]); lua_rawseti(L, -2, 1);
  lua_pushfloat(L, color[1]); lua_rawseti(L, -2, 2);
  lua_pushfloat(L, color[2]); lua_rawseti(L, -2, 3);
  lua_pushfloat(L, color[3]); lua_rawseti(L, -2, 4);
  lua_rawset(L, -3);
}


static void PushIntOrFalse(lua_State* L, const std::string& name, int index)
{
  if (index < 0) {
    luaset_strbool(L, name, false);
  } else {
    luaset_strint(L, name, index);
  }
}


int LuaBzMaterial::GetMaterialTable(lua_State* L)
{
  const BzMaterial* mat = ParseBzMat(L, 1);
  if (mat == NULL) {
    return luaL_pushnil(L);
  }

  lua_newtable(L); {
    // name
    luaset_strint(L, "id",   mat->getID());
    luaset_strstr(L, "name", mat->getName());

    // aliases
    lua_pushliteral(L, "aliases");
    lua_newtable(L);
    {
      const std::vector<std::string>& aliases = mat->getAliases();
      for (size_t a = 0; a < aliases.size(); a++) {
	lua_pushstdstring(L, aliases[a]);
	lua_rawseti(L, -2, a + 1);
      }
    }
    lua_rawset(L, -3);

    // order
    luaset_strint(L, "order", mat->getOrder());

    // colors
    PushColorTable(L, "ambient",   mat->getAmbient());
    PushColorTable(L, "diffuse",   mat->getDiffuse());
    PushColorTable(L, "emission",  mat->getEmission());
    PushColorTable(L, "specular",  mat->getSpecular());
    luaset_strnum(L,  "shininess", mat->getShininess());
    PushIntOrFalse(L, "dyncol",    mat->getDynamicColor());

    // misc
    luaset_strbool(L, "occluder",       mat->getOccluder());
    luaset_strbool(L, "culling",       !mat->getNoCulling());
    luaset_strbool(L, "sorting",       !mat->getNoSorting());
    luaset_strbool(L, "lighting",      !mat->getNoLighting());
    luaset_strbool(L, "flatShade",      mat->getFlatShade());
    luaset_strnum(L,  "alphaThresh",    mat->getAlphaThreshold());
    luaset_strbool(L, "groupAlpha",     mat->getGroupAlpha());
    luaset_strbool(L, "radarSpecial",   mat->getRadarSpecial());
    luaset_strbool(L, "radar",	 !mat->getNoRadar());
    luaset_strbool(L, "radarOutline",  !mat->getNoRadarOutline());
    luaset_strbool(L, "shadowCast",    !mat->getNoShadowCast());
    luaset_strbool(L, "shadowRecv",    !mat->getNoShadowRecv());
    luaset_strbool(L, "shadowTexture",  mat->getTextureShadow());

    // blending
    lua_pushliteral(L, "blending");
    const std::string& blendFactors = mat->getBlendFactors();
    unsigned int blendSrc, blendDst;
    if (blendParser == NULL) {
      luaset_strstr(L, "text", blendFactors);
    }
    else {
      if (!blendParser(blendFactors, blendSrc, blendDst)) {
	lua_pushboolean(L, false);
	lua_rawset(L, -3);
      }
      else {
	lua_createtable(L, 0, 2); {
	  luaset_strint(L, "src", blendSrc);
	  luaset_strint(L, "dst", blendDst);
	  luaset_strstr(L, "text", blendFactors);
	}
	lua_rawset(L, -3);
      }
    }

    // polygon offset
    lua_pushliteral(L, "polygonOffset");
    float poFactor, poUnits;
    if (!mat->getPolygonOffset(poFactor, poUnits)) {
      lua_pushboolean(L, false);
      lua_rawset(L, -3);
    }
    else {
      lua_createtable(L, 0, 2); {
	luaset_strnum(L, "factor", poFactor);
	luaset_strnum(L, "units",  poUnits);
      }
      lua_rawset(L, -3);
    }

    // textures
    const int texCount = mat->getTextureCount();
    lua_pushliteral(L, "textures");
    lua_newtable(L); {
      for (int texNum = 0; texNum < texCount; texNum++) {
	lua_pushinteger(L, texNum + 1);
	lua_newtable(L); {
	  luaset_strstr(L,  "name",      mat->getTexture(texNum));
	  luaset_strstr(L,  "localName", mat->getTextureLocal(texNum));
	  PushIntOrFalse(L, "matrix",    mat->getTextureMatrix(texNum));
	  luaset_strint(L,  "envMode",   mat->getCombineMode(texNum));
	  luaset_strbool(L, "useAlpha",  mat->getUseTextureAlpha(texNum));
	  luaset_strbool(L, "useColor",  mat->getUseColorOnTexture(texNum));
	  luaset_strbool(L, "sphereMap", mat->getUseSphereMap(texNum));

	  const fvec2& autoScale = mat->getTextureAutoScale(texNum);
	  lua_pushliteral(L, "autoScale");
	  lua_createtable(L, 2, 0); {
	    lua_pushfloat(L, autoScale.x); lua_rawseti(L, -2, 1);
	    lua_pushfloat(L, autoScale.y); lua_rawseti(L, -2, 2);
	  }
	  lua_rawset(L, -3);
	}
	lua_rawset(L, -3);
      }
    }
    lua_rawset(L, -3);

    // shaders
    const int shaderCount = mat->getShaderCount();
    lua_pushliteral(L, "shaders");
    lua_newtable(L); {
      for (int shaderNum = 0; shaderNum < shaderCount; shaderNum++) {
	lua_pushstdstring(L, mat->getShader(shaderNum));
	lua_rawseti(L, -2, shaderNum + 1);
      }
    }
    lua_rawset(L, -3);
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
