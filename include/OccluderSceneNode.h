/* bzflag
 * Copyright (c) 1993 - 2007 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* OccluderSceneNode:
 *	Encapsulates information for an occluder
 *      plane. Occluders are never rendered.
 */

#ifndef	BZF_OCCLUDER_SCENE_NODE_H
#define	BZF_OCCLUDER_SCENE_NODE_H

#include "common.h"

// common implementation headers
#include "SceneNode.h"
#include "MeshFace.h"

class OccluderSceneNode : public SceneNode {

  public:
    OccluderSceneNode(const MeshFace* face);
    ~OccluderSceneNode();

    // virtual functions from SceneNode
    bool cull(const ViewFrustum&) const;
    bool inAxisBox(const Extents& exts) const;
    void addShadowNodes(SceneRenderer&) { return; }
    void addRenderNodes(SceneRenderer&) { return; }
    void renderRadar() { return; }

    int getVertexCount () const
      { return vertexCount; }
    const GLfloat* getVertex (int vertex) const
      { return vertices[vertex]; }

  private:
    int vertexCount;
    GLfloat3* vertices;
};


#endif // BZF_OCCLUDER_SCENE_NODE_H

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
