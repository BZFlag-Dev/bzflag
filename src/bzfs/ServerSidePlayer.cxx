/* bzflag
 * Copyright (c) 1993-2018 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

// bzflag global header
#include "common.h"

#include "bzfsAPI.h"
#include "bzfsAPIServerSidePlayers.h"
#include "bzfs.h"
#include "StateDatabase.h"
#include "RejoinList.h"
#include "RobotUtils.h"
#include "ObstacleMgr.h"
#include "BaseBuilding.h"
#include "BoxBuilding.h"

// server side bot API

typedef std::map<FlagType::Ptr, std::pair<int, int> > FlagSuccessMap;

class bz_ServerSidePlayerHandler::Impl
{
public:
    FlagSuccessMap   flagSuccess;
    int      totalSum = 0;
    int      totalCnt = 0;

    TimeKeeper lastStuckTime;
    float stuckRot = 0.0f;
    float stuckSpeed = 0.0f;
    bool wantToJump = false;
    TimeKeeper lastShot;

    TimeKeeper lastNavChange;
    float navRot = 0.0f, navSpeed = 0.0f;

    int currentTarget = -1;
};

bz_ServerSidePlayerHandler::bz_ServerSidePlayerHandler() : playerID(-1), autoSpawn(true), flaps(0)
{
    pImpl = new Impl();
    input[0] = input[1] = 0;
    lastUpdate.rotVel = 0;
    lastUpdate.vec[0] = 0;
    lastUpdate.vec[1] = 0;
    lastUpdate.vec[2] = 0;
    lastUpdate.rot = 0;
    lastUpdate.pos[0] = lastUpdate.pos[1] = lastUpdate.pos[2] = 0;
    currentState = lastUpdate;
}

bz_ServerSidePlayerHandler::~bz_ServerSidePlayerHandler()
{
    delete(pImpl);
}

// higher level logic API
void bz_ServerSidePlayerHandler::spawned(void)
{
}

bool bz_ServerSidePlayerHandler::think(void)
{
    float forward = 0;
    float rot = currentState.rot;

    doAutoPilot(rot, forward);

    setMovement(forward, rot);

    return false;
}

void bz_ServerSidePlayerHandler::died ( int UNUSED(killer) )
{
    currentState.Status = bz_eTankStatus::Dead;
    pImpl->currentTarget = -1;
}

void bz_ServerSidePlayerHandler::smote ( SmiteReason UNUSED(reason) )
{
    currentState.Status = bz_eTankStatus::Dead;
    pImpl->currentTarget = -1;
}

void bz_ServerSidePlayerHandler::update ( void )
{
    think();
}


// lower level message API
void bz_ServerSidePlayerHandler::playerAdded(int) {}
void bz_ServerSidePlayerHandler::playerRemoved(int) {}

void bz_ServerSidePlayerHandler::playerSpawned(int id, const float _pos[3], float _rot)
{
    if (id==playerID)
    {
        // it was me, I'm not in limbo
        if (currentState.Status == bz_eTankStatus::Dead)
            currentState.Status = bz_eTankStatus::InAir; // maybe? next update will take care of placing us.

        // update the current state
        lastUpdate.time = bz_getCurrentTime();
        // get where I am;
        memcpy(lastUpdate.pos, _pos, sizeof(float) *3);
        lastUpdate.rotVel = 0;
        lastUpdate.vec[0] = 0;
        lastUpdate.vec[1] = 0;
        lastUpdate.vec[2] = 0;
        lastUpdate.rot = _rot;

        currentState = lastUpdate;

        input[0] = input[1] = 0;

        flaps = 0;
        // tell the high level API that we done spawned;
        spawned();
    }
}

//-------------------------------------------------------------------------

void bz_ServerSidePlayerHandler::textMessage(int, int, const char*) {}

void bz_ServerSidePlayerHandler::playerKilled(int victimIndex, int killerIndex,
        bz_ePlayerDeathReason reason, int UNUSED(shotIndex),
        const char *UNUSED(flagType), int UNUSED(phydrv))
{
    if (victimIndex != getPlayerID())
        return;

    if (reason == eGotShot || reason == eGotRunOver  || reason == eGenocideEffect)
        died(killerIndex);
    else if (reason == eGotCaptured)
        smote (eCaptureDeath);
    else if (reason == eWaterDeath || reason == ePhysicsDriverDeath)
        smote (eWorldDeath);
    else if (reason == eGotKilled || reason == eSelfDestruct)
        smote (eServerDeath);
    else
        smote (eOtherDeath);
}

void bz_ServerSidePlayerHandler::scoreLimitReached(int, bz_eTeamType) {}

void bz_ServerSidePlayerHandler::flagCaptured(int, bz_eTeamType) {}

void bz_ServerSidePlayerHandler::playerStateUpdate(int, bz_PlayerUpdateState *, double) {}

//void bz_ServerSidePlayerHandler::playerScoreUpdate(int, float, int, int, int) {}

void bz_ServerSidePlayerHandler::shotFired(int, unsigned short ) {}

void bz_ServerSidePlayerHandler::shotEnded(int, unsigned short, bool) {}

void bz_ServerSidePlayerHandler::playerTeleported( int, bz_PlayerUpdateState *, bz_PlayerUpdateState * ) {}

void bz_ServerSidePlayerHandler::setPlayerData(const char *callsign, const char *token, const char *clientVersion, bz_eTeamType _team)
{
    GameKeeper::Player *player = GameKeeper::Player::getPlayerByIndex(playerID);

    if (!player || player->playerHandler!=this)
        return ;

    player->player.setType(TankPlayer); // because we like to lie :)
    player->player.setTeam((TeamColor)convertTeam(_team));
    player->player.setCallSign(callsign);
    player->player.setToken(token);
    player->player.setClientVersion(clientVersion);

    uint16_t code = 0;
    char reason[512] = {0};
    if (!player->player.processEnter(code, reason))
        rejected((bz_eRejectCodes)code, reason);
}

//-------------------------------------------------------------------------

void bz_ServerSidePlayerHandler::joinGame(void)
{
    GameKeeper::Player *player = GameKeeper::Player::getPlayerByIndex(playerID);
    if (!player)
        return ;

    if (player->player.isAlive() || player->player.isPlaying())
        return ;

    player->lastState.order = 0;

    // set our state to signing on so we can join
    player->player.signingOn();
    playerAlive(playerID);
    player->player.setAlive();
    currentState.Status = bz_eTankStatus::InAir; // maybe? next update will take care of placing us.
}

//-------------------------------------------------------------------------

void bz_ServerSidePlayerHandler::getCurrentState(bz_PlayerUpdateState *state)
{
    GameKeeper::Player *player = GameKeeper::Player::getPlayerByIndex(playerID);
    if (!state || !player)
        return ;

    // TODO Make this actually update based on movement
    playerStateToAPIState(*state,player->lastState);
}

//-------------------------------------------------------------------------

void bz_ServerSidePlayerHandler::dropFlag(void)
{
    GameKeeper::Player *player = GameKeeper::Player::getPlayerByIndex(playerID);
    if (!player)
        return ;

    // TODO Make this actually update based on movement
    dropPlayerFlag(*player, player->lastState.pos);
}

//-------------------------------------------------------------------------

void bz_ServerSidePlayerHandler::sendServerCommand(const char *text)
{
    GameKeeper::Player *player = GameKeeper::Player::getPlayerByIndex(playerID);
    if (!player || !text)
        return ;

    ::sendPlayerMessage(player, AllPlayers, text);
}

void bz_ServerSidePlayerHandler::sendChatMessage(const char *text, int targetPlayer, bz_eMessageType type)
{
    GameKeeper::Player *player = GameKeeper::Player::getPlayerByIndex(playerID);
    if (!player || !text)
        return ;

    if (targetPlayer > LastRealPlayer)
        return ;

    PlayerId dstPlayer = targetPlayer==BZ_ALLUSERS ? AllPlayers : targetPlayer;

    MessageType msgtype = ChatMessage;

    if (type == eActionMessage)
        msgtype = ActionMessage;

    ::sendChatMessage(player->getIndex(),dstPlayer,text,msgtype);
}

//-------------------------------------------------------------------------

void bz_ServerSidePlayerHandler::sendTeamChatMessage(const char *text, bz_eTeamType targetTeam, bz_eMessageType type)
{
    GameKeeper::Player *player = GameKeeper::Player::getPlayerByIndex(playerID);
    if (!player || !text)
        return ;

    PlayerId dstPlayer = AllPlayers;

    switch (targetTeam)
    {
    case eRogueTeam:
    case eRedTeam:
    case eGreenTeam:
    case eBlueTeam:
    case ePurpleTeam:
    case eRabbitTeam:
    case eHunterTeam:
        dstPlayer = FirstTeam + (int)targetTeam;
        break;

    case eAdministrators:
        dstPlayer = AdminPlayers;
        break;
    default:
        break;
    }

    MessageType msgtype = ChatMessage;

    if (type == eActionMessage)
        msgtype = ActionMessage;

    ::sendChatMessage(player->getIndex(),dstPlayer,text,msgtype);
}

//-------------------------------------------------------------------------

void bz_ServerSidePlayerHandler::setMovement(float forward, float turn)
{
    if (input[0]==turn && input[1]==forward)
        return ;

    input[0]=turn;
    input[1]=forward;

    if (input[0] > 1.0f)
        input[0]=1.0f;
    if (input[1] > 1.0f)
        input[1]=1.0f;
    if (input[0] < -1.0f)
        input[0]=-1.0f;
    if (input[1] < -1.0f)
        input[1]=-1.0f;
}

extern RejoinList rejoinList;

void bz_ServerSidePlayerHandler::respawn()
{
    GameKeeper::Player *player = GameKeeper::Player::getPlayerByIndex(playerID);
    if (!player)
        return;

    if (isAlive() || player->player.getTeam() == ObserverTeam)
        return;

    // player is on the waiting list
    float waitTime = rejoinList.waitTime(playerID);
    if (waitTime > 0.0f)
    {
        // Make them wait for trying to rejoin quickly
        player->player.setSpawnDelay((double)waitTime);
        player->player.queueSpawn();
        return;
    }

    // player moved before countdown started
    if (clOptions->timeLimit > 0.0f && !countdownActive)
        player->player.setPlayedEarly();
    player->player.queueSpawn();
}

//-------------------------------------------------------------------------

bool bz_ServerSidePlayerHandler::fireShot(void)
{
    if (!canShoot())
        return false;

    return false;
}

//-------------------------------------------------------------------------

void bz_ServerSidePlayerHandler::jump(void)
{
    // do jump update
}

bool bz_ServerSidePlayerHandler::isAlive()
{
    return currentState.Status != bz_eTankStatus::Dead;
}

//-------------------------------------------------------------------------

bool bz_ServerSidePlayerHandler::canJump(void)
{
    return canMove() /*&& bz_allowJumping() */;
}

