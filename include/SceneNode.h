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
#include "bzfgl.h"
#include "bzfio.h"
#include "OpenGLGState.h"
#include "RenderNode.h"
#include "Extents.h"
#include "vectors.h"
#include <vector>

#if !defined(_WIN32)
// bonehead win32 cruft.  just make it go away on other platforms.
#  ifdef __stdcall
#    undef __stdcall
#  endif
#  define __stdcall
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
    virtual ~SceneNode();

    virtual void	notifyStyleChange();

    const fvec3&	getCenter() const;
    const fvec4&	getSphere() const;
    const Extents&	getExtents() const;
    virtual int		getVertexCount () const;
    virtual const GLfloat* getVertex (int vertex) const;
    const fvec4*	getPlane() const;
    const fvec4&	getPlaneRaw() const;
    virtual GLfloat	getDistance(const GLfloat* eye) const; // for BSP

    virtual bool	inAxisBox (const Extents& exts) const;

    virtual bool	cull(const ViewFrustum&) const;
    virtual bool	cullShadow(int pCount, const fvec4* planes) const;

    virtual void	addLight(SceneRenderer&);
    virtual int		split(const fvec4& plane,
			      SceneNode*& front, SceneNode*& back) const;
    virtual void	addShadowNodes(SceneRenderer&);
    virtual void	addRenderNodes(SceneRenderer&);
    virtual void	renderRadar();

    bool		isOccluder() const;
    void		setOccluder(bool value);

    struct RenderSet {
      RenderNode* node;
      const OpenGLGState* gstate;
    };
    virtual void getRenderNodes(std::vector<RenderSet>& rnodes);

    static void setStipple(GLfloat alpha) { (*stipple)(alpha); }

    static void setColorOverride(bool = true);

#ifdef __MINGW32__
    static void glColor3f(GLfloat r, GLfloat g, GLfloat b) {
      if (!colorOverride) { ::glColor3f(r, g, b); }
    }
    static void glColor4f(GLfloat r, GLfloat g, GLfloat b, GLfloat a) {
      if (!colorOverride) { ::glColor4f(r, g, b, a); }
    }
    static void glColor3fv(const GLfloat* rgb) {
      if (!colorOverride) { ::glColor3fv(rgb); }
    }
    static void glColor4fv(const GLfloat* rgba) {
      if (!colorOverride) { ::glColor4fv(rgba); }
    }
#else
    static void glColor3f(GLfloat r, GLfloat g, GLfloat b) {
      (*color3f)(r, g, b);
    }
    static void glColor4f(GLfloat r, GLfloat g, GLfloat b, GLfloat a) {
      (*color4f)(r, g, b, a);
    }
    static void glColor3fv(const GLfloat* rgb) {
      (*color3fv)(rgb);
    }
    static void glColor4fv(const GLfloat* rgba) {
      (*color4fv)(rgba);
    }
#endif // __MINGW32__

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
    void		setCenter(const fvec3& center);
    void		setCenter(GLfloat x, GLfloat y, GLfloat z);
    void		setSphere(const fvec4& sphere);

  private:
    SceneNode(const SceneNode&);
    SceneNode& operator=(const SceneNode&);

    static void noStipple(GLfloat);

#ifndef __MINGW32__
    static void __stdcall noColor3f(GLfloat, GLfloat, GLfloat);
    static void __stdcall noColor4f(GLfloat, GLfloat, GLfloat, GLfloat);
    static void __stdcall noColor3fv(const GLfloat*);
    static void __stdcall noColor4fv(const GLfloat*);
#endif

  protected:
    fvec4   plane;  // unit normal, distance to origin
    bool    noPlane;
    bool    occluder;
    Extents extents;

  private:
    fvec4 sphere;

  private:
    static void (*stipple)(GLfloat);
#ifdef __MINGW32__
    static bool colorOverride;
#else
    static void (__stdcall *color3f)(GLfloat, GLfloat, GLfloat);
    static void (__stdcall *color4f)(GLfloat, GLfloat, GLfloat, GLfloat);
    static void (__stdcall *color3fv)(const GLfloat*);
    static void (__stdcall *color4fv)(const GLfloat*);
#endif
};


inline const fvec4* SceneNode::getPlane() const
{
  if (noPlane) {
    return NULL;
  }
  return &plane;
}

inline const fvec4&   SceneNode::getPlaneRaw() const { return plane;          }
inline const fvec3&   SceneNode::getCenter()   const { return (fvec3&)sphere; }
inline const fvec4&   SceneNode::getSphere()   const { return sphere;         }
inline const Extents& SceneNode::getExtents()  const { return extents;        }
inline bool           SceneNode::isOccluder()  const { return occluder;       }

inline void SceneNode::setOccluder(bool value) { occluder = value; }


//============================================================================//
//
//  GLfloat array helper classes
//

typedef GLfloat GLfloat2[2];
typedef GLfloat GLfloat3[3];


class GLfloat2Array {
  public:
    GLfloat2Array(int s) : size(s) { data = new GLfloat2[size]; }
    GLfloat2Array(const GLfloat2Array&);
    ~GLfloat2Array() { delete[] data; }

    GLfloat2Array& operator=(const GLfloat2Array&);
    GLfloat*       operator[](int i) { return data[i]; }
    const GLfloat* operator[](int i) const { return data[i]; }

    int             getSize()  const { return size; }
    const GLfloat2* getArray() const { return data; }

  private:
    int       size;
    GLfloat2* data;
};


class GLfloat3Array {
  public:
    GLfloat3Array(int s) : size(s) { data = new GLfloat3[size]; }
    GLfloat3Array(const GLfloat3Array&);
    ~GLfloat3Array() { delete[] data; }

    GLfloat3Array& operator=(const GLfloat3Array&);
    GLfloat*       operator[](int i) { return data[i]; }
    const GLfloat* operator[](int i) const { return data[i]; }

    int             getSize()  const { return size; }
    const GLfloat3* getArray() const { return data; }

  private:
    int       size;
    GLfloat3* data;
};


#endif // BZF_SCENE_NODE_H

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
