/* bzflag
 * Copyright (c) 1993 - 2004 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* interface header */
#include "GameKeeper.h"

extern PlayerInfo       player[PlayerSlot];
// player lag info
LagInfo                *lagInfo[PlayerSlot] = {NULL};
extern PlayerAccessInfo accessInfo[PlayerSlot];
extern PlayerState      lastState[PlayerSlot];
extern DelayQueue       delayq[PlayerSlot];
extern FlagHistory      flagHistory[PlayerSlot];
extern Score           *score[PlayerSlot];

GameKeeper::Player *GameKeeper::Player::playerList[PlayerSlot] = {NULL};

GameKeeper::Player::Player(int _playerIndex):
  player(&::player[_playerIndex]), accessInfo(&::accessInfo[_playerIndex]),
  lastState(&::lastState[_playerIndex]), delayq(&::delayq[_playerIndex]),
  flagHistory(&::flagHistory[_playerIndex]), score(NULL),
  playerIndex(_playerIndex)
{
  GameKeeper::Player::playerList[playerIndex] = this;

  player->initPlayer(playerIndex);
  lastState->order       = 0;
  lagInfo                = new LagInfo(player);
  ::lagInfo[playerIndex] = lagInfo;
  score                  = new Score();
  ::score[playerIndex]   = score;
}

GameKeeper::Player::~Player()
{
  accessInfo->removePlayer();
  player->removePlayer();
  delayq->dequeuePackets();
  flagHistory->clear();
  delete lagInfo;
  delete score;
  ::lagInfo[playerIndex] = NULL;
  ::score[playerIndex]   = NULL;
  GameKeeper::Player::playerList[playerIndex] = NULL;
}

GameKeeper::Player *GameKeeper::Player::getPlayerByIndex(int _playerIndex)
{
  return GameKeeper::Player::playerList[_playerIndex];
}

void GameKeeper::Player::updateLatency(float &waitTime)
{
  int p;
  for (p = 0; p < PlayerSlot; p++)
    if (playerList[p]) {
      playerList[p]->lagInfo->updateLatency(waitTime);
  }
}

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
