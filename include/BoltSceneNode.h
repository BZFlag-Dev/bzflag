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

/* BoltSceneNode:
 *	Encapsulates information for rendering a ball lighting bolt.
 */

#ifndef	BZF_BOLT_SCENE_NODE_H
#define	BZF_BOLT_SCENE_NODE_H

#include "ShotSceneNode.h"
#include "OpenGLLight.h"

class OpenGLTexture;

class BoltSceneNode : public ShotSceneNode {
  public:
			BoltSceneNode(const GLfloat pos[3]);
			~BoltSceneNode();

    void		setFlares(boolean);
    void		setSize(float radius);
    void		setColor(GLfloat r, GLfloat g, GLfloat b);
    void		setColor(const GLfloat* rgb);
    void		setTexture(const OpenGLTexture&);
    void		setColorblindTexture(const OpenGLTexture&);
    void		setTextureAnimation(int cu, int cv);

    boolean		getColorblind() const;
    void		setColorblind(boolean);

    void		move(const GLfloat pos[3], const GLfloat forward[3]);
    void		addLight(SceneRenderer&);

    void		notifyStyleChange(const SceneRenderer&);
    void		addRenderNodes(SceneRenderer&);

  protected:
    class BoltRenderNode : public RenderNode {
      public:
			BoltRenderNode(const BoltSceneNode*);
			~BoltRenderNode();
	void		setColor(const GLfloat* rgb);
	void		render();
	const GLfloat*	getPosition() { return sceneNode->getSphere(); }
	void		setAnimation(int cu, int cv);
      private:
	const BoltSceneNode* sceneNode;
	int		u, v, cu, cv;
	GLfloat		du, dv;
	GLfloat		mainColor[3];
	GLfloat		innerColor[3];
	GLfloat		outerColor[4];
	GLfloat		coronaColor[4];
	GLfloat		flareColor[4];
	int		numFlares;
	float		theta[6];
	float		phi[6];

	static GLfloat	core[9][2];
	static GLfloat	corona[8][2];
	static const GLfloat ring[8][2];
	static const GLfloat CoreFraction;
	static const GLfloat FlareSize;
	static const GLfloat FlareSpread;
    };
    friend class BoltRenderNode;

  private:
    boolean		drawFlares;
    boolean		blending;
    boolean		texturing;
    boolean		colorblind;
    float		size;
    GLfloat		color[3];
    OpenGLLight		light;
    OpenGLGState	gstate;
    OpenGLGState	colorblindGState;
    BoltRenderNode	renderNode;
};

#endif // BZF_BOLT_SCENE_NODE_H
// ex: shiftwidth=2 tabstop=8
