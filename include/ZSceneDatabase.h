/* bzflag
 * Copyright (c) 1993 - 2004 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* ZSceneDatabase:
 *	Database of geometry to render using Z-buffer algorithm
 */

#ifndef	BZF_Z_SCENE_DATABASE_H
#define	BZF_Z_SCENE_DATABASE_H

#include "common.h"
#include "SceneDatabase.h"

class ZSceneDatabase : public SceneDatabase {
  friend class ZSceneIterator;
  public:
			ZSceneDatabase();
			~ZSceneDatabase();

    void		addStaticNode(SceneNode*);
    void		addDynamicNode(SceneNode*);
    void		addDynamicSphere(SphereSceneNode*);
    void		removeDynamicNodes();
    void		removeAllNodes();
    bool		isOrdered();

    void		updateNodeStyles();
    void		addLights(SceneRenderer& renderer);
    void		addShadowNodes(SceneRenderer &renderer);
    void		addRenderNodes(SceneRenderer& renderer);
    
    void		drawCuller();

  private:
    void		setupCullList();
    void                makeCuller();
    
  private:
    int			staticCount;
    int			staticSize;
    SceneNode**		staticList;

    int			dynamicCount;
    int			dynamicSize;
    SceneNode**		dynamicList;

    int			culledCount;
    SceneNode**         culledList;

    class Octree*       octree;
    int			cullDepth;
    int			cullElements;

};


#endif // BZF_Z_SCENE_DATABASE_H

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

