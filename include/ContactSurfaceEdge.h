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

#ifndef BZF_CONTACT_SURFACE_EDGE_H
#define BZF_CONTACT_SURFACE_EDGE_H

#include "ContactSurface.h"

class ContactSurfaceEdge : public ContactSurface {
public:
	ContactSurfaceEdge(const Vec3& v1, const Vec3& v2, const Vec3& nAvg);
	virtual ~ContactSurfaceEdge();

	static const void*	getClassType();

	// accessors

	const Vec3&			getVertex1() const;
	const Vec3&			getVertex2() const;
	const Vec3&			getAverageNormal() const;

	// ContactSurface overrides
	virtual ContactSurface*
						clone() const;
	virtual void		transform(const Matrix&);
	virtual const void*	getType() const;

private:
	Vec3				a;
	Vec3				b;
	Vec3				n;
};

inline
const Vec3&				ContactSurfaceEdge::getVertex1() const
{
	return a;
}

inline
const Vec3&				ContactSurfaceEdge::getVertex2() const
{
	return b;
}

inline
const Vec3&				ContactSurfaceEdge::getAverageNormal() const
{
	return n;
}

#endif
