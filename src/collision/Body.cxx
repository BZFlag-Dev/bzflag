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

#include <assert.h>
#include "Body.h"
#include "TransformedShape.h"
#include "ContactSurface.h"

//
// Body
//

Body::Body(Shape* shape_, Real inverseDensity) :
								originalShape(shape_)
{
	assert(originalShape != NULL);

	// compute inverse of mass and inertia tensor
	originalShape->getInertia(invInertia);
	invInertia.invert();
	invInertia *= inverseDensity;
	invMass     = inverseDensity / originalShape->getVolume();

	// create the transformed shape
	Matrix xform;
	shape = new TransformedShape(originalShape, xform);
	computeDerivedState();
}

Body::~Body()
{
	delete shape;
}

void					Body::setPosition(const Vec3& x_)
{
	x = x_;
	computeDerivedState();
}

void					Body::setOrientation(const Quaternion& q_)
{
	q = q_;
	q.normalize();
	computeDerivedState();
}

void					Body::setVelocity(const Vec3& v_)
{
	// invMass must not be zero (i.e. immovable objects cannot move)
	if (invMass != R_(0.0)) {
		P = (R_(1.0) / invMass) * v_;
		computeDerivedState();
	}
}

void					Body::setAngularVelocity(const Vec3& omega_)
{
	// note -- omega_ is in *body* coordinates
	// invInertia must be invertable
	Matrix tmp = invInertia;
	L = tmp.invert() * omega_;
	computeDerivedState();
}

Shape*					Body::getShape() const
{
	return originalShape;
}

Real					Body::getInverseMass() const
{
	return invMass;
}

const Matrix&			Body::getInverseInertia() const
{
	return invInertia;
}

const Matrix&			Body::getInverseWorldInertia() const
{
	return invInertiaWorld;
}

const Vec3&				Body::getPosition() const
{
	return x;
}

const Quaternion&		Body::getOrientation() const
{
	return q;
}

const Vec3&				Body::getOmega() const
{
	return omega;
}

void					Body::getPointVelocity(Vec3& wv, const Vec3& wx) const
{
	// velocity of point is the sum of linear and angular velocity:
	wv = v + omega % (wx - x);
}

void					Body::getPointAcceleration(
								Vec3& a, const Vec3& wx) const
{
	// acceleration of point is the sum of linear acceleration
	// (force / mass), tangential acceleration (due to the body
	// being angularly accelerated), and centripetal acceleration
	// (due to the point tracing a circle about the center of mass).

	// first compute the offset of the point from the center of mass
	Vec3 d = wx - x;

	// next compute the derivative of omega.  that's the sum of
	// external angular acceleration (torque / inertia) and the
	// change in omega due to angular momemtum.
	Vec3 omegaDot = invInertiaWorld * (L % omega + torque);

	// now compute acceleration of the point
	a = invMass * force + omega % (omega % d) + omegaDot % d;
}

void					Body::getEffectiveDirection(
								Vec3& d, const Vec3& n,
								const Vec3& p1, const Vec3& p2) const
{
	d = invMass * n + (invInertiaWorld * ((p1 - x) % n)) % (p2 - x);
}

void					Body::setExternalForces(Real)
{
	// FIXME -- get these through a helper object?
	force.zero();
	torque.zero();
	if (invMass != R_(0.0))
		force.set(R_(0.0), R_(0.0), R_(-9.8) / invMass);
}

void					Body::applyImpulse(Real magnitude,
								const Vec3& direction,
								const Vec3& position)
{
	P += magnitude * direction;
	L += magnitude * ((position - x) % direction);

	// recompute derived state
	v     = invMass * P;
	omega = invInertiaWorld * L;
}

void					Body::applyForce(Real magnitude,
								const Vec3& direction,
								const Vec3& position)
{
	force  += magnitude * direction;
	torque += magnitude * ((position - x) % direction);
}

void					Body::marshall(VectorN& y) const
{
	y.push_back(x[0]);
	y.push_back(x[1]);
	y.push_back(x[2]);

	y.push_back(q[0]);
	y.push_back(q[1]);
	y.push_back(q[2]);
	y.push_back(q[3]);

	y.push_back(P[0]);
	y.push_back(P[1]);
	y.push_back(P[2]);

	y.push_back(L[0]);
	y.push_back(L[1]);
	y.push_back(L[2]);
}

void					Body::marshallDerivative(VectorN& ydot) const
{
	ydot.push_back(v[0]);
	ydot.push_back(v[1]);
	ydot.push_back(v[2]);

	// derivative of q is (1/2) * omega * q
	Quaternion qdot = R_(0.5) * omega % q;
	ydot.push_back(qdot[0]);
	ydot.push_back(qdot[1]);
	ydot.push_back(qdot[2]);
	ydot.push_back(qdot[3]);

	ydot.push_back(force[0]);
	ydot.push_back(force[1]);
	ydot.push_back(force[2]);

	ydot.push_back(torque[0]);
	ydot.push_back(torque[1]);
	ydot.push_back(torque[2]);
}

VectorN::const_iterator	Body::unmarshall(VectorN::const_iterator index)
{
	x[0] = *index;
	x[1] = *++index;
	x[2] = *++index;

	q[0] = *++index;
	q[1] = *++index;
	q[2] = *++index;
	q[3] = *++index;

	P[0] = *++index;
	P[1] = *++index;
	P[2] = *++index;

	L[0] = *++index;
	L[1] = *++index;
	L[2] = *++index;

	// update derived state
	computeDerivedState();

	return ++index;
}

Real					Body::getVolume() const
{
	return shape->getVolume();
}

void					Body::getInertia(Matrix& m) const
{
	m = invInertiaWorld;
	m.invert();
}

