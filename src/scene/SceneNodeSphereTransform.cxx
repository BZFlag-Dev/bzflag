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

#include "SceneNodeSphereTransform.h"
#include "SceneVisitor.h"
#include "SceneVisitorParams.h"
#include "math3D.h"

//
// SceneNodeSphereTransform
//

SceneNodeSphereTransform::SceneNodeSphereTransform() :
								azimuth("azimuth", 0, 0, 1),
								altitude("altitude", 0, 0, 1),
								twist("twist", 0, 0, 1),
								radius("radius", 0, 0, 1),
								translate("translate", 0, 0, 3)
{
	// do nothing
}

SceneNodeSphereTransform::~SceneNodeSphereTransform()
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

void					SceneNodeSphereTransform::get(
								Matrix& m,
								const SceneVisitorParams& params)
{
	static std::string emptyName;
	float t, t0, t1 = 0.0f, t2 = 0.0f;
	unsigned int index = 0;
	std::string tName = emptyName;

	// matrix is Translate x Azimuth * Altitude * Twist * Radius.
	// azimuth about [0,0,1], altitude about [0,1,0], twist about [1,0,0].
	Matrix matrix;
	Matrix x;

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
		matrix *= x;
	}

	// azimuth
	n = azimuth.getNum();
	if (n > 0) {
		GET_T(azimuth.getInterpolationParameter());
		if ((index + 1) >= n) {
			// at or past end
			const float* v = azimuth.get() + n - 1;
			x.setRotate(0.0f, 0.0f, 1.0f, v[0]);
		}
		else {
			const float* v = azimuth.get() + index;
			x.setRotate(0.0f, 0.0f, 1.0f, t1 * v[0] + t2 * v[1]);
		}
		matrix *= x;
	}

	// altitude
	n = altitude.getNum();
	if (n > 0) {
		GET_T(altitude.getInterpolationParameter());
		if ((index + 1) >= n) {
			// at or past end
			const float* v = altitude.get() + n - 1;
			x.setRotate(0.0f, -1.0f, 0.0f, v[0]);
		}
		else {
			const float* v = altitude.get() + index;
			x.setRotate(0.0f, -1.0f, 0.0f, t1 * v[0] + t2 * v[1]);
		}
		matrix *= x;
	}

	// twist
	n = twist.getNum();
	if (n > 0) {
		GET_T(twist.getInterpolationParameter());
		if ((index + 1) >= n) {
			// at or past end
			const float* v = twist.get() + n - 1;
			x.setRotate(1.0f, 0.0f, 0.0f, v[0]);
		}
		else {
			const float* v = twist.get() + index;
			x.setRotate(1.0f, 0.0f, 0.0f, t1 * v[0] + t2 * v[1]);
		}
		matrix *= x;
	}

	// radius
	n = radius.getNum();
	if (n > 0) {
		GET_T(radius.getInterpolationParameter());
		if ((index + 1) >= n) {
			// at or past end
			const float* v = radius.get() + n - 1;
			x.setScale(v[0], v[0], v[0]);
		}
		else {
			const float* v = radius.get() + index;
			const float r = t1 * v[0] + t2 * v[1];
			x.setScale(r, r, r);
		}
		matrix *= x;
	}

	// apply transform
	m *= matrix;
}

bool					SceneNodeSphereTransform::visit(SceneVisitor* visitor)
{
	return visitor->visit(this);
}
