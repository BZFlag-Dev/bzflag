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
      StateGetScript,
      StateReady,
      StateFailed
    };

  public:
    static const int defaultPort = 0x425A; // 'BZ' / 16984

    HubLink(const std::string& hostPort,
            const std::string& luaCode = "");
    ~HubLink();

    bool update();

    LinkState getState() const { return state; }

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

    void activeTabChanged();

    void bzdbChange(const std::string& varName);

    void startComposing();

  protected:
    // internal call-ins
    void shutdown();
    void updateLua();
    bool sendMessage(const std::string& msg);

  private:
    void clear();

    void fail(const std::string& failMsg);
    void fail(const std::string& failMsg, int errnum);

    bool parseHostPort(std::string& host, int& port);

    std::string getLuaCodeFilename() const;
    bool loadLuaCode(std::string& code) const;
    bool saveLuaCode(const std::string& code) const;
        
    void stateInit();
    void stateDNS();
    void stateConnect();
    void stateGetScript();
    void stateReady();

    void recvMessage(const std::string& msg);

    bool updateRecv();
    bool updateSend();
    bool combineRecv(size_t minSize);
    bool nextMessage(std::string& msg);

    bool createLua(const std::string& code);
    bool pushAnsiCodes();
    bool pushCallOuts();
    bool pushConstants();

    bool pushCallIn(const char* funcName, int inArgs);
    bool runCallIn(int inArgs, int outArgs);

  private: // lua call-outs
    static int Reload(lua_State* L);
    static int Disable(lua_State* L);

    static int SendMessage(lua_State* L);

    static int Print(lua_State* L);
    static int SetAlert(lua_State* L);

    static int AddTab(lua_State* L);
    static int RemoveTab(lua_State* L);
    static int ShiftTab(lua_State* L);
    static int ClearTab(lua_State* L);
    static int RenameTab(lua_State* L);
    static int GetTabCount(lua_State* L);
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

    static int GetCode(lua_State* L);
    static int GetTime(lua_State* L);
    static int GetVersion(lua_State* L);
    static int GetHubServer(lua_State* L);
    static int GetServerInfo(lua_State* L);

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

    std::string error;

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
