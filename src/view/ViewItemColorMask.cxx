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

#include "ViewItemColorMask.h"
#include "bzfgl.h"

//
// ViewItemColorMask
//
ViewItemColorMask::ViewItemColorMask()
{
	// do nothing
}

ViewItemColorMask::~ViewItemColorMask()
{
	// do nothing
}

void ViewItemColorMask::setMask(bool _r, bool _g, bool _b, bool _a)
{
	r = _r;
	g = _g;
	b = _b;
	a = _a;
}

bool ViewItemColorMask::onPreRender(float, float, float, float)
{
	glColorMask(r, g, b, a);
	return true;
}

void ViewItemColorMask::onPostRender(float, float, float, float)
{
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
}

//
// ViewItemColorMaskReader
//
ViewItemColorMaskReader::ViewItemColorMaskReader() : item(NULL)
{
	// do nothing
}

ViewItemColorMaskReader::~ViewItemColorMaskReader()
{
	if (item != NULL)
		item->unref();
}

ViewTagReader* ViewItemColorMaskReader::clone() const
{
	return new ViewItemColorMaskReader;
}

View* ViewItemColorMaskReader::open(XMLTree::iterator xml)
{
	// create item
	assert(item == NULL);
	item = new ViewItemColorMask;

	// parse
	bool r, g, b, a;
	xml->getAttribute("red", xmlParseEnum(s_xmlEnumBool, xmlSetVar(r)));
	xml->getAttribute("green", xmlParseEnum(s_xmlEnumBool, xmlSetVar(g)));
	xml->getAttribute("blue", xmlParseEnum(s_xmlEnumBool, xmlSetVar(b)));
	xml->getAttribute("alpha", xmlParseEnum(s_xmlEnumBool, xmlSetVar(a)));

	// set parameters
	item->setMask(r, g, b, a);

	return item;
}
// ex: shiftwidth=4 tabstop=4
