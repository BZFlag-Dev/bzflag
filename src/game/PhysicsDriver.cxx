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

#include "PhysicsDriver.h"

#include <math.h>
#include <string.h>
#include <vector>

#include "TimeKeeper.h"
#include "Pack.h"


// this applies to sinusoid and clamp functions
const float PhysicsDriver::minPeriod = 0.1f;


//
// Physics Driver Manager
//

PhysicsDriverManager PHYDRVMGR;


PhysicsDriverManager::PhysicsDriverManager()
{
  return;
}


PhysicsDriverManager::~PhysicsDriverManager()
{
  clear();
  return;
}


void PhysicsDriverManager::clear()
{
  std::vector<PhysicsDriver*>::iterator it;
  for (it = drivers.begin(); it != drivers.end(); it++) {
    delete *it;
  }
  drivers.clear();
  return;
}


void PhysicsDriverManager::update()
{
  float t = TimeKeeper::getCurrent() - TimeKeeper::getStartTime();
  std::vector<PhysicsDriver*>::iterator it;
  for (it = drivers.begin(); it != drivers.end(); it++) {
    PhysicsDriver* driver = *it;
    driver->update(t);
  }
  return;
}


int PhysicsDriverManager::addDriver(PhysicsDriver* driver)
{
  drivers.push_back (driver);
  return ((int)drivers.size() - 1);
}


int PhysicsDriverManager::findDriver(const std::string& dyncol) const
{
  if (dyncol.size() <= 0) {
    return -1;
  }
  else if ((dyncol[0] >= '0') && (dyncol[0] <= '9')) {
    int index = atoi (dyncol.c_str());
    if ((index < 0) || (index >= (int)drivers.size())) {
      return -1;
    } else {
      return index;
    }
  }
  else {
    for (int i = 0; i < (int)drivers.size(); i++) {
      if (drivers[i]->getName() == dyncol) {
	return i;
      }
    }
    return -1;
  }
}


const PhysicsDriver* PhysicsDriverManager::getDriver(int id) const
{
  if ((id >= 0) && (id < (int)drivers.size())) {
    return drivers[id];
  } else {
    return NULL;
  }
}


void * PhysicsDriverManager::pack(void *buf)
{
  std::vector<PhysicsDriver*>::iterator it;
  buf = nboPackUInt(buf, (int)drivers.size());
  for (it = drivers.begin(); it != drivers.end(); it++) {
    PhysicsDriver* driver = *it;
    buf = driver->pack(buf);
  }
  return buf;
}


void * PhysicsDriverManager::unpack(void *buf)
{
  unsigned int i, count;
  buf = nboUnpackUInt (buf, count);
  for (i = 0; i < count; i++) {
    PhysicsDriver* driver = new PhysicsDriver;
    buf = driver->unpack(buf);
    addDriver(driver);
  }
  return buf;
}


int PhysicsDriverManager::packSize()
{
  int fullSize = sizeof (uint32_t);
  std::vector<PhysicsDriver*>::iterator it;
  for (it = drivers.begin(); it != drivers.end(); it++) {
    PhysicsDriver* driver = *it;
    fullSize = fullSize + driver->packSize();
  }
  return fullSize;
}


void PhysicsDriverManager::print(std::ostream& out, int level)
{
  std::vector<PhysicsDriver*>::iterator it;
  for (it = drivers.begin(); it != drivers.end(); it++) {
    PhysicsDriver* driver = *it;
    driver->print(out, level);
  }
  return;
}


//
// Physics Driver
//

PhysicsDriver::PhysicsDriver()
{
  // initialize
  name = "";
  velocity[0] = velocity[1] = velocity[2] = 0.0f;
  angularVel = 0.0f;
  angularPos[0] = angularPos[1] = 0.0f;
  return;
}


PhysicsDriver::~PhysicsDriver()
{
}


void PhysicsDriver::finalize()
{
  return;
}


bool PhysicsDriver::setName(const std::string& drvname)
{
  if (drvname.size() <= 0) {
    name = "";
    return false;
  }
  else if ((drvname[0] >= '0') && (drvname[0] <= '9')) {
    name = "";
    return false;
  }
  else {
    name = drvname;
  }
  return true;
}


const std::string& PhysicsDriver::getName() const
{
  return name;
}


void PhysicsDriver::setVelocity(const float vel[3])
{
  memcpy (velocity, vel, sizeof(float[3]));
  return;
}


void PhysicsDriver::setAngular(float angleVel, const float anglePos[2])
{
  // convert from (rotations/second) to (radians/second)
  angularVel = angleVel * (2.0f * M_PI);
  angularPos[0] = anglePos[0];
  angularPos[1] = anglePos[1];
  return;
}


void PhysicsDriver::update (float /*t*/)
{
  return;
}


void * PhysicsDriver::pack(void *buf)
{
  buf = nboPackStdString(buf, name);

  buf = nboPackVector (buf, velocity);
  buf = nboPackFloat (buf, angularVel);
  buf = nboPackFloat (buf, angularPos[0]);
  buf = nboPackFloat (buf, angularPos[1]);

  return buf;
}


void * PhysicsDriver::unpack(void *buf)
{
  buf = nboUnpackStdString(buf, name);

  finalize();

  buf = nboUnpackVector (buf, velocity);
  buf = nboUnpackFloat (buf, angularVel);
  buf = nboUnpackFloat (buf, angularPos[0]);
  buf = nboUnpackFloat (buf, angularPos[1]);

  return buf;
}


int PhysicsDriver::packSize()
{
  int fullSize = nboStdStringPackSize(name);

  fullSize += sizeof(float) * 3; // velocity
  fullSize += sizeof(float) * 1; // angular velocity
  fullSize += sizeof(float) * 2; // angular position

  return fullSize;
}


void PhysicsDriver::print(std::ostream& out, int /*level*/)
{
  out << "physics" << std::endl;

  if (name.size() > 0) {
    out << "  name " << name << std::endl;
  }

  const float* v = velocity;
  if ((v[0] != 0.0f) || (v[1] != 0.0f) || (v[2] != 0.0f)) {
    out << "  velocity " << v[0] << " " << v[1] << " " << v[2] << std::endl;
  }

  if (angularVel != 0.0f) {
    const float* ap = angularPos;
    out << "  angular " << angularVel << " "
			<< ap[0] << " " << ap[1] << std::endl;
  }

  out << "end" << std::endl << std::endl;

  return;
}


// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

