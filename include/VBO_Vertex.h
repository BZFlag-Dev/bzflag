/* bzflag
 * Copyright (c) 2019-2019 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef BZF_VBO_VERTEX_H
#define BZF_VBO_VERTEX_H

// inherits from
#include "VBO_Handler.h"

// system interface headers
#include <list>
#include <string>
#include <vector>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

// common interface headers
#include "bzfgl.h"

// This class handle the vertex GL buffer and additional
// texCoord/Normal/Color GL buffers
// The size (in number of elements) is defined at creation time
// Vertex Buffer is designed to have 3 coords
// Normals the same
// TexCoords 2 coords
// Colors 4 coords
class VBO_Vertex: public VBO_Handler
{
public:
    // Ctor: Set the availability of the extra buffers and the size
    VBO_Vertex(
        bool handleTexture,
        bool handleNormal,
        bool handleColor,
        int  vertexSize);
    ~VBO_Vertex();

    // Init will be called when a GL Context is ready
    // It creates all the GL buffers requested,
    void init() override;
    // Destroy will delete the GL Buffers
    void destroy() override;
    // Name of the VBO
    std::string vboName() override;

    // These write the GL buffers at position index from an std::vector
    void vertexData(int index, const std::vector<glm::vec3> vertices);
    void textureData(int index, const std::vector<glm::vec2> textures);
    void normalData(int index, const std::vector<glm::vec3> normals);
    void colorData(int index, const std::vector<glm::vec4> colors);

    // These are for the case the caller provides a bidinmensional array
    void vertexData(int index, int size, const GLfloat vertices[][3]);
    void textureData(int index, int size, const GLfloat textures[][2]);
    void normalData(int index, int size, const GLfloat normals[][3]);
    void colorData(int index, int size, const GLfloat colors[][4]);

    // These are for the case the caller provides a pointer to the first element
    void vertexData(int index, int size, const GLfloat *vertices);
    void textureData(int index, int size, const GLfloat *textures);
    void normalData(int index, int size, const GLfloat *normals);
    void colorData(int index, int size, const GLfloat *colors);

    // These are for the case the caller provides an array of glm::vec* elements
    void vertexData(int index, int size, const glm::vec3 vertices[]);
    void textureData(int index, int size, const glm::vec2 textures[]);
    void normalData(int index, int size, const glm::vec3 normals[]);
    void colorData(int index, int size, const glm::vec4 colors[]);

    // This method bind all the buffers, to be ready for drawing
    void enableArrays();
    // This method will select which buffer should be enabled
    // Vertex is always enabled
    void enableArrays(bool texture, bool normal, bool color);
    // This method will enable the vertex buffer only (used for shadows
    void enableVertexOnly();

private:
    // These disable the usage of TexCoord/Normals/Colors,
    // but do not unbind the related buffer
    static void disableTextures();
    static void disableNormals();
    static void disableColors();

    // The binded vertex/texture/normals/colors buffer
    static GLuint actVerts;
    static GLuint actTxcds;
    static GLuint actNorms;
    static GLuint actColrs;

    // Taken from the ctor
    bool handleTexture;
    bool handleNormal;
    bool handleColor;

    // The GL buffers created from the class
    // After enableArray these are copied in the static one
    GLuint verts;
    GLuint txcds;
    GLuint norms;
    GLuint colrs;

    // Bind vertex Buffer
    void enableVertex();
    // Bind the relevant Buffer and enable its usage
    void enableTextures();
    void enableNormals();
    void enableColors();

    // mantain the status of the enabling of the buffers
    static bool textureEnabled;
    static bool normalEnabled;
    static bool colorEnabled;
};

// This class is for the ELement (Indirect drawing)
class VBO_Element: public VBO_Handler
{
public:
    VBO_Element(int indexSize);
    ~VBO_Element();

    void init() override;
    void destroy() override;
    std::string vboName() override;

    void elementData(
        int index,
        int size,
        const GLuint element[]);

private:
    GLuint elems;
};

extern VBO_Vertex vboV;
extern VBO_Vertex vboVC;
extern VBO_Vertex vboVN;
extern VBO_Vertex vboVT;
extern VBO_Vertex vboVTN;
extern VBO_Vertex vboVTC;
extern VBO_Vertex vboVTNC;
extern VBO_Element vboElement;

#endif

// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
