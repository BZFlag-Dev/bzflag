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
#include "StateDatabase.h"
#include "MeshTransform.h"


//============================================================================//
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
  DriverVec::iterator it;
  for (it = drivers.begin(); it != drivers.end(); it++) {
    delete *it;
  }
  drivers.clear();
  driverSet.clear();
  nameMap.clear();
  return;
}


void PhysicsDriverManager::update()
{
  float t = (float)(TimeKeeper::getCurrent() - TimeKeeper::getStartTime());
  DriverVec::iterator it;
  for (it = drivers.begin(); it != drivers.end(); it++) {
    PhysicsDriver* driver = *it;
    driver->update(t);
  }
  return;
}


int PhysicsDriverManager::addDriver(PhysicsDriver* driver)
{
  const std::string name = driver->getName();

  if (name == "-1") {
    delete driver;
    return -1;
  }

  bool replaced = false;
  DriverSet::iterator it = driverSet.find(driver);
  if (it != driverSet.end()) {
    delete driver;
    driver = *it;
    replaced = true;
  }

  if (!name.empty()) {
    if (nameMap.find(name) == nameMap.end()) {
      nameMap[name] = driver;
    }
  }

  if (!replaced) {
    driver->id = (int)drivers.size();
    drivers.push_back(driver);
    driverSet.insert(driver);
  }

  return driver->getID();
}


const PhysicsDriver* PhysicsDriverManager::relativeDriver(
                       const PhysicsDriver* phydrv,
                       const MeshTransform::Tool& tool)
{
  PhysicsDriver withXForm(*phydrv);
  withXForm.xformTool = const_cast<MeshTransform::Tool*>(&tool);
  withXForm.transformLinearVel();
  withXForm.transformAngularPos();
  withXForm.transformRadialPos();

  const DriverSet::const_iterator it = driverSet.find(&withXForm);

  // make it safe for destruction
  withXForm.xformTool = NULL;
  withXForm.linearVar.clear();
  withXForm.angularVar.clear();
  withXForm.radialVar.clear();
  withXForm.slideVar.clear();
  withXForm.deathVar.clear();

  if (it != driverSet.end()) {
    return *it;
  }

  PhysicsDriver* copy = new PhysicsDriver(*phydrv);
  copy->addCallbacks();
  copy->xformTool = new MeshTransform::Tool(tool);
  copy->transformLinearVel();
  copy->transformAngularPos();
  copy->transformRadialPos();

  copy->id = (int)drivers.size();
  drivers.push_back(copy);
  driverSet.insert(copy);

  return copy;
}
                                            

int PhysicsDriverManager::findDriver(const std::string& phydrv) const
{
  if (phydrv.empty()) {
    return -1;
  }
  else if (phydrv == "-1") {
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
    NameMap::const_iterator it = nameMap.find(phydrv);
    if (it != nameMap.end()) {
      return it->second->getID();
    }
    return -1;
  }
}


void PhysicsDriverManager::getVariables(std::set<std::string>& vars) const
{
  DriverVec::const_iterator it;
  for (it = drivers.begin(); it != drivers.end(); it++) {
    PhysicsDriver* driver = *it;
    if (!driver->linearVar.empty())  { vars.insert(driver->linearVar);  }
    if (!driver->angularVar.empty()) { vars.insert(driver->angularVar); }
    if (!driver->radialVar.empty())  { vars.insert(driver->radialVar);  }
    if (!driver->slideVar.empty())   { vars.insert(driver->slideVar);   }
    if (!driver->deathVar.empty())   { vars.insert(driver->deathVar);   }
  }
}


int PhysicsDriverManager::packSize() const
{
  int fullSize = sizeof (uint32_t);
  DriverVec::const_iterator it;
  for (it = drivers.begin(); it != drivers.end(); it++) {
    PhysicsDriver* driver = *it;
    if (driver->xformTool == NULL) {
      fullSize = fullSize + driver->packSize();
    }
  }
  return fullSize;
}


void* PhysicsDriverManager::pack(void *buf) const
{
  uint32_t count = 0;
  DriverVec::const_iterator it;
  for (it = drivers.begin(); it != drivers.end(); it++) {
    PhysicsDriver* driver = *it;
    if (driver->xformTool == NULL) {
      count++;
    }
  }

  buf = nboPackUInt32(buf, count);
  for (it = drivers.begin(); it != drivers.end(); it++) {
    PhysicsDriver* driver = *it;
    if (driver->xformTool == NULL) {
      buf = driver->pack(buf);
    }
  }
  return buf;
}


void* PhysicsDriverManager::unpack(void *buf)
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


