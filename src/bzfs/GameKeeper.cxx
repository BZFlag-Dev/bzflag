/* bzflag
* Copyright (c) 1993 - 2008 Tim Riker
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
#include <iostream>
#include <vector>
#include <string>

/* common headers */
#include "GameTime.h"
#include "FlagInfo.h"
#include "StateDatabase.h"

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

void PackPlayerInfo(BufferedNetworkMessage *msg, int playerIndex, uint8_t properties )
{
  msg->packUByte(playerIndex);
  msg->packUByte(properties);
}

GameKeeper::Player::Player(int _playerIndex, NetHandler *_netHandler, tcpCallback _clientCallback):
player(_playerIndex), netHandler(_netHandler), lagInfo(&player),
playerIndex(_playerIndex), closed(false), clientCallback(_clientCallback),
needThisHostbanChecked(false), idFlag(-1)
{
  playerHandler = NULL;
  playerList[playerIndex] = this;
  canSpawn = true;

  lastState.order  = 0;
  // Timestamp 0.0 -> not yet available
  stateTimeStamp   = 0.0f;
  gameTimeRate = GameTime::startRate;
  gameTimeNext = TimeKeeper::getCurrent();
#if defined(USE_THREADS)
  int result = pthread_create(&thread, NULL, tcpRx, (void *)this);
  if (result)
    std::cerr << "Could not create thread" << std::endl;
  refCount	 = 1;
#endif
  _LSAState = start;
  bzIdentifier = "";

  botHost = -1;
  botID = -1;

  currentPos[0] = currentPos[1] = currentPos[2] = 0;
  curentVel[0] = curentVel[1] = curentVel[2] = 0;
  currentRot = 0;
  currentAngVel =0;

  efectiveShotType = StandardShot;
  isParting = false;
}

GameKeeper::Player::Player(int _playerIndex, bz_ServerSidePlayerHandler *handler):
player(_playerIndex), netHandler(NULL), lagInfo(&player),
playerIndex(_playerIndex), closed(false), clientCallback(NULL),
needThisHostbanChecked(false), idFlag(-1)
{
  canSpawn = true;
  playerHandler = handler;
  playerList[playerIndex] = this;

  lastState.order  = 0;
  // Timestamp 0.0 -> not yet available
  stateTimeStamp   = 0.0f;
  gameTimeRate = GameTime::startRate;
  gameTimeNext = TimeKeeper::getCurrent();
#if defined(USE_THREADS)
  int result = pthread_create(&thread, NULL, tcpRx, (void *)this);
  if (result)
    std::cerr << "Could not create thread" << std::endl;
  refCount	 = 1;
#endif
  _LSAState = start;
  bzIdentifier = "";

  botHost = -1;
  botID = -1;

  currentPos[0] = currentPos[1] = currentPos[2] = 0;
  curentVel[0] = curentVel[1] = curentVel[2] = 0;
  currentRot = 0;
  currentAngVel =0;
}

GameKeeper::Player::~Player()
{
  flagHistory.clear();
#if defined(USE_THREADS)
  int result = pthread_join(thread, NULL);
  if (result)
    std::cerr << "Could not join thread" << std::endl;
#endif

  playerList[playerIndex] = NULL;
}

void GameKeeper::Player::setBot ( int id, PlayerId hostID )
{
  botID = id;
  if (!childBots.size())
    botHost = hostID;
}

void GameKeeper::Player::addBot ( int id, PlayerId botPlayer )
{
  if (botHost == -1 && id > 0)
   childBots.push_back(botPlayer);
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
    if ((playerData = playerList[p]) && !playerData->closed && playerData->netHandler) {
      // get time for next lagping
      playerData->lagInfo.updateLatency(waitTime);
    }
  }
}

