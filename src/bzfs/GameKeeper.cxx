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
extern PlayerAccessInfo accessInfo[PlayerSlot];
extern PlayerState      lastState[PlayerSlot];
extern DelayQueue       delayq[PlayerSlot];

GameKeeper::Player *GameKeeper::Player::playerList[PlayerSlot] = {NULL};

GameKeeper::Player::Player(int _playerIndex):
  player(&::player[_playerIndex]), accessInfo(&::accessInfo[_playerIndex]),
  lastState(&::lastState[_playerIndex]), delayq(&::delayq[_playerIndex]),
  playerIndex(_playerIndex)
{
  playerList[playerIndex] = this;

  player->initPlayer(playerIndex);
  lastState->order       = 0;
  lagInfo                = new LagInfo(player);
}

GameKeeper::Player::~Player()
{
  accessInfo->removePlayer();
  player->removePlayer();
  delayq->dequeuePackets();
  flagHistory.clear();
  delete lagInfo;
  playerList[playerIndex] = NULL;
}

GameKeeper::Player *GameKeeper::Player::getPlayerByIndex(int _playerIndex)
{
  return playerList[_playerIndex];
}

void GameKeeper::Player::updateLatency(float &waitTime)
{
  int p;
  for (p = 0; p < PlayerSlot; p++)
    if (playerList[p]) {
      playerList[p]->lagInfo->updateLatency(waitTime);
  }
}

void GameKeeper::Player::dumpScore()
{
  Player *playerData;

  std::cout << "\n#players\n";
  int p;
  for (p = 0; p < PlayerSlot; p++) 
    if ((playerData = playerList[p]) && playerData->player->isPlaying()) {
      playerData->score.dump();
      std::cout << ' ' << playerData->player->getCallSign() << std::endl;
    }
}

int GameKeeper::Player::anointRabbit(int oldRabbit)
{
  float topRatio    = -100000.0f;
  int   rabbitIndex = NoPlayer;

  Player *playerData;
  int     i;
  bool    goodRabbitSelected = false;

  for (i = 0; i < PlayerSlot; i++)
    if ((playerData = playerList[i]))
      if (playerData->player->canBeRabbit(true)) {
	bool  goodRabbit = i != oldRabbit && playerData->player->isAlive();
	float ratio      = playerData->score.ranking();
	bool  select     = false;
	if (goodRabbitSelected) {
	  if (goodRabbit && (ratio > topRatio)) {
	    select = true;
	  }
	} else {
	  if (goodRabbit) {
	    select             = true;
	    goodRabbitSelected = true;
	  } else {
	    if (ratio > topRatio)
	      select = true;
	  }
	}
	if (select) {
	  topRatio = ratio;
	  rabbitIndex = i;
	}
      }
  return rabbitIndex;
}

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
