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


// interface header
#include "MsgStrings.h"

// system headers
#include <string>
#include <vector>
#include <map>
#include <assert.h>
#include <stdarg.h>
#include <math.h>

// common headers
#include "global.h"
#include "Protocol.h"
#include "Pack.h"
#include "AnsiCodes.h"
#include "TextUtils.h"
#include "Address.h"
#include "StateDatabase.h"
#include "PlayerState.h"
#include "Team.h"
#include "Flag.h"
#include "ShotUpdate.h"


// static bool UseDNS = true;
static bool useColor = true;
static bool useStateTracking = true;


struct PacketInfo {
  PacketInfo(uint16_t l, uint16_t c, const void* d) : len(l), code(c), data(d) {}
  uint16_t len;
  uint16_t code;
  const void *data;
};


typedef std::map<uint16_t, std::string> FlagList;
static FlagList flagList;

typedef std::map<uint16_t, std::string> PlayerList;
static PlayerList playerList;

typedef std::map<uint16_t, const struct PacketListEntry*> PacketCodeMap;
static PacketCodeMap packetCodeMap;


struct PacketListEntry {
  PacketListEntry(uint16_t _code, const char* _label,
                  MsgStringList (*_handler)(const PacketInfo&))
  : code(_code)
  , label(_label)
  , handler(_handler)
  {
    packetCodeMap[code] = this;
  }
  uint16_t code;
  const char *label;
  MsgStringList (*handler)(const PacketInfo& pi);
};


static MsgStringList listMsgBasics(const PacketInfo& pi);
static void listPush(MsgStringList &list, int level, const char *fmt, ...);
static std::string strFlag(uint16_t id);
static std::string strFlagStatus(FlagStatus status);
static std::string strTeam(uint16_t id);
static std::string strPlayer(uint16_t id);
static std::string strVector(const fvec3& vector);
static std::string strKillReason(int16_t reason);
static std::string strAddress(Address& address);


// the network packet types
#define PACKET_LIST_ENTRY(x) \
  static MsgStringList   handle##x(const PacketInfo& pi); \
  static PacketListEntry struct##x(x, #x, handle##x);

PACKET_LIST_ENTRY(MsgNull)
PACKET_LIST_ENTRY(MsgAccept)
PACKET_LIST_ENTRY(MsgAdminInfo)
PACKET_LIST_ENTRY(MsgAlive)
PACKET_LIST_ENTRY(MsgAllow)
PACKET_LIST_ENTRY(MsgAddPlayer)
PACKET_LIST_ENTRY(MsgAllowSpawn)
PACKET_LIST_ENTRY(MsgAutoPilot)
PACKET_LIST_ENTRY(MsgCapBits)
PACKET_LIST_ENTRY(MsgCaptureFlag)
PACKET_LIST_ENTRY(MsgCustomSound)
PACKET_LIST_ENTRY(MsgCacheURL)
PACKET_LIST_ENTRY(MsgDropFlag)
PACKET_LIST_ENTRY(MsgEnter)
PACKET_LIST_ENTRY(MsgExit)
PACKET_LIST_ENTRY(MsgFlagType)
PACKET_LIST_ENTRY(MsgFlagUpdate)
PACKET_LIST_ENTRY(MsgFetchResources)
PACKET_LIST_ENTRY(MsgForceState)
PACKET_LIST_ENTRY(MsgGrabFlag)
PACKET_LIST_ENTRY(MsgGMUpdate)
PACKET_LIST_ENTRY(MsgGetWorld)
PACKET_LIST_ENTRY(MsgGameSettings)
PACKET_LIST_ENTRY(MsgGameTime)
PACKET_LIST_ENTRY(MsgHandicap)
PACKET_LIST_ENTRY(MsgHit)
PACKET_LIST_ENTRY(MsgJoinServer)
PACKET_LIST_ENTRY(MsgKilled)
PACKET_LIST_ENTRY(MsgLagState)
PACKET_LIST_ENTRY(MsgLimboMessage)
PACKET_LIST_ENTRY(MsgMessage)
PACKET_LIST_ENTRY(MsgNewPlayer)
PACKET_LIST_ENTRY(MsgNearFlag)
PACKET_LIST_ENTRY(MsgNewRabbit)
PACKET_LIST_ENTRY(MsgNegotiateFlags)
PACKET_LIST_ENTRY(MsgPause)
PACKET_LIST_ENTRY(MsgPlayerData)
PACKET_LIST_ENTRY(MsgPlayerInfo)
PACKET_LIST_ENTRY(MsgPlayerUpdate)
PACKET_LIST_ENTRY(MsgPlayerUpdateSmall)
PACKET_LIST_ENTRY(MsgQueryGame)
PACKET_LIST_ENTRY(MsgQueryPlayers)
PACKET_LIST_ENTRY(MsgReject)
PACKET_LIST_ENTRY(MsgRemovePlayer)
PACKET_LIST_ENTRY(MsgReplayReset)
PACKET_LIST_ENTRY(MsgShotBegin)
PACKET_LIST_ENTRY(MsgWShotBegin)
PACKET_LIST_ENTRY(MsgWhatTimeIsIt)
PACKET_LIST_ENTRY(MsgScore)
PACKET_LIST_ENTRY(MsgScoreOver)
PACKET_LIST_ENTRY(MsgShotEnd)
PACKET_LIST_ENTRY(MsgSuperKill)
PACKET_LIST_ENTRY(MsgSetShot)
PACKET_LIST_ENTRY(MsgSetTeam)
PACKET_LIST_ENTRY(MsgSetVar)
PACKET_LIST_ENTRY(MsgTangibilityUpdate)
PACKET_LIST_ENTRY(MsgTangibilityReset)
PACKET_LIST_ENTRY(MsgTimeUpdate)
PACKET_LIST_ENTRY(MsgTeleport)
PACKET_LIST_ENTRY(MsgTransferFlag)
PACKET_LIST_ENTRY(MsgTeamUpdate)
PACKET_LIST_ENTRY(MsgWantWHash)
PACKET_LIST_ENTRY(MsgWantSettings)
PACKET_LIST_ENTRY(MsgUDPLinkRequest)
PACKET_LIST_ENTRY(MsgUDPLinkEstablished)
PACKET_LIST_ENTRY(MsgServerControl)
PACKET_LIST_ENTRY(MsgLagPing)
PACKET_LIST_ENTRY(MsgPingCodeReply)
PACKET_LIST_ENTRY(MsgPingCodeRequest)
PACKET_LIST_ENTRY(MsgEchoRequest)
PACKET_LIST_ENTRY(MsgEchoResponse)


