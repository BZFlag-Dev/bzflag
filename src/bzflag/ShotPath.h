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

/*
 * ShotPath:
 *  Encapsulates the path a shot follows.  Most paths can
 *  be computed at the instant of firing (though they may
 *  terminate early because of a hit).  Some paths need
 *  to be updated continuously during flight.
 *
 */

#ifndef __SHOTPATH_H__
#define __SHOTPATH_H__

#include "common.h"

/* common interface headers */
#include "TimeKeeper.h"
#include "Flag.h"
#include "ShotUpdate.h"

/* local interface headers */
#include "SceneDatabase.h"

#include <memory>
#include <vector>
#include <list>

class ShotPath
{
public:
    typedef std::shared_ptr<ShotPath> Ptr;
    typedef std::vector<Ptr> Vec;
    typedef std::list<Ptr> List;

    static ShotPath::Ptr Create(const FiringInfo&);

    virtual     ~ShotPath();

    bool                isExpiring() const;
    bool                isExpired() const;
    const PlayerId&     getPlayer() const;
    uint16_t            getShotId() const;
    FlagType::Ptr           getFlag() const;
    float               getLifetime() const;
    const TimeKeeper&   getStartTime() const;
    const TimeKeeper&   getCurrentTime() const;
    const float*        getPosition() const;
    const float*        getVelocity() const;

    virtual float       checkHit(const BaseLocalPlayer*, float position[3]) const = 0;
    void                setExpiring();
    void                setExpired();
    virtual bool        isStoppedByHit() const = 0;

    virtual void        addShot(SceneDatabase*, bool colorblind) = 0;

    virtual void        radarRender() const = 0;
    FiringInfo&         getFiringInfo();
    TeamColor           getTeam() const;

    virtual void        update(float dt);
    virtual void        update(const ShotUpdate& shot, uint16_t code, const void* msg);

    virtual void        sendUpdate(const FiringInfo&) const = 0;
    virtual void        readUpdate(uint16_t code, const void* msg) = 0;

    virtual void        expire() = 0;

    bool                sendUpdates = false;

protected:
    ShotPath(const FiringInfo&);
    void                updateShot(float dt);

    void                setPosition(const float*);
    void                setVelocity(const float*);

    FiringInfo      firingInfo;     // shell information
    TimeKeeper      startTime;      // time of firing
    TimeKeeper      currentTime;    // current time
    bool            expiring;       // shot has almost terminated
    bool            expired;        // shot has terminated
};

//
// ShotPath
//

inline bool     ShotPath::isExpiring() const
{
    return expiring;
}

inline bool     ShotPath::isExpired() const
{
    return expired;
}

inline const PlayerId&  ShotPath::getPlayer() const
{
    return firingInfo.shot.player;
}

inline uint16_t     ShotPath::getShotId() const
{
    return firingInfo.shot.id;
}

inline FlagType::Ptr    ShotPath::getFlag() const
{
    return firingInfo.flagType;
}

inline float        ShotPath::getLifetime() const
{
    return firingInfo.lifetime;
}

inline const TimeKeeper &ShotPath::getStartTime() const
{
    return startTime;
}

inline const TimeKeeper &ShotPath::getCurrentTime() const
{
    return currentTime;
}

inline const float* ShotPath::getPosition() const
{
    return firingInfo.shot.pos;
}

inline const float* ShotPath::getVelocity() const
{
    return firingInfo.shot.vel;
}

inline FiringInfo&  ShotPath::getFiringInfo()
{
    return firingInfo;
}

inline  TeamColor   ShotPath::getTeam() const
{
    return firingInfo.shot.team;
}


#endif /* __SHOTPATH_H__ */

// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