//-------------------------------------------------------------------------

bool bz_ServerSidePlayerHandler::canShoot(void)
{
    GameKeeper::Player *player = GameKeeper::Player::getPlayerByIndex(playerID);
    if (!player)
        return false;

    return isAlive() && player->canShoot();
}

//-------------------------------------------------------------------------

bool bz_ServerSidePlayerHandler::canMove(void)
{
    return isAlive() && !falling();
}

bool bz_ServerSidePlayerHandler::falling (void)
{
    GameKeeper::Player *player = GameKeeper::Player::getPlayerByIndex(playerID);
    if (!player)
        return false;

    return player->lastState.status & PlayerState::Falling ? true : false;
}

//-------------------------------------------------------------------------

void bz_ServerSidePlayerHandler::getPosition ( float *p )
{
    GameKeeper::Player *player = GameKeeper::Player::getPlayerByIndex(playerID);
    if (!player || !p)
        return;

    memcpy(p, player->lastState.pos, sizeof(float[3]));
}

void bz_ServerSidePlayerHandler::getVelocity ( float *v )
{
    GameKeeper::Player *player = GameKeeper::Player::getPlayerByIndex(playerID);
    if (!player ||!v)
        return;

    memcpy(v,player->lastState.velocity,sizeof(float)*3);
}

