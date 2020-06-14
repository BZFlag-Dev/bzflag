/* bzflag
 * Copyright (c) 1993-2020 Tim Riker
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

#include <glm/gtc/type_ptr.hpp>

#include "bzfsAPI.h"
#include "bzfsAPIServerSidePlayers.h"
#include "bzfs.h"
#include "StateDatabase.h"
#include "RejoinList.h"
#include "RobotUtils.h"
#include "ObstacleMgr.h"
#include "BaseBuilding.h"
#include "BoxBuilding.h"
#include "PhysicsDriver.h"

// server side bot API

typedef std::map<FlagType::Ptr, std::pair<int, int> > FlagSuccessMap;

class bz_ServerSidePlayerHandler::Impl
{
public:
    // general data
    GameKeeper::Player* myTank = nullptr;

    // autopilot data
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


    // physics and update data (could be moved to playerInfo for generic server side player states)
    TimeKeeper lastUpdate = TimeKeeper::getCurrent();
    const Obstacle* lastObstacle;           // last obstacle touched

    float lastPosition[3];
    // bbox of last motion
    float bbox[2][3];

    const float MinSearchStep = 0.0001f;
    const int MaxSearchSteps = 7;
    const int MaxSteps = 4;
    const float TinyDistance = 0.001f;

    TimeKeeper timeOfLastBlowedUp;
    int currentPhysicsDriverID = -1;

    float lastSpeed = 0;

    int stuckFrameCount = 0;
    float     crossingPlane[4];
};

bz_ServerSidePlayerHandler::bz_ServerSidePlayerHandler() : playerID(-1), autoSpawn(true), flaps(0)
{
    pImpl = new Impl();

    pImpl->lastPosition[0] = 0.0f;
    pImpl->lastPosition[1] = 0.0f;
    pImpl->lastPosition[2] = 0.0f;
    pImpl->bbox[0][0] = pImpl->bbox[1][0] = 0.0f;
    pImpl->bbox[0][1] = pImpl->bbox[1][1] = 0.0f;
    pImpl->bbox[0][2] = pImpl->bbox[1][2] = 0.0f;

    pImpl->myTank = GameKeeper::Player::getPlayerByIndex(playerID);
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
    float forward = pImpl->myTank->desiredSpeed;
    float rot = pImpl->myTank->desiredAngVel;

    doAutoPilot(rot, forward);

    setMovement(forward, rot);

    return false;
}

void bz_ServerSidePlayerHandler::died ( int UNUSED(killer) )
{
    pImpl->timeOfLastBlowedUp = TimeKeeper::getCurrent();

    pImpl->myTank->currentState.Status = bz_eTankStatus::Dead;
    pImpl->myTank->currentState.setPStatus(PlayerState::DeadStatus);
    pImpl->currentTarget = -1;
}

void bz_ServerSidePlayerHandler::smote ( SmiteReason UNUSED(reason) )
{
    pImpl->timeOfLastBlowedUp = TimeKeeper::getCurrent();

    pImpl->myTank->currentState.Status = bz_eTankStatus::Dead;
    pImpl->myTank->currentState.setPStatus(PlayerState::DeadStatus);
    pImpl->currentTarget = -1;
}

void bz_ServerSidePlayerHandler::update ( void )
{
    think();

    float dt = (float)(TimeKeeper::getCurrent() - pImpl->lastUpdate);
    pImpl->lastUpdate = TimeKeeper::getCurrent();
    processUpdate(dt);
}

#define MAX_DT_LIMIT 0.1f
#define MIN_DT_LIMIT 0.001f

void bz_ServerSidePlayerHandler::processUpdate(float dt)
{
    // save last position
    const float* oldPosition = pImpl->myTank->currentState.pos;

    pImpl->lastPosition[0] = oldPosition[0];
    pImpl->lastPosition[1] = oldPosition[1];
    pImpl->lastPosition[2] = oldPosition[2];

    doUpdateMotion(dt);

    // compute motion's bounding box around center of tank
    const float* newVelocity = pImpl->myTank->currentState.vec;
    pImpl->bbox[0][0] = pImpl->bbox[1][0] = oldPosition[0];
    pImpl->bbox[0][1] = pImpl->bbox[1][1] = oldPosition[1];
    pImpl->bbox[0][2] = pImpl->bbox[1][2] = oldPosition[2];
    if (newVelocity[0] > 0.0f)
        pImpl->bbox[1][0] += dt * newVelocity[0];
    else
        pImpl->bbox[0][0] += dt * newVelocity[0];
    if (newVelocity[1] > 0.0f)
        pImpl->bbox[1][1] += dt * newVelocity[1];
    else
        pImpl->bbox[0][1] += dt * newVelocity[1];
    if (newVelocity[2] > 0.0f)
        pImpl->bbox[1][2] += dt * newVelocity[2];
    else
        pImpl->bbox[0][2] += dt * newVelocity[2];

    // expand bounding box to include entire tank
    float size = BZDB.eval(StateDatabase::BZDB_TANKRADIUS);
    if (pImpl->myTank->getFlagEffect() == FlagEffect::Obesity) size *= BZDB.eval(StateDatabase::BZDB_OBESEFACTOR);
    else if (pImpl->myTank->getFlagEffect() == FlagEffect::Tiny) size *= BZDB.eval(StateDatabase::BZDB_TINYFACTOR);
    else if (pImpl->myTank->getFlagEffect() == FlagEffect::Thief) size *= BZDB.eval(StateDatabase::BZDB_THIEFTINYFACTOR);
    pImpl->bbox[0][0] -= size;
    pImpl->bbox[1][0] += size;
    pImpl->bbox[0][1] -= size;
    pImpl->bbox[1][1] += size;
    pImpl->bbox[1][2] += BZDB.eval(StateDatabase::BZDB_TANKHEIGHT);

    // drop bad flag if timeout has expired
    if (dt > 0.0f && (clOptions->gameOptions & ShakableGameStyle && clOptions->shakeTimeout > 0)
            && pImpl->myTank->getFlagType() != Flags::Null && pImpl->myTank->getFlagType()->endurance == FlagEndurance::Sticky
            &&  pImpl->myTank->flagShakingTime > 0.0f)
    {
        pImpl->myTank->flagShakingTime -= dt;
        if (pImpl->myTank->flagShakingTime <= 0.0f)
        {
            pImpl->myTank->flagShakingTime = 0.0f;
            dropFlag();
        }
    }

    if (pImpl->myTank->isDeadReckoningWrong())
    {
        // send update

        pImpl->myTank->lastState.order++;

        pImpl->myTank->lastState.status = pImpl->myTank->currentState.pStatus;
        for (int i = 0; i < 3; i++)
        {
            pImpl->myTank->lastState.pos[i] = pImpl->myTank->currentState.pos[i];
            pImpl->myTank->lastState.velocity[i] = pImpl->myTank->currentState.vec[i];
        }

        pImpl->myTank->lastState.azimuth = pImpl->myTank->currentState.rot;
        pImpl->myTank->lastState.angVel = pImpl->myTank->currentState.angVel;
        pImpl->myTank->lastState.phydrv = pImpl->myTank->currentState.phydrv;

        pImpl->myTank->lastState.userSpeed = pImpl->myTank->currentState.speed;
        pImpl->myTank->lastState.userAngVel = pImpl->myTank->currentState.angVel;
        pImpl->myTank->lastState.jumpJetsScale = 0;

        pImpl->myTank->lastUpdate = pImpl->myTank->currentState;

        sendPlayerUpdate(pImpl->myTank);
        pImpl->myTank->lastUpdateSent = TimeKeeper::getCurrent();
    }
}


void  bz_ServerSidePlayerHandler::doMomentum(float dt, float& speed, float& angVel)
{
    // get maximum linear and angular accelerations
    float linearAcc = (pImpl->myTank->getFlagEffect() == FlagEffect::Momentum) ? BZDB.eval(
                          StateDatabase::BZDB_MOMENTUMLINACC) :
                      clOptions->linearAcceleration;
    float angularAcc = (pImpl->myTank->getFlagEffect() == FlagEffect::Momentum) ? BZDB.eval(
                           StateDatabase::BZDB_MOMENTUMANGACC) :
                       clOptions->angularAcceleration;

    // limit linear acceleration
    if (linearAcc > 0.0f)
    {
        const float acc = (speed - pImpl->lastSpeed) / dt;
        if (acc > 20.0f * linearAcc) speed = pImpl->lastSpeed + dt * 20.0f*linearAcc;
        else if (acc < -20.0f * linearAcc) speed = pImpl->lastSpeed - dt * 20.0f*linearAcc;
    }

    // limit angular acceleration
    if (angularAcc > 0.0f)
    {
        const float oldAngVel = pImpl->myTank->currentState.angVel;
        const float angAcc = (angVel - oldAngVel) / dt;
        if (angAcc > angularAcc) angVel = oldAngVel + dt * angularAcc;
        else if (angAcc < -angularAcc) angVel = oldAngVel - dt * angularAcc;
    }
}

void bz_ServerSidePlayerHandler::doFriction(float dt, const float *oldVelocity, float *newVelocity)
{
    const float friction = (pImpl->myTank->getFlagEffect() == FlagEffect::Momentum) ? BZDB.eval(
                               StateDatabase::BZDB_MOMENTUMFRICTION) :  BZDB.eval(StateDatabase::BZDB_FRICTION);

    if (friction > 0.0f)
    {
        // limit vector acceleration

        float delta[2] = { newVelocity[0] - oldVelocity[0], newVelocity[1] - oldVelocity[1] };
        float acc2 = (delta[0] * delta[0] + delta[1] * delta[1]) / (dt*dt);
        float accLimit = 20.0f * friction;

        if (acc2 > accLimit*accLimit)
        {
            float ratio = accLimit / sqrtf(acc2);
            newVelocity[0] = oldVelocity[0] + delta[0] * ratio;
            newVelocity[1] = oldVelocity[1] + delta[1] * ratio;
        }
    }
}

float getMaxSpeed(FlagType::Ptr flag)
{
    // BURROW and AGILITY will not be taken into account
    float maxSpeed = BZDB.eval(StateDatabase::BZDB_TANKSPEED);

    if (flag->flagEffect == FlagEffect::Velocity)
        maxSpeed *= BZDB.eval(StateDatabase::BZDB_VELOCITYAD);
    else if (flag->flagEffect == FlagEffect::Thief)
        maxSpeed *= BZDB.eval(StateDatabase::BZDB_THIEFVELAD);
    return maxSpeed;
}

void bz_ServerSidePlayerHandler::doSlideMotion(float dt, float slideTime, float newAngVel, float* newVelocity)
{
    const float oldAzimuth = pImpl->myTank->currentState.rot;
    const float* oldVelocity = pImpl->myTank->currentState.vec;

    const float angle = oldAzimuth + (0.5f * dt * newAngVel);
    const float cos_val = cosf(angle);
    const float sin_val = sinf(angle);
    const float scale = (dt / slideTime);
    const float speedAdj = pImpl->myTank->desiredSpeed * scale;
    const float* ov = oldVelocity;
    const float oldSpeed = sqrtf((ov[0] * ov[0]) + (ov[1] * ov[1]));
    float* nv = newVelocity;
    nv[0] = ov[0] + (cos_val * speedAdj);
    nv[1] = ov[1] + (sin_val * speedAdj);
    const float newSpeed = sqrtf((nv[0] * nv[0]) + (nv[1] * nv[1]));

    float maxSpeed = getMaxSpeed(pImpl->myTank->getFlagType());

    if (newSpeed > maxSpeed)
    {
        float adjSpeed;
        if (oldSpeed > maxSpeed)
        {
            adjSpeed = oldSpeed - (dt * (maxSpeed / slideTime));
            if (adjSpeed < 0.0f)
                adjSpeed = 0.0f;
        }
        else
            adjSpeed = maxSpeed;
        const float speedScale = adjSpeed / newSpeed;
        nv[0] *= speedScale;
        nv[1] *= speedScale;
    }
    return;
}

void bz_ServerSidePlayerHandler::setVelocity(float newVel[3])
{
    pImpl->myTank->currentState.vec[0] = newVel[0];
    pImpl->myTank->currentState.vec[1] = newVel[1];
    pImpl->myTank->currentState.vec[2] = newVel[2];
}

void bz_ServerSidePlayerHandler::doJump()
{
    // can't jump while burrowed
    if (pImpl->myTank->currentState.pos[2] < 0.0f)
        return;

    if (pImpl->myTank->getFlagEffect() == FlagEffect::Wings)
    {
        if (flaps <= 0)
            return;
        flaps--;
    }
    else if ((pImpl->myTank->currentState.Status != bz_eTankStatus::OnGround)
             && (pImpl->myTank->currentState.Status != bz_eTankStatus::OnBuilding))
    {
        // can't jump unless on the ground or a building
        if (pImpl->myTank->getFlagEffect() != FlagEffect::Wings)
            return;
        if (flaps <= 0)
            return;
        flaps--;
    }
    else if ((pImpl->myTank->getFlagEffect() != FlagEffect::Bouncy) &&
             ((pImpl->myTank->getFlagEffect() != FlagEffect::Jumping && !allowJumping())
              || (pImpl->myTank->getFlagEffect() == FlagEffect::NoJumping)))
        return;

    // jump velocity
    const float* oldVelocity = pImpl->myTank->currentState.vec;
    float newVelocity[3];
    newVelocity[0] = oldVelocity[0];
    newVelocity[1] = oldVelocity[1];
    if (pImpl->myTank->getFlagEffect() == FlagEffect::Wings)
    {
        newVelocity[2] = BZDB.eval(StateDatabase::BZDB_WINGSJUMPVELOCITY);
        // if you're falling, wings will just slow you down
        if (oldVelocity[2] < 0)
        {
            newVelocity[2] += oldVelocity[2];
            // if you're already going up faster, just keep doing that
        }
        else if (oldVelocity[2] > newVelocity[2])
            newVelocity[2] = oldVelocity[2];
    }
    else if (pImpl->myTank->getFlagEffect() == FlagEffect::Bouncy)
    {
        const float factor = 0.25f + ((float)bzfrand() * 0.75f);
        newVelocity[2] = factor * BZDB.eval(StateDatabase::BZDB_JUMPVELOCITY);
    }
    else
        newVelocity[2] = BZDB.eval(StateDatabase::BZDB_JUMPVELOCITY);
    setVelocity(newVelocity);
    pImpl->myTank->currentState.Status = bz_eTankStatus::InAir;

    pImpl->wantToJump = false;
}


void bz_ServerSidePlayerHandler::doUpdateMotion(float dt)
{
    // save old state
    const bz_eTankStatus oldStatus = pImpl->myTank->currentState.Status;
    const float* oldPosition = pImpl->myTank->currentState.pos;
    const float oldAzimuth = pImpl->myTank->currentState.rot;
    const float oldAngVel = pImpl->myTank->currentState.angVel;
    const float* oldVelocity = pImpl->myTank->currentState.vec;

    // prepare new state
    float newVelocity[3];
    newVelocity[0] = oldVelocity[0];
    newVelocity[1] = oldVelocity[1];
    newVelocity[2] = oldVelocity[2];
    float newAngVel = 0.0f;

    // phased means we can pass through buildings
    const bool phased = ((pImpl->myTank->currentState.Status == bz_eTankStatus::Dead)
                         || (pImpl->myTank->currentState.Status == bz_eTankStatus::Exploding)
                         || (pImpl->myTank->getFlagEffect() == FlagEffect::OscillationOverthruster) || pImpl->myTank->isPhantomZoned());

    float groundLimit = 0.0f;
    if (pImpl->myTank->getFlagEffect() == FlagEffect::Burrow)
        groundLimit = BZDB.eval(StateDatabase::BZDB_BURROWDEPTH);

    float gravity = BZDB.eval(StateDatabase::BZDB_GRAVITY);
    float tankHeight = BZDB.eval(StateDatabase::BZDB_TANKHEIGHT);
    float tankWidth = BZDB.eval(StateDatabase::BZDB_TANKWIDTH);
    float tankLength = BZDB.eval(StateDatabase::BZDB_TANKLENGTH);

    // get linear and angular speed at start of time step
    if (!NEAR_ZERO(dt, ZERO_TOLERANCE))
    {
        if (pImpl->myTank->currentState.Status == bz_eTankStatus::Dead || pImpl->myTank->player.isPaused())
        {
            // can't move if paused or dead -- set dt to zero instead of
            // clearing velocity and newAngVel for when we resume (if paused)
            dt = 0.0f;
            newAngVel = oldAngVel;
        }
        else if (pImpl->myTank->currentState.Status == bz_eTankStatus::Exploding)
        {
            // see if exploding time has expired
            if (pImpl->lastUpdate - pImpl->timeOfLastBlowedUp >= BZDB.eval(StateDatabase::BZDB_EXPLODETIME))
            {
                dt -= float((pImpl->lastUpdate - pImpl->timeOfLastBlowedUp) - BZDB.eval(StateDatabase::BZDB_EXPLODETIME));
                if (dt < 0.0f)
                    dt = 0.0f;
                pImpl->myTank->currentState.Status = bz_eTankStatus::Dead;
            }

            // can't control explosion motion
            newVelocity[2] += BZDB.eval(StateDatabase::BZDB_GRAVITY) * dt;
            newAngVel = 0.0f; // or oldAngVel to spin while exploding
        }
        else if ((pImpl->myTank->currentState.Status == bz_eTankStatus::OnGround)
                 || (pImpl->myTank->currentState.Status == bz_eTankStatus::OnBuilding)
                 || (pImpl->myTank->currentState.Status == bz_eTankStatus::InBuilding && oldPosition[2] == groundLimit))
        {
            // full control
            float speed = pImpl->myTank->desiredSpeed;

            // angular velocity
            newAngVel = pImpl->myTank->desiredAngVel;

            // limit acceleration
            doMomentum(dt, speed, newAngVel);

            // compute velocity so far
            const float angle = oldAzimuth + 0.5f * dt * newAngVel;
            newVelocity[0] = speed * cosf(angle);
            newVelocity[1] = speed * sinf(angle);
            newVelocity[2] = 0.0f;

            // now friction, if any
            doFriction(dt, oldVelocity, newVelocity);

            // reset our flap count if we have wings
            if (pImpl->myTank->getFlagEffect() == FlagEffect::Wings)
                flaps = (int)BZDB.eval(StateDatabase::BZDB_WINGSJUMPCOUNT);

            if ((oldPosition[2] < 0.0f) && (pImpl->myTank->getFlagEffect() == FlagEffect::Burrow))
                newVelocity[2] += 4 * gravity * dt;
            else if (oldPosition[2] > groundLimit)
                newVelocity[2] += gravity * dt;

            // save speed for next update
            pImpl->lastSpeed = speed;
        }
        else
        {
            // can't control motion in air unless have wings
            if (pImpl->myTank->getFlagEffect() == FlagEffect::Wings)
            {
                float speed = pImpl->myTank->desiredSpeed;

                // angular velocity
                newAngVel = pImpl->myTank->desiredAngVel;

                // compute horizontal velocity so far
                const float slideTime = BZDB.eval(StateDatabase::BZDB_WINGSSLIDETIME);
                if (slideTime > 0.0)
                    doSlideMotion(dt, slideTime, newAngVel, newVelocity);
                else
                {
                    const float angle = oldAzimuth + 0.5f * dt * newAngVel;
                    newVelocity[0] = speed * cosf(angle);
                    newVelocity[1] = speed * sinf(angle);
                }

                newVelocity[2] += BZDB.eval(StateDatabase::BZDB_WINGSGRAVITY) * dt;
                pImpl->lastSpeed = speed;
            }
            else
            {
                newVelocity[2] += gravity * dt;
                newAngVel = oldAngVel;
            }
        }

        // below the ground: however I got there, creep up
        if (oldPosition[2] < groundLimit)
            newVelocity[2] = std::max(newVelocity[2], -oldPosition[2] / 2.0f + 0.5f);
    }

    // jump here, we allow a little change in horizontal motion
    if (pImpl->wantToJump)
    {
        doJump();
        if (!pImpl->wantToJump)
        {
            newVelocity[2] = oldVelocity[2];
            if ((pImpl->lastObstacle != NULL) && !pImpl->lastObstacle->isFlatTop() && BZDB.isTrue(StateDatabase::BZDB_NOCLIMB))
            {
                newVelocity[0] = 0.0f;
                newVelocity[1] = 0.0f;
            }
        }
    }

    // do the physics driver stuff
    const int driverId = pImpl->currentPhysicsDriverID;
    const PhysicsDriver* phydrv = PHYDRVMGR.getDriver(driverId);
    if (phydrv != nullptr)
    {
        const float* v = phydrv->getLinearVel();

        newVelocity[2] += v[2];

        if (phydrv->getIsSlide())
        {
            const float slideTime = phydrv->getSlideTime();
            doSlideMotion(dt, slideTime, newAngVel, newVelocity);
        }
        else
        {
            // adjust the horizontal velocity
            newVelocity[0] += v[0];
            newVelocity[1] += v[1];

            const float av = phydrv->getAngularVel();
            const float* ap = phydrv->getAngularPos();

            if (av != 0.0f)
            {
                // the angular velocity is in radians/sec
                newAngVel += av;
                const float dx = oldPosition[0] - ap[0];
                const float dy = oldPosition[1] - ap[1];
                newVelocity[0] -= av * dy;
                newVelocity[1] += av * dx;
            }
        }
    }
    pImpl->lastObstacle = nullptr;

    // get new position so far (which is just the current position)
    float newPos[3];
    newPos[0] = oldPosition[0];
    newPos[1] = oldPosition[1];
    newPos[2] = oldPosition[2];
    float newAzimuth = oldAzimuth;

    // move tank through the time step.  if there's a collision then
    // move the tank up to the collision, adjust the velocity to
    // prevent interpenetration, and repeat.  avoid infinite loops
    // by only allowing a maximum number of repeats.
    bool expelled;
    const Obstacle* obstacle;
    float timeStep = dt;
    int stuck = false;
    if (pImpl->myTank->currentState.Status != bz_eTankStatus::Dead
            && pImpl->myTank->currentState.Status != bz_eTankStatus::Exploding)
    {
        pImpl->myTank->currentState.Status = bz_eTankStatus::OnGround;

        // anti-stuck code is useful only when alive
        // then only any 100 frames while stuck, take an action

        // try to see if we are stuck on a building
        obstacle = pImpl->myTank->getHitBuilding(newPos, newAzimuth, newPos, newAzimuth, phased, expelled);

        if (obstacle && expelled)
        {
            pImpl->stuckFrameCount++;
            stuck = true;
        }
        else
            pImpl->stuckFrameCount = 0;

        if (pImpl->stuckFrameCount > 100)
        {
            pImpl->stuckFrameCount = 0;
            // we are using a maximum value on time for frame to avoid lagging problem
            pImpl->myTank->setDesiredSpeed(0.25f);
            float delta = dt > 0.1f ? 0.1f : dt;
            float normalStuck[3];
            obstacle->getNormal(newPos, normalStuck);
            // use all the given speed to exit
            float movementMax = pImpl->myTank->desiredSpeed * delta;

            newVelocity[0] = movementMax * normalStuck[0];
            newVelocity[1] = movementMax * normalStuck[1];
            if (canJump()&& (pImpl->myTank->getFlagEffect() != FlagEffect::NoJumping))
                newVelocity[2] = movementMax * normalStuck[2];
            else
                newVelocity[2] = 0.0f;

            // exit will be in the normal direction
            newPos[0] += newVelocity[0];
            newPos[1] += newVelocity[1];
            newPos[2] += newVelocity[2];
            // compute time for all other kind of movements
            timeStep -= delta;
        }
    }

    float nominalPlanarSpeed2 = newVelocity[0] * newVelocity[0] + newVelocity[1] * newVelocity[1];

    // record position at beginning of time step
    float tmpPos[3], tmpAzimuth;
    tmpAzimuth = newAzimuth;
    tmpPos[0] = newPos[0];
    tmpPos[1] = newPos[1];
    tmpPos[2] = newPos[2];

    // get position at end of time step
    newAzimuth = tmpAzimuth + timeStep * newAngVel;
    newPos[0] = tmpPos[0] + timeStep * newVelocity[0];
    newPos[1] = tmpPos[1] + timeStep * newVelocity[1];
    newPos[2] = tmpPos[2] + timeStep * newVelocity[2];
    if ((newPos[2] < groundLimit) && (newVelocity[2] < 0))
    {
        // Hit lower limit, stop falling
        newPos[2] = groundLimit;
        if (pImpl->myTank->currentState.Status == bz_eTankStatus::Exploding)
        {
            // tank pieces reach the ground, friction
            // stop them, & mainly player view
            newPos[0] = tmpPos[0];
            newPos[1] = tmpPos[1];
        }
    }

    // see if we hit anything.  if not then we're done.
    obstacle = pImpl->myTank->getHitBuilding(tmpPos, tmpAzimuth, newPos, newAzimuth, phased, expelled);

    if (obstacle != nullptr)
    {
        float obstacleTop = obstacle->getPosition()[2] + obstacle->getHeight();
        if ((oldStatus != bz_eTankStatus::InAir) && obstacle->isFlatTop() && (obstacleTop != tmpPos[2])
                && (obstacleTop < (tmpPos[2] + BZDB.eval(StateDatabase::BZDB_MAXBUMPHEIGHT))))
        {
            newPos[0] = oldPosition[0];
            newPos[1] = oldPosition[1];
            newPos[2] = obstacleTop;

            // drive over bumps
            const Obstacle* bumpObstacle = pImpl->myTank->getHitBuilding(newPos, tmpAzimuth,
                                           newPos, newAzimuth,
                                           phased, expelled);
            if (bumpObstacle == NULL)
            {
                pImpl->myTank->move(newPos, pImpl->myTank->currentState.rot);
                newPos[0] += newVelocity[0] * (dt * 0.5f);
                newPos[1] += newVelocity[1] * (dt * 0.5f);
            }
        }

        // record position when hitting
        float hitPos[3], hitAzimuth;
        hitAzimuth = newAzimuth;
        hitPos[0] = newPos[0];
        hitPos[1] = newPos[1];
        hitPos[2] = newPos[2];

        // find the latest time before the collision
        float searchTime = 0.0f, searchStep = 0.5f * timeStep;
        for (int i = 0; searchStep > pImpl->MinSearchStep && i < pImpl->MaxSearchSteps; searchStep *= 0.5f, i++)
        {
            // get intermediate position
            const float t = searchTime + searchStep;
            newAzimuth = tmpAzimuth + (t * newAngVel);
            newPos[0] = tmpPos[0] + (t * newVelocity[0]);
            newPos[1] = tmpPos[1] + (t * newVelocity[1]);
            newPos[2] = tmpPos[2] + (t * newVelocity[2]);
            if ((newPos[2] < groundLimit) && (newVelocity[2] < 0))
                newPos[2] = groundLimit;

            // see if we hit anything
            bool searchExpelled;
            const Obstacle* searchObstacle = pImpl->myTank->getHitBuilding(tmpPos, tmpAzimuth, newPos, newAzimuth, phased,
                                             searchExpelled);

            if (!searchObstacle || !searchExpelled)
            {
                // if no hit then search latter half of time step
                searchTime = t;
            }
            else if (searchObstacle)
            {
                // if we hit a building then record which one and where
                obstacle = searchObstacle;

                expelled = searchExpelled;
                hitAzimuth = newAzimuth;
                hitPos[0] = newPos[0];
                hitPos[1] = newPos[1];
                hitPos[2] = newPos[2];
            }
        }

        // get position just before impact
        newAzimuth = tmpAzimuth + (searchTime * newAngVel);
        newPos[0] = tmpPos[0] + (searchTime * newVelocity[0]);
        newPos[1] = tmpPos[1] + (searchTime * newVelocity[1]);
        newPos[2] = tmpPos[2] + (searchTime * newVelocity[2]);
        if (oldPosition[2] < groundLimit)
            newVelocity[2] = std::max(newVelocity[2], -oldPosition[2] / 2.0f + 0.5f);


        // record how much time is left in time step
        timeStep -= searchTime;

        // get normal at intersection.  sometimes fancy test says there's
        // no intersection but we're expecting one so, in that case, fall
        // back to simple normal calculation.
        float normal[3];
        if (!pImpl->myTank->getHitNormal(obstacle, newPos, newAzimuth, hitPos, hitAzimuth, normal))
            obstacle->getNormal(newPos, normal);

        // check for being on a building
        if ((newPos[2] > 0.0f) && (normal[2] > 0.001f))
        {
            if (pImpl->myTank->currentState.Status != bz_eTankStatus::Dead
                    && pImpl->myTank->currentState.Status != bz_eTankStatus::Exploding && expelled)
            {
                pImpl->myTank->currentState.Status = bz_eTankStatus::OnBuilding;
                pImpl->lastObstacle = obstacle;
            }
            newVelocity[2] = 0.0f;
        }
        else
        {
            // get component of velocity in normal direction (in horizontal plane)
            float mag = (normal[0] * newVelocity[0]) +
                        (normal[1] * newVelocity[1]);

            // handle upward normal component to prevent an upward force
            if (!NEAR_ZERO(normal[2], ZERO_TOLERANCE))
            {
                // if going down then stop falling
                if (newVelocity[2] < 0.0f && newVelocity[2] -
                        (mag + normal[2] * newVelocity[2]) * normal[2] > 0.0f)
                    newVelocity[2] = 0.0f;

                // normalize force magnitude in horizontal plane
                float horNormal = normal[0] * normal[0] + normal[1] * normal[1];
                if (!NEAR_ZERO(horNormal, ZERO_TOLERANCE))
                    mag /= horNormal;
            }

            // cancel out component in normal direction (if velocity and
            // normal point in opposite directions).  also back off a tiny
            // amount to prevent a spurious collision against the same
            // obstacle.
            if (mag < 0.0f)
            {
                newVelocity[0] -= mag * normal[0];
                newVelocity[1] -= mag * normal[1];

                newPos[0] -= pImpl->TinyDistance * mag * normal[0];
                newPos[1] -= pImpl->TinyDistance * mag * normal[1];
            }
            if (mag > -0.01f)
            {
                // assume we're not allowed to turn anymore if there's no
                // significant velocity component to cancel out.
                newAngVel = 0.0f;
            }
        }


        // pick new location if we haven't already done so
        if (pImpl->myTank->currentState.Status == bz_eTankStatus::OnGround)
        {
            if (obstacle && (!expelled || stuck))
                pImpl->myTank->currentState.Status = bz_eTankStatus::InBuilding;
            else if (newPos[2] > 0.0f)
                pImpl->myTank->currentState.Status = bz_eTankStatus::InAir;
        }

        // see if we're crossing a wall
        if (pImpl->myTank->currentState.Status == bz_eTankStatus::InBuilding
                && pImpl->myTank->getFlagEffect() == FlagEffect::OscillationOverthruster)
        {
            if (obstacle->isCrossing(newPos, newAzimuth, 0.5f * tankLength, 0.5f * tankWidth, tankHeight, NULL))
                pImpl->myTank->currentState.setPStatus(pImpl->myTank->currentState.getPStatus() | int(PlayerState::CrossingWall));
            else
                pImpl->myTank->currentState.setPStatus(pImpl->myTank->currentState.getPStatus() & int(PlayerState::CrossingWall));
        }
        else if (world->crossingTeleporter(newPos, newAzimuth, 0.5f * tankLength, 0.5f * tankWidth, tankHeight,
                                           pImpl->crossingPlane))
            pImpl->myTank->currentState.setPStatus(pImpl->myTank->currentState.getPStatus() | int(PlayerState::CrossingWall));
        else
            pImpl->myTank->currentState.setPStatus(pImpl->myTank->currentState.getPStatus() & int(PlayerState::CrossingWall));

        // compute actual velocities.  do this before teleportation.
        if (!NEAR_ZERO(dt, ZERO_TOLERANCE))
        {
            const float oodt = 1.0f / dt;
            newAngVel = (newAzimuth - oldAzimuth) * oodt;
            newVelocity[0] = (newPos[0] - oldPosition[0]) * oodt;
            newVelocity[1] = (newPos[1] - oldPosition[1]) * oodt;
            newVelocity[2] = (newPos[2] - oldPosition[2]) * oodt;

            float newPlanarSpeed2 = newVelocity[0] * newVelocity[0]
                                    + newVelocity[1] * newVelocity[1];
            float scaling = newPlanarSpeed2 / nominalPlanarSpeed2;
            if (scaling > 1.0f)
            {
                scaling = sqrtf(scaling);
                newVelocity[0] /= scaling;
                newVelocity[1] /= scaling;
            }
        }
    }

    // see if we teleported
    int face;
    const Teleporter* teleporter;
    if (!isAlive())
        teleporter = NULL;
    else
        teleporter = world->crossesTeleporter(oldPosition, newPos, face);

    if (teleporter)
    {
        if (pImpl->myTank->getFlagEffect() == FlagEffect::PhantomZone)
        {
            // change zoned state
            pImpl->myTank->currentState.setPStatus(pImpl->myTank->currentState.getPStatus() & int(PlayerState::FlagActive));
        }
        else
        {
            // teleport
            const int source = world->getTeleporter(teleporter, face);
            int targetTele = world->getTeleportTarget(source);

            int outFace;
            const Teleporter* outPort = world->getTeleporter(targetTele, outFace);
            teleporter->getPointWRT(*outPort, face, outFace, newPos, newVelocity, newAzimuth, newPos, newVelocity, &newAzimuth);

            // check for a hit on the other side
            const Obstacle* teleObs = pImpl->myTank->getHitBuilding(newPos, newAzimuth, newPos, newAzimuth, phased, expelled);
            if (teleObs != NULL)
            {
                // revert
                memcpy(newPos, oldPosition, sizeof(float[3]));
                newVelocity[0] = newVelocity[1] = 0.0f;
                newVelocity[2] = oldVelocity[2];
                newAzimuth = oldAzimuth;
            }
            else
            {
                // save teleport info
                pImpl->myTank->setTeleport(pImpl->lastUpdate, source, targetTele);
                sendTeleport(playerID,source, targetTele);
            }
        }
    }

    // setup the physics driver
    pImpl->myTank->setPhysicsDriver(-1);
    if ((pImpl->lastObstacle != NULL) && (pImpl->lastObstacle->getType() == MeshFace::getClassName()))
    {
        const MeshFace* meshFace = (const MeshFace*)pImpl->lastObstacle;
        int driverIdent = meshFace->getPhysicsDriver();
        const PhysicsDriver* phydriver = PHYDRVMGR.getDriver(driverIdent);
        if (phydriver != NULL)
            pImpl->myTank->setPhysicsDriver(driverIdent);
    }

    //  const bool justLanded = (oldStatus == bz_eTankStatus::InAir) && ((pImpl->myTank->currentState.Status == bz_eTankStatus::OnGround) || (pImpl->myTank->currentState.Status == bz_eTankStatus::OnBuilding));

    // set falling status
    if (pImpl->myTank->currentState.Status == bz_eTankStatus::OnGround
            || pImpl->myTank->currentState.Status == bz_eTankStatus::OnBuilding
            ||  (pImpl->myTank->currentState.Status == bz_eTankStatus::InBuilding && newPos[2] == 0.0f))
        pImpl->myTank->currentState.setPStatus(pImpl->myTank->currentState.getPStatus() & ~int(PlayerState::Falling));
    else if (pImpl->myTank->currentState.Status == bz_eTankStatus::InAir
             || pImpl->myTank->currentState.Status == bz_eTankStatus::InBuilding)
        pImpl->myTank->currentState.setPStatus(pImpl->myTank->currentState.getPStatus() | int(PlayerState::Falling));

    // set UserInput status (determines how animated treads are drawn)
    const PhysicsDriver* phydrv2 = PHYDRVMGR.getDriver(pImpl->myTank->currentState.phydrv);
    if (((phydrv2 != NULL) && phydrv2->getIsSlide()) ||  ((pImpl->myTank->getFlagEffect() == FlagEffect::Wings)
            && (pImpl->myTank->currentState.Status == bz_eTankStatus::InAir)
            &&  (BZDB.eval(StateDatabase::BZDB_WINGSSLIDETIME) > 0.0f)))
        pImpl->myTank->currentState.setPStatus(pImpl->myTank->currentState.getPStatus() | int(PlayerState::UserInputs));
    else
        pImpl->myTank->currentState.setPStatus(pImpl->myTank->currentState.getPStatus() & ~int(PlayerState::UserInputs));

    // calculate the list of inside buildings
    pImpl->myTank->insideBuildings.clear();
    if (pImpl->myTank->currentState.Status == bz_eTankStatus::InBuilding)
        pImpl->myTank->collectInsideBuildings();

    // move tank
    pImpl->myTank->move(newPos, newAzimuth);
    setVelocity(newVelocity);
    pImpl->myTank->currentState.angVel = newAngVel;
    newAzimuth = pImpl->myTank->currentState.rot; // pickup the limited angle range from move()

    // see if I'm over my antidote
    if (pImpl->myTank->hasAntidoteFlag && pImpl->myTank->currentState.Status == bz_eTankStatus::OnGround)
    {
        float dist =  ((pImpl->myTank->flagAntidotePos[0] - newPos[0]) * (pImpl->myTank->flagAntidotePos[0] - newPos[0])) + ((
                          pImpl->myTank->flagAntidotePos[1] - newPos[1]) * (pImpl->myTank->flagAntidotePos[1] - newPos[1]));
        const float twoRads = pImpl->myTank->getRadius() + BZDB.eval(StateDatabase::BZDB_FLAGRADIUS);
        if (dist < (twoRads * twoRads))
            dropFlag();
    }

    if ((pImpl->myTank->getFlagEffect() == FlagEffect::Bouncy)
            && ((pImpl->myTank->currentState.Status == bz_eTankStatus::OnGround)
                || (pImpl->myTank->currentState.Status == bz_eTankStatus::OnBuilding)))
    {
        if (oldStatus != bz_eTankStatus::InAir)
        {
            if ((TimeKeeper::getTick() - pImpl->myTank->bounceTime) > 0)
                doJump();
        }
        else
        {
            pImpl->myTank->bounceTime = TimeKeeper::getTick();
            pImpl->myTank->bounceTime += 0.2f;
        }
    }
}


// lower level message API
void bz_ServerSidePlayerHandler::playerAdded(int) {}
void bz_ServerSidePlayerHandler::playerRemoved(int) {}
void bz_ServerSidePlayerHandler::playerSpawned(int UNUSED(id), const float* /* const float UNUSED(_pos[3]) */,
        float UNUSED(_rot)) {} // UNUSED() doesn't work with an array parameter


