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
#include "LinkPhysics.h"

// system headers
#include <math.h>
#include <string.h>

// common headers
#include "Pack.h"


static const LinkPhysics defLinkPhysics;


//============================================================================//

LinkPhysics::LinkPhysics()
: shotSrcPosScale(1.0f, 1.0f, 1.0f)
, shotSrcVelScale(1.0f, 1.0f, 1.0f)
, shotDstVel     (0.0f, 0.0f, 0.0f)
, shotSameSpeed(false)
, tankSrcPosScale(1.0f, 1.0f, 1.0f)
, tankSrcVelScale(1.0f, 1.0f, 1.0f)
, tankDstVel     (0.0f, 0.0f, 0.0f)
, tankSameSpeed(false)
, tankAngleOffset(0.0f)
, tankAngVelOffset(0.0f)
, tankForceAngle(false)
, tankForceAngVel(false)
, tankAngle(0.0f)
, tankAngVel(0.0f)
, tankAngVelScale(1.0f)
{
  // do nothing
}


LinkPhysics::~LinkPhysics()
{
}


//============================================================================//

bool LinkPhysics::operator==(const LinkPhysics& lp) const
{
  if (shotSrcPosScale  != lp.shotSrcPosScale)  { return false; }
  if (shotSrcVelScale  != lp.shotSrcVelScale)  { return false; }
  if (shotDstVel       != lp.shotDstVel)       { return false; }
  if (shotSameSpeed    != lp.shotSameSpeed)    { return false; }
  if (tankSrcPosScale  != lp.tankSrcPosScale)  { return false; }
  if (tankSrcVelScale  != lp.tankSrcVelScale)  { return false; }
  if (tankDstVel       != lp.tankDstVel)       { return false; }
  if (tankForceAngle   != lp.tankForceAngle)   { return false; }
  if (tankForceAngVel  != lp.tankForceAngVel)  { return false; }
  if (tankAngle        != lp.tankAngle)        { return false; }
  if (tankAngleOffset  != lp.tankAngleOffset)  { return false; }
  if (tankAngVel       != lp.tankAngVel)       { return false; }
  if (tankAngVelOffset != lp.tankAngVelOffset) { return false; }
  if (tankAngVelScale  != lp.tankAngVelScale)  { return false; }
  return true;
}


bool LinkPhysics::operator<(const LinkPhysics& lp) const
{
  if (shotSrcPosScale < lp.shotSrcPosScale)  { return true;  }
  if (lp.shotSrcPosScale < shotSrcPosScale)  { return false; }

  if (shotSrcVelScale < lp.shotSrcVelScale)  { return true;  }
  if (lp.shotSrcVelScale < shotSrcVelScale)  { return false; }

  if (shotDstVel < lp.shotDstVel) { return true;  }
  if (lp.shotDstVel < shotDstVel) { return false; }

  if (!shotSameSpeed && lp.shotSameSpeed) { return true;  }
  if (shotSameSpeed && !lp.shotSameSpeed) { return false; }

  if (tankSrcPosScale < lp.tankSrcPosScale)  { return true;  }
  if (lp.tankSrcPosScale < tankSrcPosScale)  { return false; }

  if (tankSrcVelScale < lp.tankSrcVelScale)  { return true;  }
  if (lp.tankSrcVelScale < tankSrcVelScale)  { return false; }

  if (tankDstVel < lp.tankDstVel) { return true;  }
  if (lp.tankDstVel < tankDstVel) { return false; }

  if (!tankSameSpeed && lp.tankSameSpeed) { return true;  }
  if (tankSameSpeed && !lp.tankSameSpeed) { return false; }

  if (!tankForceAngle && lp.tankForceAngle)  { return true;  }
  if (tankForceAngle && !lp.tankForceAngle)  { return false; }

  if (!tankForceAngVel && lp.tankForceAngVel) { return true;  }
  if (tankForceAngVel && !lp.tankForceAngVel) { return false; }

  if (tankAngle < lp.tankAngle) { return true;  }
  if (lp.tankAngle < tankAngle) { return false; }

  if (tankAngleOffset < lp.tankAngleOffset) { return true;  }
  if (lp.tankAngleOffset < tankAngleOffset) { return false; }

  if (tankAngVel < lp.tankAngVel) { return true;  }
  if (lp.tankAngVel < tankAngVel) { return false; }

  if (tankAngVelOffset < lp.tankAngVelOffset) { return true;  }
  if (lp.tankAngVelOffset < tankAngVelOffset) { return false; }

  if (tankAngVelScale < lp.tankAngVelScale) { return true;  }
  if (lp.tankAngVelScale < tankAngVelScale) { return false; }

  return false;
}


//============================================================================//

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
  buf = nboPackFVec3(buf, shotDstVel);
  buf = nboPackFVec3(buf, tankSrcPosScale);
  buf = nboPackFVec3(buf, tankSrcVelScale);
  buf = nboPackFVec3(buf, tankDstVel);
  buf = nboPackFloat(buf, tankAngle);
  buf = nboPackFloat(buf, tankAngleOffset);
  buf = nboPackFloat(buf, tankAngVel);
  buf = nboPackFloat(buf, tankAngVelOffset);
  buf = nboPackFloat(buf, tankAngVelScale);

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
  buf = nboUnpackFVec3(buf, shotDstVel);
  buf = nboUnpackFVec3(buf, tankSrcPosScale);
  buf = nboUnpackFVec3(buf, tankSrcVelScale);
  buf = nboUnpackFVec3(buf, tankDstVel);
  buf = nboUnpackFloat(buf, tankAngle);
  buf = nboUnpackFloat(buf, tankAngleOffset);
  buf = nboUnpackFloat(buf, tankAngVel);
  buf = nboUnpackFloat(buf, tankAngVelOffset);
  buf = nboUnpackFloat(buf, tankAngVelScale);

  return buf;
}


int LinkPhysics::packSize() const
{
  int fullSize = 0;

  fullSize += sizeof(uint8_t); // state bits

  if (*this == defLinkPhysics) {
    return fullSize;
  }

  fullSize += sizeof(fvec3); // shotSrcPosScale
  fullSize += sizeof(fvec3); // shotSrcVelScale
  fullSize += sizeof(fvec3); // shotDstVel
  fullSize += sizeof(fvec3); // tankSrcPosScale
  fullSize += sizeof(fvec3); // tankSrcVelScale
  fullSize += sizeof(fvec3); // tankDstVel
  fullSize += sizeof(float); // tankAngle
  fullSize += sizeof(float); // tankAngleOffset
  fullSize += sizeof(float); // tankAngVel
  fullSize += sizeof(float); // tankAngVelOffset
  fullSize += sizeof(float); // tankAngVelScale

  return fullSize;
}


//============================================================================//

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
  if (shotDstVel != fvec3(0.0f, 0.0f, 0.0f)) {
    out << indent << "  shotDstVel " << shotDstVel.tostring() << std::endl;
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
  if (tankDstVel != fvec3(0.0f, 0.0f, 0.0f)) {
    out << indent << "  tankDstVel " << tankDstVel.tostring() << std::endl;
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

  if (tankAngVelScale != 0.0f) {
    out << indent << "  tankAngVelScale " << tankAngVelScale << std::endl;
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
