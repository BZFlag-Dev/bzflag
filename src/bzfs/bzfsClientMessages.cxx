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

// bzflag global header
#include "bzfsClientMessages.h"
#include "BZDBCache.h"
#include "bzfsMessages.h"
#include "bzfsPlayerStateVerify.h"
#include "NetMessage.h"
#include "bzfsChatVerify.h"
#include "Protocol.h"

std::map<uint16_t, ClientNetworkMessageHandler*> clientNetworkHandlers;
std::map<uint16_t, PlayerNetworkMessageHandler*> playerNetworkHandlers;


void packWorldSettings ( void )
{
  void* buffer = worldSettings;

  // the settings

  // When changing the next three lines, be sure to update
  // maxPlayersOffset in replaceSettings (RecordReplay.cxx)
  buffer = nboPackFloat  (buffer, BZDBCache::worldSize);
  buffer = nboPackUInt16 (buffer, clOptions->gameType);
  buffer = nboPackUInt16 (buffer, clOptions->gameOptions);
  // An hack to fix a bug on the client
  buffer = nboPackUInt16 (buffer, PlayerSlot);
  buffer = nboPackUInt16 (buffer, clOptions->maxShots);
  buffer = nboPackUInt16 (buffer, numFlags);
  buffer = nboPackUInt16 (buffer, clOptions->botsPerIP);
  buffer = nboPackUInt16 (buffer, clOptions->shakeTimeout);
  buffer = nboPackUInt16 (buffer, clOptions->shakeWins);
}

class SetVarHandler : public ClientNetworkMessageHandler
{
public:
  virtual bool execute ( NetHandler *handler, uint16_t &/*code*/, void * /*buf*/, int /*len*/ )
  {
    PackVars pv(handler);
    BZDB.iterate(PackVars::packIt, &pv);

    return true;
  }
};


class NegotiateFlagHandler : public ClientNetworkMessageHandler
{
public:
  virtual bool execute ( NetHandler *handler, uint16_t &/*code*/, void * buf, int len )
  {
    FlagTypeMap::iterator it;
    FlagSet::iterator m_it;
    FlagOptionMap hasFlag;
    FlagSet missingFlags;
    unsigned short numClientFlags = len/2;

    /* Unpack incoming message containing the list of flags our client supports */
    for (int i = 0; i < numClientFlags; i++) {
      FlagType *fDesc;
      buf = FlagType::unpack(buf, fDesc);
      if (fDesc != Flags::Null) {
	hasFlag[fDesc] = true;
      }
    }

    /* Compare them to the flags this game might need, generating a list of missing flags */
    for (it = FlagType::getFlagMap().begin(); it != FlagType::getFlagMap().end(); ++it) {
      if (!hasFlag[it->second]) {
	if (clOptions->flagCount[it->second] > 0) {
	  missingFlags.insert(it->second);
        }
	if ((clOptions->numExtraFlags > 0) && !clOptions->flagDisallowed[it->second]) {
	  missingFlags.insert(it->second);
        }
      }
    }

    /* Pack a message with the list of missing flags */
    NetMessage netMsg;
    for (m_it = missingFlags.begin(); m_it != missingFlags.end(); ++m_it) {
      if ((*m_it) != Flags::Null) {
	if ((*m_it)->custom) {
	  // custom flag, tell the client about it
	  NetMessage customMsg;
	  (*m_it)->packCustom(customMsg);
	  customMsg.send(handler, MsgFlagType);
	}
	else {
	  // they should already know about this one, dump it back to them
	  (*m_it)->pack(netMsg);
	}
      }
    }
    netMsg.send(handler, MsgNegotiateFlags);
    return true;
  }
};


class GetWorldHandler : public ClientNetworkMessageHandler
{
public:
  virtual bool execute ( NetHandler *handler, uint16_t &/*code*/, void * buf , int len )
  {
    if (len < 4)
      return false;

    uint32_t ptr;	// data: count (bytes read so far)
    buf = nboUnpackUInt32(buf, ptr);

    sendWorldChunk(handler, ptr);

    return true;
  }
};


class WantSettingsHandler : public ClientNetworkMessageHandler
{
public:
  virtual bool execute ( NetHandler *handler, uint16_t &/*code*/, void * /*buf*/, int /*len*/ )
  {
    NetMessage netMsg;
    netMsg.packString(worldSettings, WorldSettingsSize);
    netMsg.send(handler, MsgGameSettings);
    return true;
  }
};


class WantWHashHandler : public ClientNetworkMessageHandler
{
public:
  virtual bool execute ( NetHandler *handler, uint16_t &/*code*/, void * /*buf*/, int /*len*/ )
  {
    if (clOptions->cacheURL.size() > 0) {
      NetMessage netMsg;
      netMsg.packString(clOptions->cacheURL.c_str(), clOptions->cacheURL.size() + 1);
      netMsg.send(handler, MsgCacheURL);
    }

    NetMessage netMsg;
    netMsg.packString(hexDigest, strlen(hexDigest) + 1);
    netMsg.send(handler, MsgWantWHash);
    return true;
  }
};


