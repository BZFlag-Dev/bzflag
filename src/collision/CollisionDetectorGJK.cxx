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

#include "common.h"
#include <algorithm>
#include "CollisionDetectorGJK.h"
#include "Body.h"
#include "math3D.h"
#include "ms_minmax.h"

//
// CollisionDetectorGJK
//

CollisionDetectorGJK::CollisionDetectorGJK() : epsilon(1.0e-5)
{
	// do nothing
}

CollisionDetectorGJK::~CollisionDetectorGJK()
{
	// do nothing
}

CollisionDetector::Type	CollisionDetectorGJK::compare(
								Real intersectingTolerance,
								Real contactingTolerance,
								const TransformableShape* a,
								const TransformableShape* b) const
{
	// find closest points.  return if intersecting.
	Vec3 aPoint, bPoint;
	ContactSimplex aSimplex, bSimplex;
	const Type type = getPoints(aPoint, bPoint, &aSimplex, &bSimplex, a, b);
	if (type == Intersecting)
		return Intersecting;

	// compute distance
	bPoint -= aPoint;
	const Real distance = bPoint.length();

	// check for intersection and contact
	if (distance < intersectingTolerance)
		return Intersecting;
	if (distance < contactingTolerance)
		return Contacting;
	return Separate;
}

CollisionDetector::Type	CollisionDetectorGJK::compare(
								Plane& plane,
								ContactSurface** aSurface,
								ContactSurface** bSurface,
								Real intersectingTolerance,
								Real contactingTolerance,
								const TransformableShape* a,
								const TransformableShape* b) const
{
	assert(aSurface != NULL);
	assert(bSurface != NULL);

	// initialize
	*aSurface = NULL;
	*bSurface = NULL;

	// find closest points.  return if intersecting.
	Vec3 aPoint, bPoint;
	ContactSimplex aSimplex, bSimplex;
	const Type type = getPoints(aPoint, bPoint, &aSimplex, &bSimplex, a, b);
	if (type == Intersecting)
		return type;

	// compute the vector from aPoint to bPoint and compute it's length
	bPoint -= aPoint;
	const Real distance = bPoint.length();

	// check if intersecting
	if (distance < intersectingTolerance)
		return Intersecting;

	// construct the separating plane.  it passes through aPoint
	// and the normal is along the ray from aPoint through bPoint.
	bPoint *= R_(1.0) / distance;
	plane.set(bPoint, aPoint);

	// check if contacting
	if (distance < contactingTolerance) {
		// now get the collision surfaces.  this allows the bodies to
		// detect a face or edge embedded in the separating plane.  note
		// that we negate the plane to get b's surface so it sees an
		// outward pointing plane and increase the tolerance by
		// distance to account for the known separation between b and
		// the plane.
		*aSurface = a->getCollision(aSimplex, plane, contactingTolerance);
		plane.negate();
		*bSurface = b->getCollision(bSimplex, plane,
								distance + contactingTolerance);
		plane.negate();

		return Contacting;
	}

	// separated
	return Separate;
}

CollisionDetector::Type	CollisionDetectorGJK::getPoints(
								Vec3& aPoint, Vec3& bPoint,
								ContactSimplex* aSimplex,
								ContactSimplex* bSimplex,
								const TransformableShape* a,
								const TransformableShape* b) const
{
	// we cache some stuff for convenience
	CollisionDetectorGJK* self = const_cast<CollisionDetectorGJK*>(this);

	assert(a != NULL);
	assert(b != NULL);
	assert(aSimplex != NULL);
	assert(bSimplex != NULL);
	// note -- we assume aPoint and bPoint are both zero

	// compute v0 = center(A) - center(B).  initial guess.
// FIXME -- provide method to pick arbitary point
	Vec3 aCenter, bCenter;
	aCenter.xformPoint(a->getTransform());
	bCenter.xformPoint(b->getTransform());
	Vec3 v = aCenter - bCenter;

	// find closest points.  return if intersecting.
	Real aLambda[4], bLambda[4];
	const Type type = self->gjk(*aSimplex, *bSimplex,
								aLambda, bLambda, a, b, v);
	if (type == Intersecting)
		return type;

	// we should never get a tetrahedron unless we're intersecting
	assert(aSimplex->size() != 4 && bSimplex->size() != 4);

	// compute closest points on A and B
	aPoint.zero();
	Real* lIndex = aLambda;
	for (ContactSimplex::const_iterator index = aSimplex->begin();
								index != aSimplex->end(); ++lIndex, ++index)
		aPoint += *lIndex * index->point;
	bPoint.zero();
	lIndex = bLambda;
	for (ContactSimplex::const_iterator index = bSimplex->begin();
								index != bSimplex->end(); ++lIndex, ++index)
		bPoint += *lIndex * index->point;

	return Separate;
}

