/* bzflag
 * Copyright (c) 1993-2020 Tim Riker
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

// common headers
#include "bzfgl.h"
#include "OpenGLGState.h"
#include "MeshDrawInfo.h"
#include "bzfio.h" // for DEBUGx()


MeshDrawMgr::MeshDrawMgr(const MeshDrawInfo* drawInfo_)
    : drawInfo(drawInfo_)
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

    auto lodCount = drawInfo->getLodCount();
    lodLists.resize(lodCount);

    // This pointer is a convience way to iterate over the DrawLod objects known to the MeshDrawInfo. A first-class
    // iterator would be better
    auto curDrawLod = drawInfo->getDrawLods();

    // size each LodList to the corresponding DrawLod
    for (auto &item : lodLists)
        item.assign((curDrawLod++)->count, INVALID_GL_LIST_ID);

    makeLists();
    OpenGLGState::registerContextInitializer(freeContext, initContext, this);
}


MeshDrawMgr::~MeshDrawMgr()
{
    logDebugMessage(4,"MeshDrawMgr: killing\n");

    OpenGLGState::unregisterContextInitializer(freeContext, initContext, this);
    freeLists();

    return;
}


inline void MeshDrawMgr::rawExecuteCommands(int lod, int set)
{
    auto drawLods = drawInfo->getDrawLods();
    const DrawLod& drawLod = drawLods[lod];
    const DrawSet& drawSet = drawLod.sets[set];
    const int cmdCount = drawSet.count;
    for (int i = 0; i < cmdCount; i++)
    {
        const DrawCmd& cmd = drawSet.cmds[i];
        glDrawElements(cmd.drawMode, cmd.count, cmd.indexType, cmd.indices);
    }
    return;
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

    const GLuint list = lodLists[lod][set];
    if (list != INVALID_GL_LIST_ID)
        glCallList(list);
    else
    {
        auto vertices  = reinterpret_cast<GLfloat const*>(drawInfo->getVertices());
        auto normals   = reinterpret_cast<GLfloat const*>(drawInfo->getNormals());
        auto texcoords = reinterpret_cast<GLfloat const*>(drawInfo->getTexcoords());

        glVertexPointer(3, GL_FLOAT, 0, vertices);

        if (useNormals)
            glNormalPointer(GL_FLOAT, 0, normals);
        else
            glDisableClientState(GL_NORMAL_ARRAY);
        if (useTexcoords)
            glTexCoordPointer(2, GL_FLOAT, 0, texcoords);
        else
            glDisableClientState(GL_TEXTURE_COORD_ARRAY);

        rawExecuteCommands(lod, set);

        if (!useNormals)
            glEnableClientState(GL_NORMAL_ARRAY);
        if (!useTexcoords)
            glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    }

    if (animInfo != nullptr)
        glPopMatrix();

    return;
}


void MeshDrawMgr::executeSetGeometry(int lod, int set)
{
    // FIXME
    const AnimationInfo* animInfo = drawInfo->getAnimationInfo();
    if (animInfo != NULL)
    {
        glPushMatrix();
        glRotatef(animInfo->angle, 0.0f, 0.0f, 1.0f);
    }

    const GLuint list = lodLists[lod][set];
    if (list != INVALID_GL_LIST_ID)
        glCallList(list);
    else
    {
        auto vertices = reinterpret_cast<GLfloat const *>(drawInfo->getVertices());

        glVertexPointer(3, GL_FLOAT, 0, vertices);
        rawExecuteCommands(lod, set);
    }

    if (animInfo != NULL)
        glPopMatrix();

    return;
}


void MeshDrawMgr::makeLists()
{
    GLenum error;
    int errCount = 0;
    // reset the error state
    while (true)
    {
        error = glGetError();
        if (error == GL_NO_ERROR)
            break;
        errCount++; // avoid a possible spin-lock?
        if (errCount > 666)
        {
            logDebugMessage(1,"MeshDrawMgr::makeLists() glError: %i\n", error);
            return; // don't make the lists, something is borked
        }
    };

    auto vertices  = reinterpret_cast<GLfloat const*>(drawInfo->getVertices());
    auto normals   = reinterpret_cast<GLfloat const*>(drawInfo->getNormals());
    auto texcoords = reinterpret_cast<GLfloat const*>(drawInfo->getTexcoords());

    glVertexPointer(3, GL_FLOAT, 0, vertices);
    glEnableClientState(GL_VERTEX_ARRAY);
    glNormalPointer(GL_FLOAT, 0, normals);
    glEnableClientState(GL_NORMAL_ARRAY);
    glTexCoordPointer(2, GL_FLOAT, 0, texcoords);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    auto lod = 0;
    auto curDrawLod = drawInfo->getDrawLods();

    for (auto &item : lodLists)
    {
        const DrawLod& drawLod = *(curDrawLod++);
        for (auto set = 0; set < drawLod.count; set++)
        {
            const DrawSet& drawSet = drawLod.sets[set];
            if (!drawSet.wantList)
                continue;

            item[set] = glGenLists(1);

            glNewList(item[set], GL_COMPILE);
            {
                rawExecuteCommands(lod, set);
            }
            glEndList();

            error = glGetError();
            if (error != GL_NO_ERROR)
            {
                logDebugMessage(1,"MeshDrawMgr::makeLists() %i/%i glError: %i\n",
                                lod, set, error);
                item[set] = INVALID_GL_LIST_ID;
            }
            else
                logDebugMessage(3,"MeshDrawMgr::makeLists() %i/%i created\n", lod, set);
        }
        lod++;
    }

    return;
}


void MeshDrawMgr::freeLists()
{
    for (auto &item : lodLists)
        for (auto &itemSet : item)
            if (itemSet != INVALID_GL_LIST_ID)
            {
                glDeleteLists(itemSet, 1);
                itemSet = INVALID_GL_LIST_ID;
            }

    return;
}


void MeshDrawMgr::initContext(void* data)
{
    ((MeshDrawMgr*)data)->makeLists();
    return;
}


void MeshDrawMgr::freeContext(void* data)
{
    ((MeshDrawMgr*)data)->freeLists();
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
