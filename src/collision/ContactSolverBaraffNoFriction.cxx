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

#include "ContactSolverBaraffNoFriction.h"
#include "Body.h"
#include "LinearEquationSolverGaussian.h"

//
// ContactSolverBaraffNoFriction
//   simultaneous solver as described by Baraff, SIGGRAPH '94 for
//   frictionless systems.
//

// FIXME -- must be settable by client
static const Real s_velocityTolerance = R_(1.0e-4);		// velocity tolerance

// FIXME -- coefficient of restitution should be based on bodies
static const Real restitution = R_(0.5);

ContactSolverBaraffNoFriction::ContactSolverBaraffNoFriction()
{
	solver = new LinearEquationSolverGaussian;
}

ContactSolverBaraffNoFriction::~ContactSolverBaraffNoFriction()
{
	delete solver;
}

void					ContactSolverBaraffNoFriction::applyImpulse(
								const ContactPoints& contacts)
{
fprintf(stderr, "impulse begin: %d\n", contacts.size());
	// apply impulses until no points are colliding
	bool collided = true;
	while (collided) {
		// assume no collisions
		collided = false;

		// check all contact points for collision
		for (ContactPoints::const_iterator index = contacts.begin();
								index != contacts.end(); ++index) {
			Real vn = index->getNormalVelocity();
Vec3 va, vb;
index->a->getPointVelocity(va, index->point);
index->b->getPointVelocity(vb, index->point);
fprintf(stderr, "  va: %.11g %.11g %.11g\n",
va[0], va[1], va[2]);
fprintf(stderr, "  vb: %.11g %.11g %.11g\n",
vb[0], vb[1], vb[2]);
fprintf(stderr, "  vn: %.11g\n", vn);
			if (vn < 0.5 * s_velocityTolerance) {
				// point is colliding
				collided = true;

				// compute impulse
				const ContactPoint& contact = *index;
				Vec3 da, db;
				contact.a->getEffectiveDirection(da,
								contact.normal, contact.point, contact.point);
				contact.b->getEffectiveDirection(db,
								contact.normal, contact.point, contact.point);
				Real n   = -(R_(1.0) + restitution) * vn + s_velocityTolerance;
				Real j   = n / (contact.normal * da + contact.normal * db);

				// apply impulse to both bodies
				if (j > R_(0.0)) {
					contact.a->applyImpulse( j, contact.normal, contact.point);
					contact.b->applyImpulse(-j, contact.normal, contact.point);
				}
			}
		}
	}
fprintf(stderr, "impulse end\n");
}

void					ContactSolverBaraffNoFriction::applyForce(
								const ContactPoints& contacts)
{
	const unsigned int n = contacts.size();

	// handle trivial case
	if (n == 0)
		return;

	// allocate appropriate space
	f.resize(n);
	b.resize(n);
	A.resize(n * n);

	// compute A matrix and b vector
	computeAMatrix(A, contacts);
	computeBVector(b, contacts);

VectorN A1 = A;
VectorN b1 = b;

	// solve for forces
	solve(f);

for (unsigned int i = 0; i < n; ++i) {
 Real s = 0.0;
 fprintf(stderr, "[");
 for (unsigned int j = 0; j < n; ++j) {
  fprintf(stderr, "%+8.4e ", A1[j + n * i]);
  s += A1[j + n * i] * f[j];
 }
 fprintf(stderr, "][%+8.4e]+[%+8.4e]=[%+8.4e]\n", f[i], b1[i], s + b1[i]);
}

	// apply forces
	unsigned int i = 0;
	for (ContactPoints::const_iterator index = contacts.begin();
						index != contacts.end(); ++i, ++index) {
		index->a->applyForce( f[i], index->normal, index->point);
		index->b->applyForce(-f[i], index->normal, index->point);
	}
}


