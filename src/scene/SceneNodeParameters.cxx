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

#include "SceneNodeParameters.h"
#include "SceneVisitor.h"
#include "SceneVisitorParams.h"

//
// SceneNodeParameters
//

SceneNodeParameters::SceneNodeParameters() :
								src("src", 0, 0, 1),
								dst("dst", 0, 0, 1),
								scale("scale", 0, 0, 1),
								bias("bias", 0, 0, 1)
{
	// do nothing
}

SceneNodeParameters::~SceneNodeParameters()
{
	// do nothing
}

void					SceneNodeParameters::push(SceneVisitorParams& params)
{
	// get number of parameters
	unsigned int n = src.getNum();
	if (dst.getNum() < n)
		n = dst.getNum();
	if (scale.getNum() < n)
		n = scale.getNum();
	if (bias.getNum() < n)
		n = bias.getNum();

	// adjust and push
	for (unsigned int i = 0; i < n; ++i) {
		params.pushFloat(dst.get(i), scale.get(i) * (bias.get(i) +
								params.getFloat(src.get(i))));
	}
}

void					SceneNodeParameters::pop(SceneVisitorParams& params)
{
	// get number of parameters
	unsigned int n = src.getNum();
	if (dst.getNum() < n)
		n = dst.getNum();
	if (scale.getNum() < n)
		n = scale.getNum();
	if (bias.getNum() < n)
		n = bias.getNum();

	// pop
	for (unsigned int i = 0; i < n; ++i)
		params.popFloat(dst.get(i));
}

bool					SceneNodeParameters::visit(SceneVisitor* visitor)
{
	return visitor->visit(this);
}
// ex: shiftwidth=4 tabstop=4
