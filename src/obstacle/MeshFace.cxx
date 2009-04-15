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

/* interface header */
#include "MeshFace.h"

/* system implementation headers */
#include <iostream>
#include <math.h>

/* common implementation headers */
#include "global.h"
#include "Pack.h"
#include "vectors.h"
#include "MeshObstacle.h"
#include "PhysicsDriver.h"
#include "Intersect.h"


const char* MeshFace::typeName = "MeshFace";


MeshFace::MeshFace(MeshObstacle* _mesh)
{
  mesh = _mesh;
  id = -1;
  vertexCount = 0;
  vertices = NULL;
  normals = NULL;
  texcoords = NULL;
  noclusters = false;
  smoothBounce = false;
  driveThrough = 0;
  shootThrough = 0;
  edges = NULL;
  edgePlanes = NULL;
  specialData = NULL;
  specialState = 0;
  phydrv = -1;
  return;
}


MeshFace::MeshFace(MeshObstacle* _mesh, int _vertexCount,
                   const fvec3** _vertices,
                   const fvec3** _normals,
                   const fvec2** _texcoords,
                   const BzMaterial* _bzMaterial, int physics,
                   bool _noclusters, bool bounce,
                   unsigned char drive, unsigned char shoot, bool rico)
{
  mesh = _mesh;
  id = -1;
  vertexCount = _vertexCount;
  vertices = _vertices;
  normals = _normals;
  texcoords = _texcoords;
  bzMaterial = _bzMaterial;
  phydrv = physics;
  noclusters = _noclusters;
  smoothBounce = bounce;
  driveThrough = drive;
  shootThrough = shoot;
  ricochet     = rico;
  edges = NULL;
  edgePlanes = NULL;
  specialData = NULL;
  specialState = 0;

  finalize();

  return;
}


void MeshFace::finalize()
{
  float maxCrossSqr = 0.0f;
  fvec3 bestCross(0.0f, 0.0f, 0.0f);
  int bestSet[3] = { -1, -1, -1 };

  // find the best vertices for making the plane
  int i, j, k;
  for (i = 0; i < (vertexCount - 2); i++) {
    for (j = i; j < (vertexCount - 1); j++) {
      for (k = j; k < (vertexCount - 0); k++) {
	const fvec3 edge1 = *vertices[k] - *vertices[j];
	const fvec3 edge2 = *vertices[i] - *vertices[j];
	const fvec3 cross = fvec3::cross(edge1, edge2);
	const float lengthSq = cross.lengthSq();
	if (lengthSq > maxCrossSqr) {
	  maxCrossSqr = lengthSq;
	  bestSet[0] = i;
	  bestSet[1] = j;
	  bestSet[2] = k;
	  bestCross = cross;
	}
      }
    }
  }

  if (maxCrossSqr < +1.0e-20f) {

    logDebugMessage(1,"invalid mesh face (%f)", maxCrossSqr);
    if ((debugLevel >= 3) && (mesh != NULL)) {
      logDebugMessage(0,":");
      for (i = 0; i < vertexCount; i++) {
	logDebugMessage(0," %i", (int)(vertices[i] - mesh->getVertices()));
      }
      print(std::cerr, "");
    }
    logDebugMessage(1,"\n");

    vertexCount = 0;
    return;
  }

  // make the plane
  float scale = 1.0f / sqrtf (maxCrossSqr);
  const fvec3* vert = vertices[bestSet[1]];
  plane.xyz() = bestCross * scale;
  plane.w = -fvec3::dot(plane.xyz(), *vert);

  // see if the whole face is convex
  int v;
  for (v = 0; v < vertexCount; v++) {
    fvec3 a, b, c;
    a = *vertices[(v + 1) % vertexCount] - *vertices[(v + 0) % vertexCount];
    b = *vertices[(v + 2) % vertexCount] - *vertices[(v + 1) % vertexCount];
    c = fvec3::cross(a, b);
    const float d = fvec3::dot(c, plane.xyz());
    if (d <= 0.0f) {
      logDebugMessage(1,"non-convex mesh face (%f)", d);
      if ((debugLevel >= 3) && (mesh != NULL)) {
	logDebugMessage(0,":");
	for (i = 0; i < vertexCount; i++) {
	  logDebugMessage(0," %i", (int)(vertices[i] - mesh->getVertices()));
	}
	print(std::cerr, "");
      }
      logDebugMessage(1,"\n");

      vertexCount = 0;
      return;
    }
  }

  // see if the vertices are coplanar
  for (v = 0; v < vertexCount; v++) {
    const float cross = fvec3::dot(*vertices[v], plane.xyz());
    if (fabsf(cross + plane.w) > 1.0e-3) {
      logDebugMessage(1,"non-planar mesh face (%f)", cross + plane.w);
      if ((debugLevel >= 3) && (mesh != NULL)) {
	logDebugMessage(0,":");
	for (i = 0; i < vertexCount; i++) {
	  logDebugMessage(0," %i", (int)(vertices[i] - mesh->getVertices()));
	}
	print(std::cerr, "");
      }
      logDebugMessage(1,"\n");

      vertexCount = 0;
      return;
    }
  }

  // setup extents
  for (v = 0; v < vertexCount; v++) {
    extents.expandToPoint(*vertices[v]);
  }

  // setup fake obstacle parameters
  pos.xy()  = 0.5f * (extents.maxs.xy() + extents.mins.xy());
  size.xy() = 0.5f * (extents.maxs.xy() - extents.mins.xy());
  pos.z  = extents.mins.z;
  size.z = (extents.maxs.z - extents.mins.z);
  angle = 0.0f;
  zFlip = false;

  // make the edge planes
  edgePlanes = new fvec4[vertexCount];
  for (v = 0; v < vertexCount; v++) {
    const int next = (v + 1) % vertexCount;
    const fvec3 edge = *vertices[next] - *vertices[v];
    edgePlanes[v].xyz() = fvec3::cross(edge, plane.xyz());
    fvec3::normalize(edgePlanes[v].xyz());
    edgePlanes[v].w = -fvec3::dot(*vertices[v], edgePlanes[v].xyz());
  }

  // set the plane type
  planeBits = 0;
  const float fudge = 1.0e-3f;
  if ((fabsf(plane.z) + fudge) >= 1.0f) {
    planeBits |= ZPlane;
    if (plane.z > 0.0f) {
      planeBits |= UpPlane;
      //FIXME
      plane.z = 1.0f;
      plane.w = -pos.z;
    } else {
      planeBits |= DownPlane;
      //FIXME
      plane.z = -1.0f;
      plane.w = +pos.z;
    }
    //FIXME
    plane.x = 0.0f;
    plane.y = 0.0f;
  }
  else if ((fabsf(plane.x) + fudge) >= 1.0f) {
    planeBits |= XPlane;
  }
  else if ((fabsf(plane.y) + fudge) >= 1.0f) {
    planeBits |= YPlane;
  }

  if (fabsf(plane.z) < fudge) {
    planeBits |= WallPlane;
  }

  return;
}


