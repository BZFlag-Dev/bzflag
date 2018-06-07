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

//
// modeltool.h : defintions of the model classes
//

#ifndef _MODEL_H_
#define _MODEL_H_

#include <string>
#include <vector>
#include <map>

#include "vectors.h"

extern std::string texdir;
extern std::string groupName;
extern bool useMaterials;
extern bool useAmbient;
extern bool useDiffuse;
extern bool useSpecular;
extern bool useShininess;
extern bool useEmission;
extern bool useNormals;
extern bool useTexcoords ;
extern bool flipYZ;
extern bool useSmoothBounce;
extern float shineFactor;
extern float globalScale;
extern float globalShift[3];

extern float maxShineExponent; // OpenGL minimum shininess

enum class teModelAxis
{
    eXAxis,
    eYAxis,
    eZAxis
} ;

class CTexCoord : public fvec2
{
public:

    bool same ( const CTexCoord &c )
    {
        return x == c.x && y == c.y;
    }

    typedef std::vector<CTexCoord> Vec;
};


class CVertex : public fvec3
{
public:

    float get ( teModelAxis axis )
    {
        switch (axis)
        {
        case teModelAxis::eXAxis:
            return x;
        case teModelAxis::eYAxis:
            return y;
        default:
            return z;
        }
    }

    void translate ( float val, teModelAxis axis )
    {
        switch (axis)
        {
        case teModelAxis::eXAxis:
            x += val;
            return;
        case teModelAxis::eYAxis:
            y += val;
            return;
        case teModelAxis::eZAxis:
            z += val;
        }
    }

    bool same ( const CVertex &v )
    {
        return x == v.x && y == v.y && z == v.z;
    }

    typedef std::vector<CVertex> Vec;
};


class CFace
{
public:
    CFace() {};
    ~CFace() {};

    typedef std::vector<int> IndexVec;

    std::string material;
    IndexVec verts;
    IndexVec normals;
    IndexVec texCoords;

    void clear ( void )
    {
        verts.clear();
        normals.clear();
        texCoords.clear();
    }
    typedef std::vector<CFace> Vec;
};


class CMaterial
{
public:
    CMaterial()
    {
        clear();
    }
    ~CMaterial() {};

    std::string texture;
    float       ambient[4];
    float       diffuse[4];
    float       specular[4];
    float       emission[4];
    float       shine;

    void clear ( void )
    {
        texture = "";
        ambient[0] = ambient[1] = ambient[2] = 0.2f;
        ambient[3] = 1.0f;
        diffuse[0] = diffuse[1] = diffuse[2] = 1.0f;
        diffuse[3] = 1.0f;
        specular[0] = specular[1] = specular[2] = 0.0f;
        specular[3] = 1.0f;
        emission[0] = emission[1] = emission[2] = 0.0f;
        emission[3] = 1.0f;
        shine = 0.0f;
    }

    typedef std::map<std::string, CMaterial> NameMap;
};



class CMesh
{
public:
    CMesh() {};
    ~CMesh() {};

    CVertex::Vec  verts;
    CVertex::Vec  normals;
    CTexCoord::Vec  texCoords;

    std::string name;
    CFace::Vec  faces;

    float getMaxAxisValue ( teModelAxis axis )
    {
        if (!valid())
            return 0.0f;

        float pt = verts[0].get(axis);

        for ( unsigned int i = 0; i < verts.size(); i++ )
            if ( verts[i].get(axis) > pt)
                pt = verts[i].get(axis);

        return pt;
    }

    float getMinAxisValue ( teModelAxis axis )
    {
        if (!valid())
            return 0.0f;

        float pt = verts[0].get(axis);

        for ( unsigned int i = 0; i < verts.size(); i++ )
            if ( verts[i].get(axis) < pt)
                pt = verts[i].get(axis);

        return pt;
    }

    void translate ( float value, teModelAxis axis )
    {
        for ( unsigned int i = 0; i < verts.size(); i++ )
            verts[i].translate(value,axis);
    }

    bool valid ( void )
    {
        return faces.size() != 0;
    }

    void clear ( void )
    {
        faces.clear();
        verts.clear();
        normals.clear();
        texCoords.clear();
        name = "";
    }

    void reindex(void)
    {
        CVertex::Vec      temp_verts;
        CVertex::Vec      temp_normals;
        CTexCoord::Vec  temp_texCoords;

        CFace::Vec::iterator    faceItr = faces.begin();
        while (faceItr != faces.end())
        {
            CFace   &face = *faceItr;
            CFace   newFace;

            newFace.material = face.material;

            CFace::IndexVec::iterator indexItr = face.verts.begin();
            while (indexItr != face.verts.end())
                newFace.verts.push_back(getNewIndex(verts[*indexItr++], temp_verts));

            indexItr = face.normals.begin();
            while (indexItr != face.normals.end())
                newFace.normals.push_back(getNewIndex(normals[*indexItr++], temp_normals));

            indexItr = face.texCoords.begin();
            while (indexItr != face.texCoords.end())
                newFace.texCoords.push_back(getNewIndex(texCoords[*indexItr++], temp_texCoords));

            *faceItr = newFace;
            faceItr++;
        }
        verts = temp_verts;
        normals = temp_normals;
        texCoords = temp_texCoords;
    }

protected:
    int getNewIndex(CVertex &vert, CVertex::Vec &vertList)
    {
        CVertex::Vec::iterator itr = vertList.begin();

        int count = 0;
        while (itr != vertList.end())
        {
            if (itr->same(vert))
                return count;
            count++;
            itr++;
        }
        vertList.push_back(vert);
        return count;
    }

   int getNewIndex(CTexCoord &coord, CTexCoord::Vec &coordList)
    {
        CTexCoord::Vec::iterator itr = coordList.begin();

        int count = 0;
        while (itr != coordList.end())
        {
            if (itr->same(coord))
                return count;
            count++;
            itr++;
        }
        coordList.push_back(coord);
        return count;
    }
};

typedef std::vector<CMesh> tvMeshList;

class CModel
{
public:
    CModel() {};
    ~CModel() {};

    CMaterial::NameMap       materials;
    tvMeshList      meshes;
    void pushAboveAxis ( teModelAxis axis )
    {
        if (!meshes.size())
            return;

        float minValue = meshes[0].getMinAxisValue(axis);

        for ( unsigned int i = 0; i < meshes.size(); i++ )
            if ( minValue > meshes[i].getMinAxisValue(axis))
                minValue = meshes[i].getMinAxisValue(axis);

        for ( unsigned int i = 0; i < meshes.size(); i++ )
            meshes[i].translate(-minValue,axis);
    }

    void clear ( void )
    {
        meshes.clear();
        materials.clear();
    }
};

void DrawMeshGeometry(const CMesh& mesh, bool isShadow = false);
void DrawModelGeometry(const CModel& model, bool isShadow = false);


#endif // _MODEL_H_

