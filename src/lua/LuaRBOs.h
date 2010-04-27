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

#ifndef LUA_RBOS_H
#define LUA_RBOS_H

#include "common.h"

// common headers
#include "bzfgl.h"


struct lua_State;


class LuaRBOData {
  public:
    LuaRBOData()
    : id(0)
    , target(GL_RENDERBUFFER)
    , format(GL_RGBA)
    , xsize(0)
    , ysize(0)
    {}
    virtual ~LuaRBOData() {};

  public:
    GLuint id;
    GLenum target;
    GLenum format;
    GLsizei xsize;
    GLsizei ysize;
};


class LuaRBO : public LuaRBOData {
  public:
    LuaRBO(const LuaRBOData& rboData);
    ~LuaRBO();

    void Delete();

  private:
    void InitContext();
    void FreeContext();
    static void StaticInitContext(void* data);
    static void StaticFreeContext(void* data);
};


class LuaRBOMgr {
  public:
    LuaRBOMgr();
    ~LuaRBOMgr();

    static bool PushEntries(lua_State* L);

    static const LuaRBO* TestLuaRBO(lua_State* L, int index);
    static const LuaRBO* CheckLuaRBO(lua_State* L, int index);

    static const char* metaName;

  private: // metatable methods
    static bool CreateMetatable(lua_State* L);
    static int MetaGC(lua_State* L);
    static int MetaIndex(lua_State* L);
    static int MetaNewindex(lua_State* L);

    static LuaRBO* GetLuaRBO(lua_State* L, int index);

  private:
    static int CreateRBO(lua_State* L);
    static int DeleteRBO(lua_State* L);
};


extern LuaRBOMgr luaRBOMgr;


#endif // LUA_RBOS_H


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
