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

#ifndef BZF_CONTACT_SURFACE_POINT_H
#define BZF_CONTACT_SURFACE_POINT_H

#include "ContactSurface.h"

class ContactSurfacePoint : public ContactSurface {
public:
	ContactSurfacePoint(const Vec3& point, const Vec3& normal);
	virtual ~ContactSurfacePoint();

	static const void*	getClassType();

	// accessors

	const Vec3&			getVertex() const;
	const Vec3&			getNormal() const;

	// ContactSurface overrides
	virtual ContactSurface*
						clone() const;
	virtual void		transform(const Matrix&);
	virtual const void*	getType() const;

private:
	Vec3				point;
	Vec3				normal;
};

#endif
// ex: shiftwidth=4 tabstop=4
