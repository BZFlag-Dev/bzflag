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

// bzflag common header
#include "common.h"

// interface header
#include "TriWallSceneNode.h"

// system headers
#include <math.h>
#include <stdlib.h>
#include <glm/gtc/type_ptr.hpp>

// common implementation headers
#include "Intersect.h"
#include "StateDatabase.h"

// FIXME (SceneRenderer.cxx is in src/bzflag)
#include "SceneRenderer.h"

//
// TriWallSceneNode::Geometry
//

TriWallSceneNode::Geometry::Geometry(TriWallSceneNode* _wall, int eCount,
                                     const GLfloat base[3], const GLfloat uEdge[3], const GLfloat vEdge[3],
                                     const glm::vec3 _normal, float uRepeats, float vRepeats) :
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
            vertex[n][0] = base[0] + s * uEdge[0] + t * vEdge[0];
            vertex[n][1] = base[1] + s * uEdge[1] + t * vEdge[1];
            vertex[n][2] = base[2] + s * uEdge[2] + t * vEdge[2];
            uv[n][0] = 0.0f + s * uRepeats;
            uv[n][1] = 0.0f + t * vRepeats;
        }
    }

    if (BZDB.isTrue("remapTexCoords"))
    {
        const float uLen = sqrtf((uEdge[0] * uEdge[0]) +
                                 (uEdge[1] * uEdge[1]) +
                                 (uEdge[2] * uEdge[2]));
        const float vLen = sqrtf((vEdge[0] * vEdge[0]) +
                                 (vEdge[1] * vEdge[1]) +
                                 (vEdge[2] * vEdge[2]));
        const float uScale = 10.0f / floorf(10.0f * uLen / uRepeats);
        const float vScale = 10.0f / floorf(10.0f * vLen / vRepeats);
        if (fabsf(normal.z) > 0.999f)
        {
            // horizontal surface
            for (int i = 0; i < vertex.getSize(); i++)
            {
                uv[i][0] = uScale * vertex[i][0];
                uv[i][1] = vScale * vertex[i][1];
            }
        }
        else
        {
            // vertical surface
            const glm::vec2 normal2d(glm::normalize(glm::vec2(normal)));
            const float vs = 1.0f / sqrtf(1.0f - normal.z * normal.z);
            for (int i = 0; i < vertex.getSize(); i++)
            {
                const float* v = vertex[i];
                const float uGeoScale = (normal2d.x * v[1]) - (normal2d.y * v[0]);
                const float vGeoScale = v[2] * vs;
                uv[i][0] = uScale * uGeoScale;
                uv[i][1] = vScale * vGeoScale;
            }
        }
    }

    triangles = (eCount * eCount);
    vboIndex  = -1;
    vboIndexS = -1;
    vboManager.registerClient(this);
}


TriWallSceneNode::Geometry::~Geometry()
{
    vboVT.vboFree(vboIndex);
    vboV.vboFree(vboIndexS);
    vboManager.unregisterClient(this);
}

void TriWallSceneNode::Geometry::initVBO()
{
    std::vector<glm::vec3> vert;
    std::vector<glm::vec2> text;
    int k = 0;
    for (int t = 0; t < de; t++)
    {
        int e = de - t;
        for (int s = 0; s < e; s++)
        {
            text.push_back(glm::make_vec2(uv[k+e+1]));
            text.push_back(glm::make_vec2(uv[k]));
            vert.push_back(glm::make_vec3(vertex[k+e+1]));
            vert.push_back(glm::make_vec3(vertex[k]));
            k++;
        }
        text.push_back(glm::make_vec2(uv[k]));
        vert.push_back(glm::make_vec3(vertex[k]));
        k++;
    }
    vboIndex  = vboVT.vboAlloc(vert.size());
    vboVT.textureData(vboIndex, text);
    vboVT.vertexData(vboIndex, vert);

    vboIndexS = vboV.vboAlloc(3);
    glm::vec3 ver[3];
    ver[0] = glm::make_vec3(vertex[(de + 1) * (de + 2) / 2 - 1]);
    ver[1] = glm::make_vec3(vertex[0]);
    ver[2] = glm::make_vec3(vertex[de]);
    vboV.vertexData(vboIndexS, 3, ver);
}

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
    vboV.enableArrays();
    glDrawArrays(GL_TRIANGLE_STRIP, vboIndexS, 3);
    addTriangleCount(1);
}


void            TriWallSceneNode::Geometry::drawV() const
{
    int start = 0;
    vboVT.enableArrays(false, false, false);
    for (int t = 0; t < de; t++)
    {
        int count = 2 * (de - t) + 1;
        glDrawArrays(GL_TRIANGLE_STRIP, vboIndex + start, count);
        start += count;
    }
}


