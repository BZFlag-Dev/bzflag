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

// interface header
#include "MeshFragSceneNode.h"

// system headers
#include <assert.h>
#include <cmath>
#include <cstring>

// common implementation headers
#include "Intersect.h"
#include "MeshFace.h"
#include "MeshSceneNodeGenerator.h"
#include "BzMaterial.h"
#include "StateDatabase.h"
#include "BZDBCache.h"
#include "SceneRenderer.h"
#include "VBO_Vertex.h"


// FIXME - no tesselation is done on for shot lighting


//
// MeshFragSceneNode::Geometry
//

// NOTE: this should be based on visual pixel area
static int minLightDisabling = 100;

MeshFragSceneNode::Geometry::Geometry(MeshFragSceneNode &node)
    : style(0)
    , vboIndex(-1)
    , sceneNode(node)
{
}


MeshFragSceneNode::Geometry::~Geometry()
{
    vboVTN.vboFree(vboIndex);
    vboManager.unregisterClient(this);
}


void MeshFragSceneNode::Geometry::init()
{
    const int count = sceneNode.arrayCount * 3;
    vboIndex = vboVTN.vboAlloc(count);
    vboManager.registerClient(this);
}


void MeshFragSceneNode::Geometry::initVBO()
{
    const int count = sceneNode.arrayCount * 3;
    vboVTN.vertexData(vboIndex, count, sceneNode.vertices);
    vboVTN.textureData(vboIndex, count, sceneNode.texcoords);
    vboVTN.normalData(vboIndex, count, sceneNode.normals);

    return;
}


void MeshFragSceneNode::Geometry::renderVBO()
{
    const int triangles = sceneNode.arrayCount;
    glDrawArrays(GL_TRIANGLES, vboIndex, triangles * 3);
    addTriangleCount(triangles);

    return;
}


void MeshFragSceneNode::Geometry::render()
{
    const int triangles = sceneNode.arrayCount;
    const bool switchLights = (triangles >= minLightDisabling)
                              && BZDBCache::lighting;
    if (switchLights)
        RENDERER.disableLights(sceneNode.extents.mins, sceneNode.extents.maxs);

    // set the color
    sceneNode.setColor();

    vboVTN.enableArrays(BZDBCache::texture, BZDBCache::lighting, false);

    renderVBO();

    if (switchLights)
        RENDERER.reenableLights();

    return;
}


void MeshFragSceneNode::Geometry::renderShadow()
{
    vboVTN.enableVertexOnly();
    renderVBO();
    return;
}


const glm::vec3 MeshFragSceneNode::Geometry::getPosition() const
{
    return sceneNode.getCenter();
}

void MeshFragSceneNode::Geometry::setStyle(int style_)
{
    style = style_;
}


//
// MeshFragSceneNode
//