float bz_ServerSidePlayerHandler::getFacing ( void )
{
    GameKeeper::Player *player = GameKeeper::Player::getPlayerByIndex(playerID);
    if (!player)
        return 0.0;

    return player->lastState.azimuth;
}


float computeMaxLinVelocity(FlagType::Ptr flag, float z)
{
    float speed = BZDB.eval(StateDatabase::BZDB_TANKSPEED);

    if (flag)
    {
        if (flag->flagEffect == FlagEffect::Velocity)
            return speed * BZDB.eval(StateDatabase::BZDB_VELOCITYAD);
        else if (flag->flagEffect == FlagEffect::Thief)
            return speed * BZDB.eval(StateDatabase::BZDB_THIEFVELAD);
        else if (flag->flagEffect == FlagEffect::Burrow && z < 0.0f)
            return speed * BZDB.eval(StateDatabase::BZDB_BURROWSPEEDAD);
    }

    return speed;
}


float computeMaxAngleVelocity(FlagType::Ptr flag, float z)
{
    float angvel = BZDB.eval(StateDatabase::BZDB_TANKANGVEL);

    if (flag)
    {
        if (flag->flagEffect == FlagEffect::QuickTurn)
            return angvel * BZDB.eval(StateDatabase::BZDB_ANGULARAD);
        else if (flag->flagEffect == FlagEffect::QuickTurn && z < 0.0f)
            return angvel * BZDB.eval(StateDatabase::BZDB_BURROWANGULARAD);
    }

    return angvel;
}


float bz_ServerSidePlayerHandler::getMaxLinSpeed ( void )
{
    FlagType::Ptr flag = nullptr;
    GameKeeper::Player *player = GameKeeper::Player::getPlayerByIndex(playerID);

    if (player && player->player.haveFlag())
        flag = FlagInfo::get(player->player.getFlag())->flag.type;

    return computeMaxLinVelocity(flag,currentState.pos[2]);
}

float bz_ServerSidePlayerHandler::getMaxRotSpeed ( void )
{
    FlagType::Ptr flag = nullptr;
    GameKeeper::Player *player = GameKeeper::Player::getPlayerByIndex(playerID);

    if (player && player->player.haveFlag())
        flag = FlagInfo::get(player->player.getFlag())->flag.type;

    return computeMaxAngleVelocity(flag,currentState.pos[2]);
}

float bz_ServerSidePlayerHandler::UpdateInfo::getDelta( const UpdateInfo & state)
{
    // plot where we think we are now based on the current time
    double dt = state.time - time;

    float newPos[3];
    newPos[0] = pos[0] + (float)(vec[0] *dt);
    newPos[1] = pos[1] + (float)(vec[1] *dt);
    newPos[2] = pos[2] + (float)(vec[2] *dt);

    // that's where we thing we'll be based on movement

    float dx = newPos[0] - state.pos[0];
    float dy = newPos[1] - state.pos[1];
    float dz = newPos[1] - state.pos[2];

    // return the distance between where our projection is, and where state is
    return sqrt(dx*dx+dy*dy+dz*dz);
}

void bz_ServerSidePlayerHandler::doAutoPilot(float &rotation, float &speed)
{
    pImpl->wantToJump = false;

    dropHardFlags(); //Perhaps we should remove this and let learning do it's work
    if (!avoidBullet(rotation, speed))
    {
        if (!stuckOnWall(rotation, speed))
        {
            if (!chasePlayer(rotation, speed))
            {
                if (!lookForFlag(rotation, speed))
                    navigate(rotation, speed);
            }
        }
    }

    avoidDeathFall(rotation, speed);

    if (pImpl->wantToJump)
        jump();

    fireAtTank();
}