class QueryGameHandler : public ClientNetworkMessageHandler
{
public:
  virtual bool execute ( NetHandler *handler, uint16_t &/*code*/, void * /*buf*/, int /*len*/ )
  {
    // much like a ping packet but leave out useless stuff (like
    // the server address, which must already be known, and the
    // server version, which was already sent).
    NetMessage netMsg;
    netMsg.packUInt16(pingReply.gameType);
    netMsg.packUInt16(pingReply.gameOptions);
    netMsg.packUInt16(pingReply.maxPlayers);
    netMsg.packUInt16(pingReply.maxShots);
    netMsg.packUInt16(teamInfos[0].team.size);
    netMsg.packUInt16(teamInfos[1].team.size);
    netMsg.packUInt16(teamInfos[2].team.size);
    netMsg.packUInt16(teamInfos[3].team.size);
    netMsg.packUInt16(teamInfos[4].team.size);
    netMsg.packUInt16(teamInfos[5].team.size);
    netMsg.packUInt16(pingReply.rogueMax);
    netMsg.packUInt16(pingReply.redMax);
    netMsg.packUInt16(pingReply.greenMax);
    netMsg.packUInt16(pingReply.blueMax);
    netMsg.packUInt16(pingReply.purpleMax);
    netMsg.packUInt16(pingReply.observerMax);
    netMsg.packUInt16(pingReply.shakeWins);
    // 1/10ths of second
    netMsg.packUInt16(pingReply.shakeTimeout);
    netMsg.packUInt16(pingReply.maxPlayerScore);
    netMsg.packUInt16(pingReply.maxTeamScore);
    netMsg.packUInt16(pingReply.maxTime);
    netMsg.packUInt16((uint16_t)clOptions->timeElapsed);

    // send it
    netMsg.send(handler, MsgQueryGame);

    return true;
  }
};


class QueryPlayersHandler : public ClientNetworkMessageHandler
{
public:
  virtual bool execute ( NetHandler *handler, uint16_t &/*code*/, void * /*buf*/, int /*len*/ )
  {
    // count the number of active players
    int numPlayers = GameKeeper::Player::count();

    // first send number of teams and players being sent
    NetMessage netMsg;

    netMsg.packUInt16(NumTeams);
    netMsg.packUInt16(numPlayers);

    netMsg.send(handler, MsgQueryPlayers);

    if (sendTeamUpdateDirect(handler) < 0)
      return true;

    GameKeeper::Player *otherData;
    for (int i = 0; i < curMaxPlayers; i++) {
      otherData = GameKeeper::Player::getPlayerByIndex(i);

      if (!otherData)
	continue;

      if (sendPlayerUpdateDirect(handler, otherData) < 0)
	return true;
    }
    return true;
  }
};


class UDPLinkEstablishedHandler : public ClientNetworkMessageHandler
{
public:
  virtual bool execute ( NetHandler *handler, uint16_t &/*code*/, void * /*buf*/, int /*len*/ )
  {
    logDebugMessage(2,"Connection at %s outbound UDP up\n", handler->getTargetIP());
    return true;
  }
};

// messages that have players
class PlayerFirstHandler : public PlayerNetworkMessageHandler
{
public:
  virtual void *unpackPlayer ( void * buf, int len )
  {
    player = NULL;

    // byte * 3
    if ( len >= 1 ) {
      uint8_t temp = 0;
      buf  = nboUnpackUInt8(buf, temp);

      player = GameKeeper::Player::getPlayerByIndex(temp);

      return buf;
    }
    return buf;
  }
};


class PlayerFirstNoBumpHandler : public PlayerFirstHandler
{
public:
  virtual void *unpackPlayer ( void * buf, int len )
  {
    player = NULL;

    // byte * 3
    if ( len >= 1 ) {
      unsigned char temp = 0;
      nboUnpackUInt8(buf, temp);
      player = GameKeeper::Player::getPlayerByIndex(temp);

      return buf;
    }
    return buf;
  }
};


class NewPlayerHandler : public PlayerFirstHandler
{
public:
  virtual bool execute ( uint16_t &/*code*/, void * buf, int len )
  {
    uint8_t botID;
    int16_t team;

    if ( len < 1)
      return false;

    buf = nboUnpackUInt8(buf, botID);
    buf = nboUnpackInt16(buf, team);

    PlayerId id = getNewBot(player->getIndex(),botID);
    if (id == 0xff)
      return false;

    NetMessage netMsg;
    netMsg.packUInt8(id);
    netMsg.packUInt8(botID);
    netMsg.packInt16(team);
    netMsg.send(player->netHandler, MsgNewPlayer);

    return true;
  }
};


class CapBitsHandler : public PlayerFirstHandler
{
public:
  virtual bool execute ( uint16_t &/*code*/, void * buf, int len )
  {
    if (!player || len < 3)
      return false;

    unsigned char temp = 0;

    buf = nboUnpackUInt8(buf,temp);
    player->caps.canDownloadResources = temp != 0;

    buf = nboUnpackUInt8(buf,temp);
    player->caps.canPlayRemoteSounds = temp != 0;

    return true;
  }
};


