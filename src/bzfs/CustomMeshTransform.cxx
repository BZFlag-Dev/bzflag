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

/* interface header */
#include "CustomMeshTransform.h"

/* system implementation headers */
#include <sstream>

/* common implementation headers */
#include "MeshTransform.h"


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
    float data[3];
    if (!(input >> data[0] >> data[1] >> data[2])) {
      std::cout << "parameters errors " << std::endl;
      return false;
    }
    transform->addShift(data);
  }
  else if (strcasecmp ("scale", cmd) == 0) {
    float data[3];
    if (!(input >> data[0] >> data[1] >> data[2])) {
      std::cout << "parameters errors " << std::endl;
      return false;
    }
    transform->addScale(data);
  }
  else if (strcasecmp ("shear", cmd) == 0) {
    float data[3];
    if (!(input >> data[0] >> data[1] >> data[2])) {
      std::cout << "parameters errors " << std::endl;
      return false;
    }
    transform->addShear(data);
  }
  else if (strcasecmp ("spin", cmd) == 0) {
    float data[4];
    if (!(input >> data[0] >> data[1] >> data[2] >> data[3])) {
      std::cout << "parameters errors " << std::endl;
      return false;
    }
    transform->addSpin(data[0], &data[1]);
  }
  else if (strcasecmp ("xform", cmd) == 0) {
    std::string name;
    if (!(input >> name)) {
      std::cout << "parameters errors " << std::endl;
      return false;
    }
    int xform = TRANSFORMMGR.findTransform(name);
    if (xform == -1) {
      std::cout << "couldn't find Transform: " << name << std::endl;
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


void CustomMeshTransform::write(WorldInfo * /*world*/) const
{
  transform->setName(name);
  transform->finalize();
  TRANSFORMMGR.addTransform(transform);
  transform = NULL;
  return;
}


// Local variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
