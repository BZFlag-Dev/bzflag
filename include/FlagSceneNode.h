/* bzflag
 * Copyright (c) 1993 - 2004 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
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

#include "common.h"
#include "SceneNode.h"

const int maxChunks = 20;
class FlagSceneNode : public SceneNode {
  public:
			FlagSceneNode(const GLfloat pos[3]);
			~FlagSceneNode();

    void		waveFlag(float dt, float droop);
    void		move(const GLfloat pos[3]);
    void		turn(GLfloat angle);
    void		setBillboard(bool billboard);

    const GLfloat*	getColor() const { return color; }
    void		setColor(GLfloat r, GLfloat g,
				 GLfloat b, GLfloat a = 1.0f);
    void		setColor(const GLfloat* rgba);
    void		setTexture(const int);

    void		notifyStyleChange();
    void		addRenderNodes(SceneRenderer&);
    void		addShadowNodes(SceneRenderer&);

  protected:
    class FlagRenderNode : public RenderNode {
      public:
			FlagRenderNode(const FlagSceneNode*);
			~FlagRenderNode();
	void		render();
	void		doWave();
	const GLfloat*	getPosition() { return sceneNode->getSphere(); }
      private:
	const FlagSceneNode* sceneNode;
	float		wave0[maxChunks];
	float		wave1[maxChunks];
	float		wave2[maxChunks];
	bool		 recomputeWave;
    };
    friend class FlagRenderNode;

  private:
    bool		billboard;
    GLfloat		angle;
    float		ripple1, ripple2;
    GLfloat		color[4];
    bool		transparent;
    bool		texturing;
    OpenGLGState	gstate;
    FlagRenderNode	renderNode;
    static const GLfloat Width;
    static const GLfloat Height;
    static const float	RippleSpeed1;
    static const float	RippleSpeed2;
    static const float	DroopFactor;
};

#endif // BZF_FLAG_SCENE_NODE_H

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

