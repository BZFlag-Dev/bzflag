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

#ifndef BZF_CONTACT_POINT_H
#define BZF_CONTACT_POINT_H

#include "math3D.h"

class Body;

// contact point
class ContactPoint {
public:
	// set the normal and reset isEdgeEdge.  n is assumed to be normalized.
	void				setVertexFace(const Vec3& n);

	// set isEdgeEdge, aEdge, bEdge, and normal.  normal is flipped if
	// necessary to point in the same general direction as bOutwards.
	void				setEdgeEdge(const Vec3& a, const Vec3& b,
								const Vec3& bOutwards);

	// compute the relative velocity in the normal direction
	Real				getNormalVelocity() const;

	// compute the time derivative of the normal (based on the
	// angular velocity of the bodies).
	void				getNormalDerivative(Vec3&) const;

public:
	Body*				a;				// 1st body
	Body*				b;				// 2nd body
	Vec3				point;			// contact point (attached to a)
	Vec3				normal;			// attached to and outward from b
// FIXME -- remove if unnecessary
Vec3				normal2;			// attached to and outward from a
Real				distance;

	bool				isEdgeEdge;		// true if contact is edge/edge type
	Vec3				aEdge;			// edge direction of a (iff isEdgeEdge)
	Vec3				bEdge;			// edge direction of b (iff isEdgeEdge)
};

// list of contact points
typedef std::vector<ContactPoint> ContactPoints;

#endif
