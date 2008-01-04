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

      if (BZDB.isTrue("_speedChecksLogOnly"))
	logOnly = true;

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
      if ((playerData->lastState.pos[2] != state.pos[2]) || (playerData->lastState.velocity[2] != state.velocity[2]) || ((state.status & PlayerState::Alive) == 0))
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
	    logDebugMessage(1,"Logging Player %s [%d] tank too fast (tank: %f, allowed: %f){Dead or v[z] != 0}\n", playerData->player.getCallSign(), playerData->getIndex(), sqrt(curPlanarSpeedSqr), sqrt(maxPlanarSpeedSqr));
	  else
	  {
	    logDebugMessage(1,"Kicking Player %s [%d] tank too fast (tank: %f, allowed: %f)\n", playerData->player.getCallSign(), playerData->getIndex(), sqrt(curPlanarSpeedSqr), sqrt(maxPlanarSpeedSqr));
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
		logDebugMessage(1,"Kicking Player %s [%d] Out of map bounds at position (%.2f,%.2f,%.2f)\n", playerData->player.getCallSign(), playerData->getIndex(), state.pos[0], state.pos[1], state.pos[2]);
		sendMessage(ServerPlayer, playerData->getIndex(), "Autokick: Player location was outside the playing area.");
		removePlayer(playerData->getIndex(), "Out of map bounds", true);
		return false;
	}
	return true;
}

bool doPauseChecks ( GameKeeper::Player *playerData, PlayerState &state )
{
	// make sure the player only pauses after the waiting time for pause is over
	// we need some inaccuracy here that is computed using pauseRequestTime and pauseRequestLag
	// lag will cause all players to pause sooner than 5 seconds
	if (state.status & PlayerState::Paused) {
		TimeKeeper pauseDelay = playerData->player.pauseRequestTime;
		pauseDelay += (- playerData->player.pauseRequestLag / 1000.0);

		if ((TimeKeeper::getCurrent() - pauseDelay) < 5.0f
			&& (playerData->player.pauseRequestTime - TimeKeeper::getNullTime() != 0)) {
			// we have one of those players all love
			logDebugMessage(1,"Kicking Player %s [%d] Paused too fast!\n", playerData->player.getCallSign(),
			playerData->getIndex());
			sendMessage(ServerPlayer, playerData->getIndex(), "Autokick: Player paused too fast.");
			removePlayer(playerData->getIndex(), "Paused too fast");
			return false;
		} else {
			pausePlayer(playerData->player.getPlayerIndex(), true);
		}

		// kick the players when they do not pause within allowed situations
		if ((state.status & PlayerState::InBuilding) || (state.status & PlayerState::PhantomZoned)
			|| (state.status & PlayerState::Falling) || (state.status & PlayerState::Alive) == false) {
			// the player did pause while being a wall or in air
			logDebugMessage(1,"Kicking Player %s [%d] Paused in unallowed state!\n", playerData->player.getCallSign(),
			playerData->getIndex());
			sendMessage(ServerPlayer, playerData->getIndex(), "Autokick: Player paused in unallowed state.");
			removePlayer(playerData->getIndex(), "Paused in unallowed state");
			return false;
		}
	}

	return true;
}

