/* bzflag
 * Copyright (c) 1993 - 2005 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* SceneDatabase:
 *	Interface for database of geometry to render
 */

#ifndef	BZF_SCENE_DATABASE_H
#define	BZF_SCENE_DATABASE_H

#include "common.h"
#ifndef BUILDING_BZADMIN
#include "bzfgl.h"
#endif

class ViewFrustum;
class SceneNode;
class SphereSceneNode;
class SceneRenderer;


// NOTE -- SceneDatabase owns all static nodes added to it,
//	dynamic nodes are the responsibility of the client.

class SceneDatabase {
  public:
			SceneDatabase();
    virtual		~SceneDatabase();

    // returns true if the node would have been deleted
    virtual bool	addStaticNode(SceneNode*, bool dontFree) = 0;
    virtual void	addDynamicNode(SceneNode*) = 0;
    virtual void	addDynamicSphere(SphereSceneNode*) = 0;
    virtual void	removeDynamicNodes() = 0;
    virtual void	removeAllNodes() = 0;
    virtual bool	isOrdered() = 0;

    virtual void	updateNodeStyles() = 0;
    virtual void	addLights(SceneRenderer& renderer) = 0;
    virtual void	addShadowNodes(SceneRenderer &renderer) = 0;
    virtual void	addRenderNodes(SceneRenderer& renderer) = 0;
    virtual void	renderRadarNodes(const ViewFrustum&) = 0;

    virtual void	drawCuller() = 0;
    
    virtual void	setOccluderManager(int);

  private:
			SceneDatabase(const SceneDatabase&);
    SceneDatabase&	operator=(const SceneDatabase&);
};


#endif // BZF_SCENE_DATABASE_H

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