MeshFace::~MeshFace()
{
  delete[] vertices;
  delete[] normals;
  delete[] texcoords;
  delete[] edges;
  delete[] edgePlanes;
  delete specialData;
  return;
}


const char* MeshFace::getType() const
{
  return typeName;
}


const char* MeshFace::getClassName() // const
{
  return typeName;
}


bool MeshFace::isValid() const
{
  // this is used as a tag in finalize()
  if (vertexCount == 0) {
    return false;
  } else {
    return true;
  }
}


bool MeshFace::isFlatTop() const
{
  return isUpPlane();
}


float MeshFace::intersect(const Ray& ray) const
{
  // NOTE: i'd use a quick test here first, but the
  //       plan is to use an octree for the collision
  //       manager which should get us close enough
  //       that a quick test might actually eat up time.
  //
  // find where the ray crosses each plane, and then
  // check the dot-product of the three bounding planes
  // to see if the intersection point is contained within
  // the face.
  //
  //  L - line unit vector	    Lo - line origin
  //  N - plane normal unit vector  d  - plane offset
  //  P - point in question	    t  - time
  //
  //  (N dot P) + d = 0		       { plane equation }
  //  P = (t * L) + Lo		       { line  equation }
  //  t (N dot L) + (N dot Lo) + d = 0
  //
  //  t = - (d + (N dot Lo)) / (N dot L)     { time of impact }
  //
  const fvec3& dir = ray.getDirection();
  const fvec3& origin = ray.getOrigin();
  float hitTime;

  // get the time until the shot would hit each plane
  const float linedot = fvec3::dot(dir, plane.xyz());
  if (linedot >= -0.001f) {
    // shot is either parallel, or going through backwards
    return -1.0f;
  }
  const float origindot = fvec3::dot(origin, plane.xyz());
  // linedot should be safe to divide with now
  hitTime = - (plane.w + origindot) / linedot;
  if (hitTime < 0.0f) {
    return -1.0f;
  }

  // get the contact location
  const fvec3 point = origin + (dir * hitTime);

  // now test against the edge planes
  for (int q = 0; q < vertexCount; q++) {
    const float d = edgePlanes[q].planeDist(point);
    if (d > 0.001f) {
      return -1.0f;
    }
  }

  return hitTime;
}


