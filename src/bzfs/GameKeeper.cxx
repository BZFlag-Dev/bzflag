/* bzflag
 * Copyright (c) 1993-2023 Tim Riker
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
#include "GameKeeper.h"

/* system headers */
#include <vector>
#include <string>

/* common headers */
#include "GameTime.h"

#include "bzfs.h"
#include "WallObstacle.h"
#include "Teleporter.h"
#include "PhysicsDriver.h"
#include "Obstacle.h"
#include "MeshObstacle.h"
#include "CollisionManager.h"

GameKeeper::Player* GameKeeper::Player::playerList[PlayerSlot] = { 0 }; // this is suspect...
bool GameKeeper::Player::allNeedHostbanChecked = false;

void PackPlayerInfo(MessageBuffer::Ptr msg, int playerIndex, uint8_t properties)
{
    msg->packUByte(playerIndex);
    msg->packUByte(properties);
}

GameKeeper::Player::Player(int _playerIndex,
                           const struct sockaddr_in &clientAddr, int fd,
                           tcpCallback _clientCallback)
    : _LSAState(start),
      _JoinState(waitingForAuthentication),
      player(_playerIndex), netHandler(new NetHandler(&player, clientAddr, _playerIndex, fd)),
      lagInfo(&player),
      stateTimeStamp(0.0f), serverTimeStamp(0.0),
      gameTimeRate(GameTime::startRate), gameTimeNext(TimeKeeper::getCurrent()),
      isParting(false), hasEntered(false),
      playerHandler(0),
      addWasDelayed(false), hadEnter(false), addDelayStartTime(0.0),
      playerIndex(_playerIndex), closed(false), clientCallback(_clientCallback),
      needThisHostbanChecked(false), idFlag(-1)
{
    playerList[playerIndex] = this;

    lastState.order = 0;
    score.playerID = _playerIndex;
    lastHeldFlagID = -1;

    lastShotUpdateTime = TimeKeeper::getCurrent().getSeconds();
    setupPhysicsData();
}


GameKeeper::Player::Player(int _playerIndex,
                           NetHandler* handler,
                           tcpCallback _clientCallback)
    : _LSAState(start),
      _JoinState(waitingForAuthentication),
      player(_playerIndex), netHandler(handler),
      lagInfo(&player),
      stateTimeStamp(0.0f), serverTimeStamp(0.0),
      gameTimeRate(GameTime::startRate), gameTimeNext(TimeKeeper::getCurrent()),
      isParting(false), hasEntered(false),
      playerHandler(0),
      addWasDelayed(false), hadEnter(false), addDelayStartTime(0.0),
      playerIndex(_playerIndex), closed(false), clientCallback(_clientCallback),
      needThisHostbanChecked(false), idFlag(-1)
{
    playerList[playerIndex] = this;

    lastState.order = 0;
    score.playerID = _playerIndex;

    netHandler->setPlayer(&player, _playerIndex);
    lastHeldFlagID = -1;
    lastShotUpdateTime = TimeKeeper::getCurrent().getSeconds();
    setupPhysicsData();
}

GameKeeper::Player::Player(int _playerIndex, bz_ServerSidePlayerHandler* handler)
    : _LSAState(start),
      _JoinState(waitingForAuthentication),
      player(_playerIndex), netHandler(0),
      lagInfo(&player),
      stateTimeStamp(0.0f), serverTimeStamp(0.0),
      gameTimeRate(GameTime::startRate), gameTimeNext(TimeKeeper::getCurrent()),
      isParting(false), hasEntered(false),
      playerHandler(handler),
      addWasDelayed(false), hadEnter(false), addDelayStartTime(0.0),
      playerIndex(_playerIndex), closed(false), clientCallback(0),
      needThisHostbanChecked(false), idFlag(0)
{
    playerList[playerIndex] = this;

    lastState.order = 0;
    score.playerID = _playerIndex;
    lastHeldFlagID = -1;
    lastShotUpdateTime = TimeKeeper::getCurrent().getSeconds();
    setupPhysicsData();
}

GameKeeper::Player::~Player()
{
    flagHistory.clear();
    playerList[playerIndex] = 0;
}

void GameKeeper::Player::setupPhysicsData()
{
    // setup the dimension properties
    base_dimensions[0] = 0.5f * BZDB.eval(StateDatabase::BZDB_TANKHEIGHT);
    base_dimensions[1] = 0.5f *  BZDB.eval(StateDatabase::BZDB_TANKWIDTH);
    base_dimensions[2] = BZDB.eval(StateDatabase::BZDB_TANKHEIGHT);

    updateDimensions();
}

