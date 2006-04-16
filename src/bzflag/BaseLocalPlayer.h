/* bzflag
 * Copyright (c) 1993 - 2006 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef __BASELOCALPLAYER_H__
	#define __BASELOCALPLAYER_H__

// common - 1st one
	#include "common.h"

/* common interface headers */
	#include "Ray.h"
	#include "TimeKeeper.h"

/* local interface headers */
	#include "Player.h"
	#include "ShotPath.h"


class BaseLocalPlayer: public Player
{
public:
	BaseLocalPlayer( const PlayerId &, const char *name, const char *email );
	~BaseLocalPlayer();

	void update();
	Ray getLastMotion()const;
	const float( *getLastMotionBBox()const )[3];

	virtual void explodeTank() = 0;
	virtual bool checkHit( const Player *source, const ShotPath * &hit, float &minTime )const = 0;
protected:
	int getSalt();
	virtual void doUpdate( float dt ) = 0;
	virtual void doUpdateMotion( float dt ) = 0;

protected:
	TimeKeeper lastTime;
	float lastPosition[3];
	// bbox of last motion
	float bbox[2][3];

private:
	int salt;
};


#endif /* __BASELOCALPLAYER_H__ */

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
