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


#include "common.h"

// implementation header
#include "ObstacleGroup.h"

// system headers
#include <string>
#include <vector>
#include <iostream>

// common headers
#include "Pack.h"
#include "MeshTransform.h"
#include "Obstacle.h"
#include "BoxBuilding.h"
#include "PyramidBuilding.h"
#include "BaseBuilding.h"
#include "Teleporter.h"
#include "MeshObstacle.h"
#include "ArcObstacle.h"
#include "ConeObstacle.h"
#include "SphereObstacle.h"
#include "TetraBuilding.h"


//
// Group Instance
// - uses a group definition and a transform to produce obstacles
//


GroupInstance::GroupInstance(const std::string& _groupdef)
{
  groupdef = _groupdef;
}


GroupInstance::~GroupInstance()
{
}

void GroupInstance::setTransform(const MeshTransform& xform)
{
  transform = xform;
}

const MeshTransform& GroupInstance::getTransform() const
{
  return transform;
}

void* GroupInstance::pack(void* buf)
{
  buf = nboPackStdString(buf, groupdef);
  buf = transform.pack(buf);
  return buf;
}

void* GroupInstance::unpack(void* buf)
{
  buf = nboUnpackStdString(buf, groupdef);
  buf = transform.unpack(buf);
  return buf;
}

int GroupInstance::packSize()
{
  return nboStdStringPackSize(groupdef) + transform.packSize();
}

void GroupInstance::print(std::ostream& out, const std::string& indent) const
{
  out << indent << "group" << std::endl;
  out << indent << "  name" << groupdef << std::endl;
  transform.printTransforms(out, indent);
  out << indent << "end" << std::endl;
  return;
}


//
// Group Definition
// - defines an obstacle group
//

GroupDefinition::GroupDefinition(const std::string& _name)
{
  name = _name;
}

GroupDefinition::~GroupDefinition()
{
}

void GroupDefinition::addObstacle(Obstacle* obstacle)
{
  const char* type = obstacle->getType();
  if (type == BoxBuilding::getClassName()) {
    list[boxType].push_back(obstacle);
  }
  else if (type == BaseBuilding::getClassName()) {
    list[baseType].push_back(obstacle);
  }
  else if (type == PyramidBuilding::getClassName()) {
    list[pyrType].push_back(obstacle);
  }
  else if (type == Teleporter::getClassName()) {
    list[teleType].push_back(obstacle);
  }
  else if (type == MeshObstacle::getClassName()) {
    list[meshType].push_back(obstacle);
  }
  else if (type == ArcObstacle::getClassName()) {
    list[arcType].push_back(obstacle);
  }
  else if (type == ConeObstacle::getClassName()) {
    list[coneType].push_back(obstacle);
  }
  else if (type == SphereObstacle::getClassName()) {
    list[sphereType].push_back(obstacle);
  }
  else if (type == TetraBuilding::getClassName()) {
    list[tetraType].push_back(obstacle);
  }
  else {
    printf ("GroupDefinition::addObstacle() ERROR\n");
    exit(1);
  }
  return;
}

void GroupDefinition::addGroupInstance(GroupInstance* group)
{
  groups.push_back(group);
  return;
}

void GroupDefinition::freeAll()
{
}

void GroupDefinition::freeNonLocal()
{
}

void* GroupDefinition::pack(void* buf) const
{
  return buf;
}

void* GroupDefinition::unpack(void* buf)
{
  return buf;
}

int GroupDefinition::packSize() const
{
  return 0;
}

void GroupDefinition::print(std::ostream& out, const std::string& indent) const
{
  out << indent << "define" << std::endl;
  out << indent << "enddef" << std::endl;
  return;
}


//
// Group Definition Manager
// - utility class to keep track of group definitions
//

GroupDefinitionMgr GROUPDEFMGR;

GroupDefinitionMgr::GroupDefinitionMgr() : world("")
{
}

GroupDefinitionMgr::~GroupDefinitionMgr()
{
}

void GroupDefinitionMgr::addGroupDef(GroupDefinition* groupdef)
{
  list.push_back(groupdef);
  return;
}

GroupDefinition* GroupDefinitionMgr::findGroupDef(const std::string& name)
{
  for (unsigned int i = 0; i < list.size(); i++) {
    if (name == list[i]->getName()) {
      return list[i];
    }
  }
  return NULL;
}


// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

