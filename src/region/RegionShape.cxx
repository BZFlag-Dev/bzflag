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

#include "RegionShape.h"
#include "TransformableShape.h"

//
// RegionShapePrimitive
//

RegionShapePrimitive::RegionShapePrimitive(TransformableShape* adopted) :
								shape(adopted)
{
	// do nothing
}

RegionShapePrimitive::~RegionShapePrimitive()
{
	delete shape;
}

TransformableShape*		RegionShapePrimitive::getShape() const
{
	return shape;
}

Real					RegionShapePrimitive::getVolume() const
{
	return shape->getVolume();
}

bool					RegionShapePrimitive::isInside(const Vec3& p) const
{
	return shape->isInside(p);
}

void					RegionShapePrimitive::getRandomPoint(Vec3& p) const
{
	shape->getRandomPoint(p);
}


//
// RegionShapeBExpr
//

RegionShapeBExpr::RegionShapeBExpr(RegionShape* a_, RegionShape* b_) :
								aShape(a_), bShape(b_)
{
	// do nothing
}

RegionShapeBExpr::~RegionShapeBExpr()
{
	delete aShape;
	delete bShape;
}

Real					RegionShapeBExpr::estimateIntersectionVolume() const
{
	// sample a at random to compute an estimate of how much a and b overlap
	const unsigned int n = 100;
	unsigned int c = 0;
	Vec3 p;
	for (unsigned int i = 0; i < n; ++i) {
		aShape->getRandomPoint(p);
		if (bShape->isInside(p))
			++c;
	}
	return aShape->getVolume() * static_cast<Real>(c) / static_cast<Real>(n);
}


//
// RegionShapeUnion
//

RegionShapeUnion::RegionShapeUnion(RegionShape* a_, RegionShape* b_) :
								RegionShapeBExpr(a_, b_)
{
	// get the volumes of the children
	Real aVolume = a()->getVolume();
	Real bVolume = b()->getVolume();

	// estimate volume of union
	abVolume = aVolume + bVolume - estimateIntersectionVolume();

	// compute ratio of a's volume to the total volume (doubly counting
	// the overlapping volume)
	aFraction = aVolume / (aVolume + bVolume);
}

RegionShapeUnion::~RegionShapeUnion()
{
	// do nothing
}

Real					RegionShapeUnion::getVolume() const
{
	// add the volumes
	return abVolume;
}

bool					RegionShapeUnion::isInside(const Vec3& p) const
{
	// point is inside if inside either a or b
	return a()->isInside(p) || b()->isInside(p);
}

void					RegionShapeUnion::getRandomPoint(Vec3& p) const
{
	// pick a or b at random weighted by their respective volumes.
	// we'll end up picking points in the intersection twice as
	// often as we should so discard half of those points at random.
	do {
		if (bzfrand() < aFraction)
			a()->getRandomPoint(p);
		else
			b()->getRandomPoint(p);
	} while (bzfrand() < 0.5 && a()->isInside(p) && b()->isInside(p));
}


//
// RegionShapeDifference
//

RegionShapeDifference::RegionShapeDifference(RegionShape* a_, RegionShape* b_) :
								RegionShapeBExpr(a_, b_)
{
	// estimate volume of difference
	abVolume = a()->getVolume() - estimateIntersectionVolume();
}

RegionShapeDifference::~RegionShapeDifference()
{
	// do nothing
}

Real					RegionShapeDifference::getVolume() const
{
	// add the volumes.  this overestimates if the shapes intersect.
	return abVolume;
}

bool					RegionShapeDifference::isInside(const Vec3& p) const
{
	// point is inside if inside a but not inside b
	return a()->isInside(p) && !b()->isInside(p);
}

void					RegionShapeDifference::getRandomPoint(Vec3& p) const
{
	// pick a random point in a.  if inside b then try again.
	do {
		a()->getRandomPoint(p);
	} while (b()->isInside(p));
}


//
// RegionShapeIntersection
//

RegionShapeIntersection::RegionShapeIntersection(
								RegionShape* a_, RegionShape* b_) :
								RegionShapeBExpr(a_, b_)
{
	// estimate volume of intersection
	abVolume = estimateIntersectionVolume();
}

RegionShapeIntersection::~RegionShapeIntersection()
{
	// do nothing
}

Real					RegionShapeIntersection::getVolume() const
{
	// add the volumes.  this overestimates if the shapes intersect.
	return abVolume;
}

bool					RegionShapeIntersection::isInside(const Vec3& p) const
{
	// point is inside if inside both a and b
	return a()->isInside(p) && b()->isInside(p);
}

void					RegionShapeIntersection::getRandomPoint(Vec3& p) const
{
	// pick a random point in a.  if not inside b then try again.
	do {
		a()->getRandomPoint(p);
	} while (!b()->isInside(p));
}
// ex: shiftwidth=4 tabstop=4