void GameKeeper::Player::updateDimensions()
{
    float dimensionsTarget[3];

    dimensionsTarget[0] = 1.0f;
    dimensionsTarget[1] = 1.0f;
    dimensionsTarget[2] = 1.0f;

    if (player.haveFlag())
    {
        if (getFlagEffect() == FlagEffect::Obesity)
        {
            const float factor = BZDB.eval(StateDatabase::BZDB_OBESEFACTOR);
            dimensionsTarget[0] = factor;
            dimensionsTarget[1] = factor;
        }
        else if (getFlagEffect() == FlagEffect::Tiny)
        {
            const float factor = BZDB.eval(StateDatabase::BZDB_TINYFACTOR);
            dimensionsTarget[0] = factor;
            dimensionsTarget[1] = factor;
        }
        else if (getFlagEffect() == FlagEffect::Thief)
        {
            const float factor = BZDB.eval(StateDatabase::BZDB_THIEFTINYFACTOR);
            dimensionsTarget[0] = factor;
            dimensionsTarget[1] = factor;
        }
        else if (getFlagEffect() == FlagEffect::Narrow)
            dimensionsTarget[1] = 0.001f;
    }

    for (int i = 0; i < 3; i++)
        current_dimensions[i] = base_dimensions[i] * dimensionsTarget[i];
}

int GameKeeper::Player::count()
{
    Player* playerData(0);
    int     count = 0;

    for (int i = 0; i < PlayerSlot; i++)
        if ((playerData = playerList[i]) && !playerData->closed
                && playerData->player.isPlaying())
            count++;
    return count;
}

void GameKeeper::Player::updateLatency(float &waitTime)
{
    Player* playerData(0);

    for (int p = 0; p < PlayerSlot; ++p)
    {
        if ((playerData = playerList[p]) && !playerData->closed)
        {
            // get time for next lagping
            playerData->lagInfo.updateLatency(waitTime);
        }
    }
}

void GameKeeper::Player::dumpScore()
{
    Player* playerData(0);

    std::cout << "\n#players\n";
    for (int p = 0; p < PlayerSlot; ++p)
    {
        if ((playerData = playerList[p]) && !playerData->closed
                && playerData->player.isPlaying())
        {
            playerData->score.dump();
            std::cout << ' ' << playerData->player.getCallSign() << std::endl;
        }
    }
}

int GameKeeper::Player::anointRabbit(int oldRabbit)
{
    float topRatio(-100000.0f);
    int   newRabbit(NoPlayer);

    Player* playerData(0);
    bool    goodRabbitSelected(false);

    for (int i = 0; i < PlayerSlot; ++i)
    {
        if ((playerData = playerList[i]) && !playerData->closed
                && playerData->player.canBeRabbit(true))
        {
            bool  goodRabbit(i != oldRabbit && playerData->player.isAlive());
            float ratio(playerData->score.ranking());
            bool  select(false);
            if (goodRabbitSelected)
            {
                if (goodRabbit && (ratio > topRatio))
                    select = true;
            }
            else
            {
                if (goodRabbit)
                {
                    select = true;
                    goodRabbitSelected = true;
                }
                else
                {
                    if (ratio > topRatio) select = true;
                }
            }
            if (select)
            {
                topRatio = ratio;
                newRabbit = i;
            }
        }
    }
    return newRabbit;
}

void GameKeeper::Player::updateNextGameTime()
{
    if (gameTimeRate < GameTime::startRate)
        gameTimeRate = GameTime::startRate;
    else if (gameTimeRate < GameTime::finalRate)
        gameTimeRate = gameTimeRate * 1.25f;
    else
        gameTimeRate = GameTime::finalRate;
    gameTimeNext = TimeKeeper::getCurrent();
    gameTimeNext += gameTimeRate;
    return;
}

void GameKeeper::Player::packAdminInfo(MessageBuffer::Ptr msg)
{
    if (netHandler == 0)
    {
        msg->packUByte(5);
        msg->packUByte(playerIndex);
        msg->packUByte(127);
        msg->packUByte(0);
        msg->packUByte(0);
        msg->packUByte(1);
    }
    else
    {
        msg->packUByte(netHandler->sizeOfIP());
        msg->packUByte(playerIndex);
        msg->legacyPack(netHandler->packAdminInfo(msg->current_buffer()));
    }
}

void GameKeeper::Player::packPlayerInfo(MessageBuffer::Ptr msg)
{
    PackPlayerInfo(msg, playerIndex, accessInfo.getPlayerProperties());
}

void GameKeeper::Player::packPlayerUpdate(MessageBuffer::Ptr msg)
{
    msg->packUByte(playerIndex);
    msg->legacyPack(player.packUpdate(msg->current_buffer()));
    score.pack(msg);
    msg->legacyPack(player.packId(msg->current_buffer()));
}

void GameKeeper::Player::setPlayerAddMessage(PlayerAddMessage& msg)
{
    msg.playerID = playerIndex;
    msg.team = player.getTeam();
    msg.type = player.getType();
    msg.wins = score.getWins();
    msg.losses = score.getLosses();
    msg.tks = score.getTKs();
    msg.callsign = player.getCallSign();
    msg.motto = player.getMotto();
}


std::vector<int> GameKeeper::Player::allowed(PlayerAccessInfo::AccessPerm right,
        int targetPlayer)
{
    std::vector<int> receivers;
    Player* playerData(0);

    if (targetPlayer != -1)
    {
        if ((playerData = playerList[targetPlayer]) && !playerData->closed
                && playerData->accessInfo.hasPerm(right))
            receivers.push_back(targetPlayer);
    }
    else
    {
        for (int i = 0; i < PlayerSlot; ++i)
        {
            if ((playerData = playerList[i]) && !playerData->closed
                    && playerData->accessInfo.hasPerm(right))
                receivers.push_back(i);
        }
    }

    return receivers;
}