void MeshFace::get3DNormal(const fvec3& p, fvec3& n) const
{
  if (!smoothBounce || !useNormals()) {
    // just use the plain normal
    n = plane.xyz();
  }
  else {
    // FIXME: this isn't quite right
    // normal smoothing to fake curved surfaces
    int i;
    // calculate the triangle ares
    float totalArea = 0.0f;
    float* areas = new float[vertexCount];
    for (i = 0; i < vertexCount; i++) {
      int next = (i + 1) % vertexCount;
      const fvec3 ea = p - *vertices[i];
      const fvec3 eb = *vertices[next] - *vertices[i];
      const fvec3 cross = fvec3::cross(ea, eb);
      areas[i] = cross.length();
      totalArea = totalArea + areas[i];
    }
    float smallestArea = MAXFLOAT;
    float* twinAreas = new float[vertexCount];
    for (i = 0; i < vertexCount; i++) {
      int next = (i + 1) % vertexCount;
      twinAreas[i] = areas[i] + areas[next];
      if (twinAreas[i] < 1.0e-10f) {
	n = *normals[next];
	delete[] areas;
	delete[] twinAreas;
	return;
      }
      if (twinAreas[i] < smallestArea) {
	smallestArea = twinAreas[i];
      }
    }
    fvec3 normal(0.0f, 0.0f, 0.0f);
    for (i = 0; i < vertexCount; i++) {
      int next = (i + 1) % vertexCount;
      float factor = smallestArea / twinAreas[i];
      normal += (*normals[next] * factor);
    }
    n = normal.normalize();

    delete[] areas;
    delete[] twinAreas;
  }

  return;
}


void MeshFace::getNormal(const fvec3& /*p*/, fvec3& n) const
{
  n = plane.xyz();
  return;
}


///////////////////////////////////////////////////////////////
//  FIXME - all geometry after this point is currently JUNK! //
///////////////////////////////////////////////////////////////


bool MeshFace::getHitNormal(const fvec3& /*oldPos*/, float /*oldAngle*/,
			    const fvec3& /*newPos*/, float /*newAngle*/,
			    float /*dx*/, float /*dy*/, float /*height*/,
			    fvec3& normal) const
{
  if (normal) {
    normal = plane.xyz();
  }
  return true;
}


bool MeshFace::inCylinder(const fvec3& p,float radius, float height) const
{
  return inBox(p, 0.0f, radius, radius, height);
}


bool MeshFace::inBox(const fvec3& p, float _angle,
		     float dx, float dy, float height) const
{
  int i;

  // Z axis separation test
  if ((extents.mins.z > (p.z + height)) || (extents.maxs.z <= p.z)) {
    return false;
  }

  // translate the face so that the box is an origin box
  // centered at 0,0,0  (this assumes that it is cheaper
  // to move the polygon than the box, tris and quads will
  // probably be the dominant polygon types).

  fvec4 pln; // translated plane
  fvec3* v = new fvec3[vertexCount]; // translated vertices
  const float cos_val = cosf(-_angle);
  const float sin_val = sinf(-_angle);
  for (i = 0; i < vertexCount; i++) {
    float h[2];
    h[0] = vertices[i]->x - p[0];
    h[1] = vertices[i]->y - p[1];
    v[i][0] = (cos_val * h[0]) - (sin_val * h[1]);
    v[i][1] = (cos_val * h[1]) + (sin_val * h[0]);
    v[i][2] = vertices[i]->z - p[2];
  }
  pln.x = (cos_val * plane.x) - (sin_val * plane.y);
  pln.y = (cos_val * plane.y) + (sin_val * plane.x);
  pln.z = plane.z;
  pln.w = plane.w + fvec3::dot(plane.xyz(), p);

  // testPolygonInAxisBox() expects us to have already done all of the
  // separation tests with respect to the box planes. we could not do
  // the X and Y axis tests until we'd found the translated vertices,
  // so we will do them now.

  // X axis test
  float min, max;
  min = +MAXFLOAT;
  max = -MAXFLOAT;
  for (i = 0; i < vertexCount; i++) {
    if (v[i].x < min) {
      min = v[i].x;
    }
    if (v[i].x > max) {
      max = v[i].x;
    }
  }
  if ((min > dx) || (max < -dx)) {
    delete[] v;
    return false;
  }

  // Y axis test
  min = +MAXFLOAT;
  max = -MAXFLOAT;
  for (i = 0; i < vertexCount; i++) {
    if (v[i].y < min) {
      min = v[i].y;
    }
    if (v[i].y > max) {
      max = v[i].y;
    }
  }
  if ((min > dy) || (max < -dy)) {
    delete[] v;
    return false;
  }

  // FIXME: do not use testPolygonInAxisBox()
  Extents box;
  box.mins = fvec3(-dx, -dy, 0.0f);
  box.maxs = fvec3(+dx, +dy, height);

  bool hit = Intersect::testPolygonInAxisBox(vertexCount, v, pln, box);

  delete[] v;

  return hit;
}


