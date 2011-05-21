////////////////////////////////////////////////////////////////////////////////
//
// EventHandler.h: interface for the EventHandler class.
//
////////////////////////////////////////////////////////////////////////////////

#ifndef EVENT_HANDLER_H
#define EVENT_HANDLER_H

#include "common.h"

// system headers
#include <string>
#include <vector>
#include <set>
#include <map>

// common headers
#include "EventClient.h"
#include "EventClientList.h"


class Player;
class ShotPath;
class Flag;
class FlagType;
struct FiringInfo;


class EventHandler {
  public:
    enum LoopType {
      Basic,
      Special,
      FirstTrue,
      FirstFalse,
      FirstNumber,
      FirstString,
      BooleanOR
    };

  public:
    enum KeyModifiers {
      AltKeyModBit    = (1 << 0),
      CtrlKeyModBit   = (1 << 1),
      ShiftKeyModBit  = (1 << 2),
      RepeatKeyModBit = (1 << 7)
    };

  public:
    EventHandler();
    ~EventHandler();

    void Purify();

    void AddClient(EventClient* ec);
    void RemoveClient(EventClient* ec);

    bool InsertEvent(EventClient* ec, const std::string& ciName);
    bool RemoveEvent(EventClient* ec, const std::string& ciName);

    void GetEventList(std::vector<std::string>& ecList) const;

    class EventInfo;
    const EventInfo* GetEventInfo(const std::string& eventName) const;

    // event properties
    bool IsKnown(const std::string& eventName) const;
    bool IsManaged(const std::string& eventName) const;
    bool IsReversed(const std::string& eventName) const;
    bool IsReentrant(const std::string& eventName) const;
    bool ReqFullRead(const std::string& eventName) const;
    bool ReqGameCtrl(const std::string& eventName) const;
    bool ReqInputCtrl(const std::string& eventName) const;

    LoopType    GetLoopType(const std::string& eventName) const;
    const char* GetLoopTypeName(LoopType type) const;

    /**************************************************************************/

    void Update();

    void BZDBChange(const std::string& name);

    bool CommandFallback(const std::string& cmd);

    void RecvChatMsg(const std::string& msg, int srcID, int dstID, bool action);
    void RecvLuaData(int srcPlayerID, int srcScriptID,
                     int dstPlayerID, int dstScriptID,
                     int status, const std::string& data);

    void ServerJoined();
    void ServerParted();

    // FIXME -- potential call-ins
    // void TeamScoreChange(int teamID);
    // void FlagUpdate(const Flag&);
    // void FlagNearMsg(const Flag&);
    // void MissileUpdate(const FiringInfo&);
    // void PlayerShotType(const FiringInfo&);
    // void PlayerNewRabbit();
    // void PlayerCollision(const Player& p1, const Player& p2);
    // void GameStart();
    // void GamePaused();
    // void GameEnd();
    // bool NetMsgRecv(const char* msg, int len);
    // bool NetMsgSend(const char* msg, int len);
    // bool AllowSound(const std::string& soundName);

    void PlayerAdded(const Player&);
    void PlayerRemoved(const Player&);
    void PlayerSpawned(const Player&);
    void PlayerKilled(const Player& victim, const Player* killer,
                      int reason, const FlagType* flagType, int phyDrv);
    void PlayerJumped(const Player&);
    void PlayerLanded(const Player&, float vel);
    void PlayerTeleported(const Player&, int srcLink, int dstLink);
    void PlayerTeamChange(const Player&, int oldTeam);
    void PlayerScoreChange(const Player&);

    void FlagAdded(const Flag&);
    void FlagRemoved(const Flag&);
    void FlagGrabbed(const Flag&, const Player&);
    void FlagDropped(const Flag&, const Player&);
    void FlagCaptured(const Flag&, const Player*);
    void FlagTransferred(const Flag&, const Player& src, const Player& dst);

    void ShotAdded(const FiringInfo&);
    void ShotRemoved(const FiringInfo&);
    void ShotRicochet(const ShotPath&, const float* pos, const float* normal);
    void ShotTeleported(const ShotPath&, int srcLink, int dstLink);

