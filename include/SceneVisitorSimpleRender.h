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

#include "SceneVisitor.h"
#include "OpenGLGState.h"
#include "Matrix.h"
#include <vector>

class SceneVisitorSimpleRender : public SceneVisitor {
public:
	SceneVisitorSimpleRender();
	virtual ~SceneVisitorSimpleRender();

	// set the viewport size (in pixels)
	void				setArea(float size);

	// set the current time and frame
	void				setTime(float t);
	void				setFrame(float frameNumber);

	// instrumentation methods.  counts are reset with each call to
	// traverse().
	struct Instruments {
	public:
		float			time;
		unsigned int	nNodes;
		unsigned int	nTransforms;
		unsigned int	nPoints;
		unsigned int	nLines;
		unsigned int	nTriangles;
	};
	const Instruments*	instrGet() const;

	// SceneVisitor overrides
	virtual bool		traverse(SceneNode*);
	virtual bool		visit(SceneNodeAnimate*);
	virtual bool		visit(SceneNodeBaseTransform*);
	virtual bool		visit(SceneNodeGState*);
	virtual bool		visit(SceneNodeGeometry*);
	virtual bool		visit(SceneNodeLight*);
	virtual bool		visit(SceneNodeParameters*);
	virtual bool		visit(SceneNodePrimitive*);
	virtual bool		visit(SceneNodeSelector*);

protected:
	// SceneVisitor overrides
	virtual bool		descend(SceneNodeGroup*);

private:
	typedef std::vector<OpenGLGState> GStateStack;
	typedef std::vector<Matrix> XFormStack;
	typedef std::vector<const SceneNodeVFFloat*> GeometryStack;
	typedef std::vector<float> ShininessStack;

	GStateStack			gstateStack;
	XFormStack			modelXFormStack;
	XFormStack			projectionXFormStack;
	XFormStack			textureXFormStack;
	GeometryStack		colorStack;
	GeometryStack		texcoordStack;
	GeometryStack		normalStack;
	GeometryStack		vertexStack;
	GeometryStack		specularStack;
	GeometryStack		emissiveStack;
	ShininessStack		shininessStack;
	int					numLights;
	int					maxLights;

	Instruments			instruments;
};

#endif