void PhysicsDriverManager::print(std::ostream& out,
				 const std::string& indent) const
{
  DriverVec::const_iterator it;
  for (it = drivers.begin(); it != drivers.end(); it++) {
    const PhysicsDriver* driver = *it;
    if ((driver->xformTool == NULL) || (debugLevel >= 5)) {
      driver->print(out, indent);
    }
  }
  return;
}


//============================================================================//
//============================================================================//
//
// Physics Driver
//

PhysicsDriver::PhysicsDriver()
: name("")
, id(-1)
, relative   (false)
, linearVel  (0.0f, 0.0f, 0.0f)
, angularVel (0.0f)
, angularPos (0.0f, 0.0f)
, radialVel  (0.0f)
, radialPos  (0.0f, 0.0f)
, slideTime  (0.0f)
, deathMsg   ("")
, linearVar  ("")
, angularVar ("")
, radialVar  ("")
, slideVar   ("")
, deathVar   ("")
, xformTool  (NULL)
{
  // do nothing
}


PhysicsDriver::PhysicsDriver(const PhysicsDriver& pd)
: name("")
, id(-1)
, relative   (pd.relative)
, linearVel  (pd.linearVel)
, angularVel (pd.angularVel)
, angularPos (pd.angularPos)
, radialVel  (pd.radialVel)
, radialPos  (pd.radialPos)
, slideTime  (pd.slideTime)
, deathMsg   (pd.deathMsg)
, linearVar  (pd.linearVar)
, angularVar (pd.angularVar)
, radialVar  (pd.radialVar)
, slideVar   (pd.slideVar)
, deathVar   (pd.deathVar)
, xformTool  (NULL)
{
  // do nothing
}


PhysicsDriver::~PhysicsDriver()
{
  delete xformTool;

  if (!linearVar.empty()) {
    BZDB.removeCallback(linearVar, staticLinearCallback, this);
  }
  if (!angularVar.empty()) {
    BZDB.removeCallback(angularVar, staticAngularCallback, this);
  }
  if (!radialVar.empty()) {
    BZDB.removeCallback(radialVar, staticRadialCallback, this);
  }
  if (!slideVar.empty()) {
    BZDB.removeCallback(slideVar, staticSlideCallback, this);
  }
  if (!deathVar.empty()) {
    BZDB.removeCallback(deathVar, staticDeathCallback, this);
  }
}


bool PhysicsDriver::operator<(const PhysicsDriver& pd) const
{
  if ((xformTool == NULL) && (pd.xformTool != NULL)) { return true;  }
  if ((xformTool != NULL) && (pd.xformTool == NULL)) { return false; }

  if (xformTool != NULL) {
    if (*xformTool < *pd.xformTool) { return true;  }
    if (*pd.xformTool < *xformTool) { return false; }
  }

  if (!relative && pd.relative) { return true;  }
  if (relative && !pd.relative) { return false; }

  if (linearVel < pd.linearVel) { return true;  }
  if (pd.linearVel < linearVel) { return false; }
  if (linearVar < pd.linearVar) { return true;  }
  if (pd.linearVar < linearVar) { return false; }

  if (angularVel < pd.angularVel) { return true;  }
  if (pd.angularVel < angularVel) { return false; }
  if (angularPos < pd.angularPos) { return true;  }
  if (pd.angularPos < angularPos) { return false; }
  if (angularVar < pd.angularVar) { return true;  }
  if (pd.angularVar < angularVar) { return false; }

  if (radialVel < pd.radialVel) { return true;  }
  if (pd.radialVel < radialVel) { return false; }
  if (radialPos < pd.radialPos) { return true;  }
  if (pd.radialPos < radialPos) { return false; }
  if (radialVar < pd.radialVar) { return true;  }
  if (pd.radialVar < radialVar) { return false; }

  if (slideTime < pd.slideTime) { return true;  }
  if (pd.slideTime < slideTime) { return false; }
  if (slideVar < pd.slideVar)   { return true;  }
  if (pd.slideVar < slideVar)   { return false; }

  if (deathMsg < pd.deathMsg) { return true;  }
  if (pd.deathMsg < deathMsg) { return false; }
  if (deathVar < pd.deathVar) { return true;  }
  if (pd.deathVar < deathVar) { return false; }

  return false;
}


//============================================================================//

void PhysicsDriver::staticLinearCallback(const std::string& name, void* data)
{
  ((PhysicsDriver*)data)->linearCallback(name);
}
void PhysicsDriver::staticAngularCallback(const std::string& name, void* data)
{
  ((PhysicsDriver*)data)->angularCallback(name);
}
void PhysicsDriver::staticRadialCallback(const std::string& name, void* data)
{
  ((PhysicsDriver*)data)->radialCallback(name);
}
void PhysicsDriver::staticSlideCallback(const std::string& name, void* data)
{
  ((PhysicsDriver*)data)->slideCallback(name);
}
void PhysicsDriver::staticDeathCallback(const std::string& name, void* data)
{
  ((PhysicsDriver*)data)->deathCallback(name);
}