class EnterHandler : public PlayerFirstHandler
{
public:
  virtual bool execute ( uint16_t &/*code*/, void * buf, int len )
  {
    if (!player || len < 119)
      return false;

    // a previous MsgEnter will have set the name a few lines down from here
    if (!player->accessInfo.getName().empty()) {
      logDebugMessage(1,"Player %s [%d] sent another MsgEnter\n",
		      player->player.getCallSign(), player->getIndex());
      rejectPlayer(player->getIndex(), RejectBadRequest, "invalid request");
      return true;
    }

    uint16_t rejectCode;
    char     rejectMsg[MessageLen];

    if (!player->player.unpackEnter(buf, rejectCode, rejectMsg)) {
      rejectPlayer(player->getIndex(), rejectCode, rejectMsg);
      return true;
    }

    player->accessInfo.setName(player->player.getCallSign());
    std::string timeStamp = BzTime::timestamp();
    std::string playerIP = "local.player";
    if ( player->netHandler )
      playerIP = player->netHandler->getTargetIP();

    logDebugMessage(1,"Player %s [%d] has joined from %s at %s with token \"%s\"\n",
		    player->player.getCallSign(),
		    player->getIndex(), playerIP.c_str(), timeStamp.c_str(),
		    player->player.getToken());
    const char* referrer = player->player.getReferrer();
    if (referrer && referrer[0]) {
      logDebugMessage(1,"  referred by \"%s\"\n", referrer);
    }

    if (!clOptions->publicizeServer)
      player->_LSAState = GameKeeper::Player::notRequired;
    else if (strlen(player->player.getCallSign()))
      player->_LSAState = GameKeeper::Player::required;

    dontWait = true;

    return true;
  }
};


class ExitHandler : public PlayerFirstHandler
{
public:
  virtual bool execute ( uint16_t &/*code*/, void * /*buf*/, int /*len*/ )
  {
    if (!player)
      return false;
    removePlayer(player->getIndex(), "left", false);
    return true;
  }
};


class AliveHandler : public PlayerFirstHandler
{
public:
  virtual bool execute ( uint16_t &/*code*/, void * /*buf*/, int /*len*/ )
  {
    if (!player)
      return false;
    if (!player->player.isCompletelyAdded())
      return false;

    // player is on the waiting list
    char buffer[MessageLen];
    float waitTime = rejoinList.waitTime(player->getIndex());

    if (waitTime > 0.0f) {
      snprintf (buffer, MessageLen, "You are unable to begin playing for %.1f seconds.", waitTime);
      sendMessage(ServerPlayer, player->getIndex(), buffer);

      // Make them pay dearly for trying to rejoin quickly
      playerAlive(player->getIndex());
      playerKilled(player->getIndex(), player->getIndex(), GotKilledMsg, -1, Flags::Null, -1);
      return true;
    }

    // player moved before countdown started
    if (clOptions->timeLimit>0.0f && !countdownActive)
      player->player.setPlayedEarly();

    playerAlive(player->getIndex());
    return true;
  }
};


class KilledHandler : public PlayerFirstHandler
{
public:
  virtual bool execute ( uint16_t &/*code*/, void * buf, int len )
  {
    if (!player || len < 7)
      return false;
    if (!player->player.isCompletelyAdded())
      return false;

    if (player->player.isObserver())
      return true;

    // data: id of killer, shot id of killer
    PlayerId killer;
    FlagType* flagType;
    int16_t shot, reason;
    int phydrv = -1;

    buf = nboUnpackUInt8(buf, killer);
    buf = nboUnpackInt16(buf, reason);
    buf = nboUnpackInt16(buf, shot);
    buf = FlagType::unpack(buf, flagType);

    if (reason == PhysicsDriverDeath) {
      int32_t inPhyDrv;
      buf = nboUnpackInt32(buf, inPhyDrv);
      phydrv = int(inPhyDrv);
    }

    if (killer != ServerPlayer) {
      // Sanity check on shot: Here we have the killer
      int si = (shot == -1 ? -1 : shot & 0x00FF);
      if ((si < -1) || (si >= clOptions->maxShots))
	return true;
    }

    player->player.endShotCredit--;
    playerKilled(player->getIndex(), lookupPlayer(killer), (BlowedUpReason)reason, shot, flagType, phydrv);

    // stop pausing attempts as you can not pause when being dead
    player->pauseRequested = false;
    player->pauseActiveTime = BzTime::getNullTime();
    player->player.setPaused(false);
    return true;
  }
};


class DropFlagHandler : public PlayerFirstHandler
{
public:
  virtual bool execute ( uint16_t &/*code*/, void * buf, int len )
  {
    if (!player || len < 13)
      return false;
    if (!player->player.isCompletelyAdded())
      return false;

    // data: position of drop
    fvec3 pos;
    buf = nboUnpackFVec3(buf, pos);

    const float halfSize = BZDBCache::worldSize * 0.5f;
    if ((fabsf(pos.x) > halfSize) ||
        (fabsf(pos.y) > halfSize)) {
      // client may be cheating
      const PlayerId id = player->getIndex();
      logDebugMessage(1, "Player %s [%d] dropped flag out of bounds to %f %f %f\n",
		      player->player.getCallSign(), id, pos.x, pos.y, pos.z);
      sendMessage(ServerPlayer, id, "Autokick: Flag dropped out of bounds.");
    }

    dropPlayerFlag(*player, pos);

    return true;
  }
};


