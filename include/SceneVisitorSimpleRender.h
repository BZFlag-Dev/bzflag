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

#ifndef BZF_SCENE_VISITOR_SIMPLE_RENDER_H
#define BZF_SCENE_VISITOR_SIMPLE_RENDER_H

#include "SceneVisitorBaseRender.h"
#include "OpenGLGState.h"
#include "Matrix.h"
#include <vector>

class SceneVisitorSimpleRender : public SceneVisitorBaseRender {
public:
	SceneVisitorSimpleRender();
	virtual ~SceneVisitorSimpleRender();

	// SceneVisitor overrides
	virtual bool		traverse(SceneNode*);
	virtual bool		visit(SceneNodeAnimate*);
	virtual bool		visit(SceneNodeBaseTransform*);
	virtual bool		visit(SceneNodeGState*);
	virtual bool		visit(SceneNodeGeometry*);
	virtual bool		visit(SceneNodeLight*);
	virtual bool		visit(SceneNodeParameters*);
	virtual bool		visit(SceneNodeParticleSystem*);
	virtual bool		visit(SceneNodePrimitive*);
	virtual bool		visit(SceneNodeSelector*);

private:
	typedef std::vector<OpenGLGState> GStateStack;
	typedef std::vector<Matrix> XFormStack;
	typedef std::vector<const SceneNodeVFFloat*> GeometryStack;
	typedef std::vector<float> ShininessStack;

	GStateStack			gstateStack;
	XFormStack			modelXFormStack;
	XFormStack			projectionXFormStack;
	XFormStack			textureXFormStack;
	GeometryStack		stippleStack;
	GeometryStack		colorStack;
	GeometryStack		texcoordStack;
	GeometryStack		normalStack;
	GeometryStack		vertexStack;
	GeometryStack		specularStack;
	GeometryStack		emissiveStack;
	ShininessStack		shininessStack;
	int					numLights;
	int					maxLights;
	bool				lighting;
};

#endif