//============================================================================//

void MsgStrings::init()
{
  MsgStrings::reset();
  return;
}


void MsgStrings::reset()
{
  unsigned int i;

  //  UseDNS = true;
  useColor = true;
  useStateTracking = true;

  flagList.clear();
  playerList.clear();

  // setup default player names
  playerList[ServerPlayer] = "SERVER";
  playerList[AllPlayers]   = "ALL";
  playerList[AdminPlayers] = "ADMIN";
  playerList[NoPlayer]     = "NOPLAYER";
  for (i = 244 ; i <= 250; i++) {
    playerList[i] = Team::getName(TeamColor(250 - i));
  }

  // set default DB entries -- FIXME -- this does not belong here?
  for (i = 0; i < numGlobalDBItems; ++i) {
    assert(globalDBItems[i].name != NULL);
    if (globalDBItems[i].value != NULL) {
      BZDB.set(globalDBItems[i].name, globalDBItems[i].value);
      BZDB.setDefault(globalDBItems[i].name, globalDBItems[i].value);
    }
    BZDB.setPersistent(globalDBItems[i].name, globalDBItems[i].persistent);
    BZDB.setPermission(globalDBItems[i].name, globalDBItems[i].permission);
  }

  return;
}


void MsgStrings::useDNS (bool /*value*/)
{
  //  UseDNS = value;
  return;
}

void MsgStrings::colorize(bool value)
{
  useColor = value;
  return;
}


void MsgStrings::trackState(bool value)
{
  useStateTracking = value;
  return;
}


int MsgStrings::knownPacketTypes()
{
  return packetCodeMap.size() - 1; // MsgNull doesn't count
}


const char* MsgStrings::strMsgCode(uint16_t code)
{
  PacketCodeMap::const_iterator it = packetCodeMap.find(code);
  if (it == packetCodeMap.end()) {
    static char buf[32];
    sprintf(buf, "MsgUnknown(0x%04X)", code);
    return buf;
  }
  const PacketListEntry* entry = it->second;
  return entry->label;
}


MsgStringList MsgStrings::msgFromServer(uint16_t len,
                                        uint16_t code,
                                        const void *data)
{
  PacketCodeMap::const_iterator it = packetCodeMap.find(code);
  if (it == packetCodeMap.end()) {
    MsgStringList badcode;
    const char* c = (const char*) &code;
    listPush(badcode, 0, "Unknown message code: 0x%04X <%c%c>\n",
                         code, c[1], c[0]);
    return badcode;
  }
  const PacketListEntry* entry = it->second;
  return entry->handler(PacketInfo(len, code, data));
}


MsgStringList MsgStrings::msgFromClient(uint16_t len,
                                        uint16_t code,
                                        const void *data)
{
  PacketCodeMap::const_iterator it = packetCodeMap.find(code);
  if (it == packetCodeMap.end()) {
    MsgStringList badcode;
    const char* c = (const char*) &code;
    listPush(badcode, 0, "Unknown message code: 0x%04X <%c%c>\n",
                         code, c[1], c[0]);
    badcode[0].text += "  << client to server messages unimplemented >>";
    return badcode;
  }
  //const PacketListEntry* entry = it->second;
  MsgStringList list = listMsgBasics(PacketInfo(len, code, data));
  list[0].text += "  << client to server messages unimplemented >>";
  return list;
}


//============================================================================//