class CaptureFlagHandler : public PlayerFirstHandler
{
public:
  virtual bool execute ( uint16_t &/*code*/, void * buf, int len )
  {
    if (!player || len < 3)
      return false;
    if (!player->player.isCompletelyAdded())
      return false;

    // data: team whose territory flag was brought to
    int16_t _team;
    buf = nboUnpackInt16(buf, _team);

    captureFlag(player->getIndex(), TeamColor(_team));
    return true;
  }
};


class ShotBeginHandler : public PlayerFirstHandler
{
public:
  virtual bool execute ( uint16_t &/*code*/, void * buf, int len )
  {
    if (!player || len < 35)
      return false;
    if (!player->player.isCompletelyAdded())
      return false;

    FiringInfo firingInfo;
    firingInfo.shot.player = player->getIndex();

    // TODO, this should be made into a generic function that updates
    // the state, so that others can add a firing info to the state

    uint16_t id;
    buf = nboUnpackUInt16(buf, id);
    firingInfo.shot.id = id;

    buf = nboUnpackDouble(buf, firingInfo.timeSent);
    buf = nboUnpackFVec3(buf,  firingInfo.shot.pos);
    buf = nboUnpackFVec3(buf,  firingInfo.shot.vel);

    firingInfo.shotType = player->effectiveShotType;

    const PlayerInfo &shooter = player->player;
    if (!shooter.isAlive() || shooter.isObserver())
      return true;

    FlagInfo &fInfo = *FlagInfo::get(shooter.getFlag());

    if (shooter.haveFlag())
      firingInfo.flagType = fInfo.flag.type;
    else
      firingInfo.flagType = Flags::Null;

    if (!player->addShot(id & 0xff, id >> 8, firingInfo))
      return true;

    char message[MessageLen];
    if (shooter.haveFlag()) {
      fInfo.numShots++; // increase the # shots fired
      int limit = clOptions->flagLimit[fInfo.flag.type];
      if (limit != -1) {
	// if there is a limit for players flag
	int shotsLeft = limit -  fInfo.numShots;

	if (shotsLeft > 0) {
	  //still have some shots left
	  // give message each shot below 5, each 5th shot & at start
	  if (shotsLeft % 5 == 0 || shotsLeft <= 3 || shotsLeft == limit-1) {
	    if (shotsLeft > 1)
	      sprintf(message,"%d shots left",shotsLeft);
	    else
	      strcpy(message,"1 shot left");

	    sendMessage(ServerPlayer, player->getIndex(), message);
	  }
	} else {
	  // no shots left
	  if (shotsLeft == 0 || (limit == 0 && shotsLeft < 0)) {
	    // drop flag at last known position of player
	    // also handle case where limit was set to 0
	    fInfo.grabs = 0; // recycle this flag now
	    dropPlayerFlag(*player, player->currentPos);
	  } else {
	    // more shots fired than allowed
	    // do nothing for now -- could return and not allow shot
	  }
	} // end no shots left
      } // end is limit
    } // end of player has flag

    // ask the API if it wants to modify this shot
    bz_ShotFiredEventData_V1 shotEvent;
    shotEvent.playerID = (int)player->getIndex();
    shotEvent.shotID = (int)firingInfo.shot.id;
    shotEvent.type = firingInfo.flagType->flagAbbv;
    memcpy(shotEvent.pos, firingInfo.shot.pos, sizeof(float[3]));
    memcpy(shotEvent.vel, firingInfo.shot.vel, sizeof(float[3]));

    worldEventManager.callEvents(bz_eShotFiredEvent,&shotEvent);

    sendMsgShotBegin(player->getIndex(), id, firingInfo);

    return true;
  }
};


class ShotEndHandler : public PlayerFirstHandler
{
public:
  virtual bool execute ( uint16_t &/*code*/, void * buf, int len )
  {
    if (!player || len < 3)
      return false;
    if (!player->player.isCompletelyAdded())
      return false;

    if (player->player.isObserver())
      return true;

    int16_t shot;
    uint16_t reason;
    buf = nboUnpackInt16(buf, shot);
    buf = nboUnpackUInt16(buf, reason);

    // ask the API if it wants to modify this shot
    bz_ShotEndedEventData_V1 shotEvent;
    shotEvent.playerID = (int)player->getIndex();
    shotEvent.shotID = shot;
    shotEvent.explode = reason == 0;
    worldEventManager.callEvents(bz_eShotEndedEvent,&shotEvent);

    FiringInfo firingInfo;
    player->removeShot(shot & 0xff, shot >> 8, firingInfo);

    sendMsgShotEnd(player->getIndex(),shot,reason);

    return true;
  }
};


