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

#ifndef BZF_BODY_ODE_ASSISTANT_H
#define BZF_BODY_ODE_ASSISTANT_H

#include "ODESolver.h"
#include "ContactPoint.h"

class CollisionDetector;
class ContactSolver;

class BodyODEAssistant : public ODEAssistant {
public:
	BodyODEAssistant();
	virtual ~BodyODEAssistant();

	// ODEAssistant overrides
	virtual void		marshall(VectorN& y);
	virtual void		marshallDerivative(VectorN& ydot);
	virtual void		unmarshall(const VectorN& y);
	virtual Type		drive(Real x, DriveAction action);
	virtual void		applyDiscontinuity(Real x);
	virtual void		applyDiscontinuity2(Real x, const VectorN& y);
	virtual void		dump();

protected:
	// gets the list of contact points for the given tolerances.
	// returns true if there were any intersections, false otherwise.
	bool				getContacts(ContactPoints&,
								Real intersectingTolerance,
								Real contactingTolerance) const;

	// called by drive after finding the contact points when action
	// is Classify.  default does nothing.
	virtual void		onDrive(const ContactPoints&);

private:
	CollisionDetector*	detector;
	ContactSolver*		solver;
};

#endif
