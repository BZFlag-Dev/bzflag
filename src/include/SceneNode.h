/* bzflag
 * Copyright (c) 1993-2010 Tim Riker
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
 *  Encapsulates information for rendering an object in the scene.
 */

#ifndef BZF_SCENE_NODE_H
#define BZF_SCENE_NODE_H

#include "common.h"
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

#define myColor3f(r, g, b)  SceneNode::glColor3f(r, g, b)
#define myColor4f(r, g, b, a) SceneNode::glColor4f(r, g, b, a)
#define myColor3fv(rgb)   SceneNode::glColor3fv(rgb)
#define myColor4fv(rgba)  SceneNode::glColor4fv(rgba)
#define myStipple(alpha)  SceneNode::setStipple(alpha)


class ViewFrustum;
class SceneRenderer;


class SceneNode {
  public:
    SceneNode();
    virtual ~SceneNode();

    virtual void  notifyStyleChange();

    const fvec3&  getCenter() const;
    const fvec4&  getSphere() const;
    const Extents&  getExtents() const;
    virtual int   getVertexCount() const;
    virtual const fvec3& getVertex(int vertex) const;
    const fvec4*  getPlane() const;
    const fvec4&  getPlaneRaw() const;
    virtual float getDistanceSq(const fvec3& eye) const; // for BSP

    virtual bool  inAxisBox(const Extents& exts) const;

    virtual bool  cull(const ViewFrustum&) const;
    virtual bool  cullShadow(int pCount, const fvec4* planes) const;

    virtual void  addLight(SceneRenderer&);
    virtual int   split(const fvec4& plane,
                        SceneNode*& front, SceneNode*& back) const;
    virtual void  addShadowNodes(SceneRenderer&);
    virtual void  addRenderNodes(SceneRenderer&);
    virtual void  renderRadar();

    bool    isOccluder() const;
    void    setOccluder(bool value);

    bool    isAnimated() const;

    struct RenderSet {
      RenderNode* node;
      const OpenGLGState* gstate;
    };
    virtual void getRenderNodes(std::vector<RenderSet>& rnodes);

    static void setStipple(float alpha);

    static void setColorOverride(bool = true);

    static void glColor3fv(const float* rgb);
    static void glColor4fv(const float* rgba);
    static void glColor3f(float r, float g, float b);
    static void glColor4f(float r, float g, float b, float a);

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
    void setRadius(float radiusSquared);
    void setCenter(const fvec3& center);
    void setCenter(float x, float y, float z);
    void setSphere(const fvec4& sphere);

  private:
    SceneNode(const SceneNode&);
    SceneNode& operator=(const SceneNode&);

  protected:
    fvec4   plane;  // unit normal, distance to origin
    bool    noPlane;
    bool    occluder;
    Extents extents;
    bool    animated;

  private:
    fvec4 sphere;

  private:
    static bool showColor;
};


inline const fvec4* SceneNode::getPlane() const {
  if (noPlane) {
    return NULL;
  }
  return &plane;
}

inline const fvec4&   SceneNode::getPlaneRaw() const { return plane;        }
inline const fvec3&   SceneNode::getCenter()   const { return sphere.xyz(); }
inline const fvec4&   SceneNode::getSphere()   const { return sphere;       }
inline const Extents& SceneNode::getExtents()  const { return extents;      }
inline bool           SceneNode::isOccluder()  const { return occluder;     }
inline bool           SceneNode::isAnimated()  const { return animated;     }

inline void SceneNode::setOccluder(bool value) { occluder = value; }


//============================================================================//
//
//  float array helper classes
//

class fvec2Array {
  public:
    fvec2Array(int s) : size(s) {
      data = new fvec2[size];
    }
    ~fvec2Array() {
      delete[] data;
    }
    fvec2Array(const fvec2Array&);
    fvec2Array& operator=(const fvec2Array&);

    fvec2& operator[](int i)       { return data[i]; }
    const fvec2& operator[](int i) const { return data[i]; }

    int           getSize() const { return size; }
    const fvec2* getArray() const { return data; }

  private:
    int    size;
    fvec2* data;
};


class fvec3Array {
  public:
    fvec3Array(int s) : size(s) {
      data = new fvec3[size];
    }
    ~fvec3Array() {
      delete[] data;
    }
    fvec3Array(const fvec3Array&);
    fvec3Array& operator=(const fvec3Array&);

    fvec3& operator[](int i)       { return data[i]; }
    const fvec3& operator[](int i) const { return data[i]; }

    int          getSize()  const { return size; }
    const fvec3* getArray() const { return data; }

  private:
    int    size;
    fvec3* data;
};


#endif // BZF_SCENE_NODE_H

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=2 tabstop=8