class ShotInfoHandler : public PlayerFirstHandler
{
public:
  virtual bool execute ( uint16_t &/*code*/, void * buf, int len )
  {
    if (!player || len < 16)
      return false;
    if (!player->player.isCompletelyAdded())
      return false;

    if (player->player.isObserver()) {
      return true;
    }

    int16_t shotID;
    uint8_t infoType;
    fvec3 pos;
    uint16_t linkSrcID, linkDstID;
    FlagType* flagType;
    uint32_t obstacleGUID = (uint32_t)-1;

    buf = nboUnpackInt16(buf, shotID);
    buf = FlagType::unpack(buf, flagType);
    buf = nboUnpackUInt8(buf, infoType);
    buf = nboUnpackFVec3(buf, pos);
    switch (infoType) {
      case ShotInfoTeleport: {
        if (len < 20) { return false; }
        buf = nboUnpackUInt16(buf, linkSrcID);
        buf = nboUnpackUInt16(buf, linkDstID);
        break;
      }
      case ShotInfoStopped:
      case ShotInfoRicochet: {
        if (len < 20) { return false; }
        buf = nboUnpackUInt32(buf, obstacleGUID);
        break;
      }
      case ShotInfoExpired: {
        break;
      }
    }

    // NOTE: not broadcasting/relaying to the clients

    const char* shotFlag = flagType->flagAbbv.c_str();

    switch (infoType) {
      case ShotInfoExpired: {
        bz_ShotExpiredEventData_V1 event(player->getIndex(), shotID, shotFlag,
                                         pos.x, pos.y, pos.z);
        worldEventManager.callEvents(bz_eShotExpiredEvent, &event);
        break;
      }
      case ShotInfoStopped: {
        bz_ShotStoppedEventData_V1 event(player->getIndex(), shotID, shotFlag,
                                         pos.x, pos.y, pos.z, obstacleGUID);
        worldEventManager.callEvents(bz_eShotStoppedEvent, &event);
        break;
      }
      case ShotInfoRicochet: {
        bz_ShotRicochetEventData_V1 event(player->getIndex(), shotID, shotFlag,
                                          pos.x, pos.y, pos.z, obstacleGUID);
        worldEventManager.callEvents(bz_eShotRicochetEvent, &event);
        break;
      }
      case ShotInfoTeleport: {
        bz_ShotTeleportEventData_V1 event(player->getIndex(), shotID, shotFlag,
                                          pos.x, pos.y, pos.z,
                                          linkSrcID, linkDstID);
        worldEventManager.callEvents(bz_eShotTeleportEvent, &event);
        break;
      }
    }

    return true;
  }
};

class HitHandler : public PlayerFirstHandler
{
public:
  virtual bool execute ( uint16_t &/*code*/, void * buf, int len )
  {
    if (!player || len < 3)
      return false;
    if (!player->player.isCompletelyAdded())
      return false;

    if (player->player.isObserver() || !player->player.isAlive())
      return true;

    PlayerId hitPlayer = player->getIndex();
    PlayerId shooterPlayer;
    FiringInfo firingInfo;
    int16_t shot;

    buf = nboUnpackUInt8(buf, shooterPlayer);
    buf = nboUnpackInt16(buf, shot);
    GameKeeper::Player *shooterData = GameKeeper::Player::getPlayerByIndex(shooterPlayer);

    if (!shooterData)
      return true;

    if (shooterData->removeShot(shot & 0xff, shot >> 8, firingInfo)) {
      sendMsgShotEnd(shooterPlayer, shot, 1);

      const int flagIndex = player->player.getFlag();
      FlagInfo *flagInfo  = NULL;

      if (flagIndex >= 0) {
	flagInfo = FlagInfo::get(flagIndex);
	dropFlag(*flagInfo);
      }

      if (!flagInfo || flagInfo->flag.type != Flags::Shield)
	playerKilled(hitPlayer, shooterPlayer, GotShot, shot, firingInfo.flagType, false, false);
    }

    return true;
  }
};


class TeleportHandler : public PlayerFirstHandler
{
public:
  virtual bool execute ( uint16_t &/*code*/, void * buf, int len )
  {
    if (!player || len < 4)
      return false;
    if (!player->player.isCompletelyAdded())
      return false;

    uint16_t from, to;

    if (invalidPlayerAction(player->player, player->getIndex(), "teleport"))
      return true;

    buf = nboUnpackUInt16(buf, from);
    buf = nboUnpackUInt16(buf, to);

    bz_TeleportEventData_V1 eventData;
    eventData.playerID = player->getIndex();
    eventData.from = from;
    eventData.to = to;

    worldEventManager.callEvents(eventData);

    sendMsgTeleport(player->getIndex(), from, to);
    return true;
  }
};