bool doHeightChecks ( GameKeeper::Player *playerData, PlayerState &state )
{
	float wingsGravity = BZDB.eval(StateDatabase::BZDB_WINGSGRAVITY);
	float normalGravity = BZDBCache::gravity;
	float lgGravity = BZDB.eval(StateDatabase::BZDB_LGGRAVITY);

	// All tanks with wings are flying away or they do without a flag
	if (((wingsGravity >= 0.0f) && (normalGravity >= 0.0f)) || (normalGravity >= 0.0f)) {
		return true;
	}

	float normalMaxHeight = 0.0f;
	float wingsMaxHeight = 0.0f;
	float lgMaxHeight = 0.0f;

	if (!(state.status & PlayerState::Falling) || (playerData->player.allowedHeightAtJumpStart < 0)) {
		playerData->player.jumpStartPos = state.pos[2];
	}

	float heightFudge = BZDB.eval(StateDatabase::BZDB_HEIGHTCHECKTOL);
	if (heightFudge < 1.0f) {
		// Skip the check because the server owners disabled it
		return true;
	}

	int pFlag = playerData->player.getFlag();
	bool hasWings = false;
	bool hasLG = false;

	if (pFlag >= 0)
	{
		FlagInfo &flag = *FlagInfo::get(pFlag);
		if (flag.flag.type == Flags::Wings) {
			hasWings = true;
		} else if (flag.flag.type == Flags::LowGravity) {
		    hasLG = true;
		}
	}

	if (hasWings) {
		wingsMaxHeight = BZDB.eval(StateDatabase::BZDB_WINGSJUMPVELOCITY);
		wingsMaxHeight *= wingsMaxHeight;
		wingsMaxHeight *= BZDB.eval(StateDatabase::BZDB_WINGSJUMPCOUNT);
		wingsMaxHeight /= (-wingsGravity * 2.0f);
	} else if (hasLG) {
		lgMaxHeight = BZDB.eval(StateDatabase::BZDB_JUMPVELOCITY) * lgGravity / normalGravity;
		lgMaxHeight *= lgMaxHeight;
		lgMaxHeight /= (-normalGravity * 2.0f);;
	} else {
		normalMaxHeight = BZDB.eval(StateDatabase::BZDB_JUMPVELOCITY);
		normalMaxHeight *= normalMaxHeight;
		normalMaxHeight /= (-normalGravity * 2.0f);
	}

	// _wingsGravity is set to a positive value
	if (wingsMaxHeight < 0) {
		wingsMaxHeight = MAXFLOAT;
	}

	// use either the wings height, the usual height or the low gravity height
	float maxHeight;
	if (wingsMaxHeight > normalMaxHeight || lgMaxHeight > normalMaxHeight) {
	    if (lgMaxHeight > wingsMaxHeight) {
	    maxHeight = lgMaxHeight;
	    } else {
		maxHeight = wingsMaxHeight;
	    }
	} else {
		maxHeight = normalMaxHeight;
	}

	if (playerData->player.allowedHeightAtJumpStart > 0.0f && !(state.status & PlayerState::Falling)) {
		playerData->player.allowedHeightAtJumpStart = 0.0f;
	}

	if ((playerData->player.allowedHeightAtJumpStart <= 0.0f) && !(state.status & PlayerState::Falling)) {
		playerData->player.allowedHeightAtJumpStart = maxHeight;
	}

	// Don't kick players if the world param is still changing
	if (worldStateChanging()) {
		playerData->player.allowedHeightAtJumpStart = MAXFLOAT;
	}

	// Don't kick players that are spawning in the air
	if ((state.status & PlayerState::Falling) && (!(playerData->lastState.status & PlayerState::Alive))) {
		playerData->player.allowedHeightAtJumpStart = MAXFLOAT;
	}

	// currently we don't know how high the teleporter is so skip the check
	if ((state.status & PlayerState::Falling) && (state.status & PlayerState::Teleporting)) {
		playerData->player.allowedHeightAtJumpStart = MAXFLOAT;
	}

	// if one of the values changed while the player was in air
	// use the higher allowed one in case we did not get a
	// new update from him yet
	if (playerData->player.allowedHeightAtJumpStart > maxHeight) {
		maxHeight = playerData->player.allowedHeightAtJumpStart;
	}

	// if player was on physics driver skip check until he lands again
	// FIXME: Compute how high the player should jump
	if ((state.status & PlayerState::Falling) && (playerData->lastState.status & PlayerState::OnDriver)
		&& !cheatProtectionOptions.doHeightChecks) {
		playerData->player.allowedHeightAtJumpStart = MAXFLOAT;
	}

	if ((normalGravity < -25.0f) && !(playerData->player.allowedHeightAtJumpStart == MAXFLOAT)) {
		maxHeight += 1.0f;
	}

	// final adjustments
	if (playerData->player.allowedHeightAtJumpStart == MAXFLOAT) {
		maxHeight = MAXFLOAT;
	} else {
		maxHeight *= heightFudge;
		maxHeight += playerData->player.jumpStartPos;
	}

	if (state.pos[2] > maxHeight) {
		logDebugMessage(1,"Kicking Player %s [%d] jumped too high [max: %f height: %f]\n",
		playerData->player.getCallSign(), playerData->getIndex(), maxHeight, state.pos[2]);
		sendMessage(ServerPlayer, playerData->getIndex(), "Autokick: Player location was too high.");
		removePlayer(playerData->getIndex(), "too high", true);
		return false;
	}
	return true;
}

