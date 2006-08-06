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

/* interface header */
#include "bzfsPlayerStateVerify.h"

/* system implementation headers */
#include <iostream>
#include <assert.h>

/* common implementation headers */
#include "StateDatabase.h"
#include "BZDBCache.h"


bool doSpeedChecks ( GameKeeper::Player *playerData, PlayerState &state )
{
	// Speed problems occur around flag drops, so don't check for
	// a short period of time after player drops a flag. Currently
	// 2 second, adjust as needed.
	if (playerData->player.isFlagTransitSafe())
	{
		// we'll be checking against the player's flag type
		int pFlag = playerData->player.getFlag();

		// check for highspeed cheat; if inertia is enabled, skip test for now
		if (BZDB.eval(StateDatabase::BZDB_INERTIALINEAR) == 0.0f)
		{
			// Doesn't account for going fast backwards, or jumping/falling
			float curPlanarSpeedSqr = state.velocity[0]*state.velocity[0] + state.velocity[1]*state.velocity[1];

			float maxPlanarSpeed = BZDBCache::tankSpeed;

			bool logOnly = false;

			// if tank is not driving cannot be sure it didn't toss
			// (V) in flight

			// if tank is not alive cannot be sure it didn't just toss
			// (V)
			if (pFlag >= 0)
			{
				FlagInfo &flag = *FlagInfo::get(pFlag);
				if (flag.flag.type == Flags::Velocity)
					maxPlanarSpeed *= BZDB.eval(StateDatabase::BZDB_VELOCITYAD);
				else if (flag.flag.type == Flags::Thief)
					maxPlanarSpeed *= BZDB.eval(StateDatabase::BZDB_THIEFVELAD);
				else if (flag.flag.type == Flags::Agility)
					maxPlanarSpeed *= BZDB.eval(StateDatabase::BZDB_AGILITYADVEL);
				else if ((flag.flag.type == Flags::Burrow) && (playerData->lastState.pos[2] == state.pos[2]) && (playerData->lastState.velocity[2] == state.velocity[2]) && (state.pos[2] <= BZDB.eval(StateDatabase::BZDB_BURROWDEPTH)))
				{	// if we have burrow and are not actively burrowing
					// You may have burrow and still be above ground. Must
					// check z in ground!!
					maxPlanarSpeed *= BZDB.eval(StateDatabase::BZDB_BURROWSPEEDAD);
				}
			}

			float maxPlanarSpeedSqr = maxPlanarSpeed * maxPlanarSpeed;

			// If player is moving vertically, or not alive the speed checks
			// seem to be problematic. If this happens, just log it for now,
			// but don't actually kick
			if ((playerData->lastState.pos[2] != state.pos[2]) ||  (playerData->lastState.velocity[2] != state.velocity[2]) || ((state.status & PlayerState::Alive) == 0)) 
				logOnly = true;

			// allow a 10% tolerance level for speed if -speedtol is not sane
			if (cheatProtectionOptions.doSpeedChecks)
			{
				float realtol = 1.1f;
				if (speedTolerance > 1.0f)
					realtol = speedTolerance;
				maxPlanarSpeedSqr *= realtol;

				if (curPlanarSpeedSqr > maxPlanarSpeedSqr)
				{
					if (logOnly)
						DEBUG1("Logging Player %s [%d] tank too fast (tank: %f, allowed: %f){Dead or v[z] != 0}\n", playerData->player.getCallSign(), playerData->getIndex(), sqrt(curPlanarSpeedSqr), sqrt(maxPlanarSpeedSqr));
					else 
					{
						DEBUG1("Kicking Player %s [%d] tank too fast (tank: %f, allowed: %f)\n", playerData->player.getCallSign(), playerData->getIndex(), sqrt(curPlanarSpeedSqr), sqrt(maxPlanarSpeedSqr));
						sendMessage(ServerPlayer, playerData->getIndex(), "Autokick: Player tank is moving too fast.");
						removePlayer(playerData->getIndex(), "too fast");
					}
					return false;
				}
			}
		}
	}
	return true;
}

bool doBoundsChecks ( GameKeeper::Player *playerData, PlayerState &state )
{
	// make sure the player is still in the map
	// test all the map bounds + some fudge factor, just in case
	static const float positionFudge = 10.0f; /* linear distance */
	bool InBounds = true;
	float worldSize = BZDBCache::worldSize;

	if ( (state.pos[1] >= worldSize*0.5f + positionFudge) || (state.pos[1] <= -worldSize*0.5f - positionFudge)) 
	{
		std::cout << "y position (" << state.pos[1] << ") is out of bounds (" << worldSize * 0.5f << " + " << positionFudge << ")" << std::endl;
		InBounds = false;
	} else if ( (state.pos[0] >= worldSize*0.5f + positionFudge) || (state.pos[0] <= -worldSize*0.5f - positionFudge))
	{
		std::cout << "x position (" << state.pos[0] << ") is out of bounds (" << worldSize * 0.5f << " + " << positionFudge << ")" << std::endl;
		InBounds = false;
	}

	static const float burrowFudge = 1.0f; /* linear distance */
	if (state.pos[2]<BZDB.eval(StateDatabase::BZDB_BURROWDEPTH) - burrowFudge) 
	{
		std::cout << "z depth (" << state.pos[2] << ") is less than burrow depth (" << BZDB.eval(StateDatabase::BZDB_BURROWDEPTH) << " - " << burrowFudge << ")" << std::endl;
		InBounds = false;
	}

	// kick em cus they are most likely cheating or using a buggy client
	if (!InBounds)
	{
		DEBUG1("Kicking Player %s [%d] Out of map bounds at position (%.2f,%.2f,%.2f)\n", playerData->player.getCallSign(), playerData->getIndex(), state.pos[0], state.pos[1], state.pos[2]);
		sendMessage(ServerPlayer, playerData->getIndex(), "Autokick: Player location was outside the playing area.");
		removePlayer(playerData->getIndex(), "Out of map bounds", true);
		return false;
	}
	return true;
}