bool bz_ServerSidePlayerHandler::fireAtTank()
{
    GameKeeper::Player *player = GameKeeper::Player::getPlayerByIndex(playerID);
    if (player == nullptr)
        return false;

    FlagType::Ptr type = FlagInfo::get(player->player.getFlag())->flag.type;

    float pos[3];
    memcpy(pos, currentState.pos, sizeof(pos));
    if (pos[2] < 0.0f)
        pos[2] = 0.01f;
    float myAzimuth = currentState.rot;

    float dir[3] = { cosf(myAzimuth), sinf(myAzimuth), 0.0f };
    pos[2] += player->getMuzzleHeight();
    Ray tankRay(pos, dir);
    pos[2] -= player->getMuzzleHeight();

    float tankHeight = BZDB.eval(StateDatabase::BZDB_TANKHEIGHT);

    if (type->flagEffect == FlagEffect::ShockWave)
    {
        TimeKeeper now = TimeKeeper::getTick();
        if (canShoot())
        {
            bool hasSWTarget = false;
            for (int t = 0; t < maxPlayers; t++)
            {
                GameKeeper::Player *opponent = GameKeeper::Player::getPlayerByIndex(t);
                if (opponent == nullptr || opponent == player)
                    continue;

                if (opponent->player.isAlive() && !opponent->player.isPaused())
                {
                    const float *tp = opponent->lastState.pos;
                    float enemyPos[3];
                    //toss in some lag adjustment/future prediction - 300 millis
                    memcpy(enemyPos, tp, sizeof(enemyPos));
                    const float *tv = opponent->lastState.velocity;

                    enemyPos[0] += 0.3f * tv[0];
                    enemyPos[1] += 0.3f * tv[1];
                    enemyPos[2] += 0.3f * tv[2];
                    if (enemyPos[2] < 0.0f)
                        enemyPos[2] = 0.0f;

                    float dist = BotUtils::getTargetDistance(pos, enemyPos);
                    if (dist <= BZDB.eval(StateDatabase::BZDB_SHOCKOUTRADIUS))
                    {
                        if (!player->validTeamTarget(opponent))
                        {
                            hasSWTarget = false;
                            t = curMaxPlayers;
                        }
                        else
                            hasSWTarget = true;
                    }
                }
            }
            if (hasSWTarget)
            {
                fireShot();
                pImpl->lastShot = TimeKeeper::getTick();
                return true;
            }
        }
    }
    else
    {
        TimeKeeper now = TimeKeeper::getTick();
        if (canShoot())
        {
            float errorLimit = clOptions->maxShots * BZDB.eval(StateDatabase::BZDB_LOCKONANGLE) / 8.0f;
            float closeErrorLimit = errorLimit * 2.0f;

            for (int t = 0; t < maxPlayers; t++)
            {
                GameKeeper::Player *opponent = GameKeeper::Player::getPlayerByIndex(t);
                if (opponent == nullptr || opponent == player)
                    continue;

                if (opponent->player.isAlive() && !opponent->player.isPaused() && player->validTeamTarget(opponent))
                {
                    if (opponent->isPhantomZoned() && !player->isPhantomZoned() && (type->flagEffect != FlagEffect::SuperBullet)  && (type->flagEffect != FlagEffect::ShockWave))
                        continue;

                    const float *tp = opponent->lastState.pos;
                    float enemyPos[3];
                    //toss in some lag adjustment/future prediction - 300 millis
                    memcpy(enemyPos, tp, sizeof(enemyPos));
                    const float *tv = opponent->lastState.velocity;
                    enemyPos[0] += 0.3f * tv[0];
                    enemyPos[1] += 0.3f * tv[1];
                    enemyPos[2] += 0.3f * tv[2];
                    if (enemyPos[2] < 0.0f)
                        enemyPos[2] = 0.0f;

                    float dist = BotUtils::getTargetDistance(pos, enemyPos);

                    if ((type->flagEffect == FlagEffect::GuidedMissile) || (fabs(pos[2] - enemyPos[2]) < 2.0f * tankHeight))
                    {

                        float targetDiff = BotUtils::getTargetAngleDifference(pos, myAzimuth, enemyPos);
                        if ((targetDiff < errorLimit) || ((dist < (2.0f * BZDB.eval(StateDatabase::BZDB_SHOTSPEED))) && (targetDiff < closeErrorLimit)))
                        {
                            bool isTargetObscured;
                            if (type->flagEffect != FlagEffect::SuperBullet)
                                isTargetObscured = BotUtils::isLocationObscured(pos, enemyPos);
                            else
                                isTargetObscured = false;

                            if (!isTargetObscured)
                            {
                                fireShot();
                                pImpl->lastShot = now;
                                t = curMaxPlayers;
                                return true;
                            }
                        }
                    }
                }
            }
        }
    }

    return false;
}

bool bz_ServerSidePlayerHandler::avoidDeathFall(float & UNUSED(rotation), float &speed)
{
    GameKeeper::Player *player = GameKeeper::Player::getPlayerByIndex(playerID);
    if (player == nullptr)
        return false;

    FlagType::Ptr type = FlagInfo::get(player->player.getFlag())->flag.type;
    float tankHeight = BZDB.eval(StateDatabase::BZDB_TANKHEIGHT);

    float pos1[3], pos2[3];
    memcpy(pos1, currentState.pos, sizeof(pos1));
    memcpy(pos2, pos1, sizeof(pos1));
    pos1[2] += 10.0f * tankHeight;
    float azimuth = currentState.rot;
    if (speed < 0.0f)
        azimuth = fmodf(float(azimuth + M_PI), float(2.0 * M_PI));
    else
        azimuth = fmodf(float(azimuth), float(2.0 * M_PI));

    pos2[0] += 8.0f * tankHeight * cosf(azimuth);
    pos2[1] += 8.0f * tankHeight * sinf(azimuth);
    pos2[2] += 0.01f;

    float collisionPt[3];
    if (BotUtils::getFirstCollisionPoint(pos1, pos2, collisionPt))
    {
        if (collisionPt[2] < 0.0f)
            collisionPt[2] = 0.0f;
        if (collisionPt[2] < world->getWaterLevel())
        {
            speed = 0.0f;
            return true;
        }
    }
    else if (collisionPt[2] < (pos2[2] - 1.0f))
        speed *= 0.5f;

    return false;
}

const float* GetBasePostion(TeamColor team)
{
    auto bases = OBSTACLEMGR.getBases();
    for (size_t i = 0; i < bases.size(); i++)
    {
        const BaseBuilding* base = (const BaseBuilding*)bases[i];
        if (base == nullptr || base->getTeam() != team)
            continue;

        return base->getPosition();
    }

    return nullptr;
}

bool bz_ServerSidePlayerHandler::navigate(float &rotation, float &speed)
{
    if ((TimeKeeper::getTick() - pImpl->lastNavChange) < 1.0f)
    {
        rotation = pImpl->navRot;
        speed = pImpl->navSpeed;
        return true;
    }

    GameKeeper::Player *player = GameKeeper::Player::getPlayerByIndex(playerID);
    if (player == nullptr)
        return false;

    FlagType::Ptr type = FlagInfo::get(player->player.getFlag())->flag.type;

    float pos[3];

    memcpy(pos, currentState.pos, sizeof(pos));
    if (pos[2] < 0.0f)
        pos[2] = 0.01f;
    float myAzimuth = currentState.rot;

    float leftDistance = BotUtils::getOpenDistance(pos, (float)(myAzimuth + (M_PI / 4.0)));
    float centerDistance = BotUtils::getOpenDistance(pos, myAzimuth);
    float rightDistance = BotUtils::getOpenDistance(pos, (float)(myAzimuth - (M_PI / 4.0)));
    if (leftDistance > rightDistance)
    {
        if (leftDistance > centerDistance)
            rotation = 0.75f;
        else
            rotation = 0.0f;
    }
    else
    {
        if (rightDistance > centerDistance)
            rotation = -0.75f;
        else
            rotation = 0.0f;
    }
    if (type->flagTeam != NoTeam)
    {
        const float *temp = GetBasePostion(player->player.getTeam());
        if (temp == nullptr)
        {
            dropFlag();
        }
        else
        {
            if ((((int) *(temp) + 2 >= (int) *(pos)) || (temp[0] == pos[0] && temp[1] == pos[1])) && type->flagTeam == player->player.getTeam())
            {
                dropFlag();
            }
            else
            {
                float baseAzimuth = BotUtils::getTargetAzimuth(pos, temp);
                rotation = BotUtils::getTargetRotation(myAzimuth, baseAzimuth);
                speed = (float)(M_PI / 2.0 - fabs(rotation));
            }
        }
    }
    else
        speed = 1.0f;
    if (currentState.Status == bz_eTankStatus::InAir && type->flagEffect == FlagEffect::Wings)
        pImpl->wantToJump = true;

    pImpl->navRot = rotation;
    pImpl->navSpeed = speed;
    pImpl->lastNavChange = TimeKeeper::getTick();
    return true;
}

