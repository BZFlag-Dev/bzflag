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

#ifndef __SEGMENTEDSHOTSTRATEGY_H__
#define __SEGMENTEDSHOTSTRATEGY_H__

/* interface header */
#include "ShotStrategy.h"

/* system interface headers */
#include <vector>

/* common interface headers */
#include "SceneDatabase.h"
#include "TimeKeeper.h"
#include "LaserSceneNode.h"

/* local interface headers */
#include "BaseLocalPlayer.h"
#include "ShotPathSegment.h"
#include "BoltSceneNode.h"


class SegmentedShotStrategy : public ShotStrategy
{
public:
    SegmentedShotStrategy(const FiringInfo&, bool useSuperTexture, bool faint = false);
    ~SegmentedShotStrategy();

    void        update(float dt);
    float       checkHit(const BaseLocalPlayer*, float[3]) const;
    void        addShot(SceneDatabase*, bool colorblind);
    void        radarRender() const;
    TeamColor   team;

protected:
    enum ObstacleEffect
    {
        Stop = 0,
        Through = 1,
        Reflect = 2
    };
    void        makeSegments(ObstacleEffect = Stop);
    const std::vector<ShotPathSegment>& getSegments() const;

    void        setCurrentTime(const TimeKeeper&);
    const TimeKeeper&   getLastTime() const;

    bool        isOverlapping(const float (*bbox1)[3],
                              const float (*bbox2)[3]) const;

    void        setCurrentSegment(int segment);

private:
    TimeKeeper      prevTime;
    TimeKeeper      currentTime;
    TimeKeeper      lastTime;
    int         segment, lastSegment;
    std::vector<ShotPathSegment>    segments;
    BoltSceneNode*  boltSceneNode;
    float       bbox[2][3];
};

class NormalShotStrategy : public SegmentedShotStrategy
{
public:
    NormalShotStrategy(const FiringInfo&);
    ~NormalShotStrategy();
};

class RapidFireStrategy : public SegmentedShotStrategy
{
public:
    RapidFireStrategy(const FiringInfo&);
    ~RapidFireStrategy();
};

class ThiefStrategy : public SegmentedShotStrategy
{
public:
    ThiefStrategy(const FiringInfo&);
    ~ThiefStrategy();
    void        update(float dt);
    bool        isStoppedByHit() const;
    void        addShot(SceneDatabase*, bool colorblind);
    void        radarRender() const;

private:
    float       cumTime;
    float       endTime;
    LaserSceneNode**    thiefNodes;
};

class MachineGunStrategy : public SegmentedShotStrategy
{
public:
    MachineGunStrategy(const FiringInfo&);
    ~MachineGunStrategy();
};

class LaserStrategy : public SegmentedShotStrategy
{
public:
    LaserStrategy(const FiringInfo&);
    ~LaserStrategy();

    void        update(float dt);
    bool        isStoppedByHit() const;
    void        addShot(SceneDatabase*, bool colorblind);
    void        radarRender() const;

private:
    float       cumTime;
    float       endTime;
    LaserSceneNode**    laserNodes;
};

class RicochetStrategy : public SegmentedShotStrategy
{
public:
    RicochetStrategy(const FiringInfo&);
    ~RicochetStrategy();
};

class SuperBulletStrategy : public SegmentedShotStrategy
{
public:
    SuperBulletStrategy(const FiringInfo&);
    ~SuperBulletStrategy();
};

class PhantomBulletStrategy : public SegmentedShotStrategy
{
public:
    PhantomBulletStrategy(const FiringInfo&);
    ~PhantomBulletStrategy();
};


#endif /* __SEGMENTEDSHOTSTRATEGY_H__ */

// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
