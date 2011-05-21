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

#ifndef LUA_TEXTURES_H
#define LUA_TEXTURES_H

#include "common.h"

// system headers
#include <string>

// common headers
#include "bzfgl.h"

// local headers
#include "LuaOpenGL.h"


struct lua_State;


static const int MAX_LUA_TEXTURE_UNITS = 32;


//============================================================================//

class LuaTexture {
  public:
    LuaTexture();
    virtual ~LuaTexture();

    virtual GLuint GetTexID() const = 0;

    virtual bool Delete() = 0;

    virtual bool Bind() const = 0;
    virtual bool IsValid() const = 0;
    virtual bool IsWritable() const = 0;

    virtual GLenum GetTarget() const = 0;
    virtual GLenum GetFormat() const = 0;

    virtual GLsizei GetSizeX() const = 0;
    virtual GLsizei GetSizeY() const = 0;
    virtual GLsizei GetSizeZ() const = 0;

    virtual GLint GetBorder() const = 0;

    virtual GLenum GetMinFilter() const = 0;
    virtual GLenum GetMagFilter() const = 0;

    virtual GLenum GetWrapS() const = 0;
    virtual GLenum GetWrapT() const = 0;
    virtual GLenum GetWrapR() const = 0;

    virtual GLfloat GetAniso() const = 0;
};


//============================================================================//

class LuaTextureMgr {
  public:
    static bool PushEntries(lua_State* L);

    static const LuaTexture* TestLuaTexture(lua_State* L, int index);
    static const LuaTexture* CheckLuaTexture(lua_State* L, int index);

    static GLenum GetActiveTexture() { return activeTexture; }
    static GLvoid SetActiveTexture(GLenum t) { activeTexture = t; }

    // returns -1 for unknown combinations
    static int GetPixelSize(GLenum format, GLenum type);

  public:
    static const char* metaName;

  private:
    static bool CreateMetatable(lua_State* L);
    static int MetaGC(lua_State* L);
    static int MetaIndex(lua_State* L);

    static LuaTexture* GetLuaTexture(lua_State* L, int index);

  private: // call-outs
    static int RefTexture(lua_State* L);
    static int CreateTexture(lua_State* L);
    static int DeleteTexture(lua_State* L);

    static int Texture(lua_State* L);
    static int TexEnv(lua_State* L);
    static int TexGen(lua_State* L);
    static int TexParameter(lua_State* L);

    static int ActiveTexture(lua_State* L);
    static int MultiTexture(lua_State* L);
    static int MultiTexEnv(lua_State* L);
    static int MultiTexGen(lua_State* L);

    static int TexSubImage(lua_State* L);

    static int CopyToTexture(lua_State* L);

    static int GenerateMipMap(lua_State* L);

    static int TexBuffer(lua_State* L);

  private:
    static bool ParseTexture(lua_State* L, int index);

  private:
    static GLenum activeTexture;
};


extern LuaTextureMgr luaTextureMgr;


//============================================================================//

#endif // LUA_TEXTURES_H


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=2 tabstop=8 expandtab
