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

/* EighthDBoxSceneNode:
 *	Encapsulates information for rendering the eighth dimension
 *	of a box building.
 */

#ifndef	BZF_EIGHTHD_BOX_SCENE_NODE_H
#define	BZF_EIGHTHD_BOX_SCENE_NODE_H

#include "EighthDimSceneNode.h"

class EighthDBoxSceneNode : public EighthDimSceneNode {
  public:
			EighthDBoxSceneNode(const float pos[3],
					const float size[3], float rotation);
			~EighthDBoxSceneNode();

    void		notifyStyleChange(const SceneRenderer&);
    void		addRenderNodes(SceneRenderer&);

  protected:
    class EighthDBoxRenderNode : public RenderNode {
      public:
			EighthDBoxRenderNode(const EighthDBoxSceneNode*,
				const float pos[3],
				const float size[3], float rotation);
			~EighthDBoxRenderNode();
	void		render();
	const GLfloat*	getPosition() { return sceneNode->getSphere(); }
      private:
	const EighthDBoxSceneNode* sceneNode;
	GLfloat		corner[8][3];
    };

  private:
    OpenGLGState	 gstate;
    EighthDBoxRenderNode renderNode;
};

#endif // BZF_EIGHTHD_BOX_SCENE_NODE_H
// ex: shiftwidth=2 tabstop=8
