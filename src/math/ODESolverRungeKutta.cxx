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

#include "ODESolverRungeKutta.h"

//
// ODESolverRungeKutta
//

ODESolverRungeKutta::ODESolverRungeKutta()
{
	// do nothing
}

ODESolverRungeKutta::~ODESolverRungeKutta()
{
	// do nothing
}

ODEAssistant::Type		ODESolverRungeKutta::doIntegrate(
								VectorN& yOut, const VectorN& y,
								Real x, Real h,
								ODEAssistant* assistant)
{
	assert(yOut.empty());

	// 4th order Runge-Kutta:
	//   k1   = h * f(x, y)
	//   k2   = h * f(x + h/2, y + k1/2)
	//   k3   = h * f(x + h/2, y + k2/2)
	//   k4   = h * f(x + h, y + k3)
	//   yOut = y + k1/6 + k2/3 + k3/3 + k4/6 + O(h^5)

	// get some constants
	const Real h_2 = R_(0.5) * h;
	const unsigned int n = y.size();

	// prep scratch space
	yTmp.clear();
	yDotTmp.clear();
	yDotMid.clear();
	yTmp.reserve(n);
	yDotTmp.reserve(n);
	yDotMid.reserve(n);

	// k1.  temporarily use yOut to hold yDot at x.
	if (dydx(yOut, y, x, assistant) != ODEAssistant::Smooth)
		return ODEAssistant::Discontinuous;
	for (unsigned int i = 0; i < n; ++i)
		yTmp.push_back(y[i] + h_2 * yOut[i]);

	// k2
	if (dydx(yDotTmp, yTmp, x + h_2, assistant) != ODEAssistant::Smooth)
		return ODEAssistant::Discontinuous;
	for (unsigned int i = 0; i < n; ++i)
		yTmp[i] = y[i] + h_2 * yDotTmp[i];

	// k3
	if (dydx(yDotMid, yTmp, x + h_2, assistant) != ODEAssistant::Smooth)
		return ODEAssistant::Discontinuous;
	for (unsigned int i = 0; i < n; ++i) {
		yTmp[i]     = y[i] + h * yDotMid[i];
		yDotMid[i] += yDotTmp[i];
	}

	// k4
	yDotTmp.clear();
	ODEAssistant::Type type = dydx(yDotTmp, yTmp, x + h, assistant);
	if (type == ODEAssistant::Discontinuous)
		return type;

	// yf
	for (unsigned int i = 0; i < n; ++i)
		yOut[i] = y[i] + (h / R_(6.0)) *
							(yOut[i] + yDotTmp[i] + R_(2.0) * yDotMid[i]);
	return type;
}
// ex: shiftwidth=4 tabstop=4