static void listPush(MsgStringList &list, int level, const char *fmt, ...)
{
//  std::string str;
  MsgString mstr = { level, "", "" };
  char buffer[256];
  va_list args;

  if (!fmt)
    return;

  va_start(args, fmt);
  vsnprintf(buffer, 256, fmt, args);
  va_end(args);
  mstr.text += buffer;
  if (useColor) {
    switch (level) {
      case 0: { mstr.color = ANSI_STR_FG_MAGENTA; break; }
      case 1: { mstr.color = ANSI_STR_FG_BLUE;    break; }
      case 2: { mstr.color = ANSI_STR_FG_CYAN;    break; }
      case 3: { mstr.color = ANSI_STR_FG_GREEN;   break; }
    }
  }

  list.push_back(mstr);
  return;
}


static MsgStringList listMsgBasics(const PacketInfo& pi)
{
  MsgStringList list;
  // also include the length and code uint16_t's for the total length
  listPush(list, 0, "%s <%i>", MsgStrings::strMsgCode(pi.code), pi.len + 4);
  return list;
}


static std::string strFlag(uint16_t id)
{
  FlagList::iterator it = flagList.find(id);
  std::string name = "Invalid";
  if (it != flagList.end()) {
    name = (*it).second;
  }
  return TextUtils::format("%-2s [%i]", name.c_str(), id);
}


static std::string strFlagStatus(FlagStatus status)
{
  std::string str;
#define STRING_CASE(x)  case (x): str = #x; break
  switch (status) {
    STRING_CASE (FlagNoExist);
    STRING_CASE (FlagOnGround);
    STRING_CASE (FlagOnTank);
    STRING_CASE (FlagInAir);
    STRING_CASE (FlagComing);
    STRING_CASE (FlagGoing);
    default:
      str = TextUtils::format("UNKNOWN: 0x04%X", status);
      break;
  }
  return str;
}

static std::string strTeam(uint16_t id)
{
  return std::string(Team::getName((TeamColor) id));
}


static std::string strPlayer(uint16_t id)
{
  PlayerList::iterator it = playerList.find(id);
  std::string name;
  if (it != playerList.end()) {
    name = (*it).second;
  } else {
    name = "UnTracked";
  }
  return TextUtils::format("%s [%i]", name.c_str(), id);
}


static std::string strVector(const fvec3& vector)
{
  return "(" + vector.tostring(NULL, ", ") + ")";
}


static std::string strAddress(Address& address)
{
  return address.getDotNotation();
}


static std::string strKillReason(int16_t reason)
{
  switch (reason) {
    case 0: { return std::string("blowed up");     }
    case 1: { return std::string("shot");          }
    case 2: { return std::string("runover");       }
    case 3: { return std::string("captured");      }
    case 4: { return std::string("genocide");      }
    case 5: { return std::string("self destruct"); }
    default: {
      return TextUtils::format("unknown: %i", reason);
    }
  }
}

//============================================================================//

static MsgStringList handleMsgNull(const PacketInfo& pi)
{
  // not recorded, never seen?
  MsgStringList list = listMsgBasics(pi);
  return list;
}


static MsgStringList handleMsgAccept(const PacketInfo& pi)
{
  // not recorded
  MsgStringList list = listMsgBasics(pi);
  return list;
}


static MsgStringList handleMsgAdminInfo(const PacketInfo& pi)
{
  MsgStringList list = listMsgBasics(pi);

  void *d = (void*)pi.data;
  uint8_t count, ipsize, player;
  Address address;
  uint16_t i;
  d = nboUnpackUInt8(d, count);
  listPush(list, 1, "count: %i", count);
  for (i=0 ; i < count; i++) {
    d = nboUnpackUInt8(d, ipsize);
    d = nboUnpackUInt8(d, player);
    d = address.unpack(d);

    listPush(list, 2, "player:     %s", strPlayer(player).c_str());
    listPush(list, 2, "ipsize:     %i", ipsize);
    listPush(list, 2, "address:    %s", strAddress(address).c_str());
  }

  return list;
}


static MsgStringList handleMsgAlive(const PacketInfo& pi)
{
  MsgStringList list = listMsgBasics(pi);

  void *d = (void*)pi.data;
  uint8_t player;
  fvec3 pos;
  float azimuth;
  d = nboUnpackUInt8(d, player);
  d = nboUnpackFVec3(d, pos);
  d = nboUnpackFloat(d, azimuth);
  listPush(list, 1, "player: %s", strPlayer(player).c_str());
  listPush(list, 2, "pos:    %s", strVector(pos).c_str());
  listPush(list, 2, "angle:  %-8.3f = %8.3f deg",
	   azimuth, azimuth * (180.0f / M_PI));

  return list;
}


static MsgStringList handleMsgAllow(const PacketInfo& pi)
{
  // not recorded
  MsgStringList list = listMsgBasics(pi);
  return list;
}