class MessageHandler : public PlayerFirstHandler
{
public:
  virtual bool execute ( uint16_t &/*code*/, void * buf, int len )
  {
    if (!player || len < MessageLen+1)
      return false;
    if (!player->player.isCompletelyAdded())
      return false;

    // data: target player/team, message string
    PlayerId dstPlayer;
    char message[MessageLen];

    buf = nboUnpackUInt8(buf, dstPlayer);
    buf = nboUnpackString(buf, message, sizeof(message));
    message[MessageLen - 1] = '\0';

    player->player.hasSent();
    if (dstPlayer == AllPlayers) {
      logDebugMessage(1,"Player %s [%d] -> All: %s\n", player->player.getCallSign(), player->getIndex(), message);
    } else {
      if (dstPlayer == AdminPlayers) {
	logDebugMessage(1,"Player %s [%d] -> Admin: %s\n",player->player.getCallSign(), player->getIndex(), message);
      } else {
	if (dstPlayer > LastRealPlayer) {
	  logDebugMessage(1,"Player %s [%d] -> Team: %s\n",player->player.getCallSign(),  player->getIndex(), message);
	} else {
	  GameKeeper::Player *p = GameKeeper::Player::getPlayerByIndex(dstPlayer);
	  if (p != NULL)
	    logDebugMessage(1,"Player %s [%d] -> Player %s [%d]: %s\n",player->player.getCallSign(), player->getIndex(), p->player.getCallSign(), dstPlayer, message);
	  else
	    logDebugMessage(1,"Player %s [%d] -> Player Unknown [%d]: %s\n",  player->player.getCallSign(), player->getIndex(), dstPlayer, message);
	}
      }
    }
    // check for spamming and garbage
    if (!checkChatSpam(message, player, player->getIndex()) && !checkChatGarbage(message, player, player->getIndex()))
      sendPlayerMessage(player, dstPlayer, message);

    return true;
  }
};


class TransferFlagHandler : public PlayerFirstHandler
{
public:
  virtual bool execute ( uint16_t &/*code*/, void * buf, int len )
  {
    if (!player || len < 2)
      return false;
    if (!player->player.isCompletelyAdded())
      return false;

    PlayerId from, to;

    from = player->getIndex();

    buf = nboUnpackUInt8(buf, to);

    int flagIndex = player->player.getFlag();
    if (to == ServerPlayer) {
      if (flagIndex >= 0)
	zapFlag (*FlagInfo::get(flagIndex));
      return true;
    }

    // Sanity check
    if ( (to >= curMaxPlayers) || (flagIndex == -1) )
      return true;

    GameKeeper::Player *toData = GameKeeper::Player::getPlayerByIndex(to);
    if (!toData)
      return true;

    bz_FlagTransferredEventData_V1 eventData;

    eventData.fromPlayerID = player->player.getPlayerIndex();
    eventData.toPlayerID = toData->player.getPlayerIndex();
    eventData.flagType = NULL;
    eventData.action = eventData.ContinueSteal;

    worldEventManager.callEvents(bz_eFlagTransferredEvent, &eventData);

    if (eventData.action != eventData.CancelSteal) {
      int oFlagIndex = toData->player.getFlag();
      if (oFlagIndex >= 0)
	zapFlag (*FlagInfo::get(oFlagIndex));
    }

    if (eventData.action == eventData.ContinueSteal)
      sendFlagTransferMessage(to, from, *FlagInfo::get(flagIndex));
    return true;
  }
};


class NewRabbitHandler : public PlayerFirstHandler
{
public:
  virtual bool execute ( uint16_t &/*code*/, void * /*buf*/, int /*len*/ )
  {
    if (!player)
      return false;
    if (!player->player.isCompletelyAdded())
      return false;

    if (player->getIndex() == rabbitIndex)
      anointNewRabbit();

    return true;
  }
};


class PauseHandler : public PlayerFirstHandler
{
public:
  virtual bool execute ( uint16_t &/*code*/, void * buf, int len )
  {
    if (!player || (len < 2))
      return false;
    if (!player->player.isCompletelyAdded())
      return false;

    uint8_t pauseCode;
    nboUnpackUInt8(buf, pauseCode);
    switch (pauseCode) {
      case PauseCodeEnable:
      case PauseCodeDisable: {
        break;
      }
      default: { // ignore unexpected pause codes
        logDebugMessage(1, "unexpected pause code: %s sent %d\n",
                        player->player.getCallSign(), (int)pauseCode);
        return true;
      }
    }
    const bool wantPause = (pauseCode == PauseCodeEnable);

    // notify plugins, and let them block the request
    const int playerIndex = player->player.getPlayerIndex();
    bz_PlayerPauseRequestData_V1 eventData(playerIndex, wantPause);
    worldEventManager.callEvents(bz_ePlayerPauseRequestEvent, &eventData);
    if (eventData.allow == false) {
      std::string reason = eventData.reason.c_str();
      if (reason.empty()) {
        reason = "pause request denied";
      }
      NetMessage netMsg;
      netMsg.packUInt8(player->getIndex());
      netMsg.packUInt8(PauseCodeCancel);
      netMsg.packStdString(reason);
      netMsg.send(player->netHandler, MsgPause);
      return true;
    }

    if (!wantPause) { // unpause immediately
      pausePlayer(playerIndex, false);
      return true;
    }

    if (player->pauseRequested) {
      return true; // ignore repeat requests
    }

    static BZDB_float pauseDelay("_pauseDelay");
    // delayed pausing
    BzTime activeTime = BzTime::getCurrent();
    activeTime += pauseDelay;
    player->pauseActiveTime = activeTime;
    player->pauseRequested  = true;

    // send the PauseCodeAcknowledge message
    if (player->netHandler) {
      NetMessage netMsg;
      netMsg.packUInt8(player->getIndex());
      netMsg.packUInt8(PauseCodeAcknowledge);
      netMsg.packFloat(pauseDelay);
      netMsg.send(player->netHandler, MsgPause);
    }

    return true;
  }
};