CollisionDetector::Type	CollisionDetectorGJK::gjk(
								ContactSimplex& aSimplex,
								ContactSimplex& bSimplex,
								Real* aLambda, Real* bLambda,
								const TransformableShape* a,
								const TransformableShape* b,
								const Vec3& initialGuess)
{
	// put 4 vertices on each simplex.  (the next part of the
	// algorithm expects to treat aSimplex and bSimplex as
	// fixed length arrays.)
	aSimplex.resize(4);
	bSimplex.resize(4);

	// do GJK
	Real lambda[4];
	Type type = gjk(aSimplex, bSimplex, lambda, a, b, initialGuess);
	if (type == Intersecting)
		return type;

	// remove unset slots
	unsigned int ja = 0, jb = 0;
	for (unsigned int i = 0; i < 4; ++i) {
		if ((bitmask & (1 << i)) != 0) {
			// check for and discard duplicate points in a
			unsigned int k;
			for (k = 0; k < ja; ++k)
				if (aSimplex[k].point[0] == aSimplex[i].point[0] &&
					aSimplex[k].point[1] == aSimplex[i].point[1] &&
					aSimplex[k].point[2] == aSimplex[i].point[2])
					break;
			if (k != ja) {
				// duplicate
				aLambda[k] += lambda[i];
			}
			else {
				aSimplex[k] = aSimplex[i];
				aLambda[k]  = lambda[i];
				++ja;
			}

			// check for and discard duplicate points in b
			for (k = 0; k < jb; ++k)
				if (bSimplex[k].point[0] == bSimplex[i].point[0] &&
					bSimplex[k].point[1] == bSimplex[i].point[1] &&
					bSimplex[k].point[2] == bSimplex[i].point[2])
					break;
			if (k != jb) {
				// duplicate
				bLambda[k] += lambda[i];
			}
			else {
				bSimplex[k] = bSimplex[i];
				bLambda[k]  = lambda[i];
				++jb;
			}
		}
	}
	aSimplex.resize(ja);
	bSimplex.resize(jb);

	return type;
}

CollisionDetector::Type	CollisionDetectorGJK::gjk(
								ContactSimplex& aSimplex,
								ContactSimplex& bSimplex,
								Real* lambda,
								const TransformableShape* a,
								const TransformableShape* b,
								const Vec3& initialGuess)
{
	static const unsigned int firstEmptySlot[] = {
							0,	// 0000
							1,	// 0001
							0,	// 0010
							2,	// 0011
							0,	// 0100
							1,	// 0101
							0,	// 0110
							3,	// 0111
							0,	// 1000
							1,	// 1001
							0,	// 1010
							2,	// 1011
							0,	// 1100
							1,	// 1101
							0,	// 1110
							4	// 1111
						};

	// initial inputs
	Vec3 v(initialGuess);
	Real distance = v.length();
	Real mu = R_(0.0);
	bitmask = 0;
	newBitmask = 0;
// FIXME -- lambda not set if loop breaks on first iteration before findClosest

	static const Real maxAbsoluteError = R_(5e-6);
	static const Real maxRelativeError = R_(1e-7);
	while (bitmask < 15 && distance > maxAbsoluteError) {
		// find an unused slot in W
		n    = firstEmptySlot[bitmask];
		nBit = (1 << n);

		// map -v into the support mapping of A - B
		a->getSupportPoint(aSimplex[n], -v);
		b->getSupportPoint(bSimplex[n],  v);
		Vec3 w = aSimplex[n].point - bSimplex[n].point;

		// stop if objects are separate
		mu = std::max(mu, (v * w) / distance);
		if (distance - mu <= distance * maxRelativeError)
			break;

		// terminate with current solution if there's a degeneracy
		if (isDegenerate(w))
			break;

		// add w to simplex
		W[n][0] = w[0];
		W[n][1] = w[1];
		W[n][2] = w[2];
		newBitmask = bitmask | nBit;

		// find v closest to the origin in the simplex
		if (!findClosest(v, lambda))
			break;

		// update distance estimate
		Real newDistance = v.length();
		if (newBitmask != 1 && newDistance > distance) {
			// the distance should never increase (except on the first
			// step).  if it does then it's because of numerical error.
			// we'll assume the objects are intersecting in this case.
			return Intersecting;
		}
		distance = newDistance;
	}

	return (bitmask < 15 && distance > maxAbsoluteError) ?
								Separate : Intersecting;
}