static MsgStringList handleMsgAddPlayer(const PacketInfo& pi)
{
  MsgStringList list = listMsgBasics(pi);

  void *d = (void*)pi.data;
  uint8_t index;
  uint16_t type, team, wins, losses, tks;
  char callsign[CallSignLen];

  d = nboUnpackUInt8(d, index);
  d = nboUnpackUInt16(d, type);
  d = nboUnpackUInt16(d, team);
  d = nboUnpackUInt16(d, wins);
  d = nboUnpackUInt16(d, losses);
  d = nboUnpackUInt16(d, tks);
  d = nboUnpackString(d, callsign, CallSignLen);

  if (useStateTracking) {
    playerList[index] = callsign;
  }
  listPush(list, 1, "player: %s", strPlayer(index).c_str());
  listPush(list, 1, "team:   %s", strTeam(team).c_str());
  listPush(list, 1, "type:   %i", type);
  listPush(list, 2, "wins:   %i", wins);
  listPush(list, 2, "losses: %i", losses);
  listPush(list, 2, "tks:    %i", tks);

  return list;
}


static MsgStringList handleMsgAllowSpawn(const PacketInfo& pi)
{
  // not recorded
  MsgStringList list = listMsgBasics(pi);
  return list;
}


static MsgStringList handleMsgAutoPilot(const PacketInfo& pi)
{
  // not recorded
  MsgStringList list = listMsgBasics(pi);
  return list;
}


static MsgStringList handleMsgCapBits(const PacketInfo& pi)
{
  // not recorded
  MsgStringList list = listMsgBasics(pi);
  return list;
}


static MsgStringList handleMsgCaptureFlag(const PacketInfo& pi)
{
  MsgStringList list = listMsgBasics(pi);

  void *d = (void*)pi.data;
  uint16_t team;
  d = nboUnpackUInt16(d, team);
  listPush(list, 1, "team: %s", strTeam(team).c_str());

  return list;
}


static MsgStringList handleMsgCustomSound(const PacketInfo& pi)
{
  // not recorded
  MsgStringList list = listMsgBasics(pi);
  return list;
}


static MsgStringList handleMsgCacheURL(const PacketInfo& pi)
{
  // not recorded
  MsgStringList list = listMsgBasics(pi);
  return list;
}


static MsgStringList handleMsgDropFlag(const PacketInfo& pi)
{
  MsgStringList list = listMsgBasics(pi);

  void *d = (void*)pi.data;
  Flag flag;
  uint8_t player;
  uint16_t flagid;
  d = nboUnpackUInt8(d, player);
  d = nboUnpackUInt16(d, flagid);
  d = flag.unpack(d);
  listPush(list, 1, "player: %s", strPlayer(player).c_str());
  listPush(list, 1, "flag: %s", strFlag(flagid).c_str());

  return list;
}


static MsgStringList handleMsgEnter(const PacketInfo& pi)
{
  // not recorded
  MsgStringList list = listMsgBasics(pi);
  return list;
}


static MsgStringList handleMsgExit(const PacketInfo& pi)
{
  // not recorded, but bzfs will send a MsgRemovePlayer
  MsgStringList list = listMsgBasics(pi);
  return list;
}


static MsgStringList handleMsgFlagType(const PacketInfo& pi)
{
  // not recorded
  MsgStringList list = listMsgBasics(pi);
  return list;
}


static MsgStringList handleMsgFlagUpdate(const PacketInfo& pi)
{
  MsgStringList list = listMsgBasics(pi);

  void *d = (void*)pi.data;
  uint16_t count, index;
  int i;
  d = nboUnpackUInt16(d, count);
  listPush(list, 1, "count: %i", count);
  for (i = 0; i < (int) count; i++) {
    Flag flag;
    d = nboUnpackUInt16(d, index);
    d = flag.unpack(d);
    if (useStateTracking) {
      flagList[index] = flag.type->flagAbbv;
    }
    listPush(list, 2, "flag: %s", strFlag(index).c_str());
    listPush(list, 3, "owner:  %s", strPlayer(flag.owner).c_str());
    listPush(list, 3, "pos:    %s", strVector(flag.position).c_str());
    listPush(list, 3, "status: %s", strFlagStatus(flag.status).c_str());
  }

  return list;
}


static MsgStringList handleMsgFetchResources(const PacketInfo& pi)
{
  // not recorded
  MsgStringList list = listMsgBasics(pi);
  return list;
}


static MsgStringList handleMsgForceState(const PacketInfo& pi)
{
  MsgStringList list = listMsgBasics(pi);

  void *d = (void*)pi.data;
  uint8_t player;
  uint8_t bits;
  fvec3 tmpfvec3;
  float tmpfloat;
  d = nboUnpackUInt8(d, player);
  d = nboUnpackUInt8(d, bits);
  if ((bits & ForceStatePosBit) != 0) {
    d = nboUnpackFVec3(d, tmpfvec3);
    listPush(list, 1, "pos:    %s", strVector(tmpfvec3).c_str());
  }
  if ((bits & ForceStateVelBit) != 0) {
    d = nboUnpackFVec3(d, tmpfvec3);
    listPush(list, 1, "vel:    %s", strVector(tmpfvec3).c_str());
  }
  if ((bits & ForceStateAngleBit) != 0) {
    d = nboUnpackFloat(d, tmpfloat);
    listPush(list, 1, "angle:  %g", tmpfloat);
  }
  if ((bits & ForceStateAngVelBit) != 0) {
    d = nboUnpackFloat(d, tmpfloat);
    listPush(list, 1, "angvel: %g", tmpfloat);
  }
  
  return list;
}


