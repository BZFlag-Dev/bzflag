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

#ifndef BZF_SCENE_NODE_LOD_H
#define BZF_SCENE_NODE_LOD_H

#include "SceneNodeSelector.h"

class SceneNodeLOD : public SceneNodeSelector {
public:
	enum Type { Depth, Area };

	SceneNodeLOD();

	// SceneNode overrides
	virtual bool		visit(SceneVisitor*);

	// SceneNodeSelector overrides
	virtual unsigned int get(const Matrix& modelViewXForm,
							const Matrix& projectionXForm,
							const SceneVisitorParams&);

	// fields.  LOD is by depth or by projected area.  sphere is
	// transformed by the current matrix to compute the depth or
	// projected area.  range contains pairs or depths or areas.
	// for each pair, if the depth or area is >= the first and
	// < the second then the bit corresponding to that pair is
	// set in the mask, otherwise that bit is reset.
	SceneNodeSFEnum		type;
	SceneNodeVFFloat	sphere;				// 4-vector (x,y,z,radius)
	SceneNodeVFFloat	range;				// 2-vector * n (min,max range)

protected:
	virtual ~SceneNodeLOD();
};

#endif
