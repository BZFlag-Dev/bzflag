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

/* EighthDBaseSceneNode:
 *	Encapsulates information for rendering the eighth dimension
 *	of a base building.
 */

#ifndef BZF_EIGHTHD_BASE_SCENE_NODE_H
#define BZF_EIGHTHD_BASE_SCENE_NODE_H

#include "EighthDimSceneNode.h"

class EighthDBaseSceneNode : public EighthDimSceneNode {
  public:
    			EighthDBaseSceneNode(const float pos[3],
			    		const float size[3], float rotation);
			~EighthDBaseSceneNode();
    void		notifyStyleChange(const SceneRenderer&);
    void		addRenderNodes(SceneRenderer&);
  protected:
    class EighthDBaseRenderNode : public RenderNode {
      public:
			EighthDBaseRenderNode(const EighthDBaseSceneNode *,
			    	const float pos[3],
				const float size[3], float rotation);
			~EighthDBaseRenderNode();
	void		render();
	const GLfloat *	getPosition() { return sceneNode->getSphere(); }
      private:
	const EighthDBaseSceneNode *sceneNode;
	GLfloat corner[8][3];
    };
  private:
    OpenGLGState	  gstate;
    EighthDBaseRenderNode renderNode;
};

#endif // BZF_EIGHTHD_BASE_SCENE_NODE_H
// ex: shiftwidth=2 tabstop=8
