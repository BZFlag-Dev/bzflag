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
#include "VBO_Handler.h"

#include <iostream>
#ifdef _WANT_BACKTRACE
#include <execinfo.h>
#endif

// common implementation header
#include "OpenGLGState.h"

bool VBO_Handler::textureEnabled = false;
bool VBO_Handler::normalEnabled  = false;
bool VBO_Handler::colorEnabled   = false;
bool VBO_Handler::arrayEnabled   = true;
GLuint VBO_Handler::actVerts     = 0;
GLuint VBO_Handler::actNorms     = 0;
GLuint VBO_Handler::actTxcds     = 0;
GLuint VBO_Handler::actColrs     = 0;

bool VBO_Manager::glContextReady = false;
VBO_Manager::VBO_Manager ()
{
    OpenGLGState::registerContextInitializer(freeContext, initContext, this);
}

VBO_Manager::~VBO_Manager ()
{
    OpenGLGState::unregisterContextInitializer(freeContext, initContext, this);
}

void VBO_Manager::registerHandler(VBO_Handler *handler)
{
    handlerList.push_back(handler);
}

void VBO_Manager::unregisterHandler(VBO_Handler *handler)
{
    std::list <VBO_Handler*> :: iterator it;
    for (it = handlerList.begin(); it != handlerList.end(); it++)
        if (*it == handler)
        {
            handlerList.erase(it);
            break;
        }
}

void VBO_Manager::registerClient(VBOclient *client)
{
    clientList.push_back(client);
    if (glContextReady)
        client->initVBO();
}

void VBO_Manager::unregisterClient(VBOclient *client)
{
    std::list <VBOclient*> :: iterator it;
    for (it = clientList.begin(); it != clientList.end(); it++)
        if (*it == client)
        {
            clientList.erase(it);
            break;
        }
}

void VBO_Manager::initAll()
{
    std::list <VBO_Handler*>::const_iterator itH;
    for (itH = handlerList.begin(); itH != handlerList.end(); itH++)
        (*itH)->init();
    std::list <VBOclient*>::const_iterator itC;
    for (itC = clientList.begin(); itC != clientList.end(); itC++)
        (*itC)->initVBO();
}

void VBO_Manager::destroyAll()
{
    std::list <VBO_Handler*>::const_iterator it;
    for (it = handlerList.begin(); it != handlerList.end(); it++)
        (*it)->destroy();
}

void VBO_Manager::initContext(void* data)
{
    glContextReady = true;
    ((VBO_Manager*)data)->initAll();
}

void VBO_Manager::freeContext(void* data)
{
    glContextReady = false;
    ((VBO_Manager*)data)->destroyAll();
}

void VBO_Manager::reset ()
{
    destroyAll();
    initAll();
}

VBO_Handler::VBO_Handler(
    bool _handleTexture,
    bool _handleNormal,
    bool _handleColor,
    int _vertexSize,
    int _indexSize) :
    handleTexture(_handleTexture), handleNormal(_handleNormal),
    handleColor(_handleColor), vertexSize(_vertexSize), indexSize(_indexSize),
    arrayFillPoint(0), indexFillPoint(0)
{
    vboManager.registerHandler(this);
    freeVBOList.push_back({0, vertexSize});
}

VBO_Handler::~VBO_Handler()
{
    vboManager.unregisterHandler(this);
}

