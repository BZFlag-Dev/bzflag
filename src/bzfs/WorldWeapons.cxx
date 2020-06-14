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

// class-interface header
#include "WorldWeapons.h"

#include "WorldInfo.h"
// system headers
#include <vector>

// common-interface headers
#include "TimeKeeper.h"
#include "ShotUpdate.h"
#include "Protocol.h"
#include "Address.h"
#include "StateDatabase.h"
#include "bzfsAPI.h"

// bzfs specific headers
#include "bzfs.h"
#include "ShotManager.h"

int WorldWeapons::fireShot(FlagType::Ptr type, const float origin[3], const float vector[3], TeamColor teamColor,
                           PlayerId targetPlayerID)
{
    if (!BZDB.isTrue(StateDatabase::BZDB_WEAPONS))
        return INVALID_SHOT_GUID;

    auto buf = GetMessageBuffer();

    FiringInfo firingInfo;
    firingInfo.timeSent = (float)TimeKeeper::getCurrent().getSeconds();
    firingInfo.flagType = type;
    firingInfo.lifetime = BZDB.eval(StateDatabase::BZDB_RELOADTIME);
    firingInfo.shot.player = ServerPlayer;
    memmove(firingInfo.shot.pos, origin, 3 * sizeof(float));

    const float shotSpeed = BZDB.eval(StateDatabase::BZDB_SHOTSPEED);

    for (int i = 0; i < 3; i++)
        firingInfo.shot.vel[i] = vector[i] * shotSpeed;

    firingInfo.shot.dt = 0;
    firingInfo.shot.team = teamColor;

    bz_AllowServerShotFiredEventData_V1 allowEvent;
    allowEvent.flagType = type->flagAbbv;
    allowEvent.speed = shotSpeed;
    for (int i = 0; i < 3; i++)
    {
        allowEvent.pos[i] = origin[i];
        allowEvent.velocity[i] = firingInfo.shot.vel[i];
    }
    allowEvent.team = convertTeam(teamColor);

    worldEventManager.callEvents(bz_eAllowServerShotFiredEvent, &allowEvent);

    if (!allowEvent.allow)
        return INVALID_SHOT_GUID;

    if (allowEvent.changed)
    {
        FlagType::TypeMap &flagMap = FlagType::getFlagMap();

        if (flagMap.find(allowEvent.flagType) == flagMap.end())
            return INVALID_SHOT_GUID;

        FlagType::Ptr flag = flagMap.find(allowEvent.flagType)->second;

        if (flag == Flags::Null || flag == Flags::Unknown)
            return INVALID_SHOT_GUID;

        firingInfo.flagType = flag;
    }

    firingInfo.localID = 0xFF;
    firingInfo.shot.id = ShotManager.AddShot(firingInfo, ServerPlayer);
    buf->legacyPack(firingInfo.pack(buf->current_buffer()));

    broadcastPacket(MsgShotBegin, buf);
    // Target the gm, construct it, and send packet
    if (type->flagAbbv == "GM")
    {
        ShotManager.SetShotTarget(firingInfo.shot.id, targetPlayerID);

        buf = GetMessageBuffer();
        buf->legacyPack(firingInfo.shot.pack(buf->current_buffer()));
        buf->packUByte(targetPlayerID);

        broadcastPacket(MsgGMUpdate, buf);
    }

    bz_ServerShotFiredEventData_V1 event;
    event.guid = firingInfo.shot.id;
    event.flagType = allowEvent.flagType;
    event.speed = shotSpeed;
    for (int i = 0; i < 3; i++)
    {
        event.pos[i] = origin[i];
        event.velocity[i] = firingInfo.shot.vel[i];
    }
    event.team = convertTeam(teamColor);

    worldEventManager.callEvents(bz_eServerShotFiredEvent, &event);

    return  firingInfo.shot.id;
}

WorldWeapons::WorldWeapons()
    : worldShotId(0)
{
}


WorldWeapons::~WorldWeapons()
{
    clear();
}


void WorldWeapons::clear(void)
{
    weapons.clear();
}


float WorldWeapons::nextTime ()
{
    TimeKeeper nextShot = TimeKeeper::getSunExplodeTime();
    for (auto & w : weapons)
    {
        if (w->nextTime <= nextShot)
            nextShot = w->nextTime;
    }
    return (float)(nextShot - TimeKeeper::getCurrent());
}


