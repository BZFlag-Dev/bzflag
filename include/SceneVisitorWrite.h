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

#ifndef BZF_SCENE_VISITOR_WRITE_H
#define BZF_SCENE_VISITOR_WRITE_H

#include "SceneVisitor.h"
#include <string>
#include "bzfio.h"

class SceneVisitorWrite : public SceneVisitor {
public:
	SceneVisitorWrite(ostream*);
	virtual ~SceneVisitorWrite();

	// SceneVisitor overrides
	virtual bool		visit(SceneNodeAnimate*);
	virtual bool		visit(SceneNodeBillboard*);
	virtual bool		visit(SceneNodeGState*);
	virtual bool		visit(SceneNodeGeometry*);
	virtual bool		visit(SceneNodeGroup*);
	virtual bool		visit(SceneNodeLight*);
	virtual bool		visit(SceneNodeMatrixTransform*);
	virtual bool		visit(SceneNodeMetadata*);
	virtual bool		visit(SceneNodeParameters*);
	virtual bool		visit(SceneNodeParticleSystem*);
	virtual bool		visit(SceneNodePrimitive*);
	virtual bool		visit(SceneNodeSelector*);
	virtual bool		visit(SceneNodeTransform*);

protected:
	ostream*			getStream() const { return stream; }
	const char*			getIndent() const { return indentation.c_str(); }
	void				indent();
	void				exdent();

private:
	ostream*			stream;
	std::string			indentation;
};

#endif
