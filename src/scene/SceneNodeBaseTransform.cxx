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

#include "SceneNodeBaseTransform.h"
#include "SceneVisitor.h"

//
// SceneNodeBaseTransform
//

static const char* typeEnums[] = { "modelview", "projection", "texture" };

SceneNodeBaseTransform::SceneNodeBaseTransform() :
								type("type", ModelView,
										typeEnums, countof(typeEnums)),
								up("up", 0)
{
	// do nothing
}

SceneNodeBaseTransform::~SceneNodeBaseTransform()
{
	// do nothing
}

bool					SceneNodeBaseTransform::visit(SceneVisitor* visitor)
{
	return visitor->visit(this);
}
