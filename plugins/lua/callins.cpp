
#include "bzfsAPI.h"
#include "plugin_utils.h"

#include "mylua.h"

#include "callins.h"

#include <string.h>
#include <string>
#include <vector>
#include <set>
#include <map>
using std::string;
using std::vector;
using std::set;
using std::map;


static const bz_eEventType bz_eShutdown = (bz_eEventType)(bz_eLastEvent + 1);


/******************************************************************************/
/******************************************************************************/

static lua_State* L = NULL;


/******************************************************************************/
/******************************************************************************/


class CallIn : public bz_EventHandler {

  public:

    CallIn(int _code, const string& _name)
    : code(_code)
    , name(_name)
    , registered(false)
    {
      codeMap[code] = this;
      nameMap[name] = this;
    }

    virtual ~CallIn()
    {
      Unregister();
    }

    virtual bool execute(bz_EventData* eventData) = 0;
    void process(bz_EventData* eventData) { execute(eventData); }

    const string& GetName() const { return name; }
    const int     GetCode() const { return code; }

    bool PushCallIn(int maxSlots)
    {
      if (!lua_checkstack(L, maxSlots)) {
        return false; // FIXME -- add a message for lua_checkstack()?
      }
      if (L == NULL) {
        return false;
      }
      lua_rawgeti(L, LUA_CALLINSINDEX, code);
      if (!lua_isfunction(L, -1)) {
        lua_pop(L, 1);
        return false;
      }
      return true;
    }

    bool RunCallIn(int inArgs, int outArgs)
    {
      if (lua_pcall(L, inArgs, outArgs, 0) != 0) {
        bz_debugMessagef(0, "lua call-in error (%s): %s\n",
                         name.c_str(), lua_tostring(L, -1));
        lua_pop(L, 1);
        return false;
      }
      return true;
    }

    bool Register()
    {
      if (code > bz_eLastEvent) {
        return true; // no need to register
      }
      if (!registered) {
        bz_registerEvent((bz_eEventType)code, this);
        registered = true;
        return true;
      }
      return false;
    }

    bool Unregister()
    {
      if (code > bz_eLastEvent) {
        return true; // no need to register
      }
      if (registered) {
        bz_removeEvent((bz_eEventType)code, this);
        registered = false;
        return true;
      }
      return false;
    }

    bool IsActive() const { return registered; }

  protected:
    const int    code;
    const string name;

    bool registered;

  public:
    static CallIn* Find(const string& name)
    {
      map<string, CallIn*>::iterator it = nameMap.find(name);
      return (it == nameMap.end()) ? NULL : it->second;
    }
    static const map<int,    CallIn*>& GetCodeMap() { return codeMap; }
    static const map<string, CallIn*>& GetNameMap() { return nameMap; }

  private:
    static map<int,    CallIn*> codeMap;
    static map<string, CallIn*> nameMap;
};


map<int,    CallIn*> CallIn::codeMap;
map<string, CallIn*> CallIn::nameMap;


/******************************************************************************/
/******************************************************************************/

static int UpdateCallIn(lua_State* L)
{
  const string name = luaL_checkstring(L, 1);

  CallIn* ci = CallIn::Find(name);
  if (ci == NULL) {
    lua_pushboolean(L, false);
    return 1;
  }

  if (lua_isnone(L, 2)) {
    lua_pushboolean(L, false);
    return 1;
  }
    
  if (lua_isfunction(L, 2)) {
    // register
    lua_pushvalue(L, 2);
    lua_rawseti(L, LUA_CALLINSINDEX, ci->GetCode());

    ci->Register();
  }
  else if (lua_isnil(L, 2)) {
    // unregister
    lua_pushnil(L);
    lua_rawseti(L, LUA_CALLINSINDEX, ci->GetCode());

    ci->Unregister();
  }
  else {
    lua_pushboolean(L, false);
    return 1;
  }

  lua_pushboolean(L, true);
  return 1;
}


static int GetCallIns(lua_State* L)
{
  const map<string, CallIn*>& nameMap = CallIn::GetNameMap();
  
  lua_newtable(L);
  map<string, CallIn*>::const_iterator it;
  for (it = nameMap.begin(); it != nameMap.end(); ++it) {
    const CallIn* ci = it->second;
    lua_pushstring(L, ci->GetName().c_str());
    lua_rawgeti(L, LUA_CALLINSINDEX, ci->GetCode());
    if (!lua_isfunction(L, -1)) {
      lua_pop(L, 1);
      lua_pushboolean(L, true);
    }
    lua_rawset(L, -3);
  }
  return 1;
}


/******************************************************************************/
/******************************************************************************/

bool CallIns::PushEntries(lua_State* _L)
{
  L = _L;

  PUSH_LUA_CFUNC(L, UpdateCallIn);
  PUSH_LUA_CFUNC(L, GetCallIns);

  return true;
}


bool CallIns::CleanUp(lua_State* _L)
{
  L = NULL;

  const map<int, CallIn*>& codeMap = CallIn::GetCodeMap();
  map<int, CallIn*>::const_iterator it;
  for (it = codeMap.begin(); it != codeMap.end(); ++it) {
    CallIn* ci = it->second;
    ci->Unregister();
  }

  return true;
}


/******************************************************************************/
/******************************************************************************/

// lua plugin custom call-in
class CI_Shutdown : public CallIn {
  public:
    CI_Shutdown() : CallIn(bz_eShutdown, "Shutdown") {}
    ~CI_Shutdown() {}
    bool execute(bz_EventData* eventData);
};
static CI_Shutdown ciShutdown;