void PhysicsDriver::linearCallback(const std::string& /*varName*/)
{
  const fvec3 values = BZDB.evalFVec3(linearVar);
  if (!isnan(values.x)) {
    linearVel = values;
    transformLinearVel();
  }
}


void PhysicsDriver::angularCallback(const std::string& /*varName*/)
{
  const fvec3 values = BZDB.evalFVec3(angularVar);
  if (!isnan(values.x)) {
    angularVel = values.x * float(M_PI * 2.0);
    angularPos = values.yz();
    transformAngularPos();
    }
  else {
    const float value = BZDB.eval(angularVar);
    if (!isnan(value)) {
      angularVel = value * float(M_PI * 2.0);
    }
  }
}


void PhysicsDriver::radialCallback(const std::string& /*varName*/)
{
  const fvec3 values = BZDB.evalFVec3(radialVar);
  if (!isnan(values.x)) {
    radialVel = values.x;
    radialPos = values.yz();
    transformRadialPos();
  }
  else {
    const float value = BZDB.eval(radialVar);
    if (!isnan(value)) {
      radialVel = value;
    }
  }
}


void PhysicsDriver::slideCallback(const std::string& /*varName*/)
{
  const float value = BZDB.eval(slideVar);
  if (!isnan(value)) {
    slideTime = value;
  }
}


void PhysicsDriver::deathCallback(const std::string& /*varName*/)
{
  deathMsg = BZDB.get(deathVar);
}


void PhysicsDriver::addCallbacks()
{
  const std::string tmpLinearVar  = linearVar;
  const std::string tmpAngularVar = angularVar;
  const std::string tmpRadialVar  = radialVar;
  const std::string tmpSlideVar   = slideVar;
  const std::string tmpDeathVar   = deathVar;
  linearVar.clear();
  angularVar.clear();
  radialVar.clear();
  slideVar.clear();
  deathVar.clear();
  setLinearVar(tmpLinearVar);
  setAngularVar(tmpAngularVar);
  setRadialVar(tmpRadialVar);
  setSlideVar(tmpSlideVar);
  setDeathVar(tmpDeathVar);
}


//============================================================================//

void PhysicsDriver::finalize()
{
  return;
}


//============================================================================//

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


