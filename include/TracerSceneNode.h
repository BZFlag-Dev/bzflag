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

/* TracerSceneNode:
 *	Encapsulates information for rendering a
 *	ShellSceneNode's tail.
 */

#ifndef	BZF_TRACER_SCENE_NODE_H
#define	BZF_TRACER_SCENE_NODE_H

#include "ShotSceneNode.h"
#include "OpenGLLight.h"

class TracerSceneNode : public ShotSceneNode {
  public:
			TracerSceneNode(const GLfloat pos[3],
					const GLfloat forward[3]);
			~TracerSceneNode();

    void		addLight(SceneRenderer&);
    void		move(const GLfloat pos[3], const GLfloat forward[3]);

    void		notifyStyleChange(const SceneRenderer&);
    void		addRenderNodes(SceneRenderer&);

  protected:
    class TracerRenderNode : public RenderNode {
      public:
			TracerRenderNode(const TracerSceneNode*);
			~TracerRenderNode();
	void		render();
	const GLfloat*	getPosition() { return sceneNode->getSphere(); }
      private:
	const TracerSceneNode* sceneNode;
    };
    friend class TracerRenderNode;

  private:
    GLfloat		azimuth, elevation;
    OpenGLLight		light;
    int			style;
    OpenGLGState	gstate;
    TracerRenderNode	renderNode;
    static const GLfloat tailVertex[9][3];
    static const GLfloat TailLength;
};

#endif // BZF_TRACER_SCENE_NODE_H
// ex: shiftwidth=2 tabstop=8
