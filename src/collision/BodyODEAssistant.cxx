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

#include "BodyODEAssistant.h"
#include "BodyManager.h"
#include "Body.h"
#include "ContactSurfaceIntersector.h"
#include "CollisionDetectorGJK.h"
#include "ContactSolverBaraffNoFriction.h"

// FIXME -- tolerances should be client changable
// FIXME -- velocityTolerance is shared by solver
static const Real s_distanceTolerance = R_(2.5e-4);
static const Real s_velocityTolerance = R_(1.0e-4);		// velocity tolerance

//
// BodyODEAssistant
//

BodyODEAssistant::BodyODEAssistant()
{
	detector = new CollisionDetectorGJK;
	solver   = new ContactSolverBaraffNoFriction;
}

BodyODEAssistant::~BodyODEAssistant()
{
	delete detector;
	delete solver;
}

void					BodyODEAssistant::marshall(VectorN& y)
{
	// add all moveable bodies
	BodyManager::const_iterator end = BODYMGR->end();
	for (BodyManager::const_iterator index = BODYMGR->begin();
								index != end; ++index) {
		// skip immovable bodies
		const Body* body = *index;
		if (body->getInverseMass() == R_(0.0))
			continue;

		// add body
		body->marshall(y);
	}
}

void					BodyODEAssistant::marshallDerivative(VectorN& ydot)
{
	// add all moveable bodies
	BodyManager::const_iterator end = BODYMGR->end();
	for (BodyManager::const_iterator index = BODYMGR->begin();
								index != end; ++index) {
		// skip immovable bodies
		const Body* body = *index;
		if (body->getInverseMass() == R_(0.0))
			continue;

		// add body
		body->marshallDerivative(ydot);
	}
}

void					BodyODEAssistant::unmarshall(const VectorN& y)
{
	// extract all moveable bodies
	VectorN::const_iterator yIndex = y.begin();
	BodyManager::const_iterator end = BODYMGR->end();
	for (BodyManager::const_iterator index = BODYMGR->begin();
								index != end; ++index) {
		// skip immovable bodies
		Body* body = *index;
		if (body->getInverseMass() == R_(0.0))
			continue;

		// extract body
		yIndex = body->unmarshall(yIndex);
	}

	assert(yIndex == y.end());
}

ODEAssistant::Type		BodyODEAssistant::drive(Real t, DriveAction action)
{
	// scale factors for various actions
	static const Real s_iScale[] = { R_(1.0), R_(1.0), R_(0.25), R_(1.0) };
	static const Real s_cScale[] = { R_(18.0), R_(2.0), R_(2.0), R_(18.0) };

	// get the contact points.  return discontinuous on intersection.
	ContactPoints contacts;
	if (getContacts(contacts, s_iScale[action] * s_distanceTolerance,
								s_cScale[action] * s_distanceTolerance))
{
fprintf(stderr, "drive discontinuous at %.11g\n", t);
		return Discontinuous;
}

	// let subclasses look at the contacts
	if (action == Classify)
		onDrive(contacts);

	// set external forces/torques on bodies if required (we must do
	// this before finding the contact forces)
	if (action != Classify)
		for (BodyManager::const_iterator index = BODYMGR->begin();
								index != BODYMGR->end(); ++index)
			(*index)->setExternalForces(t);

	if (contacts.size() > 0) {
fprintf(stderr, "%d contacts\n", contacts.size());
// FIXME -- comment is wrong
		// return discontinuity if any contacts are colliding.  remove
		// contacts that are separating.
		for (ContactPoints::iterator index = contacts.begin();
								index != contacts.end(); ) {
			Real vn = index->getNormalVelocity();
// FIXME -- clean up;  this makes collision test stricter when closer
Real s = R_(0.5) * ((index->distance / s_distanceTolerance) - R_(2.0));
fprintf(stderr, "d=%.11g s=%.11g, vn=%.11g\n", index->distance, s, vn);
Vec3 v;
index->a->getPointVelocity(v, index->point);
fprintf(stderr, "x=%.11g %.11g %.11g\n", index->point[0], index->point[1], index->point[2]);
fprintf(stderr, "v=%.11g %.11g %.11g\n", v[0], v[1], v[2]);
if (s < R_(0.5))
			if (vn < s * -s_velocityTolerance)
{
//fprintf(stderr, "drive discontinuity at %.11g\n", t);
				return Discontinuity;
}
// FIXME -- if moving away then remove contact.  tried that and it crashes.
			++index;
		}

		// apply contact forces if required
		if (action != Classify)
			solver->applyForce(contacts);
	}

fprintf(stderr, "drive smooth at %.11g\n", t);
	return Smooth;
}

