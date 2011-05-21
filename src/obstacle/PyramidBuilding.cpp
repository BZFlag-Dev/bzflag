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

#include "common.h"
#include <math.h>
#include "global.h"
#include "Pack.h"
#include "PyramidBuilding.h"
#include "Intersect.h"
#include "MeshTransform.h"


const char* PyramidBuilding::typeName = "PyramidBuilding";


PyramidBuilding::PyramidBuilding() {
}


PyramidBuilding::PyramidBuilding(const fvec3& p, float a,
                                 float w, float b, float h,
                                 unsigned char drive, unsigned char shoot, bool rico)
  : Obstacle(p, a, w, b, h, drive, shoot, rico) {
  finalize();
  return;
}


PyramidBuilding::~PyramidBuilding() {
  // do nothing
}


void PyramidBuilding::finalize() {
  Obstacle::setExtents();
  return;
}


Obstacle* PyramidBuilding::copyWithTransform(const MeshTransform& xform) const {
  fvec3 newPos = pos;
  fvec3 newSize = size;
  float newAngle = angle;

  MeshTransform::Tool tool(xform);
  bool flipped;
  tool.modifyOldStyle(newPos, newSize, newAngle, flipped);

  PyramidBuilding* copy =
    new PyramidBuilding(newPos, newAngle, newSize.x, newSize.y, newSize.z,
                        driveThrough, shootThrough, ricochet);

  copy->zFlip = !(getZFlip() == flipped);

  return copy;
}


const char* PyramidBuilding::getType() const {
  return typeName;
}


const char* PyramidBuilding::getClassName() { // const
  return typeName;
}


float PyramidBuilding::intersect(const Ray& r) const {
  return Intersect::timeRayHitsPyramids(r, getPosition(), getRotation(),
                                        getWidth(), getBreadth(), getHeight(),
                                        getZFlip());
}


void PyramidBuilding::getNormal(const fvec3& p, fvec3& n) const {
  // get normal in z = const plane
  const float s = shrinkFactor(p.z);
  Intersect::getNormalRect(p, getPosition(), getRotation(),
                           s * getWidth(), s * getBreadth(), n);

  // make sure we are not way above or way below it
  // above is good so we can drive on it when it's fliped
  float top =  getPosition().z + getHeight();
  float bottom = getPosition().z;

  if (s == 0) {
    if (getZFlip()) {
      if (p.z >= top) {
        n.x = n.y = 0;
        n.z = 1;
        return;
      }
    }
    else {
      if (p.z <= bottom) {
        n.x = n.y = 0;
        n.z = -1;
        return;
      }
    }
  }

  // now angle it due to slope of wall
  // FIXME -- this assumes the pyramid has a square base!
  const float h = 1.0f / hypotf(getHeight(), getWidth());
  n.x *= h * getHeight();
  n.y *= h * getHeight();
  n.z = h * getWidth();

  if (getZFlip()) {
    n.z *= -1;
  }
}


void PyramidBuilding::get3DNormal(const fvec3& p, fvec3& n) const {
  const float epsilon = ZERO_TOLERANCE;

  // get normal in z = const plane
  const float s = shrinkFactor(p.z);
  Intersect::getNormalRect(p, getPosition(), getRotation(),
                           s * getWidth(), s * getBreadth(), n);

  // make sure we are not way above or way below it
  // above is good so we can drive on it when it's flipped
  float top =  getPosition().z + getHeight();
  float bottom = getPosition().z;

  if (s == 0) {
    if (getZFlip()) {
      if (p.z >= top) {
        n.x = n.y = 0;
        n.z = 1;
        return;
      }
    }
    else {
      if (p.z <= bottom) {
        n.x = n.y = 0;
        n.z = -1;
        return;
      }
    }
  }

  if (s >= 1.0f - epsilon) {
    n.x = n.y = 0;
    if (getZFlip()) {
      n.z = 1;
    }
    else {
      n.z = -1;
    }
    return;
  }

  // now angle it due to slope of wall
  // we figure out if it was an X or Y wall that was hit
  // by checking the normal returned from getNormalRect()
  // FIXME -- fugly beyond belief
  float baseLength = getWidth();
  const float normalAngle = atan2f(n.y, n.x);
  const float rightAngle = fabsf(fmodf(normalAngle - getRotation() + (float)(M_PI / 2.0), (float)M_PI));
  if ((rightAngle < 0.1) || (rightAngle > (M_PI - 0.1))) {
    baseLength = getBreadth();
  }
  const float h = 1.0f / hypotf(getHeight(), baseLength);
  n.x *= h * getHeight();
  n.y *= h * getHeight();
  n.z  = h * baseLength;

  if (getZFlip()) {
    n.z *= -1.0f;
  }
}


