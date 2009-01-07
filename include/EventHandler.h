////////////////////////////////////////////////////////////////////////////////
//
// EventHandler.h: interface for the EventHandler class.
//
////////////////////////////////////////////////////////////////////////////////

#ifndef EVENT_HANDLER_H
#define EVENT_HANDLER_H

#include "common.h"

#include <string>
#include <vector>
#include <set>
#include <map>

#include "EventClient.h"
#include "EventClientList.h"


class Player;
class Flag;
class FiringInfo;
class ShotPath;


class EventHandler
{
  public:
    EventHandler();
    ~EventHandler();

    void AddClient(EventClient* ec);
    void RemoveClient(EventClient* ec);

    bool InsertEvent(EventClient* ec, const std::string& ciName);
    bool RemoveEvent(EventClient* ec, const std::string& ciName);

    void GetEventList(std::vector<std::string>& list) const;

    class EventInfo;
    const EventInfo* GetEventInfo(const std::string& eventName) const;

    // event properties
    bool IsKnown(const std::string& eventName) const;
    bool IsManaged(const std::string& eventName) const;
    bool IsReversed(const std::string& eventName) const;
    bool ReqFullRead(const std::string& eventName) const;
    bool ReqInputCtrl(const std::string& eventName) const;

    /**************************************************************************/

    void Update();

    void BZDBChange(const std::string& name);

    void RecvChatMsg(const std::string& msg, int srcID, int dstID);
    void RecvLuaData(int srcPlayerID, int srcScriptID,
                     int dstPlayerID, int dstScriptID,
                     int status, const std::string& data);

    void ServerJoined();
    void ServerParted();

    // FIXME -- potential call-ins
    // FIXME void TeamScoreChange(int teamID);
    // FIXME void FlagUpdate(const Flag&);
    // FIXME void FlagNearMsg(const Flag&);
    // FIXME void MissileUpdate(const FiringInfo&);
    // FIXME void PlayerShotType(const FiringInfo&);
    // FIXME void PlayerNewRabbit();
    // FIXME void PlayerCollision(const Player& p1, const Player& p2);
    // FIXME void GameStart();
    // FIXME void GamePaused();
    // FIXME void GameEnd();

    void PlayerAdded(const Player&);
    void PlayerRemoved(const Player&);
    void PlayerSpawned(const Player&);
    void PlayerKilled(const Player&);
    void PlayerJumped(const Player&);
    void PlayerLanded(const Player&);
    void PlayerTeleported(const Player&, int srcLink, int dstLink);
    void PlayerTeamChange(const Player&); // FIXME -- oldTeam ?
    void PlayerScoreChange(const Player&);

    void FlagAdded(const Flag&);
    void FlagRemoved(const Flag&);
    void FlagGrabbed(const Flag&, const Player&);
    void FlagDropped(const Flag&, const Player&);
    void FlagCaptured(const Flag&, const Player&);
    void FlagTransferred(const Flag&, const Player& src, const Player& dst);

    void ShotAdded(const FiringInfo&);
    void ShotRemoved(const FiringInfo&);
    void ShotTeleported(const ShotPath&, int srcLink, int dstLink); // FIXME -- use pos + dirs?

    void ViewResize();
    void GLContextInit();
    void GLContextFree();

    void DrawGenesis();
    void DrawWorldStart();
    void DrawWorld();
    void DrawWorldAlpha();
    void DrawWorldShadow();
    void DrawScreenStart();
    void DrawScreen();
    void DrawRadar();

    // FIXME -- right approach ?
    bool RenderPlayer(const Player&);
    bool RenderPlayerExplosion(const Player&);
    bool RenderShot(const ShotPath&);
    bool RenderShotExplosion(const ShotPath&);
    bool RenderFlag(const Flag&);
    
