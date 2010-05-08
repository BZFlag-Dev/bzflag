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

#include "common.h"

// interface header
#include "bzfsMessages.h"

// system headers
#include <assert.h>

// common headers
#include "NetMessage.h"
#include "PlayerState.h"


void flagToAPIFlag(FlagInfo &flag, bz_FlagUpdateRecord *flagRecord)
{
  bool hide = (flag.flag.type->flagTeam == ::NoTeam) && (flag.player == -1);

  flagRecord->index = flag.getIndex();

  if (hide) {
    flagRecord->type[0] = 'P';
    flagRecord->type[1] = 'Z';
  } else {
    strncpy(flagRecord->type,flag.flag.type->flagAbbv.c_str(),2);
  }

  flagRecord->status             = flag.flag.status;
  flagRecord->endurance          = flag.flag.endurance;
  flagRecord->owner              = flag.flag.owner;
  flagRecord->position[0]        = flag.flag.position.x;
  flagRecord->position[1]        = flag.flag.position.y;
  flagRecord->position[2]        = flag.flag.position.z;
  flagRecord->launchPosition[0]  = flag.flag.launchPosition.x;
  flagRecord->launchPosition[1]  = flag.flag.launchPosition.y;
  flagRecord->launchPosition[2]  = flag.flag.launchPosition.z;
  flagRecord->landingPosition[0] = flag.flag.landingPosition.x;
  flagRecord->landingPosition[1] = flag.flag.landingPosition.y;
  flagRecord->landingPosition[2] = flag.flag.landingPosition.z;
  flagRecord->flightTime         = flag.flag.flightTime;
  flagRecord->flightEnd          = flag.flag.flightEnd;
  flagRecord->initialVelocity    = flag.flag.initialVelocity;
}


void sendRemovePlayerMessage(int playerID)
{
  NetMessage netMsg;
  netMsg.packUInt8(playerID);
  netMsg.broadcast(MsgRemovePlayer);

  for (int i = 0; i < curMaxPlayers; i++) {
    GameKeeper::Player *p = GameKeeper::Player::getPlayerByIndex(i);
    if ((p == NULL) || !p->playerHandler || playerID == p->getIndex())
      continue;
    p->playerHandler->playerRemoved(playerID);
  }
}


void sendFlagUpdateMessage(int playerID)
{
  GameKeeper::Player *playerData = GameKeeper::Player::getPlayerByIndex(playerID);
  if (!playerData)
    return;

  std::vector<bz_FlagUpdateRecord*> flagRecordList;
  if (playerData->playerHandler)
  {
    for (int flagIndex = 0; flagIndex < numFlags; flagIndex++)
    {
      FlagInfo &flag = *FlagInfo::get(flagIndex);
      if (flag.exist())
      {
	bz_FlagUpdateRecord *flagRecord = new bz_FlagUpdateRecord;
	flagToAPIFlag(flag, flagRecord);
	flagRecordList.push_back(flagRecord);
      }
    }

    bz_FlagUpdateRecord** flagHandle = (bz_FlagUpdateRecord**) malloc(sizeof(bz_FlagUpdateRecord*) * flagRecordList.size());
    for (unsigned int i = 0; i < flagRecordList.size(); i++)
      flagHandle[i] = flagRecordList[i];

    playerData->playerHandler->flagUpdate((int)flagRecordList.size(), flagHandle);

    free(flagHandle);
    for (unsigned int i = 0; i < flagRecordList.size(); i++)
      delete(flagRecordList[i]);
  }
  else
  {
    // first find all the flags we will send and get there pointers
    std::vector<FlagInfo*> flagsToSend;

    for (int flagIndex = 0; flagIndex < numFlags; flagIndex++)
    {
      FlagInfo *info = FlagInfo::get(flagIndex);
      if (info && info->exist() && info->getIndex() < 0xFFFF)
	flagsToSend.push_back(info);
    }

    // now send 10 flags at at time so we dont' flood out the guys and send ALL the flags in a nice buffered message
    size_t flagsSent = 0;

    while (flagsSent < flagsToSend.size()) {
       // do 10 at a time
      size_t flagsToDo = 10;
      if ((flagsSent + flagsToDo) >= flagsToSend.size()) {
        flagsToDo = flagsToSend.size() - flagsSent;
      }

      NetMessage netMsg;
      netMsg.packUInt16((uint16_t)flagsToDo);

      for (size_t i = 0; i < flagsToDo; i++) {
        FlagInfo &flag = *flagsToSend[flagsSent+i];
        bool hide = (flag.flag.type->flagTeam == ::NoTeam) && (flag.player == -1);
        flag.pack(netMsg, hide);
      }
      netMsg.send(playerData->netHandler, MsgFlagUpdate);

      flagsSent += flagsToDo;
    }
  }
}


void sendFlagUpdateMessage(FlagInfo &flag)
{
  NetMessage netMsg;
  netMsg.packUInt16(1);

  bool hide = (flag.flag.type->flagTeam == ::NoTeam) && (flag.player == -1);
  flag.pack(netMsg, hide);

  netMsg.broadcast(MsgFlagUpdate, false);
}


