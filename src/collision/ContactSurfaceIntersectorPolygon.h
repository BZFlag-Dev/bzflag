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

#ifndef BZF_CONTACT_SURFACE_INTERSECTOR_POLYGON_H
#define BZF_CONTACT_SURFACE_INTERSECTOR_POLYGON_H

#include "ContactSurfaceIntersector.h"

class ContactSurfacePoint;
class ContactSurfaceEdge;
class ContactSurfacePolygon;

class ContactSurfaceIntersectorPointPoint: public
ContactSurfaceIntersector<ContactSurfacePoint, ContactSurfacePoint> {
protected:
	void				doIntersect(ContactPoints& contacts,
								Body* a,
								Body* b,
								const ContactSurfacePoint* aSurface,
								const ContactSurfacePoint* bSurface) const;
};

class ContactSurfaceIntersectorPointEdge: public
ContactSurfaceIntersector<ContactSurfacePoint, ContactSurfaceEdge> {
protected:
	void				doIntersect(ContactPoints& contacts,
								Body* a,
								Body* b,
								const ContactSurfacePoint* aSurface,
								const ContactSurfaceEdge* bSurface) const;
};

class ContactSurfaceIntersectorPointPolygon: public
ContactSurfaceIntersector<ContactSurfacePoint, ContactSurfacePolygon> {
protected:
	void				doIntersect(ContactPoints& contacts,
								Body* a,
								Body* b,
								const ContactSurfacePoint* aSurface,
								const ContactSurfacePolygon* bSurface) const;
};

class ContactSurfaceIntersectorEdgeEdge : public
ContactSurfaceIntersector<ContactSurfaceEdge, ContactSurfaceEdge> {
protected:
	void				doIntersect(ContactPoints& contacts,
								Body* a,
								Body* b,
								const ContactSurfaceEdge* aSurface,
								const ContactSurfaceEdge* bSurface) const;
};

class ContactSurfaceIntersectorEdgePolygon : public
ContactSurfaceIntersector<ContactSurfaceEdge, ContactSurfacePolygon> {
protected:
	void				doIntersect(ContactPoints& contacts,
								Body* a,
								Body* b,
								const ContactSurfaceEdge* aSurface,
								const ContactSurfacePolygon* bSurface) const;

private:
	template <int x, int y>
	void				doIntersect2D(ContactPoints& contacts,
								Body* a,
								Body* b,
								const ContactSurfaceEdge* aSurface,
								const ContactSurfacePolygon* bSurface,
								bool flip) const;
};

class ContactSurfaceIntersectorPolygonPolygon : public
ContactSurfaceIntersector<ContactSurfacePolygon, ContactSurfacePolygon> {
protected:
	void				doIntersect(ContactPoints& contacts,
								Body* a,
								Body* b,
								const ContactSurfacePolygon* aSurface,
								const ContactSurfacePolygon* bSurface) const;

private:
	template <int x, int y>
	void				doIntersect2D(ContactPoints& contacts,
								Body* a,
								Body* b,
								const ContactSurfacePolygon* aSurface,
								const ContactSurfacePolygon* bSurface,
								bool flip) const;
};

#endif
