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

#ifndef BZF_ODE_SOLVER_RUNGE_KUTTA_H
#define BZF_ODE_SOLVER_RUNGE_KUTTA_H

#include "ODESolver.h"

class ODESolverRungeKutta : public ODESolver {
public:
	ODESolverRungeKutta();
	virtual ~ODESolverRungeKutta();

	// ODESolver overrides
	virtual ODEAssistant::Type
						doIntegrate(VectorN& yOut, const VectorN& y,
								Real x, Real h,
								ODEAssistant*);

private:
	VectorN				yTmp, yDotTmp, yDotMid;
};

#endif