static MsgStringList handleMsgGrabFlag(const PacketInfo& pi)
{
  MsgStringList list = listMsgBasics(pi);

  void *d = (void*)pi.data;
  Flag flag;
  uint8_t player;
  uint16_t flagid;
  d = nboUnpackUInt8(d, player);
  d = nboUnpackUInt16(d, flagid);
  d = flag.unpack(d);
  listPush(list, 1, "player: %s", strPlayer(player).c_str());
  listPush(list, 1, "flag: %s", strFlag(flagid).c_str());

  return list;
}


static MsgStringList handleMsgGMUpdate(const PacketInfo& pi)
{
  MsgStringList list = listMsgBasics(pi);

  void *d = (void*)pi.data;
  uint8_t target;
  ShotUpdate shot;
  d = shot.unpack(d);
  d = nboUnpackUInt8(d, target);
  listPush(list, 1, "player:   %s", strPlayer(shot.player).c_str());
  listPush(list, 1, "target:   %s", strPlayer(target).c_str());
  listPush(list, 2, "id:       %i", shot.id);
  listPush(list, 2, "team:     %s", strTeam(shot.team).c_str());
  listPush(list, 2, "pos:      %s", strVector(shot.pos).c_str());
  listPush(list, 2, "vel:      %s", strVector(shot.vel).c_str());

  return list;
}


static MsgStringList handleMsgGetWorld(const PacketInfo& pi)
{
  // not recorded
  MsgStringList list = listMsgBasics(pi);
  return list;
}


static MsgStringList handleMsgGameSettings(const PacketInfo& pi)
{
  // not recorded
  MsgStringList list = listMsgBasics(pi);
  return list;
}


static MsgStringList handleMsgGameTime(const PacketInfo& pi)
{
  MsgStringList list = listMsgBasics(pi);

  void *d = (void*)pi.data;
  double netTime;
  float halfLag;
  d = nboUnpackDouble(d, netTime);
  d = nboUnpackFloat(d, halfLag);
  listPush(list, 1, "netTime: %f", netTime);
  listPush(list, 1, "halfLag: %f", halfLag);

  return list;
}


static MsgStringList handleMsgHandicap(const PacketInfo& pi)
{
  // not recorded
  MsgStringList list = listMsgBasics(pi);
  return list;
}


static MsgStringList handleMsgHit(const PacketInfo& pi)
{
  // not recorded
  MsgStringList list = listMsgBasics(pi);
  return list;
}


static MsgStringList handleMsgKilled(const PacketInfo& pi)
{
  MsgStringList list = listMsgBasics(pi);

  void *d = (void*)pi.data;
  uint8_t victim, killer;
  int16_t reason, shot;
  FlagType* flagType;
  int32_t phydrv;
  d = nboUnpackUInt8(d, victim);
  d = nboUnpackUInt8(d, killer);
  d = nboUnpackInt16(d, reason);
  d = nboUnpackInt16(d, shot);
  d = FlagType::unpack(d, flagType);
  if (reason == PhysicsDriverDeath) {
    d = nboUnpackInt32(d, phydrv);
  }
  listPush(list, 1, "victim: %s", strPlayer(victim).c_str());
  listPush(list, 1, "killer: %s", strPlayer(killer).c_str());
  listPush(list, 1, "reason: %s", strKillReason(reason).c_str());
  listPush(list, 1, "shotid: %i", shot);

  if (flagType != Flags::Null) {
    listPush(list, 1, "flag:   %s", flagType->flagAbbv.c_str());
  } else {
    listPush(list, 1, "flag:   Null");
  }
  if (reason == PhysicsDriverDeath) {
    listPush(list, 1, "phydrv: %i", phydrv);
  }

  return list;
}


static MsgStringList handleMsgJoinServer(const PacketInfo& pi)
{
  MsgStringList list = listMsgBasics(pi);

  void *d = (void*)pi.data;

  std::string addr;
  int32_t port;
  int32_t team;
  std::string referrer;
  std::string message;

  d = nboUnpackStdString(d, addr);
  d = nboUnpackInt32(d, port);
  d = nboUnpackInt32(d, team);
  d = nboUnpackStdString(d, referrer);
  d = nboUnpackStdString(d, message);

  listPush(list, 1, "addr: \"%s\"", addr.c_str());
  listPush(list, 1, "port: %i", port);
  listPush(list, 1, "team: %i", team);
  listPush(list, 1, "referrer: \"%s\"", referrer.c_str());
  listPush(list, 1, "message: \"%s\"", message.c_str());

  return list;
}