class AutoPilotHandler : public PlayerFirstHandler
{
public:
  virtual bool execute ( uint16_t &/*code*/, void * buf, int len )
  {
    if (!player || len < 2)
      return false;
    if (!player->player.isCompletelyAdded())
      return false;

    uint8_t autopilot;
    nboUnpackUInt8(buf, autopilot);

    bool allow = !BZDB.isTrue(BZDBNAMES.DISABLEBOTS);

    bz_AutoPilotChangeData_V1 evnt(autopilot != 0, allow,player->getIndex());

    worldEventManager.callEvents(bz_eAllowAutoPilotChangeEvent,&evnt);

    if (evnt.allow)
      player->setAutoPilot(autopilot != 0);

    return true;
  }
};


class LagPingHandler : public PlayerFirstHandler
{
public:
  virtual bool execute ( uint16_t &/*code*/, void * buf, int len )
  {
    if (!player || len < 3) {
      return false;
    }

    bool lagwarn, lagkick, jittwarn, jittkick, plosswarn, plosskick;
    char message[MessageLen];

    player->lagInfo.updatePingLag(buf,
                                  lagwarn,   lagkick,
                                  jittwarn,  jittkick,
                                  plosswarn, plosskick);

    if (lagwarn) {
      sprintf(message,"*** Server Warning: your lag is too high (%d ms) ***", player->lagInfo.getLag());
      sendMessage(ServerPlayer, player->getIndex(), message);
      if (lagkick) {
	lagKick(player->getIndex());
      }
    }
    if (jittwarn) {
      sprintf(message, "*** Server Warning: your jitter is too high (%d ms) ***", player->lagInfo.getJitter());
      sendMessage(ServerPlayer, player->getIndex(), message);
      if (jittkick) {
	jitterKick(player->getIndex());
      }
    }
    if (plosswarn) {
      sprintf(message, "*** Server Warning: your packetloss is too high (%d%%) ***", player->lagInfo.getLoss());
      sendMessage(ServerPlayer, player->getIndex(), message);
      if (plosskick) {
	packetLossKick(player->getIndex());
      }
    }

    return true;
  }
};


class PlayerUpdateHandler : public PlayerFirstHandler
{
public:
  virtual bool execute ( uint16_t &code, void * buf, int len )
  {
    if (!player)
      return false;

    if (code == MsgPlayerUpdateSmall && len < 21)
      return false;
    else if (len < 27)
      return false;

    PlayerState state;
    double timestamp;

    buf = nboUnpackDouble(buf, timestamp);
    buf = state.unpack(buf, code);

    updatePlayerState(player, state, BzTime(timestamp), code == MsgPlayerUpdateSmall);

    return true;
  }
};


class PlayerDataHandler : public PlayerFirstHandler
{
public:
  virtual bool execute ( uint16_t & /*code*/, void * buf, int /*len*/ )
  {
    if (!player)
      return false;

    std::string key, value;
    buf = nboUnpackStdString(buf, key);
    buf = nboUnpackStdString(buf, value);

    logDebugMessage(4, "Player %s [%d] CustomData < '%s' = '%s' >\n",
                    player->player.getCallSign(), player->getIndex(),
                    key.c_str(), value.c_str());

    // let the API change the value
    bz_PlayerCustomData_V1 data(bz_ePlayerSentCustomData);
    data.playerID = player->getIndex();
    data.key = key;
    data.data = value;
    worldEventManager.callEvents(data);

    // if the key is still cool, go and set the change,
    // and notify all clients and the API that it changed
    if (data.key.size() > 0) {
      bz_setPlayerCustomData(player->getIndex(), data.key.c_str(),
                                                 data.data.c_str());
    }

    return true;
  }
};

class QueryOSHandler : public PlayerFirstHandler
{
public:
  virtual bool execute ( uint16_t & /*code*/, void * buf, int /*len*/ )
  {
    if (!player)
      return false;

    std::string vers;
    buf = nboUnpackStdString(buf, vers);

    player->player.OSVersion = vers;
    logDebugMessage(2, "Player %s [%d] OS version \"%s\"\n",
		    player->player.getCallSign(), player->getIndex(),
		    player->player.OSVersion.c_str());

    return true;
  }
};



class GMUpdateHandler : public PlayerFirstNoBumpHandler
{
public:
  virtual bool execute ( uint16_t &/*code*/, void * buf, int len )
  {
    if (!player || len < 34)
      return false;

    if (!player->player.isCompletelyAdded())
      return false;

    ShotUpdate shot;
    buf = shot.unpack(buf);
    const uint8_t shotID   = shot.id & 0xff;
    const uint8_t shotSalt = shot.id >> 8;

    unsigned char temp = 0;
    nboUnpackUInt8(buf, temp);

    PlayerId target = temp;

    if (!player->player.isAlive() || player->player.isObserver()) {
      return true;
    }

    if (!player->updateShot(shotID, shotSalt)) {
      return true ;
    }

    sendMsgGMUpdate(player->getIndex(), &shot, target);
    return true;
  }
};


