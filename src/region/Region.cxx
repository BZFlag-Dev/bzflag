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

#include "Region.h"
#include "RegionShape.h"

//
// Region
//

Region::ShapeCount		Region::shapeCounts;

Region::Region(RegionShape* adopted) : shape(adopted)
{
	addShape(shape);
}

Region::~Region()
{
	removeShape(shape);
}

RegionShape*			Region::getShape() const
{
	return shape;
}

void					Region::addShape(RegionShape* shape)
{
	// insert new shape or increment count
	ShapeCount::iterator index = shapeCounts.find(shape);
	if (index == shapeCounts.end())
		shapeCounts.insert(std::make_pair(shape, 1));
	else
		++(index->second);
}

void					Region::removeShape(RegionShape* shape)
{
	// decrement count
	ShapeCount::iterator index = shapeCounts.find(shape);
	assert(index != shapeCounts.end());
	if (--(index->second) == 0) {
		// no longer referenced
		shapeCounts.erase(index);
		delete shape;
	}
}


//
// RegionPrimitive
//

RegionPrimitive::RegionPrimitive(RegionShapePrimitive* adopted) :
								Region(adopted)
{
	// do nothing
}

RegionPrimitive::~RegionPrimitive()
{
	// do nothing
}

RegionShapePrimitive*	RegionPrimitive::getPrimitive() const
{
	return static_cast<RegionShapePrimitive*>(getShape());
}