void            TriWallSceneNode::Geometry::drawVT() const
{
    int start = 0;
    vboVT.enableArrays();
    for (int t = 0; t < de; t++)
    {
        int count = 2 * (de - t) + 1;
        glDrawArrays(GL_TRIANGLE_STRIP, vboIndex + start, count);
        start += count;
    }
}


const GLfloat*      TriWallSceneNode::Geometry::getVertex(int i) const
{
    return vertex[i];
}


//
// TriWallSceneNode
//

TriWallSceneNode::TriWallSceneNode(const GLfloat base[3],
                                   const GLfloat uEdge[3],
                                   const GLfloat vEdge[3],
                                   float uRepeats,
                                   float vRepeats,
                                   bool makeLODs)
{
    // record plane info
    GLfloat myPlane[4], mySphere[4];
    myPlane[0] = uEdge[1] * vEdge[2] - uEdge[2] * vEdge[1];
    myPlane[1] = uEdge[2] * vEdge[0] - uEdge[0] * vEdge[2];
    myPlane[2] = uEdge[0] * vEdge[1] - uEdge[1] * vEdge[0];
    myPlane[3] = -(myPlane[0] * base[0] + myPlane[1] * base[1]
                   + myPlane[2] * base[2]);
    setPlane(myPlane);

    // record bounding sphere info -- ought to calculate center and
    // and radius of circumscribing sphere but it's late and i'm tired.
    // i'll just calculate something easy.  it hardly matters as it's
    // hard to tightly bound a triangle with a sphere.
    mySphere[0] = 0.5f * (uEdge[0] + vEdge[0]);
    mySphere[1] = 0.5f * (uEdge[1] + vEdge[1]);
    mySphere[2] = 0.5f * (uEdge[2] + vEdge[2]);
    mySphere[3] = mySphere[0]*mySphere[0] + mySphere[1]*mySphere[1]
                  + mySphere[2]*mySphere[2];
    mySphere[0] += base[0];
    mySphere[1] += base[1];
    mySphere[2] += base[2];
    setSphere(mySphere);

    const glm::vec3 normal(glm::make_vec3(getPlane()));
    // get length of sides
    const float uLength = sqrtf(uEdge[0] * uEdge[0] +
                                uEdge[1] * uEdge[1] + uEdge[2] * uEdge[2]);
    const float vLength = sqrtf(vEdge[0] * vEdge[0] +
                                vEdge[1] * vEdge[1] + vEdge[2] * vEdge[2]);
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
    int numLevels = (uLevels > vLevels ? uLevels : vLevels);

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
                                  normal, uRepeats, vRepeats);
    shadowNode = new Geometry(this, uElements, base, uEdge, vEdge,
                              normal, uRepeats, vRepeats);
    shadowNode->setStyle(0);

    // make remaining levels by doubling elements in each dimension
    while (level < numLevels)
    {
        uElements <<= 1;
        area *= 0.25f;
        areas[level] = area;
        nodes[level++] = new Geometry(this, uElements, base, uEdge, vEdge,
                                      normal, uRepeats, vRepeats);
    }

    // record extents info
    for (int i = 0; i < 3; i++)
    {
        const float* point = getVertex(i);
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
    const GLfloat* eye = frustum.getEye();
    if (((eye[0] * plane[0]) + (eye[1] * plane[1]) + (eye[2] * plane[2]) +
            plane[3]) <= 0.0f)
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


int         TriWallSceneNode::split(const float* _plane,
                                    SceneNode*& front, SceneNode*& back) const
{
    return WallSceneNode::splitWall(_plane, nodes[0]->vertex, nodes[0]->uv,
                                    front, back);
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

    // NOTE: inefficient
    float vertices[3][3];
    memcpy (vertices[0], nodes[0]->getVertex(0), sizeof(float[3]));
    memcpy (vertices[1], nodes[0]->getVertex(1), sizeof(float[3]));
    memcpy (vertices[2], nodes[0]->getVertex(2), sizeof(float[3]));

    return testPolygonInAxisBox (3, vertices, getPlane(), exts);
}


int         TriWallSceneNode::getVertexCount () const
{
    return 3;
}


const GLfloat*      TriWallSceneNode::getVertex (int vertex) const
{
    return nodes[0]->getVertex(vertex);
}

void            TriWallSceneNode::getRenderNodes(std::vector<RenderSet>& rnodes)
{
    RenderSet rs = { nodes[0], getWallGState() };
    rnodes.push_back(rs);
    return;
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
