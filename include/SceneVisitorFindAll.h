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

#ifndef BZF_SCENE_VISITOR_FIND_ALL_H
#define BZF_SCENE_VISITOR_FIND_ALL_H

#include "SceneVisitor.h"
#include <vector>

class SceneNode;

class SceneVisitorFindAll : public SceneVisitor {
public:
	SceneVisitorFindAll();
	virtual ~SceneVisitorFindAll();

	// get the number of named nodes
	unsigned int		size() const;

	// get a node by index.  this does *not* ref the node.
	SceneNode*			get(unsigned int index) const;

	// SceneVisitor overrides
	virtual bool		visit(SceneNode*);
	virtual bool		visit(SceneNodeGroup*);

private:
	typedef std::vector<SceneNode*> Nodes;
	Nodes				found;
};

#endif
