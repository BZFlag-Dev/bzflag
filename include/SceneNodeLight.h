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

#ifndef BZF_SCENE_NODE_LIGHT_H
#define BZF_SCENE_NODE_LIGHT_H

#include "SceneNodeGroup.h"

class SceneNodeLight : public SceneNodeGroup {
public:
	SceneNodeLight();

	// compute the current parameters
	void				compute(const SceneVisitorParams&);

	// SceneNode overrides
	virtual bool		visit(SceneVisitor*);

	// fields
	SceneNodeVFFloat	ambient;				// 3-vector * n
	SceneNodeVFFloat	diffuse;				// 3-vector * n
	SceneNodeVFFloat	specular;				// 3-vector * n
	SceneNodeVFFloat	position;				// 4-vector * n
	SceneNodeVFFloat	spotDirection;			// 3-vector * n
	SceneNodeVFFloat	spotExponent;			// 1-vector * n
	SceneNodeVFFloat	spotCutoff;				// 1-vector * n
	SceneNodeVFFloat	attenuation;			// 3-vector * n

	// computed parameters.  colors are RGBA with A=1
	const float*		getAmbientColor() const   { return cAmbientColor; }
	const float*		getDiffuseColor() const   { return cDiffuseColor; }
	const float*		getSpecularColor() const  { return cSpecularColor; }
	const float*		getPosition() const       { return cPosition; }
	const float*		getSpotDirection() const  { return cSpotDirection; }
	float				getSpotExponent() const   { return cSpotExponent; }
	float				getSpotCutoff() const     { return cSpotCutoff; }
	const float*		getAttenuation() const    { return cAttenuation; }

protected:
	virtual ~SceneNodeLight();

	void				compute1Vec(float* dst,
							unsigned int index, float t,
							const SceneNodeVFFloat*,
							float defaultValue);
	void				compute3Vec(float* dst,
							unsigned int index, float t,
							const SceneNodeVFFloat*,
							const float* defaultVector);
	void				compute4Vec(float* dst,
							unsigned int index, float t,
							const SceneNodeVFFloat*,
							const float* defaultVector);

private:
	float				cAmbientColor[4];
	float				cDiffuseColor[4];
	float				cSpecularColor[4];
	float				cPosition[4];
	float				cSpotDirection[3];
	float				cSpotExponent;
	float				cSpotCutoff;
	float				cAttenuation[3];
};

#endif
