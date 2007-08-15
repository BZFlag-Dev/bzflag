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
#include <math.h>
#include <assert.h>
#include "global.h"
#include "Pack.h"
#include "vectors.h"

#include "ConeObstacle.h"
#include "MeshUtils.h"
#include "PhysicsDriver.h"
#include "MeshTransform.h"


const char* ConeObstacle::typeName = "ConeObstacle";


ConeObstacle::ConeObstacle()
{
  iType = coneType;
  return;
}


ConeObstacle::ConeObstacle(const MeshTransform& xform,
			   const float* _pos, const float* _size,
			   float _rotation, float _sweepAngle,
			   const float _texsize[2], bool _useNormals,
			   int _divisions, const BzMaterial* mats[MaterialCount],
			   int physics, bool bounce, unsigned char drive, unsigned char shoot)
{
  // common obstace parameters
  memcpy(pos, _pos, sizeof(pos));
  memcpy(size, _size, sizeof(size));
  angle = _rotation;
  ZFlip = false;
  driveThrough = drive;
  shootThrough = shoot;

  // arc specific parameters
  transform = xform;
  divisions = _divisions;
  sweepAngle = _sweepAngle;
  phydrv = physics;
  smoothBounce = bounce;
  useNormals = _useNormals;
  memcpy(texsize, _texsize, sizeof(texsize));
  memcpy(materials, mats, sizeof(materials));

  iType = coneType;
  finalize();

  return;
}


ConeObstacle::~ConeObstacle()
{
  return;
}


Obstacle* ConeObstacle::copyWithTransform(const MeshTransform& xform) const
{
  MeshTransform tmpXform = transform;
  tmpXform.append(xform);

  ConeObstacle* copy =
    new ConeObstacle(tmpXform, pos, size, angle, sweepAngle,
		    texsize, useNormals, divisions,
		    (const BzMaterial**)materials, phydrv,
		    smoothBounce, driveThrough, shootThrough);
  return copy;
}


const char* ConeObstacle::getType() const
{
  return typeName;
}


const char* ConeObstacle::getClassName() // const
{
  return typeName;
}


bool ConeObstacle::isValid() const
{
  return true;
}


void ConeObstacle::finalize()
{
  return;
}