#define DEFINE_CALLIN(cpp, lua)                    \
  class CI_ ## lua : public CallIn {               \
    public:                                        \
      CI_ ## lua() : CallIn(bz_e ## cpp, #lua) {}  \
      ~CI_ ## lua() {}                             \
      bool execute(bz_EventData* eventData);       \
  };                                               \
  static CI_ ## lua  ci ## lua


//        bz_e <C++ enum name>            lua call-in name         difference
//        --------------------            ----------------         ----------
DEFINE_CALLIN(AllowCTFCaptureEvent,       AllowCTFCapture);        // -Event
DEFINE_CALLIN(AllowFlagGrabEvent,         AllowFlagGrab);          // -Event
DEFINE_CALLIN(AllowKillCommandEvent,      AllowKillCommand);       // -Event
DEFINE_CALLIN(AllowPlayer,                AllowPlayer);
DEFINE_CALLIN(AllowSpawn,                 AllowSpawn);
DEFINE_CALLIN(AnointRabbitEvent,          AnointRabbit);           // -Event
DEFINE_CALLIN(BanEvent,                   Ban);                    // -Event
DEFINE_CALLIN(BZDBChange,                 BZDBChange);
DEFINE_CALLIN(CaptureEvent,               Capture);                // -Event
DEFINE_CALLIN(FilteredChatMessageEvent,   FilteredChatMessage);    // -Event
DEFINE_CALLIN(FlagDroppedEvent,           FlagDropped);            // -Event
DEFINE_CALLIN(FlagGrabbedEvent,           FlagGrabbed);            // -Event
DEFINE_CALLIN(FlagResetEvent,             FlagReset);              // -Event
DEFINE_CALLIN(FlagTransferredEvent,       FlagTransfer);           // -Event
DEFINE_CALLIN(GameEndEvent,               GameEnd);                // -Event
DEFINE_CALLIN(GameStartEvent,             GameStart);              // -Event
DEFINE_CALLIN(GetAutoTeamEvent,           GetAutoTeam);            // -Event
DEFINE_CALLIN(GetPlayerInfoEvent,         GetPlayerInfo);          // -Event
DEFINE_CALLIN(GetPlayerSpawnPosEvent,     GetPlayerSpawnPos);      // -Event
DEFINE_CALLIN(GetWorldEvent,              GetWorld);               // -Event
//DEFINE_CALLIN(HostBanModifyEvent,       HostBanModify);          //  unused
DEFINE_CALLIN(HostBanNotifyEvent,         HostBan);                // -Event-
DEFINE_CALLIN(IdBanEvent,                 IdBan);                  // -Event
DEFINE_CALLIN(IdleNewNonPlayerConnection, IdleNewNonPlayerConnection);
DEFINE_CALLIN(KickEvent,                  Kick);                   // -Event
DEFINE_CALLIN(KillEvent,                  Kill);                   // -Event
DEFINE_CALLIN(ListServerUpdateEvent,      ListServerUpdate);       // -Event
DEFINE_CALLIN(LoggingEvent,               Logging);                // -Event
DEFINE_CALLIN(LuaDataEvent,               LuaData);                // -Event
//DEFINE_CALLIN(Shutdown,                 Shutdown);               //  custom
DEFINE_CALLIN(MessageFilteredEvent,       MessageFiltered);        // -Event
DEFINE_CALLIN(NetDataReceiveEvent,        NetDataReceive);         // -Event
DEFINE_CALLIN(NetDataSendEvent,           NetDataSend);            // -Event
DEFINE_CALLIN(NewNonPlayerConnection,     RawLink);                //  renamed
DEFINE_CALLIN(NewRabbitEvent,             NewRabbit);              // -Event
DEFINE_CALLIN(PlayerAuthEvent,            PlayerAuth);             // -Event
DEFINE_CALLIN(PlayerCollision,            PlayerCollision);
DEFINE_CALLIN(PlayerCustomDataChanged,    PlayerCustomDataChanged);
DEFINE_CALLIN(PlayerDieEvent,             PlayerDied);             // -Event+d
DEFINE_CALLIN(PlayerJoinEvent,            PlayerJoined);           // -Event+ed
DEFINE_CALLIN(PlayerPartEvent,            PlayerParted);           // -Event+ed
DEFINE_CALLIN(PlayerPausedEvent,          PlayerPaused);           // -Event
DEFINE_CALLIN(PlayerSentCustomData,       PlayerSentCustomData);
DEFINE_CALLIN(PlayerSpawnEvent,           PlayerSpawned);          // -Event+ed
DEFINE_CALLIN(PlayerUpdateEvent,          PlayerUpdate);           // -Event
DEFINE_CALLIN(RawChatMessageEvent,        RawChatMessage);         // -Event
DEFINE_CALLIN(ReloadEvent,                Reload);                 // -Event
DEFINE_CALLIN(ReportFiledEvent,           ReportFiled);            // -Event
DEFINE_CALLIN(ServerMsgEvent,             ServerMsg);              // -Event
DEFINE_CALLIN(ShotEndedEvent,             ShotEnded);              // -Event
DEFINE_CALLIN(ShotFiredEvent,             ShotFired);              // -Event
DEFINE_CALLIN(SlashCommandEvent,          SlashCommand);           // -Event
DEFINE_CALLIN(TeleportEvent,              Teleport);               // -Event
DEFINE_CALLIN(TickEvent,                  Tick);                   // -Event
DEFINE_CALLIN(UnknownSlashCommand,        UnknownSlashCommand);
DEFINE_CALLIN(WorldFinalized,             WorldFinalized);
DEFINE_CALLIN(ZoneEntryEvent,             ZoneEntry);              // -Event
DEFINE_CALLIN(ZoneExitEvent,              ZoneExit);               // -Event


