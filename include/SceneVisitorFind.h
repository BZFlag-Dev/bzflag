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

#ifndef BZF_SCENE_VISITOR_FIND_H
#define BZF_SCENE_VISITOR_FIND_H

#include "SceneVisitor.h"
#include <string>

class SceneVisitorFind : public SceneVisitor {
public:
	SceneVisitorFind(const std::string& id);
	virtual ~SceneVisitorFind();

	// get() does *not* ref the node
	SceneNode*			get() const;

	// SceneVisitor overrides
	virtual bool		visit(SceneNode*);
	virtual bool		visit(SceneNodeGroup*);

private:
	std::string			id;
	SceneNode*			found;
};

#endif
