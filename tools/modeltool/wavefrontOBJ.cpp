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
// wavefrontOBJ.cpp : reader for the OBJ files
//
//

#include "common.h"

// system headers
#include <stdio.h>
#include <string.h>
#include <string>
#include <vector>
#include <map>

// common headers
#include "TextUtils.h"
#include "wavefrontOBJ.h"


static void underscoreBeforeNumbers(std::string& name) {
  const char first = name[0];
  if ((first >= '0') && (first <= '9')) {
    std::string tmp = "_";
    tmp += name;
    name = tmp;
  }
  return;
}


static void readMTL(CModel& model, std::string file) {
  FILE* fp = fopen(file.c_str(), "rb");
  if (!fp) {
    printf("Could not open MTL file %s\n", file.c_str());
    return;
  }
  else {
    printf("Loading MTL file %s\n", file.c_str());
  }

  fseek(fp, 0, SEEK_END);
  int size = ftell(fp);
  fseek(fp, 0, SEEK_SET);

  char* pData = (char*)malloc(size + 1);
  fread(pData, size, 1, fp);
  pData[size] = 0;

  fclose(fp);

  std::string lineTerminator = "\n";

  std::string fileText = pData;
  free(pData);

  fileText = TextUtils::replace_all(fileText, std::string("\r"), std::string(""));

  std::vector<std::string> lines = TextUtils::tokenize(fileText, lineTerminator);

  if (lines.size() < 2) {
    return;
  }

  CMaterial material;
  std::string matName;
  material.clear();

  std::vector<std::string>::iterator lineItr = lines.begin();
  while (lineItr != lines.end()) {
    // do a trim here

    std::vector<std::string> lineParts = TextUtils::tokenize(*lineItr, std::string(" "));

    if (lineParts.size() > 1) {
      std::string tag = lineParts[0];
      if (tag != "#") {
        if (TextUtils::tolower(tag) == "newmtl") {
          matName = lineParts[1];
          underscoreBeforeNumbers(matName);
          model.materials[matName] = material;
        }
        if (TextUtils::tolower(tag) == "ka") {
          if (lineParts.size() > 3) {
            model.materials[matName].ambient[0] = (float)atof(lineParts[1].c_str());
            model.materials[matName].ambient[1] = (float)atof(lineParts[2].c_str());
            model.materials[matName].ambient[2] = (float)atof(lineParts[3].c_str());
          }
        }
        if (TextUtils::tolower(tag) == "kd") {
          if (lineParts.size() > 3) {
            model.materials[matName].diffuse[0] = (float)atof(lineParts[1].c_str());
            model.materials[matName].diffuse[1] = (float)atof(lineParts[2].c_str());
            model.materials[matName].diffuse[2] = (float)atof(lineParts[3].c_str());
          }
        }
        if (TextUtils::tolower(tag) == "d") {
          if (lineParts.size() > 1) {
            model.materials[matName].ambient[3] = (float)atof(lineParts[1].c_str());
            model.materials[matName].diffuse[3] = (float)atof(lineParts[1].c_str());
          }
        }
        if (TextUtils::tolower(tag) == "ks") {
          if (lineParts.size() > 3) {
            model.materials[matName].specular[0] = (float)atof(lineParts[1].c_str());
            model.materials[matName].specular[1] = (float)atof(lineParts[2].c_str());
            model.materials[matName].specular[2] = (float)atof(lineParts[3].c_str());
          }
        }

        if (TextUtils::tolower(tag) == "ns") {
          if (lineParts.size() > 1) {
            float shine = (float)atof(lineParts[1].c_str());
            // convert MTL "Ns" to OpenGL shininess  [0 - 100] => [0 - 128]
            shine = shine / 100.0f;
            if (shine < 0.0f) {
              shine = 0.0f;
            }
            else if (shine > 1.0f) {
              shine = 1.0f;
            }

            model.materials[matName].shine = (shine * maxShineExponent * shineFactor);
          }
        }

        if (TextUtils::tolower(tag) == "ke") {
          if (lineParts.size() > 3) {
            model.materials[matName].emission[0] = (float)atof(lineParts[1].c_str());
            model.materials[matName].emission[1] = (float)atof(lineParts[2].c_str());
            model.materials[matName].emission[2] = (float)atof(lineParts[3].c_str());
          }
        }
        if (TextUtils::tolower(tag) == "map_kd") {
          if (lineParts.size() > 1) {
            std::string texture = lineParts[1];
            if (texture[0] == '.') {
              texture = texture.c_str() + 2;
            }

            model.materials[matName].texture = texture;
          }
        }
      }
    }
    lineItr++;
  }
}


