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

#ifndef LUA_HANDLE_H
#define LUA_HANDLE_H

#include "common.h"

// system header files
#include <string>
#include <set>
#include <map>

// common header files
#include "EventClient.h"


struct lua_State;
struct FiringInfo;
class  Flag;
class  Player;
class  ShotPath;


//============================================================================//

class LuaHandle : public EventClient {
  public:
    void CheckStack();
    void SetupValidCallIns();

  public:
    bool         RequestReload()  const { return requestReload;  }
    bool         RequestDisable() const { return requestDisable; }
    const std::string& RequestMessage() const { return requestMessage; }

  public: // call-ins
    virtual bool CanUseCallIn(int code) const;
    virtual bool CanUseCallIn(const std::string& name) const;
    virtual bool UpdateCallIn(const std::string& name, bool state);

    virtual void Shutdown(); // custom to LuaHandle

    virtual void Update();

    virtual void BZDBChange(const std::string&);

    virtual bool CommandFallback(const std::string& cmd);

    virtual bool RecvCommand(const std::string& msg); // custom to LuaHandle
    virtual void RecvChatMsg(const std::string& msg,
                             int srcID, int dstID, bool action);
    virtual void RecvLuaData(int srcPlayerID, int srcScriptID,
                             int dstPlayerID, int dstScriptID,
                             int status, const std::string& data);

    virtual void ServerJoined();
    virtual void ServerParted();

    virtual void PlayerAdded(const Player&);
    virtual void PlayerRemoved(const Player&);
    virtual void PlayerSpawned(const Player&);
    virtual void PlayerKilled(const Player& victim, const Player* killer,
                              int reason, const FlagType* flagType, int phyDrv);
    virtual void PlayerJumped(const Player&);
    virtual void PlayerLanded(const Player&, float vel);
    virtual void PlayerTeleported(const Player&, int srcLink, int dstLink);
    virtual void PlayerTeamChange(const Player&, int oldTeam);
    virtual void PlayerScoreChange(const Player&);

    virtual void ShotAdded(const FiringInfo&);
    virtual void ShotRemoved(const FiringInfo&);
    virtual void ShotRicochet(const ShotPath&, const float* pos, const float* normal);
    virtual void ShotTeleported(const ShotPath&, int srcLink, int dstLink);

    virtual void FlagAdded(const Flag&);
    virtual void FlagRemoved(const Flag&);
    virtual void FlagGrabbed(const Flag&,  const Player&);
    virtual void FlagDropped(const Flag&,  const Player&);
    virtual void FlagCaptured(const Flag&, const Player*);
    virtual void FlagTransferred(const Flag&, const Player& src, const Player& dst);

    virtual void GLResize();
    virtual void GLContextInit();
    virtual void GLContextFree();
    virtual void GLUnmapped();

    virtual void DrawGenesis();
    virtual void DrawWorldStart();
    virtual void DrawWorld();
    virtual void DrawWorldAlpha();
    virtual void DrawWorldShadow();
    virtual void DrawScreenStart();
    virtual void DrawScreen();
    virtual void DrawRadar();

    virtual void GotGfxBlock(int /*type*/, int /*id*/);
    virtual void LostGfxBlock(int /*type*/, int /*id*/);

    virtual bool KeyPress(bool /*taken*/, int /*key*/, int /*mods*/);
    virtual bool KeyRelease(bool /*taken*/, int /*key*/,  int /*mods*/);
    virtual bool UnicodeText(bool /*taken*/, uint32_t /*unicode*/);
    virtual bool MouseMove(bool /*taken*/, int /*x*/, int /*y*/);
    virtual bool MousePress(bool /*taken*/, int /*x*/, int /*y*/, int /*b*/);
    virtual bool MouseRelease(bool /*taken*/, int /*x*/, int /*y*/, int /*button*/);
    virtual bool MouseWheel(bool /*taken*/, float /*value*/);
    virtual bool IsAbove(int /*x*/, int /*y*/);
    virtual std::string GetTooltip(int /*x*/, int /*y*/);

    virtual void WordComplete(const std::string& /*line*/,
                              std::set<std::string>& /*partials*/);

    virtual bool ForbidSpawn();
    virtual bool ForbidJump();
    virtual bool ForbidFlagDrop();
    virtual bool ForbidShot();
    virtual bool ForbidShotLock(const Player&);
    virtual bool ForbidShotHit(const Player&, const ShotPath&, const fvec3&);

  protected:
    LuaHandle(const std::string& name, int16_t scriptID,
              int gameStateOrder, int drawWorldOrder, int drawScreenOrder,
              bool fullRead, bool gameCtrl, bool inputCtrl);
    virtual ~LuaHandle();

    void KillLua();

    bool SetupEnvironment();

    virtual std::string LoadSourceCode(const std::string& sourceFile,
                                       const std::string& sourceModes);
    virtual bool ExecSourceCode(const std::string& sourceCode,
                                const std::string& sourceLabel);

    bool AddBasicCalls();
    bool PushLib(const char* name, bool (*entriesFunc)(lua_State*));

    bool PushCallIn(int ciCode);
    bool RunCallIn(int ciCode, int inArgs, int outArgs);

  protected:
    lua_State* L;

    bool requestReload;
    bool requestDisable;
    std::string requestMessage;

    std::set<int> validCallIns;

  protected: // call-outs
    static int CallAsPlayer(lua_State* L);

    static int ScriptReload(lua_State* L);
    static int ScriptDisable(lua_State* L);

    static int ScriptGetID(lua_State* L);
    static int ScriptGetName(lua_State* L);

    static int ScriptHasFullRead(lua_State* L);
    static int ScriptHasGameCtrl(lua_State* L);
    static int ScriptHasInputCtrl(lua_State* L);

    static int ScriptGetCallInInfo(lua_State* L);
    static int ScriptCanUseCallIn(lua_State* L);
    static int ScriptSetCallIn(lua_State* L);

    static int ScriptGetDevMode(lua_State* L);
    static int ScriptGetGLOBALS(lua_State* L);
    static int ScriptGetCALLINS(lua_State* L);
    static int ScriptGetREGISTRY(lua_State* L);

    static int ScriptPrintPointer(lua_State* L);
    static int ScriptPrintGCInfo(lua_State* L);
    static int ScriptPrintDelayQueues(lua_State* L);

  public: // static
    static void SetDevMode(bool value) { devMode = value; }
    static bool GetDevMode() { return devMode; }

  protected: // static
    static bool devMode;   // allows real file access
};

//============================================================================//


#endif // LUA_HANDLE_H


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=2 tabstop=8 expandtab
