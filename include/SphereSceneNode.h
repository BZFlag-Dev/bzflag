/* bzflag
 * Copyright (c) 1993 - 2005 Tim Riker
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

const int		SphereRes = 8;
const int		SphereLowRes = 6;

class SphereSceneNode;

class SphereFragmentSceneNode : public SceneNode {
  public:
			SphereFragmentSceneNode(int theta, int phi,
					SphereSceneNode* sphere);
			~SphereFragmentSceneNode();

    void		move();

    void		addRenderNodes(SceneRenderer&);
    void		addShadowNodes(SceneRenderer&);

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
    class FragmentRenderNode : public RenderNode {
      public:
			FragmentRenderNode(const SphereSceneNode*,
				int theta, int phi);
			~FragmentRenderNode();
	const GLfloat*	getVertex() const;
	void		render();
	const GLfloat*	getPosition() const;
      private:
	const SphereSceneNode*	sceneNode;
	int		theta, phi;
	int		theta2, phi2;
    };
    friend class FragmentRenderNode;

  private:
    SphereSceneNode*	parentSphere;
    FragmentRenderNode	renderNode;
};

class SphereSceneNode : public SceneNode {
  friend class SphereFragmentSceneNode;
  friend class SphereFragmentSceneNode::FragmentRenderNode;
  public:
			SphereSceneNode(const GLfloat pos[3], GLfloat radius);
			~SphereSceneNode();

    void		setColor(GLfloat r, GLfloat g,
				 GLfloat b, GLfloat a = 1.0f);
    void		setColor(const GLfloat* rgba);
    void		move(const GLfloat pos[3], GLfloat radius);

    void		notifyStyleChange();
    void		addRenderNodes(SceneRenderer&);
    void		addShadowNodes(SceneRenderer&);

    SceneNode**		getParts(int& numParts);

  protected:
    GLfloat		getRadius() const { return radius; }

  private:
    void		freeParts();

  protected:
    class SphereRenderNode : public RenderNode {
      friend class SphereSceneNode;
      friend class SphereFragmentSceneNode::FragmentRenderNode;
      public:
			SphereRenderNode(const SphereSceneNode*);
			~SphereRenderNode();
	void		setHighResolution(bool);
	void		setBaseIndex(int index);
	void		render();
	const GLfloat*	getPosition() const { return sceneNode->getSphere(); }
      private:
	const SphereSceneNode* sceneNode;
	bool		highResolution;
	int		baseIndex;
	static GLfloat	geom[2 * SphereRes * (SphereRes + 1)][3];
	static GLfloat	lgeom[SphereLowRes * (SphereLowRes + 1)][3];
    };
    friend class SphereRenderNode;

  private:
    GLfloat		radius;
    GLfloat		color[4];
    bool		transparent;
    bool		lighting;
    OpenGLGState	gstate;
    SphereRenderNode	renderNode;
    SphereFragmentSceneNode**	parts;
};

#endif // BZF_FLAG_SCENE_NODE_H

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

