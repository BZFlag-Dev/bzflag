/* bzflag
 * Copyright (c) 1993 - 2002 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef BZF_REGION_MANAGER_BASE_H
#define BZF_REGION_MANAGER_BASE_H

#include "math3D.h"
#include "global.h"

#define RGNMGR_BASE	(RegionManagerBase::getInstance())

class RegionShape;

class RegionManagerBase {
public:
	~RegionManagerBase();

	static RegionManagerBase*	getInstance();

	// add a team base region.  spawn regions are where players on
	// that team are spawned.  non-spawn regions indicate the team
	// base volume.  spawn and non-spawn regions are independent;
	// if you want identical or shared regions you must add the
	// shape twice, once with spawn true and once with spawn false.
	//
	// team bases should not overlap but no check is made.
	//
	// rogue team base regions are permitted.
	//
	// do not share the adopted shape with any other region.
	void				insert(TeamColor, RegionShape* adopted,
								bool isSpawnRegion);

	// set the flag safety position for a given team.  this is where
	// another team's flag gets put when dropped in the team base.  it
	// should be outside the base.  safety positions are 0,0,0 by
	// default.
	void				setSafety(TeamColor, const Vec3&);

	// remove all regions for all team or the given team and reset the
	// safety positions.
	void				clear();
	void				clear(TeamColor);

	// get a random position to insert a player of the given team.
	// returns false if there are no non-empty regions for the
	// given team.
	bool				spawn(Vec3&, TeamColor) const;

	// test if a point is inside a team base.  returns NoTeam if
	// not in any base, otherwise returns the team.
	TeamColor			isInside(const Vec3&) const;

	// get the safety position for a team base
	const Vec3&			getSafety(TeamColor) const;

private:
	RegionManagerBase();

private:
	RegionShape*		baseRegion[NumTeams];
	RegionShape*		spawnRegion[NumTeams];
	Vec3				safety[NumTeams];

	static RegionManagerBase*	mgr;
};

#endif
// ex: shiftwidth=4 tabstop=4
