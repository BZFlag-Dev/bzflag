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

#include "RegionReaderObstacle.h"
#include "RegionManagerObstacle.h"
#include "Region.h"
#include "RegionShape.h"

//
// RegionReaderObstacle
//

RegionReaderObstacle::RegionReaderObstacle()
{
	// do nothing
}

RegionReaderObstacle::~RegionReaderObstacle()
{
	// do nothing
}

ConfigFileReader*		RegionReaderObstacle::clone()
{
	return new RegionReaderObstacle(*this);
}

void					RegionReaderObstacle::parse(XMLTree::iterator xml)
{
	RegionShapePrimitive* shape = NULL;

	// parse child tags
	XMLTree::sibling_iterator scan = xml.begin();
	XMLTree::sibling_iterator end  = xml.end();
	for (; scan != end; ++scan) {
		if (scan->type == XMLNode::Tag) {
			if (scan->value == "shape")
				shape = parseShapePrimitive(scan);
			else
				throw XMLIOException(scan->position,
							string_util::format(
								"invalid tag `%s'",
								scan->value.c_str()));
		}
	}

	// must have a shape by now
	if (shape == NULL)
		throw XMLIOException(xml->position,
							string_util::format(
								"missing shape in `%s'",
								xml->value.c_str()));

	// add region
	RGNMGR_OBSTACLE->insert(new RegionPrimitive(shape));
}
