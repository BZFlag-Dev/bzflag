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
#include "math3D.h"

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
	// if no sphere or no ranges then no children
	if (sphere.getNum() < 4 || range.getNum() == 0)
		return 0;

	// transform origin
	Vec3 v(sphere.get());
	v.xformPoint(view);
	Real w = v[0] * proj[3] + v[1] * proj[7] + v[2] * proj[11] + proj[15];
	Real z = v[0] * proj[2] + v[1] * proj[6] + v[2] * proj[10] + proj[14];

	// compute range value
	float d;
	if (type.get() == Depth) {
		d = static_cast<float>(z / w);
	}
	else {
		// FIXME -- this doesn't handle non-uniform scale or skew correctly
		d = static_cast<float>(sphere.get()[3] *
								sqrtr(view[0] * view[0] +
									view[1] * view[1] +
									view[2] * view[2]) / w);
		d = static_cast<float>(sphere.get()[3] *
								sqrtr(view[4] * view[4] +
									view[5] * view[5] +
									view[6] * view[6]) / w);
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
