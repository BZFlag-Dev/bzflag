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

#ifndef BZF_COLLISION_DETECTOR_H
#define BZF_COLLISION_DETECTOR_H

#include "mathr.h"

class Plane;
class ContactSurface;
class TransformableShape;

class CollisionDetector {
public:
	enum Type {
		Separate,
		Contacting,
		Intersecting
	};

	CollisionDetector() { }
	virtual ~CollisionDetector() { }

	// check if two bodies are colliding.  the objects are intersecting
	// if the distance between them is less than intersectingTolerance
	// and contacting if less than contactingTolerance, otherwise they're
	// separate.
	virtual Type		compare(Real intersectingTolerance,
								Real contactingTolerance,
								const TransformableShape* a,
								const TransformableShape* b) const = 0;

	// like compare() above except collision surfaces for each body and
	// a separating plane passing through a are returned if the bodies
	// are Contacting.  the plane is also returned if the bodies are
	// Separated.  the client must delete the surfaces (which are set
	// to NULL if not Contacting).  separatingPlane is in world space
	// and the surfaces are in their respective body's space.
	virtual Type		compare(Plane& separatingPlane,
								ContactSurface** aSurface,
								ContactSurface** bSurface,
								Real intersectingTolerance,
								Real contactingTolerance,
								const TransformableShape* a,
								const TransformableShape* b) const = 0;
};

#endif
