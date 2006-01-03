/* bzflag
 * Copyright (c) 1993 - 2006 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
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

    static void		waveFlag(float dt);
    static void		freeFlag();
    
    void		move(const GLfloat pos[3]);
    void		setAngle(GLfloat angle);
    void		setWind(const GLfloat wind[3], float dt);
    void		setBillboard(bool billboard);

    const GLfloat*	getColor() const { return color; }
    void		setColor(GLfloat r, GLfloat g,
				 GLfloat b, GLfloat a = 1.0f);
    void		setColor(const GLfloat* rgba);
    void		setTexture(const int);

    void		notifyStyleChange();
    void		addRenderNodes(SceneRenderer&);
    void		addShadowNodes(SceneRenderer&);

    bool		cullShadow(int planeCount,
                                   const float (*planes)[4]) const;
  protected:
    class FlagRenderNode : public RenderNode {
      public:
			FlagRenderNode(const FlagSceneNode*);
			~FlagRenderNode();
	void		render();
	const GLfloat*	getPosition() const { return sceneNode->getSphere(); }
      private:
	const FlagSceneNode* sceneNode;
	int	     waveReference;
    };
    friend class FlagRenderNode;

  private:
    bool		billboard;
    GLfloat		angle;
    GLfloat		tilt;
    GLfloat		hscl;
    GLfloat		color[4];
    bool		transparent;
    bool		texturing;
    OpenGLGState	gstate;
    FlagRenderNode	renderNode;
};

#endif // BZF_FLAG_SCENE_NODE_H

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