void VBO_Handler::init()
{
    glGenBuffers(1, &verts);
    glBindBuffer(GL_ARRAY_BUFFER, verts);
    glBufferData(
        GL_ARRAY_BUFFER,
        vertexSize * sizeof(GLfloat[3]),
        NULL,
        GL_DYNAMIC_DRAW);
    if (handleTexture)
    {
        glGenBuffers(1, &txcds);
        glBindBuffer(GL_ARRAY_BUFFER, txcds);
        glBufferData(
            GL_ARRAY_BUFFER,
            vertexSize * sizeof(GLfloat[2]),
            NULL,
            GL_DYNAMIC_DRAW);
    }
    if (handleNormal)
    {
        glGenBuffers(1, &norms);
        glBindBuffer(GL_ARRAY_BUFFER, norms);
        glBufferData(
            GL_ARRAY_BUFFER,
            vertexSize * sizeof(GLfloat[3]),
            NULL,
            GL_DYNAMIC_DRAW);
    }
    if (handleColor)
    {
        glGenBuffers(1, &colrs);
        glBindBuffer(GL_ARRAY_BUFFER, colrs);
        glBufferData(
            GL_ARRAY_BUFFER,
            vertexSize * sizeof(GLfloat[4]),
            NULL,
            GL_DYNAMIC_DRAW);
    }
    if (indexSize)
    {
        glGenBuffers(1, &elems);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elems);
        glBufferData(
            GL_ELEMENT_ARRAY_BUFFER,
            indexSize * sizeof(GLuint),
            NULL,
            GL_DYNAMIC_DRAW);
    }
    arrayFillPoint = 0;
    indexFillPoint = 0;
    glEnableClientState(GL_VERTEX_ARRAY);
    enableArrays(false, false, false);
    globalArraysEnabling(true);
    freeVBOList.clear();
    alloVBOList.clear();
    freeVBOList.push_back({0, vertexSize});
}

void VBO_Handler::destroy()
{
    enableArrays(false, false, false);
    globalArraysEnabling(false);
    glDeleteBuffers(1, &verts);
    if (handleTexture)
        glDeleteBuffers(1, &txcds);
    if (handleNormal)
        glDeleteBuffers(1, &norms);
    if (handleColor)
        glDeleteBuffers(1, &colrs);
    if (indexSize)
        glDeleteBuffers(1, &elems);
}

void VBO_Handler::vertexData(
    int index,
    int size,
    const GLfloat vertices[][3])
{
    vertexData(index, size, vertices[0]);
}

void VBO_Handler::vertexData(int index, int size, const GLfloat *vertices)
{
    glBindBuffer(GL_ARRAY_BUFFER, verts);
    glBufferSubData(
        GL_ARRAY_BUFFER,
        index * sizeof(GLfloat[3]),
        size * sizeof(GLfloat[3]),
        vertices);
}

void VBO_Handler::vertexData(int index, int size, const glm::vec3 vertices[])
{
    vertexData(index, size, &vertices[0][0]);
}

void VBO_Handler::vertexData(
    int index,
    const std::vector<glm::vec3> vertices)
{
    vertexData(index, vertices.size(), vertices.data());
}

void VBO_Handler::textureData(
    int index,
    int size,
    const GLfloat textures[][2])
{
    textureData(index, size, textures[0]);
}

void VBO_Handler::textureData(int index, int size, const GLfloat *textures)
{
    if (!handleTexture)
        return;
    glBindBuffer(GL_ARRAY_BUFFER, txcds);
    glBufferSubData(
        GL_ARRAY_BUFFER,
        index * sizeof(GLfloat[2]),
        size * sizeof(GLfloat[2]),
        textures);
}

void VBO_Handler::textureData(int index, int size, const glm::vec2 textures[])
{
    textureData(index, size, &textures[0][0]);
}

void VBO_Handler::textureData(
    int index,
    const std::vector<glm::vec2> textures)
{
    textureData(index, textures.size(), textures.data());
}

void VBO_Handler::normalData(
    int index,
    int size,
    const GLfloat normals[][3])
{
    normalData(index, size, normals[0]);
}

void VBO_Handler::normalData(int index, int size, const GLfloat *normals)
{
    if (!handleNormal)
        return;
    glBindBuffer(GL_ARRAY_BUFFER, norms);
    glBufferSubData(
        GL_ARRAY_BUFFER,
        index * sizeof(GLfloat[3]),
        size * sizeof(GLfloat[3]),
        normals);
}

void VBO_Handler::normalData(int index, int size, const glm::vec3 normals[])
{
    normalData(index, size, &normals[0][0]);
}

void VBO_Handler::normalData(
    int index,
    const std::vector<glm::vec3> normals)
{
    normalData(index, normals.size(), normals.data());
}

void VBO_Handler::colorData(
    int index,
    int size,
    const GLfloat colors[][4])
{
    colorData(index, size, colors[0]);
}

