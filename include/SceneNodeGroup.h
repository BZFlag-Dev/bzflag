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

#ifndef BZF_SCENE_NODE_GROUP_H
#define BZF_SCENE_NODE_GROUP_H

#include "SceneNode.h"
#include <vector>

class SceneVisitorParams;

class SceneNodeGroup : public SceneNode {
public:
	SceneNodeGroup();

	// modify list of children
	void				insertChild(unsigned int index, SceneNode*);
	void				eraseChild(unsigned int index);
	void				pushChild(SceneNode*);
	void				popChild();
	void				clearChildren();

	// query.  find() returns size() if not found.  get() does not
	// ref() the child.
	unsigned int		size() const { return children.size(); }
	unsigned int		findChild(SceneNode*) const;
	SceneNode*			getChild(unsigned int index) { return children[index]; }

	// visit children
	virtual bool		descend(SceneVisitor*, const SceneVisitorParams&);

	// SceneNode overrides
	virtual bool		visit(SceneVisitor*);

protected:
	virtual ~SceneNodeGroup();

private:
	typedef std::vector<SceneNode*> List;
	List				children;
};

#endif
// ex: shiftwidth=4 tabstop=4
