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

/* SphereSceneNode:
 *	Encapsulates information for rendering a sphere.
 */

#ifndef	BZF_SPHERE_SCENE_NODE_H
#define	BZF_SPHERE_SCENE_NODE_H

#include "common.h"
#include "SceneNode.h"


//============================================================================//

class SphereSceneNode : public SceneNode {
  public:
    SphereSceneNode(const fvec3& pos, float radius);
    virtual ~SphereSceneNode();

    void setColor(float r, float g, float b, float a = 1.0f);
    void setColor(const fvec4& rgba);
    void move(const fvec3& pos, float radius);
    void notifyStyleChange();

    virtual void setShockWave(bool) { return; };

    virtual SceneNode** getParts(int& numParts) = 0;

    virtual void addRenderNodes(SceneRenderer&) = 0;
    virtual void addShadowNodes(SceneRenderer&) = 0;

  protected:
    float		radius;
    fvec4		color;
    bool		transparent;
    OpenGLGState	gstate;
};


//============================================================================//

const int sphereLods = 5;

class SphereLodSceneNode : public SphereSceneNode {
  public:
    SphereLodSceneNode(const fvec3& pos, float radius);
    ~SphereLodSceneNode();

    void setColor(float r, float g, float b, float a = 1.0f);
    void setColor(const fvec4& rgba);
    void move(const fvec3& pos, float radius);

    void setShockWave(bool value);

    // this node just won't split
    SceneNode** getParts(int&) { return NULL; }

    void addRenderNodes(SceneRenderer&);
    void addShadowNodes(SceneRenderer&);

    static void init();
    static void kill();
    static void initContext(void*);
    static void freeContext(void*);

  protected:
    class SphereLodRenderNode : public RenderNode {
      friend class SphereLodSceneNode;
      public:
	SphereLodRenderNode(const SphereLodSceneNode*);
	~SphereLodRenderNode();
	void setLod(int lod);
	void render();
	const fvec3& getPosition() const { return sceneNode->getCenter(); }

      private:
	const SphereLodSceneNode* sceneNode;
	int lod;
    };

  private:
    SphereLodRenderNode	renderNode;
    bool shockWave;
    bool inside;

    static bool initialized;
    static unsigned int lodLists[sphereLods];
    static float lodPixelsSqr[sphereLods];
    static int listTriangleCount[sphereLods];

    friend class SphereLodSceneNode::SphereLodRenderNode;
};


//============================================================================//

const int		SphereRes = 8;
const int		SphereLowRes = 6;

class SphereBspSceneNode;

class SphereFragmentSceneNode : public SceneNode {
  public:
			SphereFragmentSceneNode(int theta, int phi,
					SphereBspSceneNode* sphere);
			~SphereFragmentSceneNode();

    void		move();

    void		addRenderNodes(SceneRenderer&);
    void		addShadowNodes(SceneRenderer&);

  // Irix 7.2.1 and solaris compilers appear to have a bug.  if the
  // following declaration isn't public it generates an error when trying
  // to declare SphereFragmentSceneNode::FragmentRenderNode a friend in
  // SphereBspSceneNode::SphereBspRenderNode.  i think this is a bug in the
  // compiler because:
  //   no other compiler complains
  //   public/protected/private adjust access not visibility
  //     SphereBspSceneNode isn't requesting access, it's granting it
//  protected:
  public:
    class FragmentRenderNode : public RenderNode {
      public:
			FragmentRenderNode(const SphereBspSceneNode*,
				int theta, int phi);
			~FragmentRenderNode();
	const fvec3&	getVertex() const;
	void		render();
	const fvec3&	getPosition() const;
      private:
	const SphereBspSceneNode*	sceneNode;
	int		theta, phi;
	int		theta2, phi2;
    };
    friend class FragmentRenderNode;

  private:
    SphereBspSceneNode*	parentSphere;
    FragmentRenderNode	renderNode;
};

class SphereBspSceneNode : public SphereSceneNode {
  friend class SphereFragmentSceneNode;
  friend class SphereFragmentSceneNode::FragmentRenderNode;
  public:
			SphereBspSceneNode(const fvec3& pos, float radius);
			~SphereBspSceneNode();

    void		setColor(float r, float g,
				 float b, float a = 1.0f);
    void		setColor(const fvec4& rgba);
    void		move(const fvec3& pos, float radius);

    void		addRenderNodes(SceneRenderer&);
    void		addShadowNodes(SceneRenderer&);

    SceneNode**		getParts(int& numParts);

  protected:
    float		getRadius() const { return radius; }

  private:
    void		freeParts();

  protected:
    class SphereBspRenderNode : public RenderNode {
      friend class SphereBspSceneNode;
      friend class SphereFragmentSceneNode::FragmentRenderNode;
      public:
			SphereBspRenderNode(const SphereBspSceneNode*);
			~SphereBspRenderNode();
	void		setHighResolution(bool);
	void		setBaseIndex(int index);
	void		render();
	const fvec3&	getPosition() const { return sceneNode->getCenter(); }
      private:
	const SphereBspSceneNode* sceneNode;
	bool		highResolution;
	int		baseIndex;
	static fvec3	geom[2 * SphereRes * (SphereRes + 1)];
	static fvec3	lgeom[SphereLowRes * (SphereLowRes + 1)];
    };
    friend class SphereBspRenderNode;

  private:
    SphereBspRenderNode	renderNode;
    SphereFragmentSceneNode** parts;
};


//============================================================================//


#endif // BZF_FLAG_SCENE_NODE_H

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
