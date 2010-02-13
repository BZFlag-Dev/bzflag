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
#include "CustomMeshTransform.h"

/* system implementation headers */
#include <iostream>
#include <sstream>
#include <string.h>

/* common implementation headers */
#include "MeshTransform.h"
#include "vectors.h"


CustomMeshTransform::CustomMeshTransform()
{
  transform = new MeshTransform;
  return;
}


CustomMeshTransform::~CustomMeshTransform()
{
  delete transform;
  return;
}


bool CustomMeshTransform::read(const char *cmd, std::istream& input)
{
  if (strcasecmp ("shift", cmd) == 0) {
    fvec3 data;
    if (!(input >> data.x >> data.y >> data.z)) {
      std::cout << "parameters errors " << std::endl;
      return false;
    }
    transform->addShift(data);
  }
  else if (strcasecmp ("scale", cmd) == 0) {
    fvec3 data;
    if (!(input >> data.x >> data.y >> data.z)) {
      std::cout << "parameters errors " << std::endl;
      return false;
    }
    transform->addScale(data);
  }
  else if (strcasecmp ("shear", cmd) == 0) {
    fvec3 data;
    if (!(input >> data.x >> data.y >> data.z)) {
      std::cout << "parameters errors " << std::endl;
      return false;
    }
    transform->addShear(data);
  }
  else if (strcasecmp ("spin", cmd) == 0) {
    float degrees;
    fvec3 normal;
    if (!(input >> degrees >> normal.x >> normal.y >> normal.z)) {
      std::cout << "parameters errors " << std::endl;
      return false;
    }
    transform->addSpin(degrees, normal);
  }
  else if (strcasecmp ("xform", cmd) == 0) {
    std::string _name;
    if (!(input >> _name)) {
      std::cout << "parameters errors " << std::endl;
      return false;
    }
    int xform = TRANSFORMMGR.findTransform(_name);
    if (xform == -1) {
      std::cout << "couldn't find Transform: " << _name << std::endl;
    } else {
      transform->addReference(xform);
    }
  }
  else {
    // NOTE: we don't use a WorldFileObstacle
    return WorldFileObject::read(cmd, input);
  }

  return true;
}


void CustomMeshTransform::writeToManager() const
{
  transform->setName(name);
  if ((name.size() > 0) && (TRANSFORMMGR.findTransform(name) >= 0)) {
    std::cout << "WARNING: duplicate transform"
	      << " (" << name << ")" << std::endl;
  }
  transform->finalize();
  TRANSFORMMGR.addTransform(transform);
  transform = NULL;
  return;
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
