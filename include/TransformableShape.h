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

#ifndef BZF_TRANSFORMABLE_SHAPE_H
#define BZF_TRANSFORMABLE_SHAPE_H

#include "Shape.h"

class TransformableShape : public Shape {
public:
	TransformableShape();
	virtual ~TransformableShape();

	// get the transform and its inverse
	virtual const Matrix&	getTransform() const = 0;
	virtual const Matrix&	getTransposeTransform() const = 0;
	virtual const Matrix&	getInverseTransform() const = 0;
	virtual const Matrix&	getInverseTransposeTransform() const = 0;

	// Shape overrides
	virtual Real		getVolume() const;
	virtual void		getInertia(Matrix&) const;
	virtual bool		isInside(const Vec3&) const;
	virtual bool		intersect(const Ray&) const;
	virtual bool		intersect(IntersectionPoint&, const Ray&) const;
	virtual void		getRandomPoint(Vec3&) const;
	virtual void		getSupportPoint(
								SupportPoint& supportPoint,
								const Vec3& vector) const;
	virtual ContactSurface*
						getCollision(const ContactSimplex&,
								const Plane& separationPlane,
								Real epsilon) const;
	virtual void		getDumpPoints(std::vector<Vec3>&) const;

protected:
	virtual Shape*		getShape() const = 0;
};

#endif
// ex: shiftwidth=4 tabstop=4
