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

#include "SceneNodeLight.h"
#include "SceneVisitor.h"
#include "SceneVisitorParams.h"
#include <math.h>

//
// SceneNodeLight
//

static const float black[] = { 0.0f, 0.0f, 0.0f };
static const float white[] = { 1.0f, 1.0f, 1.0f };
static const float defaultPosition[] = { 0.0f, 0.0f, 1.0f, 0.0f };
static const float defaultDirection[] = { 0.0f, 0.0f, -1.0f };
static const float defaultAttenuation[] = { 1.0f, 0.0f, 0.0f };

SceneNodeLight::SceneNodeLight() :
								ambient("ambient", 0, 0, 3),
								diffuse("diffuse", 0, 0, 3),
								specular("specular", 0, 0, 3),
								position("position", 0, 0, 4),
								spotDirection("spotDirection", 0, 0, 3),
								spotExponent("spotExponent", 0, 0, 1),
								spotCutoff("spotCutoff", 0, 0, 1),
								attenuation("attenuation", 0, 0, 3)
{
	cAmbientColor[3]  = 1.0f;
	cDiffuseColor[3]  = 1.0f;
	cSpecularColor[3] = 1.0f;
	compute3Vec(cAmbientColor, 0, 0.0f, &ambient, black);
	compute3Vec(cDiffuseColor, 0, 0.0f, &diffuse, white);
	compute3Vec(cSpecularColor, 0, 0.0f, &specular, white);
	compute4Vec(cPosition, 0, 0.0f, &position, defaultPosition);
	compute3Vec(cSpotDirection, 0, 0.0f, &spotDirection, defaultDirection);
	compute1Vec(&cSpotExponent, 0, 0.0f, &spotExponent, 0.0f);
	compute1Vec(&cSpotCutoff, 0, 0.0f, &spotCutoff, 180.0f);
	compute3Vec(cAttenuation, 0, 0.0f, &attenuation, defaultAttenuation);
}

SceneNodeLight::~SceneNodeLight()
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
		t -= t0;									\
		index = static_cast<unsigned int>(t0);		\
	}

void					SceneNodeLight::compute(
								const SceneVisitorParams& params)
{
	static BzfString emptyName;
	float t = 0.0f, t0;
	unsigned int index = 0;
	BzfString tName = emptyName;

	// interpolate
	GET_T(ambient.getInterpolationParameter());
	compute3Vec(cAmbientColor, index, t, &ambient, black);
	GET_T(diffuse.getInterpolationParameter());
	compute3Vec(cDiffuseColor, index, t, &diffuse, white);
	GET_T(specular.getInterpolationParameter());
	compute3Vec(cSpecularColor, index, t, &specular, white);
	GET_T(position.getInterpolationParameter());
	compute4Vec(cPosition, index, t, &position, defaultPosition);
	GET_T(spotDirection.getInterpolationParameter());
	compute3Vec(cSpotDirection, index, t, &spotDirection, defaultDirection);
	GET_T(spotExponent.getInterpolationParameter());
	compute1Vec(&cSpotExponent, index, t, &spotExponent, 0.0f);
	GET_T(spotCutoff.getInterpolationParameter());
	compute1Vec(&cSpotCutoff, index, t, &spotCutoff, 180.0f);
	GET_T(attenuation.getInterpolationParameter());
	compute3Vec(cAttenuation, index, t, &attenuation, defaultAttenuation);

	// clamp
	if (cSpotExponent < 0.0f)
		cSpotExponent = 0.0f;
	else if (cSpotExponent > 128.0f)
		cSpotExponent = 128.0f;
	if (cSpotCutoff < 0.0f)
		cSpotCutoff = 0.0f;
	else if (cSpotCutoff > 90.0f)
		cSpotCutoff = 180.0f;
}

bool					SceneNodeLight::visit(SceneVisitor* visitor)
{
	return visitor->visit(this);
}

void					SceneNodeLight::compute1Vec(float* dst,
								unsigned int index, float t2,
								const SceneNodeVFFloat* vector,
								float defaultValue)
{
	unsigned int n = vector->getNum();
	if (n == 0) {
		dst[0] = defaultValue;
	}
	else if ((index + 1) >= n) {
		const float* v = vector->get() + n - 1;
		dst[0] = v[0];
	}
	else {
		const float t1 = 1.0f - t2;
		const float* v = vector->get() + index;
		dst[0] = t1 * v[0] + t2 * v[1];
	}
}

void					SceneNodeLight::compute3Vec(float* dst,
								unsigned int index, float t2,
								const SceneNodeVFFloat* vector,
								const float* defaultVector)
{
	unsigned int n = vector->getNum();
	if (n == 0) {
		dst[0] = defaultVector[0];
		dst[1] = defaultVector[1];
		dst[2] = defaultVector[2];
	}
	else if (3 * (index + 1) >= n) {
		const float* v = vector->get() + n - 3;
		dst[0] = v[0];
		dst[1] = v[1];
		dst[2] = v[2];
	}
	else {
		const float t1 = 1.0f - t2;
		const float* v = vector->get() + 3 * index;
		dst[0] = t1 * v[0] + t2 * v[3];
		dst[1] = t1 * v[1] + t2 * v[4];
		dst[2] = t1 * v[2] + t2 * v[5];
	}
}

void					SceneNodeLight::compute4Vec(float* dst,
								unsigned int index, float t2,
								const SceneNodeVFFloat* vector,
								const float* defaultVector)
{
	unsigned int n = vector->getNum();
	if (n == 0) {
		dst[0] = defaultVector[0];
		dst[1] = defaultVector[1];
		dst[2] = defaultVector[2];
		dst[3] = defaultVector[3];
	}
	else if (4 * (index + 1) >= n) {
		const float* v = vector->get() + n - 4;
		dst[0] = v[0];
		dst[1] = v[1];
		dst[2] = v[2];
		dst[3] = v[3];
	}
	else {
		const float t1 = 1.0f - t2;
		const float* v = vector->get() + 4 * index;
		dst[0] = t1 * v[0] + t2 * v[4];
		dst[1] = t1 * v[1] + t2 * v[5];
		dst[2] = t1 * v[2] + t2 * v[6];
		dst[3] = t1 * v[3] + t2 * v[7];
	}
}
