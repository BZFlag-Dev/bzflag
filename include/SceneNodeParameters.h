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

#ifndef BZF_SCENE_NODE_PARAMETERS_H
#define BZF_SCENE_NODE_PARAMETERS_H

#include "SceneNodeGroup.h"

class SceneNodeParameters : public SceneNodeGroup {
public:
	SceneNodeParameters();

	// push/pop parameters
	virtual void		push(SceneVisitorParams&);
	virtual void		pop(SceneVisitorParams&);

	// SceneNode overrides
	virtual bool		visit(SceneVisitor*);

	// fields
	SceneNodeVFString	src;
	SceneNodeVFString	dst;
	SceneNodeVFFloat	scale;
	SceneNodeVFFloat	bias;

protected:
	virtual ~SceneNodeParameters();
};

#endif
