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

#include "common.h"

/* interface header */
#include "CustomPyramid.h"

/* system headers */
#include <iostream>
#include <sstream>
#include <vector>
#include <math.h>

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


CustomPyramid::CustomPyramid(bool meshed)
  : isOldPyramid(!meshed) {
  flipz = false;

  size[0] = size[1] = BZDB.eval(BZDBNAMES.PYRBASE);
  size.z = BZDB.eval(BZDBNAMES.PYRHEIGHT);

  materials[XP].setTexture("pyrwall");
  materials[XN].setTexture("pyrwall");
  materials[YP].setTexture("pyrwall");
  materials[YN].setTexture("pyrwall");
  materials[ZN].setTexture("pyrwall");

  const float wallScale = 8.0f;
  const float roofScale = 8.0f;
  materials[XP].setTextureAutoScale(fvec2(wallScale, wallScale));
  materials[XN].setTextureAutoScale(fvec2(wallScale, wallScale));
  materials[YP].setTextureAutoScale(fvec2(wallScale, wallScale));
  materials[YN].setTextureAutoScale(fvec2(wallScale, wallScale));
  materials[ZN].setTextureAutoScale(fvec2(roofScale, roofScale));

  for (int i = 0; i < FaceCount; i++) {
    texSizes[i][0] = 0.0f;
    texSizes[i][1] = 0.0f;
    texOffsets[i][0] = 0.0f;
    texOffsets[i][1] = 0.0f;
    phyDrvs[i] = -1;
    driveThroughs[i] = 0;
    shootThroughs[i] = 0;
    ricochets[i] = false;
  }

  return;
}


CustomPyramid::~CustomPyramid() {
  return;
}


