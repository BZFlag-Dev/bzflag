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

// prototypes for tank geo
// hightank
void buildHighBody ( void );
void buildHighTurret ( void );
void buildHighLTread ( void );
void buildHighRTread ( void );
void buildHighBarrel ( void );
// medtank
void buildMedBody ( void );
void buildMedTurret ( void );
void buildMedLTread ( void );
void buildMedRTread ( void );
void buildMedBarrel ( void );
// lowtank
void buildLowBody ( void );
void buildLowTurret ( void );
void buildLowLTread ( void );
void buildLowRTread ( void );
void buildLowBarrel ( void );

class TankSceneNode;
class OpenGLTexture;

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
    void		setTexture(const int);

    void		notifyStyleChange();
    void		addRenderNodes(SceneRenderer&);
    void		addShadowNodes(SceneRenderer&);

    void		setNormal();
    void		setObese();
    void		setTiny();
    void		setNarrow();
    void		setThief();
    void		setExplodeFraction(float t);
    void		setClipPlane(const GLfloat* plane);

    // hidden still renders shadow (turns off invisible)
    void		setHidden(bool hidden = true);

    // invisible renders nothing (turns off hidden)
    void		setInvisible(bool invisible = true);

    static void		setMaxLOD(int maxLevel);

  protected:
    class TankRenderNode : public RenderNode {
      public:
	enum Style {
			Normal = 0,
			Obese,
			Tiny,
			Narrow,
			Thief,
			lastStyle
	};

			TankRenderNode(const TankSceneNode*);
			~TankRenderNode();
	void		setShadow();
	void		sortOrder(bool above, bool towards);
	void		render();
	const GLfloat*	getPosition() { return sceneNode->getSphere(); }
      protected:
	enum Part {
			Body = 0,
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
	bool		isShadow;
	bool		above;
	bool		towards;
	bool		isExploding;
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
	static GLuint	parts[lastStyle];
	friend class TankSceneNode;
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
	static GLuint	parts[lastStyle];
	friend class TankSceneNode;
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
	static GLuint	parts[lastStyle];
	friend class TankSceneNode;
    };
    friend class TankRenderNode;

  private:
    GLfloat		azimuth, elevation;
    GLfloat		baseRadius;
    bool		useOverride, hidden, invisible;
    bool		transparent, sort;
    float		explodeFraction;
    bool		clip;
    GLfloat		colorOverride[4];
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

extern float curVertScale[3]; /// for the really lame #def
extern float curNormScale[3]; /// for the really lame #def

void doVertex3f(GLfloat x, GLfloat y, GLfloat z);
void doNormal3f(GLfloat x, GLfloat y, GLfloat z);


#endif // BZF_TANK_SCENE_NODE_H

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

