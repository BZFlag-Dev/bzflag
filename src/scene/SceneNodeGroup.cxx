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

#include "SceneNodeGroup.h"
#include "SceneVisitor.h"
#include <assert.h>

//
// SceneNodeGroup
//

SceneNodeGroup::SceneNodeGroup()
{
	// do nothing
}

SceneNodeGroup::~SceneNodeGroup()
{
	clearChildren();
}

void					SceneNodeGroup::insertChild(
								unsigned int index, SceneNode* child)
{
	assert(child != NULL);

	child->ref();
	children.insert(children.begin() + index, child);
}

void					SceneNodeGroup::eraseChild(unsigned int index)
{
	children[index]->unref();
	children.erase(children.begin() + index);
}

void					SceneNodeGroup::pushChild(SceneNode* child)
{
	if (child != NULL) {
		child->ref();
		children.push_back(child);
	}
}

void					SceneNodeGroup::popChild()
{
	children.back()->unref();
	children.pop_back();
}

void					SceneNodeGroup::clearChildren()
{
	for (List::iterator index = children.begin();
								index != children.end(); ++index)
		(*index)->unref();
	children.clear();
}

unsigned int				SceneNodeGroup::findChild(SceneNode* child) const
{
	for (List::const_iterator index = children.begin();
								index != children.end(); ++index)
		if (*index == child)
			return index - children.begin();
	return children.size();
}

bool					SceneNodeGroup::descend(
								SceneVisitor* visitor,
								const SceneVisitorParams&)
{
	for (List::const_iterator index = children.begin();
								index != children.end(); ++index)
		if (!(*index)->visit(visitor))
			return false;
	return true;
}

bool					SceneNodeGroup::visit(SceneVisitor* visitor)
{
	return visitor->visit(this);
}
// ex: shiftwidth=4 tabstop=4