void PhysicsDriver::setRelative(bool value)
{
  relative = value;
  return;
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


//============================================================================//

void PhysicsDriver::setLinearVar(const std::string& var)
{
  if (!linearVar.empty()) {
    BZDB.removeCallback(linearVar, staticLinearCallback, this);
  }
  linearVar = var;
  if (!linearVar.empty()) {
    BZDB.addCallback(linearVar, staticLinearCallback, this);
  }
  linearCallback(linearVar);
}


void PhysicsDriver::setAngularVar(const std::string& var)
{
  if (!angularVar.empty()) {
    BZDB.removeCallback(angularVar, staticAngularCallback, this);
  }
  angularVar = var;
  if (!angularVar.empty()) {
    BZDB.addCallback(angularVar, staticAngularCallback, this);
  }
  angularCallback(angularVar);
}


void PhysicsDriver::setRadialVar(const std::string& var)
{
  if (!radialVar.empty()) {
    BZDB.removeCallback(radialVar, staticRadialCallback, this);
  }
  radialVar = var;
  if (!radialVar.empty()) {
    BZDB.addCallback(radialVar, staticRadialCallback, this);
  }
  radialCallback(radialVar);
}


void PhysicsDriver::setSlideVar(const std::string& var)
{
  if (!slideVar.empty()) {
    BZDB.removeCallback(slideVar, staticSlideCallback, this);
  }
  slideVar = var;
  if (!slideVar.empty()) {
    BZDB.addCallback(slideVar, staticSlideCallback, this);
  }
  slideCallback(slideVar);
}


void PhysicsDriver::setDeathVar(const std::string& var)
{
  if (!deathVar.empty()) {
    BZDB.removeCallback(deathVar, staticDeathCallback, this);
  }
  deathVar = var;
  if (!deathVar.empty()) {
    BZDB.addCallback(deathVar, staticDeathCallback, this);
  }
  deathCallback(deathVar);
}


//============================================================================//

void PhysicsDriver::update (float /*t*/)
{
  return;
}


void PhysicsDriver::transformLinearVel()
{
  if (xformTool == NULL) {
    return;
  }
  const float speed = linearVel.length();
  xformTool->modifyNormal(linearVel);
  linearVel = linearVel.normalize() * speed;
}


void PhysicsDriver::transformAngularPos()
{
  if (xformTool == NULL) {
    return;
  }
  fvec3 pos(angularPos.x, angularPos.y, 0.0f);
  xformTool->modifyVertex(pos);
  angularPos = pos.xy();
}


void PhysicsDriver::transformRadialPos()
{
  if (xformTool == NULL) {
    return;
  }
  fvec3 pos(radialPos.x, radialPos.y, 0.0f);
  xformTool->modifyVertex(pos);
  radialPos = pos.xy();
}

            
//============================================================================//

int PhysicsDriver::packSize() const
{
  int fullSize = 0;
  
  fullSize += nboStdStringPackSize(name);

  fullSize += sizeof(int8_t); // bits

  fullSize += sizeof(fvec3); // linearVel
  fullSize += nboStdStringPackSize(linearVar);
  fullSize += sizeof(float); // angularVel
  fullSize += sizeof(fvec2); // angularPos
  fullSize += nboStdStringPackSize(angularVar);
  fullSize += sizeof(float); // radialVel
  fullSize += sizeof(fvec2); // radialPos
  fullSize += nboStdStringPackSize(radialVar);
  fullSize += sizeof(float); // slideTime
  fullSize += nboStdStringPackSize(slideVar);
  fullSize += nboStdStringPackSize(deathMsg);
  fullSize += nboStdStringPackSize(deathVar);

  return fullSize;
}


void* PhysicsDriver::pack(void *buf) const
{
  buf = nboPackStdString(buf, name);

  int8_t bits = 0;
  bits |= relative ? 1 : 0;
  buf = nboPackInt8(buf, bits);

  std::string varName;

  buf = nboPackFVec3    (buf, linearVel);
  buf = nboPackStdString(buf, linearVar);

  buf = nboPackFloat    (buf, angularVel);
  buf = nboPackFVec2    (buf, angularPos);
  buf = nboPackStdString(buf, angularVar);

  buf = nboPackFloat    (buf, radialVel);
  buf = nboPackFVec2    (buf, radialPos);
  buf = nboPackStdString(buf, radialVar);

  buf = nboPackFloat    (buf, slideTime);
  buf = nboPackStdString(buf, slideVar);

  buf = nboPackStdString(buf, deathMsg);
  buf = nboPackStdString(buf, deathVar);

  return buf;
}


void* PhysicsDriver::unpack(void *buf)
{
  buf = nboUnpackStdString(buf, name);

  int8_t bits;
  buf = nboUnpackInt8(buf, bits);
  relative = (bits & (1 << 0)) != 0;

  std::string varName;

  buf = nboUnpackFVec3    (buf, linearVel);
  buf = nboUnpackStdString(buf, varName);
  setLinearVar(varName);

  buf = nboUnpackFloat    (buf, angularVel);
  buf = nboUnpackFVec2    (buf, angularPos);
  buf = nboUnpackStdString(buf, varName);
  setAngularVar(varName);

  buf = nboUnpackFloat    (buf, radialVel);
  buf = nboUnpackFVec2    (buf, radialPos);
  buf = nboUnpackStdString(buf, varName);
  setRadialVar(varName);

  buf = nboUnpackFloat    (buf, slideTime);
  buf = nboUnpackStdString(buf, varName);
  setSlideVar(varName);

  buf = nboUnpackStdString(buf, deathMsg);
  buf = nboUnpackStdString(buf, varName);
  setDeathVar(varName);

  finalize();

  return buf;
}


void PhysicsDriver::print(std::ostream& out, const std::string& indent) const
{
  out << indent << "physics" << std::endl;

  if (name.size() > 0) {
    out << indent << "  name " << name << std::endl;
  }

  if (relative) {
    out << indent << "  relative" << std::endl;
  }

  if (linearVel != fvec3(0.0f, 0.0f, 0.0f)) {
    out << indent << "  linear " << linearVel << std::endl;
  }
  if (!linearVar.empty()) {
    out << indent << "  linearVar " << linearVar << std::endl;
  }

  if (angularVel != 0.0f) {
    out << indent << "  angular "
                  << (angularVel / (M_PI * 2.0f)) << " "
                  << angularPos << std::endl;
  }    
  if (!angularVar.empty()) {
    out << indent << "  angularVar " << angularVar << std::endl;
  }

  if (radialVel != 0.0f) {
    const fvec2& rp = radialPos;
    out << indent << "  radial "
	<< radialVel << " " << rp.x << " " << rp.y << std::endl;
  }
  if (!radialVar.empty()) {
    out << indent << "  radialVar " << radialVar << std::endl;
  }

  if (slideTime > 0.0f) {
    out << indent << "  slide " << slideTime << std::endl;
  }
  if (!slideVar.empty()) {
    out << indent << "  slideVar " << slideVar << std::endl;
  }

  if (!deathMsg.empty()) {
    out << indent << "  death " << deathMsg << std::endl;
  }
  if (!deathVar.empty()) {
    out << indent << "  deathVar " << deathVar << std::endl;
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
