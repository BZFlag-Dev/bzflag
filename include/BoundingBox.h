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

#ifndef BZF_BOUNDING_BOX_H
#define BZF_BOUNDING_BOX_H

#include "math3D.h"

class BoundingBox {
public:
	BoundingBox();
	BoundingBox(const Real* minXYZ, const Real* maxXYZ);
	BoundingBox(Real xMin, Real yMin, Real zMin,
							Real xMax, Real yMax, Real zMax);
	~BoundingBox();

	// set a box's axis-aligned extents
	void				set(const Real* minXYZ, const Real* maxXYZ);
	void				set(Real xMin, Real yMin, Real zMin,
							Real xMax, Real yMax, Real zMax);

	// transform a bounding box
	void				transform(const Matrix&);

	// get the box's axis-aligned extents
	void				get(Real* minXYZ, Real* maxXYZ) const;

private:
	// p is: center, +x face center, +y face center, +z face center
	Vec3				p[4];
};

#endif
// ex: shiftwidth=4 tabstop=4
