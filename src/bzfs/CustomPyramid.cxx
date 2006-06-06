/* bzflag
 * Copyright (c) 1993 - 2006 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#include "common.h"

/* interface header */
#include "CustomPyramid.h"

/* system headers */
#include <math.h>
#include <sstream>
#include <vector>

/* common implementation headers */
#include "PyramidBuilding.h"
#include "MeshObstacle.h"
#include "PhysicsDriver.h"
#include "StateDatabase.h"
#include "ObstacleMgr.h"
#include "vectors.h"

/* bzfs implementation headers */
#include "ParseMaterial.h"


const char* CustomPyramid::faceNames[FaceCount] = {
  "x+",
  "x-",
  "y+",
  "y-",
  "bottom"
};


CustomPyramid::CustomPyramid()
{
  isOldPyramid = true;
  
  flipz = false;

  size[0] = size[1] = BZDB.eval(StateDatabase::BZDB_PYRBASE);
  size[2] = BZDB.eval(StateDatabase::BZDB_PYRHEIGHT);

  materials[XP].setTexture("pyrwall");
  materials[XN].setTexture("pyrwall");
  materials[YP].setTexture("pyrwall");
  materials[YN].setTexture("pyrwall");
  materials[ZN].setTexture("pyrwall");

  for (int i = 0; i < FaceCount; i++) {
    texsize[i][0] = -8.0f;
    texsize[i][1] = -8.0f;
    texoffset[i][0] = 0.0f;
    texoffset[i][1] = 0.0f;
    phydrv[i] = -1;
    drivethrough[i] = false;
    shootthrough[i] = false;
  }

  return;
}


CustomPyramid::~CustomPyramid()
{
  return;
}


bool CustomPyramid::read(const char *cmd, std::istream& input)
{
  bool materror;

  std::string line;
  std::getline(input, line);
  std::istringstream parms(line);
  std::string tmpCmd;
  input.putback('\n');
      
  std::vector<int> faceList;
      
  // see if a face has been specified
  for (int i = 0; i < FaceCount; i++) {
    if (strcasecmp(cmd, faceNames[i]) == 0) {
      faceList.push_back(i);
      break;
    }
  }
  // extra tests
  if (faceList.size() <= 0) {
    if (strcasecmp(cmd, "bottom") == 0) {
      faceList.push_back(ZN);
    }
    else if ((strcasecmp(cmd, "edge") == 0) || // meshpyr keyword
             (strcasecmp(cmd, "sides") == 0)) {
      faceList.push_back(XP);
      faceList.push_back(XN);
      faceList.push_back(YP);
      faceList.push_back(YN);
    }
  }

  if (faceList.size() > 0) {
    isOldPyramid = false;
    if (parms >> tmpCmd) {
      // set the cmd string by eating the first parameter
      cmd = tmpCmd.c_str();
    } else {
      return false; // missing param
    }
  }
  else {
    for (int i = 0; i < FaceCount; i++) {
      faceList.push_back(i);
    }
  }
  const int faceCount = (int)faceList.size(); 
  
  
  
  // parse the command
  if (strcasecmp(cmd, "flipz") == 0) {
    flipz = true;
    return true;
  }
  else if (strcasecmp(cmd, "drivethrough") == 0) {
    for (int i = 0; i < (int)faceList.size(); i++) {
      const int f = faceList[i];
      drivethrough[f] = true;
    }
    driveThrough = true; // for old pyramids
    return true;
  }
  else if (strcasecmp(cmd, "shootthrough") == 0) {
    for (int i = 0; i < (int)faceList.size(); i++) {
      const int f = faceList[i];
      shootthrough[f] = true;
    }
    shootThrough = true; // for old pyramids
    return true;
  }
  else if (strcasecmp(cmd, "passable") == 0) {
    for (int i = 0; i < (int)faceList.size(); i++) {
      const int f = faceList[i];
      drivethrough[f] = true;
      shootthrough[f] = true;
    }
    driveThrough = true; // for old pyramids
    shootThrough = true; // for old pyramids
    return true;
  }
  else if (strcasecmp(cmd, "texsize") == 0) {
    isOldPyramid = false;
    float tmp[2];
    if (!(parms >> tmp[0] >> tmp[1])) {
      return false;
    } else {
      for (int i = 0; i < (int)faceList.size(); i++) {
        const int f = faceList[i];
        texsize[f][0] = tmp[0];
        texsize[f][1] = tmp[1];
      }
    }
    return true;
  }
  else if (strcasecmp(cmd, "texoffset") == 0) {
    isOldPyramid = false;
    float tmp[2];
    if (!(parms >> tmp[0] >> tmp[1])) {
      return false;
    } else {
      for (int i = 0; i < (int)faceList.size(); i++) {
        const int f = faceList[i];
        texoffset[f][0] = tmp[0];
        texoffset[f][1] = tmp[1];
      }
    }
    return true;
  }
  else if (strcasecmp(cmd, "phydrv") == 0) {
    isOldPyramid = false;
    std::string drvname;
    if (!(parms >> drvname)) {
      std::cout << "missing Physics Driver parameter" << std::endl;
      return false;
    }
    int pd = PHYDRVMGR.findDriver(drvname);
    if ((pd == -1) && (drvname != "-1")) {
      std::cout << "couldn't find PhysicsDriver: " << drvname << std::endl;
      return false;
    } else {
      for (int i = 0; i < (int)faceList.size(); i++) {
        const int f = faceList[i];
        phydrv[f] = pd;
      }
      return true;
    }
  }

  // check for material properties
  bool gotMaterial = false;
  bool gotMatError = false;
  for (int i = 0; i < faceCount; i++) {
    std::istringstream parmsCopy(line);
    if (faceCount != FaceCount) {
      std::string tmp;
      parmsCopy >> tmp;
    }
    const int f = faceList[i];
    if (!parseMaterials(cmd, parmsCopy, &materials[f], 1, materror)) {
      break;
    }
    else {
      gotMaterial = true;
      if (materror) {
        gotMatError = true;
      }
    }
  }
  if (gotMaterial) {
    isOldPyramid = false;
    return !gotMatError;
  }

  // last chance
  if (WorldFileObstacle::read(cmd, parms)) {
    return true;
  }
  
  return false;
}


