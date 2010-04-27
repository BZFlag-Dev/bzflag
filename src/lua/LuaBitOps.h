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

#ifndef LUA_BIT_OPS_H
#define LUA_BIT_OPS_H

struct lua_State;

class LuaBitOps {
  public:
    static bool PushEntries(lua_State* L);

  private:
    static int bit_test(lua_State* L);
    static int bit_or(lua_State* L);
    static int bit_and(lua_State* L);
    static int bit_xor(lua_State* L);
    static int bit_inv(lua_State* L);
    static int bit_bits(lua_State* L);
};


#endif // LUA_BIT_OPS_H


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
