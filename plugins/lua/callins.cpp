
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
        return true;
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
        return true;
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

#define REGISTER_LUA_CFUNC(x) \
  lua_pushliteral(L, #x);     \
  lua_pushcfunction(L, x);    \
  lua_rawset(L, -3)

  REGISTER_LUA_CFUNC(UpdateCallIn);
  REGISTER_LUA_CFUNC(GetCallIns);

  // scan for global call-ins --  FIXME -- remove? have lua do it?
  /*
  const map<int, CallIn*>& codeMap = CallIn::GetCodeMap();
  map<int, CallIn*>::const_iterator it;
  for (it = codeMap.begin(); it != codeMap.end(); ++it) {
    CallIn* ci = it->second;
    lua_getglobal(L, ci->GetName().c_str());
    if (lua_isfunction(L, -1)) {
      lua_rawseti(L, LUA_CALLINSINDEX, ci->GetCode());
      ci->Register();
    } else {
      lua_pop(L, 1);
    }
  }
  */

  return true;
}


bool CallIns::Shutdown(lua_State* _L)
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

#define DEFINE_CALLIN(x)                     \
  class CI_ ## x : public CallIn {           \
    public:                                  \
      CI_ ## x() : CallIn(bz_e ## x, #x) {}  \
      ~CI_ ## x() {}                         \
      bool execute(bz_EventData* eventData); \
  };                                         \
  static CI_ ## x  ci ## x


DEFINE_CALLIN(AllowCTFCaptureEvent);
DEFINE_CALLIN(AllowFlagGrabEvent);
DEFINE_CALLIN(AllowKillCommandEvent);
DEFINE_CALLIN(AllowPlayer);
DEFINE_CALLIN(AllowSpawn);
DEFINE_CALLIN(AnointRabbitEvent);
DEFINE_CALLIN(BanEvent);
DEFINE_CALLIN(BZDBChange);
DEFINE_CALLIN(CaptureEvent);
DEFINE_CALLIN(FilteredChatMessageEvent);
DEFINE_CALLIN(FlagDroppedEvent);
DEFINE_CALLIN(FlagGrabbedEvent);
DEFINE_CALLIN(FlagResetEvent);
DEFINE_CALLIN(FlagTransferredEvent);
DEFINE_CALLIN(GameEndEvent);
DEFINE_CALLIN(GameStartEvent);
DEFINE_CALLIN(GetAutoTeamEvent);
DEFINE_CALLIN(GetPlayerInfoEvent);
DEFINE_CALLIN(GetPlayerSpawnPosEvent);
DEFINE_CALLIN(GetWorldEvent);
DEFINE_CALLIN(HostBanModifyEvent);
DEFINE_CALLIN(HostBanNotifyEvent);
DEFINE_CALLIN(IdBanEvent);
DEFINE_CALLIN(IdleNewNonPlayerConnection);
DEFINE_CALLIN(KickEvent);
DEFINE_CALLIN(KillEvent);
DEFINE_CALLIN(ListServerUpdateEvent);
DEFINE_CALLIN(LoggingEvent);
DEFINE_CALLIN(MessageFilteredEvent);
DEFINE_CALLIN(NetDataReceiveEvent);
DEFINE_CALLIN(NetDataSendEvent);
DEFINE_CALLIN(NewNonPlayerConnection);
DEFINE_CALLIN(NewRabbitEvent);
DEFINE_CALLIN(PlayerAuthEvent);
DEFINE_CALLIN(PlayerCollision);
DEFINE_CALLIN(PlayerCustomDataChanged);
DEFINE_CALLIN(PlayerDieEvent);
DEFINE_CALLIN(PlayerJoinEvent);
DEFINE_CALLIN(PlayerPartEvent);
DEFINE_CALLIN(PlayerPausedEvent);
DEFINE_CALLIN(PlayerSentCustomData);
DEFINE_CALLIN(PlayerSpawnEvent);
DEFINE_CALLIN(PlayerUpdateEvent);
DEFINE_CALLIN(RawChatMessageEvent);
DEFINE_CALLIN(ReloadEvent);
DEFINE_CALLIN(ReportFiledEvent);
DEFINE_CALLIN(ServerMsgEvent);
DEFINE_CALLIN(ShotEndedEvent);
DEFINE_CALLIN(ShotFiredEvent);
DEFINE_CALLIN(SlashCommandEvent);
DEFINE_CALLIN(TeleportEvent);
DEFINE_CALLIN(TickEvent);
DEFINE_CALLIN(UnknownSlashCommand);
DEFINE_CALLIN(WorldFinalized);
DEFINE_CALLIN(ZoneEntryEvent);
DEFINE_CALLIN(ZoneExitEvent);


