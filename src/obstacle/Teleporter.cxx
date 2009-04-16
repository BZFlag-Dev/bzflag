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

#include "common.h"
#include <math.h>
#include "global.h"
#include "Pack.h"
#include "Teleporter.h"
#include "Intersect.h"
#include "MeshTransform.h"


const char* Teleporter::typeName = "Teleporter";


Teleporter::Teleporter()
{
  backLink = NULL;
  frontLink = NULL;
  return;
}


Teleporter::Teleporter(const fvec3& p, float a, float w,
		       float b, float h, float _border, bool _horizontal,
		       unsigned char drive, unsigned char shoot, bool rico)
: Obstacle(p, a, w, b, h, drive, shoot, rico)
, border(_border), horizontal(_horizontal)
{
  finalize();
  return;
}


Teleporter::~Teleporter()
{
  delete backLink;
  delete frontLink;
  return;
}


Obstacle* Teleporter::copyWithTransform(const MeshTransform& xform) const
{
  fvec3 newPos = pos;
  fvec3 newSize = size;
  float newAngle = angle;

  MeshTransform::Tool tool(xform);
  bool flipped;
  tool.modifyOldStyle(newPos, newSize, newAngle, flipped);

  Teleporter* copy =
    new Teleporter(newPos, newAngle, newSize[0], newSize[1], newSize[2],
		   border, horizontal, driveThrough, shootThrough, ricochet);

  copy->setName(name);

  return copy;
}


void Teleporter::finalize()
{
  origSize = size;

  if (!horizontal) {
    size[1] = origSize[1] + (border * 2.0f);
    size[2] = origSize[2] + border;
  }

  // the same as the default Obstacle::getExtents(), except
  // that we use the larger of the border half-width and size[0].
  float sizeX = border * 0.5f;
  if (size[0] > sizeX) {
    sizeX = size[0];
  }
  float xspan = (fabsf(cosf(angle)) * sizeX) + (fabsf(sinf(angle)) * size[1]);
  float yspan = (fabsf(cosf(angle)) * size[1]) + (fabsf(sinf(angle)) * sizeX);
  extents.mins[0] = pos[0] - xspan;
  extents.maxs[0] = pos[0] + xspan;
  extents.mins[1] = pos[1] - yspan;
  extents.maxs[1] = pos[1] + yspan;
  extents.mins[2] = pos[2];
  extents.maxs[2] = pos[2] + size[2];

  makeLinks();

  return;
}


