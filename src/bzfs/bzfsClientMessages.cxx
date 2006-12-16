/* bzflag
* Copyright (c) 1993 - 2006 Tim Riker
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
	eventData.time = TimeKeeper::getCurrent().getSeconds();
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