/******************************************************************************/
/******************************************************************************/

bool CI_AllowCTFCapture::execute(bz_EventData* eventData)
{
  bz_AllowCTFCaptureEventData_V1* ed = (bz_AllowCTFCaptureEventData_V1*)eventData;

  if (!PushCallIn(9)) {
    return false;
  }

  lua_pushinteger(L, ed->teamCapped);
  lua_pushinteger(L, ed->teamCapping);
  lua_pushinteger(L, ed->playerCapping);
  lua_pushnumber(L,  ed->pos[0]);
  lua_pushnumber(L,  ed->pos[1]);
  lua_pushnumber(L,  ed->pos[2]);
  lua_pushnumber(L,  ed->rot);

  if (!RunCallIn(7, 2)) {
    return false;
  }

  if (lua_isboolean(L, -2)) {
    ed->allow = lua_tobool(L, -2);
  }

  if (lua_isboolean(L, -1)) {
    ed->killTeam = lua_tobool(L, -1);
  }

  lua_pop(L, 2);

  return true;
}


bool CI_AllowFlagGrab::execute(bz_EventData* eventData)
{
  bz_AllowFlagGrabEventData_V1* ed = (bz_AllowFlagGrabEventData_V1*)eventData;

  if (!PushCallIn(9)) {
    return false;
  }

  lua_pushinteger(L, ed->playerID);
  lua_pushinteger(L, ed->flagID);
  lua_pushstring(L,  ed->flagType);
  lua_pushinteger(L, ed->shotType);
  lua_pushnumber(L,  ed->pos[0]);
  lua_pushnumber(L,  ed->pos[1]);
  lua_pushnumber(L,  ed->pos[2]);
  
  if (!RunCallIn(7, 1)) {
    return false;
  }

  if (lua_isboolean(L, -1)) {
    ed->allow = lua_tobool(L, -1);
  }

  lua_pop(L, 1);

  return true;
}


bool CI_AllowKillCommand::execute(bz_EventData* eventData)
{
  bz_AllowKillCommandEventData_V1* ed = (bz_AllowKillCommandEventData_V1*)eventData;
  
  if (!PushCallIn(4)) {
    return false;
  }

  lua_pushinteger(L, ed->playerToKill);
  lua_pushinteger(L, ed->playerKilling);

  if (!RunCallIn(2, 1)) {
    return false;
  }

  if (lua_isboolean(L, -1)) {
    ed->allow = lua_tobool(L, -1);
  }

  lua_pop(L, 1);

  return true;
}


bool CI_AllowPlayer::execute(bz_EventData* eventData)
{
  bz_AllowPlayerEventData_V1* ed = (bz_AllowPlayerEventData_V1*)eventData;

  if (!PushCallIn(5)) {
    return false;
  }

  lua_pushinteger(L, ed->playerID);
  lua_pushstring(L,  ed->callsign.c_str());
  lua_pushstring(L,  ed->ipAddress.c_str());

  if (!RunCallIn(3, 2)) {
    return false;
  }

  if (lua_isboolean(L, -2)) {
    if (!lua_tobool(L, -2)) {
      ed->allow = false;
      ed->reason = luaL_optstring(L, -1, "lua plugin says you can not play");
    }
  }

  lua_pop(L, 2);

  return true;
}


bool CI_AllowSpawn::execute(bz_EventData* eventData)
{
  bz_AllowSpawnData_V1* ed = (bz_AllowSpawnData_V1*)eventData;
  
  if (!PushCallIn(4)) {
    return false;
  }

  lua_pushinteger(L, ed->playerID);
  lua_pushinteger(L, ed->team);

  if (!RunCallIn(2, 1)) {
    return false;
  }

  if (lua_isboolean(L, -1)) {
    ed->allow = lua_tobool(L, -1);
    // ed->handled = true; FIXME?
  }

  lua_pop(L, 1);

  return true;
}


bool CI_AnointRabbit::execute(bz_EventData* eventData)
{
  bz_AnointRabbitEventData_V1* ed = (bz_AnointRabbitEventData_V1*)eventData;

  if (!PushCallIn(3)) {
    return false;
  }

  lua_pushinteger(L, ed->newRabbit);

  if (!RunCallIn(1, 1)) {
    return false;
  }

  if (lua_israwnumber(L, -1)) {
    ed->swap = true;
    ed->newRabbit = lua_toint(L, -1);
  }

  lua_pop(L, 1);

  return true;
}


bool CI_Ban::execute(bz_EventData* eventData)
{
  bz_BanEventData_V1* ed = (bz_BanEventData_V1*)eventData;

  if (!PushCallIn(7)) {
    return false;
  }

  lua_pushinteger(L, ed->bannerID);
  lua_pushinteger(L, ed->duration);
  lua_pushstring(L,  ed->reason.c_str());
  lua_pushinteger(L, ed->banneeID);
  lua_pushstring(L,  ed->ipAddress.c_str());

  return RunCallIn(5, 0);
}


bool CI_BZDBChange::execute(bz_EventData* eventData)
{
  bz_BZDBChangeData_V1* ed = (bz_BZDBChangeData_V1*)eventData;

  if (!PushCallIn(4)) {
    return false;
  }

  lua_pushstring(L, ed->key.c_str());
  lua_pushstring(L, ed->value.c_str());

  return RunCallIn(2, 0);
}


