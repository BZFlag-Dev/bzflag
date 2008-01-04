/* bzflag
 * Copyright (c) 1993 - 2008 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef _MESH_RENDER_NODE_H
#define _MESH_RENDER_NODE_H


// common implementation headers
#include "bzfgl.h"
#include "RenderNode.h"


class Extents;
class MeshDrawMgr;


class OpaqueRenderNode : public RenderNode {
  public:
    OpaqueRenderNode(MeshDrawMgr* drawMgr,
		     GLuint* xformList, bool normalize,
		     const GLfloat* color, int lod, int set,
		     const Extents* exts, int triangles);
    void render();
    void renderRadar();
    void renderShadow();
    virtual const GLfloat* getPosition() const { return NULL;}
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
    const GLfloat* color;
    const Extents* exts;
    int triangles;
};


class AlphaGroupRenderNode : public OpaqueRenderNode {
  public:
    AlphaGroupRenderNode(MeshDrawMgr* drawMgr,
			 GLuint* xformList, bool normalize,
			 const GLfloat* color, int lod, int set,
			 const Extents* exts, const float pos[3],
			 int triangles);
    const GLfloat* getPosition() const { return pos; }
    void setPosition(const GLfloat* pos);
  private:
    float pos[3];
};


#endif // _MESH_RENDER_NODE_H


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
