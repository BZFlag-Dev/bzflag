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

#ifndef BZF_CONTACT_SURFACE_POLYGON_H
#define BZF_CONTACT_SURFACE_POLYGON_H

#include "common.h"
#include <vector>
#include "ContactSurface.h"

class ContactSurfacePolygon : public ContactSurface {
public:
	typedef std::vector<Vec3> VertexList;

	// first c'tor constructs polygon from vertices[index[0]],
	// vertices[index[1]], ... up to n-1.  second form makes
	// polygon from vertices[0], vertices[1], ... up to n-1.
	// third form copies the vertex list.
	ContactSurfacePolygon(unsigned int n, const Vec3* vertices,
								const unsigned int* index);
	ContactSurfacePolygon(unsigned int n, const Vec3* vertices);
	ContactSurfacePolygon(const VertexList&);
	virtual ~ContactSurfacePolygon();

	static const void*	getClassType();

	// accessors

	const VertexList&	getVertices() const;
	const Vec3&			getNormal() const;
	const Plane&		getPlane() const;

	// ContactSurface overrides
	virtual ContactSurface*
						clone() const;
	virtual void		transform(const Matrix&);
	virtual const void*	getType() const;

private:
	void				makeNormal();
	void				project();

private:
	VertexList			vertices;
	Plane				plane;
};

inline
const ContactSurfacePolygon::VertexList&
						ContactSurfacePolygon::getVertices() const
{
	return vertices;
}

inline
const Vec3&				ContactSurfacePolygon::getNormal() const
{
	return plane.getNormal();
}

inline
const Plane&			ContactSurfacePolygon::getPlane() const
{
	return plane;
}

#endif