bool MeshFace::inMovingBox(const fvec3& oldPos, float /*oldAngle*/,
			   const fvec3& newPos, float newAngle,
			   float dx, float dy, float height) const
{
  // expand the box with respect to Z axis motion
  fvec3 _pos;
  _pos.x = newPos.x;
  _pos.y = newPos.y;
  if (oldPos.z < newPos.z) {
    _pos.z = oldPos.z;
  } else {
    _pos.z = newPos.z;
  }
  height = height + fabsf(oldPos.z - newPos.z);

  return inBox(_pos, newAngle, dx, dy, height);
}


bool MeshFace::isCrossing(const fvec3& /*p*/, float /*angle*/,
			  float /*dx*/, float /*dy*/, float /*height*/,
			  fvec4* planePtr) const
{
  if (planePtr) {
    *planePtr = plane;
  }
  return true;
}


void *MeshFace::pack(void *buf) const
{
  // state byte
  unsigned char stateByte = 0;
  stateByte |= useNormals()     ? (1 << 0) : 0;
  stateByte |= useTexcoords()   ? (1 << 1) : 0;
  stateByte |= isDriveThrough() ? (1 << 2) : 0;
  stateByte |= isShootThrough() ? (1 << 3) : 0;
  stateByte |= smoothBounce     ? (1 << 4) : 0;
  stateByte |= noclusters       ? (1 << 5) : 0;
  stateByte |= canRicochet()    ? (1 << 6) : 0;
  buf = nboPackUInt8(buf, stateByte);

  // vertices
  buf = nboPackInt32(buf, vertexCount);
  for (int i = 0; i < vertexCount; i++) {
    int32_t index = vertices[i] - mesh->getVertices();
    buf = nboPackInt32(buf, index);
  }

  // normals
  if (useNormals()) {
    for (int i = 0; i < vertexCount; i++) {
      int32_t index = normals[i] - mesh->getNormals();
      buf = nboPackInt32(buf, index);
    }
  }

  // texcoords
  if (useTexcoords()) {
    for (int i = 0; i < vertexCount; i++) {
      int32_t index = texcoords[i] - mesh->getTexcoords();
      buf = nboPackInt32(buf, index);
    }
  }

  // material
  int matindex = MATERIALMGR.getIndex(bzMaterial);
  buf = nboPackInt32(buf, matindex);

  // physics driver
  buf = nboPackInt32(buf, phydrv);

  return buf;
}


void *MeshFace::unpack(void *buf)
{
  int32_t inTmp;
  driveThrough = shootThrough = smoothBounce = false;
  // state byte
  bool tmpNormals, tmpTexcoords;
  unsigned char stateByte = 0;
  buf = nboUnpackUInt8(buf, stateByte);
  tmpNormals   =  (stateByte & (1 << 0)) != 0;
  tmpTexcoords =  (stateByte & (1 << 1)) != 0;
  driveThrough = ((stateByte & (1 << 2)) != 0) ? 0xFF : 0;
  shootThrough = ((stateByte & (1 << 3)) != 0) ? 0xFF : 0;
  smoothBounce =  (stateByte & (1 << 4)) != 0;
  noclusters   =  (stateByte & (1 << 5)) != 0;
  ricochet     =  (stateByte & (1 << 6)) != 0;

  // vertices
  buf = nboUnpackInt32(buf, inTmp);
  vertexCount = int(inTmp);
  vertices = new const fvec3*[vertexCount];
  for (int i = 0; i < vertexCount; i++) {
    int32_t index;
    buf = nboUnpackInt32(buf, index);
    vertices[i] = mesh->getVertices() + index;
  }

  // normals
  if (tmpNormals) {
    normals = new const fvec3*[vertexCount];
    for (int i = 0; i < vertexCount; i++) {
      int32_t index;
      buf = nboUnpackInt32(buf, index);
      normals[i] = mesh->getNormals() + index;
    }
  }

  // texcoords
  if (tmpTexcoords) {
    texcoords = new const fvec2*[vertexCount];
    for (int i = 0; i < vertexCount; i++) {
      int32_t index;
      buf = nboUnpackInt32(buf, index);
      texcoords[i] = mesh->getTexcoords() + index;
    }
  }

  // material
  int32_t matindex;
  buf = nboUnpackInt32(buf, matindex);
  bzMaterial = MATERIALMGR.getMaterial(matindex);

  // physics driver
  buf = nboUnpackInt32(buf, inTmp);
  phydrv = int(inTmp);

  finalize();

  return buf;
}


