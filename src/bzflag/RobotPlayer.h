/* bzflag
 * Copyright (c) 1993 - 2001 Tim Riker
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
 *
 */

#ifndef	BZF_ROBOT_PLAYER_H
#define	BZF_ROBOT_PLAYER_H

#include "LocalPlayer.h"
#include "Region.h"

class ServerLink;

BZF_DEFINE_ALIST(RegionAzimuthList, float);

class RobotPlayer : public BaseLocalPlayer {
  public:
			RobotPlayer(const PlayerId&,
				const char* name, ServerLink*,
				const char* _email = "anonymous");
			~RobotPlayer();

    float		getTargetPriority(const Player*) const;
    const Player*	getTarget() const;
    void		setTarget(const RegionList& regions, const Player*);

    ShotPath*		getShot(int index) const;

    void		setTeam(TeamColor);
    void		restart();
    boolean		checkHit(const Player* source, const ShotPath*& hit,
							float& minTime) const;
    void		explodeTank();
    void		changeScore(short deltaWins, short deltaLosses);

  private:
    boolean		doEndShot(int index, boolean isHit, float* pos);
    void		doUpdate(float dt);
    void		doUpdateMotion(float dt);
    BzfRegion*		findRegion(const RegionList& list,
					const float p[2]) const;
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
    RegionPointList	path;
    RegionAzimuthList	pathAzimuth;
    int			pathIndex;
    boolean		scoreChanged;
    float		timeSinceShot;
};

#endif // BZF_ROBOT_PLAYER_H
