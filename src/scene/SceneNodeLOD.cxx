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

#include "SceneNodeLOD.h"
#include "SceneVisitor.h"
#include "SceneVisitorParams.h"
#include "Matrix.h"
#include <math.h>

//
// SceneNodeLOD
//

static const char* typeEnum[] = { "depth", "area" };

SceneNodeLOD::SceneNodeLOD() : type("type", Depth, typeEnum, countof(typeEnum)),
								sphere("sphere", 0, 4, 4),
								range("range", 0, 0, 2)
{
	// do nothing
}

SceneNodeLOD::~SceneNodeLOD()
{
	// do nothing
}

unsigned int				SceneNodeLOD::get(
								const Matrix& view,
								const Matrix& proj,
								const SceneVisitorParams&)
{
	float d;

	// if no sphere or no ranges then no children
	if (sphere.getNum() < 4 || range.getNum() == 0)
		return 0;

	// transform origin
	const float* p  = sphere.get();
	const float* vm = view.get();
	const float* pm = proj.get();
	float x = p[0] * vm[0] + p[1] * vm[4] + p[2] * vm[8]  + vm[12];
	float y = p[0] * vm[1] + p[1] * vm[5] + p[2] * vm[9]  + vm[13];
	float z = p[0] * vm[2] + p[1] * vm[6] + p[2] * vm[10] + vm[14];
	float w = x    * pm[3] + y    * pm[7] + z    * pm[11] + pm[15];
	z       = x    * pm[2] + y    * pm[6] + z    * pm[10] + pm[14];

	// compute range value
	if (type.get() == Depth) {
		d = z / w;
	}
	else {
		// FIXME -- this doesn't handle non-uniform scale or skew correctly
		d = p[3] * hypotf(vm[0], hypotf(vm[1], vm[2])) / w;
		d = p[3] * hypotf(vm[4], hypotf(vm[5], vm[6])) / w;
	}

	// find ranges that match
	unsigned int mask = 0u;
	unsigned int bit  = 1u;
	const unsigned int n = range.getNum();
	for (unsigned int i = 0; i < n; bit <<= 1, i += 2)
		if (d >= range.get(i + 0) && d < range.get(i + 1))
			mask |= bit;

	return mask;
}

bool					SceneNodeLOD::visit(SceneVisitor* visitor)
{
	return visitor->visit(this);
}