void VBO_Handler::colorData(int index, int size, const GLfloat *colors)
{
    if (!handleColor)
        return;
    glBindBuffer(GL_ARRAY_BUFFER, colrs);
    glBufferSubData(
        GL_ARRAY_BUFFER,
        index * sizeof(GLfloat[4]),
        size * sizeof(GLfloat[4]),
        colors);
}

void VBO_Handler::colorData(int index, int size, const glm::vec4 colors[])
{
    colorData(index, size, &colors[0][0]);
}

void VBO_Handler::colorData(
    int index,
    const std::vector<glm::vec4> colors)
{
    colorData(index, colors.size(), colors.data());
}

void VBO_Handler::elementData(
    int index,
    int size,
    const GLuint element[])
{
    glBufferSubData(
        GL_ELEMENT_ARRAY_BUFFER,
        index * sizeof(GLuint),
        size * sizeof(GLuint),
        element);
}

void VBO_Handler::drawElements(GLenum mode, GLsizei count, int index)
{
    glDrawElements(
        mode,
        count,
        GL_UNSIGNED_INT,
        (const void *)(index * sizeof(GLuint)));
}

void VBO_Handler::globalArraysEnabling(bool enabled)
{
    if (arrayEnabled == enabled)
        return;
    arrayEnabled = enabled;
    if (arrayEnabled)
        return;
    disableTextures();
    disableNormals();
    disableColors();
}

void VBO_Handler::enableVertex()
{
    if (actVerts == verts)
        return;
    glBindBuffer(GL_ARRAY_BUFFER, verts);
    glVertexPointer(3, GL_FLOAT, 0, 0);
}

void VBO_Handler::disableTextures()
{
    if (textureEnabled)
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    textureEnabled = false;
}

void VBO_Handler::enableTextures()
{
    if (!handleTexture)
        return;
    if (!textureEnabled)
    {
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        textureEnabled = true;
    }
    if (actTxcds == txcds)
        return;
    glBindBuffer(GL_ARRAY_BUFFER, txcds);
    glTexCoordPointer(2, GL_FLOAT, 0, 0);
}

void VBO_Handler::disableNormals()
{
    if (normalEnabled)
        glDisableClientState(GL_NORMAL_ARRAY);
    normalEnabled = false;
}

void VBO_Handler::enableNormals()
{
    if (!handleNormal)
        return;
    if (!normalEnabled)
    {
        glEnableClientState(GL_NORMAL_ARRAY);
        normalEnabled = true;
    }
    if (actNorms == norms)
        return;
    glBindBuffer(GL_ARRAY_BUFFER, norms);
    glNormalPointer(GL_FLOAT, 0, 0);
}

void VBO_Handler::disableColors()
{
    if (colorEnabled)
        glDisableClientState(GL_COLOR_ARRAY);
    colorEnabled = false;
}

void VBO_Handler::enableColors()
{
    if (!handleColor)
        return;
    if (!colorEnabled)
    {
        glEnableClientState(GL_COLOR_ARRAY);
        colorEnabled = true;
    }
    if (actColrs == colrs)
        return;
    glBindBuffer(GL_ARRAY_BUFFER, colrs);
    glColorPointer(4, GL_FLOAT, 0, 0);
}

void VBO_Handler::enableArrays(bool texture, bool normal, bool color)
{
    enableVertex();
    if (!arrayEnabled)
        return;
    if (texture)
        enableTextures();
    else
        disableTextures();
    if (normal)
        enableNormals();
    else
        disableNormals();
    if (color)
        enableColors();
    else
        disableColors();
}

void VBO_Handler::enableArrays()
{
    enableArrays(handleTexture, handleNormal, handleColor);
}

void trace_and_abort()
{
#ifdef _WANT_BACKTRACE
    void *array[10];
    size_t size;
    char **strings;
    size_t i;
    size = backtrace (array, 10);
    strings = backtrace_symbols (array, size);
    printf ("Obtained %zd stack frames.\n", size);
    for (i = 0; i < size; i++)
        printf ("%s\n", strings[i]);
    free (strings);
#endif
    abort();
}

