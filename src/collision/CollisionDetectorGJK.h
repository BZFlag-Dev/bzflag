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

#ifndef BZF_COLLISION_DETECTOR_GJK_H
#define BZF_COLLISION_DETECTOR_GJK_H

#include "CollisionDetector.h"
#include "Shape.h"

class Vec3;

class CollisionDetectorGJK : public CollisionDetector {
public:
	CollisionDetectorGJK();
	virtual ~CollisionDetectorGJK();

	// CollisionDetector overrides
	virtual Type		compare(Real intersectingTolerance,
								Real contactingTolerance,
								const Body* a, const Body* b) const;
	virtual Type		compare(Plane& separatingPlane,
								ContactSurface** aSurface,
								ContactSurface** bSurface,
								Real intersectingTolerance,
								Real contactingTolerance,
								const Body* a, const Body* b) const;

protected:
	Type				getPoints(Vec3& aPoint, Vec3& bPoint,
								ContactSimplex* aSimplex,
								ContactSimplex* bSimplex,
								const Body* a, const Body* b) const;

	Type				gjk(ContactSimplex& aSimplex,
								ContactSimplex& bSimplex,
								Real* aLambda, Real* bLambda,
								const Body* a, const Body* b,
								const Vec3& initialGuess);

private:
	Type				gjk(ContactSimplex& aSimplex,
								ContactSimplex& bSimplex,
								Real* lambda,
								const Body* a, const Body* b,
								const Vec3& initialGuess);
	bool				findClosest(Vec3& v, Real* lambda);
	void				computeV(Vec3& v,
								Real* lambda,
								unsigned int bitmask) const;
	void				computeDeterminant();
	bool				isDegenerate(const Vec3& w) const;
	bool				isValid(unsigned int s) const;

private:
	double				epsilon;
	unsigned int		bitmask;
	unsigned int		newBitmask;
	unsigned int		n;
	unsigned int		nBit;
	double				dot[4][4];
	double				determinant[16][4];
	double				W[4][3];
};

#endif
