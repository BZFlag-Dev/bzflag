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

#ifndef BZF_REGION_READER_H
#define BZF_REGION_READER_H

#include "ConfigFileReader.h"

class Matrix;
class RegionShape;
class RegionShapePrimitive;

class RegionReader : public ConfigFileReader {
public:
	RegionReader() { }
	virtual ~RegionReader() { }

	// ConfigFileReader overrides
	virtual ConfigFileReader*
						clone() = 0;
	virtual void		parse(XMLTree::iterator) = 0;

protected:
	static RegionShape*	parseShape(XMLTree::iterator);
	static RegionShapePrimitive*	parseShapePrimitive(XMLTree::iterator);

private:
	static bool			parseTransformTag(Matrix&, XMLTree::iterator);
	static RegionShapePrimitive*	parseShapeShape(XMLTree::iterator);
	static RegionShapePrimitive*	parseBox(XMLTree::iterator);
	static RegionShapePrimitive*	parsePyramid(XMLTree::iterator);
};

#endif
// ex: shiftwidth=4 tabstop=4
