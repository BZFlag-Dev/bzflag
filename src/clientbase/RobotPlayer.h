/* bzflag
 * Copyright (c) 1993 - 2009 Tim Riker
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
 *
 */

#ifndef	BZF_ROBOT_PLAYER_H
#define	BZF_ROBOT_PLAYER_H

#include "common.h"

// interface header
#include "LocalPlayer.h"

// system headers
#include <vector>

// common headers
#include "vectors.h"

// local headers
#include "Region.h"
#include "RegionPriorityQueue.h"
#include "ServerLink.h"


class RobotPlayer : public LocalPlayer {
  public:
    RobotPlayer(const PlayerId&, const char* name, ServerLink*);

    float		getTargetPriority(const Player*) const;
    const Player*	getTarget() const;
    void		setTarget(const Player*);
    static void		setObstacleList(std::vector<BzfRegion*>*);

    void		restart(const fvec3& pos, float azimuth);
    void		explodeTank();

  private:
    void		doUpdate(float dt);
    void		doUpdateMotion(float dt);
    BzfRegion*		findRegion(const fvec2&, fvec2&) const;
    float		getRegionExitPoint(
				const fvec2&, const fvec2&,
				const fvec2&, const fvec2&,
				fvec2&, float& priority);
     void		findPath(RegionPriorityQueue& queue,
				BzfRegion* region, BzfRegion* targetRegion,
				const fvec2&, int mailbox);

     void		projectPosition(const Player *targ, const float t, fvec3& pos) const;
     void		getProjectedPosition(const Player *targ, fvec3& projpos) const;

  private:
    const Player*	target;
    std::vector<RegionPoint>	path;
    int			pathIndex;
    float		timerForShot;
    bool		drivingForward;
    static std::vector<BzfRegion*>* obstacleList;
};

#endif // BZF_ROBOT_PLAYER_H

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
