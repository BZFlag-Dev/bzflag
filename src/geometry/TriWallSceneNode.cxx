/* bzflag
 * Copyright (c) 1993-2023 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#define GLM_ENABLE_EXPERIMENTAL

// interface header
#include "TriWallSceneNode.h"

// system headers
#include <math.h>
#include <stdlib.h>
#include <glm/gtx/norm.hpp>
#include <algorithm>

// common implementation headers
#include "Intersect.h"
#include "StateDatabase.h"

// FIXME (SceneRenderer.cxx is in src/bzflag)
#include "SceneRenderer.h"

//
// TriWallSceneNode::Geometry
//

TriWallSceneNode::Geometry::Geometry(TriWallSceneNode* _wall, int eCount,
                                     const glm::vec3 &base, const glm::vec3 &uEdge, const glm::vec3 &vEdge,
                                     const glm::vec3 &_normal, float uRepeats, float vRepeats) :
    wall(_wall), style(0), de(eCount), normal(_normal),
    vertex((eCount+1) * (eCount+2) / 2),
    uv((eCount+1) * (eCount+2) / 2)
{
    for (int n = 0, j = 0; j <= eCount; j++)
    {
        const int k = eCount - j;
        const float t = (float)j / (float)eCount;
        for (int i = 0; i <= k; n++, i++)
        {
            const float s = (float)i / (float)eCount;
            vertex[n] = base + s * uEdge + t * vEdge;
            uv[n] = glm::vec2(s * uRepeats, t * vRepeats);
        }
    }

    if (BZDB.isTrue("remapTexCoords"))
    {
        const auto uvLen = glm::vec2(glm::length(uEdge) / uRepeats,
                                     glm::length(vEdge) / vRepeats);
        const auto scale = 10.0f / glm::floor(10.0f * uvLen);
        if (fabsf(normal[2]) > 0.999f)
        {
            // horizontal surface
            for (unsigned int i = 0; i < vertex.size(); i++)
                uv[i] = scale * glm::vec2(vertex[i]);
        }
        else
        {
            // vertical surface
            const auto n = glm::normalize(glm::vec2(normal));
            const float vs = glm::inversesqrt(1.0f - normal[2] * normal[2]);
            for (unsigned int i = 0; i < vertex.size(); i++)
            {
                const auto v = vertex[i];
                const float uGeoScale = (n.x * v[1]) - (n.y * v[0]);
                const float vGeoScale = v[2] * vs;
                uv[i] = scale * glm::vec2(uGeoScale, vGeoScale);
            }
        }
    }

    triangles = (eCount * eCount);
}


TriWallSceneNode::Geometry::~Geometry()
{
    // do nothing
}


#define RENDER(_e)                          \
  for (int k = 0, t = 0; t < de; t++) {                 \
    int e = de - t;                         \
    glBegin(GL_TRIANGLE_STRIP);                     \
    for (int s = 0; s < e; k++, s++) {                  \
      _e(k+e+1);                            \
      _e(k);                                \
    }                                   \
    _e(k);                              \
    glEnd();                                \
    k++;                                \
  }
#define EMITV(_i)   glVertex3fv(vertex[_i])
#define EMITVT(_i)  glTexCoord2fv(uv[_i]); glVertex3fv(vertex[_i])

void            TriWallSceneNode::Geometry::render()
{
    wall->setColor();
    glNormal3f(normal.x, normal.y, normal.z);
    if (style >= 2)
        drawVT();
    else
        drawV();
    addTriangleCount(triangles);
    return;
}

void            TriWallSceneNode::Geometry::renderShadow()
{
    glBegin(GL_TRIANGLE_STRIP);
    glVertex3fv(vertex[(de + 1) * (de + 2) / 2 - 1]);
    glVertex3fv(vertex[0]);
    glVertex3fv(vertex[de]);
    glEnd();
    addTriangleCount(1);
}

const glm::vec3 TriWallSceneNode::Geometry::getPosition() const
{
    return wall->getCenter();
}


void            TriWallSceneNode::Geometry::drawV() const
{
    RENDER(EMITV)
}


void            TriWallSceneNode::Geometry::drawVT() const
{
    RENDER(EMITVT)
}


const glm::vec3 &TriWallSceneNode::Geometry::getVertex(int i) const
{
    return vertex[i];
}


//
// TriWallSceneNode
//

TriWallSceneNode::TriWallSceneNode(const glm::vec3 &base,
                                   const glm::vec3 &uEdge,
                                   const glm::vec3 &vEdge,
                                   float uRepeats,
                                   float vRepeats,
                                   bool makeLODs)
{
    // record plane info
    auto norm = glm::cross(uEdge, vEdge);
    auto myPlane = glm::vec4(norm, -glm::dot(norm, base));
    setPlane(myPlane);

    // record bounding sphere info -- ought to calculate center and
    // and radius of circumscribing sphere but it's late and i'm tired.
    // i'll just calculate something easy.  it hardly matters as it's
    // hard to tightly bound a triangle with a sphere.
    auto myCenter = (uEdge + vEdge) / 2.0f;
    const auto myRadius = glm::length2(myCenter);
    myCenter += base;
    setCenter(myCenter);
    setRadius(myRadius);

    // get length of sides
    const float uLength = glm::length(uEdge);
    const float vLength = glm::length(vEdge);
    float area = 0.5f * uLength * vLength;

    // If negative then these values aren't a number of times to repeat
    // the texture along the surface but the width, or a desired scaled
    // width, of the texture itself. Repeat the texture as many times
    // as necessary to fit the surface.
    if (uRepeats < 0.0f)
        uRepeats = - uLength / uRepeats;

    if (vRepeats < 0.0f)
        vRepeats = - vLength / vRepeats;

    // compute how many LODs required to get larger edge down to
    // elements no bigger than 4 units on a side.
    int uElements = int(uLength) / 2;
    int vElements = int(vLength) / 2;
    int uLevels = 1, vLevels = 1;
    while (uElements >>= 1) uLevels++;
    while (vElements >>= 1) vLevels++;
    int numLevels = std::max(uLevels, vLevels);

    // if no lod's required then don't make any except most coarse
    if (!makeLODs)
        numLevels = 1;

    // make level of detail and element area arrays
    nodes = new Geometry*[numLevels];
    float* areas = new float[numLevels];

    // make top level (single polygon)
    int level = 0;
    uElements = 1;
    areas[level] = area;
    nodes[level++] = new Geometry(this, uElements, base, uEdge, vEdge,
                                  plane, uRepeats, vRepeats);
    shadowNode = new Geometry(this, uElements, base, uEdge, vEdge,
                              plane, uRepeats, vRepeats);
    shadowNode->setStyle(0);

    // make remaining levels by doubling elements in each dimension
    while (level < numLevels)
    {
        uElements <<= 1;
        area *= 0.25f;
        areas[level] = area;
        nodes[level++] = new Geometry(this, uElements, base, uEdge, vEdge,
                                      plane, uRepeats, vRepeats);
    }

    // record extents info
    for (int i = 0; i < 3; i++)
    {
        const auto point = getVertex(i);
        extents.expandToPoint(point);
    }

    // record LOD info
    setNumLODs(numLevels, areas);
}


TriWallSceneNode::~TriWallSceneNode()
{
    // free LODs
    const int numLevels = getNumLODs();
    for (int i = 0; i < numLevels; i++)
        delete nodes[i];
    delete[] nodes;
    delete shadowNode;
}


bool            TriWallSceneNode::cull(const ViewFrustum& frustum) const
{
    // cull if eye is behind (or on) plane
    const auto eye = frustum.getEye();
    if (glm::dot(eye, glm::vec3(plane)) + plane[3] <= 0.0f)
        return true;

    // if the Visibility culler tells us that we're
    // fully visible, then skip the rest of these tests
    if (octreeState == OctreeVisible)
        return false;

    const Frustum* f = (const Frustum *) &frustum;
    if (testAxisBoxInFrustum(extents, f) == Outside)
        return true;

    // probably visible
    return false;
}


void            TriWallSceneNode::addRenderNodes(
    SceneRenderer& renderer)
{
    const int lod = pickLevelOfDetail(renderer);
    nodes[lod]->setStyle(getStyle());
    renderer.addRenderNode(nodes[lod], getWallGState());
}


void            TriWallSceneNode::addShadowNodes(
    SceneRenderer& renderer)
{
    renderer.addShadowNode(shadowNode);
}


bool            TriWallSceneNode::inAxisBox(const Extents& exts) const
{
    if (!extents.touches(exts))
        return false;

    return testPolygonInAxisBox (nodes[0]->vertex, plane, exts);
}


int         TriWallSceneNode::getVertexCount () const
{
    return 3;
}


const glm::vec3 TriWallSceneNode::getVertex (int vertex) const
{
    return nodes[0]->getVertex(vertex);
}

void            TriWallSceneNode::getRenderNodes(std::vector<RenderSet>& rnodes)
{
    RenderSet rs = { nodes[0], getWallGState() };
    rnodes.push_back(rs);
    return;
}


void            TriWallSceneNode::renderRadar()
{
    if (plane[2] > 0.0f)
        nodes[0]->renderRadar();
    return;
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
