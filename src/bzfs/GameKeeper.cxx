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

/* system headers */
#include <vector>
#include <string>

// players list FIXME should be resized based on maxPlayers
extern PlayerState      lastState[PlayerSlot];

GameKeeper::Player *GameKeeper::Player::playerList[PlayerSlot] = {NULL};

GameKeeper::Player::Player(int _playerIndex,
			   const struct sockaddr_in &clientAddr, int fd):
  player(_playerIndex), lagInfo(&player),
  lastState(&::lastState[_playerIndex]),
  playerIndex(_playerIndex), closed(false)
{
  playerList[playerIndex] = this;

  lastState->order = 0;
  netHandler       = new NetHandler(&player, clientAddr, playerIndex, fd);
}

GameKeeper::Player::~Player()
{
  flagHistory.clear();
  delete netHandler;

  playerList[playerIndex] = NULL;
}

int GameKeeper::Player::count()
{
  Player *playerData;
  int     count = 0;

  for (int i = 0; i < PlayerSlot; i++)
    if ((playerData = playerList[i]) && !playerData->closed
	&& playerData->player.isPlaying())
      count++;
  return count;
}

void GameKeeper::Player::updateLatency(float &waitTime)
{
  Player* playerData;
  int p;

  for (p = 0; p < PlayerSlot; p++)
    if ((playerData = playerList[p]) && !playerData->closed) {

      // get time for next lagping
      playerData->lagInfo.updateLatency(waitTime);

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
    if ((playerData = playerList[p]) && !playerData->closed
	&& playerData->player.isPlaying()) {
      playerData->score.dump();
      std::cout << ' ' << playerData->player.getCallSign() << std::endl;
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
    if ((playerData = playerList[i]) && !playerData->closed
	&& playerData->player.canBeRabbit(true)) {
	bool  goodRabbit = i != oldRabbit && playerData->player.isAlive();
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

void *GameKeeper::Player::packPlayerUpdate(void *buf)
{
  buf = nboPackUByte(buf, playerIndex);
  buf = player.packUpdate(buf);
  buf = score.pack(buf);
  buf = player.packId(buf);
  return buf;
}

std::vector<int> GameKeeper::Player::allowed(PlayerAccessInfo::AccessPerm
					     right,
					     int targetPlayer)
{
  std::vector<int> receivers;
  Player* playerData;

  if (targetPlayer != -1) {
    if ((playerData = playerList[targetPlayer]) && !playerData->closed
	&& playerData->accessInfo.hasPerm(right))
      receivers.push_back(targetPlayer);
  } else {
    for (int i = 0; i < PlayerSlot; i++)
      if ((playerData = playerList[i]) && !playerData->closed
	  && playerData->accessInfo.hasPerm(right))
	receivers.push_back(i);
  }

  return receivers;
}

bool GameKeeper::Player::loadEnterData(void *buf,
				       uint16_t &rejectCode,
				       char *rejectMsg)
{
  bool result = player.unpackEnter(buf, rejectCode, rejectMsg);
  if (!result)
    return false;
  // look if there is as name clash, we won't allow this
  for (int i = 0; i < PlayerSlot; i++) {
    Player *otherData = playerList[i];
    if (i == playerIndex || !otherData || !otherData->player.isPlaying())
      continue;
    if (otherData->closed)
      continue;
    if (!strcasecmp(otherData->player.getCallSign(), player.getCallSign())) {
      rejectCode   = RejectRepeatCallsign;
      strcpy(rejectMsg, "The callsign specified is already in use.");
      return false;
    }
  }
  return true;
}

void GameKeeper::Player::signingOn(bool ctf)
{
  accessInfo.setName(player.getCallSign());
  player.resetPlayer(ctf);
  player.signingOn();
}

int GameKeeper::Player::getPlayerIDByName(const std::string &name)
{
  Player* playerData;
  for (int i = 0; i < PlayerSlot; i++)
    if ((playerData = playerList[i]) && !playerData->closed
	&& (playerData->accessInfo.getName() == name))
      return i;
  return -1;
}

void GameKeeper::Player::reloadAccessDatabase()
{
  Player* playerData;
  for (int i = 0; i < PlayerSlot; i++)
    if ((playerData = playerList[i]) && !playerData->closed)
      playerData->accessInfo.reloadInfo();
}

void GameKeeper::Player::close()
{
  closed = true;
}

void GameKeeper::Player::clean()
{
  Player* playerData;
  for (int i = 0; i < PlayerSlot; i++)
    if ((playerData = playerList[i]) && playerData->closed)
      delete playerData;  
}

int GameKeeper::Player::getFreeIndex(int min, int max)
{
  for (int i = min; i < max; i++)
    if (!playerList[i])
      return i;
  return max;
}

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
