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

#ifndef BZF_ODE_SOLVER_H
#define BZF_ODE_SOLVER_H

#include "mathr.h"

class ODEAssistant {
public:
	enum Type {
		Smooth,
		Discontinuity,
		Discontinuous
	};
	enum DriveAction {
		Normal,
		Search,
		Search2,
		Classify
	};

	ODEAssistant() { }
	virtual ~ODEAssistant() { }

	// copy state to y.  y must be empty on entry.
	virtual void		marshall(VectorN& y) = 0;

	// copy derivative of state to ydot.  ydot must be empty on entry.
	virtual void		marshallDerivative(VectorN& ydot) = 0;

	// copy y to state
	virtual void		unmarshall(const VectorN& y) = 0;

	// update (externally driven) derivative at x.  return Smooth if
	// the derivative has no discontinuity up to and including x.
	// return Discontinuous if there was a discontinuity prior to x.
	// return Discontinuity if there is a discontinuity at x.
	//
	// iff action is not Classify then the derivative should be
	// updated.  the solver uses Classify when it needs to check the
	// derivative up to x but doesn't need the derivative.
	//
	// if action is Search then the solver is trying to find a
	// discontinuity.  the method should adjust it's calculations in
	// preparation for a call to applyDiscontinuity().
	virtual Type		drive(Real x, DriveAction action) = 0;

	// apply discontinuity that occurs at x.  this is called by the
	// ODE solver after stepping to a discontinuity (as indicated by
	// drive(x, Search)).
	virtual void		applyDiscontinuity(Real x) = 0;

	virtual void		applyDiscontinuity2(Real x, const VectorN& y) = 0;

	// debugging
	virtual void		dump() = 0;
};

class ODESolver {
public:
	typedef ODEAssistant::Type Type;

	ODESolver() { }
	virtual ~ODESolver() { }

	// integrates y=f(x) from x to x+h.  yInOut is f(x) on entry.  returns
	// Discontinuous (and yInOut is unchanged) if the integration cannot
	// be performed (because dydx() returned Discontinuous).  otherwise
	// it updates yInOut to f(x+h) and either returns Smooth if the
	// function was smooth over the interval or returns Discontinuity
	// if the function was smooth up to but not including x + h (within
	// some tolerance).
	void				solve(VectorN& yInOut,
								Real x, Real h,
								ODEAssistant*);

protected:
	// computes dy/dx at x.  ydot must be empty on entry.  returns
	// what assistant->updateDerivative() returned.  if it returns
	// Discontinuous then ydot is *not* set.
	ODEAssistant::Type	dydx(VectorN& ydot,
								const VectorN& y, Real x,
								ODEAssistant* assistant);

	// integrates y=f(x) from x to x+h.  yOut will be empty on entry.
	// if the integration cannot be performed due to a discontinuity
	// then return false, otherwise return true.  yOut is ignored if
	// doIntegrate() returns false.
	virtual ODEAssistant::Type
						doIntegrate(VectorN& yOut, const VectorN& y,
								Real x, Real h,
								ODEAssistant*) = 0;

	// search for the discontiuity between (x,x+h).  return the h such
	// that x+h is the discontinuity within some tolerance.  the default
	// implementation uses bisection.
	virtual Real		findDiscontinuity(const VectorN& y,
								Real x, Real h,
								ODEAssistant* assistant);

private:
	Real				integrateToDiscontinuity(VectorN& yOut,
								const VectorN& y,
								Real x, Real h,
								ODEAssistant*);
	ODEAssistant::Type	integrate(VectorN& yOut, const VectorN& y,
								Real x, Real h,
								ODEAssistant*);
	ODEAssistant::Type	testClassify(const VectorN& y,
								Real x, ODEAssistant*) const;

	void				integrateToDiscontinuity2(VectorN& yOut,
								const VectorN& y,
								Real x, Real h,
								ODEAssistant*);
};

#endif
// ex: shiftwidth=4 tabstop=4
