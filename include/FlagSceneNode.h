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

/* FlagSceneNode:
 *	Encapsulates information for rendering a flag.
 */

#ifndef	BZF_FLAG_SCENE_NODE_H
#define	BZF_FLAG_SCENE_NODE_H

#include "SceneNode.h"

class FlagSceneNode : public SceneNode {
  public:
			FlagSceneNode(const GLfloat pos[3]);
			~FlagSceneNode();

    void		waveFlag(float dt, float droop);
    void		move(const GLfloat pos[3]);
    void		turn(GLfloat angle);
    void		setBillboard(boolean billboard);

    const GLfloat*	getColor() const { return color; }
    void		setColor(GLfloat r, GLfloat g,
				 GLfloat b, GLfloat a = 1.0f);
    void		setColor(const GLfloat* rgba);
    void		setTexture(const OpenGLTexture&);

    void		notifyStyleChange(const SceneRenderer&);
    void		addRenderNodes(SceneRenderer&);
    void		addShadowNodes(SceneRenderer&);

  protected:
    class FlagRenderNode : public RenderNode {
      public:
			FlagRenderNode(const FlagSceneNode*);
			~FlagRenderNode();
	void		render();
	const GLfloat*	getPosition() { return sceneNode->getSphere(); }
      private:
	const FlagSceneNode* sceneNode;
    };
    friend class FlagRenderNode;

  private:
    boolean		billboard;
    GLfloat		angle;
    float		ripple1, ripple2;
    GLfloat		color[4];
    boolean		transparent;
    boolean		blending;
    boolean		texturing;
    OpenGLGState	gstate;
    FlagRenderNode	renderNode;
    static const GLfloat Width;
    static const GLfloat Height;
    static const GLfloat Base;
    static const float	RippleSpeed1;
    static const float	RippleSpeed2;
    static const float	DroopFactor;
};

#endif // BZF_FLAG_SCENE_NODE_H
// ex: shiftwidth=2 tabstop=8
