/* bzflag
 * Copyright 1993-1999, Chris Schoeneman
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/*
 * Utilities for doing intersection calculations.
 */

#ifndef	BZF_INTERSECT_H
#define	BZF_INTERSECT_H

#include "common.h"
#include "Ray.h"

// True iff circles (in z = const plane) intersect
boolean			testCircleCircle(const float* o1, float r1,
					const float* o2, float r2);

// True iff spheres intersect
boolean			testSphereSphere(const float* o1, float r1,
					const float* o2, float r2);

// returns normal to 2d rect (size 2dx x 2dy) by point p
void			getNormalRect(const float* p, const float* boxPos,
					float boxAngle, float dx,
					float dy, float* n);

// True iff 2d rect (size 2dx x 2dy) intersects circle (in z = const plane)
boolean			testRectCircle(const float* boxPos, float boxAngle,
					float dx, float dy,
					const float* circPos, float circRadius);

// ray r1 started at time t1 minus ray r2 started at time t2
Ray			rayMinusRay(const Ray& r1, float t1,
					const Ray& r2, float t2);

// return t at which ray passes closest to origin
float			rayClosestToOrigin(const Ray& r);

// return t at which ray passes through sphere at origin of given radius
float			rayAtDistanceFromOrigin(const Ray& r, float radius);

// return t at which ray intersects box (size 2dx x 2dy x 2dz)
// (-1 if never, 0 if starts inside).
float			timeRayHitsBlock(const Ray& r, const float* boxPos,
					float boxAngle, float dx,
					float dy, float dz);

// True iff rectangles intersect (in z = const plane)
boolean			testRectRect(const float* p1, float angle1,
					float dx1, float dy1,
					const float* p2, float angle2,
					float dx2, float dy2);

// True iff first rectangle contains second intersect (in z = const plane)
boolean			testRectInRect(const float* bigPos, float angle1,
					float dx1, float dy1,
					const float* smallPos, float angle2,
					float dx2, float dy2);

// return t at which ray intersects 2d rect (size 2dx x 2dy) and side
// of intersection.  0,1,2,3 for east, north, west, south;  -1 if never;
// -2 if starts inside.
float			timeAndSideRayHitsOrigRect(
					const float* rayOrigin,
					const float* rayDir,
					float dx, float dy, int& side);
float			timeAndSideRayHitsRect(const Ray& r,
					const float* boxPos, float boxAngle,
					float dx, float dy, int& side);


#endif // BZF_INTERSECT_H
