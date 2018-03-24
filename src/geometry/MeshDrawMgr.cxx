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

#include "common.h"

// implementation header
#include "MeshDrawMgr.h"

// System headers
#include <string.h>

// common headers
#include "bzfgl.h"
#include "OpenGLGState.h"
#include "MeshDrawInfo.h"
#include "bzfio.h" // for DEBUGx()


MeshDrawMgr::MeshDrawMgr(const MeshDrawInfo* _drawInfo) : vboArrayIndex(-1)
{
    drawInfo = _drawInfo;
    if ((drawInfo == NULL) || !drawInfo->isValid())
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

    drawLods = drawInfo->getDrawLods();
    vertices = (const GLfloat*)drawInfo->getVertices();
    normals = (const GLfloat*)drawInfo->getNormals();
    texcoords = (const GLfloat*)drawInfo->getTexcoords();
    vertexCount = drawInfo->getCornerCount();

    lodCount = drawInfo->getLodCount();
    lodLists = new LodList[lodCount];

    for (int i = 0; i < lodCount; i++)
    {
        LodList& lodList = lodLists[i];
        lodList.count = drawLods[i].count;
        lodList.setLists = new GLuint[lodList.count];
    }
    vboManager.registerClient(this);
    return;
}


MeshDrawMgr::~MeshDrawMgr()
{
    logDebugMessage(4,"MeshDrawMgr: killing\n");

    vboVTN.vboFree(vboArrayIndex);
    vboManager.unregisterClient(this);

    for (int i = 0; i < lodCount; i++)
        delete[] lodLists[i].setLists;
    delete[] lodLists;

    return;
}

void MeshDrawMgr::initVBO()
{
    vboArrayIndex = vboVTN.vboAlloc(vertexCount);
    vboVTN.vertexData(vboArrayIndex, vertexCount, vertices);
    vboVTN.textureData(vboArrayIndex, vertexCount, texcoords);
    vboVTN.normalData(vboArrayIndex, vertexCount, normals);

    for (int lod = 0; lod < lodCount; lod++)
    {
        const DrawLod& drawLod = drawLods[lod];
        for (int set = 0; set < drawLod.count; set++)
        {
            const DrawSet& drawSet = drawLod.sets[set];
            const int cmdCount = drawSet.count;
            GLuint  indices[4096];
            GLuint *indexP = indices;
            int     fillP  = 0;
            for (int i = 0; i < cmdCount; i++)
            {
                const DrawCmd& cmd = drawSet.cmds[i];
                fillP += cmd.count;
                if (fillP > 4096)
                {
                    std::cout << "MeshDrawMgr cmdSet " << fillP << " is too high" << std::endl;
                    abort();
                }
                if (cmd.indexType == GL_UNSIGNED_SHORT)
                {
                    GLushort *shortP = (GLushort *)cmd.indices;
                    for (int j = 0; j < cmd.count; j++)
                        *indexP++ = (GLuint)*shortP++;
                }
                else if (cmd.indexType == GL_UNSIGNED_INT)
                {
                    memcpy(indices, cmd.indices, cmd.count * sizeof(GLuint));
                    indexP += cmd.count;
                }
            }
            for (int j = 0; j < fillP; j++)
                indices[j] += vboArrayIndex;
            if (fillP)
            {
                int elIndex = vboVTN.reserveIndex(fillP);
                lodLists[lod].setLists[set] = elIndex;
                vboVTN.elementData(elIndex, fillP, indices);
            }
        }
    }
}

inline void MeshDrawMgr::rawExecuteCommands(int lod, int set)
{
    const DrawLod& drawLod = drawLods[lod];
    const DrawSet& drawSet = drawLod.sets[set];
    const int cmdCount = drawSet.count;
    int   fillP = lodLists[lod].setLists[set];
    for (int i = 0; i < cmdCount; i++)
    {
        const DrawCmd& cmd = drawSet.cmds[i];
        vboVTN.drawElements(cmd.drawMode, cmd.count, fillP);
        fillP += cmd.count;
    }
    return;
}


void MeshDrawMgr::executeSet(int lod, int set, bool _normals, bool _texcoords)
{
    // FIXME
    const AnimationInfo* animInfo = drawInfo->getAnimationInfo();
    if (animInfo != NULL)
    {
        glPushMatrix();
        glRotatef(animInfo->angle, 0.0f, 0.0f, 1.0f);
    }

    vboVTN.enableArrays(_texcoords, _normals, false);

    rawExecuteCommands(lod, set);

    if (animInfo != NULL)
        glPopMatrix();

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