bool bz_ServerSidePlayerHandler::isFlagUseful(const char* name)
{
    FlagType::Ptr type = FlagType::getDescFromAbbreviation(name);

    if (type == Flags::Null)
        return false;

    FlagSuccessMap::iterator it = pImpl->flagSuccess.find(type);
    float flagValue;
    if (it != pImpl->flagSuccess.end())
    {
        std::pair<int, int> &pr = it->second;
        if (pr.second == 0)
            flagValue = 0.0f;
        else
            flagValue = (float)pr.first / (float)pr.second;
    }
    else
        return true;

    float avg;
    if (pImpl->totalCnt == 0)
        avg = 0.0f;
    else
        avg = (float)pImpl->totalSum / (float)pImpl->totalCnt;
    return ((float)flagValue) >= avg;
}

bool bz_ServerSidePlayerHandler::lookForFlag(float &rotation, float &speed)
{
    GameKeeper::Player *player = GameKeeper::Player::getPlayerByIndex(playerID);
    if (player == nullptr)
        return false;

    FlagType::Ptr type = FlagInfo::get(player->player.getFlag())->flag.type;

    float pos[3];

    memcpy(pos,currentState.pos, sizeof(pos));
    if (pos[2] < 0.0f)
        pos[2] = 0.0f;

    int closestFlag = -1;

    if ((type != Flags::Null)  && (isFlagUseful(type->flagAbbv.c_str())))
        return false;

    float minDist = Infinity;
    int teamFlag = -1;
  
    for (int i = 0; i < numFlags; i++)
    {
        FlagInfo *flag = FlagInfo::get(i);
        if (flag == nullptr)
            continue;

        if (flag->flag.type == Flags::Null  || (flag->flag.status != FlagStatus::OnGround))
            continue;

        if (flag->flag.type->flagTeam != NoTeam)
            teamFlag = i;
        const float* fpos = flag->flag.position;
        if (fpos[2] == pos[2])
        {
            float dist = BotUtils::getTargetDistance(pos, fpos);
            bool isTargetObscured = BotUtils::isLocationObscured(pos, fpos);
            if (isTargetObscured)
                dist *= 1.25f;

            if ((dist < 200.0f) && (dist < minDist))
            {
                minDist = dist;
                closestFlag = i;
            }
        }
    }

    if (teamFlag != -1 && (minDist < 10.0f || closestFlag == -1))
        closestFlag = teamFlag; //FIXME: should a team flag be more significant than a closer flag?
    if (closestFlag != -1)
    {
        if (minDist < 10.0f)
        {
            if (type != Flags::Null)
            {
                dropFlag();
            }
        }

        FlagInfo *flag = FlagInfo::get(closestFlag);

        const float *fpos = flag->flag.position;
        float myAzimuth = currentState.rot;
        float flagAzimuth = BotUtils::getTargetAzimuth(pos, fpos);
        rotation = BotUtils::getTargetRotation(myAzimuth, flagAzimuth);
        speed = (float)(M_PI / 2.0 - fabs(rotation));
        return true;
    }

    return false;
}


GameKeeper::Player *findBestTarget(GameKeeper::Player* myTank, float* pos, float myAzimuth)
{
    GameKeeper::Player *target = nullptr;

    FlagType::Ptr type = FlagInfo::get(myTank->player.getFlag())->flag.type;
    TeamColor myTeam = myTank->player.getTeam();

    float distance = Infinity;

    for (int t = 0; t < maxPlayers; t++)
    {
        GameKeeper::Player *opponent = GameKeeper::Player::getPlayerByIndex(t);
        if (opponent == nullptr || opponent == myTank)
            continue;

        if ((opponent->player.isAlive())  && (!opponent->player.isPaused()) && (myTank->validTeamTarget(opponent)))
        {
            if (opponent->isPhantomZoned() && !myTank->isPhantomZoned() && (type->flagEffect != FlagEffect::ShockWave) && (type->flagEffect != FlagEffect::SuperBullet))
                continue;

            FlagType::Ptr thereType = FlagInfo::get(myTank->player.getFlag())->flag.type;
            TeamColor thereTeam = opponent->player.getTeam();

            if ((thereType->flagEffect == FlagEffect::Cloaking) &&
                (type->flagEffect == FlagEffect::Laser))
                continue;

            //perform a draft that has us chase the proposed opponent if they have our flag
            if (clOptions->gameType == ClassicCTF &&
                (((myTeam == RedTeam) && (thereType->flagTeam == RedTeam)) ||
                ((myTeam == GreenTeam) && (thereType->flagTeam == GreenTeam)) ||
                    ((myTeam == BlueTeam) && (thereType->flagTeam == BlueTeam)) ||
                    ((myTeam == PurpleTeam) && (thereType->flagTeam == PurpleTeam))))
            {
                target = opponent;
                break;
            }

            float d = BotUtils::getTargetDistance(pos, opponent->lastState.pos);
            bool isObscured = BotUtils::isLocationObscured(pos, opponent->lastState.pos);
            if (isObscured) //demote the priority of obscured enemies
                d *= 1.25f;

            if (d < distance)
            {
                if ((thereType->flagEffect != FlagEffect::Stealth)
                    || (type->flagEffect == FlagEffect::Seer)
                    || ((!isObscured) &&
                    (BotUtils::getTargetAngleDifference(pos, myAzimuth, opponent->lastState.pos) <= 30.0f)))
                {
                    target = opponent;
                    distance = d;
                }
            }
        }
    }

    return target;
}

