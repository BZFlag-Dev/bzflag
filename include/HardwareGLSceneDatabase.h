/* bzflag
 * Copyright (c) 1993 - 2003 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* HardwareGLSceneDatabase:
 *	Database of geometry to render using Z-buffer algorithm
 */

#ifndef	BZF_HGL_SCENE_DATABASE_H
#define	BZF_HGL_SCENE_DATABASE_H

#include "common.h"
#include "SceneDatabase.h"

class HardwareGLSceneDatabase : public SceneDatabase {
  friend class HardwareGLSceneIterator;
  public:
			HardwareGLSceneDatabase();
			~HardwareGLSceneDatabase();

    void		addStaticNode(SceneNode*);
    void		addDynamicNode(SceneNode*);
    void		addDynamicSphere(SphereSceneNode*);
    void		removeDynamicNodes();
    void		removeAllNodes();
    bool		isOrdered();

    SceneIterator*	getRenderIterator();

  private:
    int			staticCount;
    int			staticSize;
    SceneNode**		staticList;

    int			dynamicCount;
    int			dynamicSize;
    SceneNode**		dynamicList;
};

class HardwareGLSceneIterator : public SceneIterator {
  public:
			HardwareGLSceneIterator(const HardwareGLSceneDatabase*);
    virtual		~HardwareGLSceneIterator();

    virtual void	resetFrustum(const ViewFrustum*);
    virtual void	reset();
    virtual SceneNode*	getNext();

  private:
    const HardwareGLSceneDatabase* db;
    bool		staticDone, dynamicDone;
    int			staticIndex, dynamicIndex;
};

#endif // BZF_HGL_SCENE_DATABASE_H

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

