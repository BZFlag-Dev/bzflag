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

#include <math.h>
#include "BaseBuilding.h"
#include "Intersect.h"

BzfString				BaseBuilding::typeName("BaseBuilding");

BaseBuilding::BaseBuilding(const float *p, float rotation,
		const float *size, int _team) :
				Obstacle(p, rotation, size[0], size[1], size[2]),
				team(_team)
{
	if(pos[2] != 0) {
		height = 1.0;
	} else {
		height = 0.0;
	}
}

BaseBuilding::~BaseBuilding()
{
	// do nothing
}

BzfString				BaseBuilding::getType() const
{
	return typeName;
}

BzfString				BaseBuilding::getClassName()
{
	return typeName;
}

float					BaseBuilding::intersect(const Ray &r) const
{
	return Intersect::timeRayHitsBlock(r, getPosition(), getRotation(),
						  getWidth(), getBreadth(), getHeight());
}

void					BaseBuilding::getNormal(const float *p, float *n) const
{
	Intersect::getNormalRect(p, getPosition(), getRotation(),
								getWidth(), getBreadth(), n);
}

bool					BaseBuilding::isInside(
								const float *p, float radius) const
{
	return p[2] < getPosition()[2] + getHeight() &&
								Intersect::testRectCircle(
										getPosition(), getRotation(),
      										getWidth(), getBreadth(), p, radius) &&
    								p[2] > getPosition()[2];
}

bool					BaseBuilding::isInside(
								const float *p, float angle,
								float dx, float dy) const
{
	return p[2] < getPosition()[2] + getHeight() &&
								Intersect::testRectRect(
										getPosition(), getRotation(),
      										getWidth(), getBreadth(),
										p, angle, dx, dy) &&
    								p[2] >= getPosition()[2];
}

bool					BaseBuilding::isCrossing(const float *p, float angle,
								float dx, float dy,
								float *plane) const
{
	// if not inside or contained, then not crossing
	if (!isInside(p, angle, dx, dy) ||
      Intersect::testRectInRect(getPosition(), getRotation(),
		getWidth(), getBreadth(), p, angle, dx, dy))
		return false;
	if(!plane) return true;

	// it's crossing -- choose which wall is being crossed (this
	// is a guestimate, should really do a careful test). Just
	// see which wall the point is closest to
	const float *p2 = getPosition();
	const float a2  = getRotation();
	const float c   = cosf(-a2), s = sinf(-a2);
	const float x   = c * (p[0] - p2[0]) - s * (p[1] - p2[1]);
	const float y   = c * (p[1] - p2[1]) - s * (p[0] - p2[0]);
	float pw[2];
	if(fabsf(fabsf(x) - getWidth()) < fabsf(fabsf(y) - getBreadth())) {
		plane[0] = ((x < 0.0) ? -cosf(a2) : cosf(a2));
		plane[1] = ((x < 0.0) ? -sinf(a2) : sinf(a2));
		pw[0] = p2[0] + getWidth() * plane[0];
		pw[1] = p2[1] + getWidth() * plane[1];
	} else {
		plane[0] = ((y < 0.0) ? sinf(a2) : -sinf(a2));
		plane[1] = ((y < 0.0) ? cosf(a2) : -cosf(a2));
		pw[0] = p2[0] + getBreadth() * plane[0];
		pw[1] = p2[1] + getBreadth() * plane[1];
	}

	// now finish off plane equation
	plane[2] = 0.0;
	plane[3] = -(plane[0] * pw[0] + plane[1] * pw[1]);
	return true;
}

bool					BaseBuilding::getHitNormal(
								const float *pos1, float azimuth1,
								const float *pos2, float azimuth2,
								float halfWidth, float halfBreadth,
								float *normal) const
{
	return Obstacle::getHitNormal(pos1, azimuth1,
						pos2, azimuth2, halfWidth, halfBreadth,
      						getPosition(), getRotation(), getWidth(), getBreadth(),
						getHeight(), normal) >= 0.0f;
}

void					BaseBuilding::getCorner(int index, float *pos) const
{
	const float *base = getPosition();
	const float c = cosf(getRotation());
	const float s = sinf(getRotation());
	const float w = getWidth();
	const float b = getBreadth();
	switch(index & 3) {
		case 0:
			pos[0] = base[0] + c * w - s * b;
			pos[1] = base[1] + s * w + c * b;
			break;
		case 1:
			pos[0] = base[0] - c * w - s * b;
			pos[1] = base[1] - s * w + c * b;
			break;
		case 2:
			pos[0] = base[0] - c * w + s * b;
			pos[1] = base[1] - s * w - c * b;
			break;
		case 3:
			pos[0] = base[0] + c * w + s * b;
			pos[1] = base[1] + s * w - c * b;
			break;
	}
	pos[2] = base[2];
	if(index >= 4) pos[2] += getHeight();
}

const int				BaseBuilding::getTeam() const
{
	return team;
}
