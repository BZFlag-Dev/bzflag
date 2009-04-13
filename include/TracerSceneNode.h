/* bzflag
 * Copyright (c) 1993 - 2009 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* TracerSceneNode:
 *	Encapsulates information for rendering a
 *	ShellSceneNode's tail.
 */

#ifndef	BZF_TRACER_SCENE_NODE_H
#define	BZF_TRACER_SCENE_NODE_H

#include "common.h"
#include "ShotSceneNode.h"
#include "OpenGLLight.h"

class TracerSceneNode : public ShotSceneNode {
  public:
			TracerSceneNode(const fvec3& pos, const fvec3& forward);
			~TracerSceneNode();

    void		addLight(SceneRenderer&);
    void		move(const fvec3& pos, const fvec3& forward);

    void		notifyStyleChange();
    void		addRenderNodes(SceneRenderer&);

  protected:
    class TracerRenderNode : public RenderNode {
      public:
			TracerRenderNode(const TracerSceneNode*);
			~TracerRenderNode();
	void		render();
	const fvec3&	getPosition() const { return sceneNode->getCenter(); }
      private:
	const TracerSceneNode* sceneNode;
    };
    friend class TracerRenderNode;

  private:
    float		azimuth, elevation;
    OpenGLLight		light;
    int			style;
    OpenGLGState	gstate;
    TracerRenderNode	renderNode;
    static const fvec3   tailVertex[9];
    static const float TailLength;
};

#endif // BZF_TRACER_SCENE_NODE_H

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
