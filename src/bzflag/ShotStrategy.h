/* bzflag
 * Copyright (c) 1993 - 2004 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/*
 * ShotStrategy:
 *	Interface for all shot flight path strategies.  A
 *	strategy encapsulates the algorithm for computing
 *	the path taken by a shot.
 */

#ifndef	BZF_SHOT_STRATEGY_H
#define	BZF_SHOT_STRATEGY_H

#include "common.h"
#include "ShotPath.h"
#include "Obstacle.h"
#include "Teleporter.h"
#include "Ray.h"

class BaseLocalPlayer;
class SceneDatabase;
class BoltSceneNode;
class PhotonTorpedoSceneNode;
class SphereSceneNode;
class LaserSceneNode;

class ShotStrategy {
  public:
			ShotStrategy(ShotPath*);
    virtual		~ShotStrategy();

    virtual void	update(float dt) = 0;
    virtual float	checkHit(const BaseLocalPlayer*, float pos[3]) const = 0;
    virtual bool	isStoppedByHit() const;
    virtual void	addShot(SceneDatabase*, bool colorblind) = 0;
    virtual void	expire();
    virtual void	radarRender() const = 0;

    // first part of message must be the
    // ShotUpdate portion of FiringInfo.
    virtual void	sendUpdate(const FiringInfo&) const;

    // update shot based on message.  code is the message code.  msg
    // points to the part of the message after the ShotUpdate portion.
    virtual void	readUpdate(uint16_t code, void* msg);

    static const Obstacle*	getFirstBuilding(const Ray&, float min, float& t);
    static void		reflect(float* v, const float* n); // const

  protected:
    const ShotPath&	getPath() const;
    FiringInfo&		getFiringInfo(ShotPath*) const;
    void		setReloadTime(float) const;
    void		setPosition(const float*) const;
    void		setVelocity(const float*) const;
    void		setExpiring() const;
    void		setExpired() const;

    const Teleporter*	getFirstTeleporter(const Ray&, float min,
							float& t, int& f) const;
    bool		getGround(const Ray&, float min, float &t) const;

  private:
    ShotPath*		path;
};


// TEMP - until classes below are broken out
#include "ShotPathSegment.h"
#include "SegmentedShotStrategy.h"


class ShockWaveStrategy : public ShotStrategy {
  public:
			ShockWaveStrategy(ShotPath*);
			~ShockWaveStrategy();

    void		update(float dt);
    float		checkHit(const BaseLocalPlayer*, float[3]) const;
    bool		isStoppedByHit() const;
    void		addShot(SceneDatabase*, bool colorblind);
    void		radarRender() const;

  private:
    SphereSceneNode*	shockNode;
    float		radius;
    float		radius2;
};

//
// ShotStrategy
//

inline const ShotPath&	ShotStrategy::getPath() const
{
  return *path;
}

#endif // BZF_SHOT_STRATEGY_H

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
