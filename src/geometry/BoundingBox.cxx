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

#include "BoundingBox.h"

//
// BoundingBox
//

BoundingBox::BoundingBox()
{
	set(R_(0.0), R_(0.0), R_(0.0), R_(0.0), R_(0.0), R_(0.0));
}

BoundingBox::BoundingBox(const Real* minXYZ, const Real* maxXYZ)
{
	set(minXYZ[0], minXYZ[1], minXYZ[2], maxXYZ[0], maxXYZ[1], maxXYZ[2]);
}

BoundingBox::BoundingBox(Real xMin, Real yMin, Real zMin,
								Real xMax, Real yMax, Real zMax)
{
	set(xMin, yMin, zMin, xMax, yMax, zMax);
}

BoundingBox::~BoundingBox()
{
	// do nothing
}

void					BoundingBox::set(
								const Real* minXYZ, const Real* maxXYZ)
{
	set(minXYZ[0], minXYZ[1], minXYZ[2], maxXYZ[0], maxXYZ[1], maxXYZ[2]);
}

void					BoundingBox::set(
								Real xMin, Real yMin, Real zMin,
								Real xMax, Real yMax, Real zMax)
{
	// center
	p[0][0] = R_(0.5) * (xMin + xMax);
	p[0][1] = R_(0.5) * (yMin + yMax);
	p[0][2] = R_(0.5) * (zMin + zMax);

	// +x face center
	p[1][0] = xMax;
	p[1][1] = p[0][1];
	p[1][2] = p[0][2];

	// +y face center
	p[2][0] = p[0][0];
	p[2][1] = yMax;
	p[2][2] = p[0][2];

	// +z face center
	p[3][0] = p[0][0];
	p[3][1] = p[0][1];
	p[3][2] = zMax;
}

void					BoundingBox::transform(const Matrix& m)
{
	p[0].xformPoint(m);
	p[1].xformPoint(m);
	p[2].xformPoint(m);
	p[3].xformPoint(m);
}

void					BoundingBox::get(Real* minXYZ, Real* maxXYZ) const
{
	// compute largest x, y, z
	Real x = (p[1][0] - p[0][0] > 0.0f) ? p[1][0] - p[0][0] : p[0][0] - p[1][0];
	Real y = (p[1][1] - p[0][1] > 0.0f) ? p[1][1] - p[0][1] : p[0][1] - p[1][1];
	Real z = (p[1][2] - p[0][2] > 0.0f) ? p[1][2] - p[0][2] : p[0][2] - p[1][2];
	x += (p[2][0] - p[0][0] > 0.0f) ? p[2][0] - p[0][0] : p[0][0] - p[2][0];
	y += (p[2][1] - p[0][1] > 0.0f) ? p[2][1] - p[0][1] : p[0][1] - p[2][1];
	z += (p[2][2] - p[0][2] > 0.0f) ? p[2][2] - p[0][2] : p[0][2] - p[2][2];
	x += (p[3][0] - p[0][0] > 0.0f) ? p[3][0] - p[0][0] : p[0][0] - p[3][0];
	y += (p[3][1] - p[0][1] > 0.0f) ? p[3][1] - p[0][1] : p[0][1] - p[3][1];
	z += (p[3][2] - p[0][2] > 0.0f) ? p[3][2] - p[0][2] : p[0][2] - p[3][2];

	// compute extents
	minXYZ[0] = p[0][0] - x;
	minXYZ[1] = p[0][1] - y;
	minXYZ[2] = p[0][2] - z;
	maxXYZ[0] = p[0][0] + x;
	maxXYZ[1] = p[0][1] + y;
	maxXYZ[2] = p[0][2] + z;
}