bool PyramidBuilding::inCylinder(const fvec3& p,
                                 float radius, float height) const {
  // really rough -- doesn't decrease size with height
  return (p.z < (getPosition().z + getHeight()))
         && ((p.z + height) >= getPosition().z)
         &&     Intersect::testRectCircle(getPosition(), getRotation(),
                                          getWidth(), getBreadth(), p, radius);
}


bool PyramidBuilding::inBox(const fvec3& p, float a,
                            float dx, float dy, float height) const {
  // Tank is below pyramid ?
  if (p.z + height < getPosition().z) {
    return false;
  }
  // Tank is above pyramid ?
  if (p.z >= getPosition().z + getHeight()) {
    return false;
  }
  // Could be inside. Then check collision with the rectangle at object height
  // This is a rectangle reduced by shrinking but pass the height that we are
  // not so sure where collision can be
  const float s = shrinkFactor(p.z, height);
  return Intersect::testRectRect(getPosition(), getRotation(),
                                 s * getWidth(), s * getBreadth(), p, a, dx, dy);
}


bool PyramidBuilding::inMovingBox(const fvec3&, float,
                                  const fvec3& p, float _angle,
                                  float dx, float dy, float dz) const {
  return inBox(p, _angle, dx, dy, dz);
}


bool PyramidBuilding::isCrossing(const fvec3& p, float a,
                                 float dx, float dy,
                                 float height, fvec4* planePtr) const {
  // if not inside or contained then not crossing
  if (!inBox(p, a, dx, dy, height) ||
      Intersect::testRectInRect(getPosition(), getRotation(),
                                getWidth(), getBreadth(), p, a, dx, dy)) {
    return false;
  }

  if (!planePtr) {
    return true;
  }
  fvec4& plane = *planePtr;

  // it's crossing -- choose which wall is being crossed (this
  // is a guestimate, should really do a careful test).  just
  // see which wall the point is closest to.
  const fvec3& p2 = getPosition();
  const float a2 = getRotation();
  const float c = cosf(-a2), s = sinf(-a2);
  const float x = c * (p.x - p2.x) - s * (p.y - p2.y);
  const float y = c * (p.y - p2.y) + s * (p.x - p2.x);
  fvec2 pw;
  if (fabsf(fabsf(x) - getWidth()) < fabsf(fabsf(y) - getBreadth())) {
    plane.x = ((x < 0.0) ? -cosf(a2) : cosf(a2));
    plane.y = ((x < 0.0) ? -sinf(a2) : sinf(a2));
    pw.x = p2.x + getWidth() * plane.x;
    pw.y = p2.y + getWidth() * plane.y;
  }
  else {
    plane.x = ((y < 0.0) ? sinf(a2) : -sinf(a2));
    plane.y = ((y < 0.0) ? -cosf(a2) : cosf(a2));
    pw.x = p2.x + getBreadth() * plane.x;
    pw.y = p2.y + getBreadth() * plane.y;
  }

  // now finish off plane equation (FIXME -- assumes a square base)
  const float h = 1.0f / hypotf(getHeight(), getWidth());
  plane.x *= h * getHeight();
  plane.y *= h * getHeight();
  plane.z = h * getWidth();
  plane.w = -(plane.x * pw.x + plane.y * pw.y);
  return true;
}


bool PyramidBuilding::getHitNormal(const fvec3& pos1, float,
                                   const fvec3& pos2, float,
                                   float, float, float height,
                                   fvec3& normal) const {
  // pyramids height and flipping
  // normalize height sign and report that in flip
  float oHeight = getHeight();
  bool  flip    = getZFlip();
  if (oHeight < 0) {
    flip    = !flip;
    oHeight = -oHeight;
  }

  // get Bottom and Top of building
  float oBottom = getPosition().z;
  float oTop    = oBottom + oHeight;

  // get higher and lower point of base of colliding object
  float objHigh = pos1.z;
  float objLow  = pos2.z;
  if (objHigh < objLow) {
    float temp = objHigh;
    objHigh    = objLow;
    objLow     = temp;
  }

  normal.x = normal.y = 0;
  if (flip && objHigh >= oTop) {
    // base of higher object is over the plateau
    normal.z = 1;
    return true;
  }
  else if (!flip && objLow + height < oBottom) {
    // top of lower object is below the base
    normal.z = -1;
    return true;
  }

  // get normal in z = const plane
  const float s = shrinkFactor(pos1.z, height);

  Intersect::getNormalRect(pos1, getPosition(), getRotation(),
                           s * getWidth(), s * getBreadth(), normal);

  // now angle it due to slope of wall
  // FIXME -- this assumes the pyramid has a square base!
  const float h = 1.0f / hypotf(oHeight, getWidth());
  normal.x *= h * oHeight;
  normal.y *= h * oHeight;
  normal.z  = h * getWidth();

  if (flip) {
    normal.z = -normal.z;
  }
  return true;
}


