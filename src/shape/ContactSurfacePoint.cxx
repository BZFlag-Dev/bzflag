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

#include "ContactSurfacePoint.h"

//
// ContactSurfacePoint
//

ContactSurfacePoint::ContactSurfacePoint(
								const Vec3& point_,
								const Vec3& normal_) :
								point(point_),
								normal(normal_)
{
	normal.normalize();
}

ContactSurfacePoint::~ContactSurfacePoint()
{
	// do nothing
}

const void*				ContactSurfacePoint::getClassType()
{
	static const int dummy = 0;
	return &dummy;
}

const Vec3&				ContactSurfacePoint::getVertex() const
{
	return point;
}

const Vec3&				ContactSurfacePoint::getNormal() const
{
	return normal;
}

ContactSurface*			ContactSurfacePoint::clone() const
{
	return new ContactSurfacePoint(*this);
}

void					ContactSurfacePoint::transform(const Matrix& m)
{
	point.xformPoint(m);
	Matrix mInvT(m);
	normal = mInvT.invert().transpose() * normal;
}

const void*				ContactSurfacePoint::getType() const
{
	return getClassType();
}
// ex: shiftwidth=4 tabstop=4
