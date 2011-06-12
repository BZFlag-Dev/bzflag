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

#ifndef LUA_HTTP_H
#define LUA_HTTP_H

#include "common.h"

// system headers
#include <string>

// common headers
#include "common/cURLManager.h"


struct lua_State;
class AccessList;


class LuaHTTP : private cURLManager {
  public:
    LuaHTTP(lua_State* L, const std::string& url);
    ~LuaHTTP();

    // virtuals from cURLManager
    void finalization(char* data, unsigned int length, bool good);

    bool Cancel();

    bool GetActive()  const { return active; }
    bool GetSuccess() const { return success; }

    unsigned int GetLength() const { return theLen; }

    const std::string& GetURL()      const { return url;      }
    const std::string& GetPostData() const { return postData; }
    long               GetHttpCode() const { return httpCode; }

    time_t GetFileTime();
    double GetFileSize();
    double GetFileRemoteSize();
    bool   GetFileData(unsigned int offset, std::string& data) const;

    bool PushFunc(lua_State* L) const;

  private:
    void ClearRefs();

  private:
    lua_State* httpL;
    bool active;
    bool success;

    double fileSize;
    double fileRemoteSize;
    time_t fileTime;
    long   httpCode;

    std::string url;
    std::string postData;
    int funcRef;
    int selfRef;
};


class LuaHTTPMgr {
  public:
    static bool PushEntries(lua_State* L);

    static void SetAccessList(AccessList*);

  public:
    static const char* metaName;

  private: // metatable methods
    static bool CreateMetatable(lua_State* L);
    static int MetaGC(lua_State* L);
    static int MetaToString(lua_State* L);

  public:
    static const LuaHTTP* TestHTTP(lua_State* L, int index);
    static const LuaHTTP* CheckHTTP(lua_State* L, int index);

  private:
    static LuaHTTP* GetURL(lua_State* L, int index);

  private: // call-outs

    static int Fetch(lua_State* L);

    static int Cancel(lua_State* L);
    static int Length(lua_State* L);
    static int Success(lua_State* L);
    static int IsActive(lua_State* L);
    static int GetURL(lua_State* L);
    static int GetPostData(lua_State* L);
    static int GetCallback(lua_State* L);
    static int GetFileSize(lua_State* L);
    static int GetFileRemoteSize(lua_State* L);
    static int GetFileTime(lua_State* L);
    static int GetFileData(lua_State* L);
    static int GetHttpCode(lua_State* L);
    static int TestAccess(lua_State* L);

  private:
    static AccessList* accessList;
};


#endif // LUA_HTTP_H


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=2 tabstop=8 expandtab
