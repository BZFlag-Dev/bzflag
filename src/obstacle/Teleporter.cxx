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

#include <math.h>
#include "common.h"
#include "global.h"
#include "Teleporter.h"
#include "Intersect.h"

const char* Teleporter::typeName = "Teleporter";

Teleporter::Teleporter(const float* p, float a, float w,
		       float b, float h,float _border, bool _horizontal,
		       bool drive, bool shoot) :
		       Obstacle(p, a, w, horizontal ? b : b + 2 * _border,
		                horizontal ? h : h + _border,drive,shoot),
		       border(_border), horizontal(_horizontal)
{
  makeLinks();
  return;
}


Teleporter::~Teleporter()
{
  delete backLink;
  delete frontLink;
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


void Teleporter::makeLinks()
{
  int i;
  float **fvrts = new float*[4];
  float **bvrts = new float*[4];
  for (i = 0; i < 4; i++) {
    fvrts[i] = fvertices[i];
    bvrts[i] = bvertices[i];
  }
  MeshMaterial material; // this is overriden in SceneBuilder.cxx
  
  const float* p = getPosition();
  const float a = getRotation();
  const float w = getWidth();
  const float b = getBreadth();
  const float br = getBorder();
  const float h = getHeight();

  const float cos_val = cos(a);
  const float sin_val = sin(a);
  
  if (!horizontal) {
    const float params[4][2] = 
      {{-1.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {-1.0f, 1.0f}};
    float wlen[2] = { (cos_val * w), (sin_val * w) };
    float blen[2] = { (-sin_val * (b - br)), (cos_val * (b - br)) };

    for (i = 0; i < 4 ;i++) {
      bvrts[i][0] = p[0] + (wlen[0] + (blen[0] * params[i][0]));
      bvrts[i][1] = p[1] + (wlen[1] + (blen[1] * params[i][0]));
      bvrts[i][2] = p[2] + ((h - br) * params[i][1]);
    }
    backLink =
      new MeshFace(NULL, 4, bvrts, NULL, NULL, material, false, true, true);
    backLink->finalize();  

    for (i = 0; i < 4 ;i++) {
      fvrts[i][0] = p[0] - (wlen[0] + (blen[0] * params[i][0]));
      fvrts[i][1] = p[1] - (wlen[1] + (blen[1] * params[i][0]));
      fvrts[i][2] = p[2] + ((h - br) * params[i][1]);
    }
    frontLink =
      new MeshFace(NULL, 4, fvrts, NULL, NULL, material, false, true, true);
    frontLink->finalize();  
  }
  else {
    float xlen = w - br;
    float ylen = b - br;
    bvrts[0][0] = p[0] + ((cos_val * xlen) - (sin_val * ylen));
    bvrts[0][1] = p[1] + ((cos_val * ylen) + (sin_val * xlen));
    bvrts[0][2] = p[2] + h - br;
    bvrts[1][0] = p[0] + ((cos_val * xlen) - (sin_val * -ylen));
    bvrts[1][1] = p[1] + ((cos_val * -ylen) + (sin_val * xlen));
    bvrts[1][2] = p[2] + h - br;
    bvrts[2][0] = p[0] + ((cos_val * -xlen) - (sin_val * -ylen));
    bvrts[2][1] = p[1] + ((cos_val * -ylen) + (sin_val * -xlen));
    bvrts[2][2] = p[2] + h - br;
    bvrts[3][0] = p[0] + ((cos_val * -xlen) - (sin_val * ylen));
    bvrts[3][1] = p[1] + ((cos_val * ylen) + (sin_val * -xlen));
    bvrts[3][2] = p[2] + h - br;
    backLink =
      new MeshFace(NULL, 4, bvrts, NULL, NULL, material, false, true, true);
    backLink->finalize();
    
    for (i = 0; i < 4; i++) {
      memcpy(fvrts[i], bvrts[3 - i], sizeof(float[3])); // reverse order
      fvrts[i][2] = p[2] + h; // change the height
    }
    frontLink =
      new MeshFace(NULL, 4, fvrts, NULL, NULL, material, false, true, true);
    frontLink->finalize();  
  }
  
  return;  
}


bool Teleporter::isValid() const
{
  if (!backLink->isValid() || !frontLink->isValid()) {
    return false;
  }
  return Obstacle::isValid();
}


void Teleporter::getExtents(float* mins, float* maxs) const
{
  // the same as the default Obstacle::getExtents(), except that we
  // use the border width instead of size[0] (which for teleporters
  // is half the distance between the teleporter planes).
  float sizeX = border * 0.5f;
  float xspan = (fabsf(cos(angle)) * sizeX) + (fabsf(sin(angle)) * size[1]);
  float yspan = (fabsf(cos(angle)) * size[1]) + (fabsf(sin(angle)) * sizeX);
  mins[0] = pos[0] - xspan;
  maxs[0] = pos[0] + xspan;
  mins[1] = pos[1] - yspan;
  maxs[1] = pos[1] + yspan;
  mins[2] = pos[2];
  maxs[2] = pos[2] + size[2];
  return;
}


float Teleporter::intersect(const Ray& r) const
{
  // expand to include border
  return timeRayHitsBlock(r, getPosition(), getRotation(),
			getWidth(), getBreadth(), getHeight());
}


void Teleporter::getNormal(const float* p1, float* n) const
{
  // get normal to closest border column (assume column is circular)
  const float* p2 = getPosition();
  const float c = cosf(-getRotation()), s = sinf(-getRotation());
  const float b = 0.5f * getBorder();
  const float d = getBreadth() - b;
  const float j = (c * (p1[1] - p2[1]) + s * (p1[0] - p2[0]) > 0.0f) ? d : -d;
  float cc[2];
  cc[0] = p2[0] + s * j;
  cc[1] = p2[1] + c * j;
  getNormalRect(p1, cc, getRotation(), b, b, n);
}


bool Teleporter::inCylinder(const float* p, float radius, float height) const
{
  return (p[2]+height) >= getPosition()[2] &&
	  p[2] <= getPosition()[2] + getHeight() &&
	  testRectCircle(getPosition(), getRotation(),
			 getWidth(), getBreadth(), p, radius);
}


bool Teleporter::inBox(const float* p, float a,
                       float dx, float dy, float dz) const
{
  if ((p[2] < getHeight() + getPosition()[2] - getBorder())
	  && (p[2]+dz) >= getPosition()[2]) {
    // test individual border columns
    const float c = cosf(getRotation()), s = sinf(getRotation());
    const float d = getBreadth() - 0.5f * getBorder();
    const float r = 0.5f * getBorder();
    float o[2];
    o[0] = getPosition()[0] - s * d;
    o[1] = getPosition()[1] + c * d;
    if (testRectRect(p, a, dx, dy, o, getRotation(), r, r)) return true;
    o[0] = getPosition()[0] + s * d;
    o[1] = getPosition()[1] - c * d;
    if (testRectRect(p, a, dx, dy, o, getRotation(), r, r)) return true;
  }

  else if (p[2] <= getHeight() + getPosition()[2]  && p[2] > getPosition()[2]) {
    // test crossbar
    if (testRectRect(p, a, dx, dy, getPosition(), getRotation(),
					getWidth(), getBreadth()))
      return true;
  }

  return false;
}


bool Teleporter::inMovingBox(const float* /* oldP */, float /*oldAngle */,
                             const float* p, float a,
			     float dx, float dy, float dz) const
{
  return inBox (p, a, dx, dy, dz);
}


bool Teleporter::isCrossing(const float* p, float a,
			    float dx, float dy, float /* dz */, float* plane) const
{
  // if not inside or contained then not crossing
  const float* p2 = getPosition();
  if (!testRectRect(p, a, dx, dy,
		p2, getRotation(), getWidth(), getBreadth() - getBorder()) ||
		p[2] < p2[2] || p[2] > p2[2] + getHeight() - getBorder())
    return false;
  if (!plane) return true;

  // it's crossing -- choose which wall is being crossed (this
  // is a guestimate, should really do a careful test).  just
  // see which wall the point is closest to.
  const float a2 = getRotation();
  const float c = cosf(-a2), s = sinf(-a2);
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


// return true iff ray goes through teleporting part
float Teleporter::isTeleported(const Ray& r, int& face) const
{
  // get t's for teleporter with and without border
  const float tb = intersect(r);
  const float t = timeRayHitsBlock(r, getPosition(),
			getRotation(), getWidth(),
			getBreadth() - getBorder(), getHeight() - getBorder());

  // if intersection with border is before one without then doesn't teleport
  // (cos it hit the border first).  also no teleport if no intersection.
  if ((tb >= 0.0f && t - tb > 1e-6) || t < 0.0f)
    return -1.0f;

  // get teleport position.  if above or below teleporter then no teleportation.
  float p[3];
  r.getPoint(t, p);
  p[2] -= getPosition()[2];
  if (p[2] < 0.0f || p[2] > getHeight() - getBorder())
    return -1.0f;

  // figure out which face:  rotate intersection into teleporter space,
  //	if to east of teleporter then face 0 else face 1.
  const float x = cosf(-getRotation()) * (p[0] - getPosition()[0]) -
		sinf(-getRotation()) * (p[1] - getPosition()[1]);
  face = (x > 0.0f) ? 0 : 1;
  return t;
}


float Teleporter::getProximity(const float* p, float radius) const
{
  // make sure tank is sufficiently close
  if (!testRectCircle(getPosition(), getRotation(),
			getWidth(), getBreadth() - getBorder(),
			p, 1.2f * radius))
    return 0.0f;

  // transform point to teleporter space
  // translate origin
  float pa[3];
  pa[0] = p[0] - getPosition()[0];
  pa[1] = p[1] - getPosition()[1];
  pa[2] = p[2] - getPosition()[2];

  // make sure not too far above or below teleporter
  if (pa[2] < -1.2f * radius ||
      pa[2] > getHeight() - getBorder() + 1.2f * radius)
    return 0.0f;

  // rotate and reflect into first quadrant
  const float c = cosf(-getRotation()), s = sinf(-getRotation());
  const float x = fabsf(c * pa[0] - s * pa[1]);
  const float y = fabsf(c * pa[1] + s * pa[0]);

  // get proximity to face
  float t = 1.2f - x / radius;

  // if along side then trail off as point moves away from faces
  if (y > getBreadth() - getBorder()) {
    float f = (2.0f / M_PI) * atan2f(x, y - getBreadth() + getBorder());
    t *= f * f;
  }
  else if (pa[2] < 0.0f) {
    float f = 1.0f + pa[2] / (1.2f * radius);
    if (f >= 0.0f && f <= 1.0f) t *= f * f;
  }
  else if (pa[2] > getHeight() - getBorder()) {
    float f = 1.0f - (pa[2] - getHeight() + getBorder()) / (1.2f * radius);
    if (f >= 0.0f && f <= 1.0f) t *= f * f;
  }

  return t > 0.0f ? (t > 1.0f ? 1.0f : t) : 0.0f;
}


bool Teleporter::hasCrossed(const float* p1, const float* p2, int& f) const
{
  // check above/below teleporter
  const float* p = getPosition();
  if ((p1[2] < p[2] && p2[2] < p[2]) ||
	(p1[2] > p[2] + getHeight() - getBorder() &&
	 p2[2] > p[2] + getHeight() - getBorder()))
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
			     const float* pIn, const float* dIn, float aIn,
			     float* pOut, float* dOut, float* aOut) const
{
  const float x1 = pIn[0] - getPosition()[0];
  const float y1 = pIn[1] - getPosition()[1];
  const float a = t2.getRotation() - getRotation() +
			(face1 == face2 ? M_PI : 0.0f);
  const float c = cosf(a), s = sinf(a);
  const float x2 = c * x1 - s * y1;
  const float y2 = c * y1 + s * x1;

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
  pOut[0] = t2.getPosition()[0] + (x2 * (t2.getBreadth() - t2.getBorder())) / getBreadth();
  pOut[1] = t2.getPosition()[1] + (y2 * (t2.getBreadth() - t2.getBorder())) / getBreadth();
  //T1 = pIn[2] - getPosition()[2]
  //W2 = t2.getHeight()
  //W1 = getHeight

  //(t2.getPosition()[2] - getPosition()[2]) adds the height differences between the
  //teleporters so that teleporters can be off of the ground at different heights.

  //pOut[2] = pIn[2] + t2.getPosition()[2] - getPosition()[2];
  pOut[2] = t2.getPosition()[2]
	  + ((pIn[2] - getPosition()[2]) * (t2.getHeight() - t2.getBorder()))/getHeight();

  if (dOut && dIn) {
    const float dx = dIn[0];
    const float dy = dIn[1];
    dOut[0] = c * dx - s * dy;
    dOut[1] = c * dy + s * dx;
    dOut[2] = dIn[2];
  }

  if (aOut) {
    *aOut = aIn + a;
  }
}


bool Teleporter::getHitNormal(const float* pos1, float azimuth1,
			      const float* pos2, float azimuth2,
			      float width, float breadth, float,
			      float* normal) const
{
  return Obstacle::getHitNormal(pos1, azimuth1,
			pos2, azimuth2, width, breadth,
			getPosition(), getRotation(), getWidth(), getBreadth(),
			getHeight(), normal) >= 0.0f;
}


// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

