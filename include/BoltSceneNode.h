/* bzflag
 * Copyright (c) 1993-2012 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* BoltSceneNode:
 *	Encapsulates information for rendering a ball lighting bolt.
 */

#ifndef	BZF_BOLT_SCENE_NODE_H
#define	BZF_BOLT_SCENE_NODE_H

#include "common.h"
#include "ShotSceneNode.h"
#include "OpenGLLight.h"

class BoltSceneNode : public ShotSceneNode {
  public:
			BoltSceneNode(const GLfloat pos[3], const GLfloat vel[3], bool super);
			~BoltSceneNode();

    void		setFlares(bool);
    void		setSize(float radius);
    void		setColor(GLfloat r, GLfloat g, GLfloat b, GLfloat a = 1.0f);
    void		setTextureColor(GLfloat r, GLfloat g, GLfloat b, GLfloat a = 1.0f);
    void		setColor(const GLfloat* rgb);
	void		setTeamColor(const GLfloat* rgb);
    void		setTexture(const int);
    void		setTextureAnimation(int cu, int cv);

    bool		getColorblind() const;
    void		setColorblind(bool);

	bool		getInvisible() const {return invisible;}
	void		setInvisible(bool _invisible) {invisible = _invisible;}

    void		move(const GLfloat pos[3], const GLfloat forward[3]);
    void		addLight(SceneRenderer&);

    void		notifyStyleChange();
    void		addRenderNodes(SceneRenderer&);

  protected:
	bool isSuper;

    class BoltRenderNode : public RenderNode {
      public:
			BoltRenderNode(const BoltSceneNode*);
			~BoltRenderNode();
			void		setColor(const GLfloat* rgba);
			void    setTextureColor(const GLfloat* rgba);
			void		render();
			const GLfloat*	getPosition() const { return sceneNode->getSphere(); }
			void		setAnimation(int cu, int cv);

			void		renderGeoBolt();
			void		renderGeoGMBolt();
			void		renderGeoPill( float radius, float length, int segments, float endRad = -1);

		private:
			const BoltSceneNode* sceneNode;
			int		u, v, cu, cv;
			GLfloat		du, dv;
			GLfloat		mainColor[4];
			GLfloat		innerColor[4];
			GLfloat		outerColor[4];
			GLfloat		coronaColor[4];
			GLfloat		flareColor[4];
			GLfloat		textureColor[4];
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
    bool		invisible;
    bool		drawFlares;
    bool		texturing;
    bool		colorblind;
    float		size;
    float		velocity[3];
    GLfloat		color[4];
	fvec4		teamColor;
    OpenGLLight		light;
    OpenGLGState	gstate;
    OpenGLGState	colorblindGState;
    BoltRenderNode	renderNode;
    float		azimuth, elevation, length;
};

#endif // BZF_BOLT_SCENE_NODE_H

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

