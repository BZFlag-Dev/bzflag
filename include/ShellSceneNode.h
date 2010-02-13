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

/* ShellSceneNode:
 *	Encapsulates information for rendering a regular
 *	looking shot (as opposed to a shock wave or laser).
 */

#ifndef	BZF_SHELL_SCENE_NODE_H
#define	BZF_SHELL_SCENE_NODE_H

#include "common.h"
#include "ShotSceneNode.h"

const float		ShellRadius = 0.5f;

class ShellSceneNode : public ShotSceneNode {
  public:
			ShellSceneNode(const fvec3& pos, const fvec3& forward);
			~ShellSceneNode();

    void		move(const fvec3& pos, const fvec3& forward);

    void		notifyStyleChange();
    void		addRenderNodes(SceneRenderer&);
    void		addShadowNodes(SceneRenderer&);

  protected:
    class ShellRenderNode : public RenderNode {
      public:
			ShellRenderNode(const ShellSceneNode*);
			~ShellRenderNode();
	void		setLighting(bool);
	void		render();
	const fvec3&	getPosition() const { return sceneNode->getCenter(); }
      private:
	const ShellSceneNode* sceneNode;
	bool		lighted;
    };
    friend class ShellRenderNode;

  private:
    float		azimuth, elevation;
    OpenGLGState	gstate;
    ShellRenderNode	renderNode;
    static const fvec3 shellVertex[9];
    static const fvec3 shellNormal[10];
};

#endif // BZF_SHELL_SCENE_NODE_H

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
