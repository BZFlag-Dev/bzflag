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

#ifndef BZF_CONTACT_SURFACE_H
#define BZF_CONTACT_SURFACE_H

#include "math3D.h"

class ContactSurface {
public:
	ContactSurface() { }
	virtual ~ContactSurface() { }

	// create an identical surface
	virtual ContactSurface*
						clone() const = 0;

	// transform surface
	virtual void		transform(const Matrix&) = 0;

	// get the type of the surface.  this is an arbitrary, but unique,
	// pointer that depends on the type of object, independent of the
	// particular instance.  subclasses should also provide a static
	// method to get this type.
	virtual const void*	getType() const = 0;
};

#endif
