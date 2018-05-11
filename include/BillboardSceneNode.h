/* bzflag
 * Copyright (c) 1993-2018 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* BillboardSceneNode:
 *	Encapsulates information for rendering a textured billboard.
 */

#ifndef	BZF_BILLBOARD_SCENE_NODE_H
#define	BZF_BILLBOARD_SCENE_NODE_H

#include "common.h"
#include "SceneNode.h"
#include "OpenGLLight.h"

class BillboardSceneNode : public SceneNode {
  public:
			BillboardSceneNode(const GLfloat pos[3]);
			~BillboardSceneNode();

    virtual BillboardSceneNode*	copy() const;

    void		setLoop(bool = true);
    void		setDuration(float);
    void		resetTime();
    void		updateTime(float dt);
    bool		isAtEnd() const;

    bool		isLight() const;
    void		setLight(bool = true);
    void		setLightColor(GLfloat r, GLfloat g, GLfloat b);
    void		setLightAttenuation(GLfloat c, GLfloat l, GLfloat q);
    void		setLightScaling(GLfloat intensityScaleFactor);
    void		setLightFadeStartTime(float t);
    void		setGroundLight(bool value);

    void		setSize(float side);
    void		setSize(float width, float height);
    void		setColor(GLfloat r, GLfloat g,
				GLfloat b, GLfloat a = 1.0f);
    void		setColor(const GLfloat* rgba);
    void		setTexture(const int);
    void		setTextureAnimation(int cu, int cv);

    void		move(const GLfloat pos[3]);
    void		setAngle(GLfloat);
    void		addLight(SceneRenderer&);

    void		notifyStyleChange();
    void		addRenderNodes(SceneRenderer&);

  protected:
    class BillboardRenderNode : public RenderNode {
      public:
			BillboardRenderNode(const BillboardSceneNode*);
			~BillboardRenderNode();
	void		setColor(const GLfloat* rgba);
	void		render();
	const GLfloat*	getPosition() const { return sceneNode->getSphere(); }
	void		setFrame(float u, float v);
	void		setFrameSize(float du, float dv);
      private:
	const BillboardSceneNode* sceneNode;
	float		u, v;
	GLfloat		du, dv;
    };
    friend class BillboardRenderNode;

    void		setFrame();
    void		prepLight();

  private:
    bool		show;
    bool		hasAlpha;
    bool		hasTexture;
    bool		hasTextureAlpha;
    bool		looping;
    bool		lightSource;
    bool		groundLight;
    float		width, height;
    GLfloat		color[4];
    GLfloat		angle;
    GLfloat		lightColor[3];
    GLfloat		lightScale;
    float		lightCutoffTime;
    int			cu, cv;
    float		t, duration;
    OpenGLLight		light;
    OpenGLGState	gstate;
    BillboardRenderNode	renderNode;
};

#endif // BZF_BILLBOARD_SCENE_NODE_H

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
