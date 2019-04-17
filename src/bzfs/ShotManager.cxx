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

// interface header
#include "ShotManager.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/geometric.hpp>
#include <glm/trigonometric.hpp>
#include <glm/gtx/rotate_vector.hpp>

#include "TimeKeeper.h"
#include "BZDBCache.h"
#include "bzfs.h"
#include "Obstacle.h"
#include "ObstacleList.h"
#include "WallObstacle.h"
#include "ObstacleMgr.h"
#include "CollisionManager.h"
#include "Teleporter.h"


namespace Shots
{
void PackShotUpdate(ShotUpdate &info, MessageBuffer::Ptr msg)
{
    msg->packUByte(info.player);
    msg->packUShort(info.id);
    msg->packVector(info.pos);
    msg->packVector(info.vel);
    msg->packFloat(info.dt);
    msg->packShort(info.team);
}

void PackFlagType(FlagType::Ptr info, MessageBuffer::Ptr msg)
{
    msg->packUByte((info->flagAbbv.size() > 0) ? info->flagAbbv[0] : 0);
    msg->packUByte((info->flagAbbv.size() > 1) ? info->flagAbbv[1] : 0);
}

void PackFirningInfo(FiringInfo &info, MessageBuffer::Ptr msg)
{
    msg->packFloat(info.timeSent);
    msg->packUShort((uint16_t)info.localID);
    PackShotUpdate(info.shot,msg);
    PackFlagType(info.flagType, msg);
    msg->packFloat(info.lifetime);
}

void PackCustomFlagType(FlagType::Ptr info, MessageBuffer::Ptr msg)
{
    PackFlagType(info, msg);
    msg->packUByte(uint8_t(info->flagQuality));
    msg->packUByte(uint8_t(info->flagShot));
    msg->packUByte(uint8_t(info->flagEffect));
    msg->packStdString(info->flagName);
    msg->packStdString(info->flagHelp);
}

void PackFlag(FlagInfo& flag, bool hide, MessageBuffer::Ptr msg)
{
    if (flag.flag.type->flagTeam != ::NoTeam)
        hide = false;
    if (flag.player != -1)
        hide = false;
    msg->packUShort(flag.getIndex());

    if (hide)
        PackFlagType(Flags::Unknown, msg);
    else
        PackFlagType(flag.flag.type, msg);

    msg->packUShort(uint16_t(flag.flag.status));
    msg->packUShort(uint16_t(flag.flag.endurance));
    msg->packUByte(flag.flag.owner);
    msg->packVector(flag.flag.position);
    msg->packVector(flag.flag.launchPosition);
    msg->packVector(flag.flag.landingPosition);
    msg->packFloat(flag.flag.flightTime);
    msg->packFloat(flag.flag.flightEnd);
    msg->packFloat(flag.flag.initialVelocity);
}

//----------------Manager

double Manager::DeadShotCacheTime = 10.0;

Manager::Manager()
{
    Factories[FlagEffect::Normal] = std::make_shared<ProjectileShot::Factory>();
    LastGUID = INVALID_SHOT_GUID;
}

Manager::~Manager()
{
    Factories.clear();

    LiveShots.clear();

    RecentlyDeadShots.clear();
}

void Manager::Init()
{
    Factories[FlagEffect::GuidedMissile] = std::make_shared<GuidedMissileShot::Factory>();
    Factories[FlagEffect::SuperBullet] = std::make_shared< SuperBulletShot::Factory>();
    Factories[FlagEffect::ShockWave] = std::make_shared<ShockwaveShot::Factory>();
    Factories[FlagEffect::Ricochet] = std::make_shared<RicoShot::Factory>();
    Factories[FlagEffect::RapidFire] = std::make_shared<RapidFireShot::Factory>();
    Factories[FlagEffect::MachineGun] = std::make_shared<MachineGunShot::Factory>();
    Factories[FlagEffect::Laser] = std::make_shared<LaserShot::Factory>();
    Factories[FlagEffect::Thief] = std::make_shared<ThiefShot::Factory>();
    Factories[FlagEffect::PhantomZone] = std::make_shared<PhantomShot::Factory>();
}

void Manager::SetShotFactory(FlagEffect effect, std::shared_ptr<ShotFactory> factory)
{
    Factories[effect] = factory;
}

uint16_t Manager::AddShot(const FiringInfo &info, PlayerId UNUSED(shooter))
{
    ShotFactory::Ptr factory = nullptr;
    if (Factories.find(info.flagType->flagEffect) != Factories.end())
        factory = Factories[info.flagType->flagEffect];

    if (factory == nullptr)
        factory = Factories[FlagEffect::Normal];

    Shot::Ptr shot = factory->GetShot(NewGUID(), info);

    shot->LastUpdateTime = shot->StartTime = Now();
    shot->Setup();
    shot->Update(0); // to get the initial position
    shot->StartPosition = shot->LastUpdatePosition;
    shot->LastUpdateVector = glm::vec3(shot->Info.shot.vel[0], shot->Info.shot.vel[1], shot->Info.shot.vel[2]);

    LiveShots.push_back(shot);

    if (ShotCreated)
        (*ShotCreated)(*shot);
    return shot->GetGUID();
}

void Manager::RemoveShot(uint16_t shotID)
{
    Shot::Vec::iterator itr = LiveShots.begin();
    while (itr != LiveShots.end())
    {
        if ((*itr)->GetGUID() == shotID)
        {
            (*itr)->End();
            RecentlyDeadShots.push_back((*itr));
            itr = LiveShots.erase(itr);
            if (ShotEnded)
                (*ShotEnded)(*(*itr));
            return;
        }
        else
            itr++;
    }
}

void Manager::RemovePlayer(PlayerId player)
{
    Shot::Vec::iterator itr = LiveShots.begin();
    while (itr != LiveShots.end())
    {
        if ((*itr)->GetPlayerID() == player)
        {
            (*itr)->End();
            itr = LiveShots.erase(itr);
        }
        else
            itr++;
    }
}

void Manager::UpdateShot(uint16_t shotID, ShotUpdate& update)
{
    Shot::Ptr shot = FindByID(shotID);
    if (shot != nullptr)
        shot->ProcessUpdate(update);
}

void Manager::SetShotTarget(uint16_t shotID, PlayerId target)
{
    Shot::Ptr shot = FindByID(shotID);
    if (shot != nullptr)
        shot->Retarget(target);
}

uint16_t Manager::FindShotGUID(PlayerId shooter, uint16_t localShotID)
{
    for (Shot::Vec::iterator itr = LiveShots.begin(); itr != LiveShots.end(); itr++)
    {
        if ((*itr)->GetPlayerID() == shooter && (*itr)->Info.shot.id == localShotID)
            return (*itr)->GetGUID();
    }

    for (Shot::Vec::iterator itr = RecentlyDeadShots.begin(); itr != RecentlyDeadShots.end(); itr++)
    {
        if ((*itr)->GetPlayerID() == shooter && (*itr)->Info.shot.id == localShotID)
            return (*itr)->GetGUID();
    }

    return 0;
}

bool Manager::IsValidShotID(uint16_t shotID)
{
    return FindByID(shotID) != nullptr;
}

uint16_t Manager::NewGUID()
{
    LastGUID++;

    if (LastGUID == MAX_SHOT_GUID) // handle the rollover
        LastGUID = 1 ;

    return LastGUID;
}

Shot::Ptr Manager::FindByID(uint16_t shotID)
{
    for (Shot::Vec::iterator itr = LiveShots.begin(); itr != LiveShots.end(); itr++)
    {
        if ((*itr)->GetGUID() == shotID)
            return *itr;
    }

    for (Shot::Vec::iterator itr = RecentlyDeadShots.begin(); itr != RecentlyDeadShots.end(); itr++)
    {
        if ((*itr)->GetGUID() == shotID)
            return *itr;
    }

    return nullptr;
}

double Manager::Now()
{
    return TimeKeeper::getCurrent().getSeconds();
}

std::vector<int> Manager::AllLiveShotIDs()
{
    std::vector<int> ids(LiveShots.size());
    for (auto shot : LiveShots)
        ids.push_back(shot->GetGUID());

    return ids;
}

std::vector<int>  Manager::ShotIDsInRadius(float pos[3], float radius)
{
    glm::vec3 p(pos[0],pos[1],pos[2]);
    float radSq = radius * radius;

    std::vector<int> ids(LiveShots.size());
    for (auto shot : LiveShots)
    {
        if (glm::dot(p - shot->LastUpdatePosition, p - shot->LastUpdatePosition) <= radSq)
            ids.push_back(shot->GetGUID());
    }

    return ids;
}

void Manager::Update()
{
    double now = Now();

    Shot::Vec::iterator itr = LiveShots.begin();
    while (itr != LiveShots.end())
    {
        float dt = (float)((*itr)->LastUpdateTime - now);
        (*itr)->LastUpdateTime = now;
        if ((*itr)->Update(dt))
        {
            Shot::Ptr shot = *itr;
            bz_ShotEndedEventData_V2 shotEvent;
            shotEvent.playerID = shot->Info.shot.player;
            shotEvent.shotID = shot->GetGUID();
            shotEvent.explode = true;
            shotEvent.expired = true;

            for(int i = 0; i < 3; i++)
                shotEvent.position[i] = shot->LastUpdatePosition[i];

            worldEventManager.callEvents(bz_eShotEndedEvent, &shotEvent);

            itr = LiveShots.erase(itr);
            shot->End();
            RecentlyDeadShots.push_back(shot);
        }
        else
            itr++;
    }

    itr = RecentlyDeadShots.begin();

    while (itr != RecentlyDeadShots.end())
    {
        if (now - (*itr)->GetLastUpdateTime() >= Manager::DeadShotCacheTime)
            itr = RecentlyDeadShots.erase(itr);
        else
            itr++;
    }
}

Shot::Vec Manager::LiveShotsForPlayer(PlayerId player)
{
    Shot::Vec list;

    for (Shot::Vec::iterator itr = LiveShots.begin(); itr != LiveShots.end(); itr++)
    {
        if ((*itr)->GetPlayerID() == player)
            list.push_back(*itr);
    }

    return list;
}

Shot::Vec Manager::DeadShotsForPlayer(PlayerId player)
{
    Shot::Vec list;

    for (Shot::Vec::iterator itr = RecentlyDeadShots.begin(); itr != RecentlyDeadShots.end(); itr++)
    {
        if ((*itr)->GetPlayerID() == player)
            list.push_back(*itr);
    }

    return list;
}

//----------------Shot

Shot::Shot(uint16_t guid, const FiringInfo &info) : GUID(guid), LastUpdateTime(-1.0),
    Info(info), Pimple(NULL)
{
    StartTime = -1;
    LifeTime = info.lifetime;
    Target = NoPlayer;
}

Shot::~Shot()
{
}

void Shot::SetMetaData(const std::string& name, const char* data)
{
    if (MetaData.find(name) == MetaData.end())
        MetaData[name] = MetaDataItem();
    MetaData[name].DataS = data;
}

void Shot::SetMetaData(const std::string& name, uint32_t data)
{
    if (MetaData.find(name) == MetaData.end())
        MetaData[name] = MetaDataItem();
    MetaData[name].DataI = data;
}

bool Shot::HasMetaData(const std::string& name)
{
    return MetaData.find(name) != MetaData.end();
}

const char * Shot::GetMetaDataS(const std::string& name)
{
    auto itr = MetaData.find(name);
    return itr == MetaData.end() ? nullptr : itr->second.DataS.c_str();
}

uint32_t Shot::GetMetaDataI(const std::string& name)
{
    auto itr = MetaData.find(name);
    return itr == MetaData.end() ? 0 : itr->second.DataI;
}

bool Shot::Update(float /*dt*/)
{
    return GetLastUpdateTime() - GetStartTime() >= GetLifeTime();
}

void Shot::End()
{
    LastUpdateTime = TimeKeeper::getCurrent().getSeconds();
}

void Shot::Retarget(PlayerId target)
{
    Target = target;
}

glm::vec3 Shot::ProjectShotLocation(double deltaT)
{
    glm::vec3 vec;
    vec.x = LastUpdatePosition.x + (Info.shot.vel[0] * (float)deltaT);
    vec.y = LastUpdatePosition.y + (Info.shot.vel[1] * (float)deltaT);
    vec.z = LastUpdatePosition.z + (Info.shot.vel[2] * (float)deltaT);

    return vec;
}

//----------------ProjectileShotLogic

void ProjectileShot::Setup()
{
    prevTime = StartTime;
    lastTime = currentTime = prevTime;

    // start at first segment
    lastSegment = segment = 0;

    makeSegments(ObstacleEffect::Stop);
}

bool ProjectileShot::Update(float dt)
{
    LastUpdatePosition = ProjectShotLocation(GetLastUpdateTime() - GetStartTime());
    prevTime = currentTime;
    currentTime += dt;

    bool doEvent = false;
    bool isRico = false;
    // see if we've moved to another segment
    const int numSegments = Segments.size();
    bz_ShotEventData_V1 evtData;
    evtData.prevPos[0] = Info.shot.pos[0];
    evtData.prevPos[1] = Info.shot.pos[1];
    evtData.prevPos[2] = Info.shot.pos[2];
    evtData.prevVel[0] = Info.shot.vel[0];
    evtData.prevVel[1] = Info.shot.vel[1];
    evtData.prevVel[2] = Info.shot.vel[2];

    if (segment < numSegments && Segments[segment].end <= currentTime)
    {
        lastSegment = segment;
        while (segment < numSegments && Segments[segment].end <= currentTime)
        {
            if (++segment < numSegments)
            {
                switch (Segments[segment].reason)
                {
                case SegmentReason::Ricochet:
                {
                    doEvent = true;
                    isRico = true;
                    break;
                }
                case SegmentReason::Boundary:
                    break;
                default:
                {
                    doEvent = true;
                    isRico = false;
                }
                break;
                }
            }
        }
    }

    // if ran out of segments then expire shot on next update
    if (segment == numSegments)
    {
        if (numSegments > 0)
        {
            FlightSegment &segm = Segments[numSegments - 1];
            const float     *dir = segm.ray.getDirection();
            const float speed = hypotf(dir[0], hypotf(dir[1], dir[2]));
            float pos[3];
            segm.ray.getPoint(float(segm.end - segm.start - 1.0 / speed), pos);
        }
    }
    else// otherwise update position and velocity
    {
        float p[3];
        Segments[segment].ray.getPoint(float(currentTime - Segments[segment].start), p);
        setPosition(p);
        setVelocity(Segments[segment].ray.getDirection());

        LastUpdateVector.x = Info.shot.vel[0];
        LastUpdateVector.x = Info.shot.vel[1];
        LastUpdateVector.x = Info.shot.vel[2];

        if (doEvent)
        {
            evtData.playerID = Info.shot.player;
            evtData.shotGUID = GUID;
            evtData.shotID = Info.shot.id;
            evtData.pos[0] = Info.shot.pos[0];
            evtData.pos[1] = Info.shot.pos[1];
            evtData.pos[2] = Info.shot.pos[2];
            evtData.vel[0] = Info.shot.vel[0];
            evtData.vel[1] = Info.shot.vel[1];
            evtData.vel[2] = Info.shot.vel[2];

            worldEventManager.callEvents(isRico ? bz_eShotRicocetEvent : bz_eShotTeleportedEvent, &evtData);
        }
    }


    return Shot::Update(dt);
}



void reflect(float* v, const float* n)
{
    // normal is assumed to be normalized, v needn't be
    float d = -2.0f * ((n[0] * v[0]) + (n[1] * v[1]) + (n[2] * v[2]));

    if (d >= 0.0f)
    {
        // normal reflection
        v[0] += d * n[0];
        v[1] += d * n[1];
        v[2] += d * n[2];
    }
    else
    {
        // refraction due to inverted normal (still using the 2X factor)
        float oldSpeed = sqrtf((v[0] * v[0]) + (v[1] * v[1]) + (v[2] * v[2]));
        d = -2.0f * d; // now using 4X refraction factor
        v[0] += d * n[0];
        v[1] += d * n[1];
        v[2] += d * n[2];
        // keep the same speed as the incoming vector
        float newSpeed = sqrtf((v[0] * v[0]) + (v[1] * v[1]) + (v[2] * v[2]));
        const float scale = (oldSpeed / newSpeed);
        v[0] *= scale;
        v[1] *= scale;
        v[2] *= scale;
    }

    return;
}

bool ProjectileShot::ForceShotRico()
{
    return allShotsRicochet();
}

void ProjectileShot::makeSegments(ObstacleEffect e)
{
    // compute segments of shot until total length of segments exceeds the
    // lifetime of the shot.
    const float *v = Info.shot.vel;
    double startTime = StartTime;
    float timeLeft = Info.lifetime;
    float  minTime = BZDB.eval(StateDatabase::BZDB_MUZZLEFRONT) / hypotf(v[0], hypotf(v[1], v[2]));

    // if all shots ricochet and obstacle effect is stop, then make it ricochet
    if (e == ObstacleEffect::Stop && ForceShotRico())
        e = ObstacleEffect::Reflect;

    // prepare first segment
    float o[3], d[3];
    d[0] = v[0];
    d[1] = v[1];
    d[2] = v[2];        // use v[2] to have jumping affect shot velocity
    o[0] = Info.shot.pos[0];
    o[1] = Info.shot.pos[1];
    o[2] = Info.shot.pos[2];

    Segments.clear();

    SegmentReason reason = SegmentReason::Initial;
    int i;
    const int maxSegment = 100;
    float worldSize = BZDBCache::worldSize / 2.0f - 0.01f;
    for (i = 0; (i < maxSegment) && (timeLeft > Epsilon); i++)
    {
        // construct ray and find the first building, teleporter, or outer wall
        float o2[3];
        o2[0] = o[0] - minTime * d[0];
        o2[1] = o[1] - minTime * d[1];
        o2[2] = o[2] - minTime * d[2];

        // Sometime shot start outside world
        if (o2[0] <= -worldSize)
            o2[0] = -worldSize;
        if (o2[0] >= worldSize)
            o2[0] = worldSize;
        if (o2[1] <= -worldSize)
            o2[1] = -worldSize;
        if (o2[1] >= worldSize)
            o2[1] = worldSize;

        Ray r(o2, d);
        Ray rs(o, d);
        float t = timeLeft + minTime;
        int face;
        bool hitGround = world->getGround(r, Epsilon, t);
        const Obstacle* building = ((e == ObstacleEffect::Through) ? nullptr : world->getFirstBuilding(r, Epsilon, t));
        const Teleporter* teleporter = world->getFirstTeleporter(r, Epsilon, t, face);
        t -= minTime;
        minTime = 0.0f;
        bool ignoreHit = false;

        // if hit outer wall with ricochet and hit is above top of wall
        // then ignore hit.
        if (!teleporter && building && (e == ObstacleEffect::Reflect) &&
                (building->getType() == WallObstacle::getClassName()) &&
                ((o[2] + t * d[2]) > building->getHeight()))
            ignoreHit = true;

        // construct next shot segment and add it to list
        double endTime = startTime;
        if (t < 0.0f)
            endTime += Epsilon;
        else
            endTime += t;
        FlightSegment segm(startTime, endTime, rs, reason);
        Segments.push_back(segm);
        startTime = endTime;

        // used up this much time in segment
        if (t < 0.0f)
            timeLeft -= Epsilon;
        else
            timeLeft -= t;

        // check in reverse order to see what we hit first
        reason = SegmentReason::Through;
        if (ignoreHit)
        {
            // uh...ignore this.  usually used if you shoot over the boundary wall.
            // just move the point of origin and build the next segment
            o[0] += t * d[0];
            o[1] += t * d[1];
            o[2] += t * d[2];
            reason = SegmentReason::Boundary;
        }
        else if (teleporter)
        {
            // entered teleporter -- teleport it
            unsigned int seed = Info.shot.id + i;
            int source = world->getTeleporter(teleporter, face);
            int target = world->getTeleportTarget(source, seed);

            int outFace = -1;
            const Teleporter* outTeleporter = world->getTeleporter(target, outFace);
            o[0] += t * d[0];
            o[1] += t * d[1];
            o[2] += t * d[2];
            teleporter->getPointWRT(*outTeleporter, face, outFace,  o, d, 0.0f, o, d, NULL);
            reason = SegmentReason::Teleport;
        }
        else if (building)
        {
            // hit building -- can bounce off or stop, buildings ignored for Through
            bool handled = false;
            if (e == ObstacleEffect::Stop)
            {
                if (!building->canRicochet())
                {
                    timeLeft = 0.0f;
                    handled = true;
                }
            }
            if ((e == ObstacleEffect::Stop && !handled) || e == ObstacleEffect::Reflect)
            {
                // move origin to point of reflection
                o[0] += t * d[0];
                o[1] += t * d[1];
                o[2] += t * d[2];

                // reflect direction about normal to building
                float normal[3];
                building->get3DNormal(o, normal);
                reflect(d, normal);
                reason = SegmentReason::Ricochet;
            }

//                 if (e == ObstacleEffect::Through)
//                     assert(0);
        }
        else if (hitGround)     // we hit the ground
        {

            switch (e)
            {
            case ObstacleEffect::Stop:
            case ObstacleEffect::Through:
            {
                timeLeft = 0.0f;
                break;
            }

            case ObstacleEffect::Reflect:
            {
                // move origin to point of reflection
                o[0] += t * d[0];
                o[1] += t * d[1];
                o[2] += t * d[2];

                // reflect direction about normal to ground
                float normal[3];
                normal[0] = 0.0f;
                normal[1] = 0.0f;
                normal[2] = 1.0f;
                reflect(d, normal);
                reason = SegmentReason::Ricochet;
                break;
            }
            }
        }
    }
    lastTime = startTime;

    // make bounding box for entire path
    const int numSegments = Segments.size();
    if (numSegments > 0)
    {
        const FlightSegment    & firstSeg = Segments[0];
        bbox[0][0] = firstSeg.bbox[0][0];
        bbox[0][1] = firstSeg.bbox[0][1];
        bbox[0][2] = firstSeg.bbox[0][2];
        bbox[1][0] = firstSeg.bbox[1][0];
        bbox[1][1] = firstSeg.bbox[1][1];
        bbox[1][2] = firstSeg.bbox[1][2];
        for (i = 1; i < numSegments; i++)
        {
            const FlightSegment& segm = Segments[i];
            if (bbox[0][0] > segm.bbox[0][0]) bbox[0][0] = segm.bbox[0][0];
            if (bbox[1][0] < segm.bbox[1][0]) bbox[1][0] = segm.bbox[1][0];
            if (bbox[0][1] > segm.bbox[0][1]) bbox[0][1] = segm.bbox[0][1];
            if (bbox[1][1] < segm.bbox[1][1]) bbox[1][1] = segm.bbox[1][1];
            if (bbox[0][2] > segm.bbox[0][2]) bbox[0][2] = segm.bbox[0][2];
            if (bbox[1][2] < segm.bbox[1][2]) bbox[1][2] = segm.bbox[1][2];
        }
    }
    else
    {
        bbox[0][0] = bbox[1][0] = 0.0f;
        bbox[0][1] = bbox[1][1] = 0.0f;
        bbox[0][2] = bbox[1][2] = 0.0f;
    }
}

//----------------GuidedMissileLogic
void GuidedMissileShot::End()
{
    Shot::End();
}

void GuidedMissileShot::ProcessUpdate(ShotUpdate& update)
{
    Info.shot = update;
    Info.shot.id = this->GetGUID();
}

//----------------RicoShot
void RicoShot::Setup()
{
    prevTime = StartTime;
    lastTime = currentTime = prevTime;

    // start at first segment
    lastSegment = segment = 0;

    makeSegments(ObstacleEffect::Reflect);
}

//----------------RapidFireShot
void RapidFireShot::Setup()
{
    prevTime = StartTime;
    lastTime = currentTime = prevTime;

    // start at first segment
    lastSegment = segment = 0;

    // speed up shell and decrease lifetime
    FiringInfo& f = Info;
    f.lifetime *= BZDB.eval(StateDatabase::BZDB_RFIREADLIFE);
    float fireAdVel = BZDB.eval(StateDatabase::BZDB_RFIREADVEL);
    f.shot.vel[0] *= fireAdVel;
    f.shot.vel[1] *= fireAdVel;
    f.shot.vel[2] *= fireAdVel;

    // make segments
    makeSegments(ObstacleEffect::Stop);
}

//----------------MachineGunShot
void MachineGunShot::Setup()
{
    prevTime = StartTime;
    lastTime = currentTime = prevTime;

    // start at first segment
    lastSegment = segment = 0;

    // speed up shell and decrease lifetime
    FiringInfo& f = Info;
    f.lifetime *= BZDB.eval(StateDatabase::BZDB_MGUNADLIFE);
    float mgunAdVel = BZDB.eval(StateDatabase::BZDB_MGUNADVEL);
    f.shot.vel[0] *= mgunAdVel;
    f.shot.vel[1] *= mgunAdVel;
    f.shot.vel[2] *= mgunAdVel;

    // make segments
    makeSegments(ObstacleEffect::Stop);
}

//----------------LaserShot
void LaserShot::Setup()
{
    prevTime = StartTime;
    lastTime = currentTime = prevTime;

    // start at first segment
    lastSegment = segment = 0;

    FiringInfo& f = Info;
    f.lifetime *= BZDB.eval(StateDatabase::BZDB_LASERADLIFE);
    float laserAdVel = BZDB.eval(StateDatabase::BZDB_LASERADVEL);
    f.shot.vel[0] *= laserAdVel;
    f.shot.vel[1] *= laserAdVel;
    f.shot.vel[2] *= laserAdVel;

    // make segments
    makeSegments(ObstacleEffect::Stop);
}

//----------------ThiefShot
void ThiefShot::Setup()
{
    prevTime = StartTime;
    lastTime = currentTime = prevTime;

    // start at first segment
    lastSegment = segment = 0;

    FiringInfo& f = Info;
    f.lifetime *= BZDB.eval(StateDatabase::BZDB_THIEFADLIFE);
    float thiefAdVel = BZDB.eval(StateDatabase::BZDB_THIEFADSHOTVEL);
    f.shot.vel[0] *= thiefAdVel;
    f.shot.vel[1] *= thiefAdVel;
    f.shot.vel[2] *= thiefAdVel;

    // make segments
    makeSegments(ObstacleEffect::Stop);
}

//----------------PhantomShot
void PhantomShot::Setup()
{
    prevTime = StartTime;
    lastTime = currentTime = prevTime;

    // start at first segment
    lastSegment = segment = 0;

    makeSegments(ObstacleEffect::Through);
}

//----------------SuperBulletShot
void SuperBulletShot::Setup()
{
    prevTime = StartTime;
    lastTime = currentTime = prevTime;

    // start at first segment
    lastSegment = segment = 0;

    makeSegments(ObstacleEffect::Through);
}


//----------------ShockwaveLogic
void ShockwaveShot::Setup()
{
    LastUpdatePosition.x = BZDB.eval(StateDatabase::BZDB_SHOCKINRADIUS);
    LastUpdatePosition.y = 0;
    LastUpdatePosition.z = 0;
}

bool ShockwaveShot::Update(float dt)
{
    float delta = BZDB.eval(StateDatabase::BZDB_SHOCKOUTRADIUS) - BZDB.eval(StateDatabase::BZDB_SHOCKINRADIUS);
    LastUpdatePosition.x = BZDB.eval(StateDatabase::BZDB_SHOCKINRADIUS) + (float)(delta * GetLifeParam());
    LastUpdatePosition.y = 0;
    LastUpdatePosition.z = 0;
    return Shot::Update(dt);
}

bool ShockwaveShot::CollideBox(glm::vec3& center, glm::vec3& size, float rotation)
{
    // check the top locations
    glm::vec3 xyPlus = size;
    glm::vec3 xyNeg(-size.x, -size.y, size.z);
    glm::vec3 xPlusYNeg(size.x, -size.y, size.z);
    glm::vec3 xNegYPlus(-size.x, size.y, size.z);

    float rotRads = glm::radians(rotation);

    // rotate them all into orientation
    xyPlus = glm::rotateZ(xyPlus, rotRads);
    xyNeg  = glm::rotateZ(xyNeg, rotRads);
    xPlusYNeg = glm::rotateZ(xPlusYNeg, rotRads);
    xNegYPlus = glm::rotateZ(xNegYPlus, rotRads);

    // attach them to the center
    xyPlus += center;
    xyNeg += center;
    xPlusYNeg += center;
    xNegYPlus += center;

    // check the top
    if (PointInSphere(xyPlus) || PointInSphere(xyNeg) || PointInSphere(xPlusYNeg)
            || PointInSphere(xNegYPlus))
        return true;

    // check the bottom
    xyPlus.z = center.z;
    xyNeg.z = center.z;
    xPlusYNeg.z = center.z;
    xNegYPlus.z = center.z;

    if (PointInSphere(xyPlus) || PointInSphere(xyNeg) || PointInSphere(xPlusYNeg)
            || PointInSphere(xNegYPlus))
        return true;

    return false;
}

bool ShockwaveShot::CollideSphere(glm::vec3& center, float radius)
{
    glm::vec3 vecToPoint = center - StartPosition;
    return vecToPoint.length() <= LastUpdatePosition.x - radius;
}

bool ShockwaveShot::CollideCylinder(glm::vec3&center, float height, float radius)
{
    if (center.z > StartPosition.z + LastUpdatePosition.x)
        return false; // too high

    if (center.z + height < StartPosition.z - LastUpdatePosition.x)
        return false; // too low

    glm::vec3 vecToPoint = center - StartPosition;
    vecToPoint.z = 0;

    return vecToPoint.length() <= LastUpdatePosition.x - radius;
}

bool ShockwaveShot::PointInSphere(glm::vec3& point)
{
    glm::vec3 vecToPoint = point - StartPosition;
    return vecToPoint.length() <= LastUpdatePosition.x;
}

FlightSegment::FlightSegment()
{
    // do nothing
}

FlightSegment::FlightSegment(const double _start, double _end, const Ray& _ray, SegmentReason _reason) :
    start(_start),
    end(_end),
    ray(_ray),
    reason(_reason)
{
    // compute bounding box
    ray.getPoint(0.0f, bbox[0]);
    ray.getPoint(float(end - start), bbox[1]);
    if (bbox[0][0] > bbox[1][0])
    {
        const float tmp = bbox[0][0];
        bbox[0][0] = bbox[1][0];
        bbox[1][0] = tmp;
    }
    if (bbox[0][1] > bbox[1][1])
    {
        const float tmp = bbox[0][1];
        bbox[0][1] = bbox[1][1];
        bbox[1][1] = tmp;
    }
    if (bbox[0][2] > bbox[1][2])
    {
        const float tmp = bbox[0][2];
        bbox[0][2] = bbox[1][2];
        bbox[1][2] = tmp;
    }
}

FlightSegment::FlightSegment(const FlightSegment& segment) :
    start(segment.start),
    end(segment.end),
    ray(segment.ray),
    reason(segment.reason)
{
    // copy bounding box
    bbox[0][0] = segment.bbox[0][0];
    bbox[0][1] = segment.bbox[0][1];
    bbox[0][2] = segment.bbox[0][2];
    bbox[1][0] = segment.bbox[1][0];
    bbox[1][1] = segment.bbox[1][1];
    bbox[1][2] = segment.bbox[1][2];
}

FlightSegment::~FlightSegment()
{
    // do nothing
}

FlightSegment&    FlightSegment::operator=(const FlightSegment& segment)
{
    if (this != &segment)
    {
        start = segment.start;
        end = segment.end;
        ray = segment.ray;
        reason = segment.reason;
        bbox[0][0] = segment.bbox[0][0];
        bbox[0][1] = segment.bbox[0][1];
        bbox[0][2] = segment.bbox[0][2];
        bbox[1][0] = segment.bbox[1][0];
        bbox[1][1] = segment.bbox[1][1];
        bbox[1][2] = segment.bbox[1][2];
    }
    return *this;
}
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
