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

#include "ViewItemIf.h"
#include "StateDatabase.h"
#include <assert.h>

//
// ViewItemIf
//

ViewItemIf::ViewItemIf(bool _negate) : negate(_negate), truth(true)
{
	// do nothing
}

ViewItemIf::~ViewItemIf()
{
	// do nothing
}

void					ViewItemIf::setName(const BzfString& _name)
{
	name = _name;
}

void					ViewItemIf::setValue(const BzfString& _value)
{
	value = _value;
	truth = false;
}

bool					ViewItemIf::onPreRender(
								float, float, float, float)
{
	bool result = truth ? BZDB->isTrue(name) : (BZDB->get(name) == value);
	if (negate)
		result = !result;
	return result;
}

void					ViewItemIf::onPostRender(
								float, float, float, float)
{
	// do nothing
}


//
// ViewItemIfReader
//

ViewItemIfReader::ViewItemIfReader(bool _negate) :
								negate(_negate),
								item(NULL)
{
	// do nothing
}

ViewItemIfReader::~ViewItemIfReader()
{
	if (item != NULL)
		item->unref();
}

ViewTagReader* 			ViewItemIfReader::clone() const
{
	return new ViewItemIfReader(negate);
}

View*					ViewItemIfReader::open(
								const ConfigReader::Values& values)
{
	assert(item == NULL);

	// get parameters
	bool truth = true;
	BzfString name, value;
	ConfigReader::Values::const_iterator index = values.find("name");
	if (index != values.end())
		name = index->second;
	index = values.find("value");
	if (index != values.end()) {
		truth = false;
		value = index->second;
	}

	// create item
	if (!name.empty()) {
		item = new ViewItemIf(negate);
		item->setName(name);
		if (!truth)
			item->setValue(value);
	}

	return item;
}

void					ViewItemIfReader::close()
{
	assert(item != NULL);
	item = NULL;
}
