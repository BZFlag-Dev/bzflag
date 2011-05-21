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

// loops.h : Defines the entry point for the DLL application.
//

#ifndef _LOOPS_H_
#define _LOOPS_H_

#include "bzfsAPI.h"
#include "plugin_utils.h"
#include "plugin_HTTP.h"
#include "commonItems.h"
#include <vector>

void initLoops(Templateiser& ts);
void freeLoops(void);

class LoopHandler : public TemplateCallbackClass {
  public:
    LoopHandler();
    virtual ~LoopHandler() {};
    virtual void keyCallback(std::string& data, const std::string& key);
    virtual bool ifCallback(const std::string& key);

    virtual bool loopCallback(const std::string& key); // calls setSize and increments untill it's done

  protected:
    virtual bool atStart(void);
    virtual bool increment(void);
    virtual bool done(void);

    virtual bool inLoop(void);

    virtual size_t getStart(void) {return 0;}
    virtual size_t getNext(size_t n) {return n + 1;}
    virtual void terminate(void) {return;}

    // versions that ensure that the item is valid
    virtual void getKey(size_t item, std::string& data, const std::string& key) {};
    virtual bool getIF(size_t item, const std::string& key) {return false;}

    virtual void setSize(void) {return;}   // called by the loop callback for each new loop

    size_t pos;
    size_t size;
};

class PlayerLoop : public LoopHandler, NewPageCallback {
  public:
    PlayerLoop(Templateiser& ts);
    ~PlayerLoop();
    virtual void setSize(void);
    virtual void keyCallback(std::string& data, const std::string& key);
    virtual bool ifCallback(const std::string& key);

    virtual void newPage(const std::string& pagename, const HTTPRequest& request);

    int getPlayerID(void);
  protected:
    virtual void getKey(size_t item, std::string& data, const std::string& key);
    virtual bool getIF(size_t item, const std::string& key);

    std::vector<int>  idList;

    int playerID;
};

class NavLoop : public LoopHandler {
  public:
    NavLoop(Templateiser& ts);

    virtual void keyCallback(std::string& data, const std::string& key);
    virtual void setSize(void);

    void computePageList(void);
    std::string currentTemplate;
    std::vector<std::string> pages;

  protected:
    virtual void getKey(size_t item, std::string& data, const std::string& key);
    virtual bool getIF(size_t item, const std::string& key);

};

class VarsLoop : public LoopHandler {
  public:
    VarsLoop(Templateiser& ts);
    virtual void setSize(void);

  protected:
    virtual void getKey(size_t item, std::string& data, const std::string& key);

    std::vector<std::string> keys;
    std::vector<std::string> values;
};

class PermsLoop : public LoopHandler {
  public:
    PermsLoop(Templateiser& ts);

  protected:
    virtual void getKey(size_t item, std::string& data, const std::string& key);

    std::vector<std::string> perms;
};

class ChatLoop : public LoopHandler, bz_EventHandler, NewPageCallback {
  public:
    ChatLoop(Templateiser& ts);
    virtual ~ChatLoop();
    virtual void setSize(void);

    virtual void process(bz_EventData* eventData);
    virtual void newPage(const std::string& pagename, const HTTPRequest& request);

    virtual bool loopCallback(const std::string& key);
    virtual void keyCallback(std::string& data, const std::string& key);
    virtual bool ifCallback(const std::string& key);

  protected:
    virtual void getKey(size_t item, std::string& data, const std::string& key);
    virtual bool getIF(size_t item, const std::string& key);

    virtual size_t getStart(void);

    typedef struct {
      std::string time;
      std::string from;
      std::string to;
      std::string fromTeam;
      std::string message;
      bz_eTeamType  teamType;
    } ChatMessage;

    std::vector<ChatMessage> messages;
    size_t chatLimit;
    size_t formChatLimit;
};

class LogLoop : public LoopHandler, bz_EventHandler, NewPageCallback {
  public:
    LogLoop(Templateiser& ts);
    virtual ~LogLoop();
    virtual void setSize(void);

