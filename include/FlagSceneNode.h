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

/* FlagSceneNode:
 *	Encapsulates information for rendering a flag.
 */

#ifndef	BZF_FLAG_SCENE_NODE_H
#define	BZF_FLAG_SCENE_NODE_H

#include "common.h"
#include "SceneNode.h"


const int maxFlagLODs = 9; // max 256 quads


class FlagPhase;


class FlagSceneNode : public SceneNode {

  friend class FlagRenderNode;

  public:
    FlagSceneNode(const GLfloat pos[3]);
    ~FlagSceneNode();

    static void waveFlags();
    static void setTimeStep(float dt);

    void move(const GLfloat pos[3]);
    void setAngle(GLfloat angle);
    void setWind(const GLfloat wind[3], float dt);
    void setFlat(bool flat);

    void setAlpha(GLfloat);
    void setColor(const GLfloat* rgba);
    void setColor(GLfloat r, GLfloat g, GLfloat b, GLfloat a);
    void setTexture(int);
    void setUseColor(bool);
    const GLfloat* getColor() const { return color; }

    void addRenderNodes(SceneRenderer&); // also computes the LOD
    void addShadowNodes(SceneRenderer&);

    void notifyStyleChange();

    bool cullShadow(int planeCount, const float (*planes)[4]) const;

  protected:
    class FlagRenderNode : public RenderNode {
      public:
	FlagRenderNode(const FlagSceneNode*);
	~FlagRenderNode();

	void render();
	void renderShadow();

	const GLfloat* getPosition() const { return sceneNode->getSphere(); }

      private:
	void renderFancyPole();

      private:
	const FlagSceneNode* sceneNode;
	bool isShadow;
    };


  private:
    int calcLOD(const SceneRenderer&);
    int calcShadowLOD(const SceneRenderer&);

  private:
    FlagPhase*		phase;

    int			lod;
    int			shadowLOD;

    bool		flat;
    bool		translucent;
    bool		texturing;

    GLfloat		angle;
    GLfloat		tilt;
    GLfloat		hscl;

    GLfloat*		color;
    GLfloat		realColor[4];
    GLfloat		whiteColor[4];
    bool		useColor;

    OpenGLGState	gstate;

    FlagRenderNode	renderNode;

    static const int	minPoleLOD;
    static const float	lodLengths[maxFlagLODs];
};

#endif // BZF_FLAG_SCENE_NODE_H

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