void sendExistingPlayerUpdates(int newPlayer)
{
  GameKeeper::Player *playerData = GameKeeper::Player::getPlayerByIndex(newPlayer);

  if (!playerData)
    return;

  GameKeeper::Player *otherData;
  for (int otherIndex = 0; otherIndex < curMaxPlayers; otherIndex++) {
    if (otherIndex == newPlayer) {
      continue;
    }

    otherData = GameKeeper::Player::getPlayerByIndex(otherIndex);

    if (!otherData || !otherData->player.isPlaying()) {
      continue;
    }

    if (!playerData->playerHandler) {
      if (sendPlayerUpdateDirect(playerData->netHandler, otherData) < 0) {
	break;
      }
      if (otherData->player.isPaused()) {
        NetMessage netMsg;
        netMsg.packUInt8(otherData->getIndex());
        netMsg.packUInt8(PauseCodeEnable);
        netMsg.send(playerData->netHandler, MsgPause);
      }
    }
    else { // a server-side player
      bz_PlayerInfoUpdateRecord	playerRecord;

      playerRecord.index = otherIndex;
      playerRecord.type = (bz_ePlayerType)otherData->player.getType();
      playerRecord.team = convertTeam(otherData->player.getTeam());
      playerRecord.score.rank   = otherData->score.ranking();
      playerRecord.score.wins   = otherData->score.getWins();
      playerRecord.score.losses = otherData->score.getLosses();
      playerRecord.score.tks    = otherData->score.getTKs();
      memset(playerRecord.callsign, 0, 32);
      strncpy(playerRecord.callsign, otherData->player.getCallSign(), 31);

      playerData->playerHandler->playerInfoUpdate(&playerRecord);
    }

    sendMessageAllow(newPlayer, otherIndex, otherData->player.getAllow());

    // custom data
    sendPlayerCustomDataMap(playerData->netHandler, otherData);
  }
}


bool sendTeamUpdateMessage(int newPlayer)
{
  GameKeeper::Player *playerData = GameKeeper::Player::getPlayerByIndex(newPlayer);

  if (!playerData)
    return false;

  if (!playerData->playerHandler) {
    return sendTeamUpdateDirect(playerData->netHandler) != 0;
  } else {
    bz_TeamInfoRecord **teams = (bz_TeamInfoRecord**)malloc(sizeof(bz_TeamInfoRecord*) * CtfTeams);
    for (int t = 0; t < CtfTeams; t++) {
      teams[t] = new bz_TeamInfoRecord;

      teams[t]->id = t;
      teams[t]->size   = teamInfos[t].team.size;
      teams[t]->wins   = teamInfos[t].team.won;
      teams[t]->losses = teamInfos[t].team.won;
    }
    playerData->playerHandler->teamUpdate(CtfTeams, teams);

    for (int t = 0; t < CtfTeams; t++)
      delete(teams[t]);

    free(teams);
  }
  return true;
}


void sendTeamUpdateMessageBroadcast(int teamIndex1, int teamIndex2)
{
  if (clOptions->gameType == OpenFFA)
    return;

  // If teamIndex1 is -1, send all teams
  // If teamIndex2 is -1, just send teamIndex1 team
  // else send both teamIndex1 and teamIndex2 teams

  NetMessage netMsg;

  if (teamIndex1 == -1) {
    netMsg.packUInt8(CtfTeams);
    for (int t = 0; t < CtfTeams; t++) {
      netMsg.packUInt16(t);
      teamInfos[t].team.pack(netMsg);
    }
  } else if (teamIndex2 == -1) {
    netMsg.packUInt8(1);
    netMsg.packUInt16(teamIndex1);
    teamInfos[teamIndex1].team.pack(netMsg);
  } else {
    netMsg.packUInt8(2);
    netMsg.packUInt16(teamIndex1);
    teamInfos[teamIndex1].team.pack(netMsg);
    netMsg.packUInt16(teamIndex2);
    teamInfos[teamIndex2].team.pack(netMsg);
  }

  netMsg.broadcast(MsgTeamUpdate,false);

  bz_TeamInfoRecord	**teams = NULL;

  int teamCount = 0;

  if (teamIndex1 == -1) {
    teamCount = CtfTeams;
    teams = (bz_TeamInfoRecord**)malloc(sizeof(bz_TeamInfoRecord*)*teamCount);
    for (int t = 0; t < CtfTeams; t++) {
      teams[t] = new bz_TeamInfoRecord;

      teams[t]->id = t;
      teams[t]->size   = teamInfos[t].team.size;
      teams[t]->wins   = teamInfos[t].team.won;
      teams[t]->losses = teamInfos[t].team.won;
    }
  } else if (teamIndex2 == -1) {
    teamCount = 1;
    teams = (bz_TeamInfoRecord**)malloc(sizeof(bz_TeamInfoRecord*)*teamCount);

    teams[0] = new bz_TeamInfoRecord;

    teams[0]->id = teamIndex1;
    teams[0]->size = teamInfos[teamIndex1].team.size;
    teams[0]->wins = teamInfos[teamIndex1].team.won;
  } else {
    teamCount = 2;
    teams = (bz_TeamInfoRecord**)malloc(sizeof(bz_TeamInfoRecord*)*teamCount);

    teams[0] = new bz_TeamInfoRecord;

    teams[0]->id = teamIndex1;
    teams[0]->size = teamInfos[teamIndex1].team.size;
    teams[0]->wins = teamInfos[teamIndex1].team.won;

    teams[1] = new bz_TeamInfoRecord;

    teams[1]->id = teamIndex2;
    teams[1]->size = teamInfos[teamIndex2].team.size;
    teams[1]->wins = teamInfos[teamIndex2].team.won;
  }

  // now do everyone who dosn't have network
  for (int i = 0; i < curMaxPlayers; i++) {
    GameKeeper::Player* otherData = GameKeeper::Player::getPlayerByIndex(i);
    if (otherData && otherData->playerHandler)
      otherData->playerHandler->teamUpdate(teamCount, teams);
  }

  for (int t = 0; t < teamCount; t++)
    delete(teams[t]);

  free(teams);
}


void sendRejectPlayerMessage(int playerID, uint16_t code , const char* reason)
{
  GameKeeper::Player *playerData = GameKeeper::Player::getPlayerByIndex(playerID);

  if (playerData->playerHandler) {
    playerData->playerHandler->playerRejected((bz_eRejectCodes)code,reason);
  } else {
    NetMessage netMsg;
    netMsg.packUInt16(code);
    netMsg.packString(reason, strlen(reason) + 1);
    netMsg.send(playerData->netHandler,MsgReject);
  }
}


bool sendAcceptPlayerMessage(int playerID)
{
  GameKeeper::Player *playerData = GameKeeper::Player::getPlayerByIndex(playerID);

  if (playerData->playerHandler) {
    playerData->playerHandler->playerAccepted();
  }
  else {
    NetMessage netMsg;
    netMsg.packUInt8(playerID);
    netMsg.send(playerData->netHandler, MsgAccept);
  }

  return true;
}


