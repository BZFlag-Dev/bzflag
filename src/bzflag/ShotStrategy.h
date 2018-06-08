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

/*
 * ShotStrategy:
 *  Interface for all shot flight path strategies.  A
 *  strategy encapsulates the algorithm for computing
 *  the path taken by a shot.
 */

#ifndef __SHOTSTRATEGY_H__
#define __SHOTSTRATEGY_H__

#include "common.h"

#include "ShotPath.h"

/* common interface headers */
#include "Ray.h"
#include "Obstacle.h"
#include "Teleporter.h"
#include "SceneDatabase.h"

class BaseLocalPlayer;

class ShotStrategy : public ShotPath
{
public:
    ShotStrategy(const FiringInfo& _info) : ShotPath(_info) {}

    virtual     ~ShotStrategy();

    virtual bool    isStoppedByHit() const;
    virtual void    expire();

    // first part of message must be the
    // ShotUpdate portion of FiringInfo.
    virtual void    sendUpdate(const FiringInfo&) const;

    // update shot based on message.  code is the message code.  msg
    // points to the part of the message after the ShotUpdate portion.
    virtual void    readUpdate(uint16_t code, const void* msg);

    static const Obstacle*  getFirstBuilding(const Ray&, float min, float& t);
    static void     reflect(float* v, const float* n); // const

protected:

    const Teleporter*   getFirstTeleporter(const Ray&, float min,
                                           float& t, int& f) const;
    bool        getGround(const Ray&, float min, float &t) const;
};

#endif /* __SHOTSTRATEGY_H__ */

// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
