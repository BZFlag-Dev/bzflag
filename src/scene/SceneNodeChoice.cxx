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

#include "SceneNodeChoice.h"
#include "SceneVisitor.h"
#include "SceneVisitorParams.h"

//
// SceneNodeChoice
//

SceneNodeChoice::SceneNodeChoice() : mask("mask", 0, 0, 1)
{
	// do nothing
}

SceneNodeChoice::~SceneNodeChoice()
{
	// do nothing
}

unsigned int				SceneNodeChoice::get(
								const Matrix&,
								const Matrix&,
								const SceneVisitorParams& params)
{
	// get animation parameter
	float t = params.getFloat(mask.getInterpolationParameter());

	// clamp
	if (t < 0.0f)
		t = 0.0f;

	// compute index
	unsigned int index = static_cast<unsigned int>(t);

	// choose mask
	unsigned int n = mask.getNum();
	if (n == 0)
		return 0xffffffff;
	else if (index >= n)
		return mask.get()[n - 1];
	else
		return mask.get()[index];
}

bool					SceneNodeChoice::visit(SceneVisitor* visitor)
{
	return visitor->visit(this);
}
