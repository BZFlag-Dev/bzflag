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

#ifndef LUA_SERVER_PING_H
#define LUA_SERVER_PING_H

#include "common.h"

#include <set>


struct lua_State;
class ServerPing;


class LuaServerPing {
  public:
    LuaServerPing(lua_State* L, int selfRef, int _funcRef,
                  double address, int port);
    ~LuaServerPing();

    void Update();

    void Cancel();

    int  GetLag()  const { return lag;  }
    bool IsDone()  const { return done; }
    bool IsValid() const { return (serverPing != NULL); }

    double GetAddress() const { return addr; }
    int    GetPort()    const { return port; }

  private:
    ServerPing* serverPing;
    lua_State*  pingL;
    int  funcRef;
    int  selfRef;
    bool  done;
    int  lag;
    double      addr;
    int  port;
};


class LuaServerPingMgr {

    friend class LuaServerPing;

  public:
    static bool PushEntries(lua_State* L);

    static void Update();

  public:
    static const char* metaName;

  private:
    static LuaServerPing* CheckServerPing(lua_State* L, int index);

  private: // metatable methods
    static bool CreateMetatable(lua_State* L);
    static int  MetaGC(lua_State* L);
    static int  MetaToString(lua_State* L);

  private: // call-outs
    static int SendServerPing(lua_State* L);

    static int Cancel(lua_State* L);
    static int IsDone(lua_State* L);
    static int GetLag(lua_State* L);
    static int GetAddress(lua_State* L);

  protected:
    typedef std::set<LuaServerPing*> PingSet;
    static PingSet pings;
};


#endif // LUA_SERVER_PING_H


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=2 tabstop=8 expandtab
