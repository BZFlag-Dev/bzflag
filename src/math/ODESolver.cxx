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

#include "ODESolver.h"

//
// ODESearchAssisant
//   delegates to an ODEAssistant but always passes Search as the
//   action to drive().
//

class ODESearchAssistant : public ODEAssistant {
public:
	ODESearchAssistant(ODEAssistant* delegate_, DriveAction action_) :
								delegate(delegate_), action(action_) { }
	virtual ~ODESearchAssistant() { }

	// ODEAssistant overrides
	virtual void		marshall(VectorN& y)
							{ delegate->marshall(y); }
	virtual void		marshallDerivative(VectorN& ydot)
							{ delegate->marshallDerivative(ydot); }
	virtual void		unmarshall(const VectorN& y)
							{ delegate->unmarshall(y); }
	virtual Type		drive(Real x, DriveAction)
							{ return delegate->drive(x, action); }
	virtual void		applyDiscontinuity(Real x)
							{ delegate->applyDiscontinuity(x); }
	virtual void		applyDiscontinuity2(Real x, const VectorN& y)
							{ delegate->applyDiscontinuity2(x, y); }
	virtual void		dump()
							{ delegate->dump(); }

private:
	ODEAssistant*		delegate;
	DriveAction			action;
};


//
// ODESolver
//

static const Real		s_minTimeStep((Real)1.0e-7);

void					ODESolver::solve(
								VectorN& y,
								Real x, Real h,
								ODEAssistant* assistant)
{
unsigned int retry = 0;
	VectorN yTmp;
	while (h > s_minTimeStep) {
		// get initial conditions
		yTmp.swap(y);
		y.clear();

		// solve up to a discontinuity
		const Real dh = integrateToDiscontinuity(y, yTmp, x, h, assistant);
		assert(dh >= R_(0.0));
fprintf(stderr, "h=%.10f\n", dh);

/*
if (dh + s_minTimeStep < h) {
yTmp.swap(y);
y.clear();
integrateToDiscontinuity2(y, yTmp, x + dh, s_minTimeStep, assistant);
}
*/
		// step forward
		if (dh != R_(0.0)) {
			x    += dh;
			h    -= dh;
retry = 0;
		}
else if (++retry >= 2) {
fprintf(stderr, "discontinuity2\n");
//retry = 0;
y.clear();
//integrateToDiscontinuity2(y, yTmp, x, h, assistant);
}
break;
	}
}

Real					ODESolver::integrateToDiscontinuity(
								VectorN& yOut,
								const VectorN& y,
								Real x, Real h,
								ODEAssistant* assistant)
{
	assert(yOut.empty());
	assert(h != R_(0.0));
//fprintf(stderr, "trying to integrate over %.11g\n", h);

	// try integrating
	assert(testClassify(y, x, assistant) == ODEAssistant::Smooth);
	ODEAssistant::Type type = integrate(yOut, y, x, h, assistant);

	// special handling if we can't integrate
	if (type != ODEAssistant::Smooth) {
//fprintf(stderr, "not smooth\n");
		if (type == ODEAssistant::Discontinuous) {
			// find the discontinuity
			ODESearchAssistant searcher(assistant, ODEAssistant::Search);
			h = findDiscontinuity(y, x, h, &searcher);
//fprintf(stderr, "found discontinuity at %.11g\n", h);

			// now integrate up to the discontinuity
			assert(testClassify(y, x, assistant) == ODEAssistant::Smooth);
			yOut.clear();
			type = integrate(yOut, y, x, h, &searcher);
			assert(type != ODEAssistant::Discontinuous);
		}

		// correct for the discontinuity
		assert(testClassify(yOut, x + h, assistant) != ODEAssistant::Discontinuous);
		assistant->applyDiscontinuity(x + h);
		yOut.clear();
		assistant->marshall(yOut);
		assert(testClassify(yOut, x + h, assistant) == ODEAssistant::Smooth);
	}

	// return the amount we integrated over
	return h;
}

