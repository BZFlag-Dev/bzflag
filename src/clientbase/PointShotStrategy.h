/* bzflag
 * Copyright (c) 1993-2010 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef __POINTSHOTSTRATEGY_H__
#define __POINTSHOTSTRATEGY_H__

/* interface header */
#include "ShotStrategy.h"

/* system interface headers */
#include <vector>

/* common interface headers */
#include "BzTime.h"
#include "ShotPathSegment.h"
#include "Extents.h"


class Obstacle;


class PointShotStrategy : public ShotStrategy {
  public:
    PointShotStrategy(ShotPath* _path, bool gmTest = false);
    ~PointShotStrategy();

  protected:
    float checkShotHit(const ShotCollider& tank, fvec3& position, float radius) const;

    BzTime prevTime;
    BzTime currentTime;
    BzTime lastTime;
    int    segment;
    int    lastSegment;

    std::vector<ShotPathSegment> segments;

    Extents bbox;

    const bool gmTest;
};



#endif /* __POINTSHOTSTRATEGY_H__ */

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=2 tabstop=8 expandtab expandtab
