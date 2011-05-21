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

// interface header
#include "LinkPhysics.h"

// system headers
#include <math.h>
#include <string.h>

// common headers
#include "Pack.h"


static const LinkPhysics defLinkPhysics;


//============================================================================//

LinkPhysics::LinkPhysics()
: testBits(0)
, shotSrcPosScale (1.0f, 1.0f, 1.0f)
, shotSrcVelScale (1.0f, 1.0f, 1.0f)
, shotDstVelOffset(0.0f, 0.0f, 0.0f)
, shotSameSpeed(false)
, tankSrcPosScale (1.0f, 1.0f, 1.0f)
, tankSrcVelScale (1.0f, 1.0f, 1.0f)
, tankDstVelOffset(0.0f, 0.0f, 0.0f)
, tankSameSpeed(false)
, tankForceAngle(false)
, tankForceAngVel(false)
, tankAngle(0.0f)
, tankAngleOffset(0.0f)
, tankAngleScale(1.0f)
, tankAngVel(0.0f)
, tankAngVelOffset(0.0f)
, tankAngVelScale(1.0f)
, shotMinSpeed(0.0f)
, shotMaxSpeed(0.0f)
, tankMinSpeed(0.0f)
, tankMaxSpeed(0.0f)
, shotMinAngle(0.0f)
, shotMaxAngle(0.0f)
, tankMinAngle(0.0f)
, tankMaxAngle(0.0f)
, shotBlockTeams(0)
, tankBlockTeams(0)
, shotBlockVar("")
, tankBlockVar("")
, shotPassText("")
, tankPassText("")
, tankPassSound("")
{
  // do nothing
}


LinkPhysics::~LinkPhysics()
{
}


//============================================================================//

void LinkPhysics::finalize()
{
  testBits = 0;

  if ((shotMinSpeed != 0.0f) || (shotMaxSpeed != 0.0f)) {
    testBits |= ShotSpeedTest;
  }
  if ((tankMinSpeed != 0.0f) || (tankMaxSpeed != 0.0f)) {
    testBits |= TankSpeedTest;
  }
  if ((shotMinAngle != 0.0f) || (shotMaxAngle != 0.0f)) {
    testBits |= ShotAngleTest;
  }
  if ((tankMinAngle != 0.0f) || (tankMaxAngle != 0.0f)) {
    testBits |= TankAngleTest;
  }

  if (shotBlockTeams != 0)     { testBits |= ShotTeamTest; }
  if (tankBlockTeams != 0)     { testBits |= TankTeamTest; }
  if (!shotBlockFlags.empty()) { testBits |= ShotFlagTest; }
  if (!tankBlockFlags.empty()) { testBits |= TankFlagTest; }
  if (!shotBlockVar.empty())   { testBits |= ShotVarTest; }
  if (!tankBlockVar.empty())   { testBits |= TankVarTest; }
}


//============================================================================//