void					ContactSolverBaraffNoFriction::computeAMatrix(
								VectorN& A,
								const ContactPoints& contacts) const
{
	// the A matrix contains the force dependent terms of the effect
	// of acceleration at contact point j on the acceleration of
	// contact point i.  A is symmetric for frictionless systems.

	unsigned int i, j;
	const unsigned int n = contacts.size();
	ContactPoints::const_iterator iIndex, jIndex;
	for (i = 0, iIndex = contacts.begin();
								iIndex != contacts.end(); ++i, ++iIndex) {
		const ContactPoint& iContact = *iIndex;
		for (j = i, jIndex = contacts.begin() + i;
								jIndex != contacts.end(); ++j, ++jIndex) {
			const ContactPoint& jContact = *jIndex;

			// term is 0 if contact points don't share a body
			if ((iContact.a != jContact.a) &&
				(iContact.a != jContact.b) &&
				(iContact.b != jContact.a) &&
				(iContact.b != jContact.b)) {
				A[n * i + j] = R_(0.0);
				A[n * j + i] = R_(0.0);
				continue;
			}
			// acceleration direction terms
			Vec3 aAcc, bAcc;
			if (iContact.a == jContact.a) {
				iContact.a->getEffectiveDirection(aAcc, jContact.normal,
								jContact.point, iContact.point);
			}
			else if (iContact.a == jContact.b) {
				iContact.a->getEffectiveDirection(aAcc, jContact.normal,
								jContact.point, iContact.point);
				aAcc.negate();
			}
			if (iContact.b == jContact.a) {
				iContact.b->getEffectiveDirection(bAcc, jContact.normal,
								jContact.point, iContact.point);
			}
			else if (iContact.b == jContact.b) {
				iContact.b->getEffectiveDirection(bAcc, jContact.normal,
								jContact.point, iContact.point);
				bAcc.negate();
			}

			// set the term in A
			A[n * i + j] = iContact.normal * (aAcc - bAcc);
			A[n * j + i] = A[n * i + j];
		}
	}
}

void					ContactSolverBaraffNoFriction::computeBVector(
								VectorN& b,
								const ContactPoints& contacts) const
{
	// the b vector contains the force independent terms of the effect
	// of acceleration at contact point j on the acceleration of
	// contact point i, plus external forces.
	unsigned int i;
	ContactPoints::const_iterator index;
	for (i = 0, index = contacts.begin();
								index != contacts.end(); ++i, ++index) {
		const ContactPoint& contact = *index;

		// get acceleration term
		Vec3 aAcc, bAcc;
		contact.a->getPointAcceleration(aAcc, contact.point);
		contact.b->getPointAcceleration(bAcc, contact.point);

		// get velocity term
		Vec3 aVel, bVel;
		contact.a->getPointVelocity(aVel, contact.point);
		contact.b->getPointVelocity(bVel, contact.point);

		// get the time derivative of the normal
		Vec3 normalDot;
		contact.getNormalDerivative(normalDot);

		// sum components
		b[i] = contact.normal * (aAcc - bAcc) +
					R_(2.0) * (normalDot * (aVel - bVel));

// FIXME -- clean up;  this adds a force to keep bodies 2*s_distanceTolerance
// apart.
/*
static const Real s_scale = R_(1.0);
static const Real s_tau   = R_(1.0e-1);
static const Real s_distanceTolerance = R_(2.5e-4);
Real d = (contact.distance / s_distanceTolerance) - R_(2.0);
d *= s_distanceTolerance;
b[i] -= s_scale * R_(2.0) * (contact.normal * aVel) / s_tau;
b[i] -= s_scale * d / (s_tau * s_tau);
*/
	}
}

static const Real fMin = R_(0.0);
static const Real aMin = R_(0.0);

void					ContactSolverBaraffNoFriction::solve(
								VectorN& f)
{
	const unsigned int n = f.size();

	// allocate space
	a.resize(n);
	fDelta.resize(n);
	aDelta.resize(n);
	subA.resize(n * n);
	pivot.resize(n);
	clamped.resize(n);

	// initialize vectors
	unsigned int i;
	for (i = 0; i < n; ++i) {
		f[i]     = R_(0.0);
		a[i]     = b[i];
		pivot[i] = Unknown;
	}

	// find first unsolved point
	for (i = 0; i < n && a[i] >= aMin; ++i) {
		// do nothing
	}

	// iterate
	while (i < n) {
		// drive a[i] to zero by adjusting f[i]
		driveToZero(f, i);

		// find next unsolved point
		for (i = 0; i < n && a[i] >= aMin; ++i) {
			// do nothing
		}
	}
}

