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

#ifndef BZF_BODY_H
#define BZF_BODY_H

#include "ODESolver.h"
#include "Shape.h"

class Body {
public:
	Body(Shape* adopted, Real inverseDensity);
	~Body();

	// manipulators

	void				setPosition(const Vec3&);
	void				setOrientation(const Quaternion&);
	void				setVelocity(const Vec3&);
	void				setAngularVelocity(const Vec3&);

	// accessors

	// get the shape
	Shape*				getShape() const;

	// body state
	Real				getInverseMass() const;
	const Matrix&		getInverseInertia() const;
	const Matrix&		getInverseWorldInertia() const;
	const Vec3&			getPosition() const;
	const Quaternion&	getOrientation() const;
	// FIXME -- other state
	const Vec3&			getOmega() const;
	const Matrix&		getTransform() const;
	const Matrix&		getInverseTransform() const;

const Vec3& getForce() const { return force; }
const Vec3& getTorque() const { return torque; }
const Vec3& getP() const { return P; }
const Vec3& getL() const { return L; }
const Vec3& getV() const { return v; }

	// set v to the world space velocity of the world space point p,
	// assuming p is on and moving with the body.
	void				getPointVelocity(Vec3& v, const Vec3& p) const;

	// set a to the world space acceleration of the world space point p,
	// assuming p is on and moving with the body, including the current
	// force and torque on the body.
	void				getPointAcceleration(Vec3& a, const Vec3& p) const;

	// get the effective direction of acceleration (velocity) at p2
	// when a force (impulse) is applied at p1 in direction n.  n
	// should be normalized.  p1 and p2 are in world space.
	void				getEffectiveDirection(Vec3& d, const Vec3& n,
								const Vec3& p1, const Vec3& p2) const;

	// set the total current force and torque on the body to the
	// external forces/torques at time t.
	void				setExternalForces(Real t);

	// apply an impulse/force at the given point (in world space)
	void				applyImpulse(Real magnitude,
								const Vec3& direction,
								const Vec3& position);
	void				applyForce(Real magnitude,
								const Vec3& direction,
								const Vec3& position);

	// get a support point of the shape.  input vector and output point
	// are in world space.
	void				getSupportPoint(SupportPoint&, const Vec3&) const;

	// get collision surface for this body against the plane with a
	// distance tolerance of epsilon (i.e. any point on the body
	// within epsilon is considered to be in contact with the plane.
	// simplex and plane are in world space.  the resulting surface
	// is in the body's coordinate space.
	void				getCollision(ContactSurface** surface,
								const ContactSimplex& simplex,
								const Plane& plane,
								Real epsilon) const;

	// marshall/unmarshall
	void				marshall(VectorN&) const;
	void				marshallDerivative(VectorN&) const;
	VectorN::const_iterator
						unmarshall(VectorN::const_iterator);

	// debugging
	void				dump() const;

private:
	void				computeDerivedState();

private:
	Shape*				shape;

	// constants
	Real				invMass;
	Matrix				invInertia;		// in body space

	// intrinsic state (state variables)
	Vec3				x;				// linear position
	Quaternion			q;				// angular position (orientation)
	Vec3				P;				// linear momentum
	Vec3				L;				// angular momentum

	// derived state
	Matrix				invInertiaWorld;// inverse inertia in world space
	Matrix				r;				// orientation as matrix
	Matrix				rT;				// transpose of r
	Vec3				v;				// velocity
	Vec3				omega;			// angular velocity
	Matrix				transform;		// transform local->world space
	Matrix				invTransform;	// transform world->local space

	// external state
	Vec3				force;
	Vec3				torque;
};

#endif