bool LinkPhysics::operator==(const LinkPhysics& lp) const
{
  if (shotSrcPosScale  != lp.shotSrcPosScale)  { return false; }
  if (shotSrcVelScale  != lp.shotSrcVelScale)  { return false; }
  if (shotDstVelOffset != lp.shotDstVelOffset) { return false; }
  if (shotSameSpeed    != lp.shotSameSpeed)    { return false; }
  if (tankSrcPosScale  != lp.tankSrcPosScale)  { return false; }
  if (tankSrcVelScale  != lp.tankSrcVelScale)  { return false; }
  if (tankDstVelOffset != lp.tankDstVelOffset) { return false; }
  if (tankForceAngle   != lp.tankForceAngle)   { return false; }
  if (tankForceAngVel  != lp.tankForceAngVel)  { return false; }
  if (tankAngle        != lp.tankAngle)        { return false; }
  if (tankAngVel       != lp.tankAngVel)       { return false; }
  if (tankAngleOffset  != lp.tankAngleOffset)  { return false; }
  if (tankAngleScale   != lp.tankAngleScale)   { return false; }
  if (tankAngVelOffset != lp.tankAngVelOffset) { return false; }
  if (tankAngVelScale  != lp.tankAngVelScale)  { return false; }
  if (shotMinSpeed     != lp.shotMinSpeed)     { return false; }
  if (shotMaxSpeed     != lp.shotMaxSpeed)     { return false; }
  if (tankMinSpeed     != lp.tankMinSpeed)     { return false; }
  if (tankMaxSpeed     != lp.tankMaxSpeed)     { return false; }
  if (shotMinAngle     != lp.shotMinAngle)     { return false; }
  if (shotMaxAngle     != lp.shotMaxAngle)     { return false; }
  if (tankMinAngle     != lp.tankMinAngle)     { return false; }
  if (tankMaxAngle     != lp.tankMaxAngle)     { return false; }
  if (shotBlockTeams   != lp.shotBlockTeams)   { return false; }
  if (tankBlockTeams   != lp.tankBlockTeams)   { return false; }
  if (shotBlockFlags   != lp.shotBlockFlags)   { return false; }
  if (tankBlockFlags   != lp.tankBlockFlags)   { return false; }
  if (shotBlockVar     != lp.shotBlockVar)     { return false; }
  if (tankBlockVar     != lp.tankBlockVar)     { return false; }
  if (shotPassText     != lp.shotPassText)     { return false; }
  if (tankPassText     != lp.tankPassText)     { return false; }
  if (tankPassSound    != lp.tankPassSound)    { return false; }
  return true;
}


bool LinkPhysics::operator<(const LinkPhysics& lp) const
{
  if (shotSrcPosScale < lp.shotSrcPosScale)  { return true;  }
  if (lp.shotSrcPosScale < shotSrcPosScale)  { return false; }

  if (shotSrcVelScale < lp.shotSrcVelScale)  { return true;  }
  if (lp.shotSrcVelScale < shotSrcVelScale)  { return false; }

  if (shotDstVelOffset < lp.shotDstVelOffset) { return true;  }
  if (lp.shotDstVelOffset < shotDstVelOffset) { return false; }

  if (!shotSameSpeed && lp.shotSameSpeed) { return true;  }
  if (shotSameSpeed && !lp.shotSameSpeed) { return false; }

  if (tankSrcPosScale < lp.tankSrcPosScale)  { return true;  }
  if (lp.tankSrcPosScale < tankSrcPosScale)  { return false; }

  if (tankSrcVelScale < lp.tankSrcVelScale)  { return true;  }
  if (lp.tankSrcVelScale < tankSrcVelScale)  { return false; }

  if (tankDstVelOffset < lp.tankDstVelOffset) { return true;  }
  if (lp.tankDstVelOffset < tankDstVelOffset) { return false; }

  if (!tankSameSpeed && lp.tankSameSpeed) { return true;  }
  if (tankSameSpeed && !lp.tankSameSpeed) { return false; }

  if (!tankForceAngle && lp.tankForceAngle)  { return true;  }
  if (tankForceAngle && !lp.tankForceAngle)  { return false; }

  if (!tankForceAngVel && lp.tankForceAngVel) { return true;  }
  if (tankForceAngVel && !lp.tankForceAngVel) { return false; }

  if (tankAngle < lp.tankAngle) { return true;  }
  if (lp.tankAngle < tankAngle) { return false; }

  if (tankAngVel < lp.tankAngVel) { return true;  }
  if (lp.tankAngVel < tankAngVel) { return false; }

  if (tankAngleOffset < lp.tankAngleOffset) { return true;  }
  if (lp.tankAngleOffset < tankAngleOffset) { return false; }

  if (tankAngleScale < lp.tankAngleScale) { return true;  }
  if (lp.tankAngleScale < tankAngleScale) { return false; }

  if (tankAngVelOffset < lp.tankAngVelOffset) { return true;  }
  if (lp.tankAngVelOffset < tankAngVelOffset) { return false; }

  if (tankAngVelScale < lp.tankAngVelScale) { return true;  }
  if (lp.tankAngVelScale < tankAngVelScale) { return false; }

  if (shotMinSpeed < lp.shotMinSpeed) { return true;  }
  if (lp.shotMinSpeed < shotMinSpeed) { return false; }

  if (shotMaxSpeed < lp.shotMaxSpeed) { return true;  }
  if (lp.shotMaxSpeed < shotMaxSpeed) { return false; }

  if (tankMinSpeed < lp.tankMinSpeed) { return true;  }
  if (lp.tankMinSpeed < tankMinSpeed) { return false; }

  if (tankMaxSpeed < lp.tankMaxSpeed) { return true;  }
  if (lp.tankMaxSpeed < tankMaxSpeed) { return false; }

  if (shotMinAngle < lp.shotMinAngle) { return true;  }
  if (lp.shotMinAngle < shotMinAngle) { return false; }

  if (shotMaxAngle < lp.shotMaxAngle) { return true;  }
  if (lp.shotMaxAngle < shotMaxAngle) { return false; }

  if (tankMinAngle < lp.tankMinAngle) { return true;  }
  if (lp.tankMinAngle < tankMinAngle) { return false; }

  if (tankMaxAngle < lp.tankMaxAngle) { return true;  }
  if (lp.tankMaxAngle < tankMaxAngle) { return false; }

  if (shotBlockTeams < lp.shotBlockTeams) { return true;  }
  if (lp.shotBlockTeams < shotBlockTeams) { return false; }

  if (tankBlockTeams < lp.tankBlockTeams) { return true;  }
  if (lp.tankBlockTeams < tankBlockTeams) { return false; }

  if (shotBlockFlags < lp.shotBlockFlags) { return true;  }
  if (lp.shotBlockFlags < shotBlockFlags) { return false; }

  if (tankBlockFlags < lp.tankBlockFlags) { return true;  }
  if (lp.tankBlockFlags < tankBlockFlags) { return false; }

  if (shotBlockVar < lp.shotBlockVar) { return true;  }
  if (lp.shotBlockVar < shotBlockVar) { return false; }

  if (tankBlockVar < lp.tankBlockVar) { return true;  }
  if (lp.tankBlockVar < tankBlockVar) { return false; }

  if (shotPassText < lp.shotPassText) { return true;  }
  if (lp.shotPassText < shotPassText) { return false; }

  if (tankPassText < lp.tankPassText) { return true;  }
  if (lp.tankPassText < tankPassText) { return false; }

  if (tankPassSound < lp.tankPassSound) { return true;  }
  if (lp.tankPassSound < tankPassSound) { return false; }

  return false;
}