void bz_ServerSidePlayerHandler::checkForSpawn(int id, const float _pos[3], float _rot)
{
    if (id==playerID)
    {
        // it was me, I'm not in limbo
        if (pImpl->myTank->currentState.Status == bz_eTankStatus::Dead)
            pImpl->myTank->currentState.Status = bz_eTankStatus::InAir; // maybe? next update will take care of placing us.

        // update the current state
        pImpl->myTank->currentState.time = bz_getCurrentTime();
        // get where I am;
        memcpy(pImpl->myTank->currentState.pos, _pos, sizeof(float) *3);
        pImpl->myTank->currentState.angVel = 0;
        pImpl->myTank->currentState.vec[0] = 0;
        pImpl->myTank->currentState.vec[1] = 0;
        pImpl->myTank->currentState.vec[2] = 0;
        pImpl->myTank->currentState.rot = _rot;

        pImpl->myTank->lastUpdate.setPStatus(0);
        pImpl->myTank->currentState.setPStatus(PlayerState::Alive);

        pImpl->myTank->desiredSpeed = 0;
        pImpl->myTank->desiredAngVel = 0;

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
}

//-------------------------------------------------------------------------

void bz_ServerSidePlayerHandler::joinGame(bool spawn)
{
    GameKeeper::Player *player = GameKeeper::Player::getPlayerByIndex(playerID);
    if (!player)
        return ;

    if (player->player.isAlive() || player->player.isPlaying())
        return ;

    pImpl->myTank = player;

    player->lastState.order = 0;

    // set our state to signing on so we can join
    player->player.signingOn();
    pImpl->myTank->currentState.Status = bz_eTankStatus::Dead; // maybe? next update will take care of placing us.
    pImpl->myTank->currentState.setPStatus(PlayerState::DeadStatus);
    if (spawn)
    {
        playerAlive(playerID);
        player->player.setAlive();
        pImpl->myTank->currentState.setPStatus(PlayerState::Alive);
    }
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

void bz_ServerSidePlayerHandler::setMovement(float speed, float turn)
{
    GameKeeper::Player *player = GameKeeper::Player::getPlayerByIndex(playerID);
    if (!player)
        return;

    if (player->desiredAngVel == turn && player->desiredSpeed == speed)
        return ;

    player->desiredAngVel = turn;
    player->desiredSpeed = speed;

    if (player->desiredAngVel > 1.0f)
        player->desiredAngVel  = 1.0f;
    if (player->desiredSpeed > 1.0f)
        player->desiredSpeed = 1.0f;
    if (player->desiredAngVel < -1.0f)
        player->desiredAngVel = -1.0f;
    if (player->desiredSpeed < -1.0f)
        player->desiredSpeed = -1.0f;
}

extern RejoinList rejoinList;

void bz_ServerSidePlayerHandler::respawn()
{
    if (isAlive() || pImpl->myTank->player.getTeam() == ObserverTeam)
        return;

    if (pImpl->myTank->player.hasNeverSpawned())
    {
        pImpl->myTank->player.setAlive();
        playerAlive(playerID);

        return;
    }

    // player is on the waiting list
    float waitTime = rejoinList.waitTime(playerID);
    if (waitTime > 0.0f)
    {
        // Make them wait for trying to rejoin quickly
        pImpl->myTank->player.setSpawnDelay((double)waitTime);
        pImpl->myTank->player.queueSpawn();
        return;
    }

    // player moved before countdown started
    if (clOptions->timeLimit > 0.0f && !countdownActive)
        pImpl->myTank->player.setPlayedEarly();
    pImpl->myTank->player.queueSpawn();
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
    pImpl->wantToJump = true;
}

bool bz_ServerSidePlayerHandler::isAlive()
{
    return pImpl->myTank->currentState.Status != bz_eTankStatus::Dead;
}

//-------------------------------------------------------------------------

bool bz_ServerSidePlayerHandler::canJump(void)
{
    GameKeeper::Player *player = GameKeeper::Player::getPlayerByIndex(playerID);
    if (player == nullptr)
        return false;

    return canMove() && (allowJumping() || player->getFlagEffect() == FlagEffect::Jumping
                         || player->getFlagEffect() == FlagEffect::Wings);
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

    if ( pImpl->myTank->player.haveFlag())
        flag = pImpl->myTank->getFlagType();

    return computeMaxLinVelocity(flag,pImpl->myTank->currentState.pos[2]);
}

float bz_ServerSidePlayerHandler::getMaxRotSpeed ( void )
{
    FlagType::Ptr flag = nullptr;
    GameKeeper::Player *player = GameKeeper::Player::getPlayerByIndex(playerID);

    if (player && player->player.haveFlag())
        flag = player->getFlagType();

    return computeMaxAngleVelocity(flag, pImpl->myTank->currentState.pos[2]);
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

    fireAtTank();
}

bool bz_ServerSidePlayerHandler::fireAtTank()
{
    float pos[3];
    memcpy(pos, pImpl->myTank->currentState.pos, sizeof(pos));
    if (pos[2] < 0.0f)
        pos[2] = 0.01f;
    float myAzimuth = pImpl->myTank->currentState.rot;

    float dir[3] = { cosf(myAzimuth), sinf(myAzimuth), 0.0f };
    pos[2] += pImpl->myTank->getMuzzleHeight();
    Ray tankRay(pos, dir);
    pos[2] -= pImpl->myTank->getMuzzleHeight();

    float tankHeight = BZDB.eval(StateDatabase::BZDB_TANKHEIGHT);

    if (pImpl->myTank->getFlagEffect() == FlagEffect::ShockWave)
    {
        TimeKeeper now = TimeKeeper::getTick();
        if (canShoot())
        {
            bool hasSWTarget = false;
            for (int t = 0; t < maxPlayers; t++)
            {
                GameKeeper::Player *opponent = GameKeeper::Player::getPlayerByIndex(t);
                if (opponent == nullptr || opponent == pImpl->myTank)
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
                        if (!pImpl->myTank->validTeamTarget(opponent))
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
                pImpl->lastShot = now;
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
                if (opponent == nullptr || opponent == pImpl->myTank)
                    continue;

                if (opponent->player.isAlive() && !opponent->player.isPaused() && pImpl->myTank->validTeamTarget(opponent))
                {
                    if (opponent->isPhantomZoned() && !pImpl->myTank->isPhantomZoned()
                            && (pImpl->myTank->getFlagEffect() != FlagEffect::SuperBullet)
                            && (pImpl->myTank->getFlagEffect() != FlagEffect::ShockWave))
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

                    if ((pImpl->myTank->getFlagEffect() == FlagEffect::GuidedMissile) || (fabs(pos[2] - enemyPos[2]) < 2.0f * tankHeight))
                    {
                        float targetDiff = BotUtils::getTargetAngleDifference(pos, myAzimuth, enemyPos);
                        if ((targetDiff < errorLimit) || ((dist < (2.0f * BZDB.eval(StateDatabase::BZDB_SHOTSPEED)))
                                                          && (targetDiff < closeErrorLimit)))
                        {
                            bool isTargetObscured;
                            if (pImpl->myTank->getFlagEffect() != FlagEffect::SuperBullet)
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
    float tankHeight = BZDB.eval(StateDatabase::BZDB_TANKHEIGHT);

    float pos1[3], pos2[3];
    memcpy(pos1, pImpl->myTank->currentState.pos, sizeof(pos1));
    memcpy(pos2, pos1, sizeof(pos1));
    pos1[2] += 10.0f * tankHeight;
    float azimuth = pImpl->myTank->currentState.rot;
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

const float* GetBasePostion(TeamColor _team)
{
    auto baseList = OBSTACLEMGR.getBases();
    for (size_t i = 0; i < baseList.size(); i++)
    {
        const BaseBuilding* base = (const BaseBuilding*)baseList[i];
        if (base == nullptr || base->getTeam() != _team)
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

    FlagType::Ptr type = player->getFlagType();

    float pos[3];

    memcpy(pos, pImpl->myTank->currentState.pos, sizeof(pos));
    if (pos[2] < 0.0f)
        pos[2] = 0.01f;
    float myAzimuth = pImpl->myTank->currentState.rot;

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
            dropFlag();
        else
        {
            if ((((int) *(temp) + 2 >= (int) *(pos)) || (temp[0] == pos[0] && temp[1] == pos[1]))
                    && type->flagTeam == player->player.getTeam())
                dropFlag();
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
    if (pImpl->myTank->currentState.Status == bz_eTankStatus::InAir && type->flagEffect == FlagEffect::Wings)
        jump();

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

    FlagType::Ptr type = player->getFlagType();

    float pos[3];

    memcpy(pos, pImpl->myTank->currentState.pos, sizeof(pos));
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
                dropFlag();
        }

        FlagInfo *flag = FlagInfo::get(closestFlag);

        const float *fpos = flag->flag.position;
        float myAzimuth = pImpl->myTank->currentState.rot;
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

    FlagType::Ptr type = myTank->getFlagType();;
    TeamColor myTeam = myTank->player.getTeam();

    float distance = Infinity;

    for (int t = 0; t < maxPlayers; t++)
    {
        GameKeeper::Player *opponent = GameKeeper::Player::getPlayerByIndex(t);
        if (opponent == nullptr || opponent == myTank)
            continue;

        if ((opponent->player.isAlive())  && (!opponent->player.isPaused()) && (myTank->validTeamTarget(opponent)))
        {
            if (opponent->isPhantomZoned() && !myTank->isPhantomZoned() && (type->flagEffect != FlagEffect::ShockWave)
                    && (type->flagEffect != FlagEffect::SuperBullet))
                continue;

            FlagType::Ptr thereType = opponent->getFlagType();;

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

    FlagType::Ptr type = player->getFlagType();

    GameKeeper::Player *rPlayer = findBestTarget(player, pImpl->myTank->currentState.pos, pImpl->myTank->currentState.rot);
    if (rPlayer == nullptr)
        return false;

    pImpl->currentTarget = rPlayer->getIndex();

    const float *targetPos = rPlayer->lastState.pos;
    float distance = BotUtils::getTargetDistance(pImpl->myTank->currentState.pos, targetPos);
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

    float myAzimuth = pImpl->myTank->currentState.rot;
    float enemyAzimuth = BotUtils::getTargetAzimuth(pImpl->myTank->currentState.pos, tp);
    rotation = BotUtils::getTargetRotation(myAzimuth, enemyAzimuth);

    //If we are driving relatively towards our target and a building pops up jump over it
    if (fabs(rotation) < BZDB.eval(StateDatabase::BZDB_LOCKONANGLE))
    {
        const Obstacle *building = NULL;
        float d = distance - 5.0f; //Make sure building is REALLY in front of player (-5)

        float dir[3] = { cosf(myAzimuth), sinf(myAzimuth), 0.0f };
        Ray tankRay(pImpl->myTank->currentState.pos, dir);

        building = world->getFirstBuilding(tankRay, -0.5f, d);
        if (building && !player->isPhantomZoned() &&
                (type->flagEffect != FlagEffect::OscillationOverthruster))
        {
            //If roger can drive around it, just do that

            float leftDistance = BotUtils::getOpenDistance(pImpl->myTank->currentState.pos, (float)(myAzimuth + (M_PI / 6.0)));
            if (leftDistance > (2.0f * d))
            {
                speed = 0.5f;
                rotation = -0.5f;
                return true;
            }
            float rightDistance = BotUtils::getOpenDistance(pImpl->myTank->currentState.pos, (float)(myAzimuth - (M_PI / 6.0)));
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

                if (((building->getPosition()[2] - pImpl->myTank->currentState.pos[2] + building->getHeight())) < maxJump)
                {
                    speed = d / 50.0f;
                    jump();
                    return true;
                }
            }
        }
    }

    // weave towards the player
    const GameKeeper::Player *target = GameKeeper::Player::getPlayerByIndex(pImpl->currentTarget);
    if (target == nullptr)
        return false;

    FlagType::Ptr targetFlag = target->getFlagType();

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

    FlagType::Ptr type = player->getFlagType();

    const bool phased = (type->flagEffect == FlagEffect::OscillationOverthruster) || player->isPhantomZoned();

    if (!phased && (BotUtils::getOpenDistance(pImpl->myTank->currentState.pos, pImpl->myTank->currentState.rot) < 5.0f))
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
            float leftDistance = BotUtils::getOpenDistance(pImpl->myTank->currentState.pos,
                                 (float)(pImpl->myTank->currentState.rot + (M_PI / 4.0)));
            float rightDistance = BotUtils::getOpenDistance(pImpl->myTank->currentState.pos,
                                  (float)(pImpl->myTank->currentState.rot - (M_PI / 4.0)));
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

    FlagType::Ptr type = player->getFlagType();
    if ((type->flagEffect == FlagEffect::Useless) || (type->flagEffect == FlagEffect::MachineGun)
            || (type->flagEffect == FlagEffect::Identify) || ((type->flagEffect == FlagEffect::PhantomZone)))
        dropFlag();
}

Shot::Ptr findWorstBullet(float &minDistance, int playerID, float pos[3])
{
    GameKeeper::Player *player = GameKeeper::Player::getPlayerByIndex(playerID);
    if (player == nullptr)
        return nullptr;

    FlagType::Ptr myFlag = player->getFlagType();

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
        if (fabs(shotPos.z - pos[2]) > BZDB.eval(StateDatabase::BZDB_TANKHEIGHT)
                && (shot->getFlag()->flagEffect != FlagEffect::GuidedMissile))
            continue;

        const float dist = BotUtils::getTargetDistance(pos, glm::value_ptr(shotPos));
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
    const float *pos = pImpl->myTank->currentState.pos;

    auto flag = pImpl->myTank->getFlagType();

    if (flag->flagEffect == FlagEffect::Narrow || flag->flagEffect == FlagEffect::Burrow)
        return false; // take our chances

    float minDistance = 9999999.0f;

    Shot::Ptr shot = findWorstBullet(minDistance, playerID, pImpl->myTank->currentState.pos);

    if ((shot == nullptr) || (minDistance > 100.0f))
        return false;

    auto shotPos = shot->LastUpdatePosition;
    auto shotVel = shot->LastUpdateVector;

    float shotAngle = atan2f(shotVel[1], shotVel[0]);
    float shotUnitVec[2] = { cosf(shotAngle), sinf(shotAngle) };

    float trueVec[2] = { (pos[0] - shotPos[0]) / minDistance,(pos[1] - shotPos[1]) / minDistance };
    float dotProd = trueVec[0] * shotUnitVec[0] + trueVec[1] * shotUnitVec[1];

    float tankLength = BZDB.eval(StateDatabase::BZDB_TANKLENGTH);

    if (((canJump() || flag->flagEffect == FlagEffect::Jumping || flag->flagEffect == FlagEffect::Wings))
            && (minDistance < (std::max(dotProd, 0.5f) * tankLength * 2.25f)) && (flag->flagEffect != FlagEffect::NoJumping))
    {
        jump();
        return (flag->flagEffect != FlagEffect::Wings);
    }
    else if (dotProd > 0.96f)
    {
        speed = 1.0;
        float myAzimuth = pImpl->myTank->currentState.rot;
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
                handler->checkForSpawn(spawnData->playerID, spawnData->state.pos, spawnData->state.rotation);
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
                handler->playerKilled(die->playerID, die->killerID, getDeathReason(die), die->shotID, die->flagKilledWith.c_str(),
                                      die->driverID);
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
    player->_JoinState = GameKeeper::Player::readyToAdd;

    handler->setPlayerID(playerIndex);
    serverSidePlayer.push_back(handler);

    handler->added(playerIndex);
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