void sendHandicapInfoUpdate(int playerID)
{
  GameKeeper::Player *playerData
    = GameKeeper::Player::getPlayerByIndex(playerID);
  if (!playerData)
    return;

  GameKeeper::Player *otherData;
  if (clOptions->gameOptions & HandicapGameStyle) {
    if (playerData->playerHandler) {
      std::vector<bz_HandicapUpdateRecord*> handyList;
      for (int i = 0; i < curMaxPlayers; i++) {
	otherData = GameKeeper::Player::getPlayerByIndex(i);
	if (otherData) {
	  bz_HandicapUpdateRecord *handyData = new bz_HandicapUpdateRecord;
	  handyData->player   = i;
	  handyData->handicap = otherData->score.getHandicap();
	  handyList.push_back(handyData);
	}

	bz_HandicapUpdateRecord **handyPtrList
	  = (bz_HandicapUpdateRecord**)malloc(sizeof(bz_HandicapUpdateRecord*)
	  * handyList.size());
	for (int j = 0; j < (int)handyList.size(); j++)
	  handyPtrList[j] = handyList[j];

	playerData->playerHandler->handicapUpdate((int)handyList.size(),
	  handyPtrList);

	free(handyPtrList);
	for (int k = 0; k < (int)handyList.size(); k++)
	  delete(handyList[k]);
      }
    } else {
      int numHandicaps = 0;

      // Send handicap for all players
      NetMessage netMsg;

      for (int i = 0; i < curMaxPlayers; i++) {
	if (i != playerID) {
	  otherData = GameKeeper::Player::getPlayerByIndex(i);
	  if (otherData)
	    numHandicaps++;
	}
      }

      netMsg.packUInt8(numHandicaps);

      for (int i = 0; i < curMaxPlayers; i++) {
	if (i != playerID) {
	  otherData = GameKeeper::Player::getPlayerByIndex(i);
	  if (otherData) {
	    netMsg.packUInt8(i);
	    netMsg.packInt16(otherData->score.getHandicap());
	  }
	}
      }
      netMsg.broadcast(MsgHandicap);
    }
  }
}


void sendSingleHandicapInfoUpdate(GameKeeper::Player* playerData)
{
  if (!playerData)
    return;

  NetMessage netMsg;
  netMsg.packUInt8(1);
  netMsg.packUInt8(playerData->getIndex());
  netMsg.packInt16(playerData->score.getHandicap());
  netMsg.broadcast(MsgHandicap);

  bz_HandicapUpdateRecord *handyData = new bz_HandicapUpdateRecord;
  handyData->player = playerData->getIndex();
  handyData->handicap = playerData->score.getHandicap();

  // now do everyone who dosn't have network
  for (int i = 0; i < curMaxPlayers; i++) {
    GameKeeper::Player* otherData = GameKeeper::Player::getPlayerByIndex(i);
    if (otherData && otherData->playerHandler)
      otherData->playerHandler->handicapUpdate(1,&handyData);
  }
  delete(handyData);
}


void sendAdminInfoMessage(int aboutPlayer, int toPlayer, bool record)
{
  GameKeeper::Player *aboutPlayerData = NULL;
  GameKeeper::Player *toPlayerData = NULL;

  aboutPlayerData = GameKeeper::Player::getPlayerByIndex(aboutPlayer);
  if (!aboutPlayerData || !aboutPlayerData->netHandler)
    return;

  if (!record) {
    toPlayerData = GameKeeper::Player::getPlayerByIndex(toPlayer);
    if (!toPlayerData)
      return;

    if (toPlayerData->playerHandler) {
      if (!aboutPlayerData->netHandler) {
	toPlayerData->playerHandler->playerIPUpdate(aboutPlayer,"local.player");
      } else {
	toPlayerData->playerHandler->playerIPUpdate(
	  aboutPlayer, aboutPlayerData->netHandler->getTargetIP()
        );
      }
    }
  }

  NetMessage netMsg;

  if (toPlayerData || record) {
    netMsg.packUInt8(1);
    aboutPlayerData->packAdminInfo(netMsg);
  }

  if (toPlayerData) {
    if (!toPlayerData->playerHandler)
      netMsg.send(toPlayerData->netHandler,MsgAdminInfo);
  }

  if (record) {
    Record::addPacket(MsgAdminInfo, netMsg, HiddenPacket);
  }
}


void sendFlagTransferMessage (int toPlayer, int fromPlayer , FlagInfo &flag)
{
  NetMessage netMsg;
  netMsg.packUInt8(fromPlayer);
  netMsg.packUInt8(toPlayer);

  GameKeeper::Player *toData = GameKeeper::Player::getPlayerByIndex(toPlayer);
  GameKeeper::Player *fromData = GameKeeper::Player::getPlayerByIndex(fromPlayer);

  toData->effectiveShotType = fromData->effectiveShotType;
  fromData->effectiveShotType = StandardShot;
  flag.flag.owner = toPlayer;
  flag.player = toPlayer;
  toData->player.resetFlag();
  toData->player.setFlag(flag.getIndex());
  fromData->player.resetFlag();
  flag.pack(netMsg);
  netMsg.packUInt8(toData->effectiveShotType);

  netMsg.broadcast(MsgTransferFlag);

  // now do everyone who dosn't have network
  for (int i = 0; i < curMaxPlayers; i++) {
    GameKeeper::Player* otherData = GameKeeper::Player::getPlayerByIndex(i);
    if (otherData && otherData->playerHandler) {
      otherData->playerHandler->flagTransfer(
        fromData->getIndex(), toData->getIndex(),
        flag.getIndex(), (bz_eShotType) toData->effectiveShotType
      );
    }
  }
}


