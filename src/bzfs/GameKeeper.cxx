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
extern PlayerState      lastState[PlayerSlot];

GameKeeper::Player *GameKeeper::Player::playerList[PlayerSlot] = {NULL};

GameKeeper::Player::Player(int _playerIndex,
			   const struct sockaddr_in &clientAddr, int fd):
  player(&::player[_playerIndex]),
  lastState(&::lastState[_playerIndex]),
  playerIndex(_playerIndex)
{
  playerList[playerIndex] = this;

  player->initPlayer(playerIndex);
  lastState->order       = 0;
  lagInfo                = new LagInfo(player);
  player->setLastMsg("");
  player->setSpamWarns();
  netHandler             = new NetHandler(player, clientAddr, playerIndex, fd);
}

GameKeeper::Player::~Player()
{
#ifdef NETWORK_STATS
  bool wasPlaying = player->isPlaying();
#endif
  player->removePlayer();
  flagHistory.clear();
  delete lagInfo;
#ifdef NETWORK_STATS
  if (wasPlaying)
    netHandler->dumpMessageStats();
#endif
  delete netHandler;

  playerList[playerIndex] = NULL;
}

GameKeeper::Player *GameKeeper::Player::getPlayerByIndex(int _playerIndex)
{
  if (_playerIndex < 0 || _playerIndex >= PlayerSlot)
    return NULL;
  return playerList[_playerIndex];
}

void GameKeeper::Player::updateLatency(float &waitTime)
{
  Player* playerData;
  int p;

  for (p = 0; p < PlayerSlot; p++)
    if ((playerData = playerList[p])) {

      // get time for next lagping
      playerData->lagInfo->updateLatency(waitTime);

      // get time for next delayed packet (Lag Flag)
      float nextTime = playerData->delayq.nextPacketTime();
      if (nextTime < waitTime) {
	waitTime = nextTime;
      }

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

void *GameKeeper::Player::packAdminInfo(void *buf)
{
  buf = nboPackUByte(buf, netHandler->sizeOfIP());
  buf = nboPackUByte(buf, playerIndex);
  buf = nboPackUByte(buf, accessInfo.getPlayerProperties());
  buf = netHandler->packAdminInfo(buf);
  return buf;
}

std::vector<int> GameKeeper::Player::allowed(PlayerAccessInfo::AccessPerm
					     right,
					     int targetPlayer)
{
  std::vector<int> receivers;
  Player* playerData;

  if (targetPlayer != -1) {
    if ((playerData = playerList[targetPlayer]) &&
	playerData->accessInfo.hasPerm(right))
      receivers.push_back(targetPlayer);
  } else {
    for (int i = 0; i < PlayerSlot; i++)
      if ((playerData = playerList[i]) &&
	  playerData->accessInfo.hasPerm(right))
	receivers.push_back(i);
  }

  return receivers;
}

void GameKeeper::Player::signingOn(bool ctf)
{
  accessInfo.setName(player->getCallSign());
  player->resetPlayer(ctf);
  player->signingOn();
}

int GameKeeper::Player::getPlayerIDByName(const std::string &name)
{
  Player* playerData;
  for (int i = 0; i < PlayerSlot; i++)
    if ((playerData = playerList[i]) &&
	(playerData->accessInfo.getName() == name))
      return i;
  return -1;
}

void GameKeeper::Player::reloadAccessDatabase()
{
  Player* playerData;
  for (int i = 0; i < PlayerSlot; i++)
    if ((playerData = playerList[i]))
      playerData->accessInfo.reloadInfo();
}

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