MeshObstacle* ConeObstacle::makeMesh()
{
  MeshObstacle* mesh;
  bool isCircle = false; // angle of 360 degrees
  const float minSize = 1.0e-6f; // cheezy / lazy

  // absolute the sizes
  float sz[3];
  sz[0] = fabsf(size[0]);
  sz[1] = fabsf(size[1]);
  sz[2] = fabsf(size[2]);

  // validity checking
  if ((sz[0] < minSize) || (sz[1] < minSize) || (sz[2] < minSize) ||
      (fabs(texsize[0]) < minSize) || (fabs(texsize[1]) < minSize)) {
    return NULL;
  }

  // adjust the texture sizes
  float texsz[2];
  memcpy (texsz, texsize, sizeof(float[2]));
  if (texsz[0] < 0.0f) {
    // unless you want to do elliptic integrals, here's
    // the Ramanujan approximation for the circumference
    // of an ellipse  (it will be rounded anyways)
    const float circ =
      (float)M_PI * ((3.0f * (sz[0] + sz[1])) -
	      sqrtf ((sz[0] + (3.0f * sz[1])) * (sz[1] + (3.0f * sz[0]))));
    // make sure it's an integral number so that the edges line up
    texsz[0] = -floorf(circ / texsz[0]);
  }
  if (texsz[1] < 0.0f) {
    texsz[1] = -(sz[2] / texsz[1]);
  }

  // setup the angles
  float r = angle;
  float a = sweepAngle;
  if (a > +360.0f) {
    a = +360.0f;
  }
  if (a < -360.0f) {
    a = -360.0f;
  }
  a = a * (float)(M_PI / 180.0); // convert to radians
  if (a < 0.0f) {
    r = r + a;
    a = -a;
  }

  // more validity checking
  if (divisions <= (int) ((a + minSize) / M_PI)) {
    return NULL;
  }

  if (fabsf ((float)M_PI - fmodf (a + (float)M_PI, (float)M_PI * 2.0f)) < minSize) {
    isCircle = true;
  }

  // setup the coordinates
  std::vector<char> checkTypes;
  std::vector<cfvec3> checkPoints;
  std::vector<cfvec3> vertices;
  std::vector<cfvec3> normals;
  std::vector<cfvec2> texcoords;
  cfvec3 v, n;
  cfvec2 t;

  // add the checkpoint (one is sufficient)
  if (isCircle) {
    v[0] = pos[0];
    v[1] = pos[1];
  } else {
    const float dir = r + (0.5f * a);
    v[0] = pos[0] + (cosf(dir) * sz[0] * 0.25f);
    v[1] = pos[1] + (sinf(dir) * sz[1] * 0.25f);
  }
  v[2] = pos[2] + (0.5f * sz[2]);
  checkPoints.push_back(v);
  checkTypes.push_back(MeshObstacle::CheckInside);

  int i;
  const float astep = a / (float) divisions;

  for (i = 0; i < (divisions + 1); i++) {
    float ang = r + (astep * (float)i);
    float cos_val = cosf(ang);
    float sin_val = sinf(ang);

    // vertices
    if (!isCircle || (i != divisions)) {
      float delta[2];
      // vertices (around the edge)
      delta[0] = cos_val * sz[0];
      delta[1] = sin_val * sz[1];
      v[0] = pos[0] + delta[0];
      v[1] = pos[1] + delta[1];
      v[2] = pos[2];
      vertices.push_back(v);
      // normals (around the edge)
      if (useNormals) {
	// the horizontal normals
	n[0] = cos_val / sz[0];
	n[1] = sin_val / sz[1];
	n[2] = 1.0f / sz[2];
	// normalize
	float len = (n[0] * n[0]) + (n[1] * n[1]) + (n[2] * n[2]);
	len = 1.0f / sqrtf(len);
	n[0] *= len;
	n[1] *= len;
	n[2] *= len;
	normals.push_back(n);
      }
    }

    // texture coordinates (around the edge)
    t[0] = texsz[0] * (0.5f + (0.5f * cosf(ang)));
    t[1] = texsz[1] * (0.5f + (0.5f * sinf(ang)));
    texcoords.push_back(t);
  }

  // the central normals
  if (useNormals) {
    for (i = 0; i < divisions; i++) {
      float ang = r + (astep * (0.5f + (float)i));
      // the horizontal normals
      n[0] = cosf(ang) / sz[0];
      n[1] = sinf(ang) / sz[1];
      n[2] = 1.0f / sz[2];
      // normalize
      float len = (n[0] * n[0]) + (n[1] * n[1]) + (n[2] * n[2]);
      len = 1.0f / sqrtf(len);
      n[0] *= len;
      n[1] *= len;
      n[2] *= len;
      normals.push_back(n);
    }
  }

  // the central coordinates
  v[0] = pos[0];
  v[1] = pos[1];
  v[2] = pos[2];
  vertices.push_back(v); // bottom
  v[2] = pos[2] + sz[2];
  vertices.push_back(v); // top
  t[0] = texsz[0] * 0.5f;
  t[1] = texsz[1] * 0.5f;
  texcoords.push_back(t);

  // for the start/end faces
  if (!isCircle) {
    t[0] = texsz[0] * 0.0f;
    t[1] = texsz[1] * 0.0f;
    texcoords.push_back(t);
    t[0] = texsz[0] * 1.0f;
    t[1] = texsz[1] * 0.0f;
    texcoords.push_back(t);
    t[0] = texsz[0] * 1.0f;
    t[1] = texsz[1] * 1.0f;
    texcoords.push_back(t);
    t[0] = texsz[0] * 0.0f;
    t[1] = texsz[1] * 1.0f;
    texcoords.push_back(t);
  }

  // setup the face count
  int fcount = (divisions * 2);
  if (!isCircle) {
    fcount = fcount + 2; // add the start and end faces
  }

  mesh = new MeshObstacle(transform, checkTypes, checkPoints,
			  vertices, normals, texcoords, fcount,
			  false, smoothBounce, driveThrough, shootThrough);

  // now make the faces
  int vlen;
  if (isCircle) {
    vlen = divisions;
  } else {
    vlen = divisions + 1;
  }

  std::vector<int> vlist;
  std::vector<int> nlist;
  std::vector<int> tlist;

  const int vtop = vlen + 1;
  const int vbot = vlen;
  const int tmid = divisions + 1;
  const int t00 = (divisions + 1) + 1 + 0;
  const int t10 = (divisions + 1) + 1 + 1;
  const int t11 = (divisions + 1) + 1 + 2;
  const int t01 = (divisions + 1) + 1 + 3;

  for (i = 0; i < divisions; i++) {

// handy macros
#define V(x) (((x) + i) % vlen)  // edge
#define NE(x) (((x) + i) % vlen) // edge
#define NC(x) (vlen + i)	 // center
#define T(x) ((x) + i)	   // edge
#define TI(x) (divisions - T(x)) // edge, backwards

    // edge
    push3Ints(vlist, vtop, V(0), V(1));
    if (useNormals) push3Ints(nlist, NC(0), NE(0), NE(1));
    push3Ints(tlist, tmid, T(0), T(1));
    addFace(mesh, vlist, nlist, tlist, materials[Edge], phydrv);
    // bottom
    push3Ints(vlist, vbot, V(1), V(0));
    push3Ints(tlist, tmid, TI(1), TI(0));
    addFace(mesh, vlist, nlist, tlist, materials[Bottom], phydrv);
  }

  if (!isCircle) {
    // start face
    push3Ints(vlist, vbot, 0, vtop);
    push3Ints(tlist, t00, t10, t01);
    addFace(mesh, vlist, nlist, tlist, materials[StartFace], phydrv);
    // end face
    push3Ints(vlist, vlen - 1, vbot, vtop);
    push3Ints(tlist, t00, t10, t11);
    addFace(mesh, vlist, nlist, tlist, materials[EndFace], phydrv);
  }

  // wrap it up
  mesh->finalize();

  if (!mesh->isValid()) {
    delete mesh;
    mesh = NULL;
  }
  return mesh;
}