    void GLResize();
    void GLContextInit();
    void GLContextFree();
    void GLUnmapped();

    void DrawGenesis();
    void DrawWorldStart();
    void DrawWorld();
    void DrawWorldAlpha();
    void DrawWorldShadow();
    void DrawScreenStart();
    void DrawScreen();
    void DrawRadar();

    bool KeyPress(bool taken, int key, int mods);
    bool KeyRelease(bool taken, int key, int mods);
    bool UnicodeText(bool taken, uint32_t unicode);
    bool MouseMove(bool taken, int x, int y);
    bool MousePress(bool taken, int x, int y, int button);
    bool MouseRelease(bool taken, int x, int y, int button);
    bool MouseWheel(bool taken, float value); // positive is up
    bool IsAbove(int x, int y);
    std::string GetTooltip(int x, int y);

    void WordComplete(const std::string& line, std::set<std::string>& partials);

    bool ForbidSpawn();
    bool ForbidJump();
    bool ForbidFlagDrop();
    bool ForbidShot();
    bool ForbidShotLock(const Player&);
    bool ForbidShotHit(const Player&, const ShotPath&, const fvec3& pos);

    /**************************************************************************/

  public:
    class EventInfo {

        friend class EventHandler;

      public:
        EventInfo()
          : reqFullRead(false)
          , reqGameCtrl(false)
          , reqInputCtrl(false)
          , reversed(false)
          , reentrant(false)
          , ecList(NULL)
        {}

        EventInfo(const std::string& _name, EventClientList* _ecList,
                  bool _reqFullRead, bool _reqGameCtrl, bool _reqInputCtrl,
                  bool _reversed, bool _reentrant, LoopType _loopType)
          : name(_name)
          , reqFullRead(_reqFullRead)
          , reqGameCtrl(_reqGameCtrl)
          , reqInputCtrl(_reqInputCtrl)
          , reversed(_reversed)
          , reentrant(_reentrant)
          , loopType(_loopType)
          , ecList(_ecList)
        {}

        ~EventInfo() {}

        inline const std::string& GetName() const { return name; }
        inline bool ReqFullRead()  const { return reqFullRead;      }
        inline bool ReqGameCtrl()  const { return reqGameCtrl;      }
        inline bool ReqInputCtrl() const { return reqInputCtrl;     }
        inline bool IsManaged()    const { return (ecList != NULL); }
        inline bool IsReversed()   const { return reversed;         }
        inline bool IsReentrant()  const { return reentrant;        }

      protected:
        inline EventClientList* GetList() const { return ecList; }

      private:
        std::string name;
        bool reqFullRead;
        bool reqGameCtrl;
        bool reqInputCtrl;
        bool reversed;
        bool reentrant;
        LoopType loopType;
        EventClientList* ecList;
    };

  private:
    typedef std::map<std::string, EventInfo> EventMap;

  private:
    void SetupEvent(const std::string& ciName, EventClientList* ecList,
                    int orderType, LoopType loopType, int propertyBits);
    bool CanUseEvent(EventClient* ec, const EventInfo& eInfo) const;

  private:
    EventMap eventMap;

    std::set<EventClientList*> dirtyLists;

    std::set<EventClient*> clientSet;
    EventClientList        clientList;

    EventClientList listUpdate;

    EventClientList listBZDBChange;

    EventClientList listCommandFallback;

    EventClientList listRecvChatMsg;
    EventClientList listRecvLuaData;

    EventClientList listServerJoined;
    EventClientList listServerParted;

    EventClientList listForbidSpawn;
    EventClientList listForbidJump;
    EventClientList listForbidFlagDrop;
    EventClientList listForbidShot;
    EventClientList listForbidShotLock;
    EventClientList listForbidShotHit;

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
    EventClientList listShotRicochet;
    EventClientList listShotTeleported;

    EventClientList listFlagAdded;
    EventClientList listFlagRemoved;
    EventClientList listFlagGrabbed;
    EventClientList listFlagDropped;
    EventClientList listFlagCaptured;
    EventClientList listFlagTransferred;

