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

// server side bot API

class bz_ServerSidePlayerHandler::Impl
{
public:
    TimeKeeper lastStuckTime;
    float stuckRot = 0.0f;
    float stuckSpeed = 0.0f;
};

bz_ServerSidePlayerHandler::bz_ServerSidePlayerHandler() : playerID(-1), wantToJump(false), autoSpawn(true), flaps(0),
    alive(false)
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
    float rot = lastUpdate.rot;

    doAutoPilot(rot, forward);

    setMovement(forward, rot);

    return false;
}

void bz_ServerSidePlayerHandler::died ( int UNUSED(killer) )
{
    alive = false;
}

void bz_ServerSidePlayerHandler::smote ( SmiteReason UNUSED(reason) )
{
    alive = false;
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
        alive = true;

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

void bz_ServerSidePlayerHandler::setPlayerData(const char *callsign, const char *token, const char *clientVersion,
        bz_eTeamType _team)
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

    alive = player->player.isAlive();
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
    return false;
}

//-------------------------------------------------------------------------

bool bz_ServerSidePlayerHandler::jump(void)
{
    if (canMove() && canJump())
        wantToJump = true;
    return wantToJump;
}

bool bz_ServerSidePlayerHandler::isAlive()
{
    return alive;
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

    return alive && player->canShoot();
}

//-------------------------------------------------------------------------

bool bz_ServerSidePlayerHandler::canMove(void)
{
    return alive && !falling();
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
    wantToJump = false;

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

    if (wantToJump)
        jump();

    fireAtTank();
}

bool bz_ServerSidePlayerHandler::fireAtTank()
{
    return false;
}

bool bz_ServerSidePlayerHandler::avoidDeathFall(float & UNUSED(rotation), float &speed)
{
    return false;
}

bool bz_ServerSidePlayerHandler::navigate(float &rotation, float &speed)
{
    return false;
}

bool bz_ServerSidePlayerHandler::lookForFlag(float &rotation, float &speed)
{
    return false;
}

bool bz_ServerSidePlayerHandler::chasePlayer(float &rotation, float &speed)
{
    return false;
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

    if (!phased && (BotUtils::getOpenDistance(lastUpdate.pos, lastUpdate.rot) < 5.0f))
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
            float leftDistance = BotUtils::getOpenDistance(lastUpdate.pos, (float)(lastUpdate.rot + (M_PI / 4.0)));
            float rightDistance = BotUtils::getOpenDistance(lastUpdate.pos, (float)(lastUpdate.rot - (M_PI / 4.0)));
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

     const float *pos = lastUpdate.pos;

     auto flag = FlagInfo::get(player->player.getFlag())->flag.type;

     if (flag->flagEffect == FlagEffect::Narrow || flag->flagEffect == FlagEffect::Burrow)
         return false; // take our chances

     float minDistance = 9999999.0f;

     Shot::Ptr shot = findWorstBullet(minDistance, playerID, lastUpdate.pos);

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
         wantToJump = true;
         return (flag->flagEffect != FlagEffect::Wings);
     }
     else if (dotProd > 0.96f)
     {
         speed = 1.0;
         float myAzimuth = lastUpdate.rot;
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
