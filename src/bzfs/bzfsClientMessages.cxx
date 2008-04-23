/* bzflag
* Copyright (c) 1993 - 2008 Tim Riker
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
#include "bzfsChatVerify.h"
#include "ShotManager.h"
#include "PhysicsDriver.h"

std::map<uint16_t,ClientNetworkMessageHandler*> clientNetworkHandlers;
std::map<uint16_t,PlayerNetworkMessageHandler*> playerNetworkHandlers;


void packWorldSettings ( void )
{
  if (!worldSettings)	// this stuff is static, so cache it once.
    worldSettings = (char*) malloc(4 + WorldSettingsSize);

  void* buffer = worldSettings;

  // the header
  buffer = nboPackUShort (buffer, WorldSettingsSize); // length
  buffer = nboPackUShort (buffer, MsgGameSettings);   // code

  // the settings
  buffer = nboPackFloat  (buffer, BZDBCache::worldSize);
  buffer = nboPackUShort (buffer, clOptions->gameType);
  buffer = nboPackUShort (buffer, clOptions->gameOptions);
  // An hack to fix a bug on the client
  buffer = nboPackUShort (buffer, PlayerSlot);
  buffer = nboPackUShort (buffer, clOptions->maxShots);
  buffer = nboPackUShort (buffer, numFlags);
  buffer = nboPackUShort (buffer, clOptions->shakeTimeout);
  buffer = nboPackUShort (buffer, clOptions->shakeWins);
}


// messages that don't have players
class WhatTimeIsItHandler : public ClientNetworkMessageHandler
{
public:
  virtual bool execute ( NetHandler *handler, uint16_t &/*code*/, void * buf, int len )
  {
    // the client wants to know what time we think it is.
    // he may have sent us a tag to ID the ping with ( for packet loss )
    // so we send that back to them with the time.
    // this is so the client can try and get a decent guess
    // at what our time is, and timestamp stuff with a real server
    // time, so everyone can go and compensate for some lag.
    unsigned char tag = 0;
    if (len >= 1)
      buf = nboUnpackUByte(buf,tag);

    float time = (float)TimeKeeper::getCurrent().getSeconds();

    logDebugMessage(4,"what time is it message from %s with tag %d\n",handler->getHostname(),tag);

    sendMsgWhatTimeIsIt(handler,tag,time);
    return true;
  }
};

class SetVarHandler : public ClientNetworkMessageHandler
{
public:
  virtual bool execute ( NetHandler *handler, uint16_t &/*code*/, void * /*buf*/, int /*len*/ )
  {
    void *bufStart = getDirectMessageBuffer();
    PackVars pv(bufStart, handler);
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
    for (int i = 0; i < numClientFlags; i++)
    {
      FlagType *fDesc;
      buf = FlagType::unpack(buf, fDesc);
      if (fDesc != Flags::Null)
	hasFlag[fDesc] = true;
    }

    /* Compare them to the flags this game might need, generating a list of missing flags */
    for (it = FlagType::getFlagMap().begin(); it != FlagType::getFlagMap().end(); ++it)
    {
      if (!hasFlag[it->second])
      {
	if (clOptions->flagCount[it->second] > 0)
	  missingFlags.insert(it->second);
	if ((clOptions->numExtraFlags > 0) && !clOptions->flagDisallowed[it->second])
	  missingFlags.insert(it->second);
      }
    }

    /* Pack a message with the list of missing flags */
    NetMsg msg = MSGMGR.newMessage();
    for (m_it = missingFlags.begin(); m_it != missingFlags.end(); ++m_it)
    {
      if ((*m_it) != Flags::Null)
      {
	if ((*m_it)->custom) 
	{
	  // custom flag, tell the client about it
	  static char cfbuffer[MaxPacketLen];
	  char *cfbufStart = &cfbuffer[0] + 2 * sizeof(uint16_t);
	  char *cfbuf = cfbufStart;
	  cfbuf = (char*)(*m_it)->packCustom(cfbuf);
	  NetMsg flagMsg = MSGMGR.newMessage();
	  flagMsg->addPackedData(cfbufStart,cfbuf-cfbufStart);
	  flagMsg->send(handler, MsgFlagType);
	}
	else
	{
	  // they should already know about this one, dump it back to them
	  (*m_it)->pack(msg); 
	}
      }
    }
    msg->send(handler, MsgNegotiateFlags);
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
    buf = nboUnpackUInt(buf, ptr);

    sendWorldChunk(handler, ptr);

    return true;
  }
};

