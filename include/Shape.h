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

#ifndef BZF_SHAPE_H
#define BZF_SHAPE_H

// a Shape is a shape that can participate in collision detection
// and response.  adding a new shape involves implementing a
// subclass of Shape and, if necessary, implementing subclasses
// of ContactSurface along with the functions to intersect those
// new surfaces with all other surface types.

#include "common.h"
#include <vector>
#include "math3D.h"

class Ray;
class ContactSurface;

// the type of vertex classifications used by Shape
typedef int SupportPointType;

// the type of a support point.  the type is reserved for use by
// the Shape and should not be interpreted by clients.
class SupportPoint {
public:
	SupportPointType	type;
	Vec3				point;
};

// a point of intersection between a Shape and a Ray
class IntersectionPoint {
public:
	Real				t;
	Vec3				normal;
};

// a contact simplex (should never have more than 4 vertices)
typedef std::vector<SupportPoint> ContactSimplex;

class Shape {
public:
	Shape() { }
	virtual ~Shape() { }

	// get the volume of the shape
	virtual Real		getVolume() const = 0;

	// get the inertia tensor assuming unit density
	virtual void		getInertia(Matrix&) const = 0;

	// return true iff the point (in the shape's space) is inside the shape
	virtual bool		isInside(const Vec3&) const = 0;

	// intersect shape with a ray.  the first form just tests if there
	// is an intersection or not.  the second form returns true if
	// there is an intersection along with the point of intersection
	// and the normal to the shape at that point.  if there is no
	// intersection then it returns false and the contents of the
	// intersection point are unchanged.  for both methods, the ray
	// must be in the shape's space.  the normal is in the shape's
	// space and should have unit length.
	virtual bool		intersect(const Ray&) const = 0;
	virtual bool		intersect(IntersectionPoint&, const Ray&) const = 0;

	// choose a point in the volume at random.  ideally, every point
	// in the volume will have equal probability of being chosen.
	// the point is in the shape's space.
	virtual void		getRandomPoint(Vec3&) const = 0;

	// return the support point for the given vector.  the vector
	// must be in the shape's space.  note that the vector transforms
	// like a normal.  the returned support point is in the shape's
	// space and transforms like a point.  the support point's type
	// has meaning only to the type that returned it (i.e. it is
	// stored but not interpreted by the caller).
	virtual void		getSupportPoint(
								SupportPoint& supportPoint,
								const Vec3& vector) const = 0;

	// return the contact surface given a list of support points
	// and the separating plane of the collision.  the plane is in
	// in local space.  epsilon is the maximum distance between a
	// point on the surface and the separating plane that is
	// considered to be on the surface.  the simplex should have
	// at most 3 vertices.
	virtual ContactSurface*
						getCollision(const ContactSimplex&,
								const Plane& separationPlane,
								Real epsilon) const = 0;

	// debugging -- return interesting points on the shape
	virtual void		getDumpPoints(std::vector<Vec3>&) const = 0;
};

#endif
// ex: shiftwidth=4 tabstop=4
