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

/* interface header */
#include "RemotePlayer.h"

/* common implementation headers */
#include "World.h"

#include "ShotList.h"


RemotePlayer::RemotePlayer(const PlayerId& _id, TeamColor _team, int _skinIndex, const char* _name, const char* _motto,
                           const PlayerType _type) :
    Player(_id, _team, _skinIndex, _name, _motto, _type)
{
}

RemotePlayer::~RemotePlayer()
{
    ShotList::ClearPlayerShots(getId());

}

void            RemotePlayer::addShot(const FiringInfo& info)
{
    float newpos[3];
    const float *f = getForward();
    ShotPath::Ptr newShot = ShotPath::Create(info);
    addShotToSlot(newShot); // take up the shot slot

    ShotList::AddShot(newShot);

    // Update tanks position and set dead reckoning for better lag handling
    // shot origin is center of tank for shockwave
    if (info.flagType->flagEffect == FlagEffect::ShockWave)
    {
        newpos[0] = info.shot.pos[0];
        newpos[1] = info.shot.pos[1];
        newpos[2] = info.shot.pos[2];
    }
    // shot origin is muzzle for other shots
    else
    {
        float front = BZDB.eval(StateDatabase::BZDB_MUZZLEFRONT);
        if (info.flagType->flagEffect == FlagEffect::Obesity) front *= BZDB.eval(StateDatabase::BZDB_OBESEFACTOR);
        else if (info.flagType->flagEffect == FlagEffect::Tiny) front *= BZDB.eval(StateDatabase::BZDB_TINYFACTOR);
        else if (info.flagType->flagEffect == FlagEffect::Thief) front *= BZDB.eval(StateDatabase::BZDB_THIEFTINYFACTOR);
        newpos[0] = info.shot.pos[0] - (front * f[0]);
        newpos[1] = info.shot.pos[1] - (front * f[1]);
        newpos[2] = info.shot.pos[2] - BZDB.eval(StateDatabase::BZDB_MUZZLEHEIGHT);
    }
    shotStatistics.recordFire(info.flagType,f,info.shot.vel);
    // FIXME - with dynamic dimensions, this may not be a good idea
    //     (flag each shot with a 'default dimensions' state?)
    move(newpos, getAngle());
    // FIXME - timestamp of shots are handled slightly different than for
    //     tank position updates, better so ignore them for now
    //     detail: settick | shot: current() | move | draw | tankpos: tick()
    //setDeadReckoning(info.timeSent);
    setDeadReckoning(-1.0f);
}

ShotPath::Vec       RemotePlayer::getShots() const
{
    return ShotList::GetShotsForPlayer(getId());
}

void            RemotePlayer::purgeShots() const
{
    ShotList::ClearPlayerShots(getId());
}

bool            RemotePlayer::doEndShot( int ident, bool isHit, float* pos)
{
    return ShotList::HandleEndShot(ident, isHit, pos);
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