/******************************************************************************/
/******************************************************************************/


bool CI_AllowCTFCaptureEvent::execute(bz_EventData* eventData)
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
    ed->allow = lua_toboolean(L, -2);
  }

  if (lua_isboolean(L, -1)) {
    ed->killTeam = lua_toboolean(L, -1);
  }

  lua_pop(L, 2);

  return true;
}


bool CI_AllowFlagGrabEvent::execute(bz_EventData* eventData)
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
    ed->allow = lua_toboolean(L, -1);
  }
  lua_pop(L, 1);

  return true;
}


bool CI_AllowKillCommandEvent::execute(bz_EventData* eventData)
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
    ed->allow = lua_toboolean(L, -1);
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
    if (!lua_toboolean(L, -2)) {
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
    ed->allow = lua_toboolean(L, -1);
  }
  //   bool handled; ?

  lua_pop(L, 1);

  return true;
}


bool CI_AnointRabbitEvent::execute(bz_EventData* eventData)
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


bool CI_BanEvent::execute(bz_EventData* eventData)
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


bool CI_CaptureEvent::execute(bz_EventData* eventData)
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


bool CI_FilteredChatMessageEvent::execute(bz_EventData* eventData)
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


bool CI_FlagDroppedEvent::execute(bz_EventData* eventData)
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


bool CI_FlagGrabbedEvent::execute(bz_EventData* eventData)
{
  bz_FlagGrabbedEventData_V1* ed = (bz_FlagGrabbedEventData_V1*)eventData;

  if (!PushCallIn(9)) {
    return false;
  }

  lua_pushinteger(L, ed->playerID);
  lua_pushinteger(L, ed->flagID);
  lua_pushstring(L,  ed->flagType);
  lua_pushnumber(L,  ed->shotType);
  lua_pushnumber(L,  ed->pos[0]);
  lua_pushnumber(L,  ed->pos[1]);
  lua_pushnumber(L,  ed->pos[2]);

  return RunCallIn(7, 0);
}


bool CI_FlagResetEvent::execute(bz_EventData* eventData)
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


bool CI_FlagTransferredEvent::execute(bz_EventData* eventData)
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


bool CI_GameEndEvent::execute(bz_EventData* eventData)
{
  bz_GameStartEndEventData_V1* ed = (bz_GameStartEndEventData_V1*)eventData;

  if (!PushCallIn(3)) {
    return false;
  }

  lua_pushnumber(L, ed->duration);
  
  return RunCallIn(1, 0);
}


bool CI_GameStartEvent::execute(bz_EventData* eventData)
{
  bz_GameStartEndEventData_V1* ed = (bz_GameStartEndEventData_V1*)eventData;

  if (!PushCallIn(3)) {
    return false;
  }

  lua_pushnumber(L, ed->duration);
  
  return RunCallIn(1, 0);
}


bool CI_GetAutoTeamEvent::execute(bz_EventData* eventData)
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


bool CI_GetPlayerInfoEvent::execute(bz_EventData* eventData)
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

  if (lua_isboolean(L, -3)) { ed->admin      = lua_toboolean(L, -3); }
  if (lua_isboolean(L, -2)) { ed->verified   = lua_toboolean(L, -2); }
  if (lua_isboolean(L, -1)) { ed->registered = lua_toboolean(L, -1); }

  lua_pop(L, 3);

  return true;    
}


bool CI_GetPlayerSpawnPosEvent::execute(bz_EventData* eventData)
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


bool CI_GetWorldEvent::execute(bz_EventData* eventData)
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


bool CI_HostBanModifyEvent::execute(bz_EventData* eventData)
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


bool CI_HostBanNotifyEvent::execute(bz_EventData* eventData)
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


bool CI_IdBanEvent::execute(bz_EventData* eventData)
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


bool CI_KickEvent::execute(bz_EventData* eventData)
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


bool CI_KillEvent::execute(bz_EventData* eventData)
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


bool CI_ListServerUpdateEvent::execute(bz_EventData* eventData)
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


