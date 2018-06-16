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

#include "ShotList.h"
#include "ShotStrategy.h"
#include <algorithm>

namespace ShotList
{
    static ShotPath::List GlobalShotList;

    void Clear()
    {
        GlobalShotList.clear();
    }

    void AddShot(ShotPath::Ptr shot)
    {
        GlobalShotList.push_front(shot);
    }

    void RemoveShot(int shotID)
    {
        auto itr = std::find_if(GlobalShotList.begin(), GlobalShotList.end(), [&](const ShotPath::Ptr& shot) { return shot->getFiringInfo().shot.id == shotID; });
        if (itr != GlobalShotList.end())
            GlobalShotList.remove(*itr);
    }

    const ShotPath::List& GetActiveShots()
    {
        return GlobalShotList;
    }

    ShotPath::Ptr GetShot(int shotID)
    {
        auto itr = std::find_if(GlobalShotList.begin(), GlobalShotList.end(), [&](const ShotPath::Ptr& shot) { return shot->getFiringInfo().shot.id == shotID; });
        if (itr == GlobalShotList.end())
            return nullptr;

        return *itr;
    }

    ShotPath::Vec GetShotsForPlayer(PlayerId player)
    {
        ShotPath::Vec playerShots;

        std::copy_if(GlobalShotList.begin(), GlobalShotList.end(), std::back_inserter(playerShots), [&](const ShotPath::Ptr& shot) { return shot->getPlayer() == player; });

        return playerShots;
    }

    void ClearPlayerShots(PlayerId player)
    {
        std::remove_if(GlobalShotList.begin(), GlobalShotList.end(), [&](const ShotPath::Ptr& shot) { return shot != nullptr ? shot->getPlayer() == player : false; });
    }

    void HandleShotUpdate(int shotID, const ShotUpdate& update, uint16_t code, const void* msg)
    {
        // special id used in some messages (and really shouldn't be sent here)
        if (shotID == -1)
            return;

        auto itr = std::find_if(GlobalShotList.begin(), GlobalShotList.end(), [&](const ShotPath::Ptr& shot) { return shot != nullptr ? shot->getFiringInfo().shot.id == shotID : false; });
        if (itr != GlobalShotList.end() && *itr == nullptr)      // ignore bogus shots (those with a bad index or for shots that don't exist)
            return ;

        ShotPath::Ptr shot = *itr;
        if (shot != nullptr)
            shot->update(update, code, msg);
    }

    bool HandleEndShot(int shotiD, bool isHit, float* pos)
    {
        // special id used in some messages (and really shouldn't be sent here)
        if (shotiD == -1)
            return false;

        auto itr = std::find_if(GlobalShotList.begin(), GlobalShotList.end(), [&](const ShotPath::Ptr& shot) { return shot->getFiringInfo().shot.id == shotiD; });
        if (itr == GlobalShotList.end() || *itr == nullptr)      // ignore bogus shots (those with a bad index or for shots that don't exist)
            return false;

        ShotPath::Ptr& shot = *itr;

        // ignore shots that already ending
        if (shot->isExpired() || shot->isExpiring())
            return false;

        // don't stop if it's because were hitting something and we don't stop
        // when we hit something.
        if (isHit && !shot->isStoppedByHit())
            return false;

        // end it
        const float* shotPos = shot->getPosition();
        pos[0] = shotPos[0];
        pos[1] = shotPos[1];
        pos[2] = shotPos[2];
        shot->setExpired();
        return true;
    }

    void UpdateShots(float dt)
    {
        std::vector<ShotPath::List::iterator> toKill;

        for (ShotPath::List::iterator shot  = GlobalShotList.begin(); shot != GlobalShotList.end(); shot++)
        {
            (*shot)->update(dt);

            if ((*shot)->isExpiring())
            {
                (*shot)->setExpired();
                toKill.push_back(shot);
            }    
        }

        for (auto deadShots : toKill)
            GlobalShotList.erase(deadShots);
    }


    void UpdateShotsForPlayer(PlayerId id, float dt)
    {
        std::vector<ShotPath::List::iterator> toKill;
        for (ShotPath::List::iterator shot = GlobalShotList.begin(); shot != GlobalShotList.end(); shot++)
        {
            if ((*shot)->getPlayer() != id)
                continue;

            (*shot)->update(dt);

            if ((*shot)->isExpiring())
            {
                (*shot)->setExpired();
                toKill.push_back(shot);
            }
        }

        for (auto deadShots : toKill)
            GlobalShotList.erase(deadShots);
    }
}
