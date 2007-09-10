/* bzflag
 * Copyright (c) 1993 - 2007 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* interface header */
#include "GameKeeper.h"

/* system headers */
#include <vector>
#include <string>

/* common headers */
#include "GameTime.h"

GameKeeper::Player *GameKeeper::Player::playerList[PlayerSlot] = {NULL};
bool GameKeeper::Player::allNeedHostbanChecked = false;

#if defined(USE_THREADS)
pthread_mutex_t GameKeeper::Player::mutex = PTHREAD_MUTEX_INITIALIZER;

static void *tcpRx(void* arg) {
  GameKeeper::Player *playerData = (GameKeeper::Player *)arg;
  playerData->handleTcpPacketT();
  return NULL;
}
#endif


void *PackPlayerInfo(void *buf, int playerIndex, uint8_t properties )
{
  buf = nboPackUByte(buf, playerIndex);
  buf = nboPackUByte(buf, properties);
  return buf;
}

GameKeeper::Player::Player(int _playerIndex,
			   const struct sockaddr_in &clientAddr, int fd,
			   tcpCallback _clientCallback):
  player(_playerIndex), lagInfo(&player),
  playerIndex(_playerIndex), closed(false), clientCallback(_clientCallback),
  needThisHostbanChecked(false)
{
  playerList[playerIndex] = this;

  lastState.order  = 0;
  // Timestamp 0.0 -> not yet available
  stateTimeStamp   = 0.0f;
  gameTimeRate = GameTime::startRate;
  gameTimeNext = TimeKeeper::getCurrent();
  netHandler       = new NetHandler(&player, clientAddr, playerIndex, fd);
#if defined(USE_THREADS)
  int result = pthread_create(&thread, NULL, tcpRx, (void *)this);
  if (result)
    std::cerr << "Could not create thread" << std::endl;
  refCount	 = 1;
#endif
  _LSAState = start;
  bzIdentifier = "";
}

GameKeeper::Player::~Player()
{
  flagHistory.clear();
  delete netHandler;
#if defined(USE_THREADS)
  int result = pthread_join(thread, NULL);
  if (result)
    std::cerr << "Could not join thread" << std::endl;
#endif

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

  for (p = 0; p < PlayerSlot; p++) {
    if ((playerData = playerList[p]) && !playerData->closed) {
      // get time for next lagping
      playerData->lagInfo.updateLatency(waitTime);
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
	    select	     = true;
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

void GameKeeper::Player::updateNextGameTime()
{
  if (gameTimeRate < GameTime::startRate) {
    gameTimeRate = GameTime::startRate;
  } else if (gameTimeRate < GameTime::finalRate) {
    gameTimeRate = gameTimeRate * 1.25f;
  } else {
    gameTimeRate = GameTime::finalRate;
  }
  gameTimeNext = TimeKeeper::getCurrent();
  gameTimeNext += gameTimeRate;
  return;
}

void *GameKeeper::Player::packAdminInfo(void *buf)
{
  buf = nboPackUByte(buf, netHandler->sizeOfIP());
  buf = nboPackUByte(buf, playerIndex);
  buf = netHandler->packAdminInfo(buf);
  return buf;
}

void *GameKeeper::Player::packPlayerInfo(void *buf)
{
  buf = PackPlayerInfo(buf, playerIndex, accessInfo.getPlayerProperties());
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

void GameKeeper::Player::setPlayerAddMessage ( PlayerAddMessage &msg )
{
	msg.playerID = playerIndex;
	msg.team = player.getTeam();
	msg.type = player.getType();
	msg.wins = score.getWins();
	msg.losses = score.getLosses();
	msg.tks = score.getTKs();
	msg.callsign =  player.getCallSign();
	msg.email =  player.getEMail();
}


std::vector<int> GameKeeper::Player::allowed(PlayerAccessInfo::AccessPerm right,
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

bool GameKeeper::Player::loadEnterData(uint16_t &rejectCode,
				       char *rejectMsg)
{
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
  player.resetPlayer(ctf);
  player.signingOn();
  lagInfo.reset();
}


// Attempt to retrive a slot number for a player specified as EITHER "callsign" or "#<slot>"
int GameKeeper::Player::getPlayerIDByName(const std::string &name)
{
  Player* playerData;
  int slot = -1; // invalid

  if (sscanf (name.c_str(), "#%d", &slot) == 1) {
    if ( ! GameKeeper::Player::getPlayerByIndex(slot) )
      return -1;
    return slot;
  } else {
    for (int i = 0; i < PlayerSlot; i++)
      if ((playerData = playerList[i]) && !playerData->closed
	&& (TextUtils::compare_nocase(playerData->player.getCallSign(), name) == 0))
	return i;
  }
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

bool GameKeeper::Player::clean()
{
  Player* playerData;
  // Trying to detect if this action cleaned the array of player
  bool empty    = true;
  bool ICleaned = false;
  for (int i = 0; i < PlayerSlot; i++)
    if ((playerData = playerList[i]))
      if (playerData->closed
#if defined(USE_THREADS)
	  && !playerData->refCount
#endif
	  ) {
	delete playerData;
	ICleaned = true;
      } else {
	empty = false;
      }
  return empty && ICleaned;
}

int GameKeeper::Player::getFreeIndex(int min, int max)
{
  for (int i = min; i < max; i++)
    if (!playerList[i])
      return i;
  return max;
}

#if defined(USE_THREADS)
void GameKeeper::Player::handleTcpPacketT()
{
  while (!closed) {
    const RxStatus e = netHandler->tcpReceive();
    if (e == ReadPart)
      continue;
    passTCPMutex();
    clientCallback(*netHandler, playerIndex, e);
    freeTCPMutex();
  }
  refCount = 0;
}
#else
void GameKeeper::Player::handleTcpPacket(fd_set *set)
{
  if (netHandler->isFdSet(set)) {
    const RxStatus e = netHandler->tcpReceive();
    if (e == ReadPart)
      return;
    clientCallback(*netHandler, playerIndex, e);
  }
}

#endif

void GameKeeper::Player::setPlayerState(float pos[3], float azimuth)
{
  serverTimeStamp = TimeKeeper::getCurrent().getSeconds();
  memcpy(lastState.pos, pos, sizeof(float) * 3);
  lastState.azimuth = azimuth;
  // Set Speeds to 0 too
  memset(lastState.velocity, 0, sizeof(float) * 3);
  lastState.angVel = 0.0f;
  stateTimeStamp   = 0.0f;

  // player is alive.
  player.setAlive();

}

void GameKeeper::Player::setPlayerState(PlayerState state, float timestamp)
{
  lagInfo.updateLag(timestamp, state.order - lastState.order > 1);
  player.updateIdleTime();
  lastState      = state;
  stateTimeStamp = timestamp;
}

void GameKeeper::Player::getPlayerState(float pos[3], float &azimuth)
{
  memcpy(pos, lastState.pos, sizeof(float) * 3);
  azimuth = lastState.azimuth;
}

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
