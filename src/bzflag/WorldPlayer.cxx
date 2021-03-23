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
#include "WorldPlayer.h"

#include "ShotList.h"

WorldPlayer::WorldPlayer() : Player(ServerPlayer, RogueTeam, 0, "world weapon", "", ComputerPlayer)
{
}

WorldPlayer::~WorldPlayer()
{
    ShotList::ClearPlayerShots(getId());

}

void WorldPlayer::addShot(const FiringInfo& info)
{
    ShotPath::Ptr newShot = ShotPath::Create(info);

    ShotList::AddShot(newShot);
}

bool WorldPlayer::doEndShot( int ident, bool isHit, float* pos)
{
    return ShotList::HandleEndShot(ident, isHit, pos);

}

void WorldPlayer::addShots(SceneDatabase* scene, bool colorblind) const
{
    for (auto shot : getShots())
    {
        if (shot && !shot->isExpiring() && !shot->isExpired())
            shot->addShot(scene, colorblind);
    }
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
