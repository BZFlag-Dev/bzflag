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

#ifndef BZF_CONTACT_SOLVER_BARAFF_NO_FRICTION_H
#define BZF_CONTACT_SOLVER_BARAFF_NO_FRICTION_H

#include "ContactSolver.h"

class LinearEquationSolver;

class ContactSolverBaraffNoFriction : public ContactSolver {
public:
	ContactSolverBaraffNoFriction();
	virtual ~ContactSolverBaraffNoFriction();

	// ContactSolver overrides
	virtual void		applyImpulse(const ContactPoints&);
	virtual void		applyForce(const ContactPoints&);

protected:
	void				computeAMatrix(VectorN&, const ContactPoints&) const;
	void				computeBVector(VectorN&, const ContactPoints&) const;
	void				solve(VectorN& f);
	void				driveToZero(VectorN& f, unsigned int i);
	void				fDirection(VectorN& fDelta, unsigned int i);
	Real				maxStep(unsigned int& j, unsigned int i,
								const VectorN& f);

private:
	enum Pivot { Unknown, C, NC };
	typedef std::vector<Pivot> PivotVector;
	typedef std::vector<unsigned int> ContactSet;

	VectorN				A;
	VectorN				b;
	VectorN				f;

	VectorN				a;
	VectorN				fDelta;
	VectorN				aDelta;
	VectorN				subA;
	PivotVector			pivot;
	ContactSet			clamped;

	LinearEquationSolver*	solver;
};

#endif
// ex: shiftwidth=4 tabstop=4
