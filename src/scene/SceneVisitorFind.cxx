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

#include "SceneVisitorFind.h"
#include "SceneNodes.h"

SceneVisitorFind::SceneVisitorFind(const BzfString& _id) :
								id(_id), found(NULL)
{
	// do nothing
}

SceneVisitorFind::~SceneVisitorFind()
{
	if (found != NULL)
		found->unref();
}

SceneNode*				SceneVisitorFind::get() const
{
	return found;
}

bool					SceneVisitorFind::visit(SceneNode* n)
{
	if (found == NULL && n->getID() == id) {
		found = n;
		found->ref();
		return false;
	}
	return true;
}

bool					SceneVisitorFind::visit(SceneNodeGroup* n)
{
	if (!static_cast<SceneNode*>(n))
		return false;
	return descend(n);
}
