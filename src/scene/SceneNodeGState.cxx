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

#include "SceneNodeGState.h"
#include "SceneVisitor.h"

//
// SceneNodeGState
//

SceneNodeGState::SceneNodeGState()
{
	// do nothing
}

SceneNodeGState::~SceneNodeGState()
{
	// do nothing
}

void					SceneNodeGState::set(const OpenGLGState& _gstate)
{
	gstate = _gstate;
}

bool					SceneNodeGState::visit(SceneVisitor* visitor)
{
	return visitor->visit(this);
}