bool GameKeeper::Player::loadEnterData(uint16_t& rejectCode,
                                       char* rejectMsg)
{
    // look if there is as name clash, we won't allow this
    for (int i = 0; i < PlayerSlot; i++)
    {
        Player* otherData(playerList[i]);
        if (i == playerIndex || !otherData || !otherData->player.isPlaying())
            continue;
        if (otherData->closed) continue;
        if (!strcasecmp(otherData->player.getCallSign(), player.getCallSign()))
        {
            rejectCode = RejectRepeatCallsign;
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
    Player* playerData(0);
    int slot(-1); // invalid

    if (sscanf(name.c_str(), "#%d", &slot) == 1)
    {
        if (!GameKeeper::Player::getPlayerByIndex(slot)) return -1;
        return slot;
    }
    else
    {
        for (int i = 0; i < PlayerSlot; ++i)
        {
            if ((playerData = playerList[i]) && !playerData->closed
                    && (TextUtils::compare_nocase(playerData->player.getCallSign(), name) == 0))
                return i;
        }
    }
    return -1;
}


void GameKeeper::Player::reloadAccessDatabase()
{
    Player* playerData(0);
    for (int i = 0; i < PlayerSlot; ++i)
        if ((playerData = playerList[i]) && !playerData->closed)
            playerData->accessInfo.reloadInfo();
}

void GameKeeper::Player::close()
{
    closed = true;
}

bool GameKeeper::Player::clean()
{
    Player* playerData(0);
    // Trying to detect if this action cleaned the array of player
    bool empty(true);
    bool ICleaned(false);
    for (int i = 0; i < PlayerSlot; ++i)
    {
        if ((playerData = playerList[i]))
        {
            if (playerData->closed)
            {
                playerList[i] = 0;
                delete playerData;
                ICleaned = true;
            }
            else
                empty = false;
        }
    }
    return empty && ICleaned;
}

int GameKeeper::Player::getFreeIndex(int min, int max)
{
    for (int i = min; i < max; ++i)
        if (!playerList[i]) return i;
    return max;
}

void GameKeeper::Player::handleTcpPacket(fd_set* set)
{
    if (netHandler->isFdSet(set))
    {
        RxStatus const e(netHandler->tcpReceive());
        if (e == ReadPart) return;
        clientCallback(*netHandler, playerIndex, e);
    }
}

void GameKeeper::Player::setPlayerState(float pos[3], float azimuth)
{
    serverTimeStamp = (float)TimeKeeper::getCurrent().getSeconds();
    memcpy(lastState.pos, pos, sizeof(float) * 3);
    lastState.azimuth = azimuth;
    // Set Speeds to 0 too
    memset(lastState.velocity, 0, sizeof(float) * 3);
    lastState.angVel = 0.0f;
    stateTimeStamp = 0.0f;

    // player is alive.
    player.setAlive();
}

unsigned int GameKeeper::Player::maxShots(0);

void GameKeeper::Player::setMaxShots(unsigned int _maxShots)
{
    maxShots = _maxShots;
}

float GetShotLifetime(FlagType::Ptr flagType)
{
    float lifeTime(BZDB.eval(StateDatabase::BZDB_RELOADTIME));
    if (flagType->flagEffect == FlagEffect::RapidFire)
        lifeTime *= BZDB.eval(StateDatabase::BZDB_RFIREADLIFE);
    else if (flagType->flagEffect == FlagEffect::MachineGun)
        lifeTime *= BZDB.eval(StateDatabase::BZDB_MGUNADLIFE);
    else if (flagType->flagEffect == FlagEffect::GuidedMissile)
        lifeTime *= BZDB.eval(StateDatabase::BZDB_GMADLIFE) + .01f;
    else if (flagType->flagEffect == FlagEffect::Laser)
        lifeTime *= BZDB.eval(StateDatabase::BZDB_LASERADLIFE);
    else if (flagType->flagEffect == FlagEffect::ShockWave)
        lifeTime *= BZDB.eval(StateDatabase::BZDB_SHOCKADLIFE);
    else if (flagType->flagEffect == FlagEffect::Thief)
        lifeTime *= BZDB.eval(StateDatabase::BZDB_THIEFADLIFE);

    return lifeTime;
}

bool GameKeeper::Player::canShoot()
{
    if (shotSlots.size() != maxShots)
    {
        shotSlots.resize(maxShots);

        for (int i = 0; i < (int)shotSlots.size(); i++)
            shotSlots[i].slotID = i;
    }

    float now((float)TimeKeeper::getCurrent().getSeconds());

    // find a slot
    for (auto slot : shotSlots)
    {
        if (!slot.reloading && now >= slot.expireTime)
            return true;
    }

    return false;
}

bool GameKeeper::Player::isValidShotToShoot(FiringInfo &firingInfo)
{
    if (shotSlots.size() != maxShots)
    {
        shotSlots.resize(maxShots);

        for (int i = 0; i < (int)shotSlots.size(); i++)
            shotSlots[i].slotID = i;
    }

    if (firingInfo.localID == 0xFF)
        return true; // it's a non slot shot accept it, the server generated it.

    float now((float)TimeKeeper::getCurrent().getSeconds());

    // find a slot
    for (auto slot : shotSlots)
    {
        if (slot.slotID == firingInfo.localID)
            return !slot.reloading && now >= slot.expireTime;
    }

    return false;
}

bool GameKeeper::Player::addShot(Shot::Ptr shot)
{
    double now = TimeKeeper::getCurrent().getSeconds();

    uint16_t slotID = shot->Info.localID;

    if (slotID == 0xFF || slotID >= shotSlots.size())
        return true;    // the shot doesn't take up a slot

    double lifeTime = GetShotLifetime(shot->Info.flagType) / Attributes.reloadMultiplyer;

    shotSlots[slotID].activeShot = shot;
    shotSlots[slotID].reloading = true;
    shotSlots[slotID].expireTime = now + lifeTime; // this is now when the SLOT reloads not the shot expires.
    return true;
}

bool GameKeeper::Player::removeShot(int guid)
{
    Shot::Ptr shot = ShotManager.FindShot(guid);
    if (shot == nullptr || shot->Info.shot.player != getIndex() || shot->Info.localID >= shotSlots.size())
        return false;

    shotSlots[shot->Info.localID].activeShot = nullptr;
    return true;
}

void GameKeeper::Player::updateShotSlots()
{
    double now = TimeKeeper::getCurrent().getSeconds();

    for (auto slot : shotSlots)
    {
        if (slot.reloading && slot.expireTime >= now)
        {
            slot.reloading = false;
            slot.activeShot = nullptr;
        }
    }

    lastShotUpdateTime = now;
}

std::vector<float> GameKeeper::Player::getSlotReloads()
{
    double now = TimeKeeper::getCurrent().getSeconds();

    std::vector<float> reloads(shotSlots.size());
    for (auto slot : shotSlots)
    {
        if (slot.reloading)
            reloads.push_back((float)(slot.expireTime - now));
        else
            reloads.push_back(0);
    }

    return reloads;
}

bool GameKeeper::Player::validTeamTarget(const GameKeeper::Player *possibleTarget) const
{
    if (possibleTarget == nullptr)
        return false;

    TeamColor myTeam = player.getTeam();
    TeamColor targetTeam = possibleTarget->player.getTeam();
    if (myTeam != targetTeam || !allowTeams())
        return true;

    if (myTeam != RogueTeam)
        return false;

    return clOptions->gameType != RabbitChase;
}

const Obstacle*  GameKeeper::Player::getHitBuilding(const float* p, float a, bool phased, bool& expelled) const
{
    const float* dims = getDimensions();
    const Obstacle* obstacle = world->hitBuilding(p, a, dims[0], dims[1], dims[2]);

    expelled = (obstacle != NULL);
    if (expelled && phased)
        expelled = (obstacle->getType() == WallObstacle::getClassName() || obstacle->getType() == Teleporter::getClassName()
                    || (getFlagEffect() == FlagEffect::OscillationOverthruster && desiredSpeed < 0.0f && p[2] == 0.0f));
    return obstacle;
}

const Obstacle*  GameKeeper::Player::getHitBuilding(const float* oldP, float oldA, const float* p, float a, bool phased,
        bool& expelled)
{
    const bool hasOOflag = getFlagEffect() == FlagEffect::OscillationOverthruster;
    const float* dims = getDimensions();
    const Obstacle* obstacle = world->hitBuilding(oldP, oldA, p, a, dims[0], dims[1], dims[2], !hasOOflag);

    expelled = (obstacle != NULL);
    if (expelled && phased)
        expelled = (obstacle->getType() == WallObstacle::getClassName() || obstacle->getType() == Teleporter::getClassName()
                    || (hasOOflag && desiredSpeed < 0.0f && p[2] == 0.0f));

    if (obstacle != NULL)
    {
        if (obstacle->getType() == MeshFace::getClassName())
        {
            const MeshFace* face = (const MeshFace*)obstacle;
            const int driver = face->getPhysicsDriver();
            const PhysicsDriver* phydrv = PHYDRVMGR.getDriver(driver);
            if ((phydrv != NULL) && phydrv->getIsDeath())
                deathPhyDrv = driver;
        }
    }

    return obstacle;
}

bool GameKeeper::Player::getHitNormal(const Obstacle* o, const float* pos1, float azimuth1, const float* pos2,
                                      float azimuth2, float* normal) const
{
    const float* dims = getDimensions();
    return o->getHitNormal(pos1, azimuth1, pos2, azimuth2, dims[0], dims[1], dims[2], normal);
}

float GameKeeper::Player::getHandicapFactor()
{
    float normalizedHandicap = float(score.getHandicap()) / std::max(1.0f,
                               BZDB.eval(StateDatabase::BZDB_HANDICAPSCOREDIFF));

    /* limit how much of a handicap is afforded, and only provide
    * handicap advantages instead of disadvantages.
    */
    if (normalizedHandicap > 1.0f)
        // advantage
        normalizedHandicap = 1.0f;
    else if (normalizedHandicap < 0.0f)
        // disadvantage
        normalizedHandicap = 0.0f;

    return normalizedHandicap;
}

void  GameKeeper::Player::setDesiredSpeed(float fracOfMaxSpeed)
{
    FlagType::Ptr flag = getFlagType();
    // can't go faster forward than at top speed, and backward at half speed
    if (fracOfMaxSpeed > 1.0f) fracOfMaxSpeed = 1.0f;
    else if (fracOfMaxSpeed < -0.5f) fracOfMaxSpeed = -0.5f;

    // oscillation overthruster tank in building can't back up
    if (fracOfMaxSpeed < 0.0f && currentState.Status == bz_eTankStatus::InBuilding
            &&  flag->flagEffect == FlagEffect::OscillationOverthruster)
        fracOfMaxSpeed = 0.0f;

    float tankSpeed = BZDB.eval(StateDatabase::BZDB_TANKSPEED);

    // boost speed for certain flags
    if (flag->flagEffect == FlagEffect::Velocity)
        fracOfMaxSpeed *= BZDB.eval(StateDatabase::BZDB_VELOCITYAD);
    else if (flag->flagEffect == FlagEffect::Thief)
        fracOfMaxSpeed *= BZDB.eval(StateDatabase::BZDB_THIEFVELAD);
    else if ((flag->flagEffect == FlagEffect::Burrow) && (currentState.pos[2] < 0.0f))
        fracOfMaxSpeed *= BZDB.eval(StateDatabase::BZDB_BURROWSPEEDAD);
    else if ((flag->flagEffect == FlagEffect::ForwardOnly) && (fracOfMaxSpeed < 0.0))
        fracOfMaxSpeed = 0.0f;
    else if ((flag->flagEffect == FlagEffect::ReverseOnly) && (fracOfMaxSpeed > 0.0))
        fracOfMaxSpeed = 0.0f;
    else if (flag->flagEffect == FlagEffect::Agility)
    {
        if ((TimeKeeper::getTick() - agilityTime) < BZDB.eval(StateDatabase::BZDB_AGILITYTIMEWINDOW))
            fracOfMaxSpeed *= BZDB.eval(StateDatabase::BZDB_AGILITYADVEL);
        else
        {
            float oldFrac = desiredSpeed / tankSpeed;
            if (oldFrac > 1.0f)
                oldFrac = 1.0f;
            else if (oldFrac < -0.5f)
                oldFrac = -0.5f;
            float limit = BZDB.eval(StateDatabase::BZDB_AGILITYVELDELTA);
            if (fracOfMaxSpeed < 0.0f)
                limit /= 2.0f;
            if (fabs(fracOfMaxSpeed - oldFrac) > limit)
            {
                fracOfMaxSpeed *= BZDB.eval(StateDatabase::BZDB_AGILITYADVEL);
                agilityTime = TimeKeeper::getTick();
            }
        }
    }

    // apply handicap advantage to tank speed
    fracOfMaxSpeed *= (1.0f + (getHandicapFactor() * (BZDB.eval(StateDatabase::BZDB_HANDICAPVELAD) - 1.0f)));

    // set desired speed
    desiredSpeed = tankSpeed * fracOfMaxSpeed;
    currentState.speed = desiredSpeed;
}


void GameKeeper::Player::setTeleport(const TimeKeeper& t, short from, short to)
{
    if (!player.isAlive()) return;
    teleportTime = t;
    fromTeleporter = from;
    toTeleporter = to;
    currentState.setPStatus(currentState.getPStatus() | short(PlayerState::Teleporting));
}

void GameKeeper::Player::setPhysicsDriver(int driver)
{
    currentState.phydrv = driver;

    const PhysicsDriver* phydrv = PHYDRVMGR.getDriver(driver);
    if (phydrv != NULL)
        currentState.setPStatus(currentState.getPStatus() | short(PlayerState::OnDriver));
    else
        currentState.setPStatus(currentState.getPStatus() & ~short(PlayerState::OnDriver));
}

void  GameKeeper::Player::setDesiredAngVel(float fracOfMaxAngVel)
{
    FlagType::Ptr flag = getFlagType();

    // limit turn speed to maximum
    if (fracOfMaxAngVel > 1.0f) fracOfMaxAngVel = 1.0f;
    else if (fracOfMaxAngVel < -1.0f) fracOfMaxAngVel = -1.0f;

    // further limit turn speed for certain flags
    if (fracOfMaxAngVel < 0.0f && flag->flagEffect == FlagEffect::LeftTurnOnly)
        fracOfMaxAngVel = 0.0f;
    else if (fracOfMaxAngVel > 0.0f && flag->flagEffect == FlagEffect::RightTurnOnly)
        fracOfMaxAngVel = 0.0f;

    // boost turn speed for other flags
    if (flag->flagEffect == FlagEffect::QuickTurn)
        fracOfMaxAngVel *= BZDB.eval(StateDatabase::BZDB_ANGULARAD);
    else if ((flag->flagEffect == FlagEffect::Burrow) && (currentState.pos[2] < 0.0f))
        fracOfMaxAngVel *= BZDB.eval(StateDatabase::BZDB_BURROWANGULARAD);

    // apply handicap advantage to tank speed
    fracOfMaxAngVel *= (1.0f + (getHandicapFactor()  * (BZDB.eval(StateDatabase::BZDB_HANDICAPANGAD) - 1.0f)));

    // set desired turn speed
    desiredAngVel = fracOfMaxAngVel * BZDB.eval(StateDatabase::BZDB_TANKANGVEL);
    currentState.angVel = desiredAngVel;
}


bool notInObstacleList(const Obstacle* obs, const std::vector<const Obstacle*>& list)
{
    for (unsigned int i = 0; i < list.size(); i++)
    {
        if (obs == list[i])
            return false;
    }
    return true;
}


void GameKeeper::Player::collectInsideBuildings()
{
    const float* pos = currentState.pos;
    const float angle = currentState.rot;
    const float* dims = getDimensions();

    // get the list of possible inside buildings
    const ObsList* olist = COLLISIONMGR.boxTest(pos, angle, dims[0], dims[1], dims[2]);

    for (int i = 0; i < olist->count; i++)
    {
        const Obstacle* obs = olist->list[i];
        if (obs->inBox(pos, angle, dims[0], dims[1], dims[2]))
        {
            if (obs->getType() == MeshFace::getClassName())
            {
                const MeshFace* face = (const MeshFace*)obs;
                const MeshObstacle* mesh = (const MeshObstacle*)face->getMesh();
                // check it for the death touch
                if (deathPhyDrv < 0)
                {
                    const int driver = face->getPhysicsDriver();
                    const PhysicsDriver* phydrv = PHYDRVMGR.getDriver(driver);
                    if ((phydrv != NULL) && (phydrv->getIsDeath()))
                        deathPhyDrv = driver;
                }
                // add the mesh if not already present
                if (!obs->isDriveThrough() &&
                        notInObstacleList(mesh, insideBuildings))
                    insideBuildings.push_back(mesh);
            }
            else if (!obs->isDriveThrough())
            {
                if (obs->getType() == MeshObstacle::getClassName())
                {
                    // add the mesh if not already present
                    if (notInObstacleList(obs, insideBuildings))
                        insideBuildings.push_back(obs);
                }
                else
                    insideBuildings.push_back(obs);
            }
        }
    }
}

void GameKeeper::Player::move(const float* _pos, float _azimuth)
{
    // assumes _forward is normalized
    currentState.pos[0] = _pos[0];
    currentState.pos[1] = _pos[1];
    currentState.pos[2] = _pos[2];
    currentState.rot = _azimuth;

    // limit angle
    if (currentState.rot < 0.0f)
        currentState.rot = (float)((2.0 * M_PI) - fmodf(-currentState.rot, (float)(2.0 * M_PI)));
    else if (currentState.rot >= (2.0f * M_PI))
        currentState.rot = fmodf(currentState.rot, (float)(2.0 * M_PI));

    // update forward vector (always in horizontal plane)
    currentState.forward[0] = cosf(currentState.rot);
    currentState.forward[1] = sinf(currentState.rot);
    currentState.forward[2] = 0.0f;
}

void GameKeeper::Player::update()
{
    updateShotSlots();
}

void GameKeeper::Player::setPlayerState(PlayerState state, float timestamp)
{
    lagInfo.updateLag(timestamp, state.order - lastState.order > 1);
    player.updateIdleTime();
    lastState = state;
    stateTimeStamp = timestamp;
    serverTimeStamp = (float)TimeKeeper::getCurrent().getSeconds();
}

void GameKeeper::Player::getPlayerState(float pos[3], float &azimuth)
{
    memcpy(pos, lastState.pos, sizeof(float) * 3);
    azimuth = lastState.azimuth;
}


void GameKeeper::Player::calcRelativeMotion(float vel[2], float& speed, float& angVel) const
{
    vel[0] = currentState.vec[0];
    vel[1] = currentState.vec[1];

    angVel = lastUpdate.angVel;

    const PhysicsDriver* phydrv = PHYDRVMGR.getDriver(lastUpdate.phydrv);
    if (phydrv != NULL)
    {
        const float* v = phydrv->getLinearVel();
        const float av = phydrv->getAngularVel();
        const float* ap = phydrv->getAngularPos();

        // adjust for driver velocity
        vel[0] -= v[0];
        vel[1] -= v[1];

        // adjust for driver angular velocity
        if (av != 0.0f)
        {
            const float dx = lastUpdate.pos[0] - ap[0];
            const float dy = lastUpdate.pos[1] - ap[1];
            vel[0] += av * dy;
            vel[1] -= av * dx;
            angVel = lastUpdate.angVel - av;
        }
    }

    // speed relative to the tank's direction
    // (could use forward[] instead of re-doing the trig, but this is
    //  used in the setDeadReckoning(), when forward[] is not yet set)
    speed = (vel[0] * cosf(lastUpdate.rot)) + (vel[1] * sinf(lastUpdate.rot));
}

void GameKeeper::Player::getDeadReckoning(float* predictedPos, float* predictedAzimuth, float* predictedVel,
        float dt) const
{
    float inputRelVel[2];               // relative velocity
    float inputRelSpeed;                // relative speed
    float inputRelAngVel;               // relative angular velocity

    *predictedAzimuth = lastUpdate.rot;

    calcRelativeMotion(inputRelVel, inputRelSpeed, inputRelAngVel);

    //bool inputTurning = false;
    float inputTurnVector[2];
    float inputTurnCenter[2];

    if (fabsf(inputRelAngVel) > 0.001f)
    {
        //inputTurning = true;
        const float radius = (inputRelSpeed / inputRelAngVel);
        inputTurnVector[0] = +sinf(lastUpdate.rot) * radius;
        inputTurnVector[1] = -cosf(lastUpdate.rot) * radius;
        inputTurnCenter[0] = lastUpdate.pos[0] - inputTurnVector[0];
        inputTurnCenter[1] = lastUpdate.pos[1] - inputTurnVector[1];
    }
    else
    {
        inputTurnVector[0] = 0;
        inputTurnVector[1] = 0;
        inputTurnCenter[0] = 0;
        inputTurnCenter[1] = 0;
    }

    float gravity = BZDB.eval(StateDatabase::BZDB_GRAVITY);

    if (currentState.pStatus & PlayerState::Falling)
    {
        // no control when falling
        predictedVel[0] = lastUpdate.vec[0];
        predictedVel[1] = lastUpdate.vec[1];
        predictedPos[0] = lastUpdate.pos[0] + (dt * lastUpdate.vec[0]);
        predictedPos[1] = lastUpdate.pos[1] + (dt * lastUpdate.vec[1]);

        // only turn if alive
        if (currentState.pStatus & PlayerState::Alive)
            *predictedAzimuth += (dt * lastUpdate.angVel);

        // following the parabola
        predictedVel[2] = lastUpdate.vec[2] + (gravity * dt);
        predictedPos[2] = lastUpdate.pos[2] + (lastUpdate.vec[2] * dt) + (0.5f * gravity * dt * dt);
    }
    else
    {
        // velocity[2] is zero when not falling, except for Burrow flag
        predictedVel[2] = lastUpdate.vec[2];
        predictedPos[2] = lastUpdate.pos[2] + (lastUpdate.vec[2] * dt);

        // different algorithms for tanks moving in
        // a straight line vs. turning in a circle
        if (abs(lastUpdate.angVel) > 0.001)
        {
            // move straight
            predictedVel[0] = inputRelVel[0];
            predictedVel[1] = inputRelVel[1];
            predictedPos[0] = lastUpdate.pos[0] + (dt * inputRelVel[0]);
            predictedPos[1] = lastUpdate.pos[1] + (dt * inputRelVel[1]);
        }
        else
        {
            // make a sweeping arc
            const float angle = (dt * inputRelAngVel);
            *predictedAzimuth += angle;
            const float cos_val = cosf(angle);
            const float sin_val = sinf(angle);
            const float* tc = inputTurnCenter;
            const float* tv = inputTurnVector;
            predictedPos[0] = tc[0] + ((tv[0] * cos_val) - (tv[1] * sin_val));
            predictedPos[1] = tc[1] + ((tv[1] * cos_val) + (tv[0] * sin_val));
            const float* rv = inputRelVel;
            predictedVel[0] = (rv[0] * cos_val) - (rv[1] * sin_val);
            predictedVel[1] = (rv[1] * cos_val) + (rv[0] * sin_val);
        }

        // make the physics driver adjustments
        const PhysicsDriver* phydrv = PHYDRVMGR.getDriver(lastUpdate.phydrv);
        if (phydrv != NULL)
        {
            if (phydrv->getIsSlide())
            {
                predictedVel[0] = inputRelVel[0];
                predictedVel[1] = inputRelVel[1];
                predictedPos[0] = lastUpdate.pos[0] + (dt * inputRelVel[0]);
                predictedPos[1] = lastUpdate.pos[1] + (dt * inputRelVel[1]);
            }
            else
            {
                // angular velocity adjustment
                const float pdAngVel = phydrv->getAngularVel();
                if (pdAngVel != 0.0f)
                {
                    const float angle = (dt * pdAngVel);
                    *predictedAzimuth += angle;
                    const float* pdAngPos = phydrv->getAngularPos();
                    const float dx = predictedPos[0] - pdAngPos[0];
                    const float dy = predictedPos[1] - pdAngPos[1];
                    const float cos_val = cosf(angle);
                    const float sin_val = sinf(angle);
                    predictedPos[0] = pdAngPos[0] + ((dx * cos_val) - (dy * sin_val));
                    predictedPos[1] = pdAngPos[1] + ((dy * cos_val) + (dx * sin_val));
                    predictedVel[0] += (-dy * pdAngVel);
                    predictedVel[1] += (+dx * pdAngVel);
                }
                // linear velocity adjustment
                const float* pdVel = phydrv->getLinearVel();
                predictedPos[0] += (dt * pdVel[0]);
                predictedPos[1] += (dt * pdVel[1]);
                predictedVel[0] += pdVel[0];
                predictedVel[1] += pdVel[1];
            }
        }
    }

    return;
}

// for dead reckoning
static const float  MaxUpdateTime = 1.0f;       // seconds

bool GameKeeper::Player::isDeadReckoningWrong() const
{
    //const uint16_t checkStates = (PlayerState::Alive | PlayerState::Paused | PlayerState::Falling);

    // always send a new packet when some kinds of status change
    if ((currentState.pStatus != lastUpdate.pStatus))
        return true;

    // never send a packet when dead
    if ((currentState.pStatus & PlayerState::Alive) == 0)
        return false;

//     //  send a packet if we've made some noise
//     if (currentState.pStatus != PlayerState::NoSounds)
//         return true;

    //  send a packet if we've crossed a physics driver boundary
    if (currentState.phydrv != lastState.phydrv)
        return true;

    // time since setdeadreckoning
    const float dt = float(TimeKeeper::getCurrent() - lastUpdateSent);

    // otherwise always send at least one packet per second
    if (dt >= MaxUpdateTime)
        return true;

    // get predicted state
    float predictedPos[3];
    float predictedVel[3];
    float predictedAzimuth;
    getDeadReckoning(predictedPos, &predictedAzimuth, predictedVel, dt);

    // always send a new packet on reckoned touchdown
    float groundLimit = 0.0f;
    if (getFlagEffect() == FlagEffect::Burrow)
        groundLimit = BZDB.eval(StateDatabase::BZDB_BURROWDEPTH);
    if (predictedPos[2] < groundLimit)
        return true;

    // client side throttling
    const int throttleRate = int(BZDB.eval(StateDatabase::BZDB_UPDATETHROTTLERATE));
    const float minUpdateTime = (throttleRate > 0) ? (1.0f / throttleRate) : 0.0f;
    if (dt < minUpdateTime)
        return false;

    // see if position and azimuth are close enough
    float positionTolerance = BZDB.eval(StateDatabase::BZDB_POSITIONTOLERANCE);
    if ((fabsf(currentState.pos[0] - predictedPos[0]) > positionTolerance)
            || (fabsf(currentState.pos[1] - predictedPos[1]) > positionTolerance)
            || fabsf(currentState.pos[2] - predictedPos[2]) > positionTolerance)
        return true;

    float angleTolerance = BZDB.eval(StateDatabase::BZDB_ANGLETOLERANCE);
    if (fabsf(currentState.rot - predictedAzimuth) > angleTolerance)
        return true;

    // prediction is good enoughFisad
    return false;
}


void GameKeeper::Player::setLastIdFlag(int _idFlag)
{
    idFlag = _idFlag;
}

int GameKeeper::Player::getLastIdFlag()
{
    return idFlag;
}

FlagType::Ptr GameKeeper::Player::getFlagType() const
{
    if (!player.haveFlag())
        return Flags::Null;

    return FlagInfo::get(player.getFlag())->flag.type;
}

FlagEffect GameKeeper::Player::getFlagEffect() const
{
    if (!player.haveFlag())
        return Flags::Null->flagEffect;

    return getFlagType()->flagEffect;
}

void GameKeeper::Player::grantFlag(int _flag)
{
    player.setFlag(_flag);
    updateDimensions();

    FlagType::Ptr f = getFlagType();

    float worldSize = BZDB.eval(StateDatabase::BZDB_WORLDSIZE);

    // if it's bad then reset countdowns and set antidote flag
    if (f != Flags::Null && f->endurance == FlagEndurance::Sticky)
    {
        if (clOptions->gameOptions & ShakableGameStyle)
        {
            flagShakingTime = clOptions->shakeTimeout;
            flagShakingWins = clOptions->shakeWins;
        }

        if (clOptions->gameOptions & AntidoteGameStyle)
        {
            float tankRadius = BZDB.eval(StateDatabase::BZDB_TANKRADIUS);
            float tankHeight = BZDB.eval(StateDatabase::BZDB_TANKHEIGHT);
            float baseSize = BZDB.eval(StateDatabase::BZDB_BASESIZE);
            int tryCount = 0;
            do
            {
                tryCount++;
                if (tryCount > 100) // if it takes this long, just screw it.
                    break;

                if (clOptions->gameType == ClassicCTF)
                {
                    flagAntidotePos[0] = 0.5f * worldSize * ((float)bzfrand() - 0.5f);
                    flagAntidotePos[1] = 0.5f * worldSize * ((float)bzfrand() - 0.5f);
                    flagAntidotePos[2] = 0.0f;
                }
                else
                {
                    flagAntidotePos[0] = (worldSize - baseSize) * ((float)bzfrand() - 0.5f);
                    flagAntidotePos[1] = (worldSize - baseSize) * ((float)bzfrand() - 0.5f);
                    flagAntidotePos[2] = 0.0f;
                }
            }
            while (world->inBuilding(flagAntidotePos, tankRadius, tankHeight));
            hasAntidoteFlag = true;

            // TODO, send this over the wire to remote clients so they don't compute it themselves, then we can check for the drop on the server
        }
        else
            hasAntidoteFlag = false;
    }
    else
    {
        hasAntidoteFlag = false;
        flagShakingTime = 0.0f;
        flagShakingWins = 0;
    }
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