bool bz_ServerSidePlayerHandler::chasePlayer(float &rotation, float &speed)
{
    GameKeeper::Player *player = GameKeeper::Player::getPlayerByIndex(playerID);
    if (player == nullptr)
        return false;

    FlagType::Ptr type = FlagInfo::get(player->player.getFlag())->flag.type;

    GameKeeper::Player *rPlayer = findBestTarget(player, currentState.pos, currentState.rot);
    if (rPlayer == nullptr)
        return false;

    pImpl->currentTarget = rPlayer->getIndex();

    const float *targetPos = rPlayer->lastState.pos;
    float distance = BotUtils::getTargetDistance(currentState.pos, targetPos);
    if (distance > 250.0f)
        return false;

    const float *tp = rPlayer->lastState.pos;
    float enemyPos[3];
    //toss in some lag adjustment/future prediction - 300 millis
    memcpy(enemyPos, tp, sizeof(enemyPos));
    const float *tv = rPlayer->lastState.velocity;
    enemyPos[0] += 0.3f * tv[0];
    enemyPos[1] += 0.3f * tv[1];
    enemyPos[2] += 0.3f * tv[2];
    if (enemyPos[2] < 0.0f) //Roger doesn't worry about burrow
        enemyPos[2] = 0.0;

    float myAzimuth = currentState.rot;
    float enemyAzimuth = BotUtils::getTargetAzimuth(currentState.pos, tp);
    rotation = BotUtils::getTargetRotation(myAzimuth, enemyAzimuth);

    //If we are driving relatively towards our target and a building pops up jump over it
    if (fabs(rotation) < BZDB.eval(StateDatabase::BZDB_LOCKONANGLE))
    {
        const Obstacle *building = NULL;
        float d = distance - 5.0f; //Make sure building is REALLY in front of player (-5)

        float dir[3] = { cosf(myAzimuth), sinf(myAzimuth), 0.0f };
        Ray tankRay(currentState.pos, dir);

        building = Shots::getFirstBuilding(tankRay, -0.5f, d);
        if (building && !player->isPhantomZoned() &&
            (type->flagEffect != FlagEffect::OscillationOverthruster))
        {
            //If roger can drive around it, just do that

            float leftDistance = BotUtils::getOpenDistance(currentState.pos, (float)(myAzimuth + (M_PI / 6.0)));
            if (leftDistance > (2.0f * d))
            {
                speed = 0.5f;
                rotation = -0.5f;
                return true;
            }
            float rightDistance = BotUtils::getOpenDistance(currentState.pos, (float)(myAzimuth - (M_PI / 6.0)));
            if (rightDistance > (2.0f * d))
            {
                speed = 0.5f;
                rotation = 0.5f;
                return true;
            }

            //Never did good in math, he should really see if he can reach the building
            //based on jumpvel and gravity, but settles for assuming 20-50 is a good range
            if ((d > 20.0f) && (d < 50.0f) &&
                (building->getType() == BoxBuilding::getClassName()))
            {
                float jumpVel = BZDB.eval(StateDatabase::BZDB_JUMPVELOCITY);
                float maxJump = (jumpVel * jumpVel) / (2 * -BZDB.eval(StateDatabase::BZDB_GRAVITY));

                if (((building->getPosition()[2] - currentState.pos[2] + building->getHeight())) < maxJump)
                {
                    speed = d / 50.0f;
                    pImpl->wantToJump = true;
                    return true;
                }
            }
        }
    }

    // weave towards the player
    const GameKeeper::Player *target = GameKeeper::Player::getPlayerByIndex(pImpl->currentTarget);
    if (target == nullptr)
        return false;

    FlagType::Ptr targetFlag = FlagInfo::get(target->player.getFlag())->flag.type;

    if ((distance > (BZDB.eval(StateDatabase::BZDB_SHOTSPEED) / 2.0f)) || (canShoot()))
    {
        float enemyUnitVec[2] = { cosf(enemyAzimuth), sinf(enemyAzimuth) };
        float myUnitVec[2] = { cosf(myAzimuth), sinf(myAzimuth) };
        float dotProd = (myUnitVec[0] * enemyUnitVec[0] + myUnitVec[1] * enemyUnitVec[1]);
        if (dotProd < 0.866f)
        {
            //if target is more than 30 degrees away, turn as fast as you can
            rotation *= (float)M_PI / (2.0f * fabs(rotation));
            speed = dotProd; //go forward inverse rel to how much you need to turn
        }
        else
        {
            int period = int(TimeKeeper::getTick().getSeconds());
            float absBias = (float)(M_PI / 20.0 * (distance / 100.0));
            float bias = ((period % 4) < 2) ? absBias : -absBias;
            rotation += bias;
            rotation = BotUtils::normalizeAngle(rotation);
            speed = 1.0;
        }
    }
    else if (targetFlag->flagEffect != FlagEffect::Burrow)
    {
        speed = -0.5f;
        rotation *= (float)(M_PI / (2.0 * fabs(rotation)));
    }

    return true;
}

