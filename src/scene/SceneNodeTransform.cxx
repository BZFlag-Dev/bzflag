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

#include "SceneNodeTransform.h"
#include "SceneVisitor.h"
#include "SceneVisitorParams.h"
#include "Matrix.h"
#include <math.h>

//
// SceneNodeTransform
//

SceneNodeTransform::SceneNodeTransform() :
								center("center", 0, 0, 3),
								rotate("rotate", 0, 0, 4),
								scale("scale", 0, 0, 3),
								scaleOrientation("scaleOrientation", 0, 0, 4),
								translate("translate", 0, 0, 3)
{
	// do nothing
}

SceneNodeTransform::~SceneNodeTransform()
{
	// do nothing
}

#define GET_T(__t)									\
	if (__t != tName) {								\
		tName = __t;								\
		t  = params.getFloat(tName);				\
		if (t < 0.0f)								\
			t = 0.0f;								\
		t0 = floorf(t);								\
		t2 = t - t0;								\
		t1 = 1.0f - t2;								\
		index = static_cast<unsigned int>(t0);		\
	}

void					SceneNodeTransform::get(
								Matrix& m,
								const SceneVisitorParams& params)
{
	static BzfString emptyName;
	float t, t0, t1 = 0.0f, t2 = 0.0f;
	unsigned int index = 0;
	BzfString tName = emptyName;

	// matrix is T x C x R x SO x S x -SO x -C (where -M is M inverse)
	Matrix matrix;
	Matrix x, c, so;

	// translation T
	unsigned int n = translate.getNum();
	if (n > 0) {
		GET_T(translate.getInterpolationParameter());
		if (3 * (index + 1) >= n) {
			// at or past end
			const float* v = translate.get() + n - 3;
			x.setTranslate(v[0], v[1], v[2]);
		}
		else {
			const float* v = translate.get() + 3 * index;
			x.setTranslate(t1 * v[0] + t2 * v[3],
						t1 * v[1] + t2 * v[4],
						t1 * v[2] + t2 * v[5]);
		}
		matrix.mult(x);
	}

	// translation C
	n = center.getNum();
	if (n > 0) {
		GET_T(center.getInterpolationParameter());
		if (3 * (index + 1) >= n) {
			// at or past end
			const float* v = center.get() + n - 3;
			c.setTranslate(v[0], v[1], v[2]);
		}
		else {
			const float* v = center.get() + 3 * index;
			c.setTranslate(t1 * v[0] + t2 * v[3],
						t1 * v[1] + t2 * v[4],
						t1 * v[2] + t2 * v[5]);
		}
		matrix.mult(c);

		// prepare for inverse
		c[12] = -c[12];
		c[13] = -c[13];
		c[14] = -c[14];
	}

	// rotation R
	n = rotate.getNum();
	if (n > 0) {
		GET_T(rotate.getInterpolationParameter());
		if (4 * (index + 1) >= n) {
			// at or past end
			const float* v = rotate.get() + n - 4;
			x.setRotate(v[0], v[1], v[2], v[3]);
		}
		else {
			const float* v = rotate.get() + 4 * index;
			x.setRotate(t1 * v[0] + t2 * v[4],
						t1 * v[1] + t2 * v[5],
						t1 * v[2] + t2 * v[6],
						t1 * v[3] + t2 * v[7]);
		}
		matrix.mult(x);
	}

	// rotation SO
	n = scaleOrientation.getNum();
	if (n > 0) {
		GET_T(scaleOrientation.getInterpolationParameter());
		if (4 * (index + 1) >= n) {
			// at or past end
			const float* v = scaleOrientation.get() + n - 4;
			so.setRotate(v[0], v[1], v[2], v[3]);
		}
		else {
			const float* v = scaleOrientation.get() + 4 * index;
			so.setRotate(t1 * v[0] + t2 * v[4],
						t1 * v[1] + t2 * v[5],
						t1 * v[2] + t2 * v[6],
						t1 * v[3] + t2 * v[7]);
		}
		matrix.mult(so);

		// prepare for inverse (transpose is inverse of orthonormal transform)
		so.transpose();
	}

	// scale S
	n = scale.getNum();
	if (n > 0) {
		GET_T(scale.getInterpolationParameter());
		if (3 * (index + 1) >= n) {
			// at or past end
			const float* v = scale.get() + n - 3;
			x.setScale(v[0], v[1], v[2]);
		}
		else {
			const float* v = scale.get() + 3 * index;
			x.setScale(t1 * v[0] + t2 * v[3],
						t1 * v[1] + t2 * v[4],
						t1 * v[2] + t2 * v[5]);
		}
		matrix.mult(x);
	}

	// rotation -SO
	if (scaleOrientation.getNum() > 0)
		matrix.mult(so);

	// translation -C
	if (center.getNum() > 0)
		matrix.mult(c);

	// apply transform
	m.mult(matrix);
}

bool					SceneNodeTransform::visit(SceneVisitor* visitor)
{
	return visitor->visit(this);
}