void parseOBJVertSection(const std::string& section,
                         int& vert, int& norm, int& txcd,
                         int vCount, int nCount, int tCount) {
  // TextUtils::tokenize() does not make 3
  // strings from "1//2", so do it the hard way
  const std::string::size_type npos = std::string::npos;
  std::string::size_type pos1, pos2 = npos;
  pos1 = section.find_first_of('/');

  if (pos1 != npos) {
    pos2 = section.find_first_of('/', pos1 + 1);
  }

  std::string vertPart, uvPart, normPart;

  if (pos1 == npos) {
    vertPart = section;
  }
  else {
    vertPart = section.substr(0, pos1);

    if (pos2 == npos) {
      uvPart = section.substr(pos1 + 1, npos);
    }
    else {
      uvPart = section.substr(pos1 + 1, pos2 - pos1 - 1);
      normPart = section.substr(pos2 + 1, npos);
    }
  }

  if (vertPart.size() > 0) {
    int index = atoi(vertPart.c_str());

    if (index < 0) {
      index = (vCount + 1) + index;
    }

    vert = index - 1;
  }

  if (uvPart.size() > 0) {
    int index = atoi(uvPart.c_str());

    if (index < 0) {
      index = (tCount + 1) + index;
    }

    txcd = index - 1;
  }

  if (normPart.size() > 0) {
    int index = atoi(normPart.c_str());

    if (index < 0) {
      index = (nCount + 1) + index;
    }

    norm = index - 1;
  }
}


static bool checkArgCount(int reqCount, int count,
                          const std::string& type, int lineNum) {
  if (count < (reqCount + 1)) {
    // FIXME -- lineNum is wrong because of the way the file is parsed
    printf("ERROR: not enough parameters for \"%s\" at line %i\n",
           type.c_str(), lineNum + 1);
    return false;
  }
  return true;
}