void sendClosestFlagMessage(int playerIndex,FlagType *type , const fvec3& pos)
{
  GameKeeper::Player* playerData = GameKeeper::Player::getPlayerByIndex(playerIndex);

  if (!type || !playerData) {
    return;
  }

  if (playerData->playerHandler) {
    playerData->playerHandler->nearestFlag(type->flagName.c_str(), pos);
  }
  else {
    NetMessage netMsg;
    netMsg.packFVec3(pos);
    netMsg.packStdString(std::string(type->flagName));
    netMsg.send(playerData->netHandler, MsgNearFlag);
  }
}


bool sendGrabFlagMessage (int playerIndex, FlagInfo &flag)
{
  GameKeeper::Player *playerData = GameKeeper::Player::getPlayerByIndex(playerIndex);
  if (!playerData) {
    return false;
  }

  bz_AllowFlagGrabEventData_V1	allow;
  allow.flagID = flag.getIndex();
  allow.flagType = flag.flag.type->flagAbbv.c_str();
  allow.shotType = (bz_eShotType)flag.flag.type->flagShot;
  allow.playerID = playerIndex;
  allow.allow = true;

  worldEventManager.callEvents(bz_eAllowFlagGrabEvent,&allow);

  if (!allow.allow) {
    return false;
  }

  flag.grab(playerIndex);
  playerData->player.setFlag(flag.getIndex());

  // send MsgGrabFlag
  NetMessage netMsg;

  netMsg.packUInt8(playerIndex);
  flag.pack(netMsg);

  bz_FlagGrabbedEventData_V1	data;
  data.flagID = flag.getIndex();
  data.flagType = flag.flag.type->flagAbbv.c_str();
  data.shotType = (bz_eShotType)flag.flag.type->flagShot;
  data.playerID = playerIndex;

  worldEventManager.callEvents(bz_eFlagGrabbedEvent,&data);

  // pack in the shot type, it may have been modified
  netMsg.packUInt8(data.shotType);
  playerData->effectiveShotType = (ShotType)data.shotType;

  netMsg.broadcast(MsgGrabFlag);

  // now do everyone who doesn't have network
  for (int i = 0; i < curMaxPlayers; i++) {
    GameKeeper::Player* otherData = GameKeeper::Player::getPlayerByIndex(i);
    if (otherData && otherData->playerHandler)
      otherData->playerHandler->grabFlag(playerIndex,flag.getIndex(),flag.flag.type->flagAbbv.c_str(),(bz_eShotType)playerData->effectiveShotType);
  }

  playerData->flagHistory.add(flag.flag.type);
  return true;
}


void sendSetShotType(int playerIndex, ShotType type)
{
  GameKeeper::Player *playerData = GameKeeper::Player::getPlayerByIndex(playerIndex);
  if (!playerData)
    return;

  if (type == playerData->effectiveShotType)
    return; // it's the same as what they have

  playerData->effectiveShotType = type;

  NetMessage netMsg;
  netMsg.packUInt8(playerIndex);
  netMsg.packUInt8(type);
  netMsg.broadcast(MsgSetShot);

  // now do everyone who dosn't have network
  for (int i = 0; i < curMaxPlayers; i++) {
    GameKeeper::Player* otherData = GameKeeper::Player::getPlayerByIndex(i);
    if (otherData && otherData->playerHandler)
      otherData->playerHandler->setShotType(playerIndex,(bz_eShotType)playerData->effectiveShotType);
  }
}


void sendMsgShotBegin(int player, unsigned short id, FiringInfo &firingInfo)
{
  NetMessage netMsg;
  netMsg.packUInt8(player);
  netMsg.packUInt16(id);
  firingInfo.pack(netMsg);

  netMsg.broadcast(MsgShotBegin);

  // now do everyone who dosn't have network
  for (int i = 0; i < curMaxPlayers; i++) {
    GameKeeper::Player* otherData = GameKeeper::Player::getPlayerByIndex(i);
    if (otherData && otherData->playerHandler)
      otherData->playerHandler->shotFired(player,id,(bz_eShotType)firingInfo.shotType);
  }
}


void sendMsgShotEnd(int player, unsigned short id, unsigned short reason)
{
  NetMessage netMsg;
  netMsg.packUInt8(player);
  netMsg.packInt16(id);
  netMsg.packUInt16(reason);
  netMsg.broadcast(MsgShotEnd);

  // now do everyone who dosn't have network
  for (int i = 0; i < curMaxPlayers; i++) {
    GameKeeper::Player* otherData = GameKeeper::Player::getPlayerByIndex(i);
    if (otherData && otherData->playerHandler)
      otherData->playerHandler->shotEnded(player,id,reason);
  }
}


void sendMsgTeleport(int player, unsigned short from, unsigned short to)
{
  NetMessage netMsg;
  netMsg.packUInt8(player);
  netMsg.packUInt16(from);
  netMsg.packUInt16(to);
  netMsg.broadcast(MsgTeleport,false);

  // now do everyone who dosn't have network
  for (int i = 0; i < curMaxPlayers; i++) {
    GameKeeper::Player* otherData = GameKeeper::Player::getPlayerByIndex(i);
    if (otherData && otherData->playerHandler)
      otherData->playerHandler->playerTeleported(player,from,to);
  }
}

void sendMsgAutoPilot(int player, unsigned char autopilot)
{
  NetMessage netMsg;
  netMsg.packUInt8(player);
  netMsg.packUInt8(autopilot);
  netMsg.broadcast(MsgAutoPilot);

  // now do everyone who dosn't have network
  for (int i = 0; i < curMaxPlayers; i++) {
    GameKeeper::Player* otherData = GameKeeper::Player::getPlayerByIndex(i);
    if (otherData && otherData->playerHandler)
      otherData->playerHandler->playerAutopilot(player,autopilot != 0);
  }
}


// network only messages
int sendPlayerUpdateDirect(NetHandler *handler, GameKeeper::Player *otherData)
{
  if (!otherData->player.isPlaying())
    return 0;

  NetMessage netMsg;
  otherData->packPlayerUpdate(netMsg);
  netMsg.send(handler, MsgAddPlayer);

  return (int)netMsg.getSize();
}


