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

/* EighthDimSceneNode:
 *	Encapsulates information for rendering the eighth dimension
 */

#ifndef	BZF_EIGHTH_DIM_SCENE_NODE_H
#define	BZF_EIGHTH_DIM_SCENE_NODE_H

#include "SceneNode.h"

class EighthDimSceneNode : public SceneNode {
  public:
			~EighthDimSceneNode();

    boolean		cull(const ViewFrustum&) const;
    void		notifyStyleChange(const SceneRenderer&);
    void		addRenderNodes(SceneRenderer&);

  protected:
			EighthDimSceneNode(int numPolys);

    void		setPolygon(int index, const GLfloat[3][3]);

  protected:
    class EighthDimRenderNode : public RenderNode {
      public:
			EighthDimRenderNode(
				const EighthDimSceneNode*,
				int numPolygons);
			~EighthDimRenderNode();
	void		render();
	void		setPolygon(int index, const GLfloat[3][3]);
	const GLfloat*	getPosition() { return sceneNode->getSphere(); }
      private:
	const EighthDimSceneNode* sceneNode;
	int		numPolygons;
	GLfloat		(*color)[4];
	GLfloat		(*poly)[3][3];
    };

  private:
    OpenGLGState	gstate;
    EighthDimRenderNode	renderNode;
};

#endif // BZF_EIGHTH_DIM_SCENE_NODE_H
// ex: shiftwidth=2 tabstop=8
