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

#include "RegionManagerObstacle.h"
#include "Region.h"
#include "RegionShape.h"
#include "ShapePoint.h"
#include "TransformedShape.h"
#include "CollisionDetectorGJK.h"

//
// RegionManagerObstacle
//

RegionManagerObstacle*	RegionManagerObstacle::mgr = NULL;

RegionManagerObstacle::RegionManagerObstacle()
{
	collider = new CollisionDetectorGJK;
}

RegionManagerObstacle::~RegionManagerObstacle()
{
	clear();
	delete collider;

	if (this == mgr)
		mgr = NULL;
}

RegionManagerObstacle*	RegionManagerObstacle::getInstance()
{
	if (mgr == NULL)
		mgr = new RegionManagerObstacle;
	return mgr;
}

void					RegionManagerObstacle::insert(RegionPrimitive* region)
{
	regions.push_back(region);
}

void					RegionManagerObstacle::clear()
{
	for (Regions::iterator i = regions.begin(); i != regions.end(); ++i)
		delete *i;
	regions.clear();
}

bool					RegionManagerObstacle::isInside(const Vec3& p) const
{
	for (Regions::const_iterator i = regions.begin(); i != regions.end(); ++i) {
		RegionShapePrimitive* shape = (*i)->getPrimitive();
		if (shape->isInside(p))
			return true;
	}
	return false;
}

bool					RegionManagerObstacle::isNear(
								const Vec3& p, Real distance,
								const RegionPrimitive* region) const
{
	// make a point shape to represent the point
	Matrix xform;
	xform.setTranslate(p[0], p[1], p[2]);
	TransformedShape pointShape(new ShapePoint, xform);

	// check each region
	for (Regions::const_iterator i = regions.begin(); i != regions.end(); ++i) {
		// skip region
		if (*i == region)
			continue;

		// get shape
		RegionShapePrimitive* shape = (*i)->getPrimitive();

		// check if inside
		if (shape->getShape()->isInside(p))
			return true;

		// check for contact with contact tolerance equal to distance.
		// this effectively tests for objects within distance.
		CollisionDetector::Type type = collider->compare(
								R_(0.0), distance,
								&pointShape,
								shape->getShape());
		if (type != CollisionDetector::Separate)
			return true;
	}
	return false;
}

bool					RegionManagerObstacle::intersect(const Ray& ray) const
{
	for (Regions::const_iterator i = regions.begin(); i != regions.end(); ++i) {
		RegionShapePrimitive* shape = (*i)->getPrimitive();
		if (shape->getShape()->intersect(ray))
			return true;
	}
	return false;
}

const RegionPrimitive*	RegionManagerObstacle::intersect(
								IntersectionPoint& p, const Ray& ray) const
{
	IntersectionPoint tmp;
	const RegionPrimitive* region = NULL;

	// check each region
	for (Regions::const_iterator i = regions.begin(); i != regions.end(); ++i) {
		RegionShapePrimitive* shape = (*i)->getPrimitive();
		if (shape->getShape()->intersect(tmp, ray) &&
			(region == NULL || tmp.t < p.t)) {
			p      = tmp;
			region = *i;
		}
	}

	return region;
}
// ex: shiftwidth=4 tabstop=4
