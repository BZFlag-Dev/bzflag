/* bzflag
 * Copyright (c) 1993 - 2006 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* SceneNode:
 *	Encapsulates information for rendering an object in the scene.
 *
 * GLfloat2
 * GLfloat3
 *	Arrays of two and three GLfloat's
 *
 * GLfloat2Array
 * GLfloat3Array
 *	Arrays of GLfloat2's and GLfloat3's
 *
 * Probably shouldn't use names like this (GLfloat...).  Oh well.
 */

#ifndef	BZF_SCENE_NODE_H
#define	BZF_SCENE_NODE_H

#include "common.h"

#include <crystalspace.h>

#include "bzfgl.h"
#include "bzfio.h"
#include "OpenGLGState.h"
#include "RenderNode.h"
#include "Extents.h"
#include <vector>

#if !defined(_WIN32)
// bonehead win32 cruft.  just make it go away on other platforms.
#ifdef __stdcall
#undef __stdcall
#endif
#define	__stdcall
#endif

#define	myColor3f(r, g, b)	SceneNode::glColor3f(r, g, b)
#define	myColor4f(r, g, b, a)	SceneNode::glColor4f(r, g, b, a)
#define	myColor3fv(rgb)		SceneNode::glColor3fv(rgb)
#define	myColor4fv(rgba)	SceneNode::glColor4fv(rgba)
#define	myStipple(alpha)	SceneNode::setStipple(alpha)

class ViewFrustum;
class SceneRenderer;

class SceneNode {
  public:
			SceneNode();
    virtual		~SceneNode();

    virtual void	notifyStyleChange();

    const GLfloat*	getSphere() const;
    const Extents&	getExtents() const;
    virtual int		getVertexCount () const;
    virtual const GLfloat* getVertex (int vertex) const;
    const GLfloat*      getPlane() const;
    const GLfloat*      getPlaneRaw() const;
    virtual GLfloat	getDistance(const GLfloat* eye) const; // for BSP

    virtual bool	inAxisBox (const Extents& exts) const;

    virtual bool	cull(const ViewFrustum&) const;
    virtual bool	cullShadow(int pCount, const float (*planes)[4]) const;

    bool		isOccluder() const;
    void		setOccluder(bool value);

    virtual void	addLight(SceneRenderer&);
    virtual int		split(const float* plane,
			      SceneNode*& front, SceneNode*& back) const;
    virtual void	addShadowNodes(SceneRenderer&);
    virtual void	addRenderNodes(SceneRenderer&);
    virtual void	renderRadar();

    virtual void        addToEngine(csRef<iEngine>, iSector *) {;};

    struct RenderSet {
      RenderNode* node;
      const OpenGLGState* gstate;
    };
    virtual void getRenderNodes(std::vector<RenderSet>& rnodes);


    static void		setColorOverride(bool = true);
    static void		glColor3f(GLfloat r, GLfloat g, GLfloat b)
#ifdef __MINGW32__
      {if (!colorOverride) ::glColor3f(r, g, b); };
#else
      { (*color3f)(r, g, b); }
#endif

    static void		glColor4f(GLfloat r, GLfloat g, GLfloat b, GLfloat a)
#ifdef __MINGW32__
      {if (!colorOverride) ::glColor4f(r, g, b, a); };
#else
      { (*color4f)(r, g, b, a); }
#endif

    static void		glColor3fv(const GLfloat* rgb)
#ifdef __MINGW32__
      {if (!colorOverride) ::glColor3fv(rgb); };
#else
      { (*color3fv)(rgb); }
#endif

    static void		glColor4fv(const GLfloat* rgba)
#ifdef __MINGW32__
      {if (!colorOverride) ::glColor4fv(rgba); };
#else
      { (*color4fv)(rgba); }
#endif

    static void		setStipple(GLfloat alpha) { (*stipple)(alpha); }

    enum CullState {
      OctreeCulled,
      OctreePartial,
      OctreeVisible
    };

    /** This boolean is used by the Octree code.
	Someone else can 'friend'ify it later.
    */
    CullState octreeState;

  protected:
    void		setRadius(GLfloat radiusSquared);
    void		setCenter(const GLfloat center[3]);
    void		setCenter(GLfloat x, GLfloat y, GLfloat z);
    void		setSphere(const GLfloat sphere[4]);

  private:
			SceneNode(const SceneNode&);
    SceneNode&		operator=(const SceneNode&);



#ifndef __MINGW32__
    static void __stdcall	noColor3f(GLfloat, GLfloat, GLfloat);
    static void __stdcall	noColor4f(GLfloat, GLfloat, GLfloat, GLfloat);
    static void __stdcall	noColor3fv(const GLfloat*);
    static void __stdcall	noColor4fv(const GLfloat*);
#endif
    static void			noStipple(GLfloat);

  protected:
    GLfloat		plane[4];	// unit normal, distance to origin
    bool		noPlane;
    bool		occluder;
    Extents		extents;
    static bool         errored;
  private:
    GLfloat		sphere[4];
#ifdef __MINGW32__
    static bool	 colorOverride;
#else
    static void		(__stdcall *color3f)(GLfloat, GLfloat, GLfloat);
    static void		(__stdcall *color4f)(GLfloat, GLfloat, GLfloat, GLfloat);
    static void		(__stdcall *color3fv)(const GLfloat*);
    static void		(__stdcall *color4fv)(const GLfloat*);
#endif
    static void		(*stipple)(GLfloat);
};

inline const GLfloat*   SceneNode::getPlane() const
{
  if (noPlane)
    return NULL;
  return plane;
}

inline const GLfloat*   SceneNode::getPlaneRaw() const
{
  return plane;
}

inline const GLfloat*	SceneNode::getSphere() const
{
  return sphere;
}

inline const Extents&	SceneNode::getExtents() const
{
  return extents;
}

inline bool		SceneNode::isOccluder() const
{
  return occluder;
}

inline void		SceneNode::setOccluder(bool value)
{
  occluder = value;
}


typedef GLfloat		GLfloat2[2];
typedef GLfloat		GLfloat3[3];

class GLfloat2Array {
  public:
			GLfloat2Array(int s) : size(s)
				{ data = new GLfloat2[size]; }
			GLfloat2Array(const GLfloat2Array&);
			~GLfloat2Array() { delete[] data; }
    GLfloat2Array&	operator=(const GLfloat2Array&);
    GLfloat*		operator[](int i) { return data[i]; }
    const GLfloat*	operator[](int i) const { return data[i]; }
    int			getSize() const { return size; }
    const GLfloat2*	getArray() const { return data; }

  private:
    int			size;
    GLfloat2*		data;
};

class GLfloat3Array {
  public:
			GLfloat3Array(int s) : size(s)
				{ data = new GLfloat3[size]; }
			GLfloat3Array(const GLfloat3Array&);
			~GLfloat3Array() { delete[] data; }
    GLfloat3Array&	operator=(const GLfloat3Array&);
    GLfloat*		operator[](int i) { return data[i]; }
    const GLfloat*	operator[](int i) const { return data[i]; }
    int			getSize() const { return size; }
    const GLfloat3*	getArray() const { return data; }

  private:
    int			size;
    GLfloat3*		data;
};

#endif // BZF_SCENE_NODE_H

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

