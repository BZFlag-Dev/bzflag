/* bzflag
 * Copyright (c) 1993 - 2001 Tim Riker
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
#include "Matrix.h"
#include <string.h>

//
// BoundingBox
//

BoundingBox::BoundingBox()
{
	set(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
}

BoundingBox::BoundingBox(const float* minXYZ, const float* maxXYZ)
{
	set(minXYZ[0], minXYZ[1], minXYZ[2], maxXYZ[0], maxXYZ[1], maxXYZ[2]);
}

BoundingBox::BoundingBox(float xMin, float yMin, float zMin,
								float xMax, float yMax, float zMax)
{
	set(xMin, yMin, zMin, xMax, yMax, zMax);
}

BoundingBox::BoundingBox(const BoundingBox& box)
{
	memcpy(p, box.p, sizeof(p));
}

BoundingBox::~BoundingBox()
{
	// do nothing
}

BoundingBox&			BoundingBox::operator=(const BoundingBox& box)
{
	memcpy(p, box.p, sizeof(p));
	return *this;
}

void					BoundingBox::set(
								const float* minXYZ, const float* maxXYZ)
{
	set(minXYZ[0], minXYZ[1], minXYZ[2], maxXYZ[0], maxXYZ[1], maxXYZ[2]);
}

void					BoundingBox::set(
								float xMin, float yMin, float zMin,
								float xMax, float yMax, float zMax)
{
	// center
	p[0][0] = 0.5f * (xMin + xMax);
	p[0][1] = 0.5f * (yMin + yMax);
	p[0][2] = 0.5f * (zMin + zMax);

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
	m.transform3(p[0], p[0]);
	m.transform3(p[1], p[1]);
	m.transform3(p[2], p[2]);
	m.transform3(p[3], p[3]);
}

void					BoundingBox::get(float* minXYZ, float* maxXYZ) const
{
	// compute largest x, y, z
	float x = (p[1][0] - p[0][0] > 0.0f) ? p[1][0] - p[0][0] : p[0][0] - p[1][0];
	float y = (p[1][1] - p[0][1] > 0.0f) ? p[1][1] - p[0][1] : p[0][1] - p[1][1];
	float z = (p[1][2] - p[0][2] > 0.0f) ? p[1][2] - p[0][2] : p[0][2] - p[1][2];
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