//============================================================================//

int LinkPhysics::packSize() const
{
  int fullSize = 0;

  fullSize += sizeof(uint8_t); // state bits

  if (*this == defLinkPhysics) {
    return fullSize;
  }

  fullSize += sizeof(fvec3); // shotSrcPosScale
  fullSize += sizeof(fvec3); // shotSrcVelScale
  fullSize += sizeof(fvec3); // shotDstVelOffset

  fullSize += sizeof(fvec3); // tankSrcPosScale
  fullSize += sizeof(fvec3); // tankSrcVelScale
  fullSize += sizeof(fvec3); // tankDstVelOffset

  fullSize += sizeof(float); // tankAngle
  fullSize += sizeof(float); // tankAngVel
  fullSize += sizeof(float); // tankAngleOffset
  fullSize += sizeof(float); // tankAngleScale
  fullSize += sizeof(float); // tankAngVelOffset
  fullSize += sizeof(float); // tankAngVelScale

  fullSize += sizeof(float); // shotMinSpeed
  fullSize += sizeof(float); // shotMaxSpeed
  fullSize += sizeof(float); // tankMinSpeed
  fullSize += sizeof(float); // tankMaxSpeed

  fullSize += sizeof(float); // shotMinAngle
  fullSize += sizeof(float); // shotMaxAngle
  fullSize += sizeof(float); // tankMinAngle
  fullSize += sizeof(float); // tankMaxAngle

  fullSize += sizeof(uint8_t); // shotBlockTeams
  fullSize += sizeof(uint8_t); // tankBlockTeams

  std::set<std::string>::const_iterator it;

  fullSize += sizeof(uint16_t); // shotBlockFlags count
  for (it = shotBlockFlags.begin(); it != shotBlockFlags.end(); ++it) {
    fullSize += nboStdStringPackSize(*it);
  }

  fullSize += sizeof(uint16_t); // tankBlockFlags count
  for (it = tankBlockFlags.begin(); it != tankBlockFlags.end(); ++it) {
    fullSize += nboStdStringPackSize(*it);
  }

  fullSize += nboStdStringPackSize(shotBlockVar);
  fullSize += nboStdStringPackSize(tankBlockVar);

  fullSize += nboStdStringPackSize(shotPassText);
  fullSize += nboStdStringPackSize(tankPassText);
  fullSize += nboStdStringPackSize(tankPassSound);

  return fullSize;
}