static MsgStringList handleMsgLagState(const PacketInfo& pi)
{
  // not recorded
  MsgStringList list = listMsgBasics(pi);
  return list;
}


static MsgStringList handleMsgLimboMessage(const PacketInfo& pi)
{
  // not recorded
  MsgStringList list = listMsgBasics(pi);
  return list;
}


static MsgStringList handleMsgMessage(const PacketInfo& pi)
{
  MsgStringList list = listMsgBasics(pi);

  void *d = (void*)pi.data;
  uint8_t src, dst;
  d = nboUnpackUInt8(d, src);
  d = nboUnpackUInt8(d, dst);
  listPush(list, 1, "src: %s", strPlayer(src).c_str());
  listPush(list, 1, "dst: %s", strPlayer(dst).c_str());
  listPush(list, 1, "message: \"%s\"", (char*) d);

  return list;
}


static MsgStringList handleMsgNewPlayer(const PacketInfo& pi)
{
  // not recorded
  MsgStringList list = listMsgBasics(pi);
  return list;
}


static MsgStringList handleMsgNearFlag(const PacketInfo& pi)
{
  // not recorded
  MsgStringList list = listMsgBasics(pi);
  return list;
}


static MsgStringList handleMsgNewRabbit(const PacketInfo& pi)
{
  MsgStringList list = listMsgBasics(pi);

  void *d = (void*)pi.data;
  uint8_t player, paused;
  d = nboUnpackUInt8(d, player);
  d = nboUnpackUInt8(d, paused);

  return list;
}


static MsgStringList handleMsgNegotiateFlags(const PacketInfo& pi)
{
  // not recorded
  MsgStringList list = listMsgBasics(pi);
  return list;
}


static MsgStringList handleMsgPause(const PacketInfo& pi)
{
  MsgStringList list = listMsgBasics(pi);

  void *d = (void*)pi.data;
  uint8_t player, paused;
  d = nboUnpackUInt8(d, player);
  d = nboUnpackUInt8(d, paused);
  listPush(list, 1, "player: %s", strPlayer(player).c_str());
  listPush(list, 1, "paused: %i", paused);

  return list;
}


static MsgStringList handleMsgPlayerData(const PacketInfo& pi)
{
  MsgStringList list = listMsgBasics(pi);

  void *d = (void*)pi.data;
  uint8_t player;
  std::string key, value;
  d = nboUnpackUInt8(d, player);
  d = nboUnpackStdString(d, key);
  d = nboUnpackStdString(d, value);
  listPush(list, 1, "player: %s", strPlayer(player).c_str());
  listPush(list, 2, "key:    %s", key.c_str());
  listPush(list, 2, "value:  %s", value.c_str());

  return list;
}


static MsgStringList handleMsgPlayerInfo(const PacketInfo& pi)
{
  MsgStringList list = listMsgBasics(pi);

  void *d = (void*)pi.data;
  uint8_t count, player, properties;
  Address address;
  uint16_t i;
  d = nboUnpackUInt8(d, count);
  listPush(list, 1, "count: %i", count);
  for (i=0 ; i < count; i++) {
    d = nboUnpackUInt8(d, player);
    d = nboUnpackUInt8(d, properties);

    std::string props;
    if (properties & IsRegistered) {
      props += "Registered ";
    }
    if (properties & IsVerified) {
      props += "Verified ";
    }
    if (properties & IsAdmin) {
      props += "Admin ";
    }

    listPush(list, 2, "player:     %s", strPlayer(player).c_str());
    listPush(list, 2, "properties: %s(0x%02X)", props.c_str(), properties);
  }

  return list;
}


static MsgStringList handleMsgPlayerUpdate(const PacketInfo& pi)
{
  MsgStringList list = listMsgBasics(pi);

  void *d = (void*)pi.data;
  float timestamp;
  uint8_t index;
  PlayerState state;
  d = nboUnpackFloat(d, timestamp);
  d = nboUnpackUInt8(d, index);
  d = state.unpack(d, pi.code);

  listPush(list, 1, "player: %s", strPlayer(index).c_str());
  listPush(list, 2, "state: 0x%04X  order: %i", state.status, state.order);
  listPush(list, 3, "pos:    %s", strVector(state.pos).c_str());
  listPush(list, 3, "vel:    %s", strVector(state.velocity).c_str());
  listPush(list, 3, "angle:  %-8.3f = %8.3f deg",
	   state.azimuth, state.azimuth * (180.0f / M_PI));
  listPush(list, 3, "angvel: %-8.3f = %8.3f deg/sec",
	   state.angVel, state.angVel * (180.0f / M_PI));

  return list;
}


static MsgStringList handleMsgPlayerUpdateSmall(const PacketInfo& pi)
{
  // call the normal function while maintaining the same 'code' value
  return handleMsgPlayerUpdate(pi);
}


static MsgStringList handleMsgQueryGame(const PacketInfo& pi)
{
  // not recorded
  MsgStringList list = listMsgBasics(pi);
  return list;
}