void					BodyODEAssistant::applyDiscontinuity(Real /*t*/)
{
	// get the contact points
	ContactPoints contacts;
	if (getContacts(contacts, R_(1.0) * s_distanceTolerance,
								R_(4.0) * s_distanceTolerance)) {
		// there should be no intersection here if the ODESolver is working
		assert(0 && "detected inconsistency: intersection at discontinuity");
	}

	// apply impulses
	solver->applyImpulse(contacts);
}

void					BodyODEAssistant::applyDiscontinuity2(
								Real /*t*/, const VectorN& y)
{
	// get the contact points
	ContactPoints contacts;
	if (getContacts(contacts, R_(0.25) * s_distanceTolerance,
								R_(4.0) * s_distanceTolerance)) {
		// there should be no intersection here if the ODESolver is working
		assert(0 && "detected inconsistency: intersection at discontinuity");
	}

	for (ContactPoints::iterator index = contacts.begin();
								index != contacts.end(); ++index) {
		index->point.xformPoint(index->a->getInverseTransform());
		index->normal = index->a->getInverseTransform() * index->normal;
	}

	unmarshall(y);

	for (ContactPoints::iterator index = contacts.begin();
								index != contacts.end(); ++index) {
		index->point.xformPoint(index->a->getTransform());
		index->normal = index->a->getTransform() * index->normal;
	}

	// apply impulses
	solver->applyImpulse(contacts);
}

void					BodyODEAssistant::dump()
{
	for (BodyManager::const_iterator index = BODYMGR->begin();
								index != BODYMGR->end(); ++index)
		(*index)->dump();
}

bool					BodyODEAssistant::getContacts(
								ContactPoints& contacts,
								Real intersectingTolerance,
								Real contactingTolerance) const
{
	bool intersection = false;

	// get the contact points and note if there are any intersections
	Plane plane;
	BodyManager::const_iterator end = BODYMGR->end();
	for (BodyManager::const_iterator iIndex = BODYMGR->begin();
								iIndex != end; ++iIndex) {
		BodyManager::const_iterator jIndex = iIndex;
		while (++jIndex != end) {
			ContactSurface* iSurface;
			ContactSurface* jSurface;
			switch (detector->compare(plane, &iSurface, &jSurface,
										intersectingTolerance,
										contactingTolerance,
										*iIndex, *jIndex)) {
				case CollisionDetector::Intersecting:
					intersection = true;
					break;

				case CollisionDetector::Contacting: {
					// bodies are contacting.  add contact points.
					unsigned int i = contacts.size();
					ContactSurfaceIntersectorManager::getInstance()->
						intersect(contacts, *iIndex, *jIndex,
												iSurface, jSurface);

					// done with surfaces
					delete iSurface;
					delete jSurface;

					// remove contacts that aren't contacting
					while (i < contacts.size())
						if (contacts[i].distance < contactingTolerance)
							++i;
						else
							contacts.erase(contacts.begin() + i);
					break;
				}

				case CollisionDetector::Separate:
					// nothing to see here.  move along, move along.
					break;
			}
		}
	}
	return intersection;
}

void					BodyODEAssistant::onDrive(const ContactPoints&)
{
	// do nothing
}
