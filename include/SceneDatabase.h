/* bzflag
 * Copyright (c) 1993 - 2003 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* SceneDatabase:
 *	Interface for database of geometry to render
 */

#ifndef	BZF_SCENE_DATABASE_H
#define	BZF_SCENE_DATABASE_H

#include "bzfgl.h"
#include "common.h"

class ViewFrustum;
class SceneNode;
class SphereSceneNode;

// NOTE -- SceneDatabase owns all static nodes added to it,
//	dynamic nodes are the responsibility of the client.

class SceneIterator {
  public:
			SceneIterator();
    virtual		~SceneIterator();

    virtual void	resetFrustum(const ViewFrustum*) = 0;
    virtual void	reset() = 0;
    virtual SceneNode*	getNext() = 0;
};

class SceneDatabase {
  public:
			SceneDatabase();
    virtual		~SceneDatabase();

    virtual void	addStaticNode(SceneNode*) = 0;
    virtual void	addDynamicNode(SceneNode*) = 0;
    virtual void	addDynamicSphere(SphereSceneNode*) = 0;
    virtual void	removeDynamicNodes() = 0;
    virtual void	removeAllNodes() = 0;
    virtual bool	isOrdered() = 0;

    virtual SceneIterator*	getRenderIterator() = 0;

  private:
			SceneDatabase(const SceneDatabase&);
    SceneDatabase&	operator=(const SceneDatabase&);
};

#endif // BZF_SCENE_DATABASE_H
// ex: shiftwidth=2 tabstop=8