int VBO_Handler::vboAlloc(int Vsize)
{
    MemElement memElement;
    std::list<MemElement>::iterator it;

    // First check if there is a chunk that fit precisely
    for (it = freeVBOList.begin(); it != freeVBOList.end(); it++)
        if (it->Vsize == Vsize)
            break;
    if (it == freeVBOList.end())
        // If not check if there is a chunk big enough
        for (it = freeVBOList.begin(); it != freeVBOList.end(); it++)
            if (it->Vsize > Vsize)
                break;
    if (it == freeVBOList.end())
    {
        // Bad, not enough memory (or too fragmented)
        std::cout << "V";
        if (handleTexture)
            std::cout << "T";
        if (handleNormal)
            std::cout << "N";
        if (handleColor)
            std::cout << "C";
        std::cout << " requested " << Vsize << " vertexes" << std::endl;
        trace_and_abort ();
    }

    // Push a new element in the alloc list
    memElement.Vsize    = Vsize;
    memElement.vboIndex = it->vboIndex;
    alloVBOList.push_back(memElement);

    // Reduse the size of the old chunk
    it->Vsize    -= Vsize;
    it->vboIndex += Vsize;;
    // If nothing more drop from the Free List
    if (!it->Vsize)
        freeVBOList.erase(it);

    // return the address of the chunk
    return memElement.vboIndex;
}

void VBO_Handler::vboFree(int vboIndex)
{
    MemElement memElement;
    std::list<MemElement>::iterator it;

    // Check if we allocated that index
    for (it = alloVBOList.begin(); it != alloVBOList.end(); it++)
        if (it->vboIndex == vboIndex)
            break;

    if (it == alloVBOList.end())
    {
        if (vboIndex < 0)
            return;
        // Bad, that index was never allocated
        std::cout << "V";
        if (handleTexture)
            std::cout << "T";
        if (handleNormal)
            std::cout << "N";
        if (handleColor)
            std::cout << "C";
        std::cout << " deallocated " << vboIndex << " never allocated" << std::endl;
        trace_and_abort ();
    }

    // Save the chunk and drop from the allocated list
    memElement.vboIndex = vboIndex;
    memElement.Vsize    = it->Vsize;
    alloVBOList.erase(it);

    // Check in the free list for a contiguous previous chunk
    for (it = freeVBOList.begin(); it != freeVBOList.end(); it++)
        if (it->vboIndex + it->Vsize == memElement.vboIndex)
        {
            memElement.vboIndex = it->vboIndex;
            memElement.Vsize   += it->Vsize;
            freeVBOList.erase(it);
            break;
        }

    // Check in the free list for a contiguous successor chunk
    for (it = freeVBOList.begin(); it != freeVBOList.end(); it++)
        if (it->vboIndex == memElement.vboIndex + memElement.Vsize)
        {
            memElement.Vsize   += it->Vsize;
            freeVBOList.erase(it);
            break;
        }
    freeVBOList.push_back(memElement);
}

int VBO_Handler::reserveIndex (int vSize)
{
    int result = indexFillPoint;
    indexFillPoint += vSize;
    if (indexFillPoint > indexSize)
    {
        std::cout << "V";
        if (handleTexture)
            std::cout << "T";
        if (handleNormal)
            std::cout << "N";
        if (handleColor)
            std::cout << "C";
        std::cout << " Index requested " << indexFillPoint << " indexes"
                  << std::endl;
        trace_and_abort ();
    }
    if (arrayFillPoint > indexSize)
    {
        std::cout << "V";
        if (handleTexture)
            std::cout << "T";
        if (handleNormal)
            std::cout << "N";
        if (handleColor)
            std::cout << "C";
        std::cout << " requested " << arrayFillPoint << " indexes"
                  << std::endl;
        trace_and_abort ();
    }
    return result;
}

VBO_Manager vboManager;
VBO_Handler vboV(false, false, false, 1 << 18, 0);
VBO_Handler vboVC(false, false, true, 1 << 18, 0);
VBO_Handler vboVN(false, true, false, 1 << 15, 0);
VBO_Handler vboVT(true, false, false, 1 << 21, 0);
VBO_Handler vboVTC(true, false, true, 1 << 4, 0);
VBO_Handler vboVTN(true, true, false, 1 << 19, 1 << 14);
VBO_Handler vboVTNC(true, true, true, 1 << 0, 0);

// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
