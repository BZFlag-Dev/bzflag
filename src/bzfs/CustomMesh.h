/* bzflag
 * Copyright (c) 1993 - 2009 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef __CUSTOM_MESH_H__
#define __CUSTOM_MESH_H__

/* interface header */
#include "WorldFileObstacle.h"

/* system headers */
#include <string>
#include <vector>

/* common interface headers */
#include "BzMaterial.h"
#include "MeshObstacle.h"
#include "vectors.h"

/* local interface headers */
#include "WorldInfo.h"
#include "CustomMeshFace.h"


class CustomMesh : public WorldFileObstacle {
  public:
    CustomMesh(const char* meshName);
    ~CustomMesh();
    virtual bool read(const char *cmd, std::istream& input);
    virtual void writeToGroupDef(GroupDefinition*) const;

  private:

    BzMaterial material; // holds current defaults

    std::vector<char> checkTypes;
    std::vector<fvec3> checkPoints;
    std::vector<fvec3> vertices;
    std::vector<fvec3> normals;
    std::vector<fvec2> texcoords;

    int phydrv;
    bool noclusters;
    bool smoothBounce;
    bool decorative;

    std::vector<std::string> lodOptions;
    class MeshDrawInfo* drawInfo;

    std::vector<std::string>* weapon;
    std::vector<std::vector<std::string>*> weapons;

    CustomMeshFace* face;
    std::vector<CustomMeshFace*> faces;
};


#endif  /* __CUSTOM_MESH_H__ */

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
