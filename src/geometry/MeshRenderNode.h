/* bzflag
 * Copyright (c) 1993 - 2005 Tim Riker
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
                     GLuint xformList, bool normalize,
                     const GLfloat* color, int lod, int set,
                     const Extents* exts);
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
    GLuint xformList;
    bool normalize;
    int lod, set;
    const GLfloat* color;
    const Extents* exts;
};


class AlphaGroupRenderNode : public OpaqueRenderNode {
  public:
    AlphaGroupRenderNode(MeshDrawMgr* drawMgr, GLuint xformList, bool normalize,
                         const GLfloat* color, int lod, int set,
                         const Extents* exts, const float pos[3]);
    const GLfloat* getPosition() const { return pos; }
  private:
    float pos[3];
};


class AlphaRenderNode : public RenderNode {
  public:
    AlphaRenderNode(MeshDrawMgr* drawMgr, GLuint xformList,
                    const GLfloat* color, const GLfloat* pos,
                    GLenum indexType, void* indices);
    ~AlphaRenderNode();
    void render();
    void renderRadar();
    void renderShadow();
    const GLfloat* getPosition() const { return pos; }
  private:
    void drawV() const;
    void drawVN() const;
    void drawVT() const;
    void drawVTN() const;
  private:
    MeshDrawMgr* drawMgr;
    GLuint xformList;
    float pos[3];
    const GLfloat* color;
    GLenum indexType;
    void* indices; // sadly, all are triangles
};


#endif // _MESH_RENDER_NODE_H


// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
