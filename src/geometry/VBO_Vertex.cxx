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
#include "VBO_Vertex.h"

bool VBO_Vertex::textureEnabled = false;
bool VBO_Vertex::normalEnabled  = false;
bool VBO_Vertex::colorEnabled   = false;
GLuint VBO_Vertex::actVerts     = 0;
GLuint VBO_Vertex::actNorms     = 0;
GLuint VBO_Vertex::actTxcds     = 0;
GLuint VBO_Vertex::actColrs     = 0;

VBO_Vertex::VBO_Vertex(
    bool handleTexture_,
    bool handleNormal_,
    bool handleColor_,
    int vertexSize_) :
    VBO_Handler(vertexSize_),
    handleTexture(handleTexture_), handleNormal(handleNormal_),
    handleColor(handleColor_)
{
}

VBO_Vertex::~VBO_Vertex()
{
}

void VBO_Vertex::init()
{
    glGenBuffers(1, &verts);
    glBindBuffer(GL_ARRAY_BUFFER, verts);
    glBufferData(
        GL_ARRAY_BUFFER,
        vboSize * sizeof(GLfloat[3]),
        nullptr,
        GL_DYNAMIC_DRAW);
    if (handleTexture)
    {
        glGenBuffers(1, &txcds);
        glBindBuffer(GL_ARRAY_BUFFER, txcds);
        glBufferData(
            GL_ARRAY_BUFFER,
            vboSize * sizeof(GLfloat[2]),
            nullptr,
            GL_DYNAMIC_DRAW);
    }
    if (handleNormal)
    {
        glGenBuffers(1, &norms);
        glBindBuffer(GL_ARRAY_BUFFER, norms);
        glBufferData(
            GL_ARRAY_BUFFER,
            vboSize * sizeof(GLfloat[3]),
            nullptr,
            GL_DYNAMIC_DRAW);
    }
    if (handleColor)
    {
        glGenBuffers(1, &colrs);
        glBindBuffer(GL_ARRAY_BUFFER, colrs);
        glBufferData(
            GL_ARRAY_BUFFER,
            vboSize * sizeof(GLfloat[4]),
            nullptr,
            GL_DYNAMIC_DRAW);
    }
    enableVertexOnly();
}

void VBO_Vertex::destroy()
{
    enableVertexOnly();
    glDeleteBuffers(1, &verts);
    if (handleTexture)
        glDeleteBuffers(1, &txcds);
    if (handleNormal)
        glDeleteBuffers(1, &norms);
    if (handleColor)
        glDeleteBuffers(1, &colrs);
}

std::string VBO_Vertex::vboName()
{
    std::string name = "V";
    if (handleTexture)
        name += "T";
    if (handleNormal)
        name += "N";
    if (handleColor)
        name += "C";
    return name;
}

void VBO_Vertex::vertexData(
    int index,
    int size,
    const GLfloat vertices[][3])
{
    vertexData(index, size, vertices[0]);
}

void VBO_Vertex::vertexData(int index, int size, const GLfloat *vertices)
{
    glBindBuffer(GL_ARRAY_BUFFER, verts);
    glBufferSubData(
        GL_ARRAY_BUFFER,
        index * sizeof(GLfloat[3]),
        size * sizeof(GLfloat[3]),
        vertices);
}

void VBO_Vertex::vertexData(int index, int size, const glm::vec3 vertices[])
{
    vertexData(index, size, &vertices[0][0]);
}

void VBO_Vertex::vertexData(
    int index,
    const std::vector<glm::vec3> vertices)
{
    vertexData(index, vertices.size(), vertices.data());
}

void VBO_Vertex::textureData(
    int index,
    int size,
    const GLfloat textures[][2])
{
    textureData(index, size, textures[0]);
}

void VBO_Vertex::textureData(int index, int size, const GLfloat *textures)
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

void VBO_Vertex::textureData(int index, int size, const glm::vec2 textures[])
{
    textureData(index, size, &textures[0][0]);
}

void VBO_Vertex::textureData(
    int index,
    const std::vector<glm::vec2> textures)
{
    textureData(index, textures.size(), textures.data());
}

void VBO_Vertex::normalData(
    int index,
    int size,
    const GLfloat normals[][3])
{
    normalData(index, size, normals[0]);
}

void VBO_Vertex::normalData(int index, int size, const GLfloat *normals)
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

void VBO_Vertex::normalData(int index, int size, const glm::vec3 normals[])
{
    normalData(index, size, &normals[0][0]);
}

void VBO_Vertex::normalData(
    int index,
    const std::vector<glm::vec3> normals)
{
    normalData(index, normals.size(), normals.data());
}

void VBO_Vertex::colorData(
    int index,
    int size,
    const GLfloat colors[][4])
{
    colorData(index, size, colors[0]);
}

void VBO_Vertex::colorData(int index, int size, const GLfloat *colors)
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

void VBO_Vertex::colorData(int index, int size, const glm::vec4 colors[])
{
    colorData(index, size, &colors[0][0]);
}

void VBO_Vertex::colorData(
    int index,
    const std::vector<glm::vec4> colors)
{
    colorData(index, colors.size(), colors.data());
}

void VBO_Vertex::enableArrays(bool texture, bool normal, bool color)
{
    enableVertex();
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

void VBO_Vertex::enableArrays()
{
    enableArrays(handleTexture, handleNormal, handleColor);
}

void VBO_Vertex::enableVertexOnly()
{
    enableVertex();
    disableTextures();
    disableNormals();
    disableColors();
}

void VBO_Vertex::enableVertex()
{
    if (actVerts == verts)
        return;
    glBindBuffer(GL_ARRAY_BUFFER, verts);
    glVertexPointer(3, GL_FLOAT, 0, 0);
}

void VBO_Vertex::disableTextures()
{
    if (textureEnabled)
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    textureEnabled = false;
}

void VBO_Vertex::enableTextures()
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

void VBO_Vertex::disableNormals()
{
    if (normalEnabled)
        glDisableClientState(GL_NORMAL_ARRAY);
    normalEnabled = false;
}

void VBO_Vertex::enableNormals()
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

void VBO_Vertex::disableColors()
{
    if (colorEnabled)
        glDisableClientState(GL_COLOR_ARRAY);
    colorEnabled = false;
}

void VBO_Vertex::enableColors()
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

VBO_Element::VBO_Element(int indexSize) :
    VBO_Handler(indexSize)
{
}

VBO_Element::~VBO_Element()
{
}

void VBO_Element::init()
{
    if (vboSize)
    {
        glGenBuffers(1, &elems);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elems);
        glBufferData(
            GL_ELEMENT_ARRAY_BUFFER,
            vboSize * sizeof(GLuint),
            NULL,
            GL_DYNAMIC_DRAW);
    }
}

void VBO_Element::destroy()
{
    if (vboSize)
        glDeleteBuffers(1, &elems);
}

std::string VBO_Element::vboName()
{
    return "E";
}

void VBO_Element::elementData(
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

VBO_Vertex vboV(false, false, false, 1 << 18);
VBO_Vertex vboVC(false, false, true, 1 << 18);
VBO_Vertex vboVN(false, true, false, 1 << 15);
VBO_Vertex vboVT(true, false, false, 1 << 21);
VBO_Vertex vboVTC(true, false, true, 1 << 4);
VBO_Vertex vboVTN(true, true, false, 1 << 19);
VBO_Vertex vboVTNC(true, true, true, 1 << 0);
VBO_Element vboElement(1 << 15);

// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