int sendTeamUpdateDirect(NetHandler *handler)
{
  // send all teams
  NetMessage netMsg;
  netMsg.packUInt8(CtfTeams);

  for (int t = 0; t < CtfTeams; t++) {
    netMsg.packUInt16(t);
    teamInfos[t].team.pack(netMsg);
  }
  netMsg.send(handler, MsgTeamUpdate);
  return (int)netMsg.getSize();
}


void sendWorldChunk(NetHandler *handler, uint32_t &ptr)
{
  worldWasSentToAPlayer = true;
  // send another small chunk of the world database
  assert((world != NULL) && (worldDatabase != NULL));

  uint32_t size = MaxPacketLen - 2*sizeof(uint16_t) - sizeof(uint32_t);
  uint32_t left = worldDatabaseSize - ptr;

  if (ptr >= worldDatabaseSize) {
    size = 0;
    left = 0;
  } else if (ptr + size >= worldDatabaseSize) {
    size = worldDatabaseSize - ptr;
    left = 0;
  }

  NetMessage netMsg;
  netMsg.packUInt32(uint32_t(left));
  netMsg.packString((char*)worldDatabase + ptr, size);
  netMsg.send(handler, MsgGetWorld);
}


void sendTextMessage(int destPlayer, int sourcePlayer, const char *text,
		     int len, int type, bool recordOnly)
{
  bool broadcast = false;
  bool toGroup   = false;
  GameKeeper::Player *destPlayerData = NULL;
  GameKeeper::Player *srcPlayerData = NULL;
  srcPlayerData = GameKeeper::Player::getPlayerByIndex(sourcePlayer);

  if (destPlayer == AllPlayers) {
    broadcast = true;
  } else {
    if (destPlayer > LastRealPlayer) {
      toGroup = true;
    } else {
      destPlayerData = GameKeeper::Player::getPlayerByIndex(destPlayer);
    }
  }

  if (!destPlayerData && (!broadcast && !toGroup && !recordOnly))
    return;

  char *localtext = (char*)malloc(len+1);
  strncpy(localtext, text, (size_t)len);
  localtext[len] = '\0';

  if (destPlayerData && destPlayerData->playerHandler && !recordOnly) {
    destPlayerData->playerHandler->textMessage(destPlayer, sourcePlayer, localtext);
  }

  if (recordOnly || broadcast || toGroup ||
      (destPlayerData && !destPlayerData->playerHandler)) {
    NetMessage netMsg;
    netMsg.packUInt8(sourcePlayer);
    netMsg.packUInt8(destPlayer);
    netMsg.packUInt8(type);
    netMsg.packString(localtext, len);
    netMsg.packUInt8(0);

    if (recordOnly) {
      Record::addPacket(MsgMessage, netMsg, HiddenPacket);
    } else {
      if (!broadcast && !toGroup) {
	if (srcPlayerData && (sourcePlayer != destPlayer)) {
	  netMsg.send(srcPlayerData->netHandler,MsgMessage);
        }
	if (destPlayerData->player.isHuman()) {
	  netMsg.send(destPlayerData->netHandler,MsgMessage);
        }
      }
      else {
	if (broadcast) {
	  netMsg.broadcast(MsgMessage);

	  // now do everyone who isn't a net player
	  for (int i = 0; i < curMaxPlayers; i++) {
	    GameKeeper::Player* otherData = GameKeeper::Player::getPlayerByIndex(i);
	    if (otherData && otherData->playerHandler)
	      otherData->playerHandler->textMessage(destPlayer, sourcePlayer, localtext);
	  }
	}
	else {
	  if (toGroup) {
	    if (destPlayer == AdminPlayers) {
	      if (srcPlayerData)
		netMsg.send(srcPlayerData->netHandler,MsgMessage);
	      std::vector<int> admins  = GameKeeper::Player::allowed(PlayerAccessInfo::adminMessageReceive);
	      for (unsigned int i = 0; i < admins.size(); ++i) {
		GameKeeper::Player* otherData = GameKeeper::Player::getPlayerByIndex(admins[i]);
		if (admins[i] != sourcePlayer && otherData->player.isHuman()) {
		  netMsg.send(otherData->netHandler,MsgMessage);
                }
	      }
	    } else { // to a team
	      TeamColor destTeam = TeamColor(250 - destPlayer);	// FIXME this teamcolor <-> player id conversion is in several files now
	      for (int i = 0; i < curMaxPlayers; i++) {
		GameKeeper::Player* otherData = GameKeeper::Player::getPlayerByIndex(i);
		if (otherData &&
		    otherData->player.isPlaying() &&
		    otherData->player.isHuman() &&
		    otherData->player.isTeam(destTeam)) {
		  netMsg.send(otherData->netHandler,MsgMessage);
                }
	      }
	    }
	  }
	}
      }
    }
  }

  free(localtext);
}


void sendMessageAlive(int playerID, const fvec3& pos, float rot)
{
  NetMessage netMsg;
  netMsg.packUInt8(playerID);
  netMsg.packFVec3(pos);
  netMsg.packFloat(rot);
  netMsg.broadcast(MsgAlive);

  // now do everyone who doesn't have network
  for (int i = 0; i < curMaxPlayers; i++) {
    GameKeeper::Player* otherData = GameKeeper::Player::getPlayerByIndex(i);
    if (otherData && otherData->playerHandler) {
      otherData->playerHandler->playerSpawned(playerID, pos, rot);
    }
  }
}


void sendMessageAllow(int recipID, int playerID, unsigned char allow)
{
  NetMessage netMsg;
  netMsg.packUInt8(playerID);
  netMsg.packUInt8(allow);

  GameKeeper::Player* otherData = GameKeeper::Player::getPlayerByIndex(recipID);
  if (otherData) {
    netMsg.send(otherData->netHandler, MsgAllow);
  }
}


void sendMessageAllow(int playerID, unsigned char allow)
{
  NetMessage netMsg;
  netMsg.packUInt8(playerID);
  netMsg.packUInt8(allow);
  netMsg.broadcast(MsgAllow);
  // Please note that non-network players do not currently receive the message.
}


