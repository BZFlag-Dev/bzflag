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

#include "SceneNodeSelector.h"
#include "SceneVisitor.h"
#include "SceneVisitorParams.h"
#include <assert.h>

//
// SceneNodeSelector
//

SceneNodeSelector::SceneNodeSelector()
{
	// do nothing
}

SceneNodeSelector::~SceneNodeSelector()
{
	// do nothing
}

bool					SceneNodeSelector::descend(
								SceneVisitor* visitor,
								const SceneVisitorParams& params)
{
	static const BzfString s_mask("mask");

	// get number of children and limit to mask size
	unsigned int n = size();
	if (n > 32)
		n = 32;

	// descend into children in mask
	unsigned int m = static_cast<unsigned int>(params.getInt(s_mask));
	for (unsigned int i = 0, d = 1; i < n; d <<= 1, ++i)
		if ((d & m) != 0)
			if (!getChild(i)->visit(visitor))
				return false;

	return true;
}

bool					SceneNodeSelector::visit(SceneVisitor* visitor)
{
	return visitor->visit(this);
}
