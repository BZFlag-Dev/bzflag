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
#include "CustomPhysicsDriver.h"

/* system implementation headers */
#include <sstream>

/* common implementation headers */
#include "PhysicsDriver.h"


CustomPhysicsDriver::CustomPhysicsDriver()
{
  driver = new PhysicsDriver;
  return;
}


CustomPhysicsDriver::~CustomPhysicsDriver()
{
  delete driver;
  return;
}


bool CustomPhysicsDriver::read(const char *cmd, std::istream& input)
{
  if (strcasecmp ("velocity", cmd) == 0) {
    float vel[3];
    if (!(input >> vel[0] >> vel[1] >> vel[2])) {
      return false;
    }
    driver->setVelocity(vel);
  }
  else if (strcasecmp ("angular", cmd) == 0) {
    float angVel;
    float angPos[2];
    if (!(input >> angVel >> angPos[0] >> angPos[1])) {
      return false;
    }
    driver->setAngular(angVel, angPos);
  }
  else {
    // NOTE: we don't use a WorldFileObstacle
    return WorldFileObject::read(cmd, input);
  }

  return true;
}


void CustomPhysicsDriver::write(WorldInfo * /*world*/) const
{
  driver->setName(name);
  driver->finalize();
  PHYDRVMGR.addDriver(driver);
  driver = NULL;
  return;
}


// Local variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