bool CI_Capture::execute(bz_EventData* eventData)
{
  bz_CTFCaptureEventData_V1* ed = (bz_CTFCaptureEventData_V1*)eventData;

  if (!PushCallIn(9)) {
    return false;
  }

  lua_pushinteger(L, ed->teamCapped);
  lua_pushinteger(L, ed->teamCapping);
  lua_pushinteger(L, ed->playerCapping);
  lua_pushnumber(L, ed->pos[0]);
  lua_pushnumber(L, ed->pos[1]);
  lua_pushnumber(L, ed->pos[2]);
  lua_pushnumber(L, ed->rot);

  return RunCallIn(7, 0);
}


bool CI_FilteredChatMessage::execute(bz_EventData* eventData)
{
  bz_ChatEventData_V1* ed = (bz_ChatEventData_V1*)eventData;

  if (!PushCallIn(6)) {
    return false;
  }
  lua_pushstring(L, ed->message.c_str());
  lua_pushinteger(L, ed->from);
  lua_pushinteger(L, ed->to);
  lua_pushinteger(L, ed->team);

  return RunCallIn(4, 0);
}


bool CI_FlagDropped::execute(bz_EventData* eventData)
{
  bz_FlagDroppedEventData_V1* ed = (bz_FlagDroppedEventData_V1*)eventData;

  if (!PushCallIn(8)) {
    return false;
  }

  lua_pushinteger(L, ed->playerID);
  lua_pushinteger(L, ed->flagID);
  lua_pushstring(L,  ed->flagType);
  lua_pushnumber(L,  ed->pos[0]);
  lua_pushnumber(L,  ed->pos[1]);
  lua_pushnumber(L,  ed->pos[2]);

  return RunCallIn(6, 0);
}


bool CI_FlagGrabbed::execute(bz_EventData* eventData)
{
  bz_FlagGrabbedEventData_V1* ed = (bz_FlagGrabbedEventData_V1*)eventData;

  if (!PushCallIn(9)) {
    return false;
  }

  lua_pushinteger(L, ed->playerID);
  lua_pushinteger(L, ed->flagID);
  lua_pushstring(L,  ed->flagType);
  lua_pushinteger(L, ed->shotType);
  lua_pushnumber(L,  ed->pos[0]);
  lua_pushnumber(L,  ed->pos[1]);
  lua_pushnumber(L,  ed->pos[2]);

  return RunCallIn(7, 0);
}


bool CI_FlagReset::execute(bz_EventData* eventData)
{
  bz_FlagResetEventData_V1* ed = (bz_FlagResetEventData_V1*)eventData;

  if (!PushCallIn(8)) {
    return false;
  }

  lua_pushinteger(L, ed->flagID);
  lua_pushstring(L,  ed->flagType);
  lua_pushnumber(L,  ed->pos[0]);
  lua_pushnumber(L,  ed->pos[1]);
  lua_pushnumber(L,  ed->pos[2]);
  lua_pushboolean(L, ed->teamIsEmpty);
//  lua_pushboolean(L, ed->changed); // FIXME - output, unused ?

  return RunCallIn(6, 0);
}


bool CI_FlagTransfer::execute(bz_EventData* eventData)
{
  bz_FlagTransferredEventData_V1* ed = (bz_FlagTransferredEventData_V1*)eventData;

  if (!PushCallIn(5)) {
    return false;
  }

  lua_pushinteger(L, ed->fromPlayerID);
  lua_pushinteger(L, ed->toPlayerID);
  lua_pushstring(L,  ed->flagType);

  if (!RunCallIn(3, 1)) {
    return false;
  }

  if (lua_israwnumber(L, -1)) {
    ed->action = (bz_FlagTransferredEventData_V1::Action) lua_toint(L, -1);
  }

  lua_pop(L, 1);
  
  return true;
}


bool CI_GameEnd::execute(bz_EventData* eventData)
{
  bz_GameStartEndEventData_V1* ed = (bz_GameStartEndEventData_V1*)eventData;

  if (!PushCallIn(3)) {
    return false;
  }

  lua_pushdouble(L, ed->duration);
  
  return RunCallIn(1, 0);
}


bool CI_GameStart::execute(bz_EventData* eventData)
{
  bz_GameStartEndEventData_V1* ed = (bz_GameStartEndEventData_V1*)eventData;

  if (!PushCallIn(3)) {
    return false;
  }

  lua_pushdouble(L, ed->duration);
  
  return RunCallIn(1, 0);
}


bool CI_GetAutoTeam::execute(bz_EventData* eventData)
{
  bz_GetAutoTeamEventData_V1* ed = (bz_GetAutoTeamEventData_V1*)eventData;

  if (!PushCallIn(5)) {
    return false;
  }
  lua_pushinteger(L, ed->playerID);
  lua_pushinteger(L, ed->team);
  lua_pushstring(L, ed->callsign.c_str());

  if (!RunCallIn(3, 1)) {
    return false;
  }

  if (lua_israwnumber(L, -1)) {
    ed->team = (bz_eTeamType)lua_toint(L, -1); // FIXME -- use ParseTeam()
  }

  lua_pop(L, 1);

  return true;
}


bool CI_GetPlayerInfo::execute(bz_EventData* eventData)
{
  bz_GetPlayerInfoEventData_V1* ed = (bz_GetPlayerInfoEventData_V1*)eventData;

  if (!PushCallIn(9)) {
    return false;
  }

  lua_pushinteger(L, ed->playerID);
  lua_pushinteger(L, ed->team);
  lua_pushstring(L,  ed->callsign.c_str());
  lua_pushstring(L,  ed->ipAddress.c_str());
  lua_pushboolean(L, ed->admin);
  lua_pushboolean(L, ed->verified);
  lua_pushboolean(L, ed->registered);

  if (!RunCallIn(7, 3)) {
    return false;
  }

  if (lua_isboolean(L, -3)) { ed->admin      = lua_tobool(L, -3); }
  if (lua_isboolean(L, -2)) { ed->verified   = lua_tobool(L, -2); }
  if (lua_isboolean(L, -1)) { ed->registered = lua_tobool(L, -1); }

  lua_pop(L, 3);

  return true;    
}


