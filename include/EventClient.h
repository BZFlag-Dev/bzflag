////////////////////////////////////////////////////////////////////////////////
//
// EventClient.h: interface for the EventClient class.
//
////////////////////////////////////////////////////////////////////////////////

#ifndef EVENT_CLIENT_H
#define EVENT_CLIENT_H

#include "common.h"

#include <string>
#include <vector>
#include <set>
#include <map>


class Player;
class ShotPath;
class Flag;
class FlagType;
struct FiringInfo;


class EventClient
{
  public:
    enum OrderType {
      ScriptIDOrder   = 0,
      GameStateOrder  = 1,
      DrawWorldOrder  = 2,
      DrawScreenOrder = 3,
      OrderTypeCount
    };

  public:
    inline const std::string& GetName()  const { return clientName;  }

    inline int GetScriptID() const { return scriptID; }

    inline int GetGameStateOrder()  const { return gameStateOrder;  }
    inline int GetDrawWorldOrder()  const { return drawWorldOrder;  }
    inline int GetDrawScreenOrder() const { return drawScreenOrder; }

    inline int GetOrder(int type) const {
      switch (type) {
        case ScriptIDOrder:   { return scriptID;        }
        case GameStateOrder:  { return gameStateOrder;  }
        case DrawWorldOrder:  { return drawWorldOrder;  }
        case DrawScreenOrder: { return drawScreenOrder; }
        default: {
          return -1;
        }
      }
      return -1;
    }

    inline bool HasFullRead()   const { return fullRead;    }
    inline bool HasGameCtrl()   const { return gameCtrl;    }
    inline bool HasInputCtrl()  const { return inputCtrl;   }


  protected:
    const std::string clientName;

    const int scriptID; // unique identifier (0 is reserved for the server)

    const int gameStateOrder;  // game event update order
    const int drawWorldOrder;  // world  drawing  (drawn in reverse)
    const int drawScreenOrder; // screen drawing  (also used for input events)

    bool fullRead;  // can query all game state
    bool gameCtrl;  // can control game state
    bool inputCtrl; // can control inputs (mouse, keyboard, etc...)

  protected:
    EventClient(const std::string& name, int scriptID,
                int gameStateOrder, int drawWorldOrder, int drawScreenOrder,
                bool fullRead, bool gameCtrl, bool inputCtrl);
    virtual ~EventClient();

  public:
    virtual void Update() { return; }

    virtual void BZDBChange(const std::string&) { return; }

    virtual void ServerJoined() { return; }
    virtual void ServerParted() { return; }

    virtual void PlayerAdded  (const Player&) { return; }
    virtual void PlayerRemoved(const Player&) { return; }
    virtual void PlayerSpawned(const Player&) { return; }
    virtual void PlayerKilled(const Player& /*victim*/, const Player* /*killer*/,
                              int /*reason*/, const FlagType* /*flagType*/,
                              int /*phyDrv*/) { return; }
    virtual void PlayerJumped (const Player&) { return; }
    virtual void PlayerLanded (const Player&, float /*vel*/) { return; }
    virtual void PlayerTeleported(const Player&, int /*srcLink*/, int /*dstLink*/) { return; }
    virtual void PlayerTeamChange(const Player&, int /*oldTeam*/) { return; }
    virtual void PlayerScoreChange(const Player&) { return; }

    virtual void FlagAdded(const Flag&)   { return; }
    virtual void FlagRemoved(const Flag&) { return; }
    virtual void FlagGrabbed(const Flag&,  const Player&) { return; }
    virtual void FlagDropped(const Flag&,  const Player&) { return; }
    virtual void FlagCaptured(const Flag&, const Player&) { return; }
    virtual void FlagTransferred(const Flag&,
                                 const Player& /*src*/,
                                 const Player& /*dst*/) { return; }

    virtual void ShotAdded(const FiringInfo&) { return; }
    virtual void ShotRemoved(const FiringInfo&) { return; }
    virtual void ShotRicochet(const ShotPath&, const float* /*pos*/, const float* /*normal*/) { return; }
    virtual void ShotTeleported(const ShotPath&, int /*srcLink*/, int /*dstLink*/) { return; }

    virtual bool CommandFallback(const std::string& /*cmd*/) { return false; }

    virtual void RecvChatMsg(const std::string& /*msg*/, int /*srcID*/, int /*dstID*/) { return; }
    virtual void RecvLuaData(int /*srcPlayerID*/, int /*srcScriptID*/,
                             int /*dstPlayerID*/, int /*dstScriptID*/,
                             int /*status*/, const std::string& /*data*/) { return; }

    virtual void GLResize()      { return; }
    virtual void GLContextInit() { return; }
    virtual void GLContextFree() { return; }
    virtual void GLUnmapped()    { return; }

    virtual void DrawGenesis()     { return; }
    virtual void DrawWorldStart()  { return; }
    virtual void DrawWorld()       { return; }
    virtual void DrawWorldAlpha()  { return; }
    virtual void DrawWorldShadow() { return; }
    virtual void DrawScreenStart() { return; }
    virtual void DrawScreen()      { return; }
    virtual void DrawRadar()       { return; }

    virtual void GotGfxBlock(int /*type*/, int /*id*/)  { return; }
    virtual void LostGfxBlock(int /*type*/, int /*id*/) { return; }

    virtual bool KeyPress(bool /*taken*/, int /*key*/, bool /*isRepeat*/)      { return false; }
    virtual bool KeyRelease(bool /*taken*/, int /*key*/)                       { return false; }
    virtual bool MouseMove(bool /*taken*/, int /*x*/, int /*y*/)               { return false; }
    virtual bool MousePress(bool /*taken*/, int /*x*/, int /*y*/, int /*b*/)   { return false; }
    virtual bool MouseRelease(bool /*taken*/, int /*x*/, int /*y*/, int /*b*/) { return false; }
    virtual bool MouseWheel(bool /*taken*/, float /*value*/)                   { return false; }
    virtual bool IsAbove(int /*x*/, int /*y*/)           { return false; }
    virtual std::string GetTooltip(int /*x*/, int /*y*/) { return "";    }

    virtual void WordComplete(const std::string& /*line*/,
                              std::set<std::string>& /*partials*/) { return; }

    virtual bool ForbidSpawn()                 { return false; }
    virtual bool ForbidJump()                  { return false; }
    virtual bool ForbidShot()                  { return false; }
    virtual bool ForbidShotLock(const Player&) { return false; }
    virtual bool ForbidFlagDrop()              { return false; }
};


#endif /* EVENT_CLIENT_H */