bool					Body::isInside(const Vec3& p) const
{
	return shape->isInside(p);
}

bool					Body::intersect(const Ray& ray) const
{
	return shape->intersect(ray);
}

bool					Body::intersect(
								IntersectionPoint& p, const Ray& ray) const
{
	return shape->intersect(p, ray);
}

void					Body::getRandomPoint(Vec3& p) const
{
	shape->getRandomPoint(p);
}

void					Body::getSupportPoint(
								SupportPoint& point,
								const Vec3& vector) const
{
	// get the support point.
	// transform vector from world to local coordinate space.  the
	// vector transforms like a normal, which means using the
	// inverse transpose of the transform.  however, we're going
	// from world to local space and the transform goes from local
	// to world, so we need the inverse transform.  together we
	// need the inverse of the inverse of the transpose, which is
	// just the transpose.
	originalShape->getSupportPoint(point, rT * vector);

	// and transform the point to world space
	point.point = r * point.point + x;
}

ContactSurface*			Body::getCollision(
								const ContactSimplex& simplex,
								const Plane& plane,
								Real epsilon) const
{
// FIXME -- maybe pass transforms down to shape->getCollision()
// to avoid transforming when unnecessary.
	// transform plane to local space.  see comment in getSupportPoint()
	// for why we use the transpose of the rotation.
	Plane localPlane(rT * plane.getNormal(), plane.distance(x));

	// transform simplex to local space
// FIXME -- allow modification of simplex to avoid copy
	ContactSimplex tSimplex = simplex;
	for (unsigned int i = 0; i < tSimplex.size(); ++i)
		tSimplex[i].point.xformPoint(getInverseTransform());

	// get the surface
	ContactSurface* s = originalShape->getCollision(
								tSimplex, localPlane, epsilon);

	// transform to world
	if (s != NULL)
		s->transform(getTransform());

	return s;
}

const Matrix&			Body::getTransform() const
{
	return shape->getTransform();
}

const Matrix&			Body::getTransposeTransform() const
{
	return shape->getTransposeTransform();
}

const Matrix&			Body::getInverseTransform() const
{
	return shape->getInverseTransform();
}

const Matrix&			Body::getInverseTransposeTransform() const
{
	return shape->getInverseTransposeTransform();
}

void					Body::computeDerivedState()
{
	// orientation
	q.normalize();
	r  = q;
	rT = r;
	rT.transpose();

	// inertia tensor in world space
	invInertiaWorld = r * invInertia * rT;

	// velocity from momentum (linear and angular)
	v     = invMass * P;
	omega = invInertiaWorld * L;

	// compute transform and invTransform
	Matrix transform, tmp;
	tmp.setTranslate(-x[0], -x[1], -x[2]);
	transform.setTranslate(x[0], x[1], x[2]);
	transform    *= r;
	Matrix invTransform = rT;
	invTransform *= tmp;
	shape->setTransform(transform, invTransform);
}

#include <stdio.h>
void					Body::dump() const
{
	// dump body state
	fprintf(stderr, "body %p:\n", this);
	fprintf(stderr, "  x: %14.10f %14.10f %14.10f\n", x[0], x[1], x[2]);
	fprintf(stderr, "  v: %14.10f %14.10f %14.10f\n", v[0], v[1], v[2]);
	fprintf(stderr, "  a: %14.10f %14.10f %14.10f\n", invMass * force[0],
								invMass * force[1], invMass * force[2]);
	fprintf(stderr, "  q: %14.10f %14.10f %14.10f %14.10f\n", q[0], q[1], q[2], q[3]);
	fprintf(stderr, "  r: %14.10f %14.10f %14.10f\n"
					"     %14.10f %14.10f %14.10f\n"
					"     %14.10f %14.10f %14.10f\n",
								r[0], r[4], r[8],
								r[1], r[5], r[9],
								r[2], r[6], r[10]);
	fprintf(stderr, "  w: %14.10f %14.10f %14.10f\n", omega[0], omega[1], omega[2]);
	fprintf(stderr, "  t: %14.10f %14.10f %14.10f\n",
								torque[0], torque[1], torque[2]);
	fprintf(stderr, "  P: %14.10f %14.10f %14.10f\n", P[0], P[1], P[2]);
	fprintf(stderr, "  L: %14.10f %14.10f %14.10f\n", L[0], L[1], L[2]);
	fprintf(stderr, "  I: %14.10f %14.10f %14.10f\n"
					"     %14.10f %14.10f %14.10f\n"
					"     %14.10f %14.10f %14.10f\n",
								invInertiaWorld[0], invInertiaWorld[4], invInertiaWorld[8],
								invInertiaWorld[1], invInertiaWorld[5], invInertiaWorld[9],
								invInertiaWorld[2], invInertiaWorld[6], invInertiaWorld[10]);

	// dump interesting points
	typedef std::vector<Vec3> Points;
	Points points;
	shape->getDumpPoints(points);
	for (Points::const_iterator index = points.begin();
								index != points.end(); ++index) {
		Vec3 p = *index, v, a;
		fprintf(stderr, "  p[% 2d]: % 11g % 11g % 11g\n",
								index - points.begin(), p[0], p[1], p[2]);
		p.xformPoint(getTransform());
		getPointVelocity(v, p);
		getPointAcceleration(a, p);
		fprintf(stderr, "      x: % 11g % 11g % 11g\n", p[0], p[1], p[2]);
		fprintf(stderr, "      v: % 11g % 11g % 11g\n", v[0], v[1], v[2]);
		fprintf(stderr, "      a: % 11g % 11g % 11g\n", a[0], a[1], a[2]);
	}
}
