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

#ifndef __CUSTOMTETRA_H__
#define __CUSTOMTETRA_H__

/* interface header */
#include "WorldFileObject.h"

/* local interface header */
#include "WorldInfo.h"

/* system header */
#include <string>

class CustomTetra : public WorldFileObject {
  public:
    CustomTetra();
    virtual bool read(const char *cmd, std::istream& input);
    virtual void write(WorldInfo*) const;
  private:
    int vertexCount;

    bool visible[4];
    float vertices[4][3];
    bool useColor[4];
    float colors[4][4];
    bool useNormals[4];
    float normals[4][3][3];
    bool useTexCoords[4];
    float texCoords[4][3][2];
    int textureMatrices[4];
    std::string textures[4];

    bool driveThrough; //FIXME
    bool shootThrough; //FIXME
};

#endif  /* __CUSTOMTETRA_H__ */

// Local variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