bool sendPlayerStateMessage(GameKeeper::Player* playerData, bool shortState)
{
  playerData->doPlayerDR();

  // pack up the data and send it to the net players
  uint16_t code = shortState ? MsgPlayerUpdateSmall : MsgPlayerUpdate;

  NetMessage netMsg;
  netMsg.packUInt8(playerData->getIndex());
  netMsg.packDouble(playerData->stateTimeStamp.getSeconds());

  // do not increment the order because this is just a relay
  playerData->lastState.pack(netMsg, code, false);

  netMsg.broadcast(code);

  // bots need love too
  bz_PlayerUpdateState	apiState;
  playerStateToAPIState(apiState,playerData->getCurrentStateAsState());
  // now do everyone who dosn't have network
  for (int i = 0; i < curMaxPlayers; i++) {
    GameKeeper::Player* otherData = GameKeeper::Player::getPlayerByIndex(i);
    if (otherData && otherData->playerHandler && otherData->player.isPlaying()) {
      otherData->playerHandler->playerStateUpdate(playerData->getIndex(),
						  &apiState, playerData->stateTimeStamp.getSeconds());
    }
  }
  return true;
}


void sendPlayerKilledMessage(int victimIndex, int killerIndex,
                             BlowedUpReason reason, int16_t shotIndex,
                             const FlagType*flagType, int phydrv)
{
  // send MsgKilled
  NetMessage netMsg;
  netMsg.packUInt8(victimIndex);
  netMsg.packUInt8(killerIndex);
  netMsg.packInt16(reason);
  netMsg.packInt16(shotIndex);
  flagType->pack(netMsg);

  if (reason == PhysicsDriverDeath) {
    netMsg.packInt32(phydrv);
  }

  netMsg.broadcast(MsgKilled);

  for (int i = 0; i < curMaxPlayers; i++) {
    GameKeeper::Player* otherData = GameKeeper::Player::getPlayerByIndex(i);
    if (otherData && otherData->playerHandler && otherData->player.isPlaying()) {
      otherData->playerHandler->playerKilledMessage(
        victimIndex, killerIndex,
        (bz_ePlayerDeathReason) reason, shotIndex,
        flagType->flagAbbv.c_str(), phydrv
      );
    }
  }
}


void sendPlayerScoreUpdate(GameKeeper::Player *player)
{
  NetMessage netMsg;
  netMsg.packUInt8(1);
  netMsg.packUInt8(player->getIndex());
  player->score.pack(netMsg);
  netMsg.broadcast(MsgScore);

  for (int i = 0; i < curMaxPlayers; i++) {
    GameKeeper::Player* otherData = GameKeeper::Player::getPlayerByIndex(i);
    if (otherData && otherData->playerHandler)
      otherData->playerHandler->playerScoreUpdate(player->getIndex(),
						  player->score.ranking(),
						  player->score.getWins(),
						  player->score.getLosses(),
						  player->score.getTKs());
  }
}


void sendScoreOverMessage(int playerID, TeamColor _team)
{
  NetMessage netMsg;
  netMsg.packUInt8(playerID);
  netMsg.packInt16(int16_t(_team));
  netMsg.broadcast(MsgScoreOver);

  for (int i = 0; i < curMaxPlayers; i++) {
    GameKeeper::Player* otherData = GameKeeper::Player::getPlayerByIndex(i);
    if (otherData && otherData->playerHandler)
      otherData->playerHandler->scoreLimitReached(playerID, convertTeam(_team));
  }
}


void sendDropFlagMessage(int playerIndex, FlagInfo &flag)
{
  NetMessage netMsg;
  netMsg.packUInt8(playerIndex);
  flag.pack(netMsg);
  netMsg.broadcast(MsgDropFlag);

  bz_FlagUpdateRecord		*flagRecord = new bz_FlagUpdateRecord;
  flagToAPIFlag(flag,flagRecord);

  for (int i = 0; i < curMaxPlayers; i++) {
    GameKeeper::Player* otherData = GameKeeper::Player::getPlayerByIndex(i);
    if (otherData && otherData->playerHandler)
      otherData->playerHandler->flagUpdate(1, &flagRecord);
  }
  delete(flagRecord);
}


void sendFlagCaptureMessage(int playerIndex, int flagIndex, int teamCaptured)
{
  // send MsgCaptureFlag
  NetMessage netMsg;
  netMsg.packUInt8(playerIndex);
  netMsg.packUInt16(uint16_t(flagIndex));
  netMsg.packInt16(int16_t(teamCaptured));
  netMsg.broadcast(MsgCaptureFlag,false);

  for (int i = 0; i < curMaxPlayers; i++) {
    GameKeeper::Player* otherData = GameKeeper::Player::getPlayerByIndex(i);
    if (otherData && otherData->playerHandler)
      otherData->playerHandler->flagCaptured(playerIndex, flagIndex, convertTeam((TeamColor)teamCaptured));
  }
}


void sendRabbitUpdate(int playerIndex, unsigned char mode)
{
  NetMessage netMsg;
  netMsg.packUInt8(playerIndex);
  netMsg.packUInt8(mode);
  netMsg.broadcast(MsgNewRabbit);
}


void sendMsgGMUpdate(int /*player*/, ShotUpdate *shot, int target)
{
  NetMessage netMsg;
  //netMsg.packUInt8(player);
  shot->pack(netMsg);
  netMsg.packUInt8(target);
  netMsg.broadcast(MsgGMUpdate);
}


void sendMsgTimeUpdate(int32_t timeLimit)
{
  // start client's clock
  NetMessage netMsg;
  netMsg.packInt32(timeLimit);
  netMsg.broadcast(MsgTimeUpdate);
}


void sendMsgTangibilityUpdate(unsigned int object, unsigned char tang, int player)
{
  NetMessage netMsg;
  netMsg.packUInt32(object);
  netMsg.packUInt8(tang);

  if (player == AllPlayers) {
    netMsg.broadcast(MsgTangibilityUpdate,false);
  } else {
    GameKeeper::Player* otherData = GameKeeper::Player::getPlayerByIndex(player);
    if (otherData)
      netMsg.send(otherData->netHandler, MsgTangibilityUpdate);
  }
}


