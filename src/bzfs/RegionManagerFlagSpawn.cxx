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

#include "common.h"
#include <algorithm>
#include "RegionManagerFlagSpawn.h"
#include "Region.h"
#include "RegionShape.h"

//
// RegionManagerFlagSpawn
//

RegionManagerFlagSpawn*	RegionManagerFlagSpawn::mgr = NULL;

RegionManagerFlagSpawn::RegionManagerFlagSpawn() : dirty(true)
{
	flags.resize(LastFlag + 1);
	flagProbability.resize(LastFlag + 1);
}

RegionManagerFlagSpawn::~RegionManagerFlagSpawn()
{
	clear();
	mgr = NULL;
}

RegionManagerFlagSpawn*	RegionManagerFlagSpawn::getInstance()
{
	if (mgr == NULL)
		mgr = new RegionManagerFlagSpawn;
	return mgr;
}

void					RegionManagerFlagSpawn::insert(
								FlagId id, Region* region)
{
	dirty = true;
	flags[id].regions.push_back(region);
}

void					RegionManagerFlagSpawn::clear()
{
	for (FlagInfoList::iterator i = flags.begin(); i != flags.end(); ++i)
		for (Regions::iterator j = i->regions.begin();
								j != i->regions.end(); ++j)
			delete *j;
	flags.clear();
	flags.resize(LastFlag + 1);
	dirty = true;
}

bool					RegionManagerFlagSpawn::spawn(Vec3& p, FlagId id)
{
	// get probabilities up to date
	if (dirty)
		computeProbability();

	// choose random region
	Probability::iterator b = flags[id].regionProbability.begin();
	Probability::iterator e = flags[id].regionProbability.end();
	Probability::iterator i = std::lower_bound(b, e, bzfrand());
	if (i == e)
		return false;

	// choose random point
	flags[id].regions[i - b]->getShape()->getRandomPoint(p);
	return true;
}

bool					RegionManagerFlagSpawn::spawnAny(Vec3& p, FlagId& id)
{
	// get probabilities up to date
	if (dirty)
		computeProbability();

	// choose random flag
	Probability::iterator b = flagProbability.begin();
	Probability::iterator e = flagProbability.end();
	Probability::iterator i = std::lower_bound(b, e, bzfrand());
	if (i == e)
		return false;

	// set flag id and choose random point
	id = static_cast<FlagId>(i - b);
	return spawn(p, id);
}

void					RegionManagerFlagSpawn::computeProbability(
								FlagInfo& info)
{
	// compute total volume of all regions (ignoring overlap)
	info.volume = R_(0.0);
	for (Regions::iterator j = info.regions.begin();
								j != info.regions.end(); ++j)
		info.volume += (*j)->getShape()->getVolume();

	// initialize probability function and invert the volume
	Real p, invVolume;
	if (info.volume == R_(0.0)) {
		// allow zero total volume.  it's useful only when there is exactly
		// one region, though.  if there are non-zero volumes then the zero
		// volumes have an infinitesimal chance of being chosen.  if there
		// are multiple zero volumes then only one of them will ever be used.
		p         = R_(1.0);
		invVolume = R_(1.0);
	}
	else {
		p         = R_(0.0);
		invVolume = R_(1.0) / info.volume;
	}

	// set probabilities
	info.regionProbability.clear();
	for (Regions::iterator j = info.regions.begin();
								j != info.regions.end(); ++j) {
		p += (*j)->getShape()->getVolume() * invVolume;
		info.regionProbability.push_back(p);
	}
}

void					RegionManagerFlagSpawn::computeProbability()
{
	// update each flag info
	Real invVolume = R_(0.0);
	for (FlagInfoList::iterator i = flags.begin(); i != flags.end(); ++i) {
		computeProbability(*i);
		invVolume += i->volume;
	}
	invVolume = R_(1.0) / invVolume;

	// now update the flag probabilities (probability is weighted by volume)
	Real p = R_(0.0);
	flagProbability.clear();
	for (FlagInfoList::iterator i = flags.begin(); i != flags.end(); ++i) {
		p += i->volume * invVolume;
		flagProbability.push_back(p);
	}

	dirty = false;
}
// ex: shiftwidth=4 tabstop=4
