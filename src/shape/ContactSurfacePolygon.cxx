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

#include "ContactSurfacePolygon.h"

//
// ContactSurfacePolygon
//

ContactSurfacePolygon::ContactSurfacePolygon(
								unsigned int n,
								const Vec3* vertices_,
								const unsigned int* index)
{
	for (unsigned int i = 0; i < n; ++i)
		vertices.push_back(vertices_[index[i]]);
	makeNormal();
}

ContactSurfacePolygon::ContactSurfacePolygon(
								unsigned int n,
								const Vec3* vertices_)
{
	for (unsigned int i = 0; i < n; ++i)
		vertices.push_back(vertices_[i]);
	makeNormal();
}

ContactSurfacePolygon::ContactSurfacePolygon(const VertexList& vertices_) :
								vertices(vertices_)
{
	makeNormal();
}

ContactSurfacePolygon::~ContactSurfacePolygon()
{
	// do nothing
}

const void*				ContactSurfacePolygon::getClassType()
{
	static const int dummy = 0;
	return &dummy;
}

ContactSurface*			ContactSurfacePolygon::clone() const
{
	return new ContactSurfacePolygon(*this);
}

void					ContactSurfacePolygon::transform(const Matrix& m)
{
	for (VertexList::iterator index = vertices.begin();
								index != vertices.end(); ++index)
		index->xformPoint(m);
	Matrix mInvT(m);
	mInvT.invert();
	mInvT.transpose();
	plane.set(mInvT * plane.getNormal(), vertices[0]);
}

const void*				ContactSurfacePolygon::getType() const
{
	return getClassType();
}

void					ContactSurfacePolygon::makeNormal()
{
	assert(vertices.size() >= 3);

	Vec3 normal = (vertices[1] - vertices[0]) % (vertices[2] - vertices[1]);
	plane.set(normal.normalize(), vertices[0]);
}
// ex: shiftwidth=4 tabstop=4