void sendMsgTangibilityReset()
{
  NetMessage netMsg;
  netMsg.broadcast(MsgTangibilityReset,false);
}


void sendOSRequest(int player)
{
  GameKeeper::Player* playeData = GameKeeper::Player::getPlayerByIndex(player);
  if (playeData && playeData->netHandler) {
    NetMessage netMsg;
    netMsg.packUInt8(player);
    netMsg.send(playeData->netHandler, MsgQueryOS);
  }
  else {
    playeData->player.OSVersion = "Server Side Player"; // ssp's don't have an os to query
  }
}


void sendMsgCanSpawn(int player, bool canSpawn)
{
  char t= 0;
  if (canSpawn)
    t = 1;

  GameKeeper::Player* p = GameKeeper::Player::getPlayerByIndex(player);
  if (!p)
    return;

  if (!p->playerHandler) {
    NetMessage netMsg;
    netMsg.packUInt8(t);
    netMsg.send(p->netHandler, MsgAllowSpawn);
  } else {
    p->playerHandler->allowSpawn(canSpawn);
  }
}


void sendMsgLimboMessage(int player, const std::string  &text)
{
  NetMessage netMsg;
  netMsg.packStdString(text);

  GameKeeper::Player* otherData = GameKeeper::Player::getPlayerByIndex(player);
  if (otherData)
    netMsg.send(otherData->netHandler, MsgLimboMessage);
}


void sendSetTeam(int playerIndex, int _team)
{
  NetMessage netMsg;
  netMsg.packUInt8(playerIndex);
  netMsg.packInt16(_team);
  netMsg.broadcast(MsgSetTeam);
}


void sendEchoResponse (struct sockaddr_in *uaddr, unsigned char tag)
{
  char echobuffer[5] = {0};
  void *ebuf = echobuffer;
  ebuf = nboPackUInt16(ebuf, 1);
  ebuf = nboPackUInt16(ebuf, MsgEchoResponse);
  ebuf = nboPackUInt8(ebuf, tag);
  sendto(NetHandler::getUdpSocket(), echobuffer, 5, 0, (struct sockaddr*)uaddr,
    sizeof(*uaddr));   //Low level - bad - if this could be encapsulated...
}


static bool checkPlayerCustomDataSize(const std::string &key,
                                      const std::string &value)
{
  const size_t fullSize =
    sizeof(uint16_t) + // len
    sizeof(uint16_t) + // code
    sizeof(uint8_t)  + // playerID
    sizeof(uint32_t) + // key len
    sizeof(uint32_t) + // value len
    key.size()       + // key data
    value.size();      // value data

  return (fullSize < MaxPacketLen);
}


void sendPlayerCustomDataPair(NetHandler* netHandler, int playerID,
                              const std::string &key, const std::string &value)
{
  if (!checkPlayerCustomDataSize(key, value)) {
    return;
  }
  NetMessage netMsg;
  netMsg.packUInt8(playerID);
  netMsg.packStdString(key);
  netMsg.packStdString(value);
  netMsg.send(netHandler, MsgPlayerData);
}


void broadcastPlayerCustomDataPair(int playerID,
                                   const std::string &key,
                                   const std::string &value)
{
  if (!checkPlayerCustomDataSize(key, value)) {
    return;
  }
  NetMessage netMsg;
  netMsg.packUInt8(playerID);
  netMsg.packStdString(key);
  netMsg.packStdString(value);
  netMsg.broadcast(MsgPlayerData);
}


void sendPlayerCustomDataMap(NetHandler* netHandler,
                             const GameKeeper::Player* player)
{
  if (!player) {
    return;
  }
  const int playerID = player->getIndex();
  const std::map<std::string, std::string>& customData = player->customData;
  std::map<std::string, std::string>::const_iterator it;
  for (it = customData.begin(); it != customData.end(); ++it) {
    sendPlayerCustomDataPair(netHandler, playerID, it->first, it->second);
  }
}


void broadcastPlayerCustomDataMap(const GameKeeper::Player* player)
{
  if (!player) {
    return;
  }
  const int playerID = player->getIndex();
  const std::map<std::string, std::string>& customData = player->customData;
  std::map<std::string, std::string>::const_iterator it;
  for (it = customData.begin(); it != customData.end(); ++it) {
    broadcastPlayerCustomDataPair(playerID, it->first, it->second);
  }
}


//============================================================================//
//============================================================================//
//
//messages sent TO the server
//

void getGeneralMessageInfo(void **buffer, uint16_t &code, uint16_t &len)
{
  *buffer = nboUnpackUInt16(*buffer, len);
  *buffer = nboUnpackUInt16(*buffer, code);
}


//============================================================================//
//
//  PackVars
//

PackVars::PackVars(NetHandler* _handler)
: handler(_handler)
, netMsg(new NetMessage)
{
  // start the first message
  startMessage();
}


PackVars::~PackVars()
{
  // end the last message
  endMessage();
  delete netMsg;
}


// callback forwarder
void PackVars::packIt(const std::string &key, void *pv)
{
  reinterpret_cast<PackVars*>(pv)->sendPackVars(key);
}


void PackVars::startMessage()
{
  // reset count
  count = 0;
  // placeholder
  netMsg->packUInt16(0);
}


void PackVars::endMessage()
{
  // repack the placeholder short at the beginning of
  // the buffer with the total count
  nboPackUInt16(netMsg->getData(), count);
  // send the message
  netMsg->send(handler, MsgSetVar);
  netMsg->clear();
}


void PackVars::sendPackVars(const std::string &key)
{
  std::string value = BZDB.get(key);
  size_t pairLen = key.length() + value.length() + 2 * sizeof(uint32_t);
  if ((pairLen + netMsg->getFullSize()) > MaxPacketLen) {
    endMessage();
    startMessage();
  }
  netMsg->packStdString(key);
  netMsg->packStdString(value);
  count++;
}