bool					CollisionDetectorGJK::isDegenerate(
								const Vec3& w) const
{
	// if new w is in old W then terminate.  this only happens when
	// there's been a floating point rounding error.
	for (unsigned int i = 0; i < 4; ++i)
		if ((newBitmask & (1 << i)) != 0 &&
			w[0] == W[i][0] &&
			w[1] == W[i][1] &&
			w[2] == W[i][2])
			return true;
	return false;
}

bool					CollisionDetectorGJK::isValid(unsigned int s) const
{
	static const double tol1 = 0.0;
	static const double tol2 = 1.0e-5;

	for (unsigned int i = 0, bit = 1; i < 4; bit <<= 1, ++i) {
		if (newBitmask & bit) {
			if (s & bit) {
				if (determinant[s][i] <= tol1)
					return false;
			}
			else {
				if (determinant[s | bit][i] > tol2)
					return false;
			}
		}
	}
	return true;
}

bool					CollisionDetectorGJK::findClosest(
								Vec3& v, Real* lambda)
{
	computeDeterminant();

	for (unsigned int s = 1; s <= bitmask; ++s) {
		if ((s & bitmask) == s) {
			if (isValid(s | nBit)) {
				bitmask = s | nBit;
				computeV(v, lambda, bitmask);
				return true;
			}
		}
	}
	if (isValid(nBit)) {
		bitmask   = nBit;
		lambda[n] = static_cast<Real>(determinant[bitmask][n]);
		v[0]      = static_cast<Real>(W[n][0]);
		v[1]      = static_cast<Real>(W[n][1]);
		v[2]      = static_cast<Real>(W[n][2]);
		return true;
	}
	return false;
}

void					CollisionDetectorGJK::computeV(
								Vec3& v,
								Real* lambda,
								unsigned int bits) const
{
	double s = 0.0;
	double vt[3];
	vt[0] = 0.0;
	vt[1] = 0.0;
	vt[2] = 0.0;
	for (unsigned int i = 0; i < 4; ++i)
		if ((bits & (1 << i)) != 0) {
			s        += determinant[bits][i];
			lambda[i] = static_cast<Real>(determinant[bits][i]);
			vt[0]    += determinant[bits][i] * W[i][0];
			vt[1]    += determinant[bits][i] * W[i][1];
			vt[2]    += determinant[bits][i] * W[i][2];
		}
	Real s_1 = static_cast<Real>(1.0 / s);
	for (unsigned int i = 0; i < 4; ++i)
		if ((bits & (1 << i)) != 0)
			lambda[i] *= s_1;
	v[0] = static_cast<Real>(s_1 * vt[0]);
	v[1] = static_cast<Real>(s_1 * vt[1]);
	v[2] = static_cast<Real>(s_1 * vt[2]);
}

