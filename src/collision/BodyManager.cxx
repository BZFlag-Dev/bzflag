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

#include "BodyManager.h"
#include "Body.h"

//
// BodyManager
//

BodyManager*			BodyManager::mgr = NULL;

BodyManager::BodyManager()
{
	// do nothing
}

BodyManager::~BodyManager()
{
	removeAll();
	mgr = NULL;
}

BodyManager*			BodyManager::getInstance()
{
	if (mgr == NULL)
		mgr = new BodyManager;
	return mgr;
}

void					BodyManager::add(Body* adopted)
{
	bodies.push_back(adopted);
}

void					BodyManager::remove(Body* body)
{
	for (iterator index = bodies.begin(); index != bodies.end(); ++index)
		if (*index == body) {
			bodies.erase(index);
			delete body;
			break;
		}
}

void					BodyManager::removeAll()
{
	for (iterator index = bodies.begin(); index != bodies.end(); ++index)
		delete *index;
	bodies.clear();
}
