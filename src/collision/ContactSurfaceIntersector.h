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

#ifndef BZF_CONTACT_SURFACE_INTERSECTOR_H
#define BZF_CONTACT_SURFACE_INTERSECTOR_H

#include "common.h"
#include <map>
#include "ContactPoint.h"
#include "ContactSurface.h"

// base class of collision surface intersectors
class ContactSurfaceIntersectorBase {
public:
	virtual ~ContactSurfaceIntersectorBase() { }

	// compute the contact points between two surfaces.  the surfaces
	// can be assumed to be in contact and at least one contact point
	// should be added.  the surfaces are in the same coordinate space.
	// contact points should be appended to contacts.
	//
	// all intersect() implementations are expected to follow these
	// rules:
	//
	// * is stateless;
	// * sets the a and b memebers of each contact;
	// * yield contact points inside surface a;
	// * yield contact normals pointing away from b;
	// * yield contact points even if a and b are slightly separated;
	// * yield the following:
	//   * >= 3 contact points when a and b are 2 dimensional;
	//   * 1 contact point when a and b are 1 dimensional and non-parallel;
	//   * otherwise min(dim(a), dim(b)) + 1 contact points.
	virtual void		intersect(ContactPoints& contacts,
								Body* a,
								Body* b,
								const ContactSurface* aSurface,
								const ContactSurface* bSurface) const = 0;
};

// handy template for intersectors that does casting for you
template <class A, class B>
class ContactSurfaceIntersector : public ContactSurfaceIntersectorBase {
public:
	virtual void		intersect(ContactPoints& contacts,
								Body* a,
								Body* b,
								const ContactSurface* aSurface,
								const ContactSurface* bSurface) const
	{
		doIntersect(contacts, a, b, reinterpret_cast<const A*>(aSurface),
								reinterpret_cast<const B*>(bSurface));
	}

protected:
	virtual void		doIntersect(ContactPoints& contacts,
								Body* a,
								Body* b,
								const A* aSurface,
								const B* bSurface) const = 0;
};

class ContactSurfaceIntersectorManager {
public:
	~ContactSurfaceIntersectorManager();

	// add/remove a surface/surface intersection type.  note that
	// (typeA, typeB) is different from (typeB, typeA) if typeA !=
	// typeB.
	void				add(const void* typeA, const void* typeB,
								ContactSurfaceIntersectorBase* adopted);
	void				remove(const void* typeA, const void* typeB);

	// get the intersector for a type pair
	const ContactSurfaceIntersectorBase*
						get(const void* typeA, const void* typeB) const;

	// compute the contact points between two surfaces, which should
	// both be in the world coordinate space.  returns false iff
	// there is no intersector for (typeA,typeB) or (typeB,typeA).
	// contact points are returned in world space.  the coordinate
	// space of the surfaces are indeterminate after a call that
	// returns true.
	bool				intersect(ContactPoints& contacts,
								Body* a,
								Body* b,
								ContactSurface* aSurface,
								ContactSurface* bSurface) const;

	// get the singleton instance
	static ContactSurfaceIntersectorManager*
						getInstance();

private:
	ContactSurfaceIntersectorManager();

private:
	typedef std::map<const void*, ContactSurfaceIntersectorBase*> MapB;
	typedef std::map<const void*, MapB> IntersectorMap;

	IntersectorMap		intersectors;
	static ContactSurfaceIntersectorManager* mgr;
};

#endif
// ex: shiftwidth=4 tabstop=4
