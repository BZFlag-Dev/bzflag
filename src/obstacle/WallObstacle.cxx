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

#include <math.h>
#include "WallObstacle.h"

std::string				WallObstacle::typeName("WallObstacle");

WallObstacle::WallObstacle(const float* p, float a, float b, float h) :
								Obstacle(p, a, 0.0, b, h)
{
	// compute normal
	plane[0] = cosf(a);
	plane[1] = sinf(a);
	plane[2] = 0.0;
	plane[3] = -(p[0] * plane[0] + p[1] * plane[1] + p[2] * plane[2]);
}

WallObstacle::~WallObstacle()
{
	// do nothing
}

std::string				WallObstacle::getType() const
{
	return typeName;
}

std::string				WallObstacle::getClassName() // const
{
	return typeName;
}

float					WallObstacle::intersect(const Ray& r) const
{
	const float dot = -(r.getDirection()[0] * plane[0] +
						r.getDirection()[1] * plane[1] +
						r.getDirection()[2] * plane[2]);
	if (dot == 0.0f)
		return -1.0f;
	float t = (r.getOrigin()[0] * plane[0] +
				r.getOrigin()[1] * plane[1] +
				r.getOrigin()[2] * plane[2] + plane[3]) / dot;
	return t;
}

void					WallObstacle::getNormal(const float*, float* n) const
{
	n[0] = plane[0];
	n[1] = plane[1];
	n[2] = plane[2];
}

bool					WallObstacle::isInside(const float* p, float r) const
{
	return p[0] * plane[0] + p[1] * plane[1] + p[2] * plane[2] + plane[3] < r;
}

bool					WallObstacle::isInside(const float* p, float angle,
								float halfWidth, float halfBreadth) const
{
	const float xWidth = cosf(angle);
	const float yWidth = sinf(angle);
	const float xBreadth = -yWidth;
	const float yBreadth = xWidth;
	float corner[3];
	corner[2] = p[2];

	// check to see if any corner is inside negative half-space
	corner[0] = p[0] - xWidth * halfWidth - xBreadth * halfBreadth;
	corner[1] = p[1] - yWidth * halfWidth - yBreadth * halfBreadth;
	if (isInside(corner, 0.0)) return true;
	corner[0] = p[0] + xWidth * halfWidth - xBreadth * halfBreadth;
	corner[1] = p[1] + yWidth * halfWidth - yBreadth * halfBreadth;
	if (isInside(corner, 0.0)) return true;
	corner[0] = p[0] - xWidth * halfWidth + xBreadth * halfBreadth;
	corner[1] = p[1] - yWidth * halfWidth + yBreadth * halfBreadth;
	if (isInside(corner, 0.0)) return true;
	corner[0] = p[0] + xWidth * halfWidth + xBreadth * halfBreadth;
	corner[1] = p[1] + yWidth * halfWidth + yBreadth * halfBreadth;
	if (isInside(corner, 0.0f)) return true;

	return false;
}

bool					WallObstacle::getHitNormal(
								const float*, float,
								const float*, float,
								float, float,
								float* normal) const
{
	getNormal(NULL, normal);
	return true;
}
