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

#ifndef BZF_LINEAR_EQUATION_SOLVER_H
#define BZF_LINEAR_EQUATION_SOLVER_H

#include "mathr.h"

class LinearEquationSolver {
public:
	LinearEquationSolver() { }
	virtual ~LinearEquationSolver() { }

	// solve Ax=b for x.  x and b are n-vectors and A is an n x n matrix.
	// note that A and b may be modified by the solver.  return false iff
	// Ax=b has no unique solution.
	virtual bool		solve(VectorN& x, unsigned int n,
								VectorN& A, VectorN& b) = 0;
};

#endif
