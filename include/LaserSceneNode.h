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

/* LaserSceneNode:
 *	Encapsulates information for rendering a laser beam.
 */

#ifndef	BZF_LASER_SCENE_NODE_H
#define	BZF_LASER_SCENE_NODE_H

#include "common.h"
#include "SceneNode.h"

class LaserSceneNode : public SceneNode {
  public:
			LaserSceneNode(const GLfloat pos[3],
					const GLfloat forward[3]);
			~LaserSceneNode();

    void		setTexture(const OpenGLTexture&);

    bool		cull(const ViewFrustum&) const;

    void		notifyStyleChange(const SceneRenderer&);
    void		addRenderNodes(SceneRenderer&);

  protected:
    class LaserRenderNode : public RenderNode {
      public:
			LaserRenderNode(const LaserSceneNode*);
			~LaserRenderNode();
	void		render();
	const GLfloat*	getPosition() { return sceneNode->getSphere(); }
      private:
	const LaserSceneNode* sceneNode;
	static GLfloat 	geom[6][2];
    };
    friend class LaserRenderNode;

  private:
    GLfloat		azimuth, elevation;
    GLfloat		length;
    bool		texturing;
    OpenGLGState	gstate;
    LaserRenderNode	renderNode;
};

#endif // BZF_LASER_SCENE_NODE_H
// ex: shiftwidth=2 tabstop=8
