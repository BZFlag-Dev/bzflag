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

#include "LinearEquationSolverGaussian.h"
#include "mathr.h"

//
// LinearEquationSolverGaussian
//

LinearEquationSolverGaussian::LinearEquationSolverGaussian()
{
	// do nothing
}

LinearEquationSolverGaussian::~LinearEquationSolverGaussian()
{
	// do nothing
}

bool					LinearEquationSolverGaussian::solve(
								VectorN& x,
								unsigned int n,
								VectorN& A,
								VectorN& b)
{
	pivot.resize(n);
	for (unsigned int j = 0; j < n; ++j)
		pivot[j] = j;

	for (unsigned int j = 0; j < n; ++j) {
		// get largest element in column j as pivot
		Real maxElement = R_(0.0);
		unsigned int k, row;
		for (k = j; k < n; ++k) {
			if (fabs(A[j + pivot[k] * n]) > fabs(maxElement)) {
				maxElement = A[j + pivot[k] * n];
				row = k;
			}
		}

		// check for singularity
		if (maxElement == R_(0.0))
			return false;

		// pivot row with largest element in column to be the j'th row
		k = pivot[j];
		pivot[j] = pivot[row];
		pivot[row] = k;

		// invert pivot to avoid division
		maxElement = R_(1.0) / maxElement;

		// scale pivot row to make largest element 1.0
		for (k = j; k < n; ++k)
			A[k + pivot[j] * n] *= maxElement;
		b[pivot[j]] *= maxElement;

		// scale rows to eliminate column
		for (k = j + 1; k < n; ++k) {
			Real s = A[j + pivot[k] * n];
			for (unsigned int m = j + 1; m < n; ++m)
				A[m + pivot[k] * n] -= s * A[m + pivot[j] * n];
			b[pivot[k]] -= s * b[pivot[j]];
		}
	}

	// now do back-substitution
	for (unsigned int j = n; j-- > 0; ) {
		Real s = b[pivot[j]];
		for (unsigned int k = j + 1; k < n; ++k)
			s -= x[k] * A[k + pivot[j] * n];
		x[j] = s;
	}

	return true;
}
// ex: shiftwidth=4 tabstop=4
