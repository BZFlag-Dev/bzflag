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

#ifndef BZF_SCENE_VISITOR_H
#define BZF_SCENE_VISITOR_H

#include "common.h"
#include "SceneNodes.h"
#include "SceneVisitorParams.h"

class SceneVisitor {
public:
	SceneVisitor();
	virtual ~SceneVisitor();

	SceneVisitorParams&			getParams() { return params; }
	const SceneVisitorParams&	getParams() const { return params; }

	virtual bool		traverse(SceneNode* n)
							{ return (n != NULL) ? n->visit(this) : false; }

	virtual bool		visit(SceneNode*) { return true; }
	virtual bool		visit(SceneNodeAnimate* n)
							{ return visit(static_cast<SceneNodeGroup*>(n)); }
	virtual bool		visit(SceneNodeBaseTransform* n)
							{ return visit(static_cast<SceneNodeGroup*>(n)); }
	virtual bool		visit(SceneNodeBillboard* n)
							{ return visit(static_cast<SceneNodeBaseTransform*>(n)); }
	virtual bool		visit(SceneNodeChoice* n)
							{ return visit(static_cast<SceneNodeSelector*>(n)); }
	virtual bool		visit(SceneNodeGState* n)
							{ return visit(static_cast<SceneNodeGroup*>(n)); }
	virtual bool		visit(SceneNodeGeometry* n)
							{ return visit(static_cast<SceneNodeGroup*>(n)); }
	virtual bool		visit(SceneNodeGroup* n)
							{ return descend(n); }
	virtual bool		visit(SceneNodeLight* n)
							{ return visit(static_cast<SceneNodeGroup*>(n)); }
	virtual bool		visit(SceneNodeLOD* n)
							{ return visit(static_cast<SceneNodeSelector*>(n)); }
	virtual bool		visit(SceneNodeMatrixTransform* n)
							{ return visit(static_cast<SceneNodeBaseTransform*>(n)); }
	virtual bool		visit(SceneNodeMetadata* n)
							{ return visit(static_cast<SceneNodeGroup*>(n)); }
	virtual bool		visit(SceneNodeParameters* n)
							{ return visit(static_cast<SceneNodeGroup*>(n)); }
	virtual bool		visit(SceneNodePrimitive* n)
							{ return visit(static_cast<SceneNode*>(n)); }
	virtual bool		visit(SceneNodeSelector* n)
							{ return visit(static_cast<SceneNodeGroup*>(n)); }
	virtual bool		visit(SceneNodeSphereTransform* n)
							{ return visit(static_cast<SceneNodeBaseTransform*>(n)); }
	virtual bool		visit(SceneNodeTransform* n)
							{ return visit(static_cast<SceneNodeBaseTransform*>(n)); }

protected:
	virtual bool		descend(SceneNodeGroup*);

private:
	SceneVisitorParams	params;
};

#endif