int MeshFace::packSize() const
{
  int fullSize = sizeof(unsigned char);
  fullSize += sizeof(int32_t);
  fullSize += sizeof(int32_t) * vertexCount;
  if (useNormals()) {
    fullSize += sizeof(int32_t) * vertexCount;
  }
  if (useTexcoords()) {
    fullSize += sizeof(int32_t) * vertexCount;
  }
  fullSize += sizeof(int32_t); // material
  fullSize += sizeof(int32_t); // physics driver

  return fullSize;
}


void MeshFace::print(std::ostream& out, const std::string& indent) const
{
  if (mesh == NULL) {
    return;
  }

  int i;
  out << indent << "  face" << std::endl;

  if (debugLevel >= 3) {
    out << indent << "  # plane normal = "
                  << plane.x << " " << plane.y << " "
		  << plane.z << " " << plane.w << std::endl;
  }

  out << indent << "    vertices";
  for (i = 0; i < vertexCount; i++) {
    const int index = vertices[i] - mesh->getVertices();
    out << " " << index;
  }
  if (debugLevel >= 3) {
    out << indent << " #";
    for (i = 0; i < vertexCount; i++) {
      out << " " << vertices[i]->x << " " << vertices[i]->y << " " << vertices[i]->z;
    }
  }
  out << std::endl;

  if (normals != NULL) {
    out << indent << "    normals";
    for (i = 0; i < vertexCount; i++) {
      const int index = normals[i] - mesh->getNormals();
      out << " " << index;
    }
    if (debugLevel >= 3) {
      out << " #";
      for (i = 0; i < vertexCount; i++) {
	out << " " << normals[i]->x <<  " " << normals[i]->y << " " << normals[i]->z;
      }
    }
    out << std::endl;
  }

  if (texcoords != NULL) {
    out << indent << "    texcoords";
    for (i = 0; i < vertexCount; i++) {
      const int index = texcoords[i] - mesh->getTexcoords();
      out << " " << index;
    }
    if (debugLevel >= 3) {
      out << " #";
      for (i = 0; i < vertexCount; i++) {
	out << " " << texcoords[i]->x <<  " " << texcoords[i]->y;
      }
    }
    out << std::endl;
  }

  out << indent << "    matref ";
  MATERIALMGR.printReference(out, bzMaterial);
  out << std::endl;

  const PhysicsDriver* driver = PHYDRVMGR.getDriver(phydrv);
  if (driver != NULL) {
    out << indent << "    phydrv ";
    if (driver->getName().size() > 0) {
      out << driver->getName();
    } else {
      out << phydrv;
    }
    out << std::endl;
  }

  if (noclusters && !mesh->noClusters()) {
    out << indent << "    noclusters" << std::endl;
  }
  if (smoothBounce && !mesh->useSmoothBounce()) {
    out << indent << "    smoothBounce" << std::endl;
  }
  if ((driveThrough && shootThrough) &&
      !(mesh->isDriveThrough() && mesh->isShootThrough())) {
    out << indent << "    passable" << std::endl;
  } else {
    if (driveThrough && !mesh->isDriveThrough()) {
      out << indent << "    driveThrough" << std::endl;
    }
    if (shootThrough && !mesh->isShootThrough()) {
      out << indent << "    shootThrough" << std::endl;
    }
  }
  if (ricochet && !mesh->canRicochet()) {
    out << indent << "    ricochet" << std::endl;
  }

  out << indent << "  endface" << std::endl;

  return;
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
