/* bzflag
 * Copyright (c) 1993 - 2001 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#include "ViewItemMenu.h"
#include "Menu.h"
#include "MenuManager.h"
#include "OpenGLGState.h"
#include <assert.h>

//
// ViewItemMenu
//

ViewItemMenu::ViewItemMenu()
{
	// do nothing
}

ViewItemMenu::~ViewItemMenu()
{
	// do nothing
}

bool					ViewItemMenu::onPreRender(float, float, float, float)
{
	return (MENUMGR->top() != NULL);
}

void					ViewItemMenu::onPostRender(
								float x, float y, float w, float h)
{
	MENUMGR->top()->reshape((int)x, (int)y, (int)w, (int)h);
	MENUMGR->top()->render();
}


//
// ViewItemMenuReader
//

ViewItemMenuReader::ViewItemMenuReader() : item(NULL)
{
	// do nothing
}

ViewItemMenuReader::~ViewItemMenuReader()
{
	if (item != NULL)
		item->unref();
}

ViewTagReader* 			ViewItemMenuReader::clone() const
{
	return new ViewItemMenuReader;
}

View*					ViewItemMenuReader::open(
								const ConfigReader::Values&)
{
	assert(item == NULL);
	item = new ViewItemMenu;
	return item;
}

void					ViewItemMenuReader::close()
{
	assert(item != NULL);
	item = NULL;
}
