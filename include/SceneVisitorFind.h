/* bzflag
 * Copyright (c) 1993 - 2001 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef BZF_SCENE_VISITOR_FIND_H
#define BZF_SCENE_VISITOR_FIND_H

#include "SceneVisitor.h"
#include "BzfString.h"

class SceneVisitorFind : public SceneVisitor {
public:
	SceneVisitorFind(const BzfString& id);
	virtual ~SceneVisitorFind();

	// get() does *not* ref the node
	SceneNode*			get() const;

	// SceneVisitor overrides
	virtual bool		visit(SceneNode*);
	virtual bool		visit(SceneNodeGroup*);

private:
	BzfString			id;
	SceneNode*			found;
};

#endif
