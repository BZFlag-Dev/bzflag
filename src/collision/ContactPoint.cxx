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

#include "ContactPoint.h"
#include "Body.h"

//
// ContactPoint
//

void					ContactPoint::setVertexFace(const Vec3& n)
{
	isEdgeEdge = false;
	normal     = n;
normal2    = normal;
normal2.negate();
}

void					ContactPoint::setEdgeEdge(
								const Vec3& a, const Vec3& b,
								const Vec3& bOutwards)
{
	// set flag and vectors
	isEdgeEdge = true;
	aEdge	   = a;
	bEdge	   = b;
	normal     = aEdge % bEdge;
	normal.normalize();

	// if normal is pointing the wrong way then flip it and flip
	// one of the edges to remain constistent.
	if (normal * bOutwards < 0.0f) {
		normal.negate();
		aEdge.negate();
	}
normal2    = normal;
normal2.negate();
}

Real					ContactPoint::getNormalVelocity() const
{
	Vec3 va, vb;
	a->getPointVelocity(va, point);
	b->getPointVelocity(vb, point);
	return (va - vb) * normal;
}

void					ContactPoint::getNormalDerivative(Vec3& nDot) const
{
	if (isEdgeEdge) {
		// derivative of (aEdge % bEdge) / length(aEdge % bEdge)

		// compute aEdge % bEdge and its length, then normalize the vector
		Vec3 u        = aEdge % bEdge;
		Real length_1 = R_(1.0) / u.length();
		u            *= length_1;

		// compute derivative of aEdge % bEdge
		Vec3 uDot = (a->getOmega() % aEdge) % bEdge +
							aEdge % (b->getOmega() % bEdge);

		// compute nDot
		nDot      = length_1 * (uDot - (u * uDot) * u);
/*
Vec3 nDot2 = length_1 * (uDot - (uDot % u) % u);
//fprintf(stderr, "c %.11f %.11f %.11f\n", nDot[0], nDot[1], nDot[2]);
//fprintf(stderr, "b %.11f %.11f %.11f\n", nDot2[0], nDot2[1], nDot2[2]);
nDot = nDot2;
*/
	}
	else {
		// normal is attached to b
		nDot = b->getOmega() % normal;
	}
}
