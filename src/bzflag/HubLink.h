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

#ifndef __HUB_LINK_H__
#define __HUB_LINK_H__


#include "common.h"

// system headers
#include <string>
#include <list>
#include <set>


struct lua_State;

class AresHandler;


class HubLink {
  public:
    enum LinkState {
      StateInit,
      StateDNS,
      StateConnect,
      StateGetCode,
      StateReady,
      StateFailed
    };

  public:
    static const std::string codeFileName;

  public:
    HubLink(const std::string& hostPort,
            const std::string& luaCode = "");
    ~HubLink();

    bool update();

    LinkState getState() const { return state; }
    const char* getStateString() const;

    const std::string& getHostPort() const { return hostPort; }
    const std::string& getLuaCode()  const { return luaCode;  }

    bool getWantDisable() const { return wantDisable; }
    const std::string& getReloadHostPort() const { return reloadHostPort; }
    const std::string& getReloadLuaCode()  const { return reloadLuaCode;  }

    // external call-ins
    void recvCommand(const std::string& cmd);

    void serverJoined(const std::string& location, int port,
                      const std::string& callsign);
    void serverParted();

    void wordComplete(const std::string& line, std::set<std::string>& matches);

    void tabAdded(const std::string& name);
    void tabRemoved(const std::string& name);
    void activeTabChanged();

    void bzdbChange(const std::string& varName);

    void startComposing();

  public:
    static void debugf(int level, const char* fmt, ...);

  protected:
    // internal call-ins
    void shutdown();
    void updateLua();
    bool recvData(const std::string& data);

  protected:
    void clear();

    void fail(const std::string& failMsg);
    void fail(const std::string& failMsg, int errnum);

    bool parseHostPort(std::string& host, int& port);

    std::string getLuaCodeFilename() const;
    bool loadFile(const std::string& path, std::string& data) const;
    bool saveFile(const std::string& path, const std::string& data) const;
        
    void stateInit();
    void stateDNS();
    void stateConnect();
    void stateGetCode();
    void stateReady();

    bool updateSend();
    bool updateRecv(bool useBuffer);
    bool sendData(const std::string& data);
    bool combineRecv(size_t minSize);
    bool readData(int bytes, std::string& data);
    bool peekData(int bytes, std::string& data);
    bool sendChunk(const std::string& chunk); 
    bool readChunk(std::string& chunk); 

  protected: // lua routines
    bool createLua(const std::string& code);
    bool pushAnsiCodes();
    bool pushCallOuts();
    bool pushConstants();

    bool pushCallIn(int ciCode, int inArgs);
    bool runCallIn(int inArgs, int outArgs);

  private: // lua call-outs
    static int GetCallIn(lua_State* L);
    static int SetCallIn(lua_State* L);

    static int Reload(lua_State* L);
    static int Disable(lua_State* L);

    static int SendData(lua_State* L);

    static int Print(lua_State* L);
    static int Alert(lua_State* L);
    static int PlaySound(lua_State* L);

    static int AddTab(lua_State* L);
    static int RemoveTab(lua_State* L);
    static int ShiftTab(lua_State* L);
    static int ClearTab(lua_State* L);
    static int RenameTab(lua_State* L);
    static int GetTabCount(lua_State* L);
    static int GetTabIndex(lua_State* L);
    static int GetTabLabel(lua_State* L);
    static int GetActiveTab(lua_State* L);
    static int SetActiveTab(lua_State* L);

    static int GetComposePrompt(lua_State* L);
    static int SetComposePrompt(lua_State* L);
    static int GetComposeString(lua_State* L);
    static int SetComposeString(lua_State* L);

    static int CalcMD5(lua_State* L);
    static int StripAnsiCodes(lua_State* L);

    static int SetBZDB(lua_State* L);
    static int GetBZDB(lua_State* L);

    static int LoadFile(lua_State* L);
    static int SaveFile(lua_State* L);
    static int AppendFile(lua_State* L);
    static int RemoveFile(lua_State* L);

    static int GetCode(lua_State* L);
    static int GetTime(lua_State* L);
    static int GetVersion(lua_State* L);
    static int GetHubServer(lua_State* L);
    static int GetServerInfo(lua_State* L);
    static int GetOpenGLString(lua_State* L);
    static int GetOpenGLNumbers(lua_State* L);

    static int GetKeyBindings(lua_State* L);

    static int IsVisible(lua_State* L);

    static int PackInt8(lua_State* L);
    static int PackInt16(lua_State* L);
    static int PackInt32(lua_State* L);
    static int PackInt64(lua_State* L);
    static int PackUInt8(lua_State* L);
    static int PackUInt16(lua_State* L);
    static int PackUInt32(lua_State* L);
    static int PackUInt64(lua_State* L);
    static int PackFloat(lua_State* L);
    static int PackDouble(lua_State* L);
    static int UnpackInt8(lua_State* L);
    static int UnpackInt16(lua_State* L);
    static int UnpackInt32(lua_State* L);
    static int UnpackInt64(lua_State* L);
    static int UnpackUInt8(lua_State* L);
    static int UnpackUInt16(lua_State* L);
    static int UnpackUInt32(lua_State* L);
    static int UnpackUInt64(lua_State* L);
    static int UnpackFloat(lua_State* L);
    static int UnpackDouble(lua_State* L);

  protected:
    const std::string hostPort; // host[:port] address format

    LinkState state;

    AresHandler* ares;

    lua_State* L;

    int sock;

    std::list<std::string> recvQueue;
    std::list<std::string> sendQueue;
    size_t recvTotal;
    size_t sendTotal;

    std::string failMsg;

    std::string luaCode;

    std::set<std::string> tabs;

    bool wantDisable;
    std::string reloadHostPort;
    std::string reloadLuaCode;
};


extern HubLink* hubLink;


#endif /* __HUB_LINK_H__ */


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