bool bz_ServerSidePlayerHandler::stuckOnWall(float &rotation, float &speed)
{
    float stuckPeriod = float(TimeKeeper::getTick() - pImpl->lastStuckTime);
    if (stuckPeriod < 0.5f)
    {
        rotation = pImpl->stuckRot;
        speed = pImpl->stuckSpeed;
        return true;
    }
    else if (stuckPeriod < 1.0f)
    {
        rotation = pImpl->stuckRot;
        speed = 1.0;
        return true;
    }

    GameKeeper::Player *player = GameKeeper::Player::getPlayerByIndex(playerID);
    if (player == nullptr)
        return false;

    FlagType::Ptr type = FlagInfo::get(player->player.getFlag())->flag.type;

    const bool phased = (type->flagEffect == FlagEffect::OscillationOverthruster) || player->isPhantomZoned();

    if (!phased && (BotUtils::getOpenDistance(currentState.pos, currentState.rot) < 5.0f))
    {
        pImpl->lastStuckTime = TimeKeeper::getTick();
        if (bzfrand() > 0.8f)
        {
            // Every once in a while, do something nuts
            speed = (float)(bzfrand() * 1.5f - 0.5f);
            rotation = (float)(bzfrand() * 2.0f - 1.0f);
        }
        else
        {
            float leftDistance = BotUtils::getOpenDistance(currentState.pos, (float)(currentState.rot + (M_PI / 4.0)));
            float rightDistance = BotUtils::getOpenDistance(currentState.pos, (float)(currentState.rot - (M_PI / 4.0)));
            if (leftDistance > rightDistance)
                rotation = 1.0f;
            else
                rotation = -1.0f;
            speed = -0.5f;
        }
        pImpl->stuckRot = rotation;
        pImpl->stuckSpeed = speed;
        return true;
    }
    return false;
}

 void bz_ServerSidePlayerHandler::dropHardFlags()
{
     GameKeeper::Player *player = GameKeeper::Player::getPlayerByIndex(playerID);
     if (player == nullptr || !player->player.haveFlag())
         return;

    FlagType::Ptr type = FlagInfo::get(player->player.getFlag())->flag.type;
    if ((type->flagEffect == FlagEffect::Useless) || (type->flagEffect == FlagEffect::MachineGun) || (type->flagEffect == FlagEffect::Identify) || ((type->flagEffect == FlagEffect::PhantomZone)))
        dropFlag();
}

 Shot::Ptr findWorstBullet(float &minDistance, int playerID, float pos[3])
 {
     GameKeeper::Player *player = GameKeeper::Player::getPlayerByIndex(playerID);
     if (player == nullptr)
         return nullptr;

     FlagType::Ptr myFlag = FlagInfo::get(player->player.getFlag())->flag.type;

     Shot::Ptr minPath;

     minDistance = Infinity;
     for (auto shot : ShotManager.AllLiveShots())
     {
         GameKeeper::Player *opponent = GameKeeper::Player::getPlayerByIndex(shot->GetPlayerID());

         if (opponent == nullptr || opponent == player )
             continue;

        if ((shot->getFlag()->flagEffect == FlagEffect::InvisibleBullet) && (myFlag->flagEffect != FlagEffect::Seer))
            continue; //Theoretically Roger could triangulate the sound

        if (opponent->isPhantomZoned() && !player->isPhantomZoned())
            continue;

        if ((shot->getFlag()->flagEffect == FlagEffect::Laser) && myFlag->flagEffect == FlagEffect::Cloaking)
            continue; //cloaked tanks can't die from lasers

        auto shotPos = shot->LastUpdatePosition;
        if (fabs(shotPos.z - pos[2]) > BZDB.eval(StateDatabase::BZDB_TANKHEIGHT) && (shot->getFlag()->flagEffect != FlagEffect::GuidedMissile))
            continue;

        const float dist = BotUtils::getTargetDistance(pos, shotPos);
        if (dist < minDistance)
        {
            auto shotVel = shot->LastUpdateVector;
            float shotAngle = atan2f(shotVel[1], shotVel[0]);
            float shotUnitVec[2] = { cosf(shotAngle), sinf(shotAngle) };

            float trueVec[2] = { (pos[0] - shotPos[0]) / dist, (pos[1] - shotPos[1]) / dist };
            float dotProd = trueVec[0] * shotUnitVec[0] + trueVec[1] * shotUnitVec[1];

            if (dotProd <= 0.1f) //pretty wide angle, evasive actions prolly aren't gonna work
                continue;

            minDistance = dist;
            minPath = shot;
        }
     }
     return minPath;
 }

 bool bz_ServerSidePlayerHandler::avoidBullet(float &rotation, float &speed)
 {
     GameKeeper::Player *player = GameKeeper::Player::getPlayerByIndex(playerID);
     if (player == nullptr || !isAlive())
         return false;

     const float *pos = currentState.pos;

     auto flag = FlagInfo::get(player->player.getFlag())->flag.type;

     if (flag->flagEffect == FlagEffect::Narrow || flag->flagEffect == FlagEffect::Burrow)
         return false; // take our chances

     float minDistance = 9999999.0f;

     Shot::Ptr shot = findWorstBullet(minDistance, playerID, currentState.pos);

     if ((shot == nullptr) || (minDistance > 100.0f))
         return false;

     auto shotPos = shot->LastUpdatePosition;
     auto shotVel = shot->LastUpdateVector;

     float shotAngle = atan2f(shotVel[1], shotVel[0]);
     float shotUnitVec[2] = { cosf(shotAngle), sinf(shotAngle) };

     float trueVec[2] = { (pos[0] - shotPos[0]) / minDistance,(pos[1] - shotPos[1]) / minDistance };
     float dotProd = trueVec[0] * shotUnitVec[0] + trueVec[1] * shotUnitVec[1];

     float tankLength = BZDB.eval(StateDatabase::BZDB_TANKLENGTH);

     if (((canJump() || flag->flagEffect == FlagEffect::Jumping || flag->flagEffect == FlagEffect::Wings)) && (minDistance < (std::max(dotProd, 0.5f) * tankLength * 2.25f)) && (flag->flagEffect != FlagEffect::NoJumping))
     {
         pImpl->wantToJump = true;
         return (flag->flagEffect != FlagEffect::Wings);
     }
     else if (dotProd > 0.96f)
     {
         speed = 1.0;
         float myAzimuth = currentState.rot;
         float rotation1 = BotUtils::normalizeAngle((float)((shotAngle + M_PI / 2.0) - myAzimuth));

         float rotation2 = BotUtils::normalizeAngle((float)((shotAngle - M_PI / 2.0) - myAzimuth));

         float zCross = shotUnitVec[0] * trueVec[1] - shotUnitVec[1] * trueVec[0];

         if (zCross > 0.0f)   //if i am to the left of the shot from shooter pov
         {
             rotation = rotation1;
             if (fabs(rotation1) < fabs(rotation2))
                 speed = 1.0f;
             else if (dotProd > 0.98f)
                 speed = -0.5f;
             else
                 speed = 0.5f;
         }
         else
         {
             rotation = rotation2;
             if (fabs(rotation2) < fabs(rotation1))
                 speed = 1.0f;
             else if (dotProd > 0.98f)
                 speed = -0.5f;
             else
                 speed = 0.5f;
         }
         return true;
     }
     return false;
 }


