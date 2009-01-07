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
class FiringInfo;
class Flag;


class EventClient
{
  public:
    inline const std::string& GetName()    const { return clientName;  }
    inline int                GetOrder()   const { return clientOrder; }
    inline bool HasFullRead()  const { return fullRead;  }
    inline bool HasInputCtrl() const { return inputCtrl; }

    // used by the eventHandler to register
    // call-ins when an EventClient is being added
    virtual bool WantsEvent(const std::string& /*eventName*/) { return false; }

  protected:
    const std::string clientName;
    const int clientOrder;
    bool fullRead;
    bool inputCtrl;

  protected:
    EventClient(const std::string& name, int order,
                bool fullRead, bool inputCtrl);
    virtual ~EventClient();

  public:
    virtual void Update() { return; }

    virtual void BZDBChange(const std::string&) { return; }

    virtual void ServerJoined() { return; }
    virtual void ServerParted() { return; }

    virtual void PlayerAdded  (const Player&) { return; }
    virtual void PlayerRemoved(const Player&) { return; }
    virtual void PlayerSpawned(const Player&) { return; }
    virtual void PlayerKilled (const Player&) { return; }
    virtual void PlayerJumped (const Player&) { return; }
    virtual void PlayerLanded (const Player&) { return; }
    virtual void PlayerTeleported(const Player&, int /*srcLink*/, int /*dstLink*/) { return; }
    virtual void PlayerTeamChange(const Player&) { return; }
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
    virtual void ShotTeleported(const ShotPath&, int /*srcLink*/, int /*dstLink*/) { return; }

    virtual void RecvChatMsg(const std::string& /*msg*/, int /*srcID*/, int /*dstID*/) { return; }
    virtual void RecvLuaData(int /*srcPlayerID*/, int /*srcScriptID*/,
                             int /*dstPlayerID*/, int /*dstScriptID*/,
                             int /*status*/, const std::string& /*data*/) { return; }

    virtual void ViewResize()    { return; }
    virtual void GLContextInit() { return; }
    virtual void GLContextFree() { return; }

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

    virtual bool KeyPress(int /*key*/, bool /*isRepeat*/)      { return false; }
    virtual bool KeyRelease(int /*key*/)                       { return false; }
    virtual bool MouseMove(int /*x*/, int /*y*/)               { return false; }
    virtual bool MousePress(int /*x*/, int /*y*/, int /*b*/)   { return false; }
    virtual bool MouseRelease(int /*x*/, int /*y*/, int /*b*/) { return false; }
    virtual bool MouseWheel(float /*value*/)                   { return false; }
    virtual bool IsAbove(int /*x*/, int /*y*/)                 { return false; }
    virtual std::string GetTooltip(int /*x*/, int /*y*/)       { return "";    }

    virtual void WordComplete(const std::string& /*line*/,
                              std::set<std::string>& /*partials*/) { return; }
};


#endif /* EVENT_CLIENT_H */
