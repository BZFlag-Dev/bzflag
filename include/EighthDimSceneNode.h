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

/* EighthDimSceneNode:
 *	Encapsulates information for rendering the eighth dimension
 */

#ifndef	BZF_EIGHTH_DIM_SCENE_NODE_H
#define	BZF_EIGHTH_DIM_SCENE_NODE_H

#include "common.h"
#include "SceneNode.h"

class EighthDimSceneNode : public SceneNode {
  public:
			~EighthDimSceneNode();

    bool		cull(const ViewFrustum&) const;
    void		notifyStyleChange();
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
	const GLfloat*	getPosition() const { return sceneNode->getSphere(); }
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

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