static void getEdgeLengths(const MeshTransform& xform, float lengths[6])
{
  MeshTransform::Tool xformTool(xform);
  float vo[3] = {-1.0f, -1.0f, 0.0f};
  float vx[3] = {+1.0f, -1.0f, 0.0f};
  float vy[3] = {-1.0f, +1.0f, 0.0f};
  float vt[3] = {0.0f, 0.0f, 1.0f};
  float vxp[3] = {+1.0f, 0.0f, 0.0f};
  float vxn[3] = {-1.0f, 0.0f, 0.0f};
  float vyp[3] = {0.0f, +1.0f, 0.0f};
  float vyn[3] = {0.0f, -1.0f, 0.0f};
  xformTool.modifyVertex(vo);
  xformTool.modifyVertex(vx);
  xformTool.modifyVertex(vy);
  xformTool.modifyVertex(vt);
  xformTool.modifyVertex(vxp);
  xformTool.modifyVertex(vxn);
  xformTool.modifyVertex(vyp);
  xformTool.modifyVertex(vyn);
  float dx[3], dy[3], dxp[3], dxn[3], dyp[3], dyn[3];
  vec3sub(dx, vx, vo);
  vec3sub(dy, vy, vo);
  vec3sub(dxp, vxp, vt);
  vec3sub(dxn, vxn, vt);
  vec3sub(dyp, vyp, vt);
  vec3sub(dyn, vyn, vt);
  lengths[0] = sqrtf(vec3dot(dx, dx));
  lengths[1] = sqrtf(vec3dot(dy, dy));
  lengths[2] = sqrtf(vec3dot(dxp, dxp));
  lengths[3] = sqrtf(vec3dot(dxn, dxn));
  lengths[4] = sqrtf(vec3dot(dyp, dyp));
  lengths[5] = sqrtf(vec3dot(dyn, dyn));
  return;
}