    bool KeyPress(int key, bool isRepeat);
    bool KeyRelease(int key);
    bool MouseMove(int x, int y);
    bool MousePress(int x, int y, int button);
    bool MouseRelease(int x, int y, int button);
    bool MouseWheel(float value); // positive is up
    bool IsAbove(int x, int y);
    std::string GetTooltip(int x, int y);

    void WordComplete(const std::string& line,
                      std::set<std::string>& partials);

    /**************************************************************************/

  private:
    typedef std::map<std::string, EventInfo> EventMap;

    struct LessThanEventClient {
      bool operator()(const EventClient* a, const EventClient* b) const {
        return a < b;
      };
    };

  public:
    class EventInfo {

      friend class EventHandler;

      public:
        EventInfo()
        : reqFullRead (false)
        , reqInputCtrl(false)
        , reversed    (false)
        , list        (NULL)
        {}

        EventInfo(const std::string& _name, EventClientList* _list,
                  bool _reqFullRead, bool _reqInputCtrl, bool _reversed)
        : name        (_name)
        , reqFullRead (_reqFullRead)
        , reqInputCtrl(_reqInputCtrl)
        , reversed    (_reversed)
        , list        (_list)
        {}

        ~EventInfo() {}

        inline const std::string& GetName() const { return name; }
        inline bool ReqFullRead()  const { return reqFullRead;    }
        inline bool ReqInputCtrl() const { return reqInputCtrl;   }
        inline bool IsManaged()    const { return (list != NULL); }
        inline bool IsReversed()   const { return reversed;       }

      protected:        
        inline EventClientList* GetList() const { return list; }

      private:
        std::string name;
        bool reqFullRead;
        bool reqInputCtrl;
        bool reversed;
        EventClientList* list;
    };

  private:
    void SetupEvent(const std::string& ciName, EventClientList* list,
                    int propertyBits = 0, bool reversed = false);
    bool CanUseEvent(EventClient* ec, const EventInfo& eInfo) const;

  private:
    EventMap eventMap;
    EventClient* mouseOwner;

  private:
    EventClientList clients;

    EventClientList listUpdate;

    EventClientList listBZDBChange;

    EventClientList listRecvChatMsg;
    EventClientList listRecvLuaData;

    EventClientList listServerJoined;
    EventClientList listServerParted;

    EventClientList listPlayerAdded;
    EventClientList listPlayerRemoved;
    EventClientList listPlayerSpawned;
    EventClientList listPlayerKilled;
    EventClientList listPlayerJumped;
    EventClientList listPlayerLanded;
    EventClientList listPlayerTeleported;
    EventClientList listPlayerTeamChange;
    EventClientList listPlayerScoreChange;

    EventClientList listShotAdded;
    EventClientList listShotRemoved;
    EventClientList listShotTeleported;

    EventClientList listFlagAdded;
    EventClientList listFlagRemoved;
    EventClientList listFlagGrabbed;
    EventClientList listFlagDropped;
    EventClientList listFlagCaptured;
    EventClientList listFlagTransferred;

    EventClientList listViewResize;
    EventClientList listGLContextInit;
    EventClientList listGLContextFree;

    EventClientList listDrawGenesis;
    EventClientList listDrawWorldStart;
    EventClientList listDrawWorld;
    EventClientList listDrawWorldAlpha;
    EventClientList listDrawWorldShadow;
    EventClientList listDrawScreenStart;
    EventClientList listDrawScreen;
    EventClientList listDrawRadar;

    EventClientList listRenderPlayer;
    EventClientList listRenderPlayerExplosion;
    EventClientList listRenderShot;
    EventClientList listRenderShotExplosion;
    EventClientList listRenderFlag;

    EventClientList listKeyPress;
    EventClientList listKeyRelease;
    EventClientList listMouseMove;
    EventClientList listMousePress;
    EventClientList listMouseRelease;
    EventClientList listMouseWheel;
    EventClientList listIsAbove;
    EventClientList listGetTooltip;

    EventClientList listWordComplete;
};


extern EventHandler eventHandler;