void readOBJ(CModel& model, std::string file) {
  model.clear();

  FILE* fp = fopen(file.c_str(), "r");
  if (!fp) {
    printf("Could not open OBJ file %s\n", file.c_str());
    return;
  }

  fseek(fp, 0, SEEK_END);
  int size = ftell(fp);
  fseek(fp, 0, SEEK_SET);

  char* pData = (char*)malloc(size + 1);
  fread(pData, size, 1, fp);
  pData[size] = 0;

  fclose(fp);

  // figure out the base path
  std::string baseFilePath;

  const char* p = strrchr(file.c_str(), '\\');
  if (!p) {
    p = strrchr(file.c_str(), '/');
  }

  if (p) {
    baseFilePath = file;
    baseFilePath.erase(baseFilePath.begin() + (p - file.c_str() + 1), baseFilePath.end());
  }

  std::string lineTerminator = "\n";

  std::string fileText = pData;
  free(pData);

  fileText = TextUtils::replace_all(fileText, std::string("\r"), std::string(""));

  std::vector<std::string> lines = TextUtils::tokenize(fileText, lineTerminator);

  if (lines.size() < 2) {
    return;
  }

  CMesh      mesh;
  tvVec3List temp_verts;
  tvVec3List temp_normals;
  tvVec2List temp_texCoords;

  std::string currentMaterial = "";

  for (size_t lineNum = 0; lineNum < lines.size(); lineNum++) {

    const std::vector<std::string> lineParts =
      TextUtils::tokenize(lines[lineNum], std::string(" "));

    if (lineParts.size() <= 0) {
      continue;
    }
    if (lineParts[0] == "#") {
      continue;
    }

    const std::string element = TextUtils::tolower(lineParts[0]);

    if (element == "mtllib") {
      if (!checkArgCount(1, lineParts.size(), element, lineNum)) {
        continue;
      }
      readMTL(model, baseFilePath + lineParts[1]);
    }
    else if (element == "v") {
      if (!checkArgCount(3, lineParts.size(), element, lineNum)) {
        continue;
      }
      CVector3 vert;
      vert.x = (float)atof(lineParts[1].c_str()) * globalScale + globalShift[0];

      if (flipYZ) {
        vert.y = -1.0f * (float)atof(lineParts[3].c_str()) * globalScale + globalShift[1];
        vert.z = (float)atof(lineParts[2].c_str()) * globalScale + globalShift[2];
      }
      else {
        vert.y = (float)atof(lineParts[2].c_str()) * globalScale + globalShift[1];
        vert.z = (float)atof(lineParts[3].c_str()) * globalScale + globalShift[2];
      }

      temp_verts.push_back(vert);
    }
    else if (element == "vt") {
      if (!checkArgCount(2, lineParts.size(), element, lineNum)) {
        continue;
      }
      CVector2 uv;
      uv.u = (float)atof(lineParts[1].c_str());
      uv.v = (float)atof(lineParts[2].c_str());

      temp_texCoords.push_back(uv);
    }
    else if (element == "vn") {
      if (!checkArgCount(3, lineParts.size(), element, lineNum)) {
        continue;
      }
      CVector3 norm;
      norm.x = (float)atof(lineParts[1].c_str());

      if (flipYZ) {
        norm.y = -1.0f * (float)atof(lineParts[3].c_str());
        norm.z = (float)atof(lineParts[2].c_str());
      }
      else {
        norm.y = (float)atof(lineParts[2].c_str());
        norm.z = (float)atof(lineParts[3].c_str());
      }
      temp_normals.push_back(norm);
    }
    else if ((element == "g") || (element == "o")) {
      if (!checkArgCount(1, lineParts.size(), element, lineNum)) {
        continue;
      }
      if (useGrouping) {
        if (mesh.assignData(temp_verts, temp_normals, temp_texCoords)) {
          model.meshes.push_back(mesh);
        }
        mesh.clear();
        mesh.name = lineParts[1];
      }
    }
    else if (element == "usemtl") {
      if (!checkArgCount(1, lineParts.size(), element, lineNum)) {
        continue;
      }
      currentMaterial = lineParts[1];
      underscoreBeforeNumbers(currentMaterial);
    }
    else if (element == "f") {
      if (!checkArgCount(3, lineParts.size(), element, lineNum)) {
        continue;
      }
      CFace face;
      face.material = currentMaterial;

      int partCount = (int)lineParts.size();

      for (int i = 1; i < partCount; i++) {
        int invalidValue = -2000000000;
        int v, n, t;
        v = n = t = invalidValue;
        parseOBJVertSection(lineParts[i], v, n, t,
                            temp_verts.size(),
                            temp_normals.size(),
                            temp_texCoords.size());

        if (v != invalidValue) { face.verts.push_back(v); }
        if (n != invalidValue) { face.normals.push_back(n); }
        if (t != invalidValue) { face.texCoords.push_back(t); }
      }

      bool valid = true;
      const int vSize = (int)face.verts.size();
      const int nSize = (int)face.normals.size();
      const int tSize = (int)face.texCoords.size();

      if ((nSize != 0) && (nSize != vSize)) {
        printf("vertex/normal count mismatch\n");
        valid = false;
      }
      if ((tSize != 0) && (tSize != vSize)) {
        printf("vertex/texcoord count mismatch\n");
        valid = false;
      }
      if (valid) {
        mesh.faces.push_back(face);
      }
    }
    else if (element == "t") {
      if (!checkArgCount(1, lineParts.size(), element, lineNum)) {
        continue;
      }
      CTriStrip strip;
      strip.material = currentMaterial;

      int partCount = (int)lineParts.size();

      for (int i = 1; i < partCount; i++) {
        int invalidValue = -2000000000;
        int v, n, t;
        v = n = t = invalidValue;
        parseOBJVertSection(lineParts[i], v, n, t,
                            temp_verts.size(),
                            temp_normals.size(),
                            temp_texCoords.size());

        if (v != invalidValue) { strip.verts.push_back(v); }
        if (n != invalidValue) { strip.normals.push_back(n); }
        if (t != invalidValue) { strip.texCoords.push_back(t); }
      }

      bool valid = true;
      const int vSize = (int)strip.verts.size();
      const int nSize = (int)strip.normals.size();
      const int tSize = (int)strip.texCoords.size();

      if ((nSize != 0) && (nSize != vSize)) {
        printf("vertex/normal count mismatch\n");
        valid = false;
      }
      if ((tSize != 0) && (tSize != vSize)) {
        printf("vertex/texcoord count mismatch\n");
        valid = false;
      }
      if (valid) {
        mesh.strips.push_back(strip);
      }
    }
    else if (element == "q") {
      if (!checkArgCount(1, lineParts.size(), element, lineNum)) {
        continue;
      }
      if (mesh.strips.size()) { // there has to be a last strip
        CTriStrip& strip = mesh.strips[mesh.strips.size() - 1];

        int partCount = (int)lineParts.size();

        for (int i = 1; i < partCount; i++) {
          int invalidValue = -2000000000;
          int v, n, t;
          v = n = t = invalidValue;
          parseOBJVertSection(lineParts[i], v, n, t,
                              temp_verts.size(),
                              temp_normals.size(),
                              temp_texCoords.size());

          if (v != invalidValue) { strip.verts.push_back(v); }
          if (n != invalidValue) { strip.normals.push_back(n); }
          if (t != invalidValue) { strip.texCoords.push_back(t); }
        }

        bool valid = true;
        const int vSize = (int)strip.verts.size();
        const int nSize = (int)strip.normals.size();
        const int tSize = (int)strip.texCoords.size();

        if ((nSize != 0) && (nSize != vSize)) {
          printf("vertex/normal count mismatch\n");
          valid = false;
        }
        if ((tSize != 0) && (tSize != vSize)) {
          printf("vertex/texcoord count mismatch\n");
          valid = false;
        }
      }
    }
  }

  if (mesh.assignData(temp_verts, temp_normals, temp_texCoords)) {
    model.meshes.push_back(mesh);
  }
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=2 tabstop=8