void CustomPyramid::writeToGroupDef(GroupDefinition *groupdef) const
{
  if (isOldPyramid && transform.isEmpty()) {
    PyramidBuilding* pyr =
      new PyramidBuilding(pos, rotation,
                          fabsf(size[0]), fabsf(size[1]), fabsf(size[2]),
                          driveThrough, shootThrough);
    if (flipz || (size[2] < 0.0f)) {
      pyr->setZFlip();
    }
    groupdef->addObstacle(pyr);
    return;
  }

  int i;
  
  // setup the transform
  MeshTransform xform;
  if (flipz || (size[2] < 0.0f)) {
    const float flipScale[3] = {1.0f, 1.0f, -1.0f};
    const float flipShift[3] = {0.0f, 0.0f, +1.0f};
    xform.addScale(flipScale);
    xform.addShift(flipShift);
  }
  
  const float zAxis[3] = {0.0f, 0.0f, 1.0f};
  xform.addScale(size);
  xform.addSpin((float)(rotation * (180.0 / M_PI)), zAxis);
  xform.addShift(pos);
  xform.append(transform);

  // get the length deltas from the transform
  // (X, Y, and 4 sheared Z values for pyramids)
  float edgeLengths[6];
  getEdgeLengths(xform, edgeLengths);
  
  
  std::vector<char> checkTypes;
  std::vector<cfvec3> checkPoints;
  std::vector<cfvec3> verts;
  std::vector<cfvec3> norms;
  std::vector<cfvec2> txcds;

  // add the checkpoint
  checkTypes.push_back(MeshObstacle::CheckInside);
  const float middle[3] = { 0.0f, 0.0f, 0.5f };
  checkPoints.push_back(middle);

  // add the vertex coordinates
  const float vertsData[5][3] = {
    {-1.0f, -1.0f, 0.0f}, {+1.0f, -1.0f, 0.0f},
    {+1.0f, +1.0f, 0.0f}, {-1.0f, +1.0f, 0.0f}, 
    {+0.0f, +0.0f, 1.0f}
  };
  for (i = 0; i < 5; i++) {
    verts.push_back(vertsData[i]);
  }

  // add the texture coordinates
  const int txcdAxis[6][2] = {
    {1, 2}, // XP
    {1, 3}, // XN
    {0, 4}, // YP
    {0, 5}, // YN
    {0, 1}  // ZN
  };
  const float txcdData[7][2] = {
    {0.0f, 0.0f}, {1.0f, 0.0f}, {0.5f, 1.0f}, // triangles
    {0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f} // quad
  };
  for (int face = 0; face < FaceCount; face++) {
    int cornerCount;
    int cornerOffset;
    if (face < 4) {
      cornerCount = 3;
      cornerOffset = 0;
    } else {
      cornerCount = 4;
      cornerOffset = 3;
    }
    for (int corner = 0; corner < cornerCount; corner++) {
      float txcd[2];
      for (int a = 0; a < 2; a++) {
        float scale;
        if (texsize[face][a] >= 0.0f) {
          scale = texsize[face][a];
        } else {
          const int axis = txcdAxis[face][a];
          scale = (edgeLengths[axis] / -texsize[face][a]);
        }
        const int realCorner = corner + cornerOffset;
        txcd[a] = (txcdData[realCorner][a] - texoffset[face][a]) * scale;
      }
      txcds.push_back(txcd);
    }
  }
  
  
  MeshObstacle* mesh = new MeshObstacle(xform, checkTypes, checkPoints,
                                        verts, norms, txcds, FaceCount,
                                        false, false, false, false);

  // get the material refs
  const BzMaterial* mats[FaceCount];
  for (i = 0; i < FaceCount; i++) {
    mats[i] = MATERIALMGR.addMaterial(&materials[i]);
  }

  // the index arrays
  std::vector<int> iv;
  std::vector<int> it;
  std::vector<int> in; // leave at 0 count (auto normals)

  // XP
  iv.clear(); it.clear();
  iv.push_back(1); iv.push_back(2); iv.push_back(4);
  it.push_back(0); it.push_back(1); it.push_back(2);
  mesh->addFace(iv, in, it, mats[XP], phydrv[XP], false, false,
                drivethrough[XP], shootthrough[XP], false);
                
  // XN
  iv.clear(); it.clear();
  iv.push_back(3); iv.push_back(0); iv.push_back(4);
  it.push_back(3); it.push_back(4); it.push_back(5);
  mesh->addFace(iv, in, it, mats[XN], phydrv[XN], false, false,
                drivethrough[XN], shootthrough[XN], false);
                
  // YP
  iv.clear(); it.clear();
  iv.push_back(2); iv.push_back(3); iv.push_back(4);
  it.push_back(6); it.push_back(7); it.push_back(8);
  mesh->addFace(iv, in, it, mats[YP], phydrv[YP], false, false,
                drivethrough[YP], shootthrough[YP], false);
                
  // YN
  iv.clear(); it.clear();
  iv.push_back(0); iv.push_back(1); iv.push_back(4);
  it.push_back(9); it.push_back(10); it.push_back(11);
  mesh->addFace(iv, in, it, mats[YN], phydrv[YN], false, false,
                drivethrough[YN], shootthrough[YN], false);
                
  // ZN
  iv.clear(); it.clear();
  iv.push_back(1); iv.push_back(0); iv.push_back(3); iv.push_back(2);
  it.push_back(12); it.push_back(13); it.push_back(14); it.push_back(15);
  mesh->addFace(iv, in, it, mats[ZN], phydrv[ZN], false, false,
                drivethrough[ZN], shootthrough[ZN], false);

  // to be or not to be...
  if (mesh->isValid()) {
    groupdef->addObstacle(mesh);
  } else {
    delete mesh;
  }

  return;
}


// Local variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