    EventClientList listGLResize;
    EventClientList listGLContextInit;
    EventClientList listGLContextFree;
    EventClientList listGLUnmapped;

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
    EventClientList listUnicodeText;
    EventClientList listMouseMove;
    EventClientList listMousePress;
    EventClientList listMouseRelease;
    EventClientList listMouseWheel;
    EventClientList listIsAbove;
    EventClientList listGetTooltip;

    EventClientList listWordComplete;
};


extern EventHandler eventHandler;


//============================================================================//
//============================================================================//
//
// Inlined call-in loops, for speed
//

#define EC_LOOP_START(name) \
  EventClientList& ecList = list ## name; \
  if (ecList.empty()) { return; } \
  EventClientList::const_iterator it; \
  for (it = ecList.begin(); it != ecList.end(); ++it) { \
    EventClient* ec = *it;

#define EC_LOOP_END(name) }

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

#define EC_LOOP_4_PARAM(name, t1, t2, t3, t4) \
  inline void EventHandler:: name (t1 p1, t2 p2, t3 p3, t4 p4) \
  { EC_LOOP_START(name) ec-> name (p1, p2, p3, p4); EC_LOOP_END(name) }

#define EC_LOOP_5_PARAM(name, t1, t2, t3, t4, t5) \
  inline void EventHandler:: name (t1 p1, t2 p2, t3 p3, t4 p4, t5 p5) \
  { EC_LOOP_START(name) ec-> name (p1, p2, p3, p4, p5); EC_LOOP_END(name) }


EC_LOOP_0_PARAM(Update)

EC_LOOP_1_PARAM(BZDBChange, const std::string&)

EC_LOOP_4_PARAM(RecvChatMsg, const std::string&, int /*srcID*/, int /*dstID*/, bool /*action*/)

EC_LOOP_0_PARAM(ServerJoined)
EC_LOOP_0_PARAM(ServerParted)

EC_LOOP_1_PARAM(PlayerAdded,       const Player&)
EC_LOOP_1_PARAM(PlayerRemoved,     const Player&)
EC_LOOP_1_PARAM(PlayerSpawned,     const Player&)
EC_LOOP_5_PARAM(PlayerKilled,      const Player&, const Player*, int, const FlagType*, int)
EC_LOOP_1_PARAM(PlayerJumped,      const Player&)
EC_LOOP_2_PARAM(PlayerLanded,      const Player&, float /*vel*/)
EC_LOOP_3_PARAM(PlayerTeleported,  const Player&, int /*srcLink*/, int /*dstLink*/)
EC_LOOP_2_PARAM(PlayerTeamChange,  const Player&, int /*oldTeam*/)
EC_LOOP_1_PARAM(PlayerScoreChange, const Player&)

EC_LOOP_1_PARAM(FlagAdded,       const Flag&)
EC_LOOP_1_PARAM(FlagRemoved,     const Flag&)
EC_LOOP_2_PARAM(FlagGrabbed,     const Flag&, const Player&)
EC_LOOP_2_PARAM(FlagDropped,     const Flag&, const Player&)
EC_LOOP_2_PARAM(FlagCaptured,    const Flag&, const Player*)
EC_LOOP_3_PARAM(FlagTransferred, const Flag&, const Player&, const Player&)

EC_LOOP_1_PARAM(ShotAdded,   const FiringInfo&)
EC_LOOP_1_PARAM(ShotRemoved, const FiringInfo&)
EC_LOOP_3_PARAM(ShotRicochet, const ShotPath&, const float* /*pos*/, const float* /*normal*/)
EC_LOOP_3_PARAM(ShotTeleported, const ShotPath&, int /*srcLink*/, int /*dstLink*/)

EC_LOOP_0_PARAM(GLResize)
EC_LOOP_0_PARAM(GLContextInit)
EC_LOOP_0_PARAM(GLContextFree)
EC_LOOP_0_PARAM(GLUnmapped)


#undef EC_LOOP_5_PARAM
#undef EC_LOOP_4_PARAM
#undef EC_LOOP_3_PARAM
#undef EC_LOOP_2_PARAM
#undef EC_LOOP_1_PARAM
#undef EC_LOOP_0_PARAM
#undef EC_LOOP_END
#undef EC_LOOP_START


#endif /* EVENT_HANDLER_H */