class WantSettingsHandler : public ClientNetworkMessageHandler
{
public:
  virtual bool execute ( NetHandler *handler, uint16_t &/*code*/, void * /*buf*/, int /*len*/ )
  {
    if (!worldSettings)
      packWorldSettings();

    bz_pwrite(handler, worldSettings, 4 + WorldSettingsSize);
    return true;
  }
};

class WantWHashHandler : public ClientNetworkMessageHandler
{
public:
  virtual bool execute ( NetHandler *handler, uint16_t &/*code*/, void * /*buf*/, int /*len*/ )
  {
    if (clOptions->cacheURL.size() > 0)
    {
      NetMsg   msg = MSGMGR.newMessage();
      msg->packString(clOptions->cacheURL.c_str(), clOptions->cacheURL.size() + 1);
      msg->send(handler, MsgCacheURL);
    }
    NetMsg   msg = MSGMGR.newMessage();
    msg->packString(hexDigest, strlen(hexDigest)+1);
    msg->send(handler, MsgWantWHash);
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
    NetMsg   msg = MSGMGR.newMessage();
    msg->packUShort(pingReply.gameType);
    msg->packUShort(pingReply.gameOptions);
    msg->packUShort(pingReply.maxPlayers);
    msg->packUShort(pingReply.maxShots);
    msg->packUShort(team[0].team.size);
    msg->packUShort(team[1].team.size);
    msg->packUShort(team[2].team.size);
    msg->packUShort(team[3].team.size);
    msg->packUShort(team[4].team.size);
    msg->packUShort(team[5].team.size);
    msg->packUShort(pingReply.rogueMax);
    msg->packUShort(pingReply.redMax);
    msg->packUShort(pingReply.greenMax);
    msg->packUShort(pingReply.blueMax);
    msg->packUShort(pingReply.purpleMax);
    msg->packUShort(pingReply.observerMax);
    msg->packUShort(pingReply.shakeWins);
    // 1/10ths of second
    msg->packUShort(pingReply.shakeTimeout);
    msg->packUShort(pingReply.maxPlayerScore);
    msg->packUShort(pingReply.maxTeamScore);
    msg->packUShort(pingReply.maxTime);
    msg->packUShort((uint16_t)clOptions->timeElapsed);

    // send it
    msg->send(handler, MsgQueryGame);

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
    NetMsg   msg = MSGMGR.newMessage();

    msg->packUShort(NumTeams);
    msg->packUShort(numPlayers);

    msg->send(handler, MsgQueryPlayers);

    if (sendTeamUpdateDirect(handler) < 0)
      return true;

    GameKeeper::Player *otherData;
    for (int i = 0; i < curMaxPlayers; i++)
    {
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

class NewPlayerHandler : public ClientNetworkMessageHandler
{
public:
  virtual bool execute ( NetHandler *handler, uint16_t &/*code*/, void * /*buf*/, int /*len*/ )
  {
    PlayerId id = getNewPlayer(handler);
    if (id == 0xff)
      return false;

    NetMsg   msg = MSGMGR.newMessage();

    msg->packUByte(id);
    msg->send(handler, MsgNewPlayer);

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

    if ( len >= 1 ) // byte * 3
    {
      unsigned char temp = 0;
      buf  = nboUnpackUByte(buf, temp);
      player = GameKeeper::Player::getPlayerByIndex(temp);

      return buf;
    }
    return buf;
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

    buf = nboUnpackUByte(buf,temp);
    player->caps.canDownloadResources = temp != 0;

    buf = nboUnpackUByte(buf,temp);
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

    uint16_t rejectCode;
    char     rejectMsg[MessageLen];

    if (!player->player.unpackEnter(buf, rejectCode, rejectMsg))
    {
      rejectPlayer(player->getIndex(), rejectCode, rejectMsg);
      return true;
    }

    player->accessInfo.setName(player->player.getCallSign());
    std::string timeStamp = TimeKeeper::timestamp();
    std::string playerIP = "local.player";
    if ( player->netHandler )
      playerIP = player->netHandler->getTargetIP();

    logDebugMessage(1,"Player %s [%d] has joined from %s at %s with token \"%s\"\n",
      player->player.getCallSign(),
      player->getIndex(), playerIP.c_str(), timeStamp.c_str(),
      player->player.getToken());

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

    // player is on the waiting list
    char buffer[MessageLen];
    float waitTime = rejoinList.waitTime(player->getIndex());

    if (waitTime > 0.0f)
    {
      snprintf (buffer, MessageLen, "You are unable to begin playing for %.1f seconds.", waitTime);
      sendMessage(ServerPlayer, player->getIndex(), buffer);

      // Make them pay dearly for trying to rejoin quickly
      playerAlive(player->getIndex());
      smitePlayer(player->getIndex());
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
  virtual bool execute ( uint16_t &/*code*/, void * /*buf*/, int /*len*/ )
  {
    if (!player)
      return false;

    logDebugMessage(0,"KilledHandler called from %d",player->getIndex());
    return true;
  }
};

class SelfDestructHandler : public PlayerFirstHandler
{
public:
  virtual bool execute ( uint16_t &/*code*/, void * /*buf*/, int /*len*/ )
  {
    if (!player)
      return false;

    // you can't die stupid, you are dead
    if (!player->player.isAlive())
      return true;

    smitePlayer(player->getIndex(),SelfDestruct);
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

    // data: position of drop
    float pos[3];
    buf = nboUnpackVector(buf, pos);

    const float halfSize = BZDBCache::worldSize * 0.5f;
    if (fabsf(pos[0]) > halfSize || fabsf(pos[1]) > halfSize) {
      // client may be cheating
      const PlayerId id = player->getIndex();
      logDebugMessage(1,"Player %s [%d] dropped flag out of bounds to %f %f %f\n",
		      player->player.getCallSign(), id, pos[0], pos[1], pos[2]);
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

    // data: team whose territory flag was brought to
    uint16_t _team;
    buf = nboUnpackUShort(buf, _team);

    captureFlag(player->getIndex(), TeamColor(_team));
    return true;
  }
};

class CollideHandler : public PlayerFirstHandler
{
public:
  virtual bool execute ( uint16_t &/*code*/, void * buf, int len )
  {
    if (!player || len < 13)
      return false;

    PlayerId otherPlayer;
    buf = nboUnpackUByte(buf, otherPlayer);
    float collpos[3];
    buf = nboUnpackVector(buf, collpos);
    GameKeeper::Player *otherData = GameKeeper::Player::getPlayerByIndex(otherPlayer);

    processCollision(player,otherData,collpos);
    return true;
  }
};

class ShotBeginHandler : public PlayerFirstHandler
{
public:
  virtual bool execute ( uint16_t &/*code*/, void * buf, int len )
  {
    if (!player || len < 3)
      return false;

    FiringInfo firingInfo;

    uint16_t		id;
    void		*bufTmp;

    bufTmp = nboUnpackUShort(buf, id);

    // TODO, this should be made into a generic function that updates the state, so that others can add a firing info to the state
    firingInfo.shot.player = player->getIndex();
    firingInfo.shot.id     = id;

    firingInfo.shotType = player->efectiveShotType;

    const PlayerInfo &shooter = player->player;
    if (!shooter.isAlive() || shooter.isObserver())
      return true;

    FlagInfo &fInfo = *FlagInfo::get(shooter.getFlag());

    if (shooter.haveFlag())
      firingInfo.flagType = fInfo.flag.type;
    else
      firingInfo.flagType = Flags::Null;

    if (!player->canShoot())
    {
      logDebugMessage(2,"Player %s [%d] can not shoot yet\n", shooter.getCallSign(), firingInfo.shot.player);
      return true;
    }

    int guid = ShotManager::instance().newShot(&firingInfo);

    // send the player back a message that gives them the new GUID from the temp id they gave us
    sendMsgShotID(player->getIndex(),id,guid);
    firingInfo.shot.id = guid;

    // add the shot, send the GUID and the local ID in case we have to look it up
    if (!player->addShot(guid,id, firingInfo.timeSent))
    {
      // if it was no good for some reason, remove it before we track it
      ShotManager::instance().removeShot(guid,false);
      return true;
    }

    char message[MessageLen];
    if (shooter.haveFlag())
    {
      fInfo.numShots++; // increase the # shots fired
      int limit = clOptions->flagLimit[fInfo.flag.type];
      if (limit != -1)
      {
	// if there is a limit for players flag
	int shotsLeft = limit -  fInfo.numShots;

	if (shotsLeft > 0)
	{
	  //still have some shots left
	  // give message each shot below 5, each 5th shot & at start
	  if (shotsLeft % 5 == 0 || shotsLeft <= 3 || shotsLeft == limit-1)
	  {
	    if (shotsLeft > 1)
	      sprintf(message,"%d shots left",shotsLeft);
	    else
	      strcpy(message,"1 shot left");

	    sendMessage(ServerPlayer, player->getIndex(), message);
	  }
	}
	else
	{
	  // no shots left
	  if (shotsLeft == 0 || (limit == 0 && shotsLeft < 0))
	  {
	    // drop flag at last known position of player
	    // also handle case where limit was set to 0
	    float lastPos [3];
	    for (int i = 0; i < 3; i ++)
	      lastPos[i] = player->currentPos[i];

	    fInfo.grabs = 0; // recycle this flag now
	    dropPlayerFlag(*player, lastPos);
	  }
	  else
	  {
	    // more shots fired than allowed
	    // do nothing for now -- could return and not allow shot
	  }
	} // end no shots left
      } // end is limit
    } // end of player has flag

    // tell the API a shot was fired, it can not modify it at all
    bz_ShotFiredEventData_V1 shotEvent;

    shotEvent.pos[0] = firingInfo.shot.pos[0];
    shotEvent.pos[1] = firingInfo.shot.pos[1];
    shotEvent.pos[2] = firingInfo.shot.pos[2];
    shotEvent.playerID = (int)player->getIndex();

    shotEvent.type = firingInfo.flagType->flagAbbv;
    shotEvent.shotID = guid;

    worldEventManager.callEvents(bz_eShotFiredEvent,&shotEvent);

    sendMsgShotBegin(player->getIndex(),guid,firingInfo);

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

    if (player->player.isObserver())
      return true;

    int shot;
    uint16_t reason;
    buf = nboUnpackInt(buf, shot);
    buf = nboUnpackUShort(buf, reason);

    // ask the API if it wants to modify this shot
    bz_ShotEndedEventData_V1 shotEvent;
    shotEvent.playerID = (int)player->getIndex();
    shotEvent.shotID = shot;
    shotEvent.explode = reason == 0;
    worldEventManager.callEvents(bz_eShotEndedEvent,&shotEvent);

    FiringInfo firingInfo;
    player->removeShot(shot);
    ShotManager::instance().removeShot(shot,false);

    sendMsgShotEnd(player->getIndex(),shot,reason);

    return true;
  }
};
class HitDriverHandler : public PlayerFirstHandler
{
public:
  virtual bool execute ( uint16_t &/*code*/, void * buf, int len )
  {
    if (!player || len < 4)
      return false;

    int	driverID = -1;
    buf = nboUnpackInt(buf, driverID);

    const PhysicsDriver *driver = PHYDRVMGR.getDriver(driverID);
    if (!driver)
      return true;

    if (driver->getIsDeath())
      playerKilled(player->getIndex(),PhysicsDriverDeath,driverID);
  }
};

class HitHandler : public PlayerFirstHandler
{
public:
  virtual bool execute ( uint16_t &/*code*/, void * buf, int len )
  {
    if (!player || len < 5)
      return false;

    if (player->player.isObserver() || !player->player.isAlive())
      return true; // you can't be hit stupid

    PlayerId hitPlayer = player->getIndex();
    unsigned char isAShot;
    FiringInfo firingInfo;

    int id = 0;

    buf = nboUnpackUByte(buf, isAShot);
    buf = nboUnpackInt(buf, id);

    if (isAShot)
    {
      // ok try to find the shot
      if ( id < 0 ) // damn it's a local shot, search the shooter and see what the global shotID for this guy is. it has to be his local shot
      {
	id = player->findShot(id);
	if (id < 0 )
	  return false;	  // we don't have track of that local shot anymore, so go and tell him to stuff it.
      }
      ShotManager::Shot *shot = ShotManager::instance().getShot(id);
      if (!shot)
	return true; // yeah bad shot screw it

      GameKeeper::Player *shooterData = GameKeeper::Player::getPlayerByIndex(shot->player);

      if (!shooterData)
	return true;

      // TODO verify the shot, make sure the shot is near them etc..

      ShotManager::instance().removeShot(id,false);
      if (shooterData->removeShot(id))
      {
	sendMsgShotEnd( shot, 1);

	FlagInfo *flagInfo = FlagInfo::get(player->player.getFlag());
	if (!flagInfo || flagInfo != Flags::Shield)
	  playerKilled(hitPlayer, GotShot, id, false);
	else
	  zapFlagByPlayer(hitPlayer);
      }
    }
    else // steam roller
      playerKilled(hitPlayer,GotRunOver,id);

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

    uint16_t from, to;

    if (invalidPlayerAction(player->player, player->getIndex(), "teleport"))
      return true;

    buf = nboUnpackUShort(buf, from);
    buf = nboUnpackUShort(buf, to);

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

    // data: target player/team, message string
    PlayerId dstPlayer;
    char message[MessageLen];

    buf = nboUnpackUByte(buf, dstPlayer);
    buf = nboUnpackString(buf, message, sizeof(message));
    message[MessageLen - 1] = '\0';

    player->player.hasSent();
    if (dstPlayer == AllPlayers)
      logDebugMessage(1,"Player %s [%d] -> All: %s\n", player->player.getCallSign(), player->getIndex(), message);
    else
    {
      if (dstPlayer == AdminPlayers)
	logDebugMessage(1,"Player %s [%d] -> Admin: %s\n",player->player.getCallSign(), player->getIndex(), message);
      else
      {
	if (dstPlayer > LastRealPlayer)
	  logDebugMessage(1,"Player %s [%d] -> Team: %s\n",player->player.getCallSign(),  player->getIndex(), message);
	else
	{
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

    PlayerId from, to;

    from = player->getIndex();

    buf = nboUnpackUByte(buf, to);

    int flagIndex = player->player.getFlag();
    if (to == ServerPlayer)
    {
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

    worldEventManager.callEvents(bz_eFlagTransferredEvent,&eventData);

    if (eventData.action != eventData.CancelSteal)
    {
      int oFlagIndex = toData->player.getFlag();
      if (oFlagIndex >= 0)
	zapFlag (*FlagInfo::get(oFlagIndex));
    }

    if (eventData.action == eventData.ContinueSteal)
      sendFlagTransferMessage(to,from,*FlagInfo::get(flagIndex));
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

    if (player->getIndex() == rabbitIndex)
      anointNewRabbit();

    return true;
  }
};

class PauseHandler : public PlayerFirstHandler
{
public:
  virtual bool execute ( uint16_t &/*code*/, void * /*buf*/, int /*len*/ )
  {
    if (!player)
      return false;

    if (player->player.pauseRequestTime - TimeKeeper::getNullTime() != 0)
    {
      // player wants to unpause
      player->player.pauseRequestTime = TimeKeeper::getNullTime();
      pausePlayer(player->getIndex(), false);
    }
    else
    {
      // player wants to pause
      player->player.pauseRequestTime = TimeKeeper::getCurrent();

      // adjust pauseRequestTime according to players lag to avoid kicking innocent players
      int requestLag = player->lagInfo.getLag();
      if (requestLag < 100)
	requestLag = 250;
      else
	requestLag *= 2;

      player->player.pauseRequestLag = requestLag;
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

    uint8_t autopilot;
    nboUnpackUByte(buf, autopilot);

    player->player.setAutoPilot(autopilot != 0);

    sendMsgAutoPilot(player->getIndex(),autopilot);

    return true;
  }
};

class LagPingHandler : public PlayerFirstHandler
{
public:
  virtual bool execute ( uint16_t &/*code*/, void * buf, int len )
  {
    if (!player || len < 3)
      return false;

    bool warn, kick, jittwarn, jittkick, plosswarn, plosskick;
    char message[MessageLen];

    player->lagInfo.updatePingLag(buf, warn, kick, jittwarn, jittkick, plosswarn, plosskick);

    if (warn)
    {
      sprintf(message,"*** Server Warning: your lag is too high (%d ms) ***", player->lagInfo.getLag());
      sendMessage(ServerPlayer, player->getIndex(), message);

      if (kick)
	lagKick(player->getIndex());
    }

    if (jittwarn)
    {
      sprintf(message, "*** Server Warning: your jitter is too high (%d ms) ***", player->lagInfo.getJitter());
      sendMessage(ServerPlayer, player->getIndex(), message);

      if (jittkick)
	jitterKick(player->getIndex());
    }

    if (plosswarn)
    {
      sprintf(message, "*** Server Warning: your packetloss is too high (%d%%) ***", player->lagInfo.getLoss());
      sendMessage(ServerPlayer, player->getIndex(), message);

      if (plosskick)
	packetLossKick(player->getIndex());
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
    else if(len < 27)
      return false;

    float       timestamp;
    PlayerState state;

    buf = nboUnpackFloat(buf, timestamp);
    buf = state.unpack(buf, code);

    updatePlayerState(player, state, timestamp, code == MsgPlayerUpdateSmall);

    return true;
  }
};

class PlayerFirstNoBumpHandler : public PlayerFirstHandler
{
public:
  virtual void *unpackPlayer ( void * buf, int len )
  {
    player = NULL;

    if ( len >= 1 ) // byte * 3
    {
      unsigned char temp = 0;
      nboUnpackUByte(buf, temp);
      player = GameKeeper::Player::getPlayerByIndex(temp);

      return buf;
    }
    return buf;
  }
};

void registerDefaultHandlers ( void )
{
  clientNetworkHandlers[MsgWhatTimeIsIt] = new WhatTimeIsItHandler;
  clientNetworkHandlers[MsgSetVar] = new SetVarHandler;
  clientNetworkHandlers[MsgNegotiateFlags] = new NegotiateFlagHandler;
  clientNetworkHandlers[MsgGetWorld] = new GetWorldHandler;
  clientNetworkHandlers[MsgWantSettings] = new WantSettingsHandler;
  clientNetworkHandlers[MsgWantWHash] = new WantWHashHandler;
  clientNetworkHandlers[MsgQueryGame] = new QueryGameHandler;
  clientNetworkHandlers[MsgQueryPlayers] = new QueryPlayersHandler;
  clientNetworkHandlers[MsgUDPLinkEstablished] = new UDPLinkEstablishedHandler;
  clientNetworkHandlers[MsgNewPlayer] = new NewPlayerHandler;

  playerNetworkHandlers[MsgCapBits] = new CapBitsHandler;
  playerNetworkHandlers[MsgEnter] = new EnterHandler;
  playerNetworkHandlers[MsgExit] = new ExitHandler;
  playerNetworkHandlers[MsgAlive] = new AliveHandler;
  playerNetworkHandlers[MsgKilled] = new KilledHandler;
  playerNetworkHandlers[MsgSelfDestruct] = new SelfDestructHandler;
  playerNetworkHandlers[MsgDropFlag] = new DropFlagHandler;
  playerNetworkHandlers[MsgCaptureFlag] = new CaptureFlagHandler;
  playerNetworkHandlers[MsgCollide] = new CollideHandler;
  playerNetworkHandlers[MsgShotBegin] = new ShotBeginHandler;
  playerNetworkHandlers[MsgShotEnd] = new ShotEndHandler;
  playerNetworkHandlers[MsgHit] = new HitHandler;
  playerNetworkHandlers[MsgHitDriver] = new HitDriverHandler;
  playerNetworkHandlers[MsgTeleport] = new TeleportHandler;
  playerNetworkHandlers[MsgMessage] = new MessageHandler;
  playerNetworkHandlers[MsgTransferFlag] = new TransferFlagHandler;
  playerNetworkHandlers[MsgNewRabbit] = new NewRabbitHandler;
  playerNetworkHandlers[MsgPause] = new PauseHandler;
  playerNetworkHandlers[MsgAutoPilot] = new AutoPilotHandler;
  playerNetworkHandlers[MsgLagPing] = new LagPingHandler;
  playerNetworkHandlers[MsgPlayerUpdate] = new PlayerUpdateHandler;
  playerNetworkHandlers[MsgPlayerUpdateSmall] = new PlayerUpdateHandler;
}

void cleanupDefaultHandlers ( void )
{
  std::map<uint16_t,PlayerNetworkMessageHandler*>::iterator playerIter = playerNetworkHandlers.begin();
  while(playerIter != playerNetworkHandlers.end())
    delete((playerIter++)->second);

  playerNetworkHandlers.clear();

  std::map<uint16_t,ClientNetworkMessageHandler*>::iterator clientIter = clientNetworkHandlers.begin();
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
