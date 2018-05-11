/* bzflag
 * Copyright (c) 1993-2018 Tim Riker
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
#include "Roster.h"

/* local implementation headers */
#include "World.h"


NameList silencePlayers;
int curMaxPlayers = 0;
RemotePlayer** remotePlayers = NULL;
#ifdef ROBOT
RobotPlayer* robots[MAX_ROBOTS];
#endif
int numRobots = 0;


Player* lookupPlayer(PlayerId id)
{
  // check my tank first

  LocalPlayer *myTank = LocalPlayer::getMyTank();
  if (myTank && myTank->getId() == id)
    return myTank;

  if (id == ServerPlayer) {
    World* world = World::getWorld();
    if (world)
      return world->getWorldWeapons();
    else
      return NULL;
  }

  if (id < curMaxPlayers && remotePlayers[id] && remotePlayers[id]->getId() == id)
    return remotePlayers[id];

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

  if (id < curMaxPlayers && remotePlayers[id] && remotePlayers[id]->getId() == id)
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
  return remotePlayers[index];
}

Player* getPlayerByName(const char* name)
{
  for (int i = 0; i < curMaxPlayers; i++)
    if (remotePlayers[i] && strcmp( remotePlayers[i]->getCallSign(), name ) == 0)
      return remotePlayers[i];
  World *world = World::getWorld();
  if (!world)
    return NULL;
  WorldPlayer *worldWeapons = world->getWorldWeapons();
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
    if (robots[i] && robots[i]->getId() == id)
      return robots[i];
#endif
  return NULL;
}

TeamColor PlayerIdToTeam(PlayerId id)
{
  if (LastRealPlayer < id && id <= FirstTeam)
    return TeamColor(FirstTeam - id);
  else
    return NoTeam;
}

PlayerId TeamToPlayerId(TeamColor team)
{
  if (team == NoTeam)
    return NoPlayer;
  else
    return FirstTeam - team;
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
