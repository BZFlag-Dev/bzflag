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

/* TankSceneNode:
 *	Encapsulates information for rendering a tank
 */

#ifndef	BZF_TANK_SCENE_NODE_H
#define	BZF_TANK_SCENE_NODE_H

#include "common.h"
#include "SceneNode.h"
#include "OpenGLLight.h"
#include "TankGeometryMgr.h"

class TankSceneNode;

class TankIDLSceneNode : public SceneNode {
  public:
			TankIDLSceneNode(const TankSceneNode*);
			~TankIDLSceneNode();

    void		move(const GLfloat plane[4]);

    void		notifyStyleChange();
    void		addRenderNodes(SceneRenderer&);
  // Irix 7.2.1 and solaris compilers appear to have a bug.  if the
  // following declaration isn't public it generates an error when trying
  // to declare SphereFragmentSceneNode::FragmentRenderNode a friend in
  // SphereSceneNode::SphereRenderNode.  i think this is a bug in the
  // compiler because:
  //   no other compiler complains
  //   public/protected/private adjust access not visibility
  //     SphereSceneNode isn't requesting access, it's granting it
//  protected:
  public:
    class IDLRenderNode : public RenderNode {
      public:
			IDLRenderNode(const TankIDLSceneNode*);
			~IDLRenderNode();
	void		render();
	const GLfloat*	getPosition() const { return sceneNode->getSphere(); }
      private:
	const TankIDLSceneNode* sceneNode;
	static const int	idlFaces[][5];
	static const GLfloat	idlVertex[][3];
    };
    friend class IDLRenderNode;

  private:
    const TankSceneNode	*tank;
    GLfloat		plane[4];
    OpenGLGState	gstate;
    IDLRenderNode	renderNode;
};

class TankSceneNode : public SceneNode {
  friend class TankIDLSceneNode;
  friend class TankIDLSceneNode::IDLRenderNode;
  public:
			TankSceneNode(const GLfloat pos[3],
					const GLfloat forward[3]);
			~TankSceneNode();

    void		move(const GLfloat pos[3], const GLfloat forward[3]);

    void		setColor(GLfloat r, GLfloat g,
				 GLfloat b, GLfloat a = 1.0f);
    void		setColor(const GLfloat* rgba);
    void		setMaterial(const OpenGLMaterial&);
    void		setTexture(const int);
    void		setJumpJetsTexture(const int);

    void		notifyStyleChange();
    void		addRenderNodes(SceneRenderer&);
    void		addShadowNodes(SceneRenderer&);
    void		addLight(SceneRenderer&);

    void		setDimensions(const float size[3]);
    void		ignoreDimensions();

    void		setNormal();
    void		setObese();
    void		setTiny();
    void		setNarrow();
    void		setThief();
    void		setClipPlane(const GLfloat* plane);
    void		setExplodeFraction(float t);
    void		setJumpJets(float scale);
    void		setInTheCockpit(bool value);
    void		rebuildExplosion();

    void		addTreadOffsets(float left, float right);

    // hidden still renders shadow (turns off cloaked)
    void		setHidden(bool hidden);

    // cloaked renders nothing if color[3] = 0.0f (turns off hidden)
    void		setCloaked(bool cloaked);

    static void		setMaxLOD(int maxLevel);

  protected:

    class TankRenderNode : public RenderNode {
      public:
			TankRenderNode(const TankSceneNode*);
			~TankRenderNode();
	void		setShadow();
	void		setTankLOD(TankGeometryEnums::TankLOD);
	void		setTankSize(TankGeometryEnums::TankSize);
	void		sortOrder(bool above, bool towards, bool left);
	void		setNarrowWithDepth(bool narrow);
	void		render();
	const GLfloat*	getPosition() const { return sceneNode->getSphere(); }

	void		renderPart(TankGeometryEnums::TankPart part);
	void		renderParts();
	void		renderTopParts();
	void		renderLeftParts();
	void		renderRightParts();
	void		renderNarrowWithDepth();
	void		renderLights();
	void		renderJumpJets();
	void		setupPartColor(TankGeometryEnums::TankPart part);
	bool		setupTextureMatrix(TankGeometryEnums::TankPart part);

      protected:
	const TankSceneNode* sceneNode;
	TankGeometryEnums::TankLOD drawLOD;
	TankGeometryEnums::TankSize drawSize;
	const GLfloat*	color;
	GLfloat		alpha;
	bool		isShadow;
	bool		left;
	bool		above;
	bool		towards;
	bool		isExploding;
	bool		narrowWithDepth;
	GLfloat		explodeFraction;
	static const GLfloat centerOfGravity[TankGeometryEnums::LastTankPart][3];
    };
    friend class TankRenderNode;

  private:
    GLfloat		azimuth, elevation;
    GLfloat		baseRadius;
    float		dimensions[3]; // tank dimensions
    float		leftTreadOffset;
    float		rightTreadOffset;
    float		leftWheelOffset;
    float		rightWheelOffset;
    bool		useDimensions;
    bool		useOverride, hidden, cloaked;
    bool		transparent, sort;
    float		explodeFraction;
    bool		clip;
    bool		inTheCockpit;
    GLfloat		colorOverride[4];
    GLfloat		color[4];
    GLdouble		clipPlane[4];
    OpenGLGState	gstate;
    OpenGLGState	lightsGState;
    TankRenderNode	tankRenderNode;
    TankRenderNode	shadowRenderNode;
    TankGeometryEnums::TankSize tankSize;
    GLfloat vel[TankGeometryEnums::LastTankPart][2];
    GLfloat spin[TankGeometryEnums::LastTankPart][4];
    bool		jumpJetsOn;
    GLfloat		jumpJetsScale;
    GLfloat		jumpJetsLengths[4];
    GLfloat		jumpJetsPositions[4][3];
    OpenGLLight		jumpJetsRealLight;
    OpenGLLight		jumpJetsGroundLights[4];
    OpenGLGState	jumpJetsGState;

    static int		maxLevel;
    static const int	numLOD;
    static GLfloat	jumpJetsModel[4][3];
};


#endif // BZF_TANK_SCENE_NODE_H

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

