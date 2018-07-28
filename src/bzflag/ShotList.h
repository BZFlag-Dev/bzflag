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

#pragma once

#include "common.h"

#include "ShotPath.h"


class ShotSlot
{
public:
    typedef std::vector<ShotSlot> Vec;

    int slot = 0;
    ShotPath::Ptr   activeShot = nullptr;
    FlagType        *lastFiredFlag = nullptr;
    float           totalReload = 0;
    float           reloadTime = 0;

    bool Available() const
    {
        return activeShot == nullptr && Reloaded();
    }

    bool Reloaded() const
    {
        return reloadTime <= 0;
    }

    float ReloadFactor() const
    {
        if (Available() || totalReload == 0.0f)
            return 1.0f;

        return 1.0f - (reloadTime / totalReload);
    }
};

namespace ShotList
{
void Clear();

void AddShot(ShotPath::Ptr shot);
void RemoveShot(int shotiD);

const ShotPath::List& GetActiveShots();
ShotPath::Ptr GetShot(int shotID);

ShotPath::Vec GetShotsForPlayer(PlayerId playerID);
void ClearPlayerShots(PlayerId playerID);

void UpdateShots(float dt);
void UpdateShotsForPlayer(PlayerId id, float dt);

bool HandleEndShot(int shotID, bool isHit, float* pos);

void HandleShotUpdate(int shotID, const ShotUpdate& shot, uint16_t code, const void* msg);
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
