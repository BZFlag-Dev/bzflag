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
#include "CustomBox.h"

/* system headers */
#include <iostream>
#include <math.h>
#include <sstream>
#include <vector>

/* common implementation headers */
#include "vectors.h"
#include "common/StateDatabase.h"
#include "game/PhysicsDriver.h"
#include "obstacle/BoxBuilding.h"
#include "obstacle/MeshObstacle.h"
#include "obstacle/ObstacleMgr.h"

/* bzfs implementation headers */
#include "ParseMaterial.h"


const char* CustomBox::faceNames[FaceCount] = {
  "x+",
  "x-",
  "y+",
  "y-",
  "z+",
  "z-"
};


CustomBox::CustomBox(bool meshed)
  : isOldBox(!meshed) {
  size.x = size.y = BZDB.eval(BZDBNAMES.BOXBASE);
  size.z = BZDB.eval(BZDBNAMES.BOXHEIGHT);

  materials[XP].setTexture("boxwall");
  materials[XN].setTexture("boxwall");
  materials[YP].setTexture("boxwall");
  materials[YN].setTexture("boxwall");
  materials[ZP].setTexture("roof");
  materials[ZN].setTexture("roof");

  const float wallScale = 8.0f;
  const float roofScale = 4.0f;
  materials[XP].setTextureAutoScale(fvec2(wallScale, wallScale));
  materials[XN].setTextureAutoScale(fvec2(wallScale, wallScale));
  materials[YP].setTextureAutoScale(fvec2(wallScale, wallScale));
  materials[YN].setTextureAutoScale(fvec2(wallScale, wallScale));
  materials[ZP].setTextureAutoScale(fvec2(roofScale, roofScale));
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

  ricochet = false;

  return;
}


CustomBox::~CustomBox() {
  return;
}


