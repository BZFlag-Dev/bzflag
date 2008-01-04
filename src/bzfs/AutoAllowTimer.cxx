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

// class-interface header
#include "AutoAllowTimer.h"

// common-interface headers
#include "TimeKeeper.h"
#include "StateDatabase.h"
#include "PlayerInfo.h"
#include "bzfsMessages.h"

// bzfs specific headers
#include "bzfs.h"

AutoAllowTimerTickHandler::AutoAllowTimerTickHandler()
{
}

AutoAllowTimerTickHandler::~AutoAllowTimerTickHandler()
{
}

void AutoAllowTimerTickHandler::process(bz_EventData *eventData)
{
  if (!eventData || eventData->eventType != bz_eTickEvent)
    return;

  float autoallowtime = BZDB.eval(StateDatabase::BZDB_AUTOALLOWTIME);
  TimeKeeper now = TimeKeeper::getCurrent();

  for (int i=0; i < curMaxPlayers; i++) {
    GameKeeper::Player *player = GameKeeper::Player::getPlayerByIndex(i);
    if (!player || player->player.isObserver() || !player->player.isPlaying()) {
      continue;
    }

    if (player->player.allowChangeTime - TimeKeeper::getNullTime() == 0) {
      continue;
    }

    if (!player->player.canShoot() || !player->player.canMove()) {
      if (now - player->player.allowChangeTime > autoallowtime) {
	sendMessageAllow(player->getIndex(), AllowAll);
	player->player.setAllow(AllowAll);
      }
    }
  }
}

bool AutoAllowTimerTickHandler::autoDelete()
{
  return true;
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
