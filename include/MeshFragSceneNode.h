/* bzflag
 * Copyright (c) 1993-2018 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* MeshFragSceneNode:
 *  Encapsulates information for rendering a mesh fragment
 *      (a collection of faces with the same material properties).
 *  Does not support level of detail.
 */

#ifndef BZF_MESH_FRAG_SCENE_NODE_H
#define BZF_MESH_FRAG_SCENE_NODE_H

#include "common.h"

/* common interface headers */
#include "WallSceneNode.h"
#include "BzMaterial.h"
#include "VBO_Handler.h"

//
// NOTES:
//
// - Make sure that "noPlane" is set to true, for Mesh Fragments can not be
//   used as occluders, and can not be culled as a simple plane
//
// - The lists are all GL_TRIANGLE lists
//

class MeshFace;

class MeshFragSceneNode : public WallSceneNode
{

public:
    MeshFragSceneNode(int faceCount, const MeshFace** faces);
    ~MeshFragSceneNode();

    // virtual functions from SceneNode
    bool cull(const ViewFrustum&) const;
    void addShadowNodes(SceneRenderer&);
    void addRenderNodes(SceneRenderer&);

    // virtual functions from WallSceneNode
    bool inAxisBox(const Extents& exts) const;

    void getRenderNodes(std::vector<RenderSet>& rnodes);

protected:
    class Geometry : public RenderNode, VBOclient
    {
    public:
        Geometry(MeshFragSceneNode* node);
        ~Geometry();

        virtual void initVBO();

        void init();
        void setStyle(int _style)
        {
            style = _style;
        }
        void render();
        void renderShadow();
        const GLfloat* getPosition() const
        {
            return sceneNode->getSphere();
        }

    private:
        inline void renderVBO();

        int style;
        int vboIndex;
        MeshFragSceneNode* sceneNode;
    };

private:
    Geometry renderNode;

    GLint faceCount;
    const MeshFace** faces;

    bool noRadar;
    bool noShadow;
    GLint arrayCount;
    GLfloat* vertices;
    GLfloat* normals;
    GLfloat* texcoords;

    friend class MeshFragSceneNode::Geometry;
};


#endif // BZF_MESH_FRAG_SCENE_NODE_H

// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