bool CustomPyramid::read(const char* cmd, std::istream& input) {
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
    }
    else {
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
      driveThroughs[f] = 0xFF;
    }
    driveThrough = 0xFF; // for old pyramids
    return true;
  }
  else if (strcasecmp(cmd, "shootthrough") == 0) {
    for (int i = 0; i < (int)faceList.size(); i++) {
      const int f = faceList[i];
      shootThroughs[f] = 0xFF;
    }
    shootThrough = 0xFF; // for old pyramids
    return true;
  }
  else if (strcasecmp(cmd, "passable") == 0) {
    for (int i = 0; i < (int)faceList.size(); i++) {
      const int f = faceList[i];
      driveThroughs[f] = 0xFF;
      shootThroughs[f] = 0xFF;
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
    }
    else {
      for (int i = 0; i < (int)faceList.size(); i++) {
        const int f = faceList[i];
        texSizes[f][0] = tmp[0];
        texSizes[f][1] = tmp[1];
      }
    }
    return true;
  }
  else if (strcasecmp(cmd, "texoffset") == 0) {
    isOldPyramid = false;
    float tmp[2];
    if (!(parms >> tmp[0] >> tmp[1])) {
      return false;
    }
    else {
      for (int i = 0; i < (int)faceList.size(); i++) {
        const int f = faceList[i];
        texOffsets[f][0] = tmp[0];
        texOffsets[f][1] = tmp[1];
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
    }
    else {
      for (int i = 0; i < (int)faceList.size(); i++) {
        const int f = faceList[i];
        phyDrvs[f] = pd;
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


static void getEdgeLengths(const MeshTransform& xform, float lengths[6]) {
  MeshTransform::Tool xformTool(xform);

  fvec3 vo(-1.0f, -1.0f, 0.0f);
  fvec3 vx(+1.0f, -1.0f, 0.0f);
  fvec3 vy(-1.0f, +1.0f, 0.0f);
  fvec3 vt(0.0f, 0.0f, 1.0f);
  fvec3 vxp(+1.0f, 0.0f, 0.0f);
  fvec3 vxn(-1.0f, 0.0f, 0.0f);
  fvec3 vyp(0.0f, +1.0f, 0.0f);
  fvec3 vyn(0.0f, -1.0f, 0.0f);

  xformTool.modifyVertex(vo);
  xformTool.modifyVertex(vx);
  xformTool.modifyVertex(vy);
  xformTool.modifyVertex(vt);
  xformTool.modifyVertex(vxp);
  xformTool.modifyVertex(vxn);
  xformTool.modifyVertex(vyp);
  xformTool.modifyVertex(vyn);

  lengths[0] = (vx - vo).length();
  lengths[1] = (vy - vo).length();
  lengths[2] = (vxp - vt).length();
  lengths[3] = (vxn - vt).length();
  lengths[4] = (vyp - vt).length();
  lengths[5] = (vyn - vt).length();
  return;
}


void CustomPyramid::writeToGroupDef(GroupDefinition* groupdef) const {
  if (isOldPyramid && transform.isEmpty()) {
    PyramidBuilding* pyr =
      new PyramidBuilding(pos, rotation,
                          fabsf(size.x), fabsf(size.y), fabsf(size.z),
                          driveThrough, shootThrough, ricochet);
    if (flipz || (size.z < 0.0f)) {
      pyr->setZFlip();
    }
    groupdef->addObstacle(pyr);
    return;
  }

  int i;

  // setup the transform
  MeshTransform xform;
  if (flipz || (size.z < 0.0f)) {
    const fvec3 flipScale(1.0f, 1.0f, -1.0f);
    const fvec3 flipShift(0.0f, 0.0f, +1.0f);
    xform.addScale(flipScale);
    xform.addShift(flipShift);
  }

  const fvec3 zAxis(0.0f, 0.0f, 1.0f);
  xform.addScale(size);
  xform.addSpin((float)(rotation * (180.0 / M_PI)), zAxis);
  xform.addShift(pos);
  xform.append(transform);

  // get the length deltas from the transform
  // (X, Y, and 4 sheared Z values for pyramids)
  float edgeLengths[6];
  getEdgeLengths(xform, edgeLengths);


  std::vector<char> checkTypes;
  std::vector<fvec3> checkPoints;
  std::vector<fvec3> verts;
  std::vector<fvec3> norms;
  std::vector<fvec2> txcds;

  // add the checkpoint
  checkTypes.push_back(MeshObstacle::InsideCheck);
  const fvec3 middle(0.0f, 0.0f, 0.5f);
  checkPoints.push_back(middle);

  // add the vertex coordinates
  const fvec3 vertsData[5] = {
    fvec3(-1.0f, -1.0f, 0.0f), fvec3(+1.0f, -1.0f, 0.0f),
    fvec3(+1.0f, +1.0f, 0.0f), fvec3(-1.0f, +1.0f, 0.0f),
    fvec3(+0.0f, +0.0f, 1.0f)
  };
  for (i = 0; i < 5; i++) {
    verts.push_back(vertsData[i]);
  }

  // add the texture coordinates
  bool needTexCoords = false;
  for (int face = 0; face < FaceCount; face++) {
    if ((texSizes[face][0] != 0.0f) || (texSizes[face][1] != 0.0f)) {
      needTexCoords = true;
    }
  }
  if (needTexCoords) {
    const int txcdAxis[6][2] = {
      {1, 2}, // XP
      {1, 3}, // XN
      {0, 4}, // YP
      {0, 5}, // YN
      {0, 1}  // ZN
    };
    const fvec2 txcdData[7] = {
      fvec2(0.0f, 0.0f), fvec2(1.0f, 0.0f), fvec2(0.5f, 1.0f), // triangles
      fvec2(0.0f, 0.0f), fvec2(1.0f, 0.0f), fvec2(1.0f, 1.0f), fvec2(0.0f, 1.0f) // quad
    };
    for (int face = 0; face < FaceCount; face++) {
      int cornerCount;
      int cornerOffset;
      if (face < 4) {
        cornerCount = 3;
        cornerOffset = 0;
      }
      else {
        cornerCount = 4;
        cornerOffset = 3;
      }
      for (int corner = 0; corner < cornerCount; corner++) {
        fvec2 txcd;
        for (int a = 0; a < 2; a++) {
          float scale;
          if (texSizes[face][a] >= 0.0f) {
            scale = texSizes[face][a];
          }
          else {
            const int axis = txcdAxis[face][a];
            scale = (edgeLengths[axis] / -texSizes[face][a]);
          }
          const int realCorner = corner + cornerOffset;
          txcd[a] = (txcdData[realCorner][a] - texOffsets[face][a]) * scale;
        }
        txcds.push_back(txcd);
      }
    }
  }

  MeshObstacle* mesh = new MeshObstacle(xform, checkTypes, checkPoints,
                                        verts, norms, txcds, FaceCount,
                                        false, false, 0, 0, false);

  mesh->setDriveThrough(driveThrough);
  mesh->setShootThrough(shootThrough);

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
  if ((texSizes[XP][0] != 0.0f) || (texSizes[XP][1] != 0.0f)) {
    it.push_back(0); it.push_back(1); it.push_back(2);
  }
  mesh->addFace(iv, in, it, mats[XP], phyDrvs[XP], false, false,
                driveThroughs[XP], shootThroughs[XP], ricochets[XP],
                false, NULL);

  // XN
  iv.clear(); it.clear();
  iv.push_back(3); iv.push_back(0); iv.push_back(4);
  if ((texSizes[XN][0] != 0.0f) || (texSizes[XN][1] != 0.0f)) {
    it.push_back(3); it.push_back(4); it.push_back(5);
  }
  mesh->addFace(iv, in, it, mats[XN], phyDrvs[XN], false, false,
                driveThroughs[XN], shootThroughs[XN], ricochets[XN],
                false, NULL);

  // YP
  iv.clear(); it.clear();
  iv.push_back(2); iv.push_back(3); iv.push_back(4);
  if ((texSizes[YP][0] != 0.0f) || (texSizes[YP][1] != 0.0f)) {
    it.push_back(6); it.push_back(7); it.push_back(8);
  }
  mesh->addFace(iv, in, it, mats[YP], phyDrvs[YP], false, false,
                driveThroughs[YP], shootThroughs[YP], ricochets[YP],
                false, NULL);

  // YN
  iv.clear(); it.clear();
  iv.push_back(0); iv.push_back(1); iv.push_back(4);
  if ((texSizes[YN][0] != 0.0f) || (texSizes[YN][1] != 0.0f)) {
    it.push_back(9); it.push_back(10); it.push_back(11);
  }
  mesh->addFace(iv, in, it, mats[YN], phyDrvs[YN], false, false,
                driveThroughs[YN], shootThroughs[YN], ricochets[YN],
                false, NULL);

  // ZN
  iv.clear(); it.clear();
  iv.push_back(1); iv.push_back(0); iv.push_back(3); iv.push_back(2);
  if ((texSizes[ZN][0] != 0.0f) || (texSizes[ZN][1] != 0.0f)) {
    it.push_back(12); it.push_back(13); it.push_back(14); it.push_back(15);
  }
  mesh->addFace(iv, in, it, mats[ZN], phyDrvs[ZN], false, false,
                driveThroughs[ZN], shootThroughs[ZN], ricochets[ZN],
                false, NULL);

  mesh->setName(name.c_str());

  mesh->finalize();

  // to be or not to be...
  if (mesh->isValid()) {
    groupdef->addObstacle(mesh);
  }
  else {
    delete mesh;
  }

  return;
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=2 tabstop=8