bool CI_GetPlayerSpawnPos::execute(bz_EventData* eventData)
{
  bz_GetPlayerSpawnPosEventData_V1* ed = (bz_GetPlayerSpawnPosEventData_V1*)eventData;

  if (!PushCallIn(8)) {
    return false;
  }

  lua_pushinteger(L, ed->playerID);
  lua_pushinteger(L, ed->team);

  lua_pushnumber(L, ed->pos[0]);
  lua_pushnumber(L, ed->pos[1]);
  lua_pushnumber(L, ed->pos[2]);
  lua_pushnumber(L, ed->rot);

  if (!RunCallIn(6, 4)) {
    return false;
  }

  if (lua_israwnumber(L, -4) &&
      lua_israwnumber(L, -3) &&
      lua_israwnumber(L, -2)) {
    ed->pos[0] = lua_tofloat(L, -4);
    ed->pos[1] = lua_tofloat(L, -3);
    ed->pos[2] = lua_tofloat(L, -2);
    ed->handled = true; // FIXME ?
  }

  if (lua_israwnumber(L, -1)) {
    ed->rot = lua_tofloat(L, -1);
    ed->handled = true; // FIXME ?
  }

  lua_pop(L, 4);    

  return true;
}


bool CI_GetWorld::execute(bz_EventData* eventData)
{
  bz_GetWorldEventData_V1* ed = (bz_GetWorldEventData_V1*)eventData;

  if (!PushCallIn(4)) {
    return false;
  }

  bz_eGameType gameMode = eTeamFFAGame;
  if      (ed->ctf)     { gameMode = eClassicCTFGame; }
  else if (ed->rabbit)  { gameMode = eRabbitGame;     }
  else if (ed->openFFA) { gameMode = eOpenFFAGame;    }

  lua_pushinteger(L, gameMode);
  lua_pushstring(L,  ed->worldFile.c_str());
  lua_pushboolean(L, ed->worldBlob != NULL);
//  lua_pushboolean(L, ed->generated);

  if (!RunCallIn(3, 1)) {
    return false;
  }

  if (lua_istable(L, -1)) {
    const int table = lua_gettop(L);
    for (lua_pushnil(L); lua_next(L, table) != 0; lua_pop(L, 1)) {
      if (!lua_israwstring(L, -2)) {
        continue;
      }
      const string key = lua_tostring(L, -2);
      if (key == "file") {
        if (lua_israwstring(L, -1)) {
          ed->worldFile = lua_tostring(L, -1);
        }
      }
      else if (key == "data") {
        if (lua_israwstring(L, -1)) {
          size_t len = 0;
          const char* blob = lua_tolstring(L, -1, &len);
          char* newBlob = new char[len + 1];
          memcpy(newBlob, blob, len);
          newBlob[len] = 0;
          delete[] ed->worldBlob;
          ed->worldBlob = newBlob; // FIXME: properly deleted?
        }
      }
      else if (key == "mode") {
        if (lua_israwnumber(L, -1)) {
          ed->openFFA = false;
          ed->rabbit  = false;
          ed->ctf     = false;
          switch ((bz_eGameType)lua_toint(L, -1)) {
            case eOpenFFAGame:    { ed->openFFA = true; break; }
            case eRabbitGame:     { ed->rabbit  = true; break; }
            case eClassicCTFGame: { ed->ctf     = true; break; }
            default: {
              break;
            }
          }
        }
      }
    } // end for
  } // end if lua_istable()

  lua_pop(L, 1);

  return true;
}


/* unused
bool CI_HostBanModify::execute(bz_EventData* eventData)
{
  bz_HostBanEventData_V1* ed = (bz_HostBanEventData_V1*)eventData;

  if (!PushCallIn(6)) {
    return false;
  }

  lua_pushinteger(L, ed->bannerID);
  lua_pushinteger(L, ed->duration);
  lua_pushstring(L,  ed->reason.c_str());
  lua_pushstring(L,  ed->hostPattern.c_str());

  return RunCallIn(4, 0);
}
*/


bool CI_HostBan::execute(bz_EventData* eventData)
{
  bz_HostBanEventData_V1* ed = (bz_HostBanEventData_V1*)eventData;

  if (!PushCallIn(6)) {
    return false;
  }

  lua_pushinteger(L, ed->bannerID);
  lua_pushinteger(L, ed->duration);
  lua_pushstring(L,  ed->reason.c_str());
  lua_pushstring(L,  ed->hostPattern.c_str());

  return RunCallIn(4, 0);
}


bool CI_IdBan::execute(bz_EventData* eventData)
{
  bz_IdBanEventData_V1* ed = (bz_IdBanEventData_V1*)eventData;

  if (!PushCallIn(7)) {
    return false;
  }

  lua_pushinteger(L, ed->bannerID);
  lua_pushinteger(L, ed->duration);
  lua_pushstring(L,  ed->reason.c_str());
  lua_pushinteger(L, ed->banneeID);
  lua_pushstring(L,  ed->bzId.c_str());

  return RunCallIn(5, 0);
}


bool CI_IdleNewNonPlayerConnection::execute(bz_EventData* eventData)
{
  bz_NewNonPlayerConnectionEventData_V1* ed = (bz_NewNonPlayerConnectionEventData_V1*)eventData;

  if (!PushCallIn(4)) {
    return false;
  }

  lua_pushinteger(L, ed->connectionID);
  lua_pushlstring(L, (char*)ed->data, ed->size);

  return RunCallIn(2, 0);
}


