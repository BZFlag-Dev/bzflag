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

#include "RegionManagerBase.h"
#include "RegionShape.h"

//
// RegionManagerBase
//

RegionManagerBase*	RegionManagerBase::mgr = NULL;

RegionManagerBase::RegionManagerBase()
{
	for (unsigned int team = 0; team < NumTeams; ++team) {
		baseRegion[team]  = NULL;
		spawnRegion[team] = NULL;
	}
}

RegionManagerBase::~RegionManagerBase()
{
	clear();
	mgr = NULL;
}

RegionManagerBase*	RegionManagerBase::getInstance()
{
	if (mgr == NULL)
		mgr = new RegionManagerBase;
	return mgr;
}

void					RegionManagerBase::insert(
								TeamColor team,
								RegionShape* newShape,
								bool isSpawnRegion)
{
	// ignore if no shape
	if (newShape == NULL)
		return;

	// get the existing shape
	RegionShape*& shape = isSpawnRegion ? spawnRegion[team] : baseRegion[team];

	// combine the shapes
	if (shape == NULL)
		shape = newShape;
	else
		shape = new RegionShapeUnion(shape, newShape);
}

void					RegionManagerBase::setSafety(
								TeamColor team, const Vec3& p)
{
	safety[team] = p;
}

void					RegionManagerBase::clear()
{
	for (unsigned int team = 0; team < NumTeams; ++team)
		clear(static_cast<TeamColor>(team));
}

void					RegionManagerBase::clear(TeamColor team)
{
	delete baseRegion[team];
	delete spawnRegion[team];
	baseRegion[team]  = NULL;
	spawnRegion[team] = NULL;
	safety[team]      = Vec3();
}

bool					RegionManagerBase::spawn(Vec3& p, TeamColor team) const
{
	if (spawnRegion[team] == NULL)
		return false;
	spawnRegion[team]->getRandomPoint(p);
	return true;
}

TeamColor				RegionManagerBase::isInside(const Vec3& p) const
{
	for (unsigned int team = 0; team < NumTeams; ++team)
		if (baseRegion[team] != NULL && baseRegion[team]->isInside(p))
			return static_cast<TeamColor>(team);
	return NoTeam;
}

const Vec3&				RegionManagerBase::getSafety(
								TeamColor team) const
{
	return safety[team];
}
