/* bzflag
 * Copyright 1993-1999, Chris Schoeneman
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
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

#include "SceneNode.h"
#include "OpenGLTexture.h"

class TankSceneNode;

class TankIDLSceneNode : public SceneNode {
  public:
			TankIDLSceneNode(const TankSceneNode*);
			~TankIDLSceneNode();

    void		move(const GLfloat plane[4]);

    void		notifyStyleChange(const SceneRenderer&);
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
	const GLfloat*	getPosition() { return sceneNode->getSphere(); }
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
    void		setTexture(const OpenGLTexture&);

    void		notifyStyleChange(const SceneRenderer&);
    void		addRenderNodes(SceneRenderer&);
    void		addShadowNodes(SceneRenderer&);

    void		setColorblind(boolean on);
    void		setNormal();
    void		setObese();
    void		setTiny();
    void		setNarrow();
    void		setExplodeFraction(float t);
    void		setClipPlane(const GLfloat* plane);

    // hidden still renders shadow (turns off invisible)
    void		setHidden(boolean hidden = True);

    // invisible renders nothing (turns off hidden)
    void		setInvisible(boolean invisible = True);

    static void		setMaxLOD(int maxLevel);

  protected:
    class TankRenderNode : public RenderNode {
      public:
	enum Style {
			Normal,
			Obese,
			Tiny,
			Narrow
	};

			TankRenderNode(const TankSceneNode*);
			~TankRenderNode();
	void		setShadow();
	void		sortOrder(boolean above, boolean towards);
	void		render();
	const GLfloat*	getPosition() { return sceneNode->getSphere(); }
      protected:
	enum Part {
			Body,
			Barrel,
			Turret,
			LeftTread,
			RightTread
	};

	virtual GLuint	getParts(Style) = 0;
	virtual void	freeParts() = 0;
	void		renderParts();
	void		renderPart(Part);
	void		renderLights();
	void		prepStyle(Style);
	void		doVertex3f(GLfloat x, GLfloat y, GLfloat z);
	void		doNormal3f(GLfloat x, GLfloat y, GLfloat z);
      private:
	void		doInitContext();
	static void	initContext(void*);
      protected:
	const TankSceneNode* sceneNode;
	const GLfloat*	color;
	GLfloat		alpha;
	boolean		isShadow;
	boolean		above;
	boolean		towards;
	boolean		isExploding;
	GLfloat		explodeFraction;
	GLfloat		vel[5][2];
	GLfloat		spin[5][4];
	GLuint		base;
	static const GLfloat	centerOfGravity[][3];	// of parts
	static GLfloat		vertexScale[3];
	static GLfloat		normalScale[3];
    };
    class LowTankRenderNode : public TankRenderNode {
      public:
			LowTankRenderNode(const TankSceneNode*);
			~LowTankRenderNode();
      protected:
	GLuint		getParts(Style);
	void		freeParts();
	void		makeBody();
	void		makeBarrel();
	void		makeTurret();
	void		makeLeftTread();
	void		makeRightTread();
      private:
	static GLuint	parts[4];
    };
    class MedTankRenderNode : public TankRenderNode {
      public:
			MedTankRenderNode(const TankSceneNode*);
			~MedTankRenderNode();
      protected:
	GLuint		getParts(Style);
	void		freeParts();
	void		makeBody();
	void		makeBarrel();
	void		makeTurret();
	void		makeLeftTread();
	void		makeRightTread();
      private:
	static GLuint	parts[4];
    };
    class HighTankRenderNode : public TankRenderNode {
      public:
			HighTankRenderNode(const TankSceneNode*);
			~HighTankRenderNode();
      protected:
	GLuint		getParts(Style);
	void		freeParts();
	void		makeBody();
	void		makeBarrel();
	void		makeTurret();
	void		makeLeftTread();
	void		makeRightTread();
      private:
	static GLuint	parts[4];
    };
    friend class TankRenderNode;

  private:
    GLfloat		azimuth, elevation;
    GLfloat		baseRadius;
    boolean		colorblind, hidden, invisible;
    boolean		transparent, sort, blending;
    float		explodeFraction;
    boolean		clip;
    GLfloat		color[4];
    GLdouble		clipPlane[4];
    TankRenderNode::Style style;
    OpenGLGState	gstate;
    OpenGLGState	lightsGState;
    LowTankRenderNode	lowRenderNode;
    MedTankRenderNode	medRenderNode;
    HighTankRenderNode	highRenderNode;
    LowTankRenderNode	shadowRenderNode;

    static int			maxLevel;
    static const int		numLOD;
};

#endif // BZF_TANK_SCENE_NODE_H
