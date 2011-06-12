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

#ifdef _MSC_VER
#pragma warning( 4: 4786 )
#endif
#include "common.h"

/* system implementation headers */
#include <iostream>
#include <math.h>
#include <sstream>
#include <string.h>

/* common implementation headers */
#include "net/Pack.h"
#include "WorldFileObject.h"
#include "WorldFileLocation.h"
#include "game/MeshTransform.h"


WorldFileLocation::WorldFileLocation()
  : pos(0.0f, 0.0f, 0.0f)
  , size(1.0f, 1.0f, 1.0f)
  , rotation(0.0f) {
}


bool WorldFileLocation::read(const char* cmd, std::istream& input) {
  //
  // Position, Size, and Rotation
  //
  if ((strcasecmp(cmd, "pos") == 0) ||
      (strcasecmp(cmd, "position") == 0)) {
    if (!(input >> pos.x >> pos.y >> pos.z)) {
      return false;
    }
  }
  else if (strcasecmp(cmd, "size") == 0) {
    if (!(input >> size.x >> size.y >> size.z)) {
      return false;
    }
  }
  else if ((strcasecmp(cmd, "rot") == 0) ||
           (strcasecmp(cmd, "rotation") == 0)) {
    if (!(input >> rotation)) {
      return false;
    }
    // convert to radians
    rotation = (float)(rotation * (M_PI / 180.0));
  }
  else if (strcasecmp("shift", cmd) == 0) {
    fvec3 data;
    if (!(input >> data.x >> data.y >> data.z)) {
      std::cout << "parameters errors " << std::endl;
      return false;
    }
    transform.addShift(data);
  }
  else if (strcasecmp("scale", cmd) == 0) {
    fvec3 data;
    if (!(input >> data.x >> data.y >> data.z)) {
      std::cout << "parameters errors " << std::endl;
      return false;
    }
    transform.addScale(data);
  }
  else if (strcasecmp("shear", cmd) == 0) {
    fvec3 data;
    if (!(input >> data.x >> data.y >> data.z)) {
      std::cout << "parameters errors " << std::endl;
      return false;
    }
    transform.addShear(data);
  }
  else if (strcasecmp("spin", cmd) == 0) {
    float angle;
    fvec3 normal;
    if (!(input >> angle >> normal.x >> normal.y >> normal.z)) {
      std::cout << "parameters errors " << std::endl;
      return false;
    }
    transform.addSpin(angle, normal);
  }
  else if (strcasecmp("xform", cmd) == 0) {
    std::string _name;
    if (!(input >> _name)) {
      std::cout << "parameters errors " << std::endl;
      return false;
    }
    int xform = TRANSFORMMGR.findTransform(_name);
    if (xform == -1) {
      std::cout << "couldn't find Transform: " << _name << std::endl;
    }
    else {
      transform.addReference(xform);
    }
  }
  else {
    return WorldFileObject::read(cmd, input);
  }

  return true;
}


void* WorldFileLocation::pack(void* buf) const {
  buf = nboPackFVec3(buf, pos);
  buf = nboPackFVec3(buf, size);
  buf = nboPackFloat(buf, rotation);
  return buf;
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=2 tabstop=8 expandtab