void					ODESolver::integrateToDiscontinuity2(
								VectorN& yOut,
								const VectorN& y,
								Real x, Real h,
								ODEAssistant* assistant)
{
	assert(yOut.empty());
	assert(h != R_(0.0));

	// find the discontinuity
	ODESearchAssistant searcher(assistant, ODEAssistant::Search2);
	h = findDiscontinuity(y, x, h, &searcher);
	yOut.clear();
	integrate(yOut, y, x, h, &searcher);

	// correct for the discontinuity
	assistant->applyDiscontinuity2(x + h, y);
	yOut.clear();
	assistant->marshall(yOut);
	assert(testClassify(yOut, x + h, assistant) == ODEAssistant::Smooth);
}

ODEAssistant::Type		ODESolver::integrate(
								VectorN& yOut,
								const VectorN& y,
								Real x, Real h,
								ODEAssistant* assistant)
{
	if (h != R_(0.0)) {
		// do the integration
		ODEAssistant::Type type = doIntegrate(yOut, y, x, h, assistant);
		if (type == ODEAssistant::Discontinuous) {
			yOut.clear();
			return type;
		}
	}
	else {
		// no step being taken
		yOut = y;
	}

	// update the state
	assistant->unmarshall(yOut);

	// classify function up to x + h
	return assistant->drive(x + h, ODEAssistant::Classify);
}

Real					ODESolver::findDiscontinuity(
								const VectorN& y,
								Real x, Real h,
								ODEAssistant* assistant)
{
	VectorN yScratch;
	yScratch.reserve(y.size());
//fprintf(stderr, "find in %.11g\n", h);

	// the minimum timestep to search for.  this should depend on the
	// maximum velocity in the system and the contact tolerance
	// distance.  in particular dhMin < contactTolerance * maxVelocity.
	// that should ensure that no object can move from separating to
	// intersecting in less than dhMin time (which is, of course, the
	// thing we're trying to ensure).
	//
	// dhMin should also be less than s_minTimeStep or we'll end up
	// immediately returning 0 for very small timesteps which can
	// lead to an infinite loop of zero time steps.
	const Real dhMin = R_(0.5) * s_minTimeStep;

	// use bisection method
Real m = R_(0.0);
	Real hs = R_(0.0), dhs = h;
	while (dhs > dhMin) {
		dhs *= R_(0.5);
		yScratch.clear();
//fprintf(stderr, "  try %.11g\n", hs + dhs);
		ODEAssistant::Type type = integrate(yScratch,
										y, x, hs + dhs, assistant);
//		if (type == ODEAssistant::Discontinuity)
//			return hs + dhs;
if (type == ODEAssistant::Discontinuity)
{
//fprintf(stderr, "    discontinuity at %.11g\n", hs + dhs);
m = hs + dhs;
}
		if (type == ODEAssistant::Smooth)
{
//fprintf(stderr, "    okay\n");
			hs += dhs;
}
	}

if (m > hs)
{
//fprintf(stderr, "  return discontinuity %.11g\n", m);
return m;
}
//fprintf(stderr, "    return smooth at %.11g\n", hs);
	return hs;
}

ODEAssistant::Type		ODESolver::dydx(VectorN& yDot,
								const VectorN& y, Real x,
								ODEAssistant* assistant)
{
	// move state from y to bodies
	assistant->unmarshall(y);

	// external modification of derivatives
	ODEAssistant::Type type = assistant->drive(x, ODEAssistant::Normal);
	if (type == ODEAssistant::Discontinuous)
		return type;

	// move derivative of state from bodies to ydot
	assert(yDot.empty());
	assistant->marshallDerivative(yDot);
	return type;
}

ODEAssistant::Type		ODESolver::testClassify(const VectorN& y,
								Real x, ODEAssistant* assistant) const
{
	assistant->unmarshall(y);
	return assistant->drive(x, ODEAssistant::Classify);
}
// ex: shiftwidth=4 tabstop=4
