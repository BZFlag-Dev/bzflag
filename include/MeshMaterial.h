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

/* TetraBuilding:
 *	Encapsulates a tetrahederon in the game environment.
 */

#ifndef	BZF_MESH_MATERIAL_H
#define	BZF_MESH_MATERIAL_H

#include "common.h"
#include <string>
#include <ostream>


class MeshMaterial {

  public:
    MeshMaterial();
    MeshMaterial(const MeshMaterial& material);
    
    bool operator==(const MeshMaterial& material);
    MeshMaterial& operator=(const MeshMaterial& material);
    
    void reset();
    
    void *pack(void *);
    void *unpack(void *);
    int packSize();
    
    void print(std::ostream& out, int level);

    // data
    bool useColor;
    bool useTexture;
    bool useMaterial;
    std::string texture;
    int dynamicColor;
    int textureMatrix;
    float ambient[4];
    float diffuse[4];
    float specular[4];
    float emission[4];
    float shininess;
};

#endif // BZF_MESH_MATERIAL_H

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