void WorldWeapons::fire()
{
    TimeKeeper nowTime = TimeKeeper::getCurrent();

    for (auto & w : weapons)
    {
        if (w->nextTime <= nowTime)
        {
            FlagType::Ptr  type = (w->type);   // non-const copy

            float vec[3] = { 0,0,0 };
            bz_vectorFromRotations(w->tilt, w->direction, vec);
            fireShot(type, w->origin, vec, w->teamColor);

            //Set up timer for next shot, and eat any shots that have been missed
            while (w->nextTime <= nowTime)
            {
                w->nextTime += w->delay[w->nextDelay];
                w->nextDelay++;
                if (w->nextDelay == (int)w->delay.size())
                    w->nextDelay = 0;
            }
        }
    }
}


void WorldWeapons::add(FlagType::Ptr type, const float *origin,
                       float direction, float tilt, TeamColor teamColor,
                       float initdelay, const std::vector<float> &delay,
                       TimeKeeper &sync)
{
    Weapon::Ptr w = std::make_shared<Weapon>();
    w->type = type;
    w->teamColor = teamColor;
    memmove(&w->origin, origin, 3*sizeof(float));
    w->direction = direction;
    w->tilt = tilt;
    w->nextTime = sync;
    w->nextTime += initdelay;
    w->initDelay = initdelay;
    w->nextDelay = 0;
    w->delay = delay;

    weapons.push_back(w);
}


unsigned int WorldWeapons::count(void)
{
    return weapons.size();
}


void * WorldWeapons::pack(void *buf) const
{
    buf = nboPackUInt(buf, weapons.size());

    for (unsigned int i=0 ; i < weapons.size(); i++)
    {
        const Weapon::Ptr w = weapons[i];
        buf = w->type->pack (buf);
        buf = nboPackVector(buf, w->origin);
        buf = nboPackFloat(buf, w->direction);
        buf = nboPackFloat(buf, w->initDelay);
        buf = nboPackUShort(buf, (uint16_t)w->delay.size());
        for (unsigned int j = 0; j < w->delay.size(); j++)
            buf = nboPackFloat(buf, w->delay[j]);
    }
    return buf;
}


int WorldWeapons::packSize(void) const
{
    int fullSize = 0;

    fullSize += sizeof(uint32_t);

    for (unsigned int i=0 ; i < weapons.size(); i++)
    {
        const Weapon::Ptr w = weapons[i];
        fullSize += FlagType::packSize; // flag type
        fullSize += sizeof(float[3]); // pos
        fullSize += sizeof(float);    // direction
        fullSize += sizeof(float);    // init delay
        fullSize += sizeof(uint16_t); // delay count
        for (unsigned int j = 0; j < w->delay.size(); j++)
            fullSize += sizeof(float);
    }

    return fullSize;
}


int WorldWeapons::getNewWorldShotID(void)
{
    if (worldShotId > _MAX_WORLD_SHOTS)
        worldShotId = 0;
    return worldShotId++;
}

bool shotUsedInList(int shotID, Shots::Shot::Vec& list)
{
    for (size_t s = 0; s < list.size(); s++)
    {
        if (list[s]->GetLocalShotID() == shotID)
            return true;
    }
    return false;
}

//----------WorldWeaponGlobalEventHandler---------------------
// where we do the world weapon handling for event based shots since they are not really done by the "world"

WorldWeaponGlobalEventHandler::WorldWeaponGlobalEventHandler(FlagType::Ptr _type,
        const float *_origin,
        float _direction,
        float _tilt,
        TeamColor teamColor )
{
    type = _type;
    if (_origin)
        memcpy(origin,_origin,sizeof(float)*3);
    else
        origin[0] = origin[1] = origin[2] = 0.0f;

    direction = _direction;
    tilt = _tilt;
    team = convertTeam(teamColor);
}

WorldWeaponGlobalEventHandler::~WorldWeaponGlobalEventHandler()
{
}

void WorldWeaponGlobalEventHandler::process (bz_EventData *eventData)
{
    if (!eventData || eventData->eventType != bz_eCaptureEvent)
        return;

    bz_CTFCaptureEventData_V1 *capEvent = (bz_CTFCaptureEventData_V1*)eventData;

    if (capEvent->teamCapped != team)
        return;

    float vec[3] = { 0,0,0 };
    bz_vectorFromRotations(tilt, direction, vec);

    world->getWorldWeapons().fireShot(type, origin, vec);
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
