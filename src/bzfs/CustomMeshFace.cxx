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

#include "common.h"

/* interface header */
#include "CustomMeshFace.h"

/* bzfs implementation headers */
#include "ParseMaterial.h"

/* common implementation headers */
#include "PhysicsDriver.h"

/* system headers */
#include <sstream>
#include <iostream>


CustomMeshFace::CustomMeshFace(const BzMaterial& _material, int physics,
			       bool _noclusters,
			       bool bounce, unsigned char drive, unsigned char shoot)
{
  phydrv = physics;
  noclusters = _noclusters;
  smoothBounce = bounce;
  shootThrough = shoot;
  driveThrough = drive;
  material = _material;
  return;
}


static void getIntList(std::istream& input, std::vector<int>& list)
{
  std::string args;
  int value;

  list.clear();
  std::getline(input, args);
  std::istringstream parms(args);
  input.putback('\n');

  while (parms >> value) {
    list.push_back(value);
  }

  return;
}


bool CustomMeshFace::read(const char *cmd, std::istream& input)
{
  bool materror;

  if (strcasecmp(cmd, "vertices") == 0) {
    getIntList (input, vertices);
    if (vertices.size() < 3) {
      std::cout << "mesh faces need at least 3 vertices" << std::endl;
      return false;
    }
  }
  else if (strcasecmp(cmd, "normals") == 0) {
    getIntList (input, normals);
    if (normals.size() < 3) {
      std::cout << "mesh faces need at least 3 normals" << std::endl;
      return false;
    }
  }
  else if (strcasecmp(cmd, "texcoords") == 0) {
    getIntList (input, texcoords);
    if (texcoords.size() < 3) {
      std::cout << "mesh faces need at least 3 texcoords" << std::endl;
      return false;
    }
  }
  else if (strcasecmp(cmd, "phydrv") == 0) {
    std::string drvname;
    if (!(input >> drvname)) {
      std::cout << "missing Physics Driver parameter" << std::endl;
      return false;
    }
    phydrv = PHYDRVMGR.findDriver(drvname);
    if ((phydrv == -1) && (drvname != "-1")) {
      std::cout << "couldn't find PhysicsDriver: " << drvname << std::endl;
    }
  }
  else if (strcasecmp(cmd, "smoothbounce") == 0) {
    smoothBounce = true;
  }
  else if (strcasecmp(cmd, "noclusters") == 0) {
    noclusters = true;
  }
  else if (strcasecmp(cmd, "drivethrough") == 0) {
    driveThrough = 0xFF;
  }
  else if (strcasecmp(cmd, "shootthrough") == 0) {
    shootThrough = 0xFF;
  }
  else if (strcasecmp(cmd, "passable") == 0) {
    driveThrough = shootThrough = 0xFF;
  }
  else if (strcasecmp(cmd, "ricochet") == 0) {
    ricochet = true;
  }
  //
  //  Team parameters
  //
  else if (strcasecmp(cmd, "baseTeam") == 0) {
    int baseTeam;
    if (!(input >> baseTeam)) {
      std::cout << "missing baseTeam parameter" << std::endl;
      return false;
    }
    specialData.baseTeam = (TeamColor)baseTeam;
  }
  //
  //  Common link parameters
  //
  else if (strcasecmp(cmd, "linkName") == 0) {
    std::string linkName;
    if (!(input >> linkName)) {
      std::cout << "missing linkName parameter" << std::endl;
      return false;
    }
    specialData.linkName = linkName;
  }
  //
  //  Link source parameters
  //
  else if (strcasecmp(cmd, "linkSrcRebound") == 0) {
    specialData.stateBits |=  MeshFace::LinkSrcRebound;
  }
  else if (strcasecmp(cmd, "linkSrcNoGlow") == 0) {
    specialData.stateBits |=  MeshFace::LinkSrcNoGlow;
  }
  else if (strcasecmp(cmd, "linkSrcNoRadar") == 0) {
    specialData.stateBits |=  MeshFace::LinkSrcNoRadar;
  }
  else if (strcasecmp(cmd, "linkSrcNoSound") == 0) {
    specialData.stateBits |=  MeshFace::LinkSrcNoSound;
  }
  else if (strcasecmp(cmd, "linkSrcNoEffect") == 0) {
    specialData.stateBits |=  MeshFace::LinkSrcNoEffect;
  }
  else if (strcasecmp(cmd, "linkSrcCenter") == 0) {
    int index;
    if (!(input >> index)) {
      std::cout << "missing linkSrcCenter index" << std::endl;
      return false;
    }
    specialData.linkSrcGeo.centerIndex = index;
  }
  else if (strcasecmp(cmd, "linkSrcSdir") == 0) {
    int index;
    if (!(input >> index)) {
      std::cout << "missing linkSrcSdir index" << std::endl;
      return false;
    }
    specialData.linkSrcGeo.sDirIndex = index;
  }
  else if (strcasecmp(cmd, "linkSrcTdir") == 0) {
    int index;
    if (!(input >> index)) {
      std::cout << "missing linkSrcTdir index" << std::endl;
      return false;
    }
    specialData.linkSrcGeo.tDirIndex = index;
  }
  else if (strcasecmp(cmd, "linkSrcPdir") == 0) {
    int index;
    if (!(input >> index)) {
      std::cout << "missing linkSrcPdir index" << std::endl;
      return false;
    }
    specialData.linkSrcGeo.pDirIndex = index;
  }
  else if (strcasecmp(cmd, "linkSrcSscale") == 0) {
    float scale;
    if (!(input >> scale)) {
      std::cout << "missing linkSrcSscale parameter" << std::endl;
      return false;
    }
    specialData.linkSrcGeo.sScale = scale;
    specialData.linkSrcGeo.bits &= ~MeshFace::LinkAutoSscale;
  }
  else if (strcasecmp(cmd, "linkSrcTscale") == 0) {
    float scale;
    if (!(input >> scale)) {
      std::cout << "missing linkSrcTscale parameter" << std::endl;
      return false;
    }
    specialData.linkSrcGeo.tScale = scale;
    specialData.linkSrcGeo.bits &= ~MeshFace::LinkAutoTscale;
  }
  else if (strcasecmp(cmd, "linkSrcPscale") == 0) {
    float scale;
    if (!(input >> scale)) {
      std::cout << "missing linkSrcPscale parameter" << std::endl;
      return false;
    }
    specialData.linkSrcGeo.pScale = scale;
    specialData.linkSrcGeo.bits &= ~MeshFace::LinkAutoPscale;
  }
  //
  //  Link destination parameters
  //
  else if (strcasecmp(cmd, "linkDstCenter") == 0) {
    int index;
    if (!(input >> index)) {
      std::cout << "missing linkDstCenter index" << std::endl;
      return false;
    }
    specialData.linkDstGeo.centerIndex = index;
  }
  else if (strcasecmp(cmd, "linkDstSdir") == 0) {
    int index;
    if (!(input >> index)) {
      std::cout << "missing linkDstSdir index" << std::endl;
      return false;
    }
    specialData.linkDstGeo.sDirIndex = index;
  }
  else if (strcasecmp(cmd, "linkDstTdir") == 0) {
    int index;
    if (!(input >> index)) {
      std::cout << "missing linkDstTdir index" << std::endl;
      return false;
    }
    specialData.linkDstGeo.tDirIndex = index;
  }
  else if (strcasecmp(cmd, "linkDstPdir") == 0) {
    int index;
    if (!(input >> index)) {
      std::cout << "missing linkDstPdir index" << std::endl;
      return false;
    }
    specialData.linkDstGeo.pDirIndex = index;
  }
  else if (strcasecmp(cmd, "linkDstSscale") == 0) {
    float scale;
    if (!(input >> scale)) {
      std::cout << "missing linkDstSscale parameter" << std::endl;
      return false;
    }
    specialData.linkDstGeo.sScale = scale;
    specialData.linkDstGeo.bits &= ~MeshFace::LinkAutoSscale;
  }
  else if (strcasecmp(cmd, "linkDstTscale") == 0) {
    float scale;
    if (!(input >> scale)) {
      std::cout << "missing linkDstTscale parameter" << std::endl;
      return false;
    }
    specialData.linkDstGeo.tScale = scale;
    specialData.linkDstGeo.bits &= ~MeshFace::LinkAutoTscale;
  }
  else if (strcasecmp(cmd, "linkDstPscale") == 0) {
    float scale;
    if (!(input >> scale)) {
      std::cout << "missing linkDstPscale parameter" << std::endl;
      return false;
    }
    specialData.linkDstGeo.pScale = scale;
    specialData.linkDstGeo.bits &= ~MeshFace::LinkAutoPscale;
  }
  //
  //  Material
  //
  else if (parseMaterials(cmd, input, &material, 1, materror)) {
    if (materror) {
      return false;
    }
  }
  else {
    std::cout << "unknown mesh face property: " << cmd << std::endl;
    return false;
  }

  return true;
}


void CustomMeshFace::write(MeshObstacle *mesh) const
{
  const BzMaterial* matref = MATERIALMGR.addMaterial(&material);

  // does this face need special data?
  const MeshFace::SpecialData* sd = NULL;
  if (!specialData.linkName.empty() || (specialData.baseTeam >= 0)) {
    sd = &specialData;
  }

  mesh->addFace(vertices, normals, texcoords, matref, phydrv,
		noclusters, smoothBounce,
		driveThrough, shootThrough, ricochet,
		true /* triangulate if required */, sd);
  return;
}


// Local variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