void GameKeeper::Player::dumpScore()
{
  Player *playerData;

  std::cout << std::endl << "#players" << std::endl;
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

void GameKeeper::Player::packAdminInfo(BufferedNetworkMessage *msg)
{
  msg->packUByte(netHandler->sizeOfIP());
  msg->packUByte(playerIndex);
  char temp[128];
  char* p = (char*)netHandler->packAdminInfo(temp);
  msg->addPackedData(temp,p-temp);
}

void *GameKeeper::Player::packPlayerInfo(void *buf)
{
  buf = PackPlayerInfo(buf, playerIndex, accessInfo.getPlayerProperties());
  return buf;
}

void GameKeeper::Player::packPlayerInfo(BufferedNetworkMessage *msg)
{
  PackPlayerInfo(msg,playerIndex, accessInfo.getPlayerProperties());
}

void GameKeeper::Player::packPlayerUpdate(BufferedNetworkMessage *msg)
{
  msg->packUByte(playerIndex);
  player.packUpdate(msg);
  score.pack(msg);
  player.packId(msg);
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
    if (!TextUtils::compare_nocase(otherData->player.getCallSign(), player.getCallSign())) {
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
  if (!netHandler)
    return;
  if (player.isChat())
    netHandler->setClientKind(NetHandler::clientBZAdmin);
  else
    netHandler->setClientKind(NetHandler::clientBZFlag);
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
  if (playerHandler)		// noone "owns" use anymore
    playerHandler = NULL;
}

bool GameKeeper::Player::clean()
{
  Player* playerData;
  // Trying to detect if this action cleaned the array of player
  bool empty    = true;
  bool ICleaned = false;
  for (int i = 0; i < PlayerSlot; i++) {
    if ((playerData = playerList[i])) {
      if (playerData->closed
#if defined(USE_THREADS)
	&& !playerData->refCount
#endif
	) {
	  delete playerData;
	  playerList[i] = NULL;
	  ICleaned = true;
      } else {
	empty = false;
      }
      return empty && ICleaned;
    }
  }
  return false;
}

int GameKeeper::Player::getFreeIndex(int min, int max)
{
  clean();
  for (int i = min; i < max; i++) {
    if (!playerList[i]) {
      return i;
    }
  }
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
  memcpy(lastState.pos, pos, sizeof(float) * 3);
  lastState.azimuth = azimuth;
  // Set Speeds to 0 too
  memset(lastState.velocity, 0, sizeof(float) * 3);
  lastState.angVel = 0.0f;
  stateTimeStamp   = (float)TimeKeeper::getCurrent().getSeconds();

  doPlayerDR((float)TimeKeeper::getCurrent().getSeconds());

  // player is alive.
  player.setAlive();

  lastState.status = eAlive;
}

void GameKeeper::Player::setPlayerState(PlayerState state, float timestamp)
{
  lagInfo.updateLag(timestamp, state.order - lastState.order > 1);
  player.updateIdleTime();
  lastState      = state;
  stateTimeStamp = timestamp;

  doPlayerDR((float)TimeKeeper::getCurrent().getSeconds());
}

void GameKeeper::Player::getPlayerState(float pos[3], float &azimuth)
{
  memcpy(pos, lastState.pos, sizeof(float) * 3);
  azimuth = lastState.azimuth;
}

void GameKeeper::Player::getPlayerCurrentPosRot(float pos[3], float &rot)
{
  doPlayerDR((float)TimeKeeper::getCurrent().getSeconds());

  memcpy(pos, currentPos, sizeof(float) * 3);
  rot = currentRot;
}

void GameKeeper::Player::doPlayerDR ( float time )
{
  float delta = time - stateTimeStamp;

  currentPos[0] = lastState.pos[0] + (lastState.velocity[0] * delta);
  currentPos[1] = lastState.pos[1] + (lastState.velocity[1] * delta);
  currentPos[2] = lastState.pos[2] + (lastState.velocity[2] * delta);
  if ( currentPos[2] < 0 )
    currentPos[2] = 0;	// burrow depth maybe?

  currentRot = lastState.azimuth + (lastState.angVel * delta);

  // clamp us to +- 180, makes math easy
  while (currentRot > 180.0)
    currentRot -= 180.0;

  while (currentRot < -180.0)
    currentRot += 180.0;
}

PlayerState GameKeeper::Player::getCurrentStateAsState ( void )
{
  PlayerState	newState = lastState;
  doPlayerDR();

  memcpy(newState.pos, currentPos, sizeof(float) * 3);
  newState.azimuth = currentRot;

  return newState;
}

void*	GameKeeper::Player::packCurrentState (void* buf, uint16_t& code, bool increment)
{
  return getCurrentStateAsState().pack(buf,code,increment);
}


int GameKeeper::Player::maxShots = 0;
void GameKeeper::Player::setMaxShots(int _maxShots)
{
  maxShots = _maxShots;
}

bool GameKeeper::Player::addShot(int id, int salt, FiringInfo &firingInfo)
{
  float now = (float)TimeKeeper::getCurrent().getSeconds();
  if (id < (int)shotsInfo.size() && shotsInfo[id].present
    && now < shotsInfo[id].expireTime) {
      logDebugMessage(2,"Player %s [%d] shot id %d duplicated\n",
	player.getCallSign(), playerIndex, id);
      return false;
  }
  // verify shot number
  if (id > maxShots - 1) {
    logDebugMessage(2,"Player %s [%d] shot id %d out of range %d\n",
      player.getCallSign(), playerIndex, id, maxShots);
    return false;
  }

  shotsInfo.resize(maxShots);

  float lifeTime = BZDB.eval(StateDatabase::BZDB_RELOADTIME);
  if (firingInfo.flagType == Flags::RapidFire)
    lifeTime *= BZDB.eval(StateDatabase::BZDB_RFIREADLIFE);
  else if (firingInfo.flagType == Flags::MachineGun)
    lifeTime *= BZDB.eval(StateDatabase::BZDB_MGUNADLIFE);
  else if (firingInfo.flagType == Flags::GuidedMissile)
    lifeTime *= BZDB.eval(StateDatabase::BZDB_GMADLIFE);
  else if (firingInfo.flagType == Flags::Laser)
    lifeTime *= BZDB.eval(StateDatabase::BZDB_LASERADLIFE);
  else if (firingInfo.flagType == Flags::ShockWave)
    lifeTime *= BZDB.eval(StateDatabase::BZDB_SHOCKADLIFE);
  else if (firingInfo.flagType == Flags::Thief)
    lifeTime *= BZDB.eval(StateDatabase::BZDB_THIEFADLIFE);

  ShotInfo myShot;
  myShot.firingInfo  = firingInfo;
  myShot.salt	= salt;
  myShot.expireTime  = now + lifeTime;
  myShot.present     = true;
  myShot.running     = true;

  shotsInfo[id] = myShot;
  return true;
}

bool GameKeeper::Player::removeShot(int id, int salt, FiringInfo &firingInfo)
{
  float now = (float)TimeKeeper::getCurrent().getSeconds();
  if (id >= (int)shotsInfo.size() || !shotsInfo[id].present
    || now >= shotsInfo[id].expireTime) {
      logDebugMessage(2,"Player %s [%d] trying to stop the unexistent shot id %d\n",
	player.getCallSign(), playerIndex, id);
      return false;
  }
  if (shotsInfo[id].salt != salt) {
    logDebugMessage(2,"Player %s [%d] trying to stop a mismatched shot id %d\n",
      player.getCallSign(), playerIndex, id);
    return false;
  }
  if (!shotsInfo[id].running)
    return false;
  shotsInfo[id].running = false;
  firingInfo = shotsInfo[id].firingInfo;
  return true;
}

bool GameKeeper::Player::updateShot(int id, int salt)
{
  float now = (float)TimeKeeper::getCurrent().getSeconds();
  if (id >= (int)shotsInfo.size() || !shotsInfo[id].present
    || now >= shotsInfo[id].expireTime) {
      logDebugMessage(2,"Player %s [%d] trying to update an unexistent shot id %d\n",
	player.getCallSign(), playerIndex, id);
      return false;
  }
  if (shotsInfo[id].salt != salt) {
    logDebugMessage(2,"Player %s [%d] trying to update a mismatched shot id %d\n",
      player.getCallSign(), playerIndex, id);
    return false;
  }
  if (!shotsInfo[id].running)
    return false;
  // only GM can be updated
  if (shotsInfo[id].firingInfo.flagType != Flags::GuidedMissile) {
    logDebugMessage(2,"Player %s [%d] trying to update a non GM shot id %d\n",
      player.getCallSign(), playerIndex, id);
    return false;
  }
  return true;
}

GameKeeper::Player *GameKeeper::Player::getFirstPlayer(NetHandler *_netHandler)
{
  for (int i = 0; i < PlayerSlot; i++)
    if (playerList[i] && playerList[i]->netHandler == _netHandler)
      return playerList[i];
  return NULL;
}

void GameKeeper::Player::setLastIdFlag(int _idFlag) {
  idFlag = _idFlag;
}

int GameKeeper::Player::getLastIdFlag() {
  return idFlag;
}

float GameKeeper::Player::getRealSpeed ( float input )
{
  FlagInfo* flag = FlagInfo::get(player.getFlag());

  FlagType* flagType = NULL;
  if ( flagType )
    flagType = flag->flag.type;

  float fracOfMaxSpeed = input;

  // If we aren't allowed to move, then the real speed is 0.
  if (
    (!player.canMoveForward() && fracOfMaxSpeed > 0) ||
    (!player.canMoveBackward() && fracOfMaxSpeed < 0)
    )
    return 0.0f;

  // can't go faster forward than at top speed, and backward at half speed
  if (fracOfMaxSpeed > 1.0f)
    fracOfMaxSpeed = 1.0f;
  else if (fracOfMaxSpeed < -0.5f)
    fracOfMaxSpeed = -0.5f;

  // oscillation overthruster tank in building can't back up
  if (fracOfMaxSpeed < 0.0f && (lastState.status & PlayerState::InBuilding) && flagType == Flags::OscillationOverthruster)
    fracOfMaxSpeed = 0.0f;

  // boost speed for certain flags
  if (flagType == Flags::Velocity)
    fracOfMaxSpeed *= BZDB.eval(StateDatabase::BZDB_VELOCITYAD);
  else if (flagType == Flags::Thief)
    fracOfMaxSpeed *= BZDB.eval(StateDatabase::BZDB_THIEFVELAD);
  else if ((flagType == Flags::Burrow) && (lastState.pos[2] < 0.0f))
    fracOfMaxSpeed *= BZDB.eval(StateDatabase::BZDB_BURROWSPEEDAD);
  else if ((flagType == Flags::ForwardOnly) && (fracOfMaxSpeed < 0.0))
    fracOfMaxSpeed = 0.0f;
  else if ((flagType == Flags::ReverseOnly) && (fracOfMaxSpeed > 0.0))
    fracOfMaxSpeed = 0.0f;
  else if (flagType == Flags::Agility) {
    /*	if ((TimeKeeper::getCurrent() - agilityTime) < BZDB.eval(StateDatabase::BZDB_AGILITYTIMEWINDOW))
    fracOfMaxSpeed *= BZDB.eval(StateDatabase::BZDB_AGILITYADVEL);
    else {
    float oldFrac = desiredSpeed / BZDBCache::tankSpeed;
    if (oldFrac > 1.0f)
    oldFrac = 1.0f;
    else if (oldFrac < -0.5f)
    oldFrac = -0.5f;

    float limit = BZDB.eval(StateDatabase::BZDB_AGILITYVELDELTA);

    if (fracOfMaxSpeed < 0.0f)
    limit /= 2.0f;
    if (fabs(fracOfMaxSpeed - oldFrac) > limit) {
    fracOfMaxSpeed *= BZDB.eval(StateDatabase::BZDB_AGILITYADVEL);
    agilityTime = TimeKeeper::getCurrent();
    }
    } */
  }
  float handicap = 1.0f;

  // apply handicap advantage to tank speed
  fracOfMaxSpeed *= (1.0f + (handicap * (BZDB.eval(StateDatabase::BZDB_HANDICAPVELAD) - 1.0f)));

  // set desired speed
  return BZDB.eval(StateDatabase::BZDB_TANKSPEED) * fracOfMaxSpeed;
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