bool CI_Kick::execute(bz_EventData* eventData)
{
  bz_KickEventData_V1* ed = (bz_KickEventData_V1*)eventData;

  if (!PushCallIn(5)) {
    return false;
  }
  
  lua_pushinteger(L, ed->kickerID);
  lua_pushinteger(L, ed->kickedID);
  lua_pushstring(L,  ed->reason.c_str());

  return RunCallIn(3, 0);
}


bool CI_Kill::execute(bz_EventData* eventData)
{
  bz_KillEventData_V1* ed = (bz_KillEventData_V1*)eventData;

  if (!PushCallIn(5)) {
    return false;
  }

  lua_pushinteger(L, ed->killerID);
  lua_pushinteger(L, ed->killedID);
  lua_pushstring(L,  ed->reason.c_str());

  return RunCallIn(3, 0);
}


bool CI_ListServerUpdate::execute(bz_EventData* eventData)
{
  bz_ListServerUpdateEvent_V1* ed = (bz_ListServerUpdateEvent_V1*)eventData;

  if (!PushCallIn(5)) {
    return false;
  }

  lua_pushstring(L, ed->address.c_str());
  lua_pushstring(L, ed->description.c_str());
  lua_pushstring(L, ed->groups.c_str());

  // bool handled; FIXME - used?

  return RunCallIn(3, 0);
}


bool CI_Logging::execute(bz_EventData* eventData)
{
  bz_LoggingEventData_V1* ed = (bz_LoggingEventData_V1*)eventData;

  if (!PushCallIn(4)) {
    return false;
  }

  lua_pushstring(L, ed->message.c_str());
  lua_pushinteger(L, ed->level);

  return RunCallIn(2, 0);
}


bool CI_LuaData::execute(bz_EventData* eventData)
{
  bz_LuaDataEventData_V1* ed = (bz_LuaDataEventData_V1*)eventData;

  if (!PushCallIn(8)) {
    return false;
  }

  lua_pushinteger(L, ed->srcPlayerID);
  lua_pushinteger(L, ed->srcScriptID);
  lua_pushinteger(L, ed->dstPlayerID);
  lua_pushinteger(L, ed->dstScriptID);
  lua_pushinteger(L, ed->status);
  lua_pushlstring(L, ed->data, ed->dataLen);

  if (!RunCallIn(6, 1)) {
    return false;
  }

  if (lua_isboolean(L, -1)) {
    ed->doNotSend = lua_tobool(L, -1);
  }

  lua_pop(L, 1);

  return true;
}


bool CI_Shutdown::execute(bz_EventData* eventData)
{
  if (!PushCallIn(2)) {
    return false;
  }
  return RunCallIn(0, 0);
}


bool CI_MessageFiltered::execute(bz_EventData* eventData)
{
  bz_MessageFilteredEventData_V1* ed = (bz_MessageFilteredEventData_V1*)eventData;

  if (!PushCallIn(5)) {
    return false;
  }

  lua_pushinteger(L, ed->playerID);
  lua_pushstring(L,  ed->rawMessage.c_str());
  lua_pushstring(L,  ed->filteredMessage.c_str());
  
  return RunCallIn(3, 0);
}


bool CI_NetDataReceive::execute(bz_EventData* eventData)
{
  bz_NetTransferEventData_V1* ed = (bz_NetTransferEventData_V1*)eventData;

  if (!PushCallIn(5)) {
    return false;
  }

  //lua_pushboolean(L, ed->send); // FIXME?
  lua_pushinteger(L, ed->playerID);
  lua_pushboolean(L, ed->udp);
  lua_pushlstring(L, (char*)ed->data, ed->iSize);

  return RunCallIn(3, 0);
}


bool CI_NetDataSend::execute(bz_EventData* eventData)
{
  bz_NetTransferEventData_V1* ed = (bz_NetTransferEventData_V1*)eventData;

  if (!PushCallIn(5)) {
    return false;
  }

  //lua_pushboolean(L, ed->send); // FIXME?
  lua_pushinteger(L, ed->playerID);
  lua_pushboolean(L, ed->udp);
  lua_pushlstring(L, (char*)ed->data, ed->iSize);

  return RunCallIn(3, 0);
}


bool CI_RawLink::execute(bz_EventData* eventData)
{
  bz_NewNonPlayerConnectionEventData_V1* ed = (bz_NewNonPlayerConnectionEventData_V1*)eventData;

  if (!PushCallIn(4)) {
    return false;
  }

  lua_pushinteger(L, ed->connectionID);
  lua_pushlstring(L, (char*)ed->data, ed->size);

  return RunCallIn(2, 0);
}


bool CI_NewRabbit::execute(bz_EventData* eventData)
{
  bz_NewRabbitEventData_V1* ed = (bz_NewRabbitEventData_V1*)eventData;

  if (!PushCallIn(3)) {
    return false;
  }

  lua_pushinteger(L, ed->newRabbit);

  return RunCallIn(1, 0);
}


bool CI_PlayerAuth::execute(bz_EventData* eventData)
{
  bz_PlayerAuthEventData_V1* ed = (bz_PlayerAuthEventData_V1*)eventData;

  if (!PushCallIn(5)) {
    return false;
  }

  // FIXME -- outputs ?
  lua_pushinteger(L, ed->playerID);
  lua_pushboolean(L, ed->password);
  lua_pushboolean(L, ed->globalAuth);
  
  return RunCallIn(3, 0);
}


