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

#ifndef BZF_REGION_H
#define BZF_REGION_H

#include "common.h"
#include <map>

class RegionShape;
class RegionShapePrimitive;

class Region {
public:
	// regions take ownership of shapes but one shape may be safely
	// shared among any number of regions.
	Region(RegionShape* adopted);
	virtual ~Region();

	// get the region's shape
	RegionShape*		getShape() const;

private:
	// not implemented
	Region(const Region&);
	Region& operator=(const Region&);

	static void			addShape(RegionShape*);
	static void			removeShape(RegionShape*);

private:
	typedef std::map<RegionShape*, unsigned int> ShapeCount;

	RegionShape*		shape;

	static ShapeCount	shapeCounts;
};

class RegionPrimitive : public Region {
public:
	RegionPrimitive(RegionShapePrimitive* adopted);
	virtual ~RegionPrimitive();

	// get the primitive shape
	RegionShapePrimitive*	getPrimitive() const;
};

#endif
// ex: shiftwidth=4 tabstop=4