void* LinkPhysics::pack(void* buf) const
{
  uint8_t bits = 0;
  if (*this == defLinkPhysics) {
    buf = nboPackUInt8(buf, bits);
    return buf;
  }

  bits = (1 << 0);
  bits |= shotSameSpeed   ? (1 << 1) : 0;
  bits |= tankSameSpeed   ? (1 << 2) : 0;
  bits |= tankForceAngle  ? (1 << 3) : 0;
  bits |= tankForceAngVel ? (1 << 4) : 0;
  buf = nboPackUInt8(buf, bits);

  buf = nboPackFVec3(buf, shotSrcPosScale);
  buf = nboPackFVec3(buf, shotSrcVelScale);
  buf = nboPackFVec3(buf, shotDstVelOffset);

  buf = nboPackFVec3(buf, tankSrcPosScale);
  buf = nboPackFVec3(buf, tankSrcVelScale);
  buf = nboPackFVec3(buf, tankDstVelOffset);

  buf = nboPackFloat(buf, tankAngle);
  buf = nboPackFloat(buf, tankAngVel);
  buf = nboPackFloat(buf, tankAngleOffset);
  buf = nboPackFloat(buf, tankAngleScale);
  buf = nboPackFloat(buf, tankAngVelOffset);
  buf = nboPackFloat(buf, tankAngVelScale);

  buf = nboPackFloat(buf, shotMinSpeed);
  buf = nboPackFloat(buf, shotMaxSpeed);
  buf = nboPackFloat(buf, tankMinSpeed);
  buf = nboPackFloat(buf, tankMaxSpeed);

  buf = nboPackFloat(buf, shotMinAngle);
  buf = nboPackFloat(buf, shotMaxAngle);
  buf = nboPackFloat(buf, tankMinAngle);
  buf = nboPackFloat(buf, tankMaxAngle);

  buf = nboPackUInt8(buf, shotBlockTeams);
  buf = nboPackUInt8(buf, tankBlockTeams);

  uint16_t count;
  std::set<std::string>::const_iterator it;

  count = (uint16_t)shotBlockFlags.size();
  buf = nboPackUInt16(buf, count);
  for (it = shotBlockFlags.begin(); it != shotBlockFlags.end(); ++it) {
    buf = nboPackStdString(buf, *it);
  }

  count = (uint16_t)tankBlockFlags.size();
  buf = nboPackUInt16(buf, count);
  for (it = tankBlockFlags.begin(); it != tankBlockFlags.end(); ++it) {
    buf = nboPackStdString(buf, *it);
  }

  buf = nboPackStdString(buf, shotBlockVar);
  buf = nboPackStdString(buf, tankBlockVar);

  buf = nboPackStdString(buf, shotPassText);
  buf = nboPackStdString(buf, tankPassText);
  buf = nboPackStdString(buf, tankPassSound);

  return buf;
}