bool CI_PlayerCollision::execute(bz_EventData* eventData)
{
  bz_PlayerCollisionEventData_V1* ed = (bz_PlayerCollisionEventData_V1*)eventData;

  if (ed->handled) {
    return true;
  }

  if (!PushCallIn(7)) {
    return false;
  }

  lua_pushinteger(L, ed->players[0]);
  lua_pushinteger(L, ed->players[1]);
  lua_pushnumber(L,  ed->pos[0]);
  lua_pushnumber(L,  ed->pos[1]);
  lua_pushnumber(L,  ed->pos[2]);

  if (!RunCallIn(5, 1)) {
    return false;
  }

  if (lua_isboolean(L, -1)) {
    ed->handled = lua_tobool(L, -1);
  }

  lua_pop(L, 1);

  return true;
}


bool CI_PlayerCustomDataChanged::execute(bz_EventData* eventData)
{
  bz_PlayerSentCustomData_V1* ed = (bz_PlayerSentCustomData_V1*)eventData;

  if (!PushCallIn(5)) {
    return false;
  }

  lua_pushinteger(L, ed->playerID);
  lua_pushstring(L, ed->key.c_str());
  lua_pushstring(L, ed->data.c_str());
  
  return RunCallIn(3, 0);
}


bool CI_PlayerDied::execute(bz_EventData* eventData)
{
  bz_PlayerDieEventData_V1* ed = (bz_PlayerDieEventData_V1*)eventData;

  if (!PushCallIn(12)) {
    return false;
  }

  lua_pushinteger(L, ed->playerID);
  lua_pushinteger(L, ed->team);
  lua_pushinteger(L, ed->killerID);
  lua_pushinteger(L, ed->killerTeam);
  lua_pushstring(L,  ed->flagKilledWith.c_str());
  lua_pushinteger(L, ed->shotID);
  lua_pushnumber(L,  ed->state.pos[0]);
  lua_pushnumber(L,  ed->state.pos[1]);
  lua_pushnumber(L,  ed->state.pos[2]);
  lua_pushnumber(L,  ed->state.rotation);
  // bz_PlayerUpdateState state; -- FIXME?

  return RunCallIn(10, 0);
}


bool CI_PlayerJoined::execute(bz_EventData* eventData)
{
  bz_PlayerJoinPartEventData_V1* ed = (bz_PlayerJoinPartEventData_V1*)eventData;

  if (!PushCallIn(5)) {
    return false;
  }
  lua_pushinteger(L, ed->playerID);
  lua_pushinteger(L, ed->record->team);
  lua_pushstring(L,  ed->record->callsign.c_str());
  // lua_pushstring(L,  ed->reason.c_str()); -- unused

  // bz_BasePlayerRecord* record; -- FIXME add more ?

  return RunCallIn(3, 0);
}


bool CI_PlayerParted::execute(bz_EventData* eventData)
{
  bz_PlayerJoinPartEventData_V1* ed = (bz_PlayerJoinPartEventData_V1*)eventData;
  
  if (!PushCallIn(6)) {
    return false;
  }
  lua_pushinteger(L, ed->playerID);
  lua_pushinteger(L, ed->record->team);
  lua_pushstring(L,  ed->record->callsign.c_str());
  lua_pushstring(L,  ed->reason.c_str());

  // bz_BasePlayerRecord* record; -- FIXME add more ?

  return RunCallIn(4, 0);
}


bool CI_PlayerPaused::execute(bz_EventData* eventData)
{
  bz_PlayerPausedEventData_V1* ed = (bz_PlayerPausedEventData_V1*)eventData;

  if (!PushCallIn(4)) {
    return false;
  }

  lua_pushinteger(L, ed->playerID);
  lua_pushboolean(L, ed->pause);

  return RunCallIn(2, 0);
}


bool CI_PlayerSentCustomData::execute(bz_EventData* eventData)
{
  bz_PlayerSentCustomData_V1* ed = (bz_PlayerSentCustomData_V1*)eventData;

  if (!PushCallIn(5)) {
    return false;
  }

  lua_pushinteger(L, ed->playerID);
  lua_pushstring(L, ed->key.c_str());
  lua_pushstring(L, ed->data.c_str());
  
  return RunCallIn(3, 0);
}


bool CI_PlayerSpawned::execute(bz_EventData* eventData)
{
  bz_PlayerSpawnEventData_V1* ed = (bz_PlayerSpawnEventData_V1*)eventData;

  if (!PushCallIn(4)) {
    return false;
  }

  lua_pushinteger(L, ed->playerID);
  lua_pushinteger(L, ed->team);

  const bz_PlayerUpdateState& state = ed->state;
  lua_pushnumber(L, state.pos[0]);
  lua_pushnumber(L, state.pos[1]);
  lua_pushnumber(L, state.pos[2]);
  lua_pushnumber(L, state.rotation);

  // bz_PlayerUpdateState state; -- FIXME?

  return RunCallIn(6, 0);
}


bool CI_PlayerUpdate::execute(bz_EventData* eventData)
{
  bz_PlayerUpdateEventData_V1* ed = (bz_PlayerUpdateEventData_V1*)eventData;

  if (!PushCallIn(16)) {
    return false;
  }

  const bz_PlayerUpdateState& state = ed->state;

  lua_pushinteger(L, ed->playerID);

  lua_pushinteger(L, state.status);
  lua_pushinteger(L, state.phydrv);
  lua_pushboolean(L, state.falling);
  lua_pushboolean(L, state.crossingWall);
  lua_pushboolean(L, state.inPhantomZone);

  lua_pushnumber(L, state.pos[0]);
  lua_pushnumber(L, state.pos[1]);
  lua_pushnumber(L, state.pos[2]);
  lua_pushnumber(L, state.rotation);

  lua_pushnumber(L, state.velocity[0]);
  lua_pushnumber(L, state.velocity[1]);
  lua_pushnumber(L, state.velocity[2]);
  lua_pushnumber(L, state.angVel);

  // FIXME double stateTime;
  
  return RunCallIn(14, 0);
}


