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

#ifndef __GUIDEDMISSLESTRATEGY_H__
#define __GUIDEDMISSLESTRATEGY_H__

/* interface header */
#include "PointShotStrategy.h"

/* system interface headers */
#include <vector>

/* common interface headers */
#include "SceneDatabase.h"
#include "BzTime.h"
#include "BoltSceneNode.h"

/* local interface headers */
#include "BaseLocalPlayer.h"
#include "ShotPathSegment.h"


class GuidedMissileStrategy : public PointShotStrategy {
  public:
    GuidedMissileStrategy(ShotPath*);
    ~GuidedMissileStrategy();

    void update(float dt);
    bool predictPosition(float dt, fvec3& p) const;
    bool predictVelocity(float dt, fvec3& v) const;

    float checkHit(const ShotCollider&, fvec3& hitPos) const;
    void  sendUpdate(const FiringInfo&) const;
    void  readUpdate(void* buffer);
    void  addShot(SceneDatabase*, bool colorblind);
    void  expire();
    void  radarRender() const;

  private:
    float checkBuildings(const Ray& ray);
    bool  _predict(float dt, fvec3& p, fvec3& v) const;

  private:
    BoltSceneNode* ptSceneNode;

    BzTime lastPuff;

    float speed;
    fvec3 nextPos;
    fvec3 nextVel;

    int renderTimes;

    float puffTime;
    bool  needUpdate;

    PlayerId lastTarget;
};


#endif /* __GUIDEDMISSLESTRATEGY_H__ */

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=2 tabstop=8