float ConeObstacle::intersect(const Ray&) const
{
  assert(false);
  return -1.0f;
}

void ConeObstacle::get3DNormal(const float*, float*) const
{
  assert(false);
  return;
}

void ConeObstacle::getNormal(const float*, float*) const
{
  assert(false);
  return;
}

bool ConeObstacle::getHitNormal(const float*, float, const float*, float,
				float, float, float, float*) const
{
  assert(false);
  return false;
}

bool ConeObstacle::inCylinder(const float*,float, float) const
{
  assert(false);
  return false;
}

bool ConeObstacle::inBox(const float*, float, float, float, float) const
{
  assert(false);
  return false;
}

bool ConeObstacle::inMovingBox(const float*, float, const float*, float,
			       float, float, float) const
{
  assert(false);
  return false;
}

bool ConeObstacle::isCrossing(const float* /*p*/, float /*angle*/,
			      float /*dx*/, float /*dy*/, float /*height*/,
			      float* /*_plane*/) const
{
  assert(false);
  return false;
}


void *ConeObstacle::pack(void *buf) const
{
  buf = transform.pack(buf);
  buf = nboPackVector(buf, pos);
  buf = nboPackVector(buf, size);
  buf = nboPackFloat(buf, angle);
  buf = nboPackFloat(buf, sweepAngle);
  buf = nboPackInt(buf, divisions);
  buf = nboPackInt(buf, phydrv);

  int i;
  for (i = 0; i < 2; i++) {
    buf = nboPackFloat(buf, texsize[i]);
  }
  for (i = 0; i < MaterialCount; i++) {
    int matindex = MATERIALMGR.getIndex(materials[i]);
    buf = nboPackInt(buf, matindex);
  }

  // pack the state byte
  unsigned char stateByte = 0;
  stateByte |= isDriveThrough() ? (1 << 0) : 0;
  stateByte |= isShootThrough() ? (1 << 1) : 0;
  stateByte |= smoothBounce     ? (1 << 2) : 0;
  stateByte |= useNormals       ? (1 << 3) : 0;
  buf = nboPackUByte(buf, stateByte);

  return buf;
}


