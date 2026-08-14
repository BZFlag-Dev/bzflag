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

/* QuadWallSceneNode:
 *  Encapsulates information for rendering a quadrilateral wall.
 */

#ifndef BZF_QUAD_WALL_SCENE_NODE_H
#define BZF_QUAD_WALL_SCENE_NODE_H

#include <vector>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

// Inherits from
#include "WallSceneNode.h"

class QuadWallSceneNode final : public WallSceneNode
{
public:
    QuadWallSceneNode(glm::vec3 base,
                      glm::vec3 sEdge,
                      glm::vec3 tEdge,
                      float uRepeats = 1.0,
                      float vRepeats = 1.0,
                      bool makeLODs = true,
                      bool fixedUVs = false);
    QuadWallSceneNode(glm::vec3 base,
                      glm::vec3 sEdge,
                      glm::vec3 tEdge,
                      float uOffset,
                      float vOffset,
                      float uRepeats,
                      float vRepeats,
                      bool makeLODs);
    ~QuadWallSceneNode();

    int         split(const float*, SceneNode*&, SceneNode*&) const override;

    void        addRenderNodes(SceneRenderer&) override;
    void        addShadowNodes(SceneRenderer&) override;
    void        renderRadar() override;


    bool        inAxisBox (const Extents& exts) const override;

    int         getVertexCount () const override;
    const       GLfloat* getVertex (int vertex) const override;

    void    getRenderNodes(std::vector<RenderSet>& rnodes) override;

private:
    void        init(glm::vec3 base,
                     glm::vec3 uEdge,
                     glm::vec3 vEdge,
                     float uOffset,
                     float vOffset,
                     float uRepeats,
                     float vRepeats,
                     bool makeLODs, bool fixedUVs);

protected:
    class Geometry final : public RenderNode
    {
    public:
        Geometry(QuadWallSceneNode*,
                 int uCount, int vCount,
                 glm::vec3 base,
                 glm::vec3 uEdge,
                 glm::vec3 vEdge,
                 const GLfloat* normal,
                 float uOffset, float vOffset,
                 float uRepeats, float vRepeats,
                 bool fixedUVs);
        ~Geometry();
        void        setStyle(int _style)
        {
            style = _style;
        }
        void        render() override;
        void        renderShadow() override;
        const GLfloat*  getVertex(int i) const;
        const GLfloat* getPosition() const override;
    private:
        void        drawV() const;
        void        drawVT() const;
    private:
        WallSceneNode*  wall;
        int     style;
        int     ds, dt;
        int     dsq, dsr;
        const GLfloat*  normal;
    public:
        std::vector<glm::vec3> vertex;
        std::vector<glm::vec2> uv;
        int      triangles;
    };

private:
    Geometry**      nodes;
    Geometry*       shadowNode;
};

#endif // BZF_QUAD_WALL_SCENE_NODE_H

// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
