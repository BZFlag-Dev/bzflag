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

#include "ContactSurfaceEdge.h"

//
// ContactSurfaceEdge
//

ContactSurfaceEdge::ContactSurfaceEdge(
								const Vec3& a_, const Vec3& b_,
								const Vec3& n_) :
								a(a_), b(b_), n(n_)
{
	// do nothing
}

ContactSurfaceEdge::~ContactSurfaceEdge()
{
	// do nothing
}

const void*				ContactSurfaceEdge::getClassType()
{
	static const int dummy = 0;
	return &dummy;
}

ContactSurface*			ContactSurfaceEdge::clone() const
{
	return new ContactSurfaceEdge(*this);
}

void					ContactSurfaceEdge::transform(const Matrix& m)
{
	a.xformPoint(m);
	b.xformPoint(m);
	Matrix mInvT(m);
	n = mInvT.invert().transpose() * n;
}

const void*				ContactSurfaceEdge::getType() const
{
	return getClassType();
}
