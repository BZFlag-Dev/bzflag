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

#include "SceneNodeFieldReader.h"

//
// SceneNodeFieldReader
//
SceneNodeFieldReader::SceneNodeFieldReader()
{
}

SceneNodeFieldReader::~SceneNodeFieldReader()
{
}

bool	SceneNodeFieldReader::parse(XMLTree::iterator xml)
{
	parseBegin(xml);
	XMLTree::sibling_iterator scan = xml.begin();
	XMLTree::sibling_iterator end  = xml.end();
	for (; scan != end; ++scan) {
		if (scan->type != XMLNode::Data)
			throw XMLIOException(scan->position,
								"fields may not have children");
			parseData(scan, scan->value);
	}
	parseEnd(xml);
	return true;
}

void	SceneNodeFieldReader::parseBegin(XMLTree::iterator)
{
}

void	SceneNodeFieldReader::parseData(XMLTree::iterator, const std::string&)
{
}

void	SceneNodeFieldReader::parseEnd(XMLTree::iterator)
{
}
// ex: shiftwidth=4 tabstop=4
