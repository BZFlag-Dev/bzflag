/* bzflag
 * Copyright (c) 1993 - 2003 Tim Riker
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
 *
 */

#ifndef	BZF_ROBOT_PLAYER_H
#define	BZF_ROBOT_PLAYER_H

#if defined(_WIN32)
	#pragma warning(disable: 4786)
#endif

#include <vector>
#include "common.h"
#include "LocalPlayer.h"
#include "Region.h"

class ServerLink;

class RobotPlayer : public BaseLocalPlayer {
  public:
			RobotPlayer(const PlayerId&,
				const char* name, ServerLink*,
				const char* _email);
			~RobotPlayer();

    float		getTargetPriority(const Player*) const;
    const Player*	getTarget() const;
    void		setTarget(const Player*);
    static void		setObstacleList(std::vector<BzfRegion*>*);

    ShotPath*		getShot(int index) const;

    void		setTeam(TeamColor);
    void		restart();
    bool		checkHit(const Player* source, const ShotPath*& hit,
							float& minTime) const;
    void		explodeTank();
    void		changeScore(short deltaWins, short deltaLosses, short deltaTeamKills);

  private:
    bool		doEndShot(int index, bool isHit, float* pos);
    void		doUpdate(float dt);
    void		doUpdateMotion(float dt);
    BzfRegion*		findRegion(const float p[2], float nearest[2]) const;
    float		getRegionExitPoint(
				const float p1[2], const float p2[2],
				const float a[2], const float targetPoint[2],
				float mid[2], float& priority);
     void		findPath(RegionPriorityQueue& queue,
				BzfRegion* region, BzfRegion* targetRegion,
				const float targetPoint[2], int mailbox);

  private:
    ServerLink*		server;
    LocalShotPath**	shots;
    const Player*	target;
    std::vector<RegionPoint>	path;
    int			pathIndex;
    float		timeSinceShot;
    float		timerForShot;
    static std::vector<BzfRegion*>* obstacleList;
};

#endif // BZF_ROBOT_PLAYER_H

/* Local Variables: ***
 * mode:C++ ***
 * tab-width: 8 ***
 * c-basic-offset: 2 ***
 * indent-tabs-mode: t ***
 * End: ***
 * ex: shiftwidth=2 tabstop=8
 */

