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

#ifndef BZF_SCENE_NODE_CHOICE_H
#define BZF_SCENE_NODE_CHOICE_H

#include "SceneNodeSelector.h"

class SceneNodeChoice : public SceneNodeSelector {
public:
	SceneNodeChoice();

	// SceneNode overrides
	virtual bool		visit(SceneVisitor*);

	// SceneNodeSelector overrides
	virtual unsigned int get(const Matrix& modelViewXForm,
							const Matrix& projectionXForm,
							const SceneVisitorParams&);

	// fields
	SceneNodeVFUInt		mask;				// 1-vector * n

protected:
	virtual ~SceneNodeChoice();
};

#endif
