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


void handleWhatTimeMessage( NetHandler *handler, void* buf, uint16_t len )
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

	/* Pack a message with the list of missing flags */
	void *bufStart;
	void *buf2 = bufStart = getDirectMessageBuffer();
	buf2 = nboPackUByte(bufStart,tag);
	buf2 = nboPackFloat(buf2,time);

	directMessage(handler, MsgWhatTimeIsIt, (char*)buf2-(char*)bufStart, bufStart);
}

void handeCapBits ( void* buf, uint16_t len, GameKeeper::Player *playerData )
{
	if (!playerData)
		return;

	unsigned char bits[2] = {0};

	if (len >= 1)
		buf = nboUnpackUByte(buf,bits[0]);
	if (len >= 2)
		buf = nboUnpackUByte(buf,bits[1]);

	playerData->caps.canDownloadResources = bits[0] != 0;
	playerData->caps.canPlayRemoteSounds = bits[1] != 0;
}

void handleClientEnter(void **buf, GameKeeper::Player *playerData)
{
	if (!playerData)
		return;

	uint16_t rejectCode;
	char     rejectMsg[MessageLen];

	if (!playerData->player.unpackEnter(*buf, rejectCode, rejectMsg))
	{
		rejectPlayer(playerData->getIndex(), rejectCode, rejectMsg);
		return;
	}

	playerData->accessInfo.setName(playerData->player.getCallSign());
	std::string timeStamp = TimeKeeper::timestamp();
	std::string playerIP = "local.player";
	if ( playerData->netHandler )
		playerIP = playerData->netHandler->getTargetIP();

	logDebugMessage(1,"Player %s [%d] has joined from %s at %s with token \"%s\"\n",
		playerData->player.getCallSign(),
		playerData->getIndex(), playerIP.c_str(), timeStamp.c_str(),
		playerData->player.getToken());

	if (!clOptions->publicizeServer)
		playerData->_LSAState = GameKeeper::Player::notRequired;
	else if (strlen(playerData->player.getCallSign()))
		playerData->_LSAState = GameKeeper::Player::required;

	dontWait = true;
}

void handleClientExit ( GameKeeper::Player *playerData )
{
	// data: <none>
	removePlayer(playerData->getIndex(), "left", false);
}

void handleSetVar ( NetHandler *netHandler )
{
	if (!netHandler)
		return;

	void *bufStart = getDirectMessageBuffer();
	PackVars pv(bufStart, netHandler);
	BZDB.iterate(PackVars::packIt, &pv);
}

void handleFlagNegotiation( NetHandler *handler, void **buf, int len )
{
	if (!handler)
		return;

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
		*buf = FlagType::unpack(*buf, fDesc);
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
}

void handleWorldChunk( NetHandler *handler, void *buf )
{
	uint32_t ptr;	// data: count (bytes read so far)
	buf = nboUnpackUInt(buf, ptr);

	sendWorldChunk(handler, ptr);
}

void handleWorldSettings( NetHandler *handler )
{
	if(handler)
		bz_pwrite(handler, worldSettings, 4 + WorldSettingsSize);
}

void handleWorldHash( NetHandler *handler )
{
	if(!handler)
		return;

	void *obuf, *obufStart = getDirectMessageBuffer();
	if (clOptions->cacheURL.size() > 0)
	{
		obuf = nboPackString(obufStart, clOptions->cacheURL.c_str(), clOptions->cacheURL.size() + 1);
		directMessage(handler, MsgCacheURL, (char*)obuf-(char*)obufStart, obufStart);
	}
	obuf = nboPackString(obufStart, hexDigest, strlen(hexDigest)+1);
	directMessage(handler, MsgWantWHash, (char*)obuf-(char*)obufStart, obufStart);
}

void handlePlayerKilled ( GameKeeper::Player *playerData, void* buffer )
{
	if (!playerData || playerData->player.isObserver())
		return;

	// data: id of killer, shot id of killer
	PlayerId killer;
	FlagType* flagType;
	int16_t shot, reason;
	int phydrv = -1;

	buffer = nboUnpackUByte(buffer, killer);
	buffer = nboUnpackShort(buffer, reason);
	buffer = nboUnpackShort(buffer, shot);
	buffer = FlagType::unpack(buffer, flagType);

	if (reason == PhysicsDriverDeath)
	{
		int32_t inPhyDrv;
		buffer = nboUnpackInt(buffer, inPhyDrv);
		phydrv = int(inPhyDrv);
	}

	if (killer != ServerPlayer)	// Sanity check on shot: Here we have the killer
	{
		int si = (shot == -1 ? -1 : shot & 0x00FF);
		if ((si < -1) || (si >= clOptions->maxShots))
			return;
	}
	playerData->player.endShotCredit--;
	playerKilled(playerData->getIndex(), lookupPlayer(killer), (BlowedUpReason)reason, shot, flagType, phydrv);
}

void handleGameJoinRequest( GameKeeper::Player *playerData )
{
	// player is on the waiting list
	char buffer[MessageLen];
	float waitTime = rejoinList.waitTime(playerData->getIndex());
	if (waitTime > 0.0f)
	{
		snprintf (buffer, MessageLen, "You are unable to begin playing for %.1f seconds.", waitTime);
		sendMessage(ServerPlayer, playerData->getIndex(), buffer);

		// Make them pay dearly for trying to rejoin quickly
		playerAlive(playerData->getIndex());
		playerKilled(playerData->getIndex(), playerData->getIndex(), GotKilledMsg, -1, Flags::Null, -1);
		return;
	}

	// player moved before countdown started
	if (clOptions->timeLimit>0.0f && !countdownActive)
		playerData->player.setPlayedEarly();
	playerAlive(playerData->getIndex());
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
	memcpy(eventData.pos,state.pos,sizeof(float)*3);
	memcpy(eventData.velocity,state.velocity,sizeof(float)*3);
	eventData.angVel = state.angVel;
	eventData.azimuth = state.azimuth;
	eventData.phydrv = state.phydrv;
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

void handleShotFired(void *buf, int len, NetHandler *handler)
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

	if (playerData->netHandler != handler)
		return;

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
					{
						lastPos[i] = playerData->currentPos[i];
					}
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

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