void					ContactSolverBaraffNoFriction::driveToZero(
								VectorN& f, unsigned int i)
{
	const unsigned int n = f.size();

	// drive a[i] to zero by adjusting f[i]
	for (;;) {
		// compute direction of change in force
		fDirection(fDelta, i);

		// compute direction of change in acceleration
		for (unsigned int j = 0; j < n; ++j) {
			aDelta[j] = R_(0.0);
			for (unsigned int k = 0; k < n; ++k)
				aDelta[j] += A[j * n + k] * fDelta[k];
		}

		// compute the largest permitted step
		unsigned int limiter;
		Real s = maxStep(limiter, i, f);

		// check for failure
		assert(s >= R_(0.0));
		if (limiter == n) {
			// unable to drive a[i] to zero
			// FIXME -- this indicates an error
			assert(0 && "can't find pivot");
			f[i]     = R_(0.0);
			pivot[i] = NC;
			continue;
		}

		// step forward
		for (unsigned int j = 0; j < n; ++j) {
			switch (pivot[j]) {
				case Unknown:
					f[j] += s * fDelta[j];
					a[j] += s * aDelta[j];
					break;

				case C:
					if (j == limiter ||
						(f[j] += s * fDelta[j]) <= fMin) {
						f[j]     = fMin;
						pivot[j] = NC;
					}
					break;

				case NC:
					if (j == limiter ||
						(a[j] += s * aDelta[j]) <= aMin) {
						a[j]     = aMin;
						pivot[j] = C;
					}
					break;
			}
		}

		// see if we've succeeded in driving a[i] to zero
		if (pivot[limiter] == Unknown) {
			assert(limiter == i);
			a[i]     = aMin;
			pivot[i] = C;
			return;
		}
	}
}

void					ContactSolverBaraffNoFriction::fDirection(
								VectorN& fDelta,
								unsigned int i)
{
	// make clamped index table
	const unsigned int n = fDelta.size();
	unsigned int j, numClamped = 0;
	for (j = 0; j < n; ++j)
		if (pivot[j] == C) {
			assert(j != i);
    		clamped[numClamped++] = j;
		}

	// degenerate case
	if (numClamped == 0) {
		for (j = 0; j < n; j++)
			fDelta[j] = R_(0.0);
		fDelta[i] = R_(1.0);
		return;
	}

	// construct submatrix of A for clamped points
	for (j = 0; j < numClamped; ++j)
		for (unsigned int k = 0; k < numClamped; ++k)
			subA[j * numClamped + k] = A[clamped[j] * n + clamped[k]];

	// construct subvector of v for clamped points.  put it in aDelta which
	// is not otherwise needed at this point;  note that what we're putting
	// in there has nothing to do with aDelta, so we'll rename it.
	VectorN& v1 = aDelta;
	for (j = 0; j < numClamped; ++j)
		v1[j] = -A[clamped[j] * n + i];

	// solve for subvector of fDelta corresponding to clamped points
	solver->solve(fDelta, numClamped, subA, v1);

	// now transfer fDelta elements to correct locations.  we put our
	// solution in the first numClamped elements but they really belong
	// in the first i-1 elements, with positions determined by clamped[].
	// copy result to v1 for safekeeping, zero out fDelta, except for
	// element i which gets 1, then put the elements in their correct
	// locations.
	v1.swap(fDelta);
	for (j = 0; j < n; ++j)
		fDelta[j] = R_(0.0);
	fDelta[i] = R_(1.0);
	for (j = 0; j < numClamped; ++j)
		fDelta[clamped[j]] = v1[j];
}

Real					ContactSolverBaraffNoFriction::maxStep(
								unsigned int& j,
								unsigned int i,
								const VectorN& f)
{
	const unsigned int n = f.size();
	j         = n;
	Real step = R_(0.0);

	// limit step if a is increasing
	if (aDelta[i] > R_(0.0)) {
		j    = i;
		step = (aMin - a[i]) / aDelta[i];
	}

	// search contacts for a shorter step
	for (unsigned int k = 0; k < n; ++k) {
		switch (pivot[k]) {
			case C:
				if (fDelta[k] < R_(0.0)) {
					Real newStep = (fMin - f[k]) / fDelta[k];
					if (j == n || newStep < step) {
						if (newStep == R_(0.0)) {
							// point appears to be in the wrong set.
							// move it into NC and ignore the step
							pivot[k] = NC;
						}
						else {
							j    = k;
							step = newStep;
						}
					}
				}
				break;

			case NC:
				if (aDelta[k] < R_(0.0)) {
					Real newStep = (aMin - a[k]) / aDelta[k];
					if (j == n || newStep < step) {
						if (newStep == R_(0.0)) {
							// point appears to be in the wrong set.
							// move it into C and ignore the step
							pivot[k] = C;
						}
						else {
							j    = k;
							step = newStep;
						}
					}
				}
				break;

			default:
				// ignore
				break;
		}
	}

	return step;
}
