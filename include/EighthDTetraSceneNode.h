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

/* EighthDBoxSceneNode:
 *	Encapsulates information for rendering the eighth dimension
 *	of a pyramid building.
 */

#ifndef	BZF_EIGHTHD_TETRA_SCENE_NODE_H
#define	BZF_EIGHTHD_TETRA_SCENE_NODE_H

#include "common.h"
#include "EighthDimSceneNode.h"

class EighthDTetraSceneNode : public EighthDimSceneNode {
  public:
			EighthDTetraSceneNode(const float (*vertices)[3],
			                      const float (*planes)[4]);
			~EighthDTetraSceneNode();

    void		notifyStyleChange();
    void		addRenderNodes(SceneRenderer&);

  protected:
    class EighthDTetraRenderNode : public RenderNode {
      public:
			EighthDTetraRenderNode(const EighthDTetraSceneNode*,
			                       const float (*vertices)[3]);
			~EighthDTetraRenderNode();
	void		render();
	const GLfloat*	getPosition() { return sceneNode->getSphere(); }

      private:
	const EighthDTetraSceneNode* sceneNode;
	GLfloat		corner[4][3];
    };

  private:
    OpenGLGState	 gstate;
    EighthDTetraRenderNode renderNode;
};

#endif // BZF_EIGHTHD_TETRA_SCENE_NODE_H

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

