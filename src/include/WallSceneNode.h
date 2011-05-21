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

/* WallSceneNode:
 *  Encapsulates information for rendering an wall.
 *
 * WallGeometry:
 *  Encapsulates vertices and uv's for a wall
 *
 * Walls are flat and don't move.  A wall also picks a level of
 * detail based on its projected area and on the presence of
 * light sources nearby (to capture light highlights).
 */

#ifndef BZF_WALL_SCENE_NODE_H
#define BZF_WALL_SCENE_NODE_H

#include "common.h"
#include "SceneNode.h"
#include "vectors.h"


class BzMaterial;


class WallSceneNode : public SceneNode {
  public:
    WallSceneNode();
    ~WallSceneNode();

    float   getDistanceSq(const fvec3& eye) const;
    virtual bool  inAxisBox(const Extents& exts) const;

    void    setBzMaterial(const BzMaterial* bzmat);

    void    applyColor() const;

    bool    cull(const ViewFrustum&) const;
    void    notifyStyleChange();

    void    copyStyle(WallSceneNode*);

    inline const fvec4& getColor()    const { return color;    }
    inline const fvec4* getColorPtr() const { return colorPtr; }

    virtual void  setRadarSpecial(bool) { return; }

  protected:
    void    setNumLODs(int, float* elementAreas);
    void    setPlane(const fvec4&);
    int     pickLevelOfDetail(const SceneRenderer&) const;

    inline int      getStyle()      const { return style;   }
    inline int      getNumLODs()    const { return numLODs; }
    inline const OpenGLGState*  getGState()     const { return &gstate; }
    inline const OpenGLGState*  getWallGState() const { return &gstate; }

    static int    splitWall(const fvec4& plane,
                            const fvec3Array& vertices,
                            const fvec2Array& uvs,
                            SceneNode*& front, SceneNode*& back); // const

  private:
    static void splitEdge(float d1, float d2,
                          const fvec3& p1, const fvec3& p2,
                          const fvec2& uv1, const fvec2& uv2,
                          fvec3& p, fvec2& uv); //const
    void burnLighting();

  private:
    int     style;
    int     numLODs;
    float*    elementAreas;

    const BzMaterial* bzMaterial;
    fvec4   color;
    const fvec4*  colorPtr;

    OpenGLGState  gstate;
};


#endif // BZF_WALL_SCENE_NODE_H


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=2 tabstop=8
