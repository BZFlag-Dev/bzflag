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

#ifndef BZF_REGION_MANAGER_OBSTACLE_H
#define BZF_REGION_MANAGER_OBSTACLE_H

#include "common.h"
#include <vector>
#include "math3D.h"

#define RGNMGR_OBSTACLE (RegionManagerObstacle::getInstance())

class RegionPrimitive;
class IntersectionPoint;
class CollisionDetector;

class RegionManagerObstacle {
public:
	~RegionManagerObstacle();

	static RegionManagerObstacle*	getInstance();

	// add an obstacle region
	void				insert(RegionPrimitive*);

	// remove all regions
	void				clear();

	// return true iff point is inside any obstacle
	bool				isInside(const Vec3&) const;

	// return true iff point is inside or within distance of any obstacle.
	// if region is not NULL then that region is not checked.
	bool				isNear(const Vec3&, Real distance,
								const RegionPrimitive* region = NULL) const;

	// intersect ray with obstacles.  return true iff there's an
	// intersection.
	bool					intersect(const Ray&) const;

	// intersect ray with obstacles.  returns the closest intersection
	// and the region that was hit.  returns NULL if there was no hit.
	const RegionPrimitive*	intersect(IntersectionPoint&, const Ray&) const;

private:
	RegionManagerObstacle();

private:
	typedef std::vector<RegionPrimitive*> Regions;

	CollisionDetector*	collider;
	Regions				regions;

	static RegionManagerObstacle*	mgr;
};

#endif
// ex: shiftwidth=4 tabstop=4
