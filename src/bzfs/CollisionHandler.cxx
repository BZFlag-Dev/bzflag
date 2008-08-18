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

#include "global.h"
#include "bzfs.h"

/* implementation header */
#include "CollisionHandler.h"

CollisionHandler::CollisionHandler() {
  lastUpdate = TimeKeeper::getCurrent().getSeconds();
}

void CollisionHandler::process(bz_EventData*)
{
  float currentTime = TimeKeeper::getCurrent().getSeconds();
  float dt = currentTime - lastUpdate;

  // Loop through the players, updating shot positions and
  // checking for collisions
  for (int i = 0; i < curMaxPlayers; i++) {

    GameKeeper::Player* player = GameKeeper::Player::getPlayerByIndex(i);

    if (!player)
      continue;

    for (unsigned int p = 0; p < player->shotStrategies.size(); ++p) {
      // predict the current shot position for this shot
      float shotPos[3];
      SegmentedShotStrategy* strategy = player->shotStrategies.at(p);
      strategy->update(dt);
      strategy->predictPosition(0, shotPos);
    }
  }

  lastUpdate = currentTime;
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
