/* bzflag
 * Copyright (c) 1993-2010 Tim Riker
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
#include "bzfsMessages.h"
#include "common/BzTime.h"
#include "common/StateDatabase.h"
#include "game/PlayerInfo.h"

// bzfs specific headers
#include "bzfs.h"

AutoAllowTimerTickHandler::AutoAllowTimerTickHandler() {
}

AutoAllowTimerTickHandler::~AutoAllowTimerTickHandler() {
}

void AutoAllowTimerTickHandler::process(bz_EventData* eventData) {
  if (!eventData || eventData->eventType != bz_eTickEvent) {
    return;
  }

  float autoallowtime = BZDB.eval(BZDBNAMES.AUTOALLOWTIME);
  BzTime now = BzTime::getCurrent();

  for (int i = 0; i < curMaxPlayers; i++) {
    GameKeeper::Player* player = GameKeeper::Player::getPlayerByIndex(i);
    if (!player || player->player.isObserver() || !player->player.isPlaying()) {
      continue;
    }

    if (player->player.allowChangeTime - BzTime::getNullTime() == 0) {
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

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=2 tabstop=8 expandtab
