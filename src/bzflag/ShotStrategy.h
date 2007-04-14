/* bzflag
 * Copyright (c) 1993 - 2007 Tim Riker
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
 *	Interface for all shot flight path strategies.  A
 *	strategy encapsulates the algorithm for computing
 *	the path taken by a shot.
 */

#ifndef __SHOTSTRATEGY_H__
#define __SHOTSTRATEGY_H__

#include "common.h"

/* common interface headers */
#include "Ray.h"
#include "Obstacle.h"
#include "Teleporter.h"
#include "SceneDatabase.h"

/* local interface headers */
#include "BaseLocalPlayer.h"
#include "ShotPath.h"

class ShotCollider
{
public:
  float	  position[3];
  Ray	  motion;
  float	  radius;
  float	  size[3];

  bool	  test2D;
  float	  angle;
  float	  lenght;

  float	  bbox[2][3];

  bool	  testLastSegment;
};

class ShotPath;

class ShotStrategy {
  public:
			ShotStrategy(ShotPath*);
    virtual		~ShotStrategy();

    virtual void	update(float dt) = 0;
    virtual float	checkHit(const ShotCollider&, float[3])const = 0;
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

inline const ShotPath&	ShotStrategy::getPath() const
{
  return *path;
}


#endif /* __SHOTSTRATEGY_H__ */

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
