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

#ifndef BZF_SCENE_NODE_SELECTOR_H
#define BZF_SCENE_NODE_SELECTOR_H

#include "SceneNodeGroup.h"

class Matrix;

class SceneNodeSelector : public SceneNodeGroup {
public:
	SceneNodeSelector();

	// get the node's choice of mask
	virtual unsigned int get(const Matrix& modelViewXForm,
							const Matrix& projectionXForm,
							const SceneVisitorParams&) = 0;

	// visit children in mask
	virtual bool		descend(SceneVisitor*, const SceneVisitorParams&);

	// SceneNode overrides
	virtual bool		visit(SceneVisitor*);

protected:
	virtual ~SceneNodeSelector();
};

#endif
