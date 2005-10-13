/* bzflag
 * Copyright (c) 1993 - 2005 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* interface header */
#include "PhysicsDriver.h"

/* system implementation headers */
#include <math.h>
#include <ctype.h>
#include <string.h>

/* common implementation headers */
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
  float t = (float)(TimeKeeper::getCurrent() - TimeKeeper::getStartTime());
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


void * PhysicsDriverManager::pack(void *buf) const
{
  std::vector<PhysicsDriver*>::const_iterator it;
  buf = nboPackUInt(buf, (int)drivers.size());
  for (it = drivers.begin(); it != drivers.end(); it++) {
    PhysicsDriver* driver = *it;
    buf = driver->pack(buf);
  }
  return buf;
}


void * PhysicsDriverManager::unpack(void *buf)
{
  unsigned int i;
  uint32_t count;
  buf = nboUnpackUInt (buf, count);
  for (i = 0; i < count; i++) {
    PhysicsDriver* driver = new PhysicsDriver;
    buf = driver->unpack(buf);
    addDriver(driver);
  }
  return buf;
}


int PhysicsDriverManager::packSize() const
{
  int fullSize = sizeof (uint32_t);
  std::vector<PhysicsDriver*>::const_iterator it;
  for (it = drivers.begin(); it != drivers.end(); it++) {
    PhysicsDriver* driver = *it;
    fullSize = fullSize + driver->packSize();
  }
  return fullSize;
}


void PhysicsDriverManager::print(std::ostream& out,
				 const std::string& indent) const
{
  std::vector<PhysicsDriver*>::const_iterator it;
  for (it = drivers.begin(); it != drivers.end(); it++) {
    const PhysicsDriver* driver = *it;
    driver->print(out, indent);
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
  linear[0] = linear[1] = linear[2] = 0.0f;
  angularVel = 0.0f;
  angularPos[0] = angularPos[1] = 0.0f;
  radialVel = 0.0f;
  radialPos[0] = radialPos[1] = 0.0f;
  slide = false;
  slideTime = 0.0f;
  death = false;
  deathMsg = "";
  return;
}


PhysicsDriver::~PhysicsDriver()
{
}


void PhysicsDriver::finalize()
{
  if (slideTime > 0.0f) {
    slide = true;
  } else {
    slide = false;
    slideTime = 0.0f;
  }
  if (deathMsg.size() > 0) {
    death = true;
  }
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


void PhysicsDriver::setLinear(const float vel[3])
{
  memcpy (linear, vel, sizeof(float[3]));
  return;
}


void PhysicsDriver::setAngular(float vel, const float pos[2])
{
  // convert from (rotations/second) to (radians/second)
  angularVel = (float)(vel * (2.0 * M_PI));
  angularPos[0] = pos[0];
  angularPos[1] = pos[1];
  return;
}


void PhysicsDriver::setRadial(float vel, const float pos[2])
{
  radialVel = vel;
  radialPos[0] = pos[0];
  radialPos[1] = pos[1];
  return;
}


void PhysicsDriver::setSlideTime(float _slideTime)
{
  slideTime = _slideTime;
  return;
}


void PhysicsDriver::setDeathMessage(const std::string& msg)
{
  // strip any leading whitespace
  int first = 0;
  const char* c = msg.c_str();
  while ((*c != '\0') && isspace(*c)) {
    c++;
    first++;
  }
  std::string str = msg.substr(first);

  // limit the length
  if (str.size() > 256) {
    str.resize(256);
  }

  deathMsg = str;

  return;
}


void PhysicsDriver::update (float /*t*/)
{
  return;
}


void * PhysicsDriver::pack(void *buf) const
{
  buf = nboPackStdString(buf, name);

  buf = nboPackVector (buf, linear);
  buf = nboPackFloat (buf, angularVel);
  buf = nboPackFloat (buf, angularPos[0]);
  buf = nboPackFloat (buf, angularPos[1]);
  buf = nboPackFloat (buf, radialVel);
  buf = nboPackFloat (buf, radialPos[0]);
  buf = nboPackFloat (buf, radialPos[1]);

  buf = nboPackFloat (buf, slideTime);
  buf = nboPackStdString(buf, deathMsg);

  return buf;
}


void * PhysicsDriver::unpack(void *buf)
{
  buf = nboUnpackStdString(buf, name);

  buf = nboUnpackVector (buf, linear);
  buf = nboUnpackFloat (buf, angularVel);
  buf = nboUnpackFloat (buf, angularPos[0]);
  buf = nboUnpackFloat (buf, angularPos[1]);
  buf = nboUnpackFloat (buf, radialVel);
  buf = nboUnpackFloat (buf, radialPos[0]);
  buf = nboUnpackFloat (buf, radialPos[1]);

  buf = nboUnpackFloat (buf, slideTime);
  buf = nboUnpackStdString(buf, deathMsg);

  finalize();

  return buf;
}


int PhysicsDriver::packSize() const
{
  int fullSize = nboStdStringPackSize(name);

  fullSize += sizeof(float) * 3; // linear velocity
  fullSize += sizeof(float) * 1; // angular velocity
  fullSize += sizeof(float) * 2; // angular position
  fullSize += sizeof(float) * 1; // radial velocity
  fullSize += sizeof(float) * 2; // radial position
  fullSize += sizeof(float) * 1; // slide time

  fullSize += nboStdStringPackSize(deathMsg);

  return fullSize;
}


void PhysicsDriver::print(std::ostream& out, const std::string& indent) const
{
  out << indent << "physics" << std::endl;

  if (name.size() > 0) {
    out << indent << "  name " << name << std::endl;
  }

  const float* v = linear;
  if ((v[0] != 0.0f) || (v[1] != 0.0f) || (v[2] != 0.0f)) {
    out << indent << "  linear "
                  << v[0] << " " << v[1] << " " << v[2] << std::endl;
  }

  if (angularVel != 0.0f) {
    const float* ap = angularPos;
    out << indent << "  angular " << (angularVel / (M_PI * 2.0f)) << " "
			          << ap[0] << " " << ap[1] << std::endl;
  }

  if (radialVel != 0.0f) {
    const float* rp = radialPos;
    out << indent << "  radial "
                  << radialVel << " " << rp[0] << " " << rp[1] << std::endl;
  }

  if (slide) {
    out << indent << "  slide " << slideTime << std::endl;
  }

  if (death) {
    out << indent << "  death " << deathMsg << std::endl;
  }

  out << indent << "end" << std::endl << std::endl;

  return;
}


// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