bool doPauseChecks ( GameKeeper::Player *playerData, PlayerState &state )
{
	// make sure the player only pauses after the waiting time for pause is over
	// we don not want cheaters that have instant pause

	// if smooth the pause using the players lag, so server side pause and players own view are nearly identical
	if   ((TimeKeeper::getCurrent() - playerData->player.pauseRequestTime) >= 5.0f
		&& (playerData->player.pauseRequestTime - TimeKeeper::getNullTime() != 0)
		&& playerData->player.isPaused()==false)
	{
		// if they are in an unallowed state, stop pausing them, otherwise pause them
		if (state.status & PlayerState::CrossingWall || state.status & PlayerState::Falling
			|| (state.status & PlayerState::Alive) == false)
        {
			playerData->player.pauseRequestTime = TimeKeeper::getNullTime();
		} else {
			uint8_t pause;
			void *buf = getDirectMessageBuffer();
			nboUnpackUByte(buf, pause);
			pausePlayer(playerData->player.getPlayerIndex(), true);
		}
	}

	// we need some inaccuracy here that was previously computed into pauseRequestTime
    // lag will cause all players to pause sooner than 5 seconds
    if ((TimeKeeper::getCurrent() - playerData->player.pauseRequestTime) < 5.0f
		&& (playerData->player.pauseRequestTime - TimeKeeper::getNullTime() != 0)
        && state.status & PlayerState::Paused)
    {
		// we have one of those players all love
		DEBUG1("Kicking Player %s [%d] Paused too fast!\n", playerData->player.getCallSign(), playerData->getIndex());
		sendMessage(ServerPlayer, playerData->getIndex(), "Autokick: Player paused too fast.");
		removePlayer(playerData->getIndex(), "Paused too fast");
		return false;
	}

	return true;
}

bool doHeightChecks ( GameKeeper::Player *playerData, PlayerState &state )
{
	if (cheatProtectionOptions.doHeightChecks)
		return true;

	static const float heightFudge = 1.10f; /* 10% */

	float wingsGravity = BZDB.eval(StateDatabase::BZDB_WINGSGRAVITY);
	float normalGravity = BZDBCache::gravity;

	if ((wingsGravity < 0.0f) && (normalGravity < 0.0f))
	{
		float wingsMaxHeight = BZDB.eval(StateDatabase::BZDB_WINGSJUMPVELOCITY);
		wingsMaxHeight *= wingsMaxHeight;
		wingsMaxHeight *= (1 + BZDB.eval(StateDatabase::BZDB_WINGSJUMPCOUNT));
		wingsMaxHeight /= (-wingsGravity * 0.5f);

		float normalMaxHeight = BZDB.eval(StateDatabase::BZDB_JUMPVELOCITY);
		normalMaxHeight *= normalMaxHeight;
		normalMaxHeight /= (-normalGravity * 0.5f);

		float maxHeight;
		if (wingsMaxHeight > normalMaxHeight) 
			maxHeight = wingsMaxHeight;
		else 
			maxHeight = normalMaxHeight;

		// final adjustments
		maxHeight *= heightFudge;
		maxHeight += maxWorldHeight;

		if (state.pos[2] > maxHeight) 
		{
			DEBUG1("Kicking Player %s [%d] jumped too high [max: %f height: %f]\n", playerData->player.getCallSign(), playerData->getIndex(), maxHeight, state.pos[2]);
			sendMessage(ServerPlayer, playerData->getIndex(), "Autokick: Player location was too high.");
			removePlayer(playerData->getIndex(), "too high", true);
			return false;
		}
	}
	return true;
}

bool validatePlayerState(GameKeeper::Player *playerData, PlayerState &state)
{
	// Don't kick players if the world param is still changing,
	if (!worldStateChanging())
	{
		// see if the player is too high
		if (!doHeightChecks(playerData,state))
			return false;

		if (!doBoundsChecks(playerData,state))
			return false;

		if (!doSpeedChecks(playerData,state))
			return false;

		if (!doPauseChecks(playerData,state))
			return false;
	}
	return true;
}

bool checkFlagCheats ( GameKeeper::Player *playerData, int teamIndex )
{
	bool foundACheat = false;
	TeamColor base = whoseBase(playerData->lastState.pos[0], playerData->lastState.pos[1], playerData->lastState.pos[2]);
	if ((teamIndex == playerData->player.getTeam() && base == playerData->player.getTeam()))
	{
		DEBUG1("Player %s [%d] might have sent MsgCaptureFlag for taking their own "
		"flag onto their own base\n",
		playerData->player.getCallSign(), playerData->getIndex());
		foundACheat = true;
	}

	if ((teamIndex != playerData->player.getTeam() && base != playerData->player.getTeam()))
	{
		DEBUG1("Player %s [%d] (%s) might have tried to capture %s flag without "
				"reaching their own base. (Player position: %f %f %f)\n",
				playerData->player.getCallSign(), playerData->getIndex(),
				Team::getName(playerData->player.getTeam()),
				Team::getName((TeamColor)teamIndex),
				playerData->lastState.pos[0], playerData->lastState.pos[1],
				playerData->lastState.pos[2]);
		foundACheat = true;
	}

	return foundACheat;
}

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
