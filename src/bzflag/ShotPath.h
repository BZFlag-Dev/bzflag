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

/*
 * ShotPath:
 *	Encapsulates the path a shot follows.  Most paths can
 *	be computed at the instant of firing (though they may
 *	terminate early because of a hit).  Some paths need
 *	to be updated continuously during flight.
 *
 * RemoteShotPath:
 *	A ShotPath acting as a proxy for a remote ShotPath.
 *	Created by a LocalPlayer on behalf of a RemotePlayer.
 */

#ifndef	__SHOTPATH_H__
#define	__SHOTPATH_H__

#include "common.h"

/* common interface headers */
#include "TimeKeeper.h"
#include "Flag.h"
#include "ShotUpdate.h"

/* local interface headers */
#include "ShotStrategy.h"
#include "SceneDatabase.h"

class ShotStrategy;
class ShotCollider;

class ShotPath {
  public:
    ShotPath(const FiringInfo&, double);
   virtual		~ShotPath();

    bool		isExpiring() const;
    bool		isExpired() const;
    bool		isReloaded() const;
    const PlayerId&	getPlayer() const;
    int		getShotId() const;
    ShotType		getShotType() const;
    FlagType*		getFlag() const;
    float		getLifetime() const;
    float		getReloadTime() const;
    const double	getStartTime() const;
    const double	getCurrentTime() const;
    const float*	getPosition() const;
    const float*	getVelocity() const;

    float		checkHit(const ShotCollider&, float[3]) const;
    void		setExpiring();
    void		setExpired();
    bool		isStoppedByHit() const;
    void		boostReloadTime(float dt);
    void		setLocal (bool loc) {local = loc;}
    bool		isLocal ( void ){return local;}

    void		addShot(SceneDatabase*, bool colorblind);

    void		radarRender() const;
    FiringInfo&		getFiringInfo();
    TeamColor		getTeam() const;

  virtual void	  update(float);

  //This function can be used to predict the position of the shot after a given time dt. Function returns true iff. the shot is still alive.
  bool    predictPosition(float dt, float p[3]) const;
  bool    predictVelocity(float dt, float p[3]) const;

  protected:
    void		updateShot(float dt);
    const ShotStrategy*	getStrategy() const;
    ShotStrategy*	getStrategy();

    friend class ShotStrategy;
    void		setReloadTime(float);
    void		setPosition(const float*);
    void		setVelocity(const float*);

  private:
    ShotStrategy*	strategy;		// strategy for moving shell
    FiringInfo		firingInfo;		// shell information
    float		reloadTime;		// time to reload
    double		startTime;		// time of firing
    double		currentTime;		// current time
    bool		expiring;		// shot has almost terminated
    bool		expired;		// shot has terminated
    bool		local;			// shot is local, and must be ended localy, REMOVE ME WHEN THE SERVER DOES THIS
};

class LocalShotPath : public ShotPath {
  public:
			LocalShotPath(const FiringInfo&,double);
			~LocalShotPath();

    void		update(float dt);
};

class RemoteShotPath : public ShotPath {
  public:
			RemoteShotPath(const FiringInfo&,double);
			~RemoteShotPath();

    void		update(float dt);
    void		update(const ShotUpdate& shot,
				uint16_t code, void* msg);
};

//
// ShotPath
//

inline bool		ShotPath::isExpiring() const
{
  return expiring;
}

inline bool		ShotPath::isExpired() const
{
  return expired;
}

inline bool		ShotPath::isReloaded() const
{
  return (currentTime - startTime >= reloadTime);
}

inline const PlayerId&	ShotPath::getPlayer() const
{
  return firingInfo.shot.player;
}

inline int		ShotPath::getShotId() const
{
  return firingInfo.shot.id;
}

inline ShotType		ShotPath::getShotType() const
{
	return firingInfo.shotType;
}

inline FlagType*	ShotPath::getFlag() const
{
  return firingInfo.flagType;
}

inline float		ShotPath::getLifetime() const
{
  return firingInfo.lifetime;
}

inline float		ShotPath::getReloadTime() const
{
  return reloadTime;
}

inline const double ShotPath::getStartTime() const
{
  return startTime;
}

inline const double ShotPath::getCurrentTime() const
{
  return currentTime;
}

inline const float*	ShotPath::getPosition() const
{
  return firingInfo.shot.pos;
}

inline const float*	ShotPath::getVelocity() const
{
  return firingInfo.shot.vel;
}

inline FiringInfo&	ShotPath::getFiringInfo()
{
  return firingInfo;
}

inline	TeamColor	ShotPath::getTeam() const
{
  return firingInfo.shot.team;
}

inline const ShotStrategy*	ShotPath::getStrategy() const
{
  return strategy;
}

inline ShotStrategy*	ShotPath::getStrategy()
{
  return strategy;
}

#endif /* __SHOTPATH_H__ */

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
