/* bzflag
 * Copyright (c) 1993 - 2008 Tim Riker
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
#include "ShotPath.h"

/* system headers */
#include <assert.h>

/* local implementation headers */
#include "SegmentedShotStrategy.h"
#include "GuidedMissleStrategy.h"
#include "ShockWaveStrategy.h"

//
// ShotPath
//

ShotPath::ShotPath(const FiringInfo& info, double now) :
				firingInfo(info),
				reloadTime(BZDB.eval(StateDatabase::BZDB_RELOADTIME)),
				expiring(false),
				expired(false)
{
  startTime = info.timeSent;
  currentTime = now;

  switch(info.shotType)
  {
    default:
      strategy = new NormalShotStrategy(this);
    break;

    case GMShot:
      strategy = new GuidedMissileStrategy(this);
      break;

    case LaserShot:
      strategy = new LaserStrategy(this);
      break;

    case ThiefShot:
      strategy = new ThiefStrategy(this);
      break;

    case SuperShot:
      strategy = new SuperBulletStrategy(this);
      break;

    case PhantomShot:
      strategy = new PhantomBulletStrategy(this);
      break;

    case ShockWaveShot:
      strategy = new ShockWaveStrategy(this);
      break;

    case RicoShot:
      strategy = new RicochetStrategy(this);
      break;

    case MachineGunShot:
      strategy = new MachineGunStrategy(this);
      break;

    case RapidFireShot:
      strategy = new RapidFireStrategy(this);
      break;
  }
}

ShotPath::~ShotPath()
{
  delete strategy;
}

float			ShotPath::checkHit(const ShotCollider& tank,
							float position[3]) const
{
  return strategy->checkHit(tank, position);
}

bool			ShotPath::isStoppedByHit() const
{
  return strategy->isStoppedByHit();
}

void			ShotPath::addShot(SceneDatabase* scene,
						bool colorblind)
{
  strategy->addShot(scene, colorblind);
}

void			ShotPath::radarRender() const
{
  if (!isExpired())
    strategy->radarRender();
}

void			ShotPath::updateShot(float dt)
{
  // get new time step and set current time
  currentTime += dt;

  // update shot
  if (!expired)
  {
    if (expiring)
      setExpired();
    else
      getStrategy()->update(dt);
  }
}

void			ShotPath::setReloadTime(float _reloadTime)
{
  reloadTime = _reloadTime;
}

void			ShotPath::setPosition(const float* p)
{
  firingInfo.shot.pos[0] = p[0];
  firingInfo.shot.pos[1] = p[1];
  firingInfo.shot.pos[2] = p[2];
}

void			ShotPath::setVelocity(const float* v)
{
  firingInfo.shot.vel[0] = v[0];
  firingInfo.shot.vel[1] = v[1];
  firingInfo.shot.vel[2] = v[2];
}

void			ShotPath::setExpiring()
{
  expiring = true;
}

void			ShotPath::setExpired()
{
  expiring = true;
  expired = true;
  getStrategy()->expire();
}

void			ShotPath::boostReloadTime(float dt)
{
  reloadTime += dt;
}

bool                    ShotPath::predictPosition(float dt, float p[3]) const
{
  return getStrategy()->predictPosition(dt, p);
}

bool                    ShotPath::predictVelocity(float dt, float p[3]) const
{
  return getStrategy()->predictVelocity(dt, p);
}

//
// LocalShotPath
//

LocalShotPath::LocalShotPath(const FiringInfo& info, double now) :
				ShotPath(info,now)
{
  // do nothing
}

LocalShotPath::~LocalShotPath()
{
  // do nothing
}

void			LocalShotPath::update(float dt)
{
  getFiringInfo().shot.dt += dt;
  updateShot(dt);

  // send updates if necessary
  getStrategy()->sendUpdate(getFiringInfo());
}

//
// RemoteShotPath
//

RemoteShotPath::RemoteShotPath(const FiringInfo& info,double now) :
				ShotPath(info,now)
{
  // do nothing
}

RemoteShotPath::~RemoteShotPath()
{
  // do nothing
}

void			RemoteShotPath::update(float dt)
{
  // update shot
  updateShot(dt);
}

void			RemoteShotPath::update(const ShotUpdate& shot,
				uint16_t code, void* msg)
{
  // update shot info
  getFiringInfo().shot = shot;

  // let the strategy see the message
  getStrategy()->readUpdate(code, msg);
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
