/* bzflag
 * Copyright (c) 1993 - 2004 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* interface header */
#include "CustomGroup.h"

/* system headers */
#include <sstream>
#include <string>

/* common headers */
#include "global.h" // for CtfTeams
#include "ObstacleMgr.h"
#include "ParseColor.h"
#include "PhysicsDriver.h"


CustomGroup::CustomGroup(const std::string& groupdef)
{
  group = new GroupInstance(groupdef);
  if (groupdef.size() <= 0) {
    std::cout << "warning: group instance has no group definition" << std::endl;
  }
  return;
}


CustomGroup::~CustomGroup()
{
  delete group;
  return;
}


bool CustomGroup::read(const char *cmd, std::istream& input) {
  
  if (strcmp(cmd, "team") == 0) {
    int team;
    if (!(input >> team) || (team < 0) || (team >= CtfTeams)) {
      std::cout << "bad team specification" << std::endl;
      return false;
    } else {
      group->setTeam(team);
    }
  }
  else if (strcasecmp(cmd, "tint") == 0) {
    float tint[4];
    if (!parseColorStream(input, tint)) {
      std::cout << "bad " << cmd << " specification" << std::endl;
    } else {
      group->setTint(tint);
    }
  }
  else if (strcasecmp(cmd, "phydrv") == 0) {
    std::string drvname;
    if (!(input >> drvname)) {
      std::cout << "missing Physics Driver parameter" << std::endl;
      return false;
    }
    int phydrv = PHYDRVMGR.findDriver(drvname);
    if ((phydrv == -1) && (drvname != "-1")) {
      std::cout << "couldn't find PhysicsDriver: " << drvname << std::endl;
    } else {
      group->setPhysicsDriver(phydrv);
    }
  }
  else if (!WorldFileLocation::read(cmd, input)) {
    return false;
  }
  
  return true;
}


void CustomGroup::writeToGroupDef(GroupDefinition *grpdef) const
{
  // include the old style parameters
  MeshTransform xform;
  if ((size[0] != 1.0f) || (size[1] != 1.0f) || (size[2] != 1.0f)) {
    xform.addScale(size);
  }
  if (rotation != 0.0f) {
    const float zAxis[3] = {0.0f, 0.0f, 1.0f};
    xform.addSpin(rotation * (180.0f / M_PI), zAxis);
  }  
  if ((pos[0] != 0.0f) || (pos[1] != 0.0f) || (pos[2] != 0.0f)) {
    xform.addShift(pos);
  }
  xform.append(transform);
  
  group->setTransform(xform);
  
  // make the group instance
  if (group->getGroupDef().size() > 0) {
    grpdef->addGroupInstance(group);
  } else {
    delete group;
  }
  group = NULL;
  
  return;
}


// Local variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