//============================================================================//

//utils
bool isUDPAttackMessage(uint16_t &code)
{
  switch (code) {
    case MsgShotBegin:
    case MsgShotEnd:
    case MsgPlayerUpdate:
    case MsgPlayerUpdateSmall:
    case MsgGMUpdate:
    case MsgLuaDataFast:
    case MsgUDPLinkRequest:
    case MsgUDPLinkEstablished:
    case MsgHit:{
      return false;
    }
  }
  return true;
}


void playerStateToAPIState(bz_PlayerUpdateState &apiState, const PlayerState &playerState)
{
  apiState.status = eAlive;
  if (playerState.status == PlayerState::DeadStatus) // DeadStatus is 0
    apiState.status = eDead;
//FIXME  else if (playerState.status & PlayerState::Paused)
//FIXME    apiState.status = ePaused;
  else if (playerState.status & PlayerState::Exploding)
    apiState.status = eExploding;
  else if (playerState.status & PlayerState::Teleporting)
    apiState.status = eTeleporting;
  else if (playerState.status & PlayerState::InBuilding)
    apiState.status = eInBuilding;

  apiState.inPhantomZone = (playerState.status & PlayerState::PhantomZoned) != 0;
  apiState.falling = (playerState.status & PlayerState::Falling) != 0;
  apiState.crossingWall = (playerState.status & PlayerState::CrossingWall) != 0;
  apiState.phydrv = (playerState.status & PlayerState::OnDriver) ? playerState.phydrv : -1;
  apiState.rotation = playerState.azimuth;
  apiState.angVel = playerState.angVel;
  memcpy(apiState.pos,playerState.pos,sizeof(float)*3);
  memcpy(apiState.velocity,playerState.velocity,sizeof(float)*3);
}


void APIStateToplayerState(PlayerState &playerState, const bz_PlayerUpdateState &apiState)
{
  playerState.status = 0;
  switch(apiState.status) {
    case eDead:
      playerState.status = PlayerState::DeadStatus; // DeadStatus = 0
      break;
    case eAlive:
      playerState.status |= PlayerState::Alive;
      break;
    case ePaused:
//FIXME      playerState.status |= PlayerState::Paused;
      break;
    case eExploding:
      playerState.status |= PlayerState::Exploding;
      break;
    case eTeleporting:
      playerState.status |= PlayerState::Teleporting;
      break;

    case eInBuilding:
      playerState.status |= PlayerState::InBuilding;
      break;
  }

  if (apiState.inPhantomZone)
    playerState.status |=  PlayerState::PhantomZoned;

  if (apiState.falling)
    playerState.status |=  PlayerState::Falling;

  if (apiState.crossingWall)
    playerState.status |=  PlayerState::CrossingWall;

  if (apiState.phydrv != -1) {
    playerState.status |=  PlayerState::OnDriver;
    playerState.phydrv = apiState.phydrv;
  }

  playerState.azimuth = apiState.rotation;
  playerState.angVel = apiState.angVel;
  memcpy(playerState.pos,apiState.pos,sizeof(float)*3);
  memcpy(playerState.velocity,apiState.velocity,sizeof(float)*3);
}


bool sendMsgLuaData(PlayerId srcPlayerID, int16_t srcScriptID,
                    PlayerId dstPlayerID, int16_t dstScriptID,
                    uint8_t status, const std::string& data)
{
  if (srcPlayerID != ServerPlayer) {
    GameKeeper::Player* srcPlayer =
      GameKeeper::Player::getPlayerByIndex(srcPlayerID);
    if (srcPlayer == NULL) {
      return false;
    }

    const uint8_t knownBits =
      (MsgLuaDataUdpBit | IsAdmin | IsVerified | IsRegistered);

    status &= knownBits;

    const PlayerAccessInfo& info = srcPlayer->accessInfo;

    if ((status & IsAdmin) && !info.isAdmin()) {
      status &= ~IsAdmin; // clear the bit
    }
    if ((status & IsVerified) && !info.isVerified()) {
      status &= ~IsVerified; // clear the bit
    }
    if ((status & IsRegistered) && !info.isRegistered()) {
      status &= ~IsRegistered; // clear the bit
    }
  }

  uint16_t netCode = (status & MsgLuaDataUdpBit) ? MsgLuaDataFast : MsgLuaData;

  NetMessage netMsg;
  netMsg.packUInt8(srcPlayerID);
  netMsg.packInt16(srcScriptID);
  netMsg.packUInt8(dstPlayerID);
  netMsg.packInt16(dstScriptID);
  netMsg.packUInt8(status);
  netMsg.packStdString(data);

  // broadcast
  if (dstPlayerID == AllPlayers) {
    netMsg.broadcast(netCode);
    return true;
  }
   
  // specific player
  if (dstPlayerID <= LastRealPlayer) {
    GameKeeper::Player* dstPlayer =   
      GameKeeper::Player::getPlayerByIndex(dstPlayerID);
    if (dstPlayer == NULL) {
      return false;
    }
    netMsg.send(dstPlayer->netHandler, netCode);
    return true;
  }
   
  // admin group
  if (dstPlayerID == AdminPlayers) {
    std::vector<int> admins =
      GameKeeper::Player::allowed(PlayerAccessInfo::adminMessageReceive);
    for (size_t i = 0; i < admins.size(); ++i) {
      GameKeeper::Player* adminPlayer =
        GameKeeper::Player::getPlayerByIndex(admins[i]);
      netMsg.send(adminPlayer->netHandler, netCode);
    }
    return true;
  }
   
  // send to a team
  TeamColor dstTeam = TeamColor(250 - dstPlayerID);
  for (int i = 0; i < curMaxPlayers; i++) {
    GameKeeper::Player* dstPlayer = GameKeeper::Player::getPlayerByIndex(i);
    if (dstPlayer &&
        dstPlayer->player.isPlaying() &&
        dstPlayer->player.isTeam(dstTeam)) {
      netMsg.send(dstPlayer->netHandler, netCode);
    }
  }  
  return true;
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
