/* bzflag
* Copyright (c) 1993 - 2007 Tim Riker
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

std::map<uint16_t,ClientNetworkMessageHandler*> clientNeworkHandlers;
std::map<uint16_t,PlayerNetworkMessageHandler*> playerNeworkHandlers;

bool isPlayerMessage ( uint16_t &code )
{
  switch (code)
  {
    case MsgEnter:
    case MsgExit:
    case MsgAlive:
    case MsgKilled:
    case MsgGrabFlag:
    case MsgDropFlag:
    case MsgCaptureFlag:
    case MsgShotEnd:
    case MsgHit:
    case MsgTeleport:
    case MsgMessage:
    case MsgTransferFlag:
    case MsgPause:
    case MsgAutoPilot:
    case MsgLagPing:
    case MsgNewRabbit:
    case MsgPlayerUpdate:
    case MsgPlayerUpdateSmall:
    case MsgCollide:
    case MsgGMUpdate:
    case MsgShotBegin:
    case MsgCapBits:
      return true;
  }
  return false;
}

GameKeeper::Player *getPlayerMessageInfo ( void **buffer, uint16_t &code, int &playerID )
{
  if (isPlayerMessage(code))
  {
    uint8_t id;
    *buffer  = nboUnpackUByte(*buffer, id);
    playerID = id;
    if (code == MsgGMUpdate || code == MsgShotBegin) // the player was in the shot, don't move past it, the shot unpack needs it
      *buffer--;
    return GameKeeper::Player::getPlayerByIndex(playerID);
  }
  return NULL;
}

void handlePlayerUpdate(void **buf, uint16_t &code,
						GameKeeper::Player *playerData, const void *, int)
{
	if (!playerData)
		return;

	float       timestamp;
	PlayerState state;

	*buf = nboUnpackFloat(*buf, timestamp);
	*buf = state.unpack(*buf, code);

	updatePlayerState(playerData, state, timestamp,
		code == MsgPlayerUpdateSmall);
}

bool updatePlayerState(GameKeeper::Player *playerData, PlayerState &state, float timeStamp, bool shortState)
{
	// observer updates are not relayed, or checked
	if (playerData->player.isObserver())
	{
		// skip all of the checks
		playerData->setPlayerState(state, timeStamp);
		return true;
	}

	bz_PlayerUpdateEventData_V1 eventData;
	playerStateToAPIState(eventData.state,state);
	eventData.stateTime = timeStamp;
	eventData.eventTime = TimeKeeper::getCurrent().getSeconds();
	eventData.player = playerData->getIndex();
	worldEventManager.callEvents(bz_ePlayerUpdateEvent,&eventData);

	// silently drop old packet
	if (state.order <= playerData->lastState.order)
		return true;

	if(!validatePlayerState(playerData,state))
		return false;

	playerData->setPlayerState(state, timeStamp);

	// Player might already be dead and did not know it yet (e.g. teamkill)
	// do not propogate
	if (!playerData->player.isAlive() && (state.status & short(PlayerState::Alive)))
		return true;

	searchFlag(*playerData);

	sendPlayerStateMessage(playerData,shortState);
	return true;
}

void handlePlayerFlagDrop( GameKeeper::Player *playerData, void* buffer )
{
	// data: position of drop
	float pos[3];
	buffer = nboUnpackVector(buffer, pos);

	dropPlayerFlag(*playerData, pos);
}

void handlePlayerMessage ( GameKeeper::Player *playerData, void* buffer )
{
	// data: target player/team, message string
	PlayerId dstPlayer;
	char message[MessageLen];

	buffer = nboUnpackUByte(buffer, dstPlayer);
	buffer = nboUnpackString(buffer, message, sizeof(message));
	message[MessageLen - 1] = '\0';

	playerData->player.hasSent();
	if (dstPlayer == AllPlayers)
	{
		logDebugMessage(1,"Player %s [%d] -> All: %s\n", playerData->player.getCallSign(), playerData->getIndex(), message);
	}
	else
	{
		if (dstPlayer == AdminPlayers)
		{
			logDebugMessage(1,"Player %s [%d] -> Admin: %s\n",playerData->player.getCallSign(), playerData->getIndex(), message);
		}
		else
		{
			if (dstPlayer > LastRealPlayer)
			{
				logDebugMessage(1,"Player %s [%d] -> Team: %s\n",playerData->player.getCallSign(),
					playerData->getIndex(), message);
			}
			else
			{
				GameKeeper::Player *p = GameKeeper::Player::getPlayerByIndex(dstPlayer);
				if (p != NULL)
				{
					logDebugMessage(1,"Player %s [%d] -> Player %s [%d]: %s\n",playerData->player.getCallSign(),
						playerData->getIndex(), p->player.getCallSign(), dstPlayer, message);
				}
				else
				{
					logDebugMessage(1,"Player %s [%d] -> Player Unknown [%d]: %s\n",
						playerData->player.getCallSign(), playerData->getIndex(), dstPlayer, message);
				}
			}
		}
	}
	// check for spamming
	if (checkChatSpam(message, playerData, playerData->getIndex()))
		return;

	// check for garbage
	if (checkChatGarbage(message, playerData, playerData->getIndex()))
		return;

	sendPlayerMessage(playerData, dstPlayer, message);
}

void handleFlagCapture ( GameKeeper::Player *playerData, void* buffer)
{
	// data: team whose territory flag was brought to
	uint16_t _team;
	buffer = nboUnpackUShort(buffer, _team);

	captureFlag(playerData->getIndex(), TeamColor(_team));
}

void handleFlagTransfer ( GameKeeper::Player *playerData, void* buffer)
{
	PlayerId from, to;

	from = playerData->getIndex();

	buffer = nboUnpackUByte(buffer, to);

	GameKeeper::Player *fromData = playerData;

	int flagIndex = fromData->player.getFlag();
	if (to == ServerPlayer) 
	{
		if (flagIndex >= 0)
			zapFlag (*FlagInfo::get(flagIndex));
		return;
	}

	// Sanity check
	if (to >= curMaxPlayers)
		return;

	if (flagIndex == -1)
		return;

	GameKeeper::Player *toData = GameKeeper::Player::getPlayerByIndex(to);
	if (!toData)
		return;

	bz_FlagTransferredEventData_V1 eventData;

	eventData.fromPlayerID = fromData->player.getPlayerIndex();
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
}

void handleShotEnded ( GameKeeper::Player *playerData, void *buf, int len )
{
	if ( len != 3 || playerData->player.isObserver())
		return;

	// data: shooter id, shot number, reason
	PlayerId sourcePlayer = playerData->getIndex();

	int16_t shot;
	uint16_t reason;
	buf = nboUnpackShort(buf, shot);
	buf = nboUnpackUShort(buf, reason);

	// ask the API if it wants to modify this shot
	bz_ShotEndedEventData_V1 shotEvent;
	shotEvent.playerID = (int)sourcePlayer;
	shotEvent.shotID = shot;
	shotEvent.exlpode = reason == 0;
	worldEventManager.callEvents(bz_eShotEndedEvent,&shotEvent);

	FiringInfo firingInfo;
	playerData->removeShot(shot & 0xff, shot >> 8, firingInfo);

	sendMsgShotEnd(sourcePlayer,shot,reason);
}

void handleShotFired(void *buf, int len )
{
  // Sanity check
  if (len != 3)
	  return;

  FiringInfo firingInfo;

  PlayerId		player;
  uint16_t		id;
  void                 *bufTmp;

  bufTmp = nboUnpackUByte(buf, player);
  bufTmp = nboUnpackUShort(bufTmp, id);

  firingInfo.shot.player = player;
  firingInfo.shot.id     = id;

  int playerIndex = player;

  // verify playerId
  GameKeeper::Player *playerData = GameKeeper::Player::getPlayerByIndex(playerIndex);
  if (!playerData)
    return;

  firingInfo.shotType = playerData->efectiveShotType;

  const PlayerInfo &shooter = playerData->player;
  if (!shooter.isAlive() || shooter.isObserver())
    return;

  FlagInfo &fInfo = *FlagInfo::get(shooter.getFlag());

  if (shooter.haveFlag())
    firingInfo.flagType = fInfo.flag.type;
  else
    firingInfo.flagType = Flags::Null;

  if (!playerData->addShot(id & 0xff, id >> 8, firingInfo))
    return;

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

	  sendMessage(ServerPlayer, playerIndex, message);
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
	    lastPos[i] = playerData->currentPos[i];

	  fInfo.grabs = 0; // recycle this flag now
	  dropPlayerFlag(*playerData, lastPos);
	}
	else
	{
	  // more shots fired than allowed
	  // do nothing for now -- could return and not allow shot
	}
      } // end no shots left
    } // end is limit
  } // end of player has flag

  // ask the API if it wants to modify this shot
  bz_ShotFiredEventData_V1 shotEvent;

  shotEvent.pos[0] = firingInfo.shot.pos[0];
  shotEvent.pos[1] = firingInfo.shot.pos[1];
  shotEvent.pos[2] = firingInfo.shot.pos[2];
  shotEvent.player = (int)player;

  shotEvent.type = firingInfo.flagType->flagAbbv;

  worldEventManager.callEvents(bz_eShotFiredEvent,&shotEvent);

  sendMsgShotBegin(player,id,firingInfo);
}

void handleTankHit ( GameKeeper::Player *playerData, void *buf, int len )
{
  if (playerData->player.isObserver() || !playerData->player.isAlive())
    return;

  PlayerId hitPlayer = playerData->getIndex();
  PlayerId shooterPlayer;
  FiringInfo firingInfo;
  int16_t shot;

  buf = nboUnpackUByte(buf, shooterPlayer);
  buf = nboUnpackShort(buf, shot);
  GameKeeper::Player *shooterData = GameKeeper::Player::getPlayerByIndex(shooterPlayer);

  if (!shooterData)
    return;

  if (shooterData->removeShot(shot & 0xff, shot >> 8, firingInfo))
  {
    sendMsgShotEnd(shooterPlayer, shot, 1);
    
    const int flagIndex = playerData->player.getFlag();
    FlagInfo *flagInfo  = NULL;
   
    if (flagIndex >= 0) 
    {
      flagInfo = FlagInfo::get(flagIndex);
      dropFlag(*flagInfo);
    }

    if (!flagInfo || flagInfo->flag.type != Flags::Shield)
      playerKilled(hitPlayer, shooterPlayer, GotShot, shot, firingInfo.flagType, false, false);
  }
}

void handleTeleport( GameKeeper::Player *playerData, void *buf, int len)
{
  uint16_t from, to;

  if (invalidPlayerAction(playerData->player, playerData->getIndex(), "teleport"))
    return;

  buf = nboUnpackUShort(buf, from);
  buf = nboUnpackUShort(buf, to);
  
  sendMsgTeleport(playerData->getIndex(), from, to);
}

void handleRabbitMessage( GameKeeper::Player *playerData )
{
  if (playerData->getIndex() == rabbitIndex)
     anointNewRabbit();
}

void handlePauseMessage( GameKeeper::Player *playerData, void *buf, int len )
{
  if (playerData->player.pauseRequestTime - TimeKeeper::getNullTime() != 0)
  {
    // player wants to unpause
    playerData->player.pauseRequestTime = TimeKeeper::getNullTime();
    pausePlayer(playerData->getIndex(), false);
  }
  else
  {
    // player wants to pause
    playerData->player.pauseRequestTime = TimeKeeper::getCurrent();

    // adjust pauseRequestTime according to players lag to avoid kicking innocent players
    int requestLag = playerData->lagInfo.getLag();
    if (requestLag < 100)
      requestLag = 250;
    else
      requestLag *= 2;
   
    playerData->player.pauseRequestLag = requestLag;
  }
}

void handleAutoPilotMessage( GameKeeper::Player *playerData, void *buf, int len )
{
  uint8_t autopilot;
  nboUnpackUByte(buf, autopilot);
  
  playerData->player.setAutoPilot(autopilot != 0);

  sendMsgAutoPilot(playerData->getIndex(),autopilot);
}

void handleLagPing( GameKeeper::Player *playerData, void *buf, int len )
{
  bool warn, kick, jittwarn, jittkick, plosswarn, plosskick;
  char message[MessageLen];
  
  playerData->lagInfo.updatePingLag(buf, warn, kick, jittwarn, jittkick, plosswarn, plosskick);
  
  if (warn)
  {
    sprintf(message,"*** Server Warning: your lag is too high (%d ms) ***", playerData->lagInfo.getLag());
    sendMessage(ServerPlayer, playerData->getIndex(), message);
    
    if (kick)
      lagKick(playerData->getIndex());
  }

  if (jittwarn) 
  {
    sprintf(message, "*** Server Warning: your jitter is too high (%d ms) ***", playerData->lagInfo.getJitter());
    sendMessage(ServerPlayer, playerData->getIndex(), message);
    
    if (jittkick)
      jitterKick(playerData->getIndex());
  }

  if (plosswarn) 
  {
    sprintf(message, "*** Server Warning: your packetloss is too high (%d%%) ***", playerData->lagInfo.getLoss());
    sendMessage(ServerPlayer, playerData->getIndex(), message);
   
    if (plosskick)
      packetLossKick(playerData->getIndex());
  }
}

void handleShotUpdate ( GameKeeper::Player *playerData, void *buf, int len )
{
  ShotUpdate shot;
  shot.unpack(buf);

  if (!playerData->player.isAlive() || playerData->player.isObserver())
    return;

  if (!playerData->updateShot(shot.id & 0xff, shot.id >> 8))
    return;

  sendMsgGMUpdate( playerData->getIndex(), &shot );
}

const float *closestBase( TeamColor color, float *position )
{
  float bestdist = Infinity;
  const float *bestbase = NULL;

  if (bases.find(color) == bases.end()) {
    return NULL;
  }

  TeamBases &teamBases = bases[color];
  int count = teamBases.size();
  for (int i=0; i<count; i++) {
    const float *basepos = teamBases.getBasePosition(i);
    float dx = position[0] - basepos[0];
    float dy = position[1] - basepos[1];
    float dist = sqrt(dx * dx + dy * dy);

    if (dist < bestdist) {
      bestbase = basepos;
      bestdist = dist;
    }
  }

  return bestbase;
}

void handleCollide ( GameKeeper::Player *playerData, void* buffer)
{
  if (!playerData) {
    std::cerr << "Invalid MsgCollide (no such player)\n";
    return;
  }

  if (clOptions->gameOptions & FreezeTagGameStyle) {
    TeamColor playerTeam = playerData->player.getTeam();

    PlayerId otherPlayer;
    buffer = nboUnpackUByte(buffer, otherPlayer);
    GameKeeper::Player *otherData =
	  GameKeeper::Player::getPlayerByIndex(otherPlayer);
    TeamColor otherTeam = otherData->player.getTeam();

    float collpos[3];
    buffer = nboUnpackVector(buffer, collpos);

    if (playerTeam == otherTeam) {
      // unfreeze
      if (!playerData->player.canShoot() || !playerData->player.canMove()) {
	sendMessageAllow(playerData->getIndex(), true, true);
	playerData->player.setAllowShooting(true);
	playerData->player.setAllowMovement(true);
      }
    } else {
      float dx, dy, dist, angle, cos_angle;
      const float *playerBase = closestBase(playerTeam, collpos);
      const float *otherBase = closestBase(otherTeam, collpos);

      if (!playerBase || !otherBase) {
	return;
      }

      angle = atan2f(otherBase[1] - playerBase[1],
		otherBase[0] - playerBase[0]);
      cos_angle = fabs(cosf(angle));

      dx = collpos[0] - playerBase[0];
      dy = collpos[1] - playerBase[1];
      dist = sqrt(dx * dx + dy * dy);
      float playerDist = dist * cos_angle;

      dx = collpos[0] - otherBase[0];
      dy = collpos[1] - otherBase[1];
      dist = sqrt(dx * dx + dy * dy);
      float otherDist = dist * cos_angle;

      if (playerDist - otherDist > 2.0 * BZDBCache::dmzWidth) {
	// freeze
	if (playerData->player.canShoot() || playerData->player.canMove()) {
	  sendMessageAllow(playerData->getIndex(), false, false);
	  playerData->player.setAllowMovement(false);
	  playerData->player.setAllowShooting(false);
	}
      }

    }
  }
}

// messages that don't have players
class WhatTimeIsItHandler : public ClientNetworkMessageHandler
{
public:
  virtual bool execute ( NetHandler *handler, uint16_t &code, void * buf, int len )
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
  virtual bool execute ( NetHandler *handler, uint16_t &code, void * buf, int len )
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
  virtual bool execute ( NetHandler *handler, uint16_t &code, void * buf, int len )
  {
    void *bufStart;
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
    void *buf2 = bufStart = getDirectMessageBuffer();
    for (m_it = missingFlags.begin(); m_it != missingFlags.end(); ++m_it)
    {
      if ((*m_it) != Flags::Null)
	buf2 = (*m_it)->pack(buf2);
    }
    directMessage(handler, MsgNegotiateFlags, (char*)buf2-(char*)bufStart, bufStart);
    return true;
  }
};

class GetWorldHandler : public ClientNetworkMessageHandler
{
public:
  virtual bool execute ( NetHandler *handler, uint16_t &code, void * buf, int len )
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
  virtual bool execute ( NetHandler *handler, uint16_t &code, void * buf, int len )
  {
    if (!worldSettings)	// this stuff is static, so cache it once.
    {
      worldSettings = (char*) malloc(4 + WorldSettingsSize);
      
      void* buffer = worldSettings;

      // the header
      buffer = nboPackUShort (buffer, WorldSettingsSize); // length
      buffer = nboPackUShort (buffer, MsgGameSettings);   // code

      // the settings
      buf = nboPackFloat  (buffer, BZDBCache::worldSize);
      buf = nboPackUShort (buffer, clOptions->gameType);
      buf = nboPackUShort (buffer, clOptions->gameOptions);
      // An hack to fix a bug on the client
      buffer = nboPackUShort (buffer, PlayerSlot);
      buffer = nboPackUShort (buffer, clOptions->maxShots);
      buffer = nboPackUShort (buffer, numFlags);
      buffer = nboPackUShort (buffer, clOptions->shakeTimeout);
      buffer = nboPackUShort (buffer, clOptions->shakeWins);
      buffer = nboPackUInt   (buffer, 0); // FIXME - used to be sync time
    }

    bz_pwrite(handler, worldSettings, 4 + WorldSettingsSize);
    return true;
  }
};

class WantWHashHandler : public ClientNetworkMessageHandler
{
public:
  virtual bool execute ( NetHandler *handler, uint16_t &code, void * buf, int len )
  {
    void *obuf, *obufStart = getDirectMessageBuffer();
    if (clOptions->cacheURL.size() > 0)
    {
      obuf = nboPackString(obufStart, clOptions->cacheURL.c_str(), clOptions->cacheURL.size() + 1);
      directMessage(handler, MsgCacheURL, (char*)obuf-(char*)obufStart, obufStart);
    }
    obuf = nboPackString(obufStart, hexDigest, strlen(hexDigest)+1);
    directMessage(handler, MsgWantWHash, (char*)obuf-(char*)obufStart, obufStart);
    return true;
  }
};

class QueryGameHandler : public ClientNetworkMessageHandler
{
public:
  virtual bool execute ( NetHandler *handler, uint16_t &code, void * buf, int len )
  {
    // much like a ping packet but leave out useless stuff (like
    // the server address, which must already be known, and the
    // server version, which was already sent).
    void *buffer, *bufStart = getDirectMessageBuffer();
    buffer = nboPackUShort(bufStart, pingReply.gameType);
    buffer = nboPackUShort(buffer, pingReply.gameOptions);
    buffer = nboPackUShort(buffer, pingReply.maxPlayers);
    buffer = nboPackUShort(buffer, pingReply.maxShots);
    buffer = nboPackUShort(buffer, team[0].team.size);
    buffer = nboPackUShort(buffer, team[1].team.size);
    buffer = nboPackUShort(buffer, team[2].team.size);
    buffer = nboPackUShort(buffer, team[3].team.size);
    buffer = nboPackUShort(buffer, team[4].team.size);
    buffer = nboPackUShort(buffer, team[5].team.size);
    buffer = nboPackUShort(buffer, pingReply.rogueMax);
    buffer = nboPackUShort(buffer, pingReply.redMax);
    buffer = nboPackUShort(buffer, pingReply.greenMax);
    buffer = nboPackUShort(buffer, pingReply.blueMax);
    buffer = nboPackUShort(buffer, pingReply.purpleMax);
    buffer = nboPackUShort(buffer, pingReply.observerMax);
    buffer = nboPackUShort(buffer, pingReply.shakeWins);
    // 1/10ths of second
    buffer = nboPackUShort(buffer, pingReply.shakeTimeout);
    buffer = nboPackUShort(buffer, pingReply.maxPlayerScore);
    buffer = nboPackUShort(buffer, pingReply.maxTeamScore);
    buffer = nboPackUShort(buffer, pingReply.maxTime);
    buffer = nboPackUShort(buffer, (uint16_t)clOptions->timeElapsed);

    // send it
    directMessage(handler, MsgQueryGame, (char*)buf-(char*)bufStart, bufStart);

    return true;
  }
};

class QueryPlayersHandler : public ClientNetworkMessageHandler
{
public:
  virtual bool execute ( NetHandler *handler, uint16_t &code, void * buf, int len )
  {
    // count the number of active players
    int numPlayers = GameKeeper::Player::count();

    // first send number of teams and players being sent
    void *buffer, *bufStart = getDirectMessageBuffer();
    buffer = nboPackUShort(bufStart, NumTeams);
    buffer = nboPackUShort(buffer, numPlayers);
    
    if (directMessage(handler, MsgQueryPlayers,(char*)buffer-(char*)bufStart, bufStart) < 0)
      return true;
   
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
  virtual bool execute ( uint16_t &code, void * buf, int len )
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
  virtual bool execute ( uint16_t &code, void * buf, int len )
  {
    if (!player || len < 247)
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
  virtual bool execute ( uint16_t &code, void * buf, int len )
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
  virtual bool execute ( uint16_t &code, void * buf, int len )
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
  virtual bool execute ( uint16_t &code, void * buf, int len )
  {
    if (!player || len < 7)
      return false;
   
    if (player->player.isObserver())
      return true;

    // data: id of killer, shot id of killer
    PlayerId killer;
    FlagType* flagType;
    int16_t shot, reason;
    int phydrv = -1;

    buf = nboUnpackUByte(buf, killer);
    buf = nboUnpackShort(buf, reason);
    buf = nboUnpackShort(buf, shot);
    buf = FlagType::unpack(buf, flagType);

    if (reason == PhysicsDriverDeath)
    {
      int32_t inPhyDrv;
      buf = nboUnpackInt(buf, inPhyDrv);
      phydrv = int(inPhyDrv);
    }

    if (killer != ServerPlayer)	// Sanity check on shot: Here we have the killer
    {
      int si = (shot == -1 ? -1 : shot & 0x00FF);
      if ((si < -1) || (si >= clOptions->maxShots))
	return true;
    }

    player->player.endShotCredit--;
    playerKilled(player->getIndex(), lookupPlayer(killer), (BlowedUpReason)reason, shot, flagType, phydrv);

    // stop pausing attempts as you can not pause when being dead
    player->player.pauseRequestTime = TimeKeeper::getNullTime();
    return true;
  }
};

void registerDefaultHandlers ( void )
{ 
  clientNeworkHandlers[MsgWhatTimeIsIt] = new WhatTimeIsItHandler;
  clientNeworkHandlers[MsgSetVar] = new SetVarHandler;
  clientNeworkHandlers[MsgNegotiateFlags] = new NegotiateFlagHandler;
  clientNeworkHandlers[MsgGetWorld] = new GetWorldHandler;
  clientNeworkHandlers[MsgWantSettings] = new WantSettingsHandler;
  clientNeworkHandlers[MsgWantWHash] = new WantWHashHandler;
  clientNeworkHandlers[MsgQueryGame] = new QueryGameHandler;
  clientNeworkHandlers[MsgQueryPlayers] = new QueryPlayersHandler;

  playerNeworkHandlers[MsgCapBits] = new CapBitsHandler;
  playerNeworkHandlers[MsgEnter] = new EnterHandler;
  playerNeworkHandlers[MsgExit] = new ExitHandler;
  playerNeworkHandlers[MsgAlive] = new AliveHandler;
  playerNeworkHandlers[MsgKilled] = new KilledHandler;
}

void cleanupDefaultHandlers ( void )
{
  std::map<uint16_t,PlayerNetworkMessageHandler*>::iterator playerIter = playerNeworkHandlers.begin();
  while(playerIter != playerNeworkHandlers.end())
    delete((playerIter++)->second);

  playerNeworkHandlers.clear();

  std::map<uint16_t,ClientNetworkMessageHandler*>::iterator clientIter = clientNeworkHandlers.begin();
  while(clientIter != clientNeworkHandlers.end())
    delete((clientIter++)->second);

  clientNeworkHandlers.clear();
}

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