void* LinkPhysics::unpack(void* buf)
{
  uint8_t bits;
  buf = nboUnpackUInt8(buf, bits);
  if (bits == 0) {
    return buf;
  }
  shotSameSpeed   = (bits & (1 << 1)) != 0;
  tankSameSpeed   = (bits & (1 << 2)) != 0;
  tankForceAngle  = (bits & (1 << 3)) != 0;
  tankForceAngVel = (bits & (1 << 4)) != 0;

  buf = nboUnpackFVec3(buf, shotSrcPosScale);
  buf = nboUnpackFVec3(buf, shotSrcVelScale);
  buf = nboUnpackFVec3(buf, shotDstVelOffset);

  buf = nboUnpackFVec3(buf, tankSrcPosScale);
  buf = nboUnpackFVec3(buf, tankSrcVelScale);
  buf = nboUnpackFVec3(buf, tankDstVelOffset);

  buf = nboUnpackFloat(buf, tankAngle);
  buf = nboUnpackFloat(buf, tankAngVel);
  buf = nboUnpackFloat(buf, tankAngleOffset);
  buf = nboUnpackFloat(buf, tankAngleScale);
  buf = nboUnpackFloat(buf, tankAngVelOffset);
  buf = nboUnpackFloat(buf, tankAngVelScale);

  buf = nboUnpackFloat(buf, shotMinSpeed);
  buf = nboUnpackFloat(buf, shotMaxSpeed);
  buf = nboUnpackFloat(buf, tankMinSpeed);
  buf = nboUnpackFloat(buf, tankMaxSpeed);

  buf = nboUnpackFloat(buf, shotMinAngle);
  buf = nboUnpackFloat(buf, shotMaxAngle);
  buf = nboUnpackFloat(buf, tankMinAngle);
  buf = nboUnpackFloat(buf, tankMaxAngle);

  buf = nboUnpackUInt8(buf, shotBlockTeams);
  buf = nboUnpackUInt8(buf, tankBlockTeams);

  uint16_t count;
  std::string flag;

  buf = nboUnpackUInt16(buf, count);
  for (uint16_t i = 0; i < count; i++) {
    buf = nboUnpackStdString(buf, flag);
    shotBlockFlags.insert(flag);
  }

  buf = nboUnpackUInt16(buf, count);
  for (uint16_t i = 0; i < count; i++) {
    buf = nboUnpackStdString(buf, flag);
    tankBlockFlags.insert(flag);
  }

  buf = nboUnpackStdString(buf, shotBlockVar);
  buf = nboUnpackStdString(buf, tankBlockVar);

  buf = nboUnpackStdString(buf, shotPassText);
  buf = nboUnpackStdString(buf, tankPassText);
  buf = nboUnpackStdString(buf, tankPassSound);

  return buf;
}


//============================================================================//

static void outputBits(std::ostream& out, uint8_t bits)
{
  for (int i = 0; i < 8; i++) {
    if ((bits & (1 << i)) != 0) {
      out << " " << i;
    }
  }
}


static void outputStringSet(std::ostream& out,
                            const std::set<std::string>& strings)
{
  std::set<std::string>::const_iterator it;
  for (it = strings.begin(); it != strings.end(); ++it) {
    out << " " << *it;
  }
}


