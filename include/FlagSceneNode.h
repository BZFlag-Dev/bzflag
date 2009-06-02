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

/* FlagSceneNode:
 *	Encapsulates information for rendering a flag.
 */

#ifndef	BZF_FLAG_SCENE_NODE_H
#define	BZF_FLAG_SCENE_NODE_H

#include "common.h"
#include "SceneNode.h"
#include "vectors.h"


const int maxFlagLODs = 9; // max 256 quads


class FlagPhase;


class FlagSceneNode : public SceneNode {

  friend class FlagRenderNode;

  public:
    FlagSceneNode(const fvec3& pos);
    ~FlagSceneNode();

    static void waveFlags(float waveSpeed);
    static void setTimeStep(float dt);

    void move(const fvec3& pos);
    void setAngle(float angle);
    void setWind(const fvec3& wind, float dt);
    void setFlat(bool flat);

    void setAlpha(float);
    void setColor(const fvec4& rgba);
    void setColor(float r, float g, float b, float a);
    void setTexture(int);
    void setUseColor(bool);
    const fvec4* getColor() const { return color; }

    void addRenderNodes(SceneRenderer&); // also computes the LOD
    void addShadowNodes(SceneRenderer&);

    void notifyStyleChange();

    bool cullShadow(int planeCount, const fvec4* planes) const;

  protected:
    class FlagRenderNode : public RenderNode {
      public:
	FlagRenderNode(const FlagSceneNode*);
	~FlagRenderNode();

	void render();
	void renderShadow();

	const fvec3& getPosition() const { return sceneNode->getCenter(); }

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

    float		angle;
    float		tilt;
    float		hscl;

    fvec4*		color;
    fvec4		realColor;
    fvec4		whiteColor;
    bool		useColor;

    OpenGLGState	gstate;

    FlagRenderNode	renderNode;

    static const int	minPoleLOD;
    static const float	lodLengths[maxFlagLODs];
};

#endif // BZF_FLAG_SCENE_NODE_H

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