void					CollisionDetectorGJK::computeDeterminant()
{
	for (unsigned int i = 0; i < 4; ++i)
		if ((newBitmask & (1 << i)) != 0)
			dot[i][n] = dot[n][i] =
								W[i][0] * W[n][0] +
								W[i][1] * W[n][1] +
								W[i][2] * W[n][2];

	// compute determinants that use new element
	if (newBitmask & 1)
		determinant[1][0]  = 1.0;

	if (newBitmask & 2)
		determinant[2][1]  = 1.0;

	if ((newBitmask & 3) == (nBit | 3)) {
		determinant[3][0]  = dot[1][1] - dot[1][0];
		determinant[3][1]  = dot[0][0] - dot[0][1];
	}

	if (newBitmask & 4)
		determinant[4][2]  = 1.0;

	if ((newBitmask & 5) == (nBit | 5)) {
		determinant[5][0]  = dot[2][2] - dot[2][0];
		determinant[5][2]  = dot[0][0] - dot[0][2];
	}

	if ((newBitmask & 6) == (nBit | 6)) {
		determinant[6][1]  = dot[2][2] - dot[2][1];
		determinant[6][2]  = dot[1][1] - dot[1][2];
	}

	if ((newBitmask & 7) == (nBit | 7)) {
		determinant[7][0]  = determinant[6][1] * (dot[1][1] - dot[1][0]) +
							 determinant[6][2] * (dot[2][1] - dot[2][0]);
		determinant[7][1]  = determinant[5][0] * (dot[0][0] - dot[0][1]) +
							 determinant[5][2] * (dot[2][0] - dot[2][1]);
		determinant[7][2]  = determinant[3][0] * (dot[0][0] - dot[0][2]) +
							 determinant[3][1] * (dot[1][0] - dot[1][2]);
	}

	if (newBitmask & 8)
		determinant[8][3]  = 1.0;

	if ((newBitmask & 9) == (nBit | 9)) {
		determinant[9][0]  = dot[3][3] - dot[3][0];
		determinant[9][3]  = dot[0][0] - dot[0][3];
	}

	if ((newBitmask & 10) == (nBit | 10)) {
		determinant[10][1] = dot[3][3] - dot[3][1];
		determinant[10][3] = dot[1][1] - dot[1][3];
	}

	if ((newBitmask & 11) == (nBit | 11)) {
		determinant[11][0] = determinant[10][1] * (dot[1][1] - dot[1][0]) +
							 determinant[10][3] * (dot[3][1] - dot[3][0]);
		determinant[11][1] = determinant[9][0]  * (dot[0][0] - dot[0][1]) +
							 determinant[9][3]  * (dot[3][0] - dot[3][1]);
		determinant[11][3] = determinant[3][0]  * (dot[0][0] - dot[0][3]) +
							 determinant[3][1]  * (dot[1][0] - dot[1][3]);
	}

	if ((newBitmask & 12) == (nBit | 12)) {
		determinant[12][2] = dot[3][3] - dot[3][2];
		determinant[12][3] = dot[2][2] - dot[2][3];
	}

	if ((newBitmask & 13) == (nBit | 13)) {
		determinant[13][0] = determinant[12][2] * (dot[2][2] - dot[2][0]) +
							 determinant[12][3] * (dot[3][2] - dot[3][0]);
		determinant[13][2] = determinant[9][0]  * (dot[0][0] - dot[0][2]) +
							 determinant[9][3]  * (dot[3][0] - dot[3][2]);
		determinant[13][3] = determinant[5][0]  * (dot[0][0] - dot[0][3]) +
							 determinant[5][2]  * (dot[2][0] - dot[2][3]);
	}

	if ((newBitmask & 14) == (nBit | 14)) {
		determinant[14][1] = determinant[12][2] * (dot[2][2] - dot[2][1]) +
							 determinant[12][3] * (dot[3][2] - dot[3][1]);
		determinant[14][2] = determinant[10][1] * (dot[1][1] - dot[1][2]) +
							 determinant[10][3] * (dot[3][1] - dot[3][2]);
		determinant[14][3] = determinant[6][1]  * (dot[1][1] - dot[1][3]) +
							 determinant[6][2]  * (dot[2][1] - dot[2][3]);
	}

	if ((newBitmask & 15) == 15) {
		determinant[15][0] = determinant[14][1] * (dot[1][1] - dot[1][0]) +
							 determinant[14][2] * (dot[2][1] - dot[2][0]) +
							 determinant[14][3] * (dot[3][1] - dot[3][0]);
		determinant[15][1] = determinant[13][0] * (dot[0][0] - dot[0][1]) +
							 determinant[13][2] * (dot[2][0] - dot[2][1]) +
							 determinant[13][3] * (dot[3][0] - dot[3][1]);
		determinant[15][2] = determinant[11][0] * (dot[0][0] - dot[0][2]) +
							 determinant[11][1] * (dot[1][0] - dot[1][2]) +
							 determinant[11][3] * (dot[3][0] - dot[3][2]);
		determinant[15][3] = determinant[7][0]  * (dot[0][0] - dot[0][3]) +
							 determinant[7][1]  * (dot[1][0] - dot[1][3]) +
							 determinant[7][2]  * (dot[2][0] - dot[2][3]);
	}
}
// ex: shiftwidth=4 tabstop=4
