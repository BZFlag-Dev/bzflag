/* bzflag
 * Copyright (c) 1993 - 2007 Tim Riker
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
  if (strcasecmp ("linear", cmd) == 0) {
    float vel[3];
    if (!(input >> vel[0] >> vel[1] >> vel[2])) {
      std::cout << "parameters errors " << std::endl;
      return false;
    }
    driver->setLinear(vel);
  }
  else if (strcasecmp ("angular", cmd) == 0) {
    float vel;
    float pos[2];
    if (!(input >> vel >> pos[0] >> pos[1])) {
      std::cout << "parameters errors " << std::endl;
      return false;
    }
    driver->setAngular(vel, pos);
  }
  else if (strcasecmp ("radial", cmd) == 0) {
    float vel;
    float pos[2];
    if (!(input >> vel >> pos[0] >> pos[1])) {
      std::cout << "parameters errors " << std::endl;
      return false;
    }
    driver->setRadial(vel, pos);
  }
  else if (strcasecmp ("slide", cmd) == 0) {
    float slideTime;
    if (!(input >> slideTime)) {
      std::cout << "parameters errors " << std::endl;
      return false;
    }
    driver->setSlideTime(slideTime);
  }
  else if (strcasecmp ("death", cmd) == 0) {
    std::string line;
    std::getline(input, line);
    driver->setDeathMessage(line);
    input.putback('\n');
    if (driver->getDeathMsg().size() == 0) {
      std::cout << "Physics Driver: empty death message, pacifying" << std::endl;
    }
  }
  else {
    // NOTE: we don't use a WorldFileObstacle
    return WorldFileObject::read(cmd, input);
  }

  return true;
}


void CustomPhysicsDriver::writeToManager() const
{
  driver->setName(name);
  if ((name.size() > 0) && (PHYDRVMGR.findDriver(name) >= 0)) {
    std::cout << "warning: duplicate physics driver"
	      << " (" << name << ")" << std::endl;
  }
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
