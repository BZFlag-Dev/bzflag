/* bzflag
 * Copyright (c) 1993 - 2004 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#include "common.h"

/* interface header */
#include "CustomTetra.h"

/* common implementation headers */
#include "TextureMatrix.h"


CustomTetra::CustomTetra()
{
  vertexCount = 0; // no vertices have yet been defined

  // make all of the planes visible
  for (int i = 0; i < 4; i++) {
    visible[i] = true;
    for (int j = 0; j < 4; j++) {
      colors[j][i] = 1.0f;
    }
    useColor[i] = false;
    useNormals[i] = false;
    useTexCoords[i] = false;
    textureMatrices[i] = -1;
    textures[i] = "";
  }

  // NOTE - we can't use WorldFileObstable as the base class
  //        because of the position, size, and rotation fields
  driveThrough = false;
  shootThrough = false;
}

bool CustomTetra::read(const char *cmd, std::istream& input)
{
  if (strcasecmp(cmd, "vertex") == 0) {
    if (vertexCount >= 4) {
      std::cout << "Extra tetrahedron vertex" << std::endl;
      // keep on chugging
    }
    else {
      float* vertex = vertices[vertexCount];
      input >> vertex[0] >> vertex[1] >> vertex[2];
      vertexCount++;
    }
  }
  else if (strcasecmp(cmd, "visible") == 0) {
    input >> visible[0] >> visible[1] >> visible[2] >> visible[3];
  }
  else if (strcasecmp(cmd, "color") == 0) {
    unsigned int bytecolor[4];
    if (vertexCount < 1) {
      // assign to all planes
      input >> bytecolor[0] >> bytecolor[1]
            >> bytecolor[2] >> bytecolor[3];
      for (int v = 0; v < 4; v++) {
        useColor[v] = true;
        for (int c = 0; c < 4; c++) {
          colors[v][c] = (float)bytecolor[c];
        }
      }
    }
    else if (vertexCount > 4) {
      std::cout << "Tetrahedron color for extra vertex" << std::endl;
      // keep on chugging
    }
    else if (useColor[vertexCount - 1]) {
      std::cout << "Extra tetrahedron color" << std::endl;
      // keep on chugging
    }
    else {
      input >> bytecolor[0] >> bytecolor[1]
            >> bytecolor[2] >> bytecolor[3];
      useColor[vertexCount - 1] = true;
      float* color = colors[vertexCount - 1];
      for (int c = 0; c < 4; c++) {
        color[c] = (float)bytecolor[c];
      }
    }
  }
  else if (strcasecmp(cmd, "normals") == 0) {
    if (vertexCount < 1) {
      std::cout << "Normals defined before any vertex" << std::endl;
      // keep on chugging
    }
    else if (vertexCount > 4) {
      std::cout << "Extra tetrahedron normals" << std::endl;
      // keep on chugging
    }
    else {
      useNormals[vertexCount - 1] = true;
      for (int v = 0; v < 3; v++) {
        float* normal = normals[vertexCount - 1][v];
        input >> normal[0] >> normal[1] >> normal[2];
      }
    }
  }
  else if (strcasecmp(cmd, "texcoords") == 0) {
    if (vertexCount < 1) {
      std::cout << "TexCoords defined before any vertex" << std::endl;
      // keep on chugging
    }
    else if (vertexCount > 4) {
      std::cout << "Extra tetrahedron texCoords" << std::endl;
      // keep on chugging
    }
    else {
      useTexCoords[vertexCount - 1] = true;
      for (int v = 0; v < 3; v++) {
        float* texCoord = texCoords[vertexCount - 1][v];
        input >> texCoord[0] >> texCoord[1];
      }
    }
  }
  else if (strcasecmp(cmd, "texture") == 0) {
    if (vertexCount < 1) {
      std::cout << "Texture defined before any vertex" << std::endl;
      // keep on chugging
    }
    else if (vertexCount > 4) {
      std::cout << "Extra tetrahedron texture" << std::endl;
      // keep on chugging
    }
    else {
      input >> textures[vertexCount - 1];
    }
  }
  else if (strcasecmp(cmd, "texmat") == 0) {
    if (vertexCount < 1) {
      std::cout << "TextureMatrix defined before any vertex" << std::endl;
      // keep on chugging
    }
    else if (vertexCount > 4) {
      std::cout << "Extra tetrahedron TextureMatrix" << std::endl;
      // keep on chugging
    }
    else {
      input >> textureMatrices[vertexCount - 1];
    }
  }
  else if (strcasecmp(cmd, "drivethrough") == 0) {
    driveThrough = true;
  }
  else if (strcasecmp(cmd, "shootthrough") == 0) {
    shootThrough = true;
  }
  else if (strcasecmp(cmd, "passable") == 0) {
    driveThrough = shootThrough = true;
  }
  else {
    // NOTE: we don't use a WorldFileObstacle
    return WorldFileObject::read(cmd, input);
  }

  return true;
}


void CustomTetra::write(WorldInfo *world) const
{
  if (vertexCount < 4) {
    std::cout << "Not creating tetrahedron, not enough vertices ("
              << vertexCount << ")" << std::endl;
    return;
  }

  world->addTetra(vertices, visible,
                  useColor, colors,
                  useNormals, normals,
                  useTexCoords, texCoords,
                  textureMatrices, textures, 
                  driveThrough, shootThrough);
}

// Local variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
