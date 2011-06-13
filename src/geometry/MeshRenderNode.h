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

#ifndef _MESH_RENDER_NODE_H
#define _MESH_RENDER_NODE_H


// common headers
#include "bzfgl.h"
#include "vectors.h"
#include "ogl/RenderNode.h"


class Extents;
class MeshDrawMgr;


class MeshRenderNode : public RenderNode {
  public:
    MeshRenderNode(MeshDrawMgr* drawMgr,
                   GLuint* xformList, bool normalize,
                   const fvec4* color, int lod, int set, int triangles);
    void render();
    void renderRadar();
    void renderShadow();
    const fvec3& getPosition() const { return pos; }
    void setPosition(const fvec3& p) { pos = p; }
    void setExtents(const Extents* extPtr) { exts = extPtr; }

  private:
    void drawV() const;
    void drawVN() const;
    void drawVT() const;
    void drawVTN() const;

  private:
    MeshDrawMgr* drawMgr;
    GLuint* xformList;
    bool normalize;
    int lod, set;
    const fvec4* color;
    const Extents* exts;
    int triangles;

  private:
    fvec3 pos;
};


#endif // _MESH_RENDER_NODE_H


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=2 tabstop=8 expandtab