/******************************************************************************/
/******************************************************************************/
//
// Inlined call-in loops, for speed
//

#define EC_LOOP_START(name)                \
  EventClientList& list = list ## name; \
  if (list.empty()) { return; }         \
  EventClient* ec;                      \
  size_t i = 0;                         \
  list.start(i);                        \
  while (list.next(i, ec)) {            \

#define EC_LOOP_END(name) \
  }                    \
  list.finish();


#define EC_LOOP_0_PARAM(name) \
  inline void EventHandler:: name () \
  { EC_LOOP_START(name) ec-> name (); EC_LOOP_END(name) }

#define EC_LOOP_1_PARAM(name, t1) \
  inline void EventHandler:: name (t1 p1) \
  { EC_LOOP_START(name) ec-> name (p1); EC_LOOP_END(name) }

#define EC_LOOP_2_PARAM(name, t1, t2) \
  inline void EventHandler:: name (t1 p1, t2 p2) \
  { EC_LOOP_START(name) ec-> name (p1, p2); EC_LOOP_END(name) }

#define EC_LOOP_3_PARAM(name, t1, t2, t3) \
  inline void EventHandler:: name (t1 p1, t2 p2, t3 p3) \
  { EC_LOOP_START(name) ec-> name (p1, p2, p3); EC_LOOP_END(name) }

#define EC_LOOPBOOL_0_PARAM(name) \
  inline void EventHandler:: name () { \
    EC_LOOP_START(name) \
    if (ec-> name ()) { list.finish(i); return true; } \
    EC_LOOP_END(name) \
  }


EC_LOOP_0_PARAM(Update);

EC_LOOP_1_PARAM(BZDBChange, const std::string&);

EC_LOOP_3_PARAM(RecvChatMsg, const std::string&, int /*srcID*/, int /*dstID*/);

EC_LOOP_0_PARAM(ServerJoined);
EC_LOOP_0_PARAM(ServerParted);

EC_LOOP_1_PARAM(PlayerAdded,       const Player&);
EC_LOOP_1_PARAM(PlayerRemoved,     const Player&);
EC_LOOP_1_PARAM(PlayerSpawned,     const Player&);
EC_LOOP_1_PARAM(PlayerKilled,      const Player&);
EC_LOOP_1_PARAM(PlayerJumped,      const Player&);
EC_LOOP_1_PARAM(PlayerLanded,      const Player&);
EC_LOOP_3_PARAM(PlayerTeleported,  const Player&, int /*srcLink*/, int /*dstLink*/);
EC_LOOP_1_PARAM(PlayerTeamChange,  const Player&);
EC_LOOP_1_PARAM(PlayerScoreChange, const Player&);

EC_LOOP_1_PARAM(FlagAdded,       const Flag&);
EC_LOOP_1_PARAM(FlagRemoved,     const Flag&);
EC_LOOP_2_PARAM(FlagGrabbed,     const Flag&, const Player&);
EC_LOOP_2_PARAM(FlagDropped,     const Flag&, const Player&);
EC_LOOP_2_PARAM(FlagCaptured,    const Flag&, const Player&);
EC_LOOP_3_PARAM(FlagTransferred, const Flag&, const Player&, const Player&);

EC_LOOP_1_PARAM(ShotAdded,   const FiringInfo&);
EC_LOOP_1_PARAM(ShotRemoved, const FiringInfo&);
EC_LOOP_3_PARAM(ShotTeleported, const ShotPath&, int /*srcLink*/, int /*dstLink*/);

EC_LOOP_0_PARAM(ViewResize);
EC_LOOP_0_PARAM(GLContextInit);
EC_LOOP_0_PARAM(GLContextFree);


#undef EC_LOOP_3_PARAM
#undef EC_LOOP_2_PARAM
#undef EC_LOOP_1_PARAM
#undef EC_LOOP_0_PARAM
#undef EC_LOOP_END
#undef EC_LOOP_START


#endif /* EVENT_HANDLER_H */
