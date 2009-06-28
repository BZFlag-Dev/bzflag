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

// interface header
#include "PhysicsDriver.h"

// system headers
#include <math.h>
#include <ctype.h>
#include <string.h>

// common headers
#include "TimeKeeper.h"
#include "Pack.h"
#include "TextUtils.h"


//============================================================================//
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
  nameMap.clear();
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
  const std::string& name = driver->getName();
  if (!name.empty()) {
    if (nameMap.find(name) == nameMap.end()) {
      nameMap[name] = (int)(drivers.size());
    }
  }
  driver->id = (int)drivers.size();
  drivers.push_back(driver);
  return ((int)drivers.size() - 1);
}


int PhysicsDriverManager::findDriver(const std::string& phydrv) const
{
  if (phydrv.empty()) {
    return -1;
  }
  else if ((phydrv[0] >= '0') && (phydrv[0] <= '9')) {
    int index = atoi (phydrv.c_str());
    if ((index < 0) || (index >= (int)drivers.size())) {
      return -1;
    } else {
      return index;
    }
  }
  else {
    std::map<std::string, int>::const_iterator it = nameMap.find(phydrv);
    if (it != nameMap.end()) {
      return it->second;
    }
    return -1;
  }
}


void * PhysicsDriverManager::pack(void *buf) const
{
  std::vector<PhysicsDriver*>::const_iterator it;
  buf = nboPackUInt32(buf, (int)drivers.size());
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
  buf = nboUnpackUInt32 (buf, count);
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


//============================================================================//
//
// Physics Driver
//

PhysicsDriver::PhysicsDriver()
: name("")
, linearVel(0.0f, 0.0f, 0.0f)
, angularVel(0.0f)
, angularPos(0.0f, 0.0f)
, radialVel(0.0f)
, radialPos(0.0f, 0.0f)
, slideTime(0.0f)
, deathMsg("")
{
  // do nothing
}


PhysicsDriver::~PhysicsDriver()
{
}


bool PhysicsDriver::operator<(const PhysicsDriver& pd) const
{
  if (linearVel < pd.linearVel) { return true;  }
  if (pd.linearVel < linearVel) { return false; }

  if (angularVel < pd.angularVel) { return true;  }
  if (pd.angularVel < angularVel) { return false; }
  if (angularPos < pd.angularPos) { return true;  }
  if (pd.angularPos < angularPos) { return false; }

  if (radialVel < pd.radialVel) { return true;  }
  if (pd.radialVel < radialVel) { return false; }
  if (radialPos < pd.radialPos) { return true;  }
  if (pd.radialPos < radialPos) { return false; }

  if (slideTime < pd.slideTime) { return true;  }
  if (pd.slideTime < slideTime) { return false; }

  if (deathMsg < pd.deathMsg) { return true;  }
  if (pd.deathMsg < deathMsg) { return false; }

  return false;
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
  } else if ((drvname[0] >= '0') && (drvname[0] <= '9')) {
    name = "";
    return false;
  } else {
    name = drvname;
  }
  return true;
}


void PhysicsDriver::setLinear(const fvec3& vel)
{
  linearVel = vel;
  return;
}


void PhysicsDriver::setAngular(float vel, const fvec2& pos)
{
  // convert from (rotations/second) to (radians/second)
  angularVel = (float)(vel * (2.0 * M_PI));
  angularPos = pos;
  return;
}


void PhysicsDriver::setRadial(float vel, const fvec2& pos)
{
  radialVel = vel;
  radialPos = pos;
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
  int first = TextUtils::firstVisible(msg);
  if (first < 0) first = 0;
  const char* c = msg.c_str();
  c = TextUtils::skipWhitespace(c);
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

  buf = nboPackFVec3(buf, linearVel);
  buf = nboPackFloat(buf, angularVel);
  buf = nboPackFVec2(buf, angularPos);
  buf = nboPackFloat(buf, radialVel);
  buf = nboPackFVec2(buf, radialPos);

  buf = nboPackFloat(buf, slideTime);
  buf = nboPackStdString(buf, deathMsg);

  return buf;
}


void * PhysicsDriver::unpack(void *buf)
{
  buf = nboUnpackStdString(buf, name);

  buf = nboUnpackFVec3(buf, linearVel);
  buf = nboUnpackFloat(buf, angularVel);
  buf = nboUnpackFVec2(buf, angularPos);
  buf = nboUnpackFloat(buf, radialVel);
  buf = nboUnpackFVec2(buf, radialPos);

  buf = nboUnpackFloat(buf, slideTime);
  buf = nboUnpackStdString(buf, deathMsg);

  finalize();

  return buf;
}


int PhysicsDriver::packSize() const
{
  int fullSize = nboStdStringPackSize(name);

  fullSize += sizeof(fvec3); // linear velocity
  fullSize += sizeof(float); // angular velocity
  fullSize += sizeof(fvec2); // angular position
  fullSize += sizeof(float); // radial velocity
  fullSize += sizeof(fvec2); // radial position
  fullSize += sizeof(float); // slide time

  fullSize += nboStdStringPackSize(deathMsg);

  return fullSize;
}


void PhysicsDriver::print(std::ostream& out, const std::string& indent) const
{
  out << indent << "physics" << std::endl;

  if (name.size() > 0) {
    out << indent << "  name " << name << std::endl;
  }

  const fvec3& v = linearVel;
  if ((v.x != 0.0f) || (v.y != 0.0f) || (v.z != 0.0f)) {
    out << indent << "  linear "
	<< v.x << " " << v.y << " " << v.z << std::endl;
  }

  if (angularVel != 0.0f) {
    const fvec2& ap = angularPos;
    out << indent << "  angular " << (angularVel / (M_PI * 2.0f)) << " "
	<< ap.x << " " << ap.y << std::endl;
  }

  if (radialVel != 0.0f) {
    const fvec2& rp = radialPos;
    out << indent << "  radial "
	<< radialVel << " " << rp.x << " " << rp.y << std::endl;
  }

  if (slideTime > 0.0f) {
    out << indent << "  slide " << slideTime << std::endl;
  }

  if (!deathMsg.empty()) {
    out << indent << "  death " << deathMsg << std::endl;
  }

  out << indent << "end" << std::endl << std::endl;

  return;
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
