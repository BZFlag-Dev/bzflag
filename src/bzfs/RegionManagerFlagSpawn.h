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

#ifndef BZF_REGION_MANAGER_FLAG_SPAWN_H
#define BZF_REGION_MANAGER_FLAG_SPAWN_H

#include "common.h"
#include <map>
#include <vector>
#include "math3D.h"
#include "global.h"

#define RGNMGR_FLAG_SPAWN	(RegionManagerFlagSpawn::getInstance())

class Region;

class RegionManagerFlagSpawn {
public:
	~RegionManagerFlagSpawn();

	static RegionManagerFlagSpawn*	getInstance();

	// add a flag spawn region
	void				insert(FlagId, Region*);

	// remove all regions
	void				clear();

	// get a random position to insert a given or random flag type.
	// spawnAny picks a flag a random, weighting its choice by the
	// volume of each region the flag may spawn within.  returns
	// false if there are no non-empty regions for the given flag
	// or any flag.
	bool				spawn(Vec3&, FlagId);
	bool				spawnAny(Vec3&, FlagId&);

private:
	RegionManagerFlagSpawn();

	typedef std::vector<float> Probability;
	typedef std::vector<Region*> Regions;
	class FlagInfo {
	public:
		Regions			regions;
		Probability		regionProbability;
		Real			volume;
	};

	void				computeProbability(FlagInfo&);
	void				computeProbability();

private:
	typedef std::vector<FlagInfo> FlagInfoList;

	bool				dirty;
	FlagInfoList		flags;
	Probability			flagProbability;

	static RegionManagerFlagSpawn*	mgr;
};

#endif
// ex: shiftwidth=4 tabstop=4
