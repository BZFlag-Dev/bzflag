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

#ifndef BZF_SCENE_NODE_BILLBOARD_H
#define BZF_SCENE_NODE_BILLBOARD_H

#include "SceneNodeBaseTransform.h"

class SceneNodeBillboard : public SceneNodeBaseTransform {
public:
	SceneNodeBillboard();

	// SceneNode overrides
	virtual bool		visit(SceneVisitor*);

	// SceneNodeXForm overrides
	virtual void		get(Matrix& currentXForm,
							const SceneVisitorParams&);

	// fields
	SceneNodeVFFloat	axis;		// 3-vector
	SceneNodeSFBool		turn;
	SceneNodeSFBool		center;

protected:
	virtual ~SceneNodeBillboard();
};

#endif
// ex: shiftwidth=4 tabstop=4