void PyramidBuilding::getCorner(int index, fvec3& _pos) const {
  const fvec3& base = getPosition();
  const float top = getHeight() + base.z;

  fvec2 polarity;

  switch (index) {
    case 4: {
      _pos.xy() = base.xy();
      _pos.z = getZFlip() ? base.z : top;
      return;
    }
    case 0: { polarity = fvec2(+1.0f, +1.0f); break; }
    case 1: { polarity = fvec2(-1.0f, +1.0f); break; }
    case 2: { polarity = fvec2(-1.0f, -1.0f); break; }
    case 3: { polarity = fvec2(+1.0f, -1.0f); break; }
    default: {
      return;
    }
  }
  const float w = polarity.x * getWidth();
  const float h = polarity.y * getBreadth();
  const float c = cosf(getRotation());
  const float s = sinf(getRotation());
  _pos.x = base.x + (c * w) - (s * h);
  _pos.y = base.y + (s * w) + (c * h);
  _pos.z = getZFlip() ? top : base.z;
}


float PyramidBuilding::shrinkFactor(float z, float height) const {
  float shrink;

  // Normalize Height and flip to have height > 0
  float oHeight = getHeight();
  bool  flip    = getZFlip();
  if (oHeight < 0) {
    flip    = !flip;
    oHeight = - oHeight;
  }

// Remove heights bias
  const fvec3& _pos = getPosition();
  z -= _pos.z;
  if (oHeight <= ZERO_TOLERANCE) {
    shrink = 1.0f;
  }
  else {
    // Normalize heights
    z /= oHeight;

    // if flipped the bigger intersection is at top of the object
    if (flip) {
      // Normalize the object height, we have not done yet
      z += height / oHeight;
    }

    // shrink is that
    if (flip) {
      shrink = z;
    }
    else {
      shrink = 1.0f - z;
    }
  }

  // clamp in 0 .. 1
  if (shrink < 0.0) {
    shrink = 0.0;
  }
  else if (shrink > 1.0) {
    shrink = 1.0;
  }

  return shrink;
}


bool PyramidBuilding::isFlatTop() const {
  return getZFlip();
}


void* PyramidBuilding::pack(void* buf) const {
  buf = nboPackFVec3(buf, pos);
  buf = nboPackFloat(buf, angle);
  buf = nboPackFVec3(buf, size);

  unsigned char stateByte = 0;
  stateByte |= isDriveThrough() ? _DRIVE_THRU : 0;
  stateByte |= isShootThrough() ? _SHOOT_THRU : 0;
  stateByte |= getZFlip()       ? _FLIP_Z     : 0;
  stateByte |= canRicochet()    ? _RICOCHET   : 0;
  buf = nboPackUInt8(buf, stateByte);

  return buf;
}


void* PyramidBuilding::unpack(void* buf) {
  buf = nboUnpackFVec3(buf, pos);
  buf = nboUnpackFloat(buf, angle);
  buf = nboUnpackFVec3(buf, size);

  unsigned char stateByte;
  buf = nboUnpackUInt8(buf, stateByte);
  driveThrough = (stateByte & _DRIVE_THRU) != 0 ? 0xFF : 0;
  shootThrough = (stateByte & _SHOOT_THRU) != 0 ? 0xFF : 0;
  zFlip        = (stateByte & _FLIP_Z)     != 0;
  ricochet     = (stateByte & _RICOCHET)   != 0;

  finalize();

  return buf;
}


int PyramidBuilding::packSize() const {
  int fullSize = 0;
  fullSize += sizeof(fvec3);   // pos
  fullSize += sizeof(fvec3);   // size
  fullSize += sizeof(float);   // rotation
  fullSize += sizeof(uint8_t); // state bits
  return fullSize;
}


