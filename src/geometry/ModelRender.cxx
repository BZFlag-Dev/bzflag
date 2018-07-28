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

#include "Model.h"
#include "bzfgl.h"

void DrawMeshGeometry(const CMesh& mesh, bool isShadow)
{
    glBegin(GL_POLYGON);    // TODO, should be able to load this into a VBO super easy

    for (auto face : mesh.faces)
    {
        for (size_t f = 0; f < face.verts.size(); f++)
        {
            if (!isShadow)
            {
                if ((int)mesh.texCoords.size() > 0 && face.texCoords[f] < (int)mesh.texCoords.size())
                    glTexCoord2f(mesh.texCoords[face.texCoords[f]].x, mesh.texCoords[face.texCoords[f]].y);

                if ((int)mesh.normals.size() > 0 && face.normals[f] < (int)mesh.normals.size())
                    glNormal3f(mesh.normals[face.normals[f]].x, mesh.normals[face.normals[f]].y, mesh.normals[face.normals[f]].z);
            }
            glVertex3f(mesh.verts[face.verts[f]].x, mesh.verts[face.verts[f]].y, mesh.verts[face.verts[f]].z);
        }
      
    }
    glEnd();
}

void DrawModelGeometry(const CModel& model, bool isShadow)
{
    for (auto mesh : model.meshes)
        DrawMeshGeometry(mesh, isShadow);
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
