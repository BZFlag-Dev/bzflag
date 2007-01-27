/* bzflag
 * Copyright (c) 1993 - 2007 Tim Riker
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

const GLfloat		ShellRadius = 0.5f;

class ShellSceneNode : public ShotSceneNode {
  public:
			ShellSceneNode(const GLfloat pos[3],
					const GLfloat forward[3]);
			~ShellSceneNode();

    void		move(const GLfloat pos[3], const GLfloat forward[3]);

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
	const GLfloat*	getPosition() const { return sceneNode->getSphere(); }
      private:
	const ShellSceneNode* sceneNode;
	bool		lighted;
    };
    friend class ShellRenderNode;

  private:
    GLfloat		azimuth, elevation;
    OpenGLGState	gstate;
    ShellRenderNode	renderNode;
    static const GLfloat shellVertex[9][3];
    static const GLfloat shellNormal[10][3];
};

#endif // BZF_SHELL_SCENE_NODE_H

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
