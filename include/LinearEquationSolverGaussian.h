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

#ifndef BZF_LINEAR_EQUATION_SOLVER_GAUSSIAN_H
#define BZF_LINEAR_EQUATION_SOLVER_GAUSSIAN_H

#include "LinearEquationSolver.h"

//
// solve using gaussian elimination with partial pivoting
//

class LinearEquationSolverGaussian : public LinearEquationSolver {
public:
	LinearEquationSolverGaussian();
	virtual ~LinearEquationSolverGaussian();

	// LinearEquationSolver overrides
	virtual bool		solve(VectorN& x, unsigned int n,
								VectorN& A, VectorN& b);

private:
	std::vector<unsigned int> pivot;
};

#endif
// ex: shiftwidth=4 tabstop=4