void Teleporter::makeLinks()
{
  int i;

  // make the new pointers to floats,
  // the MeshFace will delete[] them
  const fvec3** fvrts  = new const fvec3*[4];
  const fvec3** bvrts  = new const fvec3*[4];
  const fvec2** ftxcds = new const fvec2*[4];
  const fvec2** btxcds = new const fvec2*[4];
  for (i = 0; i < 4; i++) {
    fvrts[i]  = &fvertices[i];
    bvrts[i]  = &bvertices[i];
    ftxcds[i] = &texcoords[i];
    btxcds[i] = &texcoords[i];
  }

  // get the basics
  const fvec3& p  = getPosition();
  const float  a  = getRotation();
  const float  w  = getWidth();
  const float  b  = getBreadth();
  const float  br = getBorder();
  const float  h  = getHeight();

  // setup the texcoord coordinates
  const float xtxcd = 1.0f;
  float ytxcd;
  if ((b - br) > 0.0f) {
    ytxcd = h / (2.0f * (b - br));
  } else {
    ytxcd = 1.0f;
  }
  texcoords[0].x = 0.0f;
  texcoords[0].y = 0.0f;
  texcoords[1].x = xtxcd;
  texcoords[1].y = 0.0f;
  texcoords[2].x = xtxcd;
  texcoords[2].y = ytxcd;
  texcoords[3].x = 0.0f;
  texcoords[3].y = ytxcd;

  const float cos_val = cosf(a);
  const float sin_val = sinf(a);

  if (!horizontal) {
    const float params[4][2] =
      {{-1.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {-1.0f, 1.0f}};
    float wlen[2] = { (cos_val * w), (sin_val * w) };
    float blen[2] = { (-sin_val * (b - br)), (cos_val * (b - br)) };

    for (i = 0; i < 4 ;i++) {
      bvertices[i].x = p.x + (wlen[0] + (blen[0] * params[i][0]));
      bvertices[i].y = p.y + (wlen[1] + (blen[1] * params[i][0]));
      bvertices[i].z = p.z + ((h - br) * params[i][1]);
    }
    backLink = new MeshFace(NULL, 4, bvrts, NULL, btxcds,
			    NULL, -1, false, false, true, true, false);

    for (i = 0; i < 4 ;i++) {
      fvertices[i].x = p.x - (wlen[0] + (blen[0] * params[i][0]));
      fvertices[i].y = p.y - (wlen[1] + (blen[1] * params[i][0]));
      fvertices[i].z = p.z + ((h - br) * params[i][1]);
    }
    frontLink = new MeshFace(NULL, 4, fvrts, NULL, ftxcds,
			     NULL, -1, false, false, true, true, false);
  }
  else {
    float xlen = w - br;
    float ylen = b - br;
    bvertices[0].x = p.x + ((cos_val * xlen) - (sin_val * ylen));
    bvertices[0].y = p.y + ((cos_val * ylen) + (sin_val * xlen));
    bvertices[0].z = p.z + h - br;
    bvertices[1].x = p.x + ((cos_val * xlen) - (sin_val * -ylen));
    bvertices[1].y = p.y + ((cos_val * -ylen) + (sin_val * xlen));
    bvertices[1].z = p.z + h - br;
    bvertices[2].x = p.x + ((cos_val * -xlen) - (sin_val * -ylen));
    bvertices[2].y = p.y + ((cos_val * -ylen) + (sin_val * -xlen));
    bvertices[2].z = p.z + h - br;
    bvertices[3].x = p.x + ((cos_val * -xlen) - (sin_val * ylen));
    bvertices[3].y = p.y + ((cos_val * ylen) + (sin_val * -xlen));
    bvertices[3].z = p.z + h - br;
    backLink = new MeshFace(NULL, 4, bvrts, NULL, btxcds,
			    NULL, -1, false, false, true, true, false);

    for (i = 0; i < 4; i++) {
      fvertices[i] = bvertices[3 - i];
      fvertices[i].z = p.z + h; // change the height
    }
    frontLink = new MeshFace(NULL, 4, fvrts, NULL, ftxcds,
			     NULL, -1, false, false, true, true, false);
  }

  return;
}


const char* Teleporter::getType() const
{
  return typeName;
}


const char* Teleporter::getClassName() // const
{
  return typeName;
}


bool Teleporter::isValid() const
{
  if (!backLink->isValid() || !frontLink->isValid()) {
    return false;
  }
  return Obstacle::isValid();
}


float Teleporter::intersect(const Ray& r) const
{
  // expand to include border
  return Intersect::timeRayHitsBlock(r, getPosition(), getRotation(),
                                     getWidth(), getBreadth(), getHeight());
}


void Teleporter::getNormal(const fvec3& p1, fvec3& n) const
{
  // get normal to closest border column (assume column is circular)
  const fvec3& p2 = getPosition();
  const float c = cosf(-getRotation());
  const float s = sinf(-getRotation());
  const float b = 0.5f * getBorder();
  const float d = getBreadth() - b;
  const float j = (c * (p1[1] - p2[1]) + s * (p1[0] - p2[0]) > 0.0f) ? d : -d;
  fvec3 cc;
  cc[0] = p2[0] + (s * j);
  cc[1] = p2[1] + (c * j);
  Intersect::getNormalRect(p1, cc, getRotation(), b, b, n);
}


bool Teleporter::inCylinder(const fvec3& p, float radius, float height) const
{
  return (p[2]+height) >= getPosition()[2] &&
	  p[2] <= getPosition()[2] + getHeight() &&
	  Intersect::testRectCircle(getPosition(), getRotation(),
	                            getWidth(), getBreadth(), p, radius);
}


bool Teleporter::inBox(const fvec3& p, float a,
		       float dx, float dy, float dz) const
{
  const float tankTop = p[2] + dz;
  const float teleTop = getExtents().maxs[2];
  const float crossbarBottom = teleTop - getBorder();

  if ((p[2] < crossbarBottom) && (tankTop >= getPosition()[2])) {
    // test individual border columns
    const float c = cosf(getRotation());
    const float s = sinf(getRotation());
    const float d = getBreadth() - (0.5f * getBorder());
    const float r = 0.5f * getBorder();
    fvec3 o;
    o[0] = getPosition()[0] - (s * d);
    o[1] = getPosition()[1] + (c * d);
    if (Intersect::testRectRect(p, a, dx, dy, o, getRotation(), r, r)) {
      return true;
    }
    o[0] = getPosition()[0] + (s * d);
    o[1] = getPosition()[1] - (c * d);
    if (Intersect::testRectRect(p, a, dx, dy, o, getRotation(), r, r)) {
      return true;
    }
  }

  if ((p[2] < teleTop) && (tankTop >= crossbarBottom)) {
    // test crossbar
    if (Intersect::testRectRect(p, a, dx, dy, getPosition(),
		                getRotation(), getWidth(), getBreadth())) {
      return true;
    }
  }

  return false;
}


bool Teleporter::inMovingBox(const fvec3& oldP, float /*oldAngle */,
			     const fvec3& p, float a,
			     float dx, float dy, float dz) const
{
  fvec3 minPos;
  minPos[0] = p[0];
  minPos[1] = p[1];
  if (oldP[2] < p[2]) {
    minPos[2] = oldP[2];
  } else {
    minPos[2] = p[2];
  }
  dz += fabsf(oldP[2] - p[2]);
  return inBox(minPos, a, dx, dy, dz);
}


bool Teleporter::isCrossing(const fvec3& p, float a,
			    float dx, float dy, float /* dz */,
			    fvec4* planePtr) const
{
  // if not inside or contained then not crossing
  const fvec3& p2 = getPosition();
  if (!Intersect::testRectRect(p, a, dx, dy,
		               p2, getRotation(), getWidth(), getBreadth() - getBorder())
      || (p[2] < p2[2]) || (p[2] > (p2[2] + getHeight() - getBorder()))) {
    return false;
  }

  if (!planePtr) {
    return true;
  }
  fvec4& plane = *planePtr;

  // it's crossing -- choose which wall is being crossed (this
  // is a guestimate, should really do a careful test).  just
  // see which wall the point is closest to.
  const float a2 = getRotation();
  const float c = cosf(-a2);
  const float s = sinf(-a2);
  const float x = c * (p[0] - p2[0]) - s * (p[1] - p2[1]);
  float pw[2];
  plane[0] = ((x < 0.0f) ? -cosf(a2) : cosf(a2));
  plane[1] = ((x < 0.0f) ? -sinf(a2) : sinf(a2));
  pw[0] = p2[0] + getWidth() * plane[0];
  pw[1] = p2[1] + getWidth() * plane[1];

  // now finish off plane equation
  plane[2] = 0.0f;
  plane[3] = -(plane[0] * pw[0] + plane[1] * pw[1]);
  return true;
}


// return true if ray goes through teleporting part
float Teleporter::isTeleported(const Ray& r, int& face) const
{
  // get t's for teleporter with and without border
  const float tb = intersect(r);
  const float t = Intersect::timeRayHitsBlock(r, getPosition(),
                                              getRotation(), getWidth(),
                                              getBreadth() - getBorder(),
                                              getHeight() - getBorder());

  // if intersection with border is before one without then doesn't teleport
  // (cos it hit the border first).  also no teleport if no intersection.
  if ((tb >= 0.0f && t - tb > 1e-6) || t < 0.0f)
    return -1.0f;

  // get teleport position.  if above or below teleporter then no teleportation.
  fvec3 p;
  r.getPoint(t, p);
  p[2] -= getPosition()[2];
  if (p[2] < 0.0f || p[2] > getHeight() - getBorder()) {
    return -1.0f;
  }

  // figure out which face:  rotate intersection into teleporter space,
  //	if to east of teleporter then face 0 else face 1.
  const float x = cosf(-getRotation()) * (p[0] - getPosition()[0]) -
		sinf(-getRotation()) * (p[1] - getPosition()[1]);
  face = (x > 0.0f) ? 0 : 1;
  return t;
}


float Teleporter::getProximity(const fvec3& p, float radius) const
{
  // make sure tank is sufficiently close
  if (!Intersect::testRectCircle(getPosition(), getRotation(),
                                 getWidth(), getBreadth() - getBorder(),
                                 p, 1.2f * radius)) {
    return 0.0f;
  }

  // transform point to teleporter space
  // translate origin
  fvec3 pa = p - getPosition();

  // make sure not too far above or below teleporter
  if (pa.z < -1.2f * radius ||
      pa.z > getHeight() - getBorder() + 1.2f * radius)
    return 0.0f;

  // rotate and reflect into first quadrant
  const float c = cosf(-getRotation()), s = sinf(-getRotation());
  const float x = fabsf((c * pa.x) - (s * pa.y));
  const float y = fabsf((c * pa.y) + (s * pa.x));

  // get proximity to face
  float t = 1.2f - x / radius;

  // if along side then trail off as point moves away from faces
  if (y > getBreadth() - getBorder()) {
    float f = (float)(2.0 / M_PI) * atan2f(x, y - getBreadth() + getBorder());
    t *= f * f;
  }
  else if (pa.z < 0.0f) {
    float f = 1.0f + pa.z / (1.2f * radius);
    if (f >= 0.0f && f <= 1.0f) t *= f * f;
  }
  else if (pa.z > getHeight() - getBorder()) {
    float f = 1.0f - (pa.z - getHeight() + getBorder()) / (1.2f * radius);
    if (f >= 0.0f && f <= 1.0f) t *= f * f;
  }

  return t > 0.0f ? (t > 1.0f ? 1.0f : t) : 0.0f;
}


bool Teleporter::hasCrossed(const fvec3& p1, const fvec3& p2, int& f) const
{
  // check above/below teleporter
  const fvec3& p = getPosition();
  if ((p1.z < p.z && p2.z < p.z) ||
	(p1.z > p.z + getHeight() - getBorder() &&
	 p2.z > p.z + getHeight() - getBorder()))
    return false;

  const float c = cosf(-getRotation()), s = sinf(-getRotation());
  const float x1 = c * (p1[0] - p[0]) - s * (p1[1] - p[1]);
  const float x2 = c * (p2[0] - p[0]) - s * (p2[1] - p[1]);
  const float y2 = c * (p2[1] - p[1]) + s * (p2[0] - p[0]);
  if (x1 * x2 < 0.0f && fabsf(y2) <= getBreadth() - getBorder()) {
    f = (x1 > 0.0f) ? 0 : 1;
    return true;
  }
  return false;
}


void Teleporter::getPointWRT(const Teleporter& t2, int face1, int face2,
			     const fvec3& pIn, const fvec3* dInPtr, float aIn,
			     fvec3& pOut, fvec3* dOutPtr, float* aOutPtr) const
{
  const float x1 = pIn.x - getPosition().x;
  const float y1 = pIn.y - getPosition().y;
  const float a = t2.getRotation() - getRotation() +
			(face1 == face2 ? (float)M_PI : 0.0f);
  const float c = cosf(a);
  const float s = sinf(a);
  const float x2 = (c * x1) - (s * y1);
  const float y2 = (c * y1) + (s * x1);

  /*
	Here's what the next statements do:

  In order to account for different-size teleporters, each of the dimensions
  is expressed a a ratio of the length of the transporter in the dimension
  divided by position of the tank relative to the transporter in that dimension, and
  is proportional to the width of the target teleporter in that dimension.
  Here is the formula, with example lengths:

  W1/W2 = T1/T2


  |--------|	Tank Pos (T1)
  |----------------------------------------------------------|	Transport width (W1)


  |-|	New tank Pos (T2)
  |---------|	New Transport width (W2)

  We are looking for T2, and simple algebra tells us that T2 = (W2 * T1) / W1

  Now, we can correctly position the tank.

  Note that this is only the position relative to the transporter, to get the real position,
  it is added to the rest.  Since I'm not 100% sure of my work, I am leaving the old code
  commented above.
  */

  //T1 = x2 and y2
  //W2 = t2.getWidth()
  //W1 = getWidth()

  //pOut[0] = x2 + t2.getPosition()[0];
  //pOut[1] = y2 + t2.getPosition()[1];
  pOut.x = t2.getPosition().x + (x2 * (t2.getBreadth() - t2.getBorder())) / (getBreadth() - getBorder());
  pOut.y = t2.getPosition().y + (y2 * (t2.getBreadth() - t2.getBorder())) / (getBreadth() - getBorder());
  //T1 = pIn[2] - getPosition()[2]
  //W2 = t2.getHeight()
  //W1 = getHeight

  //(t2.getPosition()[2] - getPosition()[2]) adds the height differences between the
  //teleporters so that teleporters can be off of the ground at different heights.

  //pOut[2] = pIn[2] + t2.getPosition()[2] - getPosition()[2];
  pOut.z = t2.getPosition().z
	  + ((pIn.z - getPosition().z) * (t2.getHeight() - t2.getBorder())) / (getHeight() - getBorder());

  if (dOutPtr && dInPtr) {
    const float dx = dInPtr->x;
    const float dy = dInPtr->y;
    dOutPtr->x = (c * dx) - (s * dy);
    dOutPtr->y = (c * dy) + (s * dx);
    dOutPtr->z = dInPtr->z;
  }

  if (aOutPtr) {
    *aOutPtr = aIn + a;
  }
}


bool Teleporter::getHitNormal(const fvec3& pos1, float azimuth1,
			      const fvec3& pos2, float azimuth2,
			      float width, float breadth, float,
			      fvec3& normal) const
{
  return Obstacle::getHitNormal(pos1, azimuth1,
			pos2, azimuth2, width, breadth,
			getPosition(), getRotation(), getWidth(), getBreadth(),
			getHeight(), normal) >= 0.0f;
}


void* Teleporter::pack(void* buf) const
{
  buf = nboPackStdString(buf, name);

  buf = nboPackFVec3(buf, pos);
  buf = nboPackFloat(buf, angle);
  buf = nboPackFVec3(buf, origSize);
  buf = nboPackFloat(buf, border);

  unsigned char horizontalByte = horizontal ? 1 : 0;
  buf = nboPackUInt8(buf, horizontalByte);

  unsigned char stateByte = 0;
  stateByte |= isDriveThrough() ? _DRIVE_THRU : 0;
  stateByte |= isShootThrough() ? _SHOOT_THRU : 0;
  stateByte |= canRicochet()    ? _RICOCHET   : 0;
  buf = nboPackUInt8(buf, stateByte);

  return buf;
}


void* Teleporter::unpack(void* buf)
{
  buf = nboUnpackStdString(buf, name);

  buf = nboUnpackFVec3(buf, pos);
  buf = nboUnpackFloat(buf, angle);
  buf = nboUnpackFVec3(buf, size);
  buf = nboUnpackFloat(buf, border);

  unsigned char horizontalByte;
  buf = nboUnpackUInt8(buf, horizontalByte);
  horizontal = (horizontalByte == 0) ? false : true;

  unsigned char stateByte;
  buf = nboUnpackUInt8(buf, stateByte);
  driveThrough = ((stateByte & _DRIVE_THRU) != 0) ? 0xFF : 0;
  shootThrough = ((stateByte & _SHOOT_THRU) != 0) ? 0xFF : 0;
  ricochet     = ((stateByte & _RICOCHET) != 0);

  finalize();

  return buf;
}


int Teleporter::packSize() const
{
  int fullSize = 0;
  fullSize += nboStdStringPackSize(name);
  fullSize += sizeof(fvec3);   // pos
  fullSize += sizeof(float);   // rotation
  fullSize += sizeof(fvec3);   // size
  fullSize += sizeof(float);   // border
  fullSize += sizeof(uint8_t); // horizontal
  fullSize += sizeof(uint8_t); // state bits
  return fullSize;
}


void Teleporter::print(std::ostream& out, const std::string& indent) const
{
  out << indent << "teleporter";
  if (name.size() > 0) {
    out << " " << name;
  }
  out << std::endl;
  const fvec3& _pos = getPosition();
  out << indent << "  position " << _pos[0] << " " << _pos[1] << " "
				 << _pos[2] << std::endl;
  out << indent << "  size " << origSize[0] << " " << origSize[1] << " "
			     << origSize[2] << std::endl;
  out << indent << "  rotation " << ((getRotation() * 180.0) / M_PI)
				 << std::endl;
  out << indent << "  border " << getBorder() << std::endl;
  if (horizontal) {
    out << indent << "  horizontal" << std::endl;
  }
  if (ricochet) {
    out << indent << "  ricochet" << std::endl;
  }
  out << indent << "end" << std::endl << std::endl;
  return;
}


static void outputFloat(std::ostream& out, float value)
{
  char buffer[32];
  snprintf(buffer, 30, " %.8f", value);
  out << buffer;
  return;
}

void Teleporter::printOBJ(std::ostream& out, const std::string& /*indent*/) const
{
  int i;
  fvec3 verts[8] = {
    fvec3(-1.0f, -1.0f, 0.0f),
    fvec3(+1.0f, -1.0f, 0.0f),
    fvec3(+1.0f, +1.0f, 0.0f),
    fvec3(-1.0f, +1.0f, 0.0f),
    fvec3(-1.0f, -1.0f, 1.0f),
    fvec3(+1.0f, -1.0f, 1.0f),
    fvec3(+1.0f, +1.0f, 1.0f),
    fvec3(-1.0f, +1.0f, 1.0f)
  };
  fvec3 norms[6] = {
    fvec3(0.0f, -1.0f, 0.0f), fvec3(+1.0f, 0.0f, 0.0f),
    fvec3(0.0f, +1.0f, 0.0f), fvec3(-1.0f, 0.0f, 0.0f),
    fvec3(0.0f, 0.0f, -1.0f), fvec3(0.0f, 0.0f, +1.0f)
  };
  fvec2 txcds[4] = {
    fvec2(0.0f, 0.0f), fvec2(1.0f, 0.0f), fvec2(1.0f, 1.0f), fvec2(0.0f, 1.0f)
  };
  MeshTransform xform;
  const float degrees = getRotation() * (float)(180.0 / M_PI);
  const fvec3 zAxis(0.0f, 0.0f, +1.0f);
  xform.addScale(getSize());
  xform.addSpin(degrees, zAxis);
  xform.addShift(getPosition());
  xform.finalize();
  MeshTransform::Tool xtool(xform);
  for (i = 0; i < 8; i++) {
    xtool.modifyVertex(verts[i]);
  }
  for (i = 0; i < 6; i++) {
    xtool.modifyNormal(norms[i]);
  }

  out << "# OBJ - start tele" << std::endl;
  out << "o bztele_" << getObjCounter() << std::endl;

  for (i = 0; i < 8; i++) {
    out << "v";
    outputFloat(out, verts[i][0]);
    outputFloat(out, verts[i][1]);
    outputFloat(out, verts[i][2]);
    out << std::endl;
  }
  for (i = 0; i < 4; i++) {
    out << "vt";
    outputFloat(out, txcds[i][0]);
    outputFloat(out, txcds[i][1]);
    out << std::endl;
  }
  for (i = 0; i < 6; i++) {
    out << "vn";
    outputFloat(out, norms[i][0]);
    outputFloat(out, norms[i][1]);
    outputFloat(out, norms[i][2]);
    out << std::endl;
  }
  out << "usemtl telefront" << std::endl;
  out << "f -7/-4/-5 -6/-3/-5 -2/-2/-5 -3/-1/-5" << std::endl;
  out << "usemtl teleback" << std::endl;
  out << "f -5/-4/-3 -8/-3/-3 -4/-2/-3 -1/-1/-3" << std::endl;
  out << "usemtl telerim" << std::endl;
  out << "f -5/-4/-2 -6/-3/-2 -7/-2/-2 -8/-1/-2" << std::endl;
  out << "f -4/-4/-1 -3/-3/-1 -2/-2/-1 -1/-1/-1" << std::endl;
  out << "f -8/-4/-6 -7/-3/-6 -3/-2/-6 -4/-1/-6" << std::endl;
  out << "f -6/-4/-4 -5/-3/-4 -1/-2/-4 -2/-1/-4" << std::endl;

  out << std::endl;

  incObjCounter();

  return;
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