static MsgStringList handleMsgQueryPlayers(const PacketInfo& pi)
{
  // not recorded
  MsgStringList list = listMsgBasics(pi);
  return list;
}


static MsgStringList handleMsgReject(const PacketInfo& pi)
{
  // not recorded
  MsgStringList list = listMsgBasics(pi);
  return list;
}


static MsgStringList handleMsgRemovePlayer(const PacketInfo& pi)
{
  MsgStringList list = listMsgBasics(pi);

  void *d = (void*)pi.data;
  uint8_t index;
  d = nboUnpackUInt8(d, index);
  listPush(list, 1, "player: %s", strPlayer(index).c_str());
  if (useStateTracking) {
    playerList.erase(index);
  }

  return list;
}


static MsgStringList handleMsgReplayReset(const PacketInfo& pi)
{
  // not recorded
  MsgStringList list = listMsgBasics(pi);
  return list;
}


static MsgStringList handleMsgShotBegin(const PacketInfo& pi)
{
  MsgStringList list = listMsgBasics(pi);

  void *d = (void*)pi.data;
  FiringInfo finfo;
  d = finfo.unpack(d);
  const ShotUpdate& shot = finfo.shot;
  listPush(list, 1, "player:   %s", strPlayer(shot.player).c_str());
  listPush(list, 2, "id:       %i", shot.id);
  listPush(list, 2, "timeSent: %f", finfo.timeSent);
  listPush(list, 3, "shotPlayer: %s", strPlayer(shot.player).c_str());
  listPush(list, 3, "shotID:     %i", shot.id);
  listPush(list, 2, "team:     %s", strTeam(shot.team).c_str());
  listPush(list, 2, "pos:      %s", strVector(shot.pos).c_str());
  listPush(list, 2, "vel:      %s", strVector(shot.vel).c_str());
  listPush(list, 1, "type:     %.2s", finfo.flagType->flagAbbv.c_str()); // FIXME ?
  listPush(list, 2, "lifetime: %-8.3f", finfo.lifetime);

  return list;
}


static MsgStringList handleMsgWShotBegin(const PacketInfo& pi)
{
  // not recorded
  MsgStringList list = listMsgBasics(pi);
  return list;
}


static MsgStringList handleMsgWhatTimeIsIt(const PacketInfo& pi)
{
  // not recorded
  MsgStringList list = listMsgBasics(pi);
  return list;
}


static MsgStringList handleMsgScore(const PacketInfo& pi)
{
  MsgStringList list = listMsgBasics(pi);

  void *d = (void*)pi.data;
  uint8_t count, player;
  uint16_t wins, losses, tks, i;
  d = nboUnpackUInt8(d, count);
  listPush(list, 1, "count: %i", count);
  for (i=0; i < count; i++) {
    d = nboUnpackUInt8(d, player);
    d = nboUnpackUInt16(d, wins);
    d = nboUnpackUInt16(d, losses);
    d = nboUnpackUInt16(d, tks);
    listPush(list, 2, "player: %s", strPlayer(player).c_str());
    listPush(list, 3, "wins: %i  losses: %i  tks: %i", wins, losses, tks);
  }

  return list;
}


static MsgStringList handleMsgScoreOver(const PacketInfo& pi)
{
  MsgStringList list = listMsgBasics(pi);

  void *d = (void*)pi.data;
  uint8_t player;
  uint16_t team;
  d = nboUnpackUInt8(d, player);
  d = nboUnpackUInt16(d, team);
  listPush(list, 1, "player: %s", strPlayer(player).c_str());
  listPush(list, 1, "team:   %i", strTeam(team).c_str());

  return list;
}


static MsgStringList handleMsgShotEnd(const PacketInfo& pi)
{
  MsgStringList list = listMsgBasics(pi);

  void *d = (void*)pi.data;
  uint8_t player;
  uint16_t shotid;
  int16_t reason;
  d = nboUnpackUInt8(d, player);
  d = nboUnpackUInt16(d, shotid);
  d = nboUnpackInt16(d, reason);
  listPush(list, 1, "player: %s", strPlayer(player).c_str());
  listPush(list, 1, "shotid: %i", shotid);
  listPush(list, 1, "reason: %i", reason); // FIXME

  return list;
}


static MsgStringList handleMsgSuperKill(const PacketInfo& pi)
{
  // not recorded
  MsgStringList list = listMsgBasics(pi);
  return list;
}


static MsgStringList handleMsgSetShot(const PacketInfo& pi)
{
  // not recorded
  MsgStringList list = listMsgBasics(pi);
  return list;
}


static MsgStringList handleMsgSetTeam(const PacketInfo& pi)
{
  // not recorded
  MsgStringList list = listMsgBasics(pi);
  return list;
}


