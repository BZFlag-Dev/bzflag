/* bzflag
 * Copyright (c) 1993-2021 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

// interface header
#include "ShotPath.h"

/* system headers */
#include <assert.h>

/* local implementation headers */
#include "SegmentedShotStrategy.h"
#include "GuidedMissleStrategy.h"
#include "ShockWaveStrategy.h"


//
// FiringInfo (with BaseLocalPlayer)
//

FiringInfo::FiringInfo(const BaseLocalPlayer& tank, int _localID)
{
    shot.player = tank.getId();
    shot.id = 0xFFFF;    // local players always send in 0xFFFF for the global ID since it's assigned by the server
    localID = _localID;
    tank.getMuzzle(shot.pos);
    const float* dir = tank.getForward();
    const float* tankVel = tank.getVelocity();
    float shotSpeed = BZDB.eval(StateDatabase::BZDB_SHOTSPEED);
    shot.vel[0] = tankVel[0] + shotSpeed * dir[0];
    shot.vel[1] = tankVel[1] + shotSpeed * dir[1];
    shot.vel[2] = tankVel[2] + shotSpeed * dir[2];
    shot.dt = 0.0f;

    flagType = tank.getFlag();
    // wee bit o hack -- if phantom flag but not phantomized
    // the shot flag is normal -- otherwise FiringInfo will have
    // to be changed to add a real bitwise status variable
    if (tank.getFlag()->flagEffect == FlagEffect::PhantomZone && !tank.isFlagActive())
        flagType = Flags::Null;
    lifetime = BZDB.eval(StateDatabase::BZDB_RELOADTIME);
}

//
// ShotPath
//


ShotPath::Ptr ShotPath::Create(const FiringInfo& info)
{
    // eek!  a giant switch statement, how un-object-oriented!
    // each flag should be a flyweight object derived from a
    // base Flag class with a virtual makeShotStrategy() member.
    // just remember -- it's only a game.
    if (info.flagType->flagShot == ShotType::Normal)
        return std::make_shared<NormalShotStrategy>(info);
    else
    {
        if (info.flagType->flagEffect == FlagEffect::RapidFire)
            return std::make_shared<RapidFireStrategy>(info);
        else if (info.flagType->flagEffect == FlagEffect::MachineGun)
            return std::make_shared< MachineGunStrategy>(info);
        else if (info.flagType->flagEffect == FlagEffect::GuidedMissile)
            return std::make_shared<GuidedMissileStrategy>(info);
        else if (info.flagType->flagEffect == FlagEffect::Laser)
            return std::make_shared<LaserStrategy>(info);
        else if (info.flagType->flagEffect == FlagEffect::Ricochet)
            return std::make_shared<RicochetStrategy>(info);
        else if (info.flagType->flagEffect == FlagEffect::SuperBullet)
            return std::make_shared<SuperBulletStrategy>(info);
        else if (info.flagType->flagEffect == FlagEffect::ShockWave)
            return std::make_shared<ShockWaveStrategy>(info);
        else if (info.flagType->flagEffect == FlagEffect::Thief)
            return std::make_shared<ThiefStrategy>(info);
        else if (info.flagType->flagEffect == FlagEffect::PhantomZone)
            return std::make_shared<PhantomBulletStrategy>(info);
        else
            assert(0);    // shouldn't happen
    }
    return nullptr;
}

ShotPath::ShotPath(const FiringInfo& info) :
    firingInfo(info),
    startTime(TimeKeeper::getTick()),
    currentTime(TimeKeeper::getTick()),
    expiring(false),
    expired(false)
{

}

ShotPath::~ShotPath()
{
}

void            ShotPath::updateShot(float dt)
{
    getFiringInfo().shot.dt += dt;

    // get new time step and set current time
    currentTime += dt;

    // update shot
    if (!expired)
    {
        if (expiring) setExpired();
        else update(dt);
    }

    if (sendUpdates)
        sendUpdate(getFiringInfo());
}

void            ShotPath::setPosition(const float* p)
{
    firingInfo.shot.pos[0] = p[0];
    firingInfo.shot.pos[1] = p[1];
    firingInfo.shot.pos[2] = p[2];
}

void            ShotPath::setVelocity(const float* v)
{
    firingInfo.shot.vel[0] = v[0];
    firingInfo.shot.vel[1] = v[1];
    firingInfo.shot.vel[2] = v[2];
}

void            ShotPath::setExpiring()
{
    expiring = true;
}

void            ShotPath::setExpired()
{
    expiring = true;
    expired = true;
    expire();
}

void            ShotPath::update(float dt)
{
    // update shot
    updateShot(dt);
}

void            ShotPath::update(const ShotUpdate& shot,
                                 uint16_t code, const void* msg)
{
    // update shot info
    getFiringInfo().shot = shot;

    // let the strategy see the message
    readUpdate(code, msg);
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
