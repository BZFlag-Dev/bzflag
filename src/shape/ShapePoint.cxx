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

#include "ShapePoint.h"
#include "ContactSurfacePoint.h"

//
// ShapePoint
//

ShapePoint::ShapePoint()
{
	// do nothing
}

ShapePoint::~ShapePoint()
{
	// do nothing
}

Real					ShapePoint::getVolume() const
{
	return R_(0.0);
}

void					ShapePoint::getInertia(Matrix& i) const
{
	i.zero();
}

bool					ShapePoint::isInside(const Vec3& p) const
{
	return (p == v);
}

bool					ShapePoint::intersect(const Ray&) const
{
	return false;
}

bool					ShapePoint::intersect(
								IntersectionPoint&, const Ray&) const
{
	return false;
}

void					ShapePoint::getRandomPoint(Vec3& p) const
{
	p = v;
}

void					ShapePoint::getSupportPoint(
								SupportPoint& supportPoint,
								const Vec3&) const
{
	supportPoint.type  = 0;
	supportPoint.point = v;
}

ContactSurface*			ShapePoint::getCollision(
								const ContactSimplex& simplex,
								const Plane& separationPlane,
								Real /*epsilon*/) const
{
	switch (simplex.size()) {
		case 1:
			return new ContactSurfacePoint(v, separationPlane.getNormal());
	}

	assert(0 && "bad simplex dimension");
	return NULL;
}

void					ShapePoint::getDumpPoints(
								std::vector<Vec3>& points) const
{
	points.push_back(v);
}
