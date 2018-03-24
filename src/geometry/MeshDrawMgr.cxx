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

// implementation header
#include "MeshDrawMgr.h"

// System headers
#include <algorithm>
#include <array>

// common headers
#include "bzfgl.h"
#include "OpenGLGState.h"
#include "MeshDrawInfo.h"
#include "bzfio.h" // for DEBUGx()
#include "VBO_Vertex.h"


MeshDrawMgr::MeshDrawMgr(const MeshDrawInfo* drawInfo_)
    : VBOclient()
    , drawInfo(drawInfo_)
{
    if ((drawInfo == nullptr) || !drawInfo->isValid())
    {
        printf("MeshDrawMgr: invalid drawInfo\n");
        fflush(stdout);
        return;
    }
    else
    {
        logDebugMessage(4,"MeshDrawMgr: initializing\n");
        fflush(stdout);
    }

    auto drawLods = drawInfo->getDrawLods();
    auto vertexCount = drawInfo->getCornerCount();

    auto lodCount = drawInfo->getLodCount();
    lodLists.resize(lodCount);

    // This pointer is a convience way to iterate over the DrawLod objects known to the MeshDrawInfo. A first-class
    // iterator would be better
    auto curDrawLod = drawInfo->getDrawLods();

    // size each LodList to the corresponding DrawLod
    for (auto &item : lodLists)
        item.assign( (curDrawLod++)->count, -1);

    vboArrayIndex = vboVTN.vboAlloc(vertexCount);
    for (int lod = 0; lod < lodCount; lod++)
    {
        const DrawLod& drawLod = drawLods[lod];
        for (int set = 0; set < drawLod.count; set++)
        {
            const DrawSet& drawSet = drawLod.sets[set];
            const int cmdCount = drawSet.count;
            int     fillP  = 0;
            for (int i = 0; i < cmdCount; i++)
            {
                const DrawCmd& cmd = drawSet.cmds[i];
                fillP += cmd.count;
                if (fillP > 8192)
                {
                    std::cout << "MeshDrawMgr cmdSet " << fillP << " is too high" << std::endl;
                    abort();
                }
            }
            if (fillP)
                lodLists[lod][set] = vboElement.vboAlloc(fillP);
        }
    }
    vboManager.registerClient(this);
}


MeshDrawMgr::~MeshDrawMgr()
{
    logDebugMessage(4,"MeshDrawMgr: killing\n");

    vboVTN.vboFree(vboArrayIndex);

    for (auto &item : lodLists)
    {
        std::for_each(item.begin(), item.end(),
                      [](auto set)
        {
            vboElement.vboFree(set);
        });
    }
    vboManager.unregisterClient(this);
}


void MeshDrawMgr::executeSet(int lod, int set, bool useNormals, bool useTexcoords)
{
    // FIXME (what is broken?)
    const AnimationInfo* animInfo = drawInfo->getAnimationInfo();
    if (animInfo != nullptr)
    {
        glPushMatrix();
        glRotatef(animInfo->angle, 0.0f, 0.0f, 1.0f);
    }

    vboVTN.enableArrays(useTexcoords, useNormals, false);

    auto drawLods = drawInfo->getDrawLods();
    const DrawLod& drawLod = drawLods[lod];
    const DrawSet& drawSet = drawLod.sets[set];
    const int cmdCount = drawSet.count;
    auto fillP = lodLists[lod][set]; // no error checking (for performance?)
    for (int i = 0; i < cmdCount; ++i)
    {
        const DrawCmd& cmd = drawSet.cmds[i];
        glDrawElements(
            cmd.drawMode,
            cmd.count,
            GL_UNSIGNED_INT,
            (const void *)(fillP * sizeof(GLuint)));
        fillP += cmd.count;
    }

    if (animInfo != nullptr)
        glPopMatrix();

    return;
}


void MeshDrawMgr::initVBO()
{
    // This should be encapsulated in a VBO object
    {
        // This is dangerous. The drawinfo accessors return float const(*)[3], and we are taking a
        // sledgehammer to convert to GLfloat const* (float const*).
        // The types should be congruent throughout.  Or
        // better yet, use vectors and arrays
        auto vertices = reinterpret_cast<GLfloat const*>(drawInfo->getVertices());
        auto normals = reinterpret_cast<GLfloat const*>(drawInfo->getNormals());
        auto texcoords = reinterpret_cast<GLfloat const*>(drawInfo->getTexcoords());

        auto vertexCount = drawInfo->getCornerCount();

        vboVTN.vertexData(vboArrayIndex, vertexCount, vertices);
        vboVTN.textureData(vboArrayIndex, vertexCount, texcoords);
        vboVTN.normalData(vboArrayIndex, vertexCount, normals);
    }

    auto drawLods = drawInfo->getDrawLods();

    // TODO: retrofit MeshDrawInfo to use containers
    for (unsigned int lod = 0; lod < lodLists.size(); ++lod)
    {
        const DrawLod& drawLod = drawLods[lod];
        for (int set = 0; set < drawLod.count; ++set)
        {
            const DrawSet& drawSet = drawLod.sets[set];
            const int cmdCount = drawSet.count;

            // Working array of indices ... this could be refactored as a
            // vector for more robustness, unless 8192 is a hard limit for
            // the GL methods
            std::array<GLuint,8192>  indices;

            // Iterator for traversing the above array.
            auto indexP = indices.begin();

            // Number of used elements of the array
            unsigned int fillP  = 0;

            // for each DrawCmd, append its indices to our array
            for (int i = 0; i < cmdCount; i++)
            {
                const DrawCmd& cmd = drawSet.cmds[i];
                fillP += cmd.count;
                if (fillP > indices.size())
                {
                    std::cout << "MeshDrawMgr cmdSet " << fillP << " is too high" << std::endl;
                    abort();
                }
                if (cmd.indexType == GL_UNSIGNED_SHORT)
                {
                    auto src = static_cast<GLushort*>(cmd.indices);
                    indexP = std::transform(src, src + cmd.count, indexP,
                                            [&](auto p)
                    {
                        return p + vboArrayIndex;
                    });
                }
                else if (cmd.indexType == GL_UNSIGNED_INT)
                {
                    auto src = static_cast<GLuint*>(cmd.indices);
                    indexP = std::transform(src, src + cmd.count, indexP,
                                            [&](auto p)
                    {
                        return p + vboArrayIndex;
                    });
                }
            }

            if (fillP)
            {
                int elIndex = lodLists[lod][set];
                vboElement.elementData(elIndex, fillP, indices.data());
            }
        }
    }

    return;
}


/******************************************************************************/

// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
