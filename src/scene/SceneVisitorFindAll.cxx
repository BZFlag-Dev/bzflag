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

#include "SceneVisitorFindAll.h"
#include "SceneNodes.h"
#include "BzfString.h"

SceneVisitorFindAll::SceneVisitorFindAll()
{
	// do nothing
}

SceneVisitorFindAll::~SceneVisitorFindAll()
{
	// unref found nodes
	for (Nodes::iterator index = found.begin();
								index != found.end(); ++index)
		(*index)->unref();
}

unsigned int				SceneVisitorFindAll::size() const
{
	return found.size();
}

SceneNode*				SceneVisitorFindAll::get(unsigned int index) const
{
	return found[index];
}

bool					SceneVisitorFindAll::visit(SceneNode* n)
{
	if (!n->getID().empty()) {
		n->ref();
		found.push_back(n);
	}
	return true;
}

bool					SceneVisitorFindAll::visit(SceneNodeGroup* n)
{
	if (!n->getID().empty()) {
		n->ref();
		found.push_back(n);
	}

	// descend into all children (regardless of group type)
	return n->SceneNodeGroup::descend(this, getParams());
}
