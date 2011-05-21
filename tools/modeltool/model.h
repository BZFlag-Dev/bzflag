/* bzflag
 * Copyright (c) 1993-2010 Tim Riker
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
#include <set>
#include <map>
#include <math.h>


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
extern bool useGrouping;
extern bool useSmoothBounce;
extern bool flipYZ;
extern float shineFactor;
extern float globalScale;
extern float globalShift[3];
extern float fudgeFactor;

extern float maxShineExponent; // OpenGL minimum shininess

extern std::vector<std::string> bspMaterialSkips; // materials to skip in a bsp map

typedef std::set<int>    tsIndexSet;
typedef std::vector<int> tvIndexList;

enum teModelAxis {
  eXAxis,
  eYAxis,
  eZAxis
};


#define LESS_THAN_TEST(a, b) {     \
  const float diff = (a) - (b);    \
  if (fabsf(diff) > fudgeFactor) { \
    return (diff < 0.0f);          \
  }                                \
}


class CVector2 {
  public:
    CVector2() : u(0.0f), v(0.0f) {}
    CVector2(float _u, float _v) : u(_u), v(_v) {}
    ~CVector2() {};
    float u, v;

    bool operator<(const CVector2& d) const {
      LESS_THAN_TEST(u, d.u)
      LESS_THAN_TEST(v, d.v)
      return false;
    }

    bool same(const CVector2& c) {
      return u == c.u && v == c.v;
    }
};
typedef std::vector<CVector2> tvVec2List;


class CVector3 {
  public:
    CVector3() : x(0.0f), y(0.0f), z(0.0f) {}
    CVector3(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {}
    ~CVector3() {};
    float x, y, z;

    bool operator<(const CVector3& d) const {
      LESS_THAN_TEST(x, d.x)
      LESS_THAN_TEST(y, d.y)
      LESS_THAN_TEST(z, d.z)
      return false;
    }

    float get(teModelAxis axis) {
      switch (axis) {
        case eXAxis:
          return x;
        case eYAxis:
          return y;
        default:
          return z;
      }
    }

    void translate(float val, teModelAxis axis) {
      switch (axis) {
        case eXAxis:
          x += val;
          return;
        case eYAxis:
          y += val;
          return;
        case eZAxis:
          z += val;
      }
    }

    bool same(const CVector3& v) {
      return x == v.x && y == v.y && z == v.z;
    }
};
typedef std::vector<CVector3> tvVec3List;


class CFace {
  public:
    CFace() {};
    ~CFace() {};

    std::string material;
    tvIndexList verts;
    tvIndexList normals;
    tvIndexList texCoords;

    void clear() {
      verts.clear();
      normals.clear();
      texCoords.clear();
    }
};
typedef std::vector<CFace> tvFaceList;


class CTriStrip {
  public:
    CTriStrip() {};
    ~CTriStrip() {};

    std::string material;
    tvIndexList verts;
    tvIndexList normals;
    tvIndexList texCoords;

    void clear() {
      verts.clear();
      normals.clear();
      texCoords.clear();
    }
};
typedef std::vector<CTriStrip> tvTriStripList;


class CTriFan {
  public:
    CTriFan() {};
    ~CTriFan() {};

    std::string material;
    tvIndexList verts;
    tvIndexList normals;
    tvIndexList texCoords;

    void clear() {
      verts.clear();
      normals.clear();
      texCoords.clear();
    }
};
typedef std::vector<CTriFan> tvTriFanList;


class CMaterial {
  public:
    CMaterial() {clear();}
    ~CMaterial() {};

    std::string texture;
    float   ambient[4];
    float   diffuse[4];
    float   specular[4];
    float   emission[4];
    float   shine;

    void clear() {
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
};

typedef std::map<std::string, CMaterial> tmMaterialMap;


class CMesh {
  public:
    CMesh() : needReindex(true) {};
    ~CMesh() {};

    bool needReindex;

    std::string name;

    tvVec3List verts;
    tvVec3List normals;
    tvVec2List texCoords;

    tvFaceList     faces;
    tvTriFanList   fans;
    tvTriStripList strips;

#ifdef _MODEL_TOOL_GFX
    void draw();
#endif

    float getMaxAxisValue(teModelAxis axis) {
      if (!valid()) {
        return 0.0f;
      }

      float pt = verts[0].get(axis);

      for (unsigned int i = 0; i < verts.size(); i++) {
        if (verts[i].get(axis) > pt) {
          pt = verts[i].get(axis);
        }
      }

      return pt;
    }

    float getMinAxisValue(teModelAxis axis) {
      if (!valid()) {
        return 0.0f;
      }

      float pt = verts[0].get(axis);

      for (unsigned int i = 0; i < verts.size(); i++) {
        if (verts[i].get(axis) < pt) {
          pt = verts[i].get(axis);
        }
      }

      return pt;
    }

    void translate(float value, teModelAxis axis) {
      for (unsigned int i = 0; i < verts.size(); i++) {
        verts[i].translate(value, axis);
      }
    }

    bool valid() {
      return faces.size() || strips.size() || fans.size();
    }

    void clear() {
      faces.clear();
      strips.clear();
      fans.clear();
      verts.clear();
      normals.clear();
      texCoords.clear();
      name = "";
    }

    void reindex();

    bool assignData(const tvVec3List& verts,
                    const tvVec3List& norms,
                    const tvVec2List& txcds);

    void getUsedIndices(tsIndexSet& verts, tsIndexSet& norms, tsIndexSet& txcds);

    static void mapVec2List(const tsIndexSet& indices,
                            const tvVec2List& input,
                            tvVec2List& output,
                            std::map<int, int>& indexMap);
    static void mapVec3List(const tsIndexSet& indices,
                            const tvVec3List& input,
                            tvVec3List& output,
                            std::map<int, int>& indexMap);
};
typedef std::vector<CMesh> tvMeshList;


class CCustomObject {
  public:
    std::string name;
    std::vector<std::string> params;

    void clear() { params.clear(); name = ""; }
};
typedef std::vector<CCustomObject> tvCustomObjectList;


class CModel {
  public:
    CModel() {};
    ~CModel() {};

    tmMaterialMap      materials;
    tvMeshList         meshes;
    tvCustomObjectList customObjects;

#ifdef _MODEL_TOOL_GFX
    void draw();
#endif

    void pushAboveAxis(teModelAxis axis) {
      if (meshes.empty()) {
        return;
      }

      float minValue = meshes[0].getMinAxisValue(axis);

      for (unsigned int i = 0; i < meshes.size(); i++) {
        if (minValue > meshes[i].getMinAxisValue(axis)) {
          minValue = meshes[i].getMinAxisValue(axis);
        }
      }
      for (unsigned int i = 0; i < meshes.size(); i++) {
        meshes[i].translate(-minValue, axis);
      }
    }

    void clear() {
      meshes.clear();
      materials.clear();
      customObjects.clear();
    }
};


#endif // _MODEL_H_

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=2 tabstop=8