void LinkPhysics::print(std::ostream& out, const std::string& indent) const
{
  // shot data
  if (shotSrcPosScale != fvec3(1.0f, 1.0f, 1.0f)) {
    out << indent << "  shotSrcPosScale "
                  << shotSrcPosScale.tostring() << std::endl;
  }
  if (shotSrcVelScale != fvec3(1.0f, 1.0f, 1.0f)) {
    out << indent << "  shotSrcVelScale "
                  << shotSrcVelScale.tostring() << std::endl;
  }
  if (shotDstVelOffset != fvec3(0.0f, 0.0f, 0.0f)) {
    out << indent << "  shotDstVelOffset "
                  << shotDstVelOffset.tostring() << std::endl;
  }
  if (shotSameSpeed) {
    out << indent << "  shotSameSpeed" << std::endl;
  }

  // tank data
  if (tankSrcPosScale != fvec3(1.0f, 1.0f, 1.0f)) {
    out << indent << "  tankSrcPosScale "
                  << tankSrcPosScale.tostring() << std::endl;
  }
  if (tankSrcVelScale != fvec3(1.0f, 1.0f, 1.0f)) {
    out << indent << "  tankSrcVelScale "
                  << tankSrcVelScale.tostring() << std::endl;
  }
  if (tankDstVelOffset != fvec3(0.0f, 0.0f, 0.0f)) {
    out << indent << "  tankDstVelOffset "
                  << tankDstVelOffset.tostring() << std::endl;
  }
  if (tankSameSpeed) {
    out << indent << "  tankSameSpeed" << std::endl;
  }

  // tank angular data
  if (tankForceAngle)  {
    out << indent << "  tankAngle "
                  << (tankAngle * RAD2DEGf)  << std::endl;
  }
  if (tankForceAngVel) {
    out << indent << "  tankAngVel "
                  << (tankAngVel * RAD2DEGf) << std::endl;
  }

  if (tankAngleOffset != 0.0f) {
    out << indent << "  tankAngleOffset "
                  << (tankAngleOffset * RAD2DEGf) << std::endl;
  }
  if (tankAngVelOffset != 0.0f) {
    out << indent << "  tankAngVelOffset "
                  << (tankAngleOffset * RAD2DEGf) << std::endl;
  }

  if (tankAngleScale != 1.0f) {
    out << indent << "  tankAngleScale " << tankAngleScale << std::endl;
  }
  if (tankAngVelScale != 1.0f) {
    out << indent << "  tankAngVelScale " << tankAngVelScale << std::endl;
  }

  // speed blocks
  if (shotMinSpeed != 0.0f) {
    out << indent << "  shotMinSpeed " << shotMinSpeed << std::endl;
  }

  if (shotMaxSpeed != 0.0f) {
    out << indent << "  shotMaxSpeed " << shotMaxSpeed << std::endl;
  }

  if (tankMinSpeed != 0.0f) {
    out << indent << "  tankMinSpeed " << tankMinSpeed << std::endl;
  }

  if (tankMaxSpeed != 0.0f) {
    out << indent << "  tankMaxSpeed " << tankMaxSpeed << std::endl;
  }

  // angle blocks
  if (shotMinAngle != 0.0f) {
    out << indent << "  shotMinAngle " << shotMinAngle * RAD2DEGf << std::endl;
  }

  if (shotMaxAngle != 0.0f) {
    out << indent << "  shotMaxAngle " << shotMaxAngle * RAD2DEGf << std::endl;
  }

  if (tankMinAngle != 0.0f) {
    out << indent << "  tankMinAngle " << tankMinAngle * RAD2DEGf << std::endl;
  }

  if (tankMaxAngle != 0.0f) {
    out << indent << "  tankMaxAngle " << tankMaxAngle * RAD2DEGf << std::endl;
  }

  // team blocks
  if (shotBlockTeams != 0) {
    out << indent << "  shotBlockTeams";
    outputBits(out, shotBlockTeams);
    out << std::endl;
  }

  if (tankBlockTeams != 0) {
    out << indent << "  tankBlockTeams";
    outputBits(out, tankBlockTeams);
    out << std::endl;
  }

  // flag blocks
  if (!shotBlockFlags.empty()) {
    out << indent << "  shotBlockFlags";
    outputStringSet(out, shotBlockFlags);
    out << std::endl;
  }

  if (!tankBlockFlags.empty()) {
    out << indent << "  tankBlockFlags";
    outputStringSet(out, tankBlockFlags);
    out << std::endl;
  }

  // bzdb blocks
  if (!shotBlockVar.empty()) {
    out << indent << "  shotBlockVar " << shotBlockVar << std::endl;
  }

  if (!tankBlockVar.empty()) {
    out << indent << "  tankBlockVar " << tankBlockVar << std::endl;
  }

  // pass messages
  if (!shotPassText.empty()) {
    out << indent << "  shotPassText " << shotPassText << std::endl;
  }

  if (!tankPassText.empty()) {
    out << indent << "  tankPassText " << tankPassText << std::endl;
  }

  if (!tankPassSound.empty()) {
    out << indent << "  tankPassSound " << tankPassSound << std::endl;
  }

  return;
}


//============================================================================//


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