std::vector<bz_ServerSidePlayerHandler*> serverSidePlayer;


bz_ePlayerDeathReason getDeathReason (bz_PlayerDieEventData_V1* data)
{
    if (data->driverID > 0)
        return ePhysicsDriverDeath;

    if (data->flagKilledWith == "SR")
        return eGotRunOver;

    if (data->flagKilledWith == "G")
        return eGenocideEffect;

    if (data->shotID > 0)
        return eGotShot;

    // TODO, do better here
    return eGotKilled;
}

class BotEventHandler : public bz_EventHandler
{
    virtual void process ( bz_EventData *eventData )
    {
        std::vector<bz_ServerSidePlayerHandler*>::iterator itr = serverSidePlayer.begin();
        while (itr != serverSidePlayer.end())
        {
            bz_ServerSidePlayerHandler* handler = *itr;

            switch (eventData->eventType)
            {
            default:
                break;

            case bz_ePlayerJoinEvent:
                handler->playerAdded(((bz_PlayerJoinPartEventData_V1*)eventData)->playerID);
                break;

            case bz_ePlayerPartEvent:
                handler->playerRemoved(((bz_PlayerJoinPartEventData_V1*)eventData)->playerID);
                break;

            case bz_ePlayerSpawnEvent:
            {
                bz_PlayerSpawnEventData_V1* spawnData = (bz_PlayerSpawnEventData_V1*)eventData;
                handler->playerSpawned(spawnData->playerID,spawnData->state.pos,spawnData->state.rotation);
            }
            break;

            case bz_eCaptureEvent:
            {
                bz_CTFCaptureEventData_V1* capData = (bz_CTFCaptureEventData_V1*)eventData;
                handler->flagCaptured(capData->playerCapping,capData->teamCapped);
            }
            break;

            case bz_eFilteredChatMessageEvent:
            {
                bz_ChatEventData_V1* chat = (bz_ChatEventData_V1*)eventData;
                handler->textMessage (chat->to, chat->from, chat->message.c_str());
            }
            break;

            case bz_ePlayerDieEvent:
            {
                bz_PlayerDieEventData_V1* die = (bz_PlayerDieEventData_V1*)eventData;
                handler->playerKilled(die->playerID, die->killerID, getDeathReason(die), die->shotID, die->flagKilledWith.c_str(), die->driverID);
            }
            break;

            case bz_eShotFiredEvent:
            {
                bz_ShotFiredEventData_V1* fired = (bz_ShotFiredEventData_V1*)eventData;
                handler->shotFired(fired->playerID,fired->shotID);
            }
            break;

            case bz_eShotEndedEvent:
            {
                bz_ShotEndedEventData_V2* ended = (bz_ShotEndedEventData_V2*)eventData;
                handler->shotEnded(ended->playerID,ended->shotID, ended->expired);
            }
            break;

            case bz_ePlayerUpdateEvent:
            {
                bz_PlayerUpdateEventData_V1* updated = (bz_PlayerUpdateEventData_V1*)eventData;
                handler->playerStateUpdate(updated->playerID, &updated->state, updated->stateTime);

                if (updated->lastState.status == eTeleporting &&  updated->lastState.status != eTeleporting)
                    handler->playerTeleported(updated->playerID,&updated->state,&updated->lastState);

                if (updated->playerID == handler->getPlayerID())
                {
                    // check for stuff on us
                }
            }
            break;
            }
            ++itr;
        }
    }
};

bz_EventHandler *botEventHandler = NULL;

//-------------------------------------------------------------------------

BZF_API int bz_addServerSidePlayer(bz_ServerSidePlayerHandler *handler)
{
    handler->setPlayerID(-1);

    PlayerId playerIndex = getNewPlayerID();
    if (playerIndex >= 0xFF)
        return -1;

    if (botEventHandler == NULL)
    {
        botEventHandler = new BotEventHandler();
        worldEventManager.addEvent(bz_ePlayerJoinEvent,botEventHandler);
        worldEventManager.addEvent(bz_ePlayerPartEvent,botEventHandler);
        worldEventManager.addEvent(bz_ePlayerSpawnEvent,botEventHandler);
        worldEventManager.addEvent(bz_eCaptureEvent,botEventHandler);
        worldEventManager.addEvent(bz_eFilteredChatMessageEvent,botEventHandler);
        worldEventManager.addEvent(bz_ePlayerDieEvent,botEventHandler);
        worldEventManager.addEvent(bz_eShotFiredEvent,botEventHandler);
        worldEventManager.addEvent(bz_eShotEndedEvent,botEventHandler);
        worldEventManager.addEvent(bz_ePlayerUpdateEvent,botEventHandler);
    }

    // make the player, check the game stuff, and don't do DNS stuff
    GameKeeper::Player *player = new GameKeeper::Player(playerIndex, handler);
    checkGameOn();
    player->_LSAState = GameKeeper::Player::notRequired;

    handler->setPlayerID(playerIndex);

    handler->added(playerIndex);

    serverSidePlayer.push_back(handler);
    return playerIndex;
}

//-------------------------------------------------------------------------

BZF_API bool bz_removeServerSidePlayer(int playerID, bz_ServerSidePlayerHandler *handler)
{
    if (playerID < 0)
        return false;

    std::vector<bz_ServerSidePlayerHandler*>::iterator itr = std::find(serverSidePlayer.begin(),serverSidePlayer.end(),
            handler);
    if (itr != serverSidePlayer.end())
        serverSidePlayer.erase(itr);

    PlayerId playerIndex=(PlayerId)playerID;
    GameKeeper::Player *player = GameKeeper::Player::getPlayerByIndex(playerIndex);

    if (player->playerHandler!=handler)
        return false;


    removePlayer(playerIndex, NULL, true);
    return true;
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
