/* bzflag
 * Copyright 1993-1999, Chris Schoeneman
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/*
 * ShotUpdate:
 *	Encapsulates info needed to update a shot on remote
 *	hosts. Can be packed for transmission on the net.
 *
 * FiringInfo:
 *	Encapsulates info needed to create RemoteShotPath.
 *	Can be packed for transmission on the net.
 *
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

#ifndef	BZF_SHOT_PATH_H
#define	BZF_SHOT_PATH_H

#include "common.h"
#include "global.h"
#include "TimeKeeper.h"
#include "Pack.h"
#include "Address.h"
#include "Player.h"

class ShotStrategy;
class BaseLocalPlayer;
class SceneDatabase;

const int		ShotUpdatePLen = PlayerIdPLen + 30;
const int		FiringInfoPLen = ShotUpdatePLen + 6;

struct ShotUpdate {
  public:
    void*		pack(void*) const;
    void*		unpack(void*);

  public:
    PlayerId		player;			// who's shot
    uint16_t		id;			// shot id unique to player
    float		pos[3];			// shot position
    float		vel[3];			// shot velocity
    float		dt;			// time shot has existed
};

struct FiringInfo {
  public:
			FiringInfo();
			FiringInfo(const BaseLocalPlayer&, int id);

    void*		pack(void*) const;
    void*		unpack(void*);

  public:
    ShotUpdate		shot;
    FlagId		flag;			// flag when fired
    float		lifetime;		// lifetime of shot (s)
};

class ShotPath {
  public:
    virtual		~ShotPath();

    boolean		isExpiring() const;
    boolean		isExpired() const;
    boolean		isReloaded() const;
    const PlayerId&	getPlayer() const;
    uint16_t		getShotId() const;
    FlagId		getFlag() const;
    float		getLifetime() const;
    float		getReloadTime() const;
    const TimeKeeper&	getStartTime() const;
    const TimeKeeper&	getCurrentTime() const;
    const float*	getPosition() const;
    const float*	getVelocity() const;

    float		checkHit(const BaseLocalPlayer*, float position[3]) const;
    void		setExpiring();
    void		setExpired();
    boolean		isStoppedByHit() const;
    void		boostReloadTime(float dt);

    void		addShot(SceneDatabase*, boolean colorblind);

    void		radarRender() const;

  protected:
			ShotPath(const FiringInfo&);
    void		updateShot(float dt);
    FiringInfo&		getFiringInfo();
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
    TimeKeeper		startTime;		// time of firing
    TimeKeeper		currentTime;		// current time
    boolean		expiring;		// shot has almost terminated
    boolean		expired;		// shot has terminated
};

class LocalShotPath : public ShotPath {
  public:
			LocalShotPath(const FiringInfo&);
			~LocalShotPath();

    void		update(float dt);
};

class RemoteShotPath : public ShotPath {
  public:
			RemoteShotPath(const FiringInfo&);
			~RemoteShotPath();

    void		update(float dt);
    void		update(const ShotUpdate& shot,
				uint16_t code, void* msg);
};

//
// ShotPath
//

inline boolean		ShotPath::isExpiring() const
{
  return expiring;
}

inline boolean		ShotPath::isExpired() const
{
  return expired;
}

inline boolean		ShotPath::isReloaded() const
{
  return (currentTime - startTime >= reloadTime);
}

inline const PlayerId&	ShotPath::getPlayer() const
{
  return firingInfo.shot.player;
}

inline uint16_t		ShotPath::getShotId() const
{
  return firingInfo.shot.id;
}

inline FlagId		ShotPath::getFlag() const
{
  return firingInfo.flag;
}

inline float		ShotPath::getLifetime() const
{
  return firingInfo.lifetime;
}

inline float		ShotPath::getReloadTime() const
{
  return reloadTime;
}

inline const TimeKeeper	&ShotPath::getStartTime() const
{
  return startTime;
}

inline const TimeKeeper	&ShotPath::getCurrentTime() const
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

inline const ShotStrategy*	ShotPath::getStrategy() const
{
  return strategy;
}

inline ShotStrategy*	ShotPath::getStrategy()
{
  return strategy;
}

#endif // BZF_SHOT_PATH_H