bool doOOChecks ( GameKeeper::Player *playerData, PlayerState &state )
{
	// skip if tank does not have OO flag
	int flagId = playerData->player.getFlag();
	if (flagId >= 0) {
		FlagInfo &playerFlag = *FlagInfo::get(flagId);
		if (playerFlag.flag.type != Flags::OscillationOverthruster)
			return true;
	}
	if (flagId < 0)
		return true;

	// tank did not move therefore skip check
	if  (!(((state.velocity[0]) != (playerData->lastState.velocity[0]))
	  || ((state.velocity[1]) != (playerData->lastState.velocity[1]))
	  || ((state.velocity[2]) != (playerData->lastState.velocity[2]))))
	{
	    return true;
	}

	float forward[3];
	float rotation = state.azimuth;
	forward[0] = cosf(rotation);
	forward[1] = sinf(rotation);
	forward[2] = 0.0f;
	bool droveForward = false;
	if (state.velocity[0] != 0)
		droveForward = ((forward[0] / (state.velocity[0])) > ZERO_TOLERANCE);
	if (state.velocity[1] != 0 && !droveForward)
		droveForward = ((forward[1] / (state.velocity[1])) > ZERO_TOLERANCE);


	if (!droveForward && (((state.velocity[0]) == 0) || ((state.velocity[1]) == 0)))
		droveForward = true;

	// InBuilding state doesn't get set when tank is inside a drivethrough obstacle
	if (!(state.status & PlayerState::Falling) && !droveForward && !(playerData->lastState.status & PlayerState::Falling)
	    && (state.status & PlayerState::InBuilding))
	{
		logDebugMessage(1, "%s drove backward in building\n", playerData->player.getCallSign());
		char message[MessageLen];
		snprintf(message, MessageLen, "Cheat checks: %s drove backwards in building", playerData->player.getCallSign());
		sendMessage(ServerPlayer, AdminPlayers, message);
	}
	return true;
}

bool validatePlayerState(GameKeeper::Player *playerData, PlayerState &state)
{
	// Don't kick players if the world param is still changing
	if (!worldStateChanging())
	{
	  // exploding or dead players can do unpredictable things
	  if (state.status != PlayerState::Exploding && state.status != PlayerState::DeadStatus)
	  {
	    if (!doBoundsChecks(playerData,state))
	      return false;

	    if (!doSpeedChecks(playerData,state))
	      return false;
	  }

	  if (!doPauseChecks(playerData,state))
	    return false;
	}

	// see if the player is too high
	if (!doHeightChecks(playerData,state))
		return false;

	// see if player drives backwards in buildings
	if (!doOOChecks(playerData,state))
		return false;
	return true;
}

bool checkFlagCheats ( GameKeeper::Player *playerData, int teamIndex )
{
	bool foundACheat = false;
	TeamColor base = whoseBase(playerData->currentPos[0], playerData->currentPos[1], playerData->currentPos[2]);
	if ((teamIndex == playerData->player.getTeam() && base == playerData->player.getTeam()))
	{
		logDebugMessage(1,"Player %s [%d] might have sent MsgCaptureFlag for taking their own "
		"flag onto their own base\n",
		playerData->player.getCallSign(), playerData->getIndex());
		foundACheat = true;
	}

	if ((teamIndex != playerData->player.getTeam() && base != playerData->player.getTeam()))
	{
		logDebugMessage(1,"Player %s [%d] (%s) might have tried to capture %s flag without "
				"reaching their own base. (Player position: %f %f %f)\n",
				playerData->player.getCallSign(), playerData->getIndex(),
				Team::getName(playerData->player.getTeam()),
				Team::getName((TeamColor)teamIndex),
				playerData->currentPos[0], playerData->currentPos[1],
				playerData->currentPos[2]);
		foundACheat = true;
	}

	return foundACheat;
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
