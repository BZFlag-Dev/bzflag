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

#include "ViewItemAccum.h"
#include "bzfgl.h"

//
// ViewItemAccum
//
ViewItemAccum::ViewItemAccum()
{
}

ViewItemAccum::~ViewItemAccum()
{
}

bool ViewItemAccum::onPreRender(float, float, float, float)
{
	glClearAccum(0.0, 0.0, 0.0, 0.0);
	glClear(GL_ACCUM_BUFFER_BIT);
	return true;
}

void ViewItemAccum::onPostRender(float, float, float, float)
{
	glAccum(GL_RETURN, 1.0);
}

void ViewItemAccum::renderChildren(float x, float y, float w, float h)
{
	bool first = true;
	for (Views::const_iterator index = views.begin();
								index != views.end(); ++index) {
		(*index)->render(x, y, w, h);
		if (first) {
			glAccum(GL_LOAD, 1.0);
			first = false;
		}
		else {
			glAccum(GL_ACCUM, 1.0);
		}
	}
}

// ViewItemAccumReader
ViewItemAccumReader::ViewItemAccumReader() : item(NULL)
{
	// do nothing
}

ViewItemAccumReader::~ViewItemAccumReader()
{
	if (item != NULL)
		item->unref();
}

ViewTagReader* ViewItemAccumReader::clone() const
{
	return new ViewItemAccumReader;
}

View* ViewItemAccumReader::open(XMLTree::iterator)
{
	// create itema
	assert(item == NULL);
	item = new ViewItemAccum;

	return item;
}
// ex: shiftwidth=4 tabstop=4
