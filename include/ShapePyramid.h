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

#ifndef BZF_SHAPE_PYRAMID_H
#define BZF_SHAPE_PYRAMID_H

#include "Shape.h"

class ShapePyramid : public Shape {
public:
	// pyramid base is axis-aligned in the xy plane centered on the
	// z-axis and the tip is z units from the base.  the base is at
	// -z/4 and the tip is at 3z/4.  this puts the center of mass at
	// the origin.  x,y are the min/max extents of the base (i.e.
	// *half* the width and length).  z can be positive for an
	// upward pointing pyramid or negative for a downward pointing
	// pyramid.
	ShapePyramid(Real x, Real y, Real z);
	~ShapePyramid();

	// accessors

	Real				getX() const { return x; }
	Real				getY() const { return y; }
	Real				getZ() const { return z; }

	// Shape overrides
	virtual Real		getVolume() const;
	virtual void		getInertia(Matrix&) const;
	virtual bool		isInside(const Vec3&) const;
	virtual bool		intersect(const Ray&) const;
	virtual bool		intersect(IntersectionPoint&, const Ray&) const;
	virtual void		getRandomPoint(Vec3&) const;
	virtual void		getSupportPoint(SupportPoint&, const Vec3&) const;
	virtual ContactSurface*
						getCollision(const ContactSimplex&,
								const Plane& separationPlane,
								Real epsilon) const;
	virtual void		getDumpPoints(std::vector<Vec3>&) const;

private:
	const unsigned int*	getAdjacentFaces(unsigned int) const;
	const unsigned int*	getAdjacentFaces(unsigned int, unsigned int) const;
	const unsigned int*	getAdjacentEdges(unsigned int index) const;
	unsigned int		getParallelEdge(const Plane& plane,
								const unsigned int* edgePairList,
								unsigned int nPairs) const;
	unsigned int		getParallelFace(const Plane& plane,
								const unsigned int* faceList,
								unsigned int n) const;

private:
	Real				x, y, z;
	Vec3				vertex[5];
	Plane				plane[5];
};

#endif
