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

#ifndef BZF_SCENE_NODE_GSTATE_H
#define BZF_SCENE_NODE_GSTATE_H

#include "SceneNodeGroup.h"
#include "OpenGLGState.h"

class SceneNodeGState : public SceneNodeGroup {
public:
	SceneNodeGState();

	void				set(const OpenGLGState&);

	const OpenGLGState&	get() const { return gstate; }

	// SceneNode overrides
	virtual bool		visit(SceneVisitor*);

protected:
	virtual ~SceneNodeGState();

private:
	OpenGLGState		gstate;
};

#endif
// ex: shiftwidth=4 tabstop=4
