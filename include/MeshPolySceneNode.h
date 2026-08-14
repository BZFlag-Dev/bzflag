/* bzflag
 * Copyright (c) 1993-2025 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* MeshPolySceneNode:
 *  Encapsulates information for rendering a planar
 *  polygonal wall.  Does not support level of detail.
 */

#ifndef BZF_MESH_POLY_SCENE_NODE_H
#define BZF_MESH_POLY_SCENE_NODE_H

#include <vector>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/gtc/type_ptr.hpp>

// Inherits from
#include "WallSceneNode.h"

class MeshPolySceneNode final : public WallSceneNode
{
public:
    MeshPolySceneNode(const float plane[4],
                      bool noRadar, bool noShadow,
                      const std::vector<glm::vec3>& vertices,
                      const std::vector<glm::vec3>& normals,
                      const std::vector<glm::vec2>& texcoords);
    ~MeshPolySceneNode() = default;

    bool cull(const ViewFrustum& frustum) const override;
    bool inAxisBox (const Extents& exts) const override;
    int getVertexCount () const override;
    const GLfloat* getVertex (int vertex) const override;
    const glm::vec3* getVertices() const;

    int split(const float* plane, SceneNode*&, SceneNode*&) const override;

    void addRenderNodes(SceneRenderer&) override;
    void addShadowNodes(SceneRenderer&) override;
    void renderRadar() override;

    void getRenderNodes(std::vector<RenderSet>& rnodes) override;


protected:
    class Geometry final : public RenderNode
    {
    public:
        Geometry(MeshPolySceneNode* sceneNode,
                 const std::vector<glm::vec3>& vertices,
                 const std::vector<glm::vec3>& normals,
                 const std::vector<glm::vec2>& texcoords,
                 const GLfloat* normal);
        ~Geometry() = default;
        void setStyle(int _style)
        {
            style = _style;
        }
        void render() override;
        void renderShadow() override;
        const GLfloat* getVertex(int i) const;
        const glm::vec3* getVertices() const;
        int getVertexCount() const;
        const GLfloat* getPosition() const override;
    private:
        void drawV() const; // draw with just vertices
        void drawVT() const; // draw with texcoords
        void drawVN() const; // draw with normals
        void drawVTN() const; // draw with texcoords and normals
    private:
        MeshPolySceneNode* sceneNode;
        int style;
        const GLfloat* normal;
    public:
        std::vector<glm::vec3> vertices;
        std::vector<glm::vec3> normals;
        std::vector<glm::vec2> texcoords;
    };

private:
    int splitWallVTN(const GLfloat* plane,
                     const std::vector<glm::vec3>& vertices,
                     const std::vector<glm::vec3>& normals,
                     const std::vector<glm::vec2>& texcoords,
                     SceneNode*& front, SceneNode*& back) const;

    void splitEdgeVTN(float d1, float d2,
                      glm::vec3 p1,  glm::vec3 p2,
                      glm::vec3 n1,  glm::vec3 n2,
                      glm::vec2 uv1, glm::vec2 uv2,
                      glm::vec3& p, glm::vec3& n, glm::vec2& uv) const;

    int splitWallVT(const GLfloat* plane,
                    const std::vector<glm::vec3>& vertices,
                    const std::vector<glm::vec2>& texcoords,
                    SceneNode*& front, SceneNode*& back) const;

    void splitEdgeVT(float d1, float d2,
                     glm::vec3 p1,  glm::vec3 p2,
                     glm::vec2 uv1, glm::vec2 uv2,
                     glm::vec3& p, glm::vec2& uv) const;

    Geometry node;
    bool noRadar;
    bool noShadow;
};

inline int MeshPolySceneNode::Geometry::getVertexCount() const
{
    return static_cast<int>(vertices.size());
}

inline int MeshPolySceneNode::getVertexCount () const
{
    return node.getVertexCount();
}

inline const GLfloat* MeshPolySceneNode::Geometry::getVertex(int i) const
{
    return glm::value_ptr(vertices[i]);
}

inline const glm::vec3* MeshPolySceneNode::Geometry::getVertices() const
{
    return vertices.data();
}

inline const GLfloat* MeshPolySceneNode::getVertex(int i) const
{
    return node.getVertex(i);
}

inline const glm::vec3* MeshPolySceneNode::getVertices() const
{
    return node.getVertices();
}


#endif // BZF_MESH_POLY_SCENE_NODE_H

// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