MeshFragSceneNode::MeshFragSceneNode(int faceCount_, const MeshFace** faces_)
    : WallSceneNode()
    , renderNode(*this)
    , faceCount(faceCount_)
    , faces(faces_)
    , noRadar(false)
    , noShadow(false)
    , arrayCount(0)
    , vertices(nullptr)
    , normals(nullptr)
    , texcoords(nullptr)
{
    int i, j, k;

    // This is semi-pointless, as assert is typically compiled out in production code.
    // If protection is needed, a more/ robust solution should be provided
    assert ((faceCount > 0) && (faces != NULL));

    // disable the plane
    static const auto fakePlane = glm::vec4(0.0f, 0.0f, 1.0f, 0.0f);
    setPlane(fakePlane);

    const BzMaterial* bzmat = faces[0]->getMaterial();

    // disable radar and shadows if required
    noRadar = bzmat->getNoRadar();
    noShadow = bzmat->getNoShadow();

    // set lod info
    setNumLODs(0, NULL /* unused because LOD = 0 */);

    // record extents info
    for (i = 0; i < faceCount; i++)
    {
        const Extents& fExts = faces[i]->getExtents();
        extents.expandToBox(fExts);
    }

    // setup sphere
    float diffs[3];
    diffs[0] = extents.maxs[0] - extents.mins[0];
    diffs[1] = extents.maxs[1] - extents.mins[1];
    diffs[2] = extents.maxs[2] - extents.mins[2];
    glm::vec4 mySphere;
    mySphere[0] = 0.5f * (extents.maxs[0] + extents.mins[0]);
    mySphere[1] = 0.5f * (extents.maxs[1] + extents.mins[1]);
    mySphere[2] = 0.5f * (extents.maxs[2] + extents.mins[2]);
    mySphere[3] = 0.25f *
                  ((diffs[0] * diffs[0]) + (diffs[1] * diffs[1]) + (diffs[2] * diffs[2]));
    setSphere(mySphere);

    // count the number of actual vertices
    for (i = 0; i < faceCount; i++)
    {
        const MeshFace* face = faces[i];
        arrayCount = arrayCount + (face->getVertexCount() - 2);
    }

    const int vertexCount = (arrayCount * 3);
    normals = new GLfloat[vertexCount * 3];
    vertices = new GLfloat[vertexCount * 3];
    texcoords = new GLfloat[vertexCount * 2];

    int arrayIndex = 0;
    for (i = 0; i < faceCount; i++)
    {
        const MeshFace* face = faces[i];

        // pre-generate the texcoords if required
        std::vector<glm::vec2> t(face->getVertexCount());
        if (!face->useTexcoords())
        {
            std::vector<glm::vec3> v(face->getVertexCount());
            for (j = 0; j < face->getVertexCount(); j++)
                v[j] = glm::make_vec3(face->getVertex(j));
            MeshSceneNodeGenerator::makeTexcoords(face->getPlane(), v, t);
        }

        // number of triangles
        const int tcount = (face->getVertexCount() - 2);

        for (j = 0; j < tcount; j++)
        {
            for (k = 0; k < 3 ; k++)
            {
                const int aIndex = (arrayIndex + (j * 3) + k);
                int vIndex; // basically GL_TRIANGLE_FAN done the hard way
                if (k == 0)
                    vIndex = 0;
                else
                    vIndex = (j + k) % face->getVertexCount();

                // get the vertices
                memcpy(&vertices[aIndex * 3], face->getVertex(vIndex), sizeof(float[3]));

                // get the normals
                if (face->useNormals())
                    memcpy(&normals[aIndex * 3], face->getNormal(vIndex), sizeof(float[3]));
                else
                    memcpy(&normals[aIndex * 3], glm::value_ptr(face->getPlane()), sizeof(float[3]));

                // get the texcoords
                if (face->useTexcoords())
                    memcpy(&texcoords[aIndex * 2], face->getTexcoord(vIndex), sizeof(float[2]));
                else
                    memcpy(&texcoords[aIndex * 2], glm::value_ptr(t[vIndex]), sizeof(float[2]));
            }
        }

        arrayIndex = arrayIndex + (3 * tcount);
    }

    assert(arrayIndex == (arrayCount * 3));

    renderNode.init();
}


MeshFragSceneNode::~MeshFragSceneNode()
{
    delete[] faces;
    delete[] vertices;
    delete[] normals;
    delete[] texcoords;
}


const glm::vec4 MeshFragSceneNode::getPlane() const
{
    return glm::vec4(0.0f);
}

bool MeshFragSceneNode::cull(const ViewFrustum& frustum) const
{
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


bool MeshFragSceneNode::inAxisBox (const Extents& exts) const
{
    // NOTE: it should be OK to use the faces while building

    float pos[3];
    pos[0] = 0.5f * (exts.maxs[0] + exts.mins[0]);
    pos[1] = 0.5f * (exts.maxs[1] + exts.mins[1]);
    pos[2] = exts.mins[2];
    float size[3];
    size[0] = 0.5f * (exts.maxs[0] - exts.mins[0]);
    size[1] = 0.5f * (exts.maxs[1] - exts.mins[1]);
    size[2] = (exts.maxs[2] - exts.mins[2]);

    for (int i = 0; i < faceCount; i++)
    {
        if (faces[i]->inBox(pos, 0.0f, size[0], size[1], size[2]))
            return true;
    }

    return false;
}


void MeshFragSceneNode::addRenderNodes(SceneRenderer& renderer)
{
    renderNode.setStyle(getStyle());
    const glm::vec4 *dyncol = getDynamicColor();
    if ((dyncol == NULL) || (dyncol->a != 0.0f))
        renderer.addRenderNode(&renderNode, getWallGState());
}


void MeshFragSceneNode::addShadowNodes(SceneRenderer& renderer)
{
    if (!noShadow)
    {
        const glm::vec4 *dyncol = getDynamicColor();
        if ((dyncol == NULL) || (dyncol->a != 0.0f))
            renderer.addShadowNode(&renderNode);
    }
}


void MeshFragSceneNode::renderRadar()
{
    if (!noRadar)
        renderNode.renderRadar();
    return;
}


void MeshFragSceneNode::getRenderNodes(std::vector<RenderSet>& rnodes)
{
    RenderSet rs = { &renderNode, getWallGState() };
    rnodes.push_back(rs);
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
