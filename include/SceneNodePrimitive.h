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

#ifndef BZF_SCENE_NODE_PRIMITIVE_H
#define BZF_SCENE_NODE_PRIMITIVE_H

#include "SceneNode.h"

class SceneNodeGeometry;

class SceneNodePrimitive : public SceneNode {
public:
	enum Type {
		Points,
		Lines,
		LineStrip,
		LineLoop,
		Triangles,
		TriangleStrip,
		TriangleFan
	};

	SceneNodePrimitive();

	// SceneNode overrides
	virtual bool		visit(SceneVisitor*);

	// fields
	SceneNodeSFEnum		type;
	SceneNodeVFUInt		index;

protected:
	virtual ~SceneNodePrimitive();
};

#endif
// ex: shiftwidth=4 tabstop=4