void *ConeObstacle::unpack(void *buf)
{
  int32_t inTmp;
  buf = transform.unpack(buf);
  buf = nboUnpackVector(buf, pos);
  buf = nboUnpackVector(buf, size);
  buf = nboUnpackFloat(buf, angle);
  buf = nboUnpackFloat(buf, sweepAngle);
  buf = nboUnpackInt(buf, inTmp);
  divisions = int(inTmp);
  buf = nboUnpackInt(buf, inTmp);
  phydrv = int(inTmp);

  int i;
  for (i = 0; i < 2; i++) {
    buf = nboUnpackFloat(buf, texsize[i]);
  }
  for (i = 0; i < MaterialCount; i++) {
    int32_t matindex;
    buf = nboUnpackInt(buf, matindex);
    materials[i] = MATERIALMGR.getMaterial(matindex);
  }

  // unpack the state byte
  unsigned char stateByte;
  buf = nboUnpackUByte(buf, stateByte);
  driveThrough = (stateByte & (1 << 0)) != 0;
  shootThrough = (stateByte & (1 << 1)) != 0;
  smoothBounce = (stateByte & (1 << 2)) != 0;
  useNormals   = (stateByte & (1 << 3)) != 0;

  finalize();

  return buf;
}


int ConeObstacle::packSize() const
{
  int fullSize = transform.packSize();
  fullSize += sizeof(float[3]);
  fullSize += sizeof(float[3]);
  fullSize += sizeof(float);
  fullSize += sizeof(float);
  fullSize += sizeof(int32_t);
  fullSize += sizeof(int32_t);
  fullSize += sizeof(float[2]);
  fullSize += sizeof(int32_t[MaterialCount]);
  fullSize += sizeof(unsigned char);
  return fullSize;
}


void ConeObstacle::print(std::ostream& out, const std::string& indent) const
{
  int i;

  out << indent << "cone" << std::endl;

  out << indent << "  position " << pos[0] << " " << pos[1] << " "
				 << pos[2] << std::endl;
  out << indent << "  size " << size[0] << " " << size[1] << " "
			     << size[2] << std::endl;
  out << indent << "  rotation " << ((angle * 180.0) / M_PI) << std::endl;
  out << indent << "  angle " << sweepAngle << std::endl;
  out << indent << "  divisions " << divisions << std::endl;

  transform.printTransforms(out, indent);

  out << indent << "  texsize " << texsize[0] << " "
				<< texsize[1] << std::endl;

  const char* sideNames[MaterialCount] =
    { "edge", "bottom", "startside", "endside" };
  for (i = 0; i < MaterialCount; i++) {
    out << indent << "  " << sideNames[i] << " matref ";
    MATERIALMGR.printReference(out, materials[i]);
    out << std::endl;
  }

  const PhysicsDriver* driver = PHYDRVMGR.getDriver(phydrv);
  if (driver != NULL) {
    out << indent << "  phydrv ";
    if (driver->getName().size() > 0) {
      out << driver->getName();
    } else {
      out << phydrv;
    }
    out << std::endl;
  }

  if (smoothBounce) {
    out << indent << "  smoothBounce" << std::endl;
  }
  if (driveThrough) {
    out << indent << "  driveThrough" << std::endl;
  }
  if (shootThrough) {
    out << indent << "  shootThrough" << std::endl;
  }
  if (!useNormals) {
    out << indent << "  flatshading" << std::endl;
  }

  out << indent << "end" << std::endl;

  return;
}


// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