bool CustomBox::read(const char* cmd, std::istream& input) {
  bool materror;

  std::string line;
  std::getline(input, line);
  std::istringstream parms(line);
  input.putback('\n');

  std::string tmpCmd;
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
    if (strcasecmp(cmd, "top") == 0) {
      faceList.push_back(ZP);
    }
    else if (strcasecmp(cmd, "bottom") == 0) {
      faceList.push_back(ZN);
    }
    else if ((strcasecmp(cmd, "sides") == 0) ||
             (strcasecmp(cmd, "outside") == 0)) { // meshbox keyword
      faceList.push_back(XP);
      faceList.push_back(XN);
      faceList.push_back(YP);
      faceList.push_back(YN);
    }
  }

  if (faceList.size() > 0) {
    isOldBox = false;
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
  if (strcasecmp(cmd, "drivethrough") == 0) {
    for (int i = 0; i < faceCount; i++) {
      const int f = faceList[i];
      driveThroughs[f] = 0xFF;
    }
    driveThrough = 0xFF; // for old boxes
    return true;
  }
  else if (strcasecmp(cmd, "shootthrough") == 0) {
    for (int i = 0; i < faceCount; i++) {
      const int f = faceList[i];
      shootThroughs[f] = 0xFF;
    }
    shootThrough = 0xFF; // for old boxes
    return true;
  }
  else if (strcasecmp(cmd, "passable") == 0) {
    for (int i = 0; i < faceCount; i++) {
      const int f = faceList[i];
      driveThroughs[f] = 0xFF;
      shootThroughs[f] = 0xFF;
    }
    driveThrough = 0xFF; // for old boxes
    shootThrough = 0xFF; // for old boxes
    return true;
  }
  else if (strcasecmp(cmd, "ricochet") == 0) {
    for (int i = 0; i < faceCount; i++) {
      const int f = faceList[i];
      ricochets[f] = true;
    }
    ricochet = true; // for old boxes
    return true;
  }
  else if (strcasecmp(cmd, "texsize") == 0) {
    isOldBox = false;
    float tmp[2];
    if (!(parms >> tmp[0] >> tmp[1])) {
      return false;
    }
    else {
      for (int i = 0; i < faceCount; i++) {
        const int f = faceList[i];
        texSizes[f][0] = tmp[0];
        texSizes[f][1] = tmp[1];
      }
    }
    return true;
  }
  else if (strcasecmp(cmd, "texoffset") == 0) {
    isOldBox = false;
    float tmp[2];
    if (!(parms >> tmp[0] >> tmp[1])) {
      return false;
    }
    else {
      for (int i = 0; i < faceCount; i++) {
        const int f = faceList[i];
        texOffsets[f][0] = tmp[0];
        texOffsets[f][1] = tmp[1];
      }
    }
    return true;
  }
  else if (strcasecmp(cmd, "phydrv") == 0) {
    isOldBox = false;
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
      for (int i = 0; i < faceCount; i++) {
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
    isOldBox = false;
    return !gotMatError;
  }

  // last chance
  if (WorldFileObstacle::read(cmd, parms)) {
    return true;
  }

  return false;
}


static fvec3 getEdgeLengths(const MeshTransform& xform) {
  MeshTransform::Tool xformTool(xform);

  fvec3 vo(-1.0f, -1.0f, 0.0f);
  fvec3 vx(+1.0f, -1.0f, 0.0f);
  fvec3 vy(-1.0f, +1.0f, 0.0f);
  fvec3 vz(-1.0f, -1.0f, 1.0f);

  xformTool.modifyVertex(vo);
  xformTool.modifyVertex(vx);
  xformTool.modifyVertex(vy);
  xformTool.modifyVertex(vz);

  return fvec3((vx - vo).length(),
               (vy - vo).length(),
               (vz - vo).length());
}


void CustomBox::writeToGroupDef(GroupDefinition* groupdef) const {
  if (isOldBox && transform.isEmpty()) {
    BoxBuilding* box =
      new BoxBuilding(pos, rotation,
                      fabsf(size.x), fabsf(size.y), fabsf(size.z),
                      driveThrough, shootThrough, ricochet, false);
    groupdef->addObstacle(box);
    return;
  }


  int i;

  // setup the transform
  MeshTransform xform;
  const fvec3 zAxis(0.0f, 0.0f, 1.0f);
  xform.addScale(size);
  xform.addSpin((float)(rotation * (180.0 / M_PI)), zAxis);
  xform.addShift(pos);
  xform.append(transform);

  // get the length deltas from the transform
  const fvec3 edgeLengths = getEdgeLengths(xform);

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
  const fvec3 vertsData[8] = {
    fvec3(-1.0f, -1.0f, 0.0f), fvec3(+1.0f, -1.0f, 0.0f),
    fvec3(+1.0f, +1.0f, 0.0f), fvec3(-1.0f, +1.0f, 0.0f),
    fvec3(-1.0f, -1.0f, 1.0f), fvec3(+1.0f, -1.0f, 1.0f),
    fvec3(+1.0f, +1.0f, 1.0f), fvec3(-1.0f, +1.0f, 1.0f)
  };
  for (i = 0; i < 8; i++) {
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
      {1, 2}, // XN
      {0, 2}, // YP
      {0, 2}, // YN
      {0, 1}, // ZP
      {0, 1}  // ZN
    };
    const fvec2 txcdData[4] = {
      fvec2(0.0f, 0.0f), fvec2(1.0f, 0.0f),
      fvec2(1.0f, 1.0f), fvec2(0.0f, 1.0f)
    };
    for (int face = 0; face < FaceCount; face++) {
      for (int corner = 0; corner < 4; corner++) {
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
          txcd[a] = (txcdData[corner][a] - texOffsets[face][a]) * scale;
        }
        txcds.push_back(txcd);
      }
    }
  }

  MeshObstacle* mesh = new MeshObstacle(xform, checkTypes, checkPoints,
                                        verts, norms, txcds, FaceCount,
                                        false, false, 0, 0, false);

  mesh->setName(name.c_str());

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
  iv.push_back(1); iv.push_back(2); iv.push_back(6); iv.push_back(5);
  if ((texSizes[XP][0] != 0.0f) || (texSizes[XP][1] != 0.0f)) {
    it.push_back(0); it.push_back(1); it.push_back(2); it.push_back(3);
  }
  mesh->addFace(iv, in, it, mats[XP], phyDrvs[XP], false, false,
                driveThroughs[XP], shootThroughs[XP], ricochets[XP],
                false, NULL);

  // XN
  iv.clear(); it.clear();
  iv.push_back(3); iv.push_back(0); iv.push_back(4); iv.push_back(7);
  if ((texSizes[XN][0] != 0.0f) || (texSizes[XN][1] != 0.0f)) {
    it.push_back(4); it.push_back(5); it.push_back(6); it.push_back(7);
  }
  mesh->addFace(iv, in, it, mats[XN], phyDrvs[XN], false, false,
                driveThroughs[XN], shootThroughs[XN], ricochets[XN],
                false, NULL);

  // YP
  iv.clear(); it.clear();
  iv.push_back(2); iv.push_back(3); iv.push_back(7); iv.push_back(6);
  if ((texSizes[YP][0] != 0.0f) || (texSizes[YP][1] != 0.0f)) {
    it.push_back(8); it.push_back(9); it.push_back(10); it.push_back(11);
  }
  mesh->addFace(iv, in, it, mats[YP], phyDrvs[YP], false, false,
                driveThroughs[YP], shootThroughs[YP], ricochets[YP],
                false, NULL);

  // YN
  iv.clear(); it.clear();
  iv.push_back(0); iv.push_back(1); iv.push_back(5); iv.push_back(4);
  if ((texSizes[YN][0] != 0.0f) || (texSizes[YN][1] != 0.0f)) {
    it.push_back(12); it.push_back(13); it.push_back(14); it.push_back(15);
  }
  mesh->addFace(iv, in, it, mats[YN], phyDrvs[YN], false, false,
                driveThroughs[YN], shootThroughs[YN], ricochets[YN],
                false, NULL);

  // ZP
  iv.clear(); it.clear();
  iv.push_back(4); iv.push_back(5); iv.push_back(6); iv.push_back(7);
  if ((texSizes[ZP][0] != 0.0f) || (texSizes[ZP][1] != 0.0f)) {
    it.push_back(16); it.push_back(17); it.push_back(18); it.push_back(19);
  }
  mesh->addFace(iv, in, it, mats[ZP], phyDrvs[ZP], false, false,
                driveThroughs[ZP], shootThroughs[ZP], ricochets[ZP],
                false, NULL);

  // ZN
  iv.clear(); it.clear();
  iv.push_back(1); iv.push_back(0); iv.push_back(3); iv.push_back(2);
  if ((texSizes[ZN][0] != 0.0f) || (texSizes[ZN][1] != 0.0f)) {
    it.push_back(20); it.push_back(21); it.push_back(22); it.push_back(23);
  }
  mesh->addFace(iv, in, it, mats[ZN], phyDrvs[ZN], false, false,
                driveThroughs[ZN], shootThroughs[ZN], ricochets[ZN],
                false, NULL);

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
// ex: shiftwidth=2 tabstop=8 expandtab