void PyramidBuilding::print(std::ostream& out, const std::string& indent) const {
  out << indent << "pyramid" << std::endl;
  const fvec3& _pos = getPosition();
  out << indent << "  position " << _pos.x << " " << _pos.y << " "
      << _pos.z << std::endl;
  out << indent << "  size " << getWidth() << " " << getBreadth()
      << " " << getHeight() << std::endl;
  out << indent << "  rotation " << (getRotation() * RAD2DEGf)
      << std::endl;
  if (getZFlip()) {
    out << indent << "  flipz" << std::endl;
  }

  if (isPassable()) {
    out << indent << "  passable" << std::endl;
  }
  else {
    if (isDriveThrough()) {
      out << indent << "  drivethrough" << std::endl;
    }
    if (isShootThrough()) {
      out << indent << "  shootthrough" << std::endl;
    }
  }
  if (canRicochet()) {
    out << indent << "  ricochet" << std::endl;
  }
  out << indent << "end" << std::endl << std::endl;
  return;
}


static void outputFloat(std::ostream& out, float value) {
  char buffer[32];
  snprintf(buffer, 30, " %.8f", value);
  out << buffer;
  return;
}

void PyramidBuilding::printOBJ(std::ostream& out, const std::string& /*indent*/) const {
  int i;
  fvec3 verts[5] = {
    fvec3(-1.0f, -1.0f, 0.0f),
    fvec3(+1.0f, -1.0f, 0.0f),
    fvec3(+1.0f, +1.0f, 0.0f),
    fvec3(-1.0f, +1.0f, 0.0f),
    fvec3(0.0f,  0.0f, 1.0f)
  };
  const float sqrt1_2 = (float)M_SQRT1_2;
  fvec3 norms[5] = {
    fvec3(0.0f, -sqrt1_2, +sqrt1_2), fvec3(+sqrt1_2, 0.0f, +sqrt1_2),
    fvec3(0.0f, +sqrt1_2, +sqrt1_2), fvec3(-sqrt1_2, 0.0f, +sqrt1_2),
    fvec3(0.0f, 0.0f, -1.0f)
  };
  const fvec3& s = getSize();
  const float k = 1.0f / 8.0f;
  fvec2 txcds[7] = {
    fvec2(0.0f, 0.0f), fvec2(k* s.x, 0.0f), fvec2(k* s.x, k* s.y),
    fvec2(0.0f, k* s.y), fvec2(0.5f * k* s.x, k* sqrtf(s.x* s.x + s.z* s.z)),
    fvec2(k* s.y, 0.0f), fvec2(0.5f * k* s.y, k* sqrtf(s.y* s.y + s.z* s.z))
  };
  MeshTransform xform;
  const float degrees = getRotation() * RAD2DEGf;
  const fvec3 zAxis(0.0f, 0.0f, +1.0f);
  if (getZFlip()) {
    const fvec3 xAxis(1.0f, 0.0f, 0.0f);
    xform.addSpin(180.0f, xAxis);
    xform.addShift(zAxis);
  }
  xform.addScale(getSize());
  xform.addSpin(degrees, zAxis);
  xform.addShift(getPosition());
  xform.finalize();
  MeshTransform::Tool xtool(xform);
  for (i = 0; i < 5; i++) {
    xtool.modifyVertex(verts[i]);
  }
  for (i = 0; i < 5; i++) {
    xtool.modifyNormal(norms[i]);
  }

  out << "# OBJ - start pyramid" << std::endl;
  out << "o bzpyr_" << getObjCounter() << std::endl;

  for (i = 0; i < 5; i++) {
    out << "v";
    outputFloat(out, verts[i].x);
    outputFloat(out, verts[i].y);
    outputFloat(out, verts[i].z);
    out << std::endl;
  }
  for (i = 0; i < 7; i++) {
    out << "vt";
    outputFloat(out, txcds[i].x);
    outputFloat(out, txcds[i].y);
    out << std::endl;
  }
  for (i = 0; i < 5; i++) {
    out << "vn";
    outputFloat(out, norms[i].x);
    outputFloat(out, norms[i].y);
    outputFloat(out, norms[i].z);
    out << std::endl;
  }
  out << "usemtl pyrwall" << std::endl;
  out << "f -1/-1/-5 -5/-7/-5 -4/-6/-5" << std::endl;
  out << "f -1/-3/-4 -4/-7/-4 -3/-2/-4" << std::endl;
  out << "f -1/-1/-3 -3/-7/-3 -2/-6/-3" << std::endl;
  out << "f -1/-3/-2 -2/-7/-2 -5/-2/-2" << std::endl;
  out << "f -2/-7/-1 -3/-6/-1 -4/-5/-1 -5/-4/-1" << std::endl;

  out << std::endl;

  incObjCounter();

  return;
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=2 tabstop=8
