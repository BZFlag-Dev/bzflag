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

#ifndef BZF_SCENE_NODE_SPHERE_TRANSFORM_H
#define BZF_SCENE_NODE_SPHERE_TRANSFORM_H

#include "SceneNodeBaseTransform.h"

class SceneNodeSphereTransform : public SceneNodeBaseTransform {
public:
	SceneNodeSphereTransform();

	// get the transformation
	virtual void		get(Matrix& currentXForm,
							const SceneVisitorParams&);

	// SceneNode overrides
	virtual bool		visit(SceneVisitor*);

	// fields
	SceneNodeVFFloat	azimuth;				// 1-vector * n
	SceneNodeVFFloat	altitude;				// 1-vector * n
	SceneNodeVFFloat	twist;					// 1-vector * n
	SceneNodeVFFloat	radius;					// 1-vector * n
	SceneNodeVFFloat	translate;				// 3-vector * n

protected:
	virtual ~SceneNodeSphereTransform();
};

#endif
// ex: shiftwidth=4 tabstop=4
