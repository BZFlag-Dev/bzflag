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

#include "SceneNodeBillboard.h"
#include "SceneVisitor.h"
#include "Matrix.h"
#include "ViewFrustum.h"
#include <math.h>

//
// SceneNodeBillboard
//

SceneNodeBillboard::SceneNodeBillboard() :
								axis("axis", 0, 3, 3),
								turn("turn", true),
								center("center", false)
{
	// do nothing
}

SceneNodeBillboard::~SceneNodeBillboard()
{
	// do nothing
}

void					SceneNodeBillboard::get(
								Matrix& m,
								const SceneVisitorParams&)
{
	// FIXME -- should use axis

/*
	// transform the origin by the top model matrix to get the vector
	// to the billboard object (the direction).
	float x = m[12];
	float y = m[13];
	float z = m[14];
	float d = 1.0f / hypotf(x, hypotf(y, z));
fprintf(stderr, "v: %f %f %f\n", x, y, z);

	// rotate z axis ([0 0 1]) onto direction (x y z).  the axis
	// is the cross product of those normalized vectors.  the cosine
	// of the angle is the dot product of the normalized vectors.
	float ax = d * -y;
	float ay = 0.0f;
	float az = d *  z;
	float c  = d * x;
	if (c > 1.0f)
		c = 1.0f;
	float s  = sqrtf(1.0f - c * c);
fprintf(stderr, "r1: %f %f %f %f %f\n", ax, ay, az, c, s);
	float m1[16];
	SceneNodeXForm::loadIdentity(m1);
	m1[0]  = ax * ax * (1 - c) + c;
	m1[1]  = ax * ay * (1 - c) + az * s;
	m1[2]  = ax * az * (1 - c) - ay * s;
	m1[4]  = ay * ax * (1 - c) - az * s;
	m1[5]  = ay * ay * (1 - c) + c;
	m1[6]  = ay * az * (1 - c) + ax * s;
	m1[8]  = az * ax * (1 - c) + ay * s;
	m1[9]  = az * ay * (1 - c) - ax * s;
	m1[10] = az * az * (1 - c) + c;

	// now rotate the billboard object's z axis transformed by the above
	// matrix around the transformed y axis to [0 0 1].
	c = m1[10];
	s = sqrtf(1.0 - c * c);
fprintf(stderr, "r2: %f %f\n", c, s);
	float m2[16];
	SceneNodeXForm::loadIdentity(m2);
	m2[0]  =  c;
	m2[1]  =  0.0f;
	m2[2]  = -s;
	m2[4]  =  0.0f;
	m2[5]  =  1.0f;
	m2[6]  =  0.0f;
	m2[8]  =  s;
	m2[9]  =  0.0f;
	m2[10] =  c;

	// combine transforms
	SceneNodeXForm::mult(m2, m1);

	// multiply the matrix
	SceneNodeXForm::mult(m, m2);
*/
	// FIXME -- this is too specific.  make a general billboard and use axis.
	if (turn.get() && axis.getNum() == 3) {
		float* matrix = m.get();
		float x = hypotf(matrix[0], hypotf(matrix[4], matrix[8]));
		float y = hypotf(matrix[1], hypotf(matrix[5], matrix[9]));
		float z = hypotf(matrix[2], hypotf(matrix[6], matrix[10]));
		matrix[0]  =  x;
		matrix[1]  =  0.0f;
		matrix[2]  =  0.0f;
		matrix[4]  =  0.0f;
		matrix[5]  =  y;
		matrix[6]  =  0.0f;
		matrix[8]  =  0.0f;
		matrix[9]  =  0.0f;
		matrix[10] =  z;

		// undo view frustum coordinate transform
		m.mult(ViewFrustum::getTransform());
	}
	if (center.get()) {
		float* matrix = m.get();
		matrix[12] = 0.0f;
		matrix[13] = 0.0f;
		matrix[14] = 0.0f;
	}
}

bool					SceneNodeBillboard::visit(SceneVisitor* visitor)
{
	return visitor->visit(this);
}
