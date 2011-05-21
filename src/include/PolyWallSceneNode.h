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

/* PolyWallSceneNode:
 *  Encapsulates information for rendering a planar
 *  polygonal wall.  Does not support level of detail.
 */

#ifndef BZF_POLY_WALL_SCENE_NODE_H
#define BZF_POLY_WALL_SCENE_NODE_H

#include "common.h"
#include "WallSceneNode.h"

class PolyWallSceneNode : public WallSceneNode {
  public:
    PolyWallSceneNode(const fvec3Array& vertices,
                      const fvec2Array& uvs);
    ~PolyWallSceneNode();

    int     split(const fvec4&, SceneNode*&, SceneNode*&) const;

    void    addRenderNodes(SceneRenderer&);
    void    addShadowNodes(SceneRenderer&);
    void    renderRadar();

    void    getRenderNodes(std::vector<RenderSet>& rnodes);

  protected:
    class Geometry : public RenderNode {
      public:
        Geometry(PolyWallSceneNode*,
                 const fvec3Array& vertices,
                 const fvec2Array& uvs,
                 const float* normal);
        ~Geometry();
        void    setStyle(int _style) { style = _style; }
        void    render();
        const fvec3&  getPosition() const { return wall->getCenter(); }
      private:
        void    drawV() const;
        void    drawVT() const;
      private:
        PolyWallSceneNode* wall;
        int   style;
        const float*  normal;
      public:
        fvec3Array  vertex;
        fvec2Array  uv;
    };

  private:
    Geometry*   node;
    Geometry*   shadowNode;
};

#endif // BZF_POLY_WALL_SCENE_NODE_H

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=2 tabstop=8
