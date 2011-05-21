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
#include "LuaTexMat.h"

// system headers
#include <string>
using std::string;

// common headers
#include "TextureMatrix.h"

// local headers
#include "LuaHeader.h"


//============================================================================//
//============================================================================//

bool LuaTexMat::PushEntries(lua_State* L) {
  PUSH_LUA_CFUNC(L, GetTexMatID);
  PUSH_LUA_CFUNC(L, GetTexMatName);
  PUSH_LUA_CFUNC(L, GetTexMat);
  PUSH_LUA_CFUNC(L, SetTexMat);

  return true;
}


//============================================================================//
//============================================================================//

static inline const TextureMatrix* ParseTexMat(lua_State* L, int index) {
  const int texmatIndex = luaL_checkint(L, index);
  return TEXMATRIXMGR.getMatrix(texmatIndex);
}


//============================================================================//
//============================================================================//

int LuaTexMat::GetTexMatID(lua_State* L) {
  const std::string name = luaL_checkstring(L, 1);
  const int texmatID = TEXMATRIXMGR.findMatrix(name);
  if (texmatID < 0) {
    lua_pushboolean(L, false);
  }
  else {
    lua_pushint(L, texmatID);
  }
  return 1;
}


int LuaTexMat::GetTexMatName(lua_State* L) {
  const TextureMatrix* texmat = ParseTexMat(L, 1);
  if (texmat == NULL) {
    return luaL_pushnil(L);
  }
  lua_pushstdstring(L, texmat->getName());
  return 1;
}


int LuaTexMat::GetTexMat(lua_State* L) {
  const TextureMatrix* texmat = ParseTexMat(L, 1);
  if (texmat == NULL) {
    return luaL_pushnil(L);
  }
  const float* matrix = texmat->getMatrix();
  for (int i = 0; i < 16; i++) {
    lua_pushfloat(L, matrix[i]);
  }
  return 16;
}


int LuaTexMat::SetTexMat(lua_State* L) {
  TextureMatrix* texmat = const_cast<TextureMatrix*>(ParseTexMat(L, 1));
  if (texmat == NULL) {
    return luaL_pushnil(L);
  }
  const float* oldMatrix = texmat->getMatrix();
  float newMatrix[4][4];
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 4; j++) {
      const int k = (i * 4) + j;
      newMatrix[i][j] = luaL_optfloat(L, k + 2, oldMatrix[k]);
    }
  }
  texmat->setMatrix(newMatrix);
  lua_pushboolean(L, true);
  return 1;
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