class LuaDataHandler : public PlayerFirstNoBumpHandler
{
public:
  virtual bool execute(uint16_t& /*code*/, void* buf, int len)
  {
    const size_t minSize =
      sizeof(PlayerId) + // src playerID
      sizeof(int16_t)  + // src scriptID
      sizeof(PlayerId) + // dst playerID
      sizeof(int16_t)  + // dst scriptID
      sizeof(uint8_t)  + // status
      sizeof(uint32_t);  // the data

    if ((len + 1) < (int)minSize) { // +1 for the 'NoBump' srcPlayerID
      return false;
    }

    PlayerId srcPlayerID;
    int16_t  srcScriptID;
    PlayerId dstPlayerID;
    int16_t  dstScriptID;
    uint8_t  status;
    std::string data;

    buf = nboUnpackUInt8(buf, srcPlayerID);
    buf = nboUnpackInt16(buf, srcScriptID);
    buf = nboUnpackUInt8(buf, dstPlayerID);
    buf = nboUnpackInt16(buf, dstScriptID);
    buf = nboUnpackUInt8(buf, status);
    buf = nboUnpackStdStringRaw(buf, data);

    if (GameKeeper::Player::getPlayerByIndex(srcPlayerID) == NULL) {
      return false;
    }

    bz_LuaDataEventData_V1 eventData(srcPlayerID, srcScriptID,
                                     dstPlayerID, dstScriptID,
                                     status, data);
    worldEventManager.callEvents(bz_eLuaDataEvent, &eventData);
    if (eventData.doNotSend) {
      return true; // blocked by the event system, no warnings
    }

    sendMsgLuaData(srcPlayerID, srcScriptID,
                   dstPlayerID, dstScriptID,
                   status, data);

    return true;
  }
};


void registerDefaultHandlers ( void )
{
  clientNetworkHandlers[MsgSetVar]             = new SetVarHandler;
  clientNetworkHandlers[MsgNegotiateFlags]     = new NegotiateFlagHandler;
  clientNetworkHandlers[MsgGetWorld]           = new GetWorldHandler;
  clientNetworkHandlers[MsgWantSettings]       = new WantSettingsHandler;
  clientNetworkHandlers[MsgWantWHash]          = new WantWHashHandler;
  clientNetworkHandlers[MsgQueryGame]          = new QueryGameHandler;
  clientNetworkHandlers[MsgQueryPlayers]       = new QueryPlayersHandler;
  clientNetworkHandlers[MsgUDPLinkEstablished] = new UDPLinkEstablishedHandler;

  playerNetworkHandlers[MsgNewPlayer]         = new NewPlayerHandler;
  playerNetworkHandlers[MsgCapBits]           = new CapBitsHandler;
  playerNetworkHandlers[MsgEnter]             = new EnterHandler;
  playerNetworkHandlers[MsgExit]              = new ExitHandler;
  playerNetworkHandlers[MsgAlive]             = new AliveHandler;
  playerNetworkHandlers[MsgKilled]            = new KilledHandler;
  playerNetworkHandlers[MsgDropFlag]          = new DropFlagHandler;
  playerNetworkHandlers[MsgCaptureFlag]       = new CaptureFlagHandler;
  playerNetworkHandlers[MsgShotBegin]         = new ShotBeginHandler;
  playerNetworkHandlers[MsgShotEnd]           = new ShotEndHandler;
  playerNetworkHandlers[MsgShotInfo]          = new ShotInfoHandler;
  playerNetworkHandlers[MsgHit]               = new HitHandler;
  playerNetworkHandlers[MsgTeleport]          = new TeleportHandler;
  playerNetworkHandlers[MsgMessage]           = new MessageHandler;
  playerNetworkHandlers[MsgTransferFlag]      = new TransferFlagHandler;
  playerNetworkHandlers[MsgNewRabbit]         = new NewRabbitHandler;
  playerNetworkHandlers[MsgPause]             = new PauseHandler;
  playerNetworkHandlers[MsgAutoPilot]         = new AutoPilotHandler;
  playerNetworkHandlers[MsgLagPing]           = new LagPingHandler;
  playerNetworkHandlers[MsgPlayerUpdate]      = new PlayerUpdateHandler;
  playerNetworkHandlers[MsgPlayerUpdateSmall] = new PlayerUpdateHandler;
  playerNetworkHandlers[MsgGMUpdate]          = new GMUpdateHandler;
  playerNetworkHandlers[MsgPlayerData]        = new PlayerDataHandler;
  playerNetworkHandlers[MsgLuaData]	      = new LuaDataHandler;
  playerNetworkHandlers[MsgLuaDataFast]       = new LuaDataHandler;
  playerNetworkHandlers[MsgQueryOS]	      = new QueryOSHandler;
}


void cleanupDefaultHandlers ( void )
{
  std::map<uint16_t, PlayerNetworkMessageHandler*>::iterator playerIter =
    playerNetworkHandlers.begin();
  while(playerIter != playerNetworkHandlers.end())
    delete((playerIter++)->second);

  playerNetworkHandlers.clear();

  std::map<uint16_t, ClientNetworkMessageHandler*>::iterator clientIter =
    clientNetworkHandlers.begin();
  while(clientIter != clientNetworkHandlers.end())
    delete((clientIter++)->second);

  clientNetworkHandlers.clear();
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
