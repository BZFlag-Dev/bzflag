/* bzflag
 * Copyright (c) 1993 - 2004 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* interface header */
#include "common.h"
#include "Roster.h"

/* local implementation headers */
#include "World.h"
#include "WorldPlayer.h"


NameList silencePlayers;
int curMaxPlayers = 0;
RemotePlayer** player = NULL;
RobotPlayer* robots[MAX_ROBOTS];
int numRobots = 0;


Player* lookupPlayer(PlayerId id)
{
  // check my tank first

  LocalPlayer *myTank = LocalPlayer::getMyTank();
  if (myTank->getId() == id)
    return myTank;

  if (id == ServerPlayer)
    return World::getWorld()->getWorldWeapons();

  if (id < curMaxPlayers && player[id] && player[id]->getId() == id)
    return player[id];

  // it's nobody we know about
  return NULL;
}

int lookupPlayerIndex(PlayerId id)
{
  // check my tank first
  
  if (LocalPlayer::getMyTank()->getId() == id)
    return -2;

  if (id == ServerPlayer)
    return ServerPlayer;

  if (id < curMaxPlayers && player[id] && player[id]->getId() == id)
    return id;

  // it's nobody we know about
  return -1;
}

Player* getPlayerByIndex(int index)
{
  if (index == -2)
    return LocalPlayer::getMyTank();
  if (index == ServerPlayer)
    return World::getWorld()->getWorldWeapons();
  if (index == -1 || index >= curMaxPlayers)
    return NULL;
  return player[index];
}

Player* getPlayerByName(const char* name)
{
  for (int i = 0; i < curMaxPlayers; i++)
    if (player[i] && strcmp( player[i]->getCallSign(), name ) == 0)
      return player[i];
  WorldPlayer *worldWeapons = World::getWorld()->getWorldWeapons();
  if (strcmp(worldWeapons->getCallSign(), name) == 0)
    return worldWeapons;
  return NULL;
}

BaseLocalPlayer* getLocalPlayer(PlayerId id)
{
  LocalPlayer *myTank = LocalPlayer::getMyTank();
  if (myTank->getId() == id) return myTank;
#ifdef ROBOT
  for (int i = 0; i < numRobots; i++)
    if (robots[i]->getId() == id)
      return robots[i];
#endif
  return NULL;
}

TeamColor PlayerIdToTeam(PlayerId id)
{
  if (id >= 244 && id<=250)
    return TeamColor(250 - id);
  else
    return NoTeam;
}

PlayerId TeamToPlayerId(TeamColor team)
{
  if (team == NoTeam)
    return NoPlayer;
  else
    return 250-team;
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