bool CI_RawChatMessage::execute(bz_EventData* eventData)
{
  bz_ChatEventData_V1* ed = (bz_ChatEventData_V1*)eventData;

  if (!PushCallIn(6)) {
    return false;
  }
  lua_pushstring(L, ed->message.c_str());
  lua_pushinteger(L, ed->from);
  lua_pushinteger(L, ed->to);
  lua_pushinteger(L, ed->team);

  if (!RunCallIn(4, 1)) {
    return false;
  }

  if (lua_israwstring(L, -1)) {
    ed->message = lua_tostring(L, -1);
  }

  lua_pop(L, 1);

  return true;
}


bool CI_Reload::execute(bz_EventData* eventData)
{
  bz_ReloadEventData_V1* ed = (bz_ReloadEventData_V1*)eventData;

  if (!PushCallIn(3)) {
    return false;
  }

  lua_pushinteger(L, ed->playerID);

  return RunCallIn(1, 0);
}


bool CI_ReportFiled::execute(bz_EventData* eventData)
{
  bz_ReportFiledEventData_V1* ed = (bz_ReportFiledEventData_V1*)eventData;

  if (!PushCallIn(4)) {
    return false;
  }

  lua_pushstring(L, ed->from.c_str());
  lua_pushstring(L, ed->message.c_str());
  
  return RunCallIn(2, 0);
}


bool CI_ServerMsg::execute(bz_EventData* eventData)
{
  bz_ServerMsgEventData_V1* ed = (bz_ServerMsgEventData_V1*)eventData;

  if (!PushCallIn(5)) {
    return false;
  }

  lua_pushinteger(L, ed->to);
  lua_pushinteger(L, ed->team);
  lua_pushstring(L,  ed->message.c_str());
  
  return RunCallIn(3, 0);
}


bool CI_ShotEnded::execute(bz_EventData* eventData)
{
  bz_ShotEndedEventData_V1* ed = (bz_ShotEndedEventData_V1*)eventData;

  if (!PushCallIn(4)) {
    return false;
  }

  lua_pushinteger(L, ed->playerID);
  lua_pushinteger(L, ed->shotID);
//  lua_pushboolean(L, ed->explode); // FIXME?
  
  return RunCallIn(2, 0);
}


bool CI_ShotFired::execute(bz_EventData* eventData)
{
  bz_ShotFiredEventData_V1* ed = (bz_ShotFiredEventData_V1*)eventData;

  if (!PushCallIn(7)) {
    return false;
  }

  lua_pushinteger(L, ed->playerID);
  lua_pushstring(L,  ed->type.c_str());
  lua_pushnumber(L,  ed->pos[0]);
  lua_pushnumber(L,  ed->pos[1]);
  lua_pushnumber(L,  ed->pos[2]);

  //lua_pushboolean(L, ed->changed); // FIXME - output? used?
  
  return RunCallIn(5, 0);
}


bool CI_SlashCommand::execute(bz_EventData* eventData)
{
  bz_SlashCommandEventData_V1* ed = (bz_SlashCommandEventData_V1*)eventData;

  if (!PushCallIn(4)) {
    return false;
  }

  lua_pushstring(L,  ed->message.c_str());
  lua_pushinteger(L, ed->from);

  return RunCallIn(2, 0);
}


bool CI_Teleport::execute(bz_EventData* eventData)
{
  bz_TeleportEventData_V1* ed = (bz_TeleportEventData_V1*)eventData;

  if (!PushCallIn(5)) {
    return false;
  }

  lua_pushinteger(L, ed->playerID);
  lua_pushinteger(L, ed->from);
  lua_pushinteger(L, ed->to);
  
  return RunCallIn(3, 0);
}


bool CI_Tick::execute(bz_EventData* eventData)
{
  bz_TickEventData_V1* ed = (bz_TickEventData_V1*)eventData;

  if (!PushCallIn(2)) {
    return false;
  }

  return RunCallIn(0, 0);
}


bool CI_UnknownSlashCommand::execute(bz_EventData* eventData)
{
  bz_UnknownSlashCommandEventData_V1* ed = (bz_UnknownSlashCommandEventData_V1*)eventData;

  if (ed->handled) {
    return true;
  }
  
  if (!PushCallIn(4)) {
    return false;
  }
  lua_pushstring(L,  ed->message.c_str());
  lua_pushinteger(L, ed->from);

  if (!RunCallIn(2, 1)) {
    return false;
  }

  if (lua_isboolean(L, -1)) {
    ed->handled = lua_tobool(L, -1);
  }

  lua_pop(L, 1);

  return true;
}


bool CI_WorldFinalized::execute(bz_EventData* eventData)
{
  // NOTE: plain bz_EventData type, no extra data
  if (!PushCallIn(2)) {
    return false;
  }
  return RunCallIn(0, 0);
}


bool CI_ZoneEntry::execute(bz_EventData* eventData)
{
  // FIXME -- not implemented ?
  if (!PushCallIn(2)) {
    return false;
  }
  return RunCallIn(0, 0);
}


bool CI_ZoneExit::execute(bz_EventData* eventData)
{
  // FIXME -- not implemented ?
  if (!PushCallIn(2)) {
    return false;
  }
  return RunCallIn(0, 0);
}


/******************************************************************************/
/******************************************************************************/

bool CallIns::Shutdown()
{
  bz_EventData eventData(bz_eShutdown);
  return ciShutdown.execute(&eventData);
}


/******************************************************************************/
/******************************************************************************/
