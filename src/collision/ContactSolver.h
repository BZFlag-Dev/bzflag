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

#ifndef BZF_CONTACT_SOLVER_H
#define BZF_CONTACT_SOLVER_H

#include "ContactPoint.h"

class ContactSolver {
public:
	ContactSolver() { }
	virtual ~ContactSolver() { }

	// solve for and apply impulses at contact points
	virtual void		applyImpulse(const ContactPoints&) = 0;

	// solve for and apply forces at contact points
	virtual void		applyForce(const ContactPoints&) = 0;
};

#endif
// ex: shiftwidth=4 tabstop=4