    virtual void process(bz_EventData* eventData);
    virtual void newPage(const std::string& pagename, const HTTPRequest& request);

    void getLogAsFile(std::string& file);
    void clearLog(void);

  protected:
    virtual void getKey(size_t item, std::string& data, const std::string& key);

    virtual size_t getStart(void);

    typedef struct {
      std::string time;
      std::string message;
    } LogMessage;

    std::vector<LogMessage> messages;
    size_t displayLimit;

  private:
    void logChatMessage(bz_ChatEventData_V1* data, LogMessage& message);
    void logJoinPartMessage(bz_PlayerJoinPartEventData_V1* data, LogMessage& message, bool join);
    void logSpawnMessage(bz_PlayerSpawnEventData_V1* data, LogMessage& message);
    void logDieMessage(bz_PlayerDieEventData_V1* data, LogMessage& message);

    void logGetWorldMessage(bz_GetWorldEventData_V1* data, LogMessage& message);
    void logWorldDoneMessage(LogMessage& message);

    void logBanMessage(bz_BanEventData_V1* data, LogMessage& message);
    void logHostBanMessage(bz_HostBanEventData_V1* data, LogMessage& message);
    void logIDBanMessage(bz_IdBanEventData_V1* data, LogMessage& message);
    void logKickMessage(bz_KickEventData_V1* data, LogMessage& message);
    void logKillMessage(bz_KillEventData_V1* data, LogMessage& message);
    void logPausedMessage(bz_PlayerPausedEventData_V1* data, LogMessage& message);
    void logGameStartEndMessage(bz_GameStartEndEventData_V1* data, LogMessage& message, bool start);
    void logSlashMessage(bz_SlashCommandEventData_V1* data, LogMessage& message);
    void logAuthMessage(bz_PlayerAuthEventData_V1* data, LogMessage& message);
    void logReportMessage(bz_ReportFiledEventData_V1* data, LogMessage& message);
};

class IPBanLoop : public LoopHandler, NewPageCallback {
  public:
    IPBanLoop(Templateiser& ts);
    ~IPBanLoop(void);
    virtual void setSize(void);
    void newPage(const std::string& pagename, const HTTPRequest& request);

  protected:
    virtual void getKey(size_t item, std::string& data, const std::string& key);
    virtual bool getIF(size_t item, const std::string& key);

    virtual size_t getNext(size_t n);

    bool filterMasterBans;
};

class HostBanLoop : public LoopHandler {
  public:
    HostBanLoop(Templateiser& ts);
    virtual void setSize(void);

  protected:
    virtual void getKey(size_t item, std::string& data, const std::string& key);
    virtual bool getIF(size_t item, const std::string& key);
};

class IDBanLoop : public LoopHandler {
  public:
    IDBanLoop(Templateiser& ts);
    virtual void setSize(void);

  protected:
    virtual void getKey(size_t item, std::string& data, const std::string& key);
    virtual bool getIF(size_t item, const std::string& key);
};

class PlayerGroupLoop : public LoopHandler {
  public:
    PlayerGroupLoop(Templateiser& ts);
    virtual void setSize(void);

  protected:
    virtual void getKey(size_t item, std::string& data, const std::string& key);

    std::vector<std::string> groups;
};

class FlagHistoryLoop : public LoopHandler {
  public:
    FlagHistoryLoop(Templateiser& ts);
    virtual void setSize(void);

  protected:
    virtual void getKey(size_t item, std::string& data, const std::string& key);

    std::vector<std::string> history;
};

class ReportsLoop : public LoopHandler {
  public:
    ReportsLoop(Templateiser& ts);
    virtual void setSize(void);

  protected:
    virtual void getKey(size_t item, std::string& data, const std::string& key);
};




extern NavLoop* navLoop;
extern LogLoop* logLoop;


#endif //_PAGES_H_

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=2 tabstop=8
