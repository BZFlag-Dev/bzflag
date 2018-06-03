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