static MsgStringList handleMsgSetVar(const PacketInfo& pi)
{
  MsgStringList list = listMsgBasics(pi);

  void *d = (void*)pi.data;
  uint16_t i;
  uint16_t count;
  d = nboUnpackUInt16(d, count);
  listPush(list, 1, "count: %i", count);

  std::string name;
  std::string value;
  for (i = 0; i < count; i++) {
    d = nboUnpackStdString(d, name);
    d = nboUnpackStdString(d, value);
    listPush(list, 2, "%-20s = \"%s\"", name.c_str(), value.c_str());

    if (useStateTracking) {
      BZDB.set(name, value, StateDatabase::Locked);
    }
  }

  return list;
}


static MsgStringList handleMsgTangibilityUpdate(const PacketInfo& pi)
{
  // not recorded
  MsgStringList list = listMsgBasics(pi);
  return list;
}


static MsgStringList handleMsgTangibilityReset(const PacketInfo& pi)
{
  // not recorded
  MsgStringList list = listMsgBasics(pi);
  return list;
}


static MsgStringList handleMsgTimeUpdate(const PacketInfo& pi)
{
  MsgStringList list = listMsgBasics(pi);
  void *d = (void*)pi.data;
  int32_t timeLeft;
  d = nboUnpackInt32(d, timeLeft);
  listPush(list, 1, "timeLeft: %i", timeLeft);

  return list;
}


static MsgStringList handleMsgTeleport(const PacketInfo& pi)
{
  MsgStringList list = listMsgBasics(pi);

  void *d = (void*)pi.data;
  uint8_t player;
  uint16_t to, from;
  d = nboUnpackUInt8(d, player);
  d = nboUnpackUInt16(d, from);
  d = nboUnpackUInt16(d, to);
  listPush(list, 1, "player: %s", strPlayer(player).c_str());
  listPush(list, 1, "from:   %i", from);
  listPush(list, 1, "to:     %i", to);

  return list;
}


static MsgStringList handleMsgTransferFlag(const PacketInfo& pi)
{
  MsgStringList list = listMsgBasics(pi);

  void *d = (void*)pi.data;
  uint8_t to, from;
  uint16_t flagid;
  Flag flag;
  d = nboUnpackUInt8(d, from);
  d = nboUnpackUInt8(d, to);
  d = nboUnpackUInt16(d, flagid);
  d = flag.unpack(d);
  listPush(list, 1, "from: %s", strPlayer(from).c_str());
  listPush(list, 1, "to:   %s", strPlayer(to).c_str());
  listPush(list, 1, "flag: %s", strFlag(flagid).c_str());

  return list;
}


static MsgStringList handleMsgTeamUpdate(const PacketInfo& pi)
{
  MsgStringList list = listMsgBasics(pi);

  void *d = (void*)pi.data;
  uint8_t count;
  uint16_t i, team, size, won, lost;
  d = nboUnpackUInt8(d, count);
  listPush(list, 1, "count: %i", count);
  for (i = 0; i < count; i++) {
    d = nboUnpackUInt16(d, team);
    d = nboUnpackUInt16(d, size);
    d = nboUnpackUInt16(d, won);
    d = nboUnpackUInt16(d, lost);
    listPush(list, 2, "team: %s", strTeam(team).c_str());
    listPush(list, 3, "size = %i, won = %i, lost = %i", size, won, lost);
  }

  return list;
}


static MsgStringList handleMsgWantWHash(const PacketInfo& pi)
{
  // not recorded
  MsgStringList list = listMsgBasics(pi);
  return list;
}


static MsgStringList handleMsgWantSettings(const PacketInfo& pi)
{
  // not recorded
  MsgStringList list = listMsgBasics(pi);
  return list;
}


static MsgStringList handleMsgUDPLinkRequest(const PacketInfo& pi)
{
  // not recorded
  MsgStringList list = listMsgBasics(pi);
  return list;
}


static MsgStringList handleMsgUDPLinkEstablished(const PacketInfo& pi)
{
  // not recorded
  MsgStringList list = listMsgBasics(pi);
  return list;
}


static MsgStringList handleMsgServerControl(const PacketInfo& pi)
{
  // packet type hasn't not been implemented
  MsgStringList list = listMsgBasics(pi);
  return list;
}


static MsgStringList handleMsgLagPing(const PacketInfo& pi)
{
  MsgStringList list = listMsgBasics(pi);

  void *d = (void*)pi.data;
  uint16_t seqno;
  d = nboUnpackUInt16(d, seqno);
  listPush(list, 1, "seqno: %u", seqno);

  return list;
}

static MsgStringList handleMsgPingCodeReply(const PacketInfo& pi)
{
  // not recorded
  MsgStringList list = listMsgBasics(pi);
  return list;
}

static MsgStringList handleMsgPingCodeRequest(const PacketInfo& pi)
{
  // not recorded
  MsgStringList list = listMsgBasics(pi);
  return list;
}

static MsgStringList handleMsgEchoRequest(const PacketInfo& pi)
{
  // not recorded
  MsgStringList list = listMsgBasics(pi);
  return list;
}

static MsgStringList handleMsgEchoResponse(const PacketInfo& pi)
{
  // not recorded
  MsgStringList list = listMsgBasics(pi);
  return list;
}



//============================================================================//

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
