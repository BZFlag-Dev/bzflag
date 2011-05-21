/* bzflag
 * Copyright (c) 1993-2010 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* EighthDBoxSceneNode:
 *	Encapsulates information for rendering the eighth dimension
 *	of a box building.
 */

#ifndef	BZF_EIGHTHD_BOX_SCENE_NODE_H
#define	BZF_EIGHTHD_BOX_SCENE_NODE_H

#include "common.h"
#include "EighthDimSceneNode.h"

class EighthDBoxSceneNode : public EighthDimSceneNode {
  public:
    EighthDBoxSceneNode(const fvec3& pos,
                        const fvec3& size, float rotation);
			~EighthDBoxSceneNode();

    void		notifyStyleChange();
    void		addRenderNodes(SceneRenderer&);

  protected:
    class EighthDBoxRenderNode : public RenderNode {
      public:
			EighthDBoxRenderNode(const EighthDBoxSceneNode*,
				const fvec3& pos,
				const fvec3& size, float rotation);
			~EighthDBoxRenderNode();
	void		render();
	const fvec3&	getPosition() const { return sceneNode->getCenter(); }
      private:
	const EighthDBoxSceneNode* sceneNode;
	fvec3		corner[8];
    };

  private:
    OpenGLGState	 gstate;
    EighthDBoxRenderNode renderNode;
};

#endif // BZF_EIGHTHD_BOX_SCENE_NODE_H

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
