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

#include "TransformableShape.h"
#include "ContactSurface.h"

//
// TransformableShape
//

TransformableShape::TransformableShape()
{
	// do nothing
}

TransformableShape::~TransformableShape()
{
	// do nothing
}

Real					TransformableShape::getVolume() const
{
	const Matrix& xform = getTransform();
	Real x = xform[0] * xform[0] + xform[4] * xform[4] + xform[8]  * xform[8];
	Real y = xform[1] * xform[1] + xform[5] * xform[5] + xform[9]  * xform[9];
	Real z = xform[2] * xform[2] + xform[6] * xform[6] + xform[10] * xform[10];
	return getShape()->getVolume() * x * y * z;
}

void					TransformableShape::getInertia(Matrix& /*m*/) const
{
	// FIXME
	assert(0 && "not implemented");
}

bool					TransformableShape::isInside(const Vec3& p) const
{
	Vec3 p2 = p;
	p2.xformPoint(getInverseTransform());
	return getShape()->isInside(p2);
}

bool					TransformableShape::intersect(const Ray& ray) const
{
	Vec3 o = ray.getOrigin();
	o.xformPoint(getInverseTransform());
	Ray rayLocal = Ray(o, getInverseTransform() * ray.getDirection());
	return getShape()->intersect(rayLocal);
}

bool					TransformableShape::intersect(
								IntersectionPoint& p, const Ray& ray) const
{
	Vec3 o = ray.getOrigin();
	o.xformPoint(getInverseTransform());
	Ray rayLocal = Ray(o, getInverseTransform() * ray.getDirection());
	if (!getShape()->intersect(p, rayLocal))
		return false;
	p.normal = getInverseTransposeTransform() * p.normal;
	return true;
}

void					TransformableShape::getRandomPoint(Vec3& p) const
{
	getShape()->getRandomPoint(p);
	p.xformPoint(getTransform());
}

void					TransformableShape::getSupportPoint(
								SupportPoint& point,
								const Vec3& vector) const
{
	// get the support point.
	// transform vector from world to local coordinate space.  the
	// vector transforms like a normal, which means using the
	// inverse transpose of the transform.  however, we're going
	// from world to local space and the transform goes from local
	// to world, so we need the inverse transform.  together we
	// need the inverse of the inverse of the transpose, which is
	// just the transpose.
	getShape()->getSupportPoint(point, getTransposeTransform() * vector);

	// and transform the point to world space
	point.point.xformPoint(getTransform());
}

ContactSurface*			TransformableShape::getCollision(
								const ContactSimplex& simplex,
								const Plane& plane,
								Real epsilon) const
{
// FIXME -- maybe pass transforms down to getShape()->getCollision()
// to avoid transforming when unnecessary.
	// transform plane to local space.  see comment in getSupportPoint()
	// for why we use the transpose of the transform.
	const Matrix& xform = getTransform();
	Plane localPlane(getTransposeTransform() * plane.getNormal(),
						plane.distance(Vec3(xform[12], xform[13], xform[14])));

	// transform simplex to local space
// FIXME -- allow modification of simplex to avoid copy
	ContactSimplex tSimplex = simplex;
	for (unsigned int i = 0; i < tSimplex.size(); ++i)
		tSimplex[i].point.xformPoint(getInverseTransform());

	// get the surface
	ContactSurface* s = getShape()->getCollision(tSimplex, localPlane, epsilon);

	// transform to world
	s->transform(getTransform());

	return s;
}

void					TransformableShape::getDumpPoints(
								std::vector<Vec3>& points) const
{
	getShape()->getDumpPoints(points);
	for (unsigned int i = 0; i < points.size(); ++i)
		points[i].xformPoint(getTransform());
}
