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

#include "SceneVisitor.h"

//
// SceneVisitor
//

SceneVisitor::SceneVisitor()
{
	params.pushFloat("t", 0.0f);
	params.pushInt("mask", 0);
}

SceneVisitor::~SceneVisitor()
{
	// do nothing
}

bool					SceneVisitor::descend(SceneNodeGroup* n)
{
	return n->descend(this, getParams());
}
// ex: shiftwidth=4 tabstop=4