bool CI_LoggingEvent::execute(bz_EventData* eventData)
{
  bz_LoggingEventData_V1* ed = (bz_LoggingEventData_V1*)eventData;

  if (!PushCallIn(4)) {
    return false;
  }

  lua_pushstring(L, ed->message.c_str());
  lua_pushinteger(L, ed->level);

  return RunCallIn(2, 0);
}


bool CI_MessageFilteredEvent::execute(bz_EventData* eventData)
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


bool CI_NetDataReceiveEvent::execute(bz_EventData* eventData)
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


bool CI_NetDataSendEvent::execute(bz_EventData* eventData)
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


bool CI_NewNonPlayerConnection::execute(bz_EventData* eventData)
{
  bz_NewNonPlayerConnectionEventData_V1* ed = (bz_NewNonPlayerConnectionEventData_V1*)eventData;

  if (!PushCallIn(4)) {
    return false;
  }

  lua_pushinteger(L, ed->connectionID);
  lua_pushlstring(L, (char*)ed->data, ed->size);

  return RunCallIn(2, 0);
}


bool CI_NewRabbitEvent::execute(bz_EventData* eventData)
{
  bz_NewRabbitEventData_V1* ed = (bz_NewRabbitEventData_V1*)eventData;

  if (!PushCallIn(3)) {
    return false;
  }

  lua_pushinteger(L, ed->newRabbit);

  return RunCallIn(1, 0);
}


bool CI_PlayerAuthEvent::execute(bz_EventData* eventData)
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
    ed->handled = lua_toboolean(L, -1);
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


bool CI_PlayerDieEvent::execute(bz_EventData* eventData)
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


bool CI_PlayerJoinEvent::execute(bz_EventData* eventData)
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


bool CI_PlayerPartEvent::execute(bz_EventData* eventData)
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


bool CI_PlayerPausedEvent::execute(bz_EventData* eventData)
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


bool CI_PlayerSpawnEvent::execute(bz_EventData* eventData)
{
  bz_PlayerSpawnEventData_V1* ed = (bz_PlayerSpawnEventData_V1*)eventData;

  if (!PushCallIn(4)) {
    return false;
  }

  lua_pushinteger(L, ed->playerID);
  lua_pushinteger(L, ed->team);

  // bz_PlayerUpdateState state; -- FIXME?

  return RunCallIn(2, 0);
}


bool CI_PlayerUpdateEvent::execute(bz_EventData* eventData)
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


bool CI_RawChatMessageEvent::execute(bz_EventData* eventData)
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


bool CI_ReloadEvent::execute(bz_EventData* eventData)
{
  bz_ReloadEventData_V1* ed = (bz_ReloadEventData_V1*)eventData;

  if (!PushCallIn(3)) {
    return false;
  }

  lua_pushinteger(L, ed->playerID);

  return RunCallIn(1, 0);
}


bool CI_ReportFiledEvent::execute(bz_EventData* eventData)
{
  bz_ReportFiledEventData_V1* ed = (bz_ReportFiledEventData_V1*)eventData;

  if (!PushCallIn(4)) {
    return false;
  }

  lua_pushstring(L, ed->from.c_str());
  lua_pushstring(L, ed->message.c_str());
  
  return RunCallIn(2, 0);
}


bool CI_ServerMsgEvent::execute(bz_EventData* eventData)
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


bool CI_ShotEndedEvent::execute(bz_EventData* eventData)
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


bool CI_ShotFiredEvent::execute(bz_EventData* eventData)
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


bool CI_SlashCommandEvent::execute(bz_EventData* eventData)
{
  bz_SlashCommandEventData_V1* ed = (bz_SlashCommandEventData_V1*)eventData;

  if (!PushCallIn(4)) {
    return false;
  }

  lua_pushstring(L,  ed->message.c_str());
  lua_pushinteger(L, ed->from);

  return RunCallIn(2, 0);
}


bool CI_TeleportEvent::execute(bz_EventData* eventData)
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


bool CI_TickEvent::execute(bz_EventData* eventData)
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
    ed->handled = lua_toboolean(L, -1);
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


bool CI_ZoneEntryEvent::execute(bz_EventData* eventData)
{
  // FIXME -- not implemented ?
  if (!PushCallIn(2)) {
    return false;
  }
  return RunCallIn(0, 0);
}


bool CI_ZoneExitEvent::execute(bz_EventData* eventData)
{
  // FIXME -- not implemented ?
  if (!PushCallIn(2)) {
    return false;
  }
  return RunCallIn(0, 0);
}


/******************************************************************************/
/******************************************************************************/
