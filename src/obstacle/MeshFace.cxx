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
#include "LinkPhysics.h"
#include "Intersect.h"
#include "BZDBCache.h"


const char* MeshFace::typeName = "MeshFace";


static BZDB_bool debugTele("debugTele");


MeshFace::MeshFace(MeshObstacle* _mesh)
{
  mesh = _mesh;
  faceID = -1;
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
  phydrv = -1;
  return;
}


MeshFace::MeshFace(MeshObstacle* _mesh, int _vertexCount,
                   const fvec3** _vertices,
                   const fvec3** _normals,
                   const fvec2** _texcoords,
                   const BzMaterial* _bzMaterial, int physics,
                   bool _noclusters, bool bounce,
                   unsigned char drive, unsigned char shoot, bool rico,
                   const SpecialData* special)
{
  mesh = _mesh;
  faceID = -1;
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
  if (special == NULL) {
    specialData = NULL;
  } else {
    specialData = new SpecialData;
    *specialData = *special;
  }
  edges = NULL;
  edgePlanes = NULL;

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

  // calculate the extents
  fvec3 p(0.0f, 0.0f, 0.0f);
  for (v = 0; v < vertexCount; v++) {
    const fvec3& vertex = *vertices[v];
    p += vertex;
    extents.expandToPoint(vertex);
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
  const float fudge = 2.0e-4f;
  if ((fabsf(plane.z) + fudge) >= 1.0f) {
    planeBits |= ZPlane;
    if (plane.z > 0.0f) {
      planeBits |= UpPlane;
      plane.z = 1.0f;
      plane.w = -pos.z;
    } else {
      planeBits |= DownPlane;
      plane.z = -1.0f;
      plane.w = +pos.z;
    }
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

  // setup SpecialData parameters
  setupSpecialData();

  return;
}


void MeshFace::setupSpecialData()
{
  if (!isSpecial()) {
    return;
  }
  SpecialData& sd = *specialData;

  // team bases must be flat tops
  if (sd.baseTeam >= 0) {
    if (!isFlatTop()) {
      logDebugMessage(0, "baseTeam mesh faces must be flat tops (team %i)\n",
                      sd.baseTeam);
      sd.baseTeam = -1;
    }
  }

  // setup the LinkGeometries
  if (!sd.linkName.empty()) {
    setupSpecialGeometry(sd.linkSrcGeo, true);
    setupSpecialGeometry(sd.linkDstGeo, false);
  }
}


void MeshFace::setupSpecialGeometry(LinkGeometry& geo, bool isSrc)
{
  const float zFudge = 1.0e-6f;

  const fvec3 zPos(0.0f, 0.0f, 1.0f);

  // center
  if (mesh && mesh->isValidVertex(geo.centerIndex)) {
    geo.center = mesh->getVertices()[geo.centerIndex];
  } else {
    geo.center = calcCenter();
  }

  // pDir
  if (mesh && mesh->isValidNormal(geo.pDirIndex)) {
    geo.pDir = mesh->getNormals()[geo.pDirIndex];
  } else {
    geo.pDir = isSrc ? -plane.xyz() : plane.xyz();
  }
  fvec3::normalize(geo.pDir);
  if (fabsf(geo.pDir.z) < zFudge) {
    geo.pDir.z = 0.0f; // clean up the dusty normal
  }
  const bool pZplane = (fabsf(geo.pDir.z) > 0.99985f); // about 1 degree

  // sDir
  if (mesh && mesh->isValidNormal(geo.sDirIndex)) {
    geo.sDir = mesh->getNormals()[geo.sDirIndex];
  }
  else {
    if (!pZplane) {
      geo.sDir = fvec3::cross(zPos, geo.pDir);
    } else {
      const fvec3 edge0 = *vertices[1] - *vertices[0];
      geo.sDir = isSrc ? -edge0 : edge0;
    }
  }
  fvec3::normalize(geo.sDir);
  if (fabsf(geo.sDir.z) < zFudge) {
    geo.sDir.z = 0.0f; // clean up the dusty normal
  }

  // tDir
  if (mesh && mesh->isValidNormal(geo.tDirIndex)) {
    geo.tDir = mesh->getNormals()[geo.tDirIndex];
  } else {
    geo.tDir = fvec3::cross(geo.pDir, geo.sDir);
  }
  fvec3::normalize(geo.tDir);
  if (fabsf(geo.tDir.z) < zFudge) {
    geo.tDir.z = 0.0f; // clean up the dusty normal
  }

  // sScale
  if ((geo.bits & LinkAutoSscale) != 0) {
    float minDist, maxDist;
    calcAxisSpan(geo.center, geo.sDir, minDist, maxDist);
    geo.sScale = (maxDist > -minDist) ? maxDist : -minDist;
    if (isSrc && (geo.sScale != 0.0f)) {
      geo.sScale = (1.0f / geo.sScale);
    }
  }

  // tScale
  if ((geo.bits & LinkAutoTscale) != 0) {
    float minDist, maxDist;
    calcAxisSpan(geo.center, geo.tDir, minDist, maxDist);
    geo.tScale = (maxDist > -minDist) ? maxDist : -minDist;
    if (isSrc && (geo.tScale != 0.0f)) {
      geo.tScale = (1.0f / geo.tScale);
    }
  }

  // pScale
  if ((geo.bits & LinkAutoPscale) != 0) {
    geo.pScale = 0.0f; // clamp to the plane
  }

  // angle
  if (!pZplane) {
    geo.angle = atan2f(geo.pDir.y, geo.pDir.x);
  } else {
    geo.angle = atan2f(geo.sDir.y, geo.sDir.x);
  }
}


fvec3 MeshFace::calcCenter() const
{
  fvec3 center(0.0f, 0.0f, 0.0f);
  for (int v = 0; v < vertexCount; v++) {
    center += *vertices[v];
  }
  return center * (1.0f / (float)vertexCount);
}


void MeshFace::calcAxisSpan(const fvec3& point, const fvec3& dir,
                            float& minDist, float& maxDist) const
{
  maxDist = 0.0f;
  minDist = 0.0f;
  for (int v = 0; v < vertexCount; v++) {
    float d = fvec3::dot((*vertices[v] - point), dir);
    if (minDist > d) { minDist = d; }
    if (maxDist < d) { maxDist = d; }
  }
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


//============================================================================//
//
//  SpecialData configuration routines
//

void MeshFace::setSpecialData(const SpecialData* sd)
{
  delete specialData;
  if (sd) {
    specialData = new SpecialData(*sd);
  } else {
    specialData = NULL;
  }
}


//============================================================================//
//
//  Collision routines
//

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
  if (linedot >= -1.0e-6f) {
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


bool MeshFace::getHitNormal(const fvec3& /*oldPos*/, float /*oldAngle*/,
			    const fvec3& /*newPos*/, float /*newAngle*/,
			    float /*dx*/, float /*dy*/, float /*height*/,
			    fvec3& normal) const
{
  normal = plane.xyz();
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
    fvec2 h;
    h.x = vertices[i]->x - p.x;
    h.y = vertices[i]->y - p.y;
    v[i].x = (cos_val * h.x) - (sin_val * h.y);
    v[i].y = (cos_val * h.y) + (sin_val * h.x);
    v[i].z = vertices[i]->z - p[2];
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


bool MeshFace::isCrossing(const fvec3& p, float _angle,
			  float dx, float dy, float height,
			  fvec4* planePtr) const
{
  // the position must be on the front side of the plane
  if (plane.planeDist(p) < 0.0f) {
    return false;
  }

  // the box must touch the face
  if (!inBox(p, _angle, dx, dy, height)) {
    return false;
  }

  // copy the plane
  if (planePtr) {
    *planePtr = plane;
  }

  return true;
}


//============================================================================//
//
//  Team base routines
//

fvec3 MeshFace::getRandomPoint() const
{
  // using barycentric vectors
  const fvec3 center = calcCenter();
  while (true) {
    float total = 0.0f;
    fvec3 point(0.0f, 0.0f, 0.0f);
    for (int i = 0; i < vertexCount; i++) {
      const float factor = (float)bzfrand();
      point += factor * (getVertex(i) - center);
      total += factor;
    }
    if (total > 1.0e-3f) {
      return center + (point / total);
    }
  }
}


//============================================================================//
//
//  Teleportation routines
//

float MeshFace::isTeleported(const Ray& ray) const
{
  const float t = intersect(ray);
  return (t >= 0.0f) && (t <= 1.0f);
}


bool MeshFace::hasCrossed(const fvec3& oldPos, const fvec3& newPos) const
{
  Ray ray(oldPos, newPos - oldPos);
  const float t = intersect(ray);
  return (t >= 0.0f) && (t <= 1.0f);
}


float MeshFace::getProximity(const fvec3& pIn, float radius) const
{
  float maxDist;

  // plane distance check
  maxDist = fabsf(plane.planeDist(pIn));
  if (maxDist > radius) {
    return 0.0f;
  }

  // extents check
  const Extents& exts = extents;
  if ((pIn.x < (exts.mins.x - radius)) || (pIn.x > (exts.maxs.x + radius)) ||
      (pIn.y < (exts.mins.y - radius)) || (pIn.y > (exts.maxs.y + radius)) ||
      (pIn.z < (exts.mins.z - radius)) || (pIn.z > (exts.maxs.z + radius))) {
    return 0.0f;
  }

  // check distance from each plane
  for (int i = 0; i < vertexCount; i++) {
    const float dist = edgePlanes[i].planeDist(pIn);
    if (dist > radius) {
      return 0.0;
    }
    else if (dist > maxDist) {
      maxDist = dist;
    }
  }

  return 1.0f - (maxDist / radius);
}


bool MeshFace::shotCanCross(const fvec3& /*pos*/,
                            const fvec3& vel, int team, int /*shotType*/) const
{
  if (!isSpecial()) {
    return false;
  }
  const SpecialData& sd = *specialData;
  if (sd.linkSrcShotBlockBits != 0) {
    if ((sd.linkSrcShotBlockBits & (1 << team)) != 0) {
      return false;
    }
  }
  const float speed = -fvec3::dot(plane.xyz(), vel);
  if (speed < sd.linkSrcMinSpeed) {
    return false;
  }
  if (speed > sd.linkSrcMaxSpeed) {
    return false;
  }
  return true;
}


bool MeshFace::tankCanCross(const fvec3& /*pos*/,
                            const fvec3& vel, int team) const
{
  if (!isSpecial()) {
    return false;
  }
  const SpecialData& sd = *specialData;
  if (sd.linkSrcTankBlockBits != 0) {
    if ((sd.linkSrcTankBlockBits & (1 << team)) != 0) {
      return false;
    }
  }
  const float speed = -fvec3::dot(plane.xyz(), vel);
  if (speed < sd.linkSrcMinSpeed) {
    return false;
  }
  if (speed > sd.linkSrcMaxSpeed) {
    return false;
  }
  return true;
}


bool MeshFace::teleportShot(const MeshFace& dstFace,
                            const LinkPhysics& linkPhysics,
                            const fvec3& srcPos, fvec3& dstPos,
                            const fvec3& srcVel, fvec3& dstVel) const
{
  const MeshFace& srcFace = *this;

  if (!srcFace.isLinkSrc()) {
    logDebugMessage(0, "MeshFace::teleportShot() invalid src\n");
    return false;
  }
  if (!dstFace.isLinkDst()) {
    logDebugMessage(0, "MeshFace::teleportShot() invalid dst\n");
    return false;
  }

  const SpecialData&  srcSD = *srcFace.specialData;
  const SpecialData&  dstSD = *dstFace.specialData;
  const LinkGeometry& srcGeo = srcSD.linkSrcGeo;
  const LinkGeometry& dstGeo = dstSD.linkDstGeo;

  if (debugTele && !BZDBCache::forbidDebug) {
    logDebugMessage(0, "teleportShot  %s -> %s\n",
                       srcFace.getLinkName().c_str(),
                       dstFace.getLinkName().c_str());
    linkPhysics.print(std::cout, "  linkPhysics:");
    logDebugMessage(0, "  srcPos = %s\n", srcPos.tostring().c_str());
    logDebugMessage(0, "  srcVel = %s\n", srcVel.tostring().c_str());
    const LinkGeometry& sg = srcGeo;
    logDebugMessage(0, "  srcGeo.center = %s\n", sg.center.tostring().c_str());
    logDebugMessage(0, "  srcGeo.sDir   = %s\n", sg.sDir.tostring().c_str());
    logDebugMessage(0, "  srcGeo.tDir   = %s\n", sg.tDir.tostring().c_str());
    logDebugMessage(0, "  srcGeo.pDir   = %s\n", sg.pDir.tostring().c_str());
    logDebugMessage(0, "  srcGeo.sScale = %f\n", sg.sScale);
    logDebugMessage(0, "  srcGeo.tScale = %f\n", sg.tScale);
    logDebugMessage(0, "  srcGeo.pScale = %f\n", sg.pScale);
    logDebugMessage(0, "  srcGeo.angle  = %f\n", sg.angle);
  }

  // position
  const float sScale = linkPhysics.shotSrcPosScale.x * srcGeo.sScale;
  const float tScale = linkPhysics.shotSrcPosScale.y * srcGeo.tScale;
  const float pScale = linkPhysics.shotSrcPosScale.z * srcGeo.pScale;
  const fvec3 relPos = (srcPos - srcGeo.center);
  const float sLen = sScale * fvec3::dot(relPos, srcGeo.sDir);
  const float tLen = tScale * fvec3::dot(relPos, srcGeo.tDir);
  const float pLen = pScale * fvec3::dot(relPos, srcGeo.pDir);
  fvec3 p = dstGeo.center + (sLen * dstGeo.sScale * dstGeo.sDir)
                          + (tLen * dstGeo.tScale * dstGeo.tDir)
                          + (pLen * dstGeo.pScale * dstGeo.tDir);
  p += dstGeo.pDir * 0.001f; // move forwards 1mm
  dstPos = p;

  // velocity
  const fvec3& srcVelScale = linkPhysics.shotSrcVelScale;
  fvec3 vel = (dstGeo.sDir * (srcVelScale.x * fvec3::dot(srcVel, srcGeo.sDir)))
            + (dstGeo.tDir * (srcVelScale.y * fvec3::dot(srcVel, srcGeo.tDir)))
            + (dstGeo.pDir * (srcVelScale.z * fvec3::dot(srcVel, srcGeo.pDir)));
  vel += linkPhysics.shotDstVel;
  if (linkPhysics.shotSameSpeed) {
    const float srcSpeed = srcVel.length();
    const float dstSpeed =    vel.length();
    if (dstSpeed > 0.0f) {
      vel *= (srcSpeed / dstSpeed);
    }
  }
  dstVel = vel;

  if (debugTele && !BZDBCache::forbidDebug) {
    logDebugMessage(0, "  dstPos = %s\n", dstPos.tostring().c_str());
    logDebugMessage(0, "  dstVel = %s\n", dstVel.tostring().c_str());
    const LinkGeometry& dg = dstGeo;
    logDebugMessage(0, "  dstGeo.center = %s\n", dg.center.tostring().c_str());
    logDebugMessage(0, "  dstGeo.sDir   = %s\n", dg.sDir.tostring().c_str());
    logDebugMessage(0, "  dstGeo.tDir   = %s\n", dg.tDir.tostring().c_str());
    logDebugMessage(0, "  dstGeo.pDir   = %s\n", dg.pDir.tostring().c_str());
    logDebugMessage(0, "  dstGeo.sScale = %f\n", dg.sScale);
    logDebugMessage(0, "  dstGeo.tScale = %f\n", dg.tScale);
    logDebugMessage(0, "  dstGeo.pScale = %f\n", dg.pScale);
    logDebugMessage(0, "  dstGeo.angle  = %f\n", dg.angle);
  }

  return true;
}


bool MeshFace::teleportTank(const MeshFace& dstFace,
                            const LinkPhysics& linkPhysics,
                            const fvec3& srcPos,    fvec3& dstPos,
                            const fvec3& srcVel,    fvec3& dstVel,
                            const float& srcAngle,  float& dstAngle,
                            const float& srcAngVel, float& dstAngVel) const
{
  const MeshFace& srcFace = *this;

  if (!srcFace.isLinkSrc()) {
    logDebugMessage(0, "MeshFace::teleportTank() invalid src\n");
    return false;
  }
  if (!dstFace.isLinkDst()) {
    logDebugMessage(0, "MeshFace::teleportTank() invalid dst\n");
    return false;
  }

  const SpecialData&  srcSD = *srcFace.specialData;
  const SpecialData&  dstSD = *dstFace.specialData;
  const LinkGeometry& srcGeo = srcSD.linkSrcGeo;
  const LinkGeometry& dstGeo = dstSD.linkDstGeo;

  if (debugTele) {
    logDebugMessage(0, "teleportTank  %s -> %s\n",
                       srcFace.getLinkName().c_str(),
                       dstFace.getLinkName().c_str());
    linkPhysics.print(std::cout, "  linkPhysics:");
    logDebugMessage(0, "  srcPos = %s\n", srcPos.tostring().c_str());
    logDebugMessage(0, "  srcVel = %s\n", srcVel.tostring().c_str());
    const LinkGeometry& sg = srcGeo;
    logDebugMessage(0, "  srcGeo.center = %s\n", sg.center.tostring().c_str());
    logDebugMessage(0, "  srcGeo.sDir   = %s\n", sg.sDir.tostring().c_str());
    logDebugMessage(0, "  srcGeo.tDir   = %s\n", sg.tDir.tostring().c_str());
    logDebugMessage(0, "  srcGeo.pDir   = %s\n", sg.pDir.tostring().c_str());
    logDebugMessage(0, "  srcGeo.sScale = %f\n", sg.sScale);
    logDebugMessage(0, "  srcGeo.tScale = %f\n", sg.tScale);
    logDebugMessage(0, "  srcGeo.pScale = %f\n", sg.pScale);
    logDebugMessage(0, "  srcGeo.angle  = %f\n", sg.angle);
  }

  // position
  const float sScale = linkPhysics.tankSrcPosScale.x * srcGeo.sScale;
  const float tScale = linkPhysics.tankSrcPosScale.y * srcGeo.tScale;
  const float pScale = linkPhysics.tankSrcPosScale.z * srcGeo.pScale;
  fvec3 relPos = (srcPos - srcGeo.center);
  const float sLen = sScale * fvec3::dot(relPos, srcGeo.sDir);
  const float tLen = tScale * fvec3::dot(relPos, srcGeo.tDir);
  const float pLen = pScale * fvec3::dot(relPos, srcGeo.pDir);
  fvec3 p = dstGeo.center + (sLen * dstGeo.sScale * dstGeo.sDir)
                          + (tLen * dstGeo.tScale * dstGeo.tDir)
                          + (pLen * dstGeo.pScale * dstGeo.tDir);
  p += dstGeo.pDir * 0.001f; // move forwards 1mm
  dstPos = p;

  // velocity
  const fvec3& srcVelScale = linkPhysics.tankSrcVelScale;
  fvec3 vel = (dstGeo.sDir * (srcVelScale.x * fvec3::dot(srcVel, srcGeo.sDir)))
            + (dstGeo.tDir * (srcVelScale.y * fvec3::dot(srcVel, srcGeo.tDir)))
            + (dstGeo.pDir * (srcVelScale.z * fvec3::dot(srcVel, srcGeo.pDir)));
  vel += linkPhysics.tankDstVel;
  if (linkPhysics.tankSameSpeed) {
    const float srcSpeed = srcVel.length();
    const float dstSpeed =    vel.length();
    if (dstSpeed > 0.0f) {
      vel *= (srcSpeed / dstSpeed);
    }
  }
  dstVel = vel;

  // angle
  if (linkPhysics.tankForceAngle) {
    dstAngle = linkPhysics.tankAngle;
  } else {
    dstAngle = srcAngle + (dstGeo.angle - srcGeo.angle);
    dstAngle += linkPhysics.tankAngleOffset;
  }

  // angular velocity
  if (linkPhysics.tankForceAngVel) {
    dstAngVel = linkPhysics.tankAngVel;
  } else {
    dstAngVel = linkPhysics.tankAngVelScale * srcAngVel ;
    dstAngVel += linkPhysics.tankAngVelOffset;
  }

  if (debugTele) {
    logDebugMessage(0, "  dstPos = %s\n", dstPos.tostring().c_str());
    logDebugMessage(0, "  dstVel = %s\n", dstVel.tostring().c_str());
    const LinkGeometry& dg = dstGeo;
    logDebugMessage(0, "  dstGeo.center = %s\n", dg.center.tostring().c_str());
    logDebugMessage(0, "  dstGeo.sDir   = %s\n", dg.sDir.tostring().c_str());
    logDebugMessage(0, "  dstGeo.tDir   = %s\n", dg.tDir.tostring().c_str());
    logDebugMessage(0, "  dstGeo.pDir   = %s\n", dg.pDir.tostring().c_str());
    logDebugMessage(0, "  dstGeo.sScale = %f\n", dg.sScale);
    logDebugMessage(0, "  dstGeo.tScale = %f\n", dg.tScale);
    logDebugMessage(0, "  dstGeo.pScale = %f\n", dg.pScale);
    logDebugMessage(0, "  dstGeo.angle  = %f\n", dg.angle);
  }

  return true;
}


//============================================================================//
//
//  Pack/Unpack routines
//

void *MeshFace::pack(void *buf) const
{
  // state byte
  uint16_t stateBytes = 0;
  stateBytes |= useNormals()     ? (1 << 0) : 0;
  stateBytes |= useTexcoords()   ? (1 << 1) : 0;
  stateBytes |= isDriveThrough() ? (1 << 2) : 0;
  stateBytes |= isShootThrough() ? (1 << 3) : 0;
  stateBytes |= smoothBounce     ? (1 << 4) : 0;
  stateBytes |= noclusters       ? (1 << 5) : 0;
  stateBytes |= canRicochet()    ? (1 << 6) : 0;
  stateBytes |= specialData      ? (1 << 7) : 0;
  buf = nboPackUInt16(buf, stateBytes);

  buf = nboPackStdString(buf, name);

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

  if (specialData) {
    buf = nboPackUInt16(buf, specialData->stateBits);

    int32_t tmpTeam = specialData->baseTeam;
    buf = nboPackInt32(buf, tmpTeam);

    buf = nboPackStdString(buf, specialData->linkName);

    if (!specialData->linkName.empty()) {
      // linkSrc geometry
      buf = nboPackUInt8(buf, specialData->linkSrcGeo.bits);
      buf = nboPackInt32(buf, specialData->linkSrcGeo.centerIndex);
      buf = nboPackInt32(buf, specialData->linkSrcGeo.sDirIndex);
      buf = nboPackInt32(buf, specialData->linkSrcGeo.tDirIndex);
      buf = nboPackInt32(buf, specialData->linkSrcGeo.pDirIndex);
      buf = nboPackFloat(buf, specialData->linkSrcGeo.sScale);
      buf = nboPackFloat(buf, specialData->linkSrcGeo.tScale);
      buf = nboPackFloat(buf, specialData->linkSrcGeo.pScale);
      // linkDst geometry
      buf = nboPackUInt8(buf, specialData->linkDstGeo.bits);
      buf = nboPackInt32(buf, specialData->linkDstGeo.centerIndex);
      buf = nboPackInt32(buf, specialData->linkDstGeo.sDirIndex);
      buf = nboPackInt32(buf, specialData->linkDstGeo.tDirIndex);
      buf = nboPackInt32(buf, specialData->linkDstGeo.pDirIndex);
      buf = nboPackFloat(buf, specialData->linkDstGeo.sScale);
      buf = nboPackFloat(buf, specialData->linkDstGeo.tScale);
      buf = nboPackFloat(buf, specialData->linkDstGeo.pScale);
      // special linkSrc properties
      buf = nboPackFloat(buf, specialData->linkSrcMinSpeed);
      buf = nboPackFloat(buf, specialData->linkSrcMaxSpeed);
      buf = nboPackUInt8(buf, specialData->linkSrcShotBlockBits);
      buf = nboPackUInt8(buf, specialData->linkSrcTankBlockBits);
    }
  }

  return buf;
}


void *MeshFace::unpack(void *buf)
{
  int32_t inTmp;
  driveThrough = shootThrough = smoothBounce = false;
  // state byte
  bool tmpNormals, tmpTexcoords, hasSpecial;
  uint16_t stateBytes = 0;
  buf = nboUnpackUInt16(buf, stateBytes);
  tmpNormals   =  (stateBytes & (1 << 0)) != 0;
  tmpTexcoords =  (stateBytes & (1 << 1)) != 0;
  driveThrough = ((stateBytes & (1 << 2)) != 0) ? 0xFF : 0;
  shootThrough = ((stateBytes & (1 << 3)) != 0) ? 0xFF : 0;
  smoothBounce =  (stateBytes & (1 << 4)) != 0;
  noclusters   =  (stateBytes & (1 << 5)) != 0;
  ricochet     =  (stateBytes & (1 << 6)) != 0;
  hasSpecial   =  (stateBytes & (1 << 7)) != 0;

  buf = nboUnpackStdString(buf, name);

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

  if (hasSpecial) {
    specialData = new SpecialData;

    buf = nboUnpackUInt16(buf, specialData->stateBits);

    int32_t baseTeam;
    buf = nboUnpackInt32(buf, baseTeam);
    specialData->baseTeam = (int)baseTeam;

    buf = nboUnpackStdString(buf, specialData->linkName);

    if (!specialData->linkName.empty()) {
      // linkSrc geometry
      buf = nboUnpackUInt8(buf, specialData->linkSrcGeo.bits);
      buf = nboUnpackInt32(buf, specialData->linkSrcGeo.centerIndex);
      buf = nboUnpackInt32(buf, specialData->linkSrcGeo.sDirIndex);
      buf = nboUnpackInt32(buf, specialData->linkSrcGeo.tDirIndex);
      buf = nboUnpackInt32(buf, specialData->linkSrcGeo.pDirIndex);
      buf = nboUnpackFloat(buf, specialData->linkSrcGeo.sScale);
      buf = nboUnpackFloat(buf, specialData->linkSrcGeo.tScale);
      buf = nboUnpackFloat(buf, specialData->linkSrcGeo.pScale);
      // linkDst geometry
      buf = nboUnpackUInt8(buf, specialData->linkDstGeo.bits);
      buf = nboUnpackInt32(buf, specialData->linkDstGeo.centerIndex);
      buf = nboUnpackInt32(buf, specialData->linkDstGeo.sDirIndex);
      buf = nboUnpackInt32(buf, specialData->linkDstGeo.tDirIndex);
      buf = nboUnpackInt32(buf, specialData->linkDstGeo.pDirIndex);
      buf = nboUnpackFloat(buf, specialData->linkDstGeo.sScale);
      buf = nboUnpackFloat(buf, specialData->linkDstGeo.tScale);
      buf = nboUnpackFloat(buf, specialData->linkDstGeo.pScale);
      // special linkSrc properties
      buf = nboUnpackFloat(buf, specialData->linkSrcMinSpeed);
      buf = nboUnpackFloat(buf, specialData->linkSrcMaxSpeed);
      buf = nboUnpackUInt8(buf, specialData->linkSrcShotBlockBits);
      buf = nboUnpackUInt8(buf, specialData->linkSrcTankBlockBits);
    }
  }

  finalize();

  return buf;
}


int MeshFace::packSize() const
{
  int fullSize = 0;
  fullSize += sizeof(uint16_t); // stateBytes

  fullSize += nboStdStringPackSize(name);

  fullSize += sizeof(int32_t);  // vertexCount
  fullSize += sizeof(int32_t) * vertexCount;
  if (useNormals()) {
    fullSize += sizeof(int32_t) * vertexCount;
  }
  if (useTexcoords()) {
    fullSize += sizeof(int32_t) * vertexCount;
  }

  fullSize += sizeof(int32_t); // material
  fullSize += sizeof(int32_t); // physics driver

  if (specialData != NULL) {
    fullSize += sizeof(uint16_t); // state
    fullSize += sizeof(int32_t);  // teamNum
    fullSize += nboStdStringPackSize(specialData->linkName);
    if (!specialData->linkName.empty()) {
      // linkSrc geometry
      fullSize += sizeof(uint8_t); // linkSrcGeo.bits
      fullSize += sizeof(int32_t); // linkSrcGeo.centerIndex
      fullSize += sizeof(int32_t); // linkSrcGeo.sDirIndex
      fullSize += sizeof(int32_t); // linkSrcGeo.tDirIndex
      fullSize += sizeof(int32_t); // linkSrcGeo.pDirIndex
      fullSize += sizeof(float);   // linkSrcGeo.sScale
      fullSize += sizeof(float);   // linkSrcGeo.tScale
      fullSize += sizeof(float);   // linkSrcGeo.pScale
      // linkDst geometry
      fullSize += sizeof(uint8_t); // linkDstGeo.bits
      fullSize += sizeof(int32_t); // linkDstGeo.center
      fullSize += sizeof(int32_t); // linkDstGeo.sDir
      fullSize += sizeof(int32_t); // linkDstGeo.tDir
      fullSize += sizeof(int32_t); // linkDstGeo.pDir
      fullSize += sizeof(float);   // linkDstGeo.sScale
      fullSize += sizeof(float);   // linkDstGeo.tScale
      fullSize += sizeof(float);   // linkDstGeo.pScale
      // special linkSrc properties
      fullSize += sizeof(float);   // linkSrcMinSpeed
      fullSize += sizeof(float);   // linkSrcMaxSpeed
      fullSize += sizeof(uint8_t); // linkSrcShotBlockBits
      fullSize += sizeof(uint8_t); // linkSrcTankBlockBits
    }
  }

  return fullSize;
}


//============================================================================//
//
//  Printing
//

static void outputBits(std::ostream& out, uint8_t bits)
{
  for (int i = 0; i < 8; i++) {
    if ((bits & (1 << i)) != 0) {
      out << " " << i;
    }
  }
}


void MeshFace::print(std::ostream& out, const std::string& indent) const
{
  if (mesh == NULL) {
    return;
  }

  int i;
  out << indent << "  face" << std::endl;

  if (!name.empty()) {
    out << indent << "    name " << name << std::endl;
  }

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
      out << " " << vertices[i]->x << " "
                 << vertices[i]->y << " "
                 << vertices[i]->z;
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
	out << " " << normals[i]->x << " "
	           << normals[i]->y << " "
	           << normals[i]->z;
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

  if (specialData) {
    const SpecialData& sd = *specialData;

    const uint16_t stateBits = sd.stateBits;

    if (sd.baseTeam >= 0) {
      out << indent << "    baseTeam " << sd.baseTeam << std::endl;
    }

    if (!sd.linkName.empty()) {
      out << indent << "    linkName " << sd.linkName << std::endl;
    }

    if ((stateBits & LinkSrcRebound) != 0) {
      out << indent << "    linkSrcRebound" << std::endl;
    }

    if ((stateBits & LinkSrcNoGlow) != 0) {
      out << indent << "    linkSrcNoGlow" << std::endl;
    }

    if ((stateBits & LinkSrcNoRadar) != 0) {
      out << indent << "    linkSrcNoRadar" << std::endl;
    }

    if ((stateBits & LinkSrcNoSound) != 0) {
      out << indent << "    linkSrcNoSound" << std::endl;
    }

    if ((stateBits & LinkSrcNoEffect) != 0) {
      out << indent << "    linkSrcNoEffect" << std::endl;
    }

    if (sd.linkSrcMinSpeed != -MAXFLOAT) {
      out << indent << "    linkSrcMinSpeed "
                    << sd.linkSrcMinSpeed << std::endl;
    }

    if (sd.linkSrcMaxSpeed != +MAXFLOAT) {
      out << indent << "    linkSrcMaxSpeed "
                    << sd.linkSrcMaxSpeed << std::endl;
    }

    if (sd.linkSrcShotBlockBits != 0) {
      out << indent << "    linkSrcShotBlockTeams ";
      outputBits(out, sd.linkSrcShotBlockBits);
      out << std::endl;
    }

    if (sd.linkSrcTankBlockBits != 0) {
      out << indent << "    linkSrcTankBlockTeams ";
      outputBits(out, sd.linkSrcTankBlockBits);
      out << std::endl;
    }

    // linkSrc
    const LinkGeometry& srcGeo = sd.linkSrcGeo;
    if (srcGeo.centerIndex >= 0) {
      out << indent << "    linkSrcCenter " << srcGeo.centerIndex << std::endl;
    }
    if (srcGeo.sDirIndex >= 0) {
      out << indent << "    linkSrcSdir " << srcGeo.sDirIndex << std::endl;
    }
    if (srcGeo.tDirIndex >= 0) {
      out << indent << "    linkSrcTdir " << srcGeo.tDirIndex << std::endl;
    }
    if (srcGeo.pDirIndex >= 0) {
      out << indent << "    linkSrcPdir " << srcGeo.pDirIndex << std::endl;
    }
    if ((srcGeo.bits & LinkAutoSscale) == 0) {
      out << indent << "    linkSrcSscale " << srcGeo.sScale << std::endl;
    }
    if ((srcGeo.bits & LinkAutoTscale) == 0) {
      out << indent << "    linkSrcTscale " << srcGeo.tScale << std::endl;
    }
    if ((srcGeo.bits & LinkAutoPscale) == 0) {
      out << indent << "    linkSrcPscale " << srcGeo.pScale << std::endl;
    }

    // linkDst
    const LinkGeometry& dstGeo = sd.linkDstGeo;
    if (dstGeo.centerIndex >= 0) {
      out << indent << "    linkDstCenter " << dstGeo.centerIndex << std::endl;
    }
    if (dstGeo.sDirIndex >= 0) {
      out << indent << "    linkDstSdir " << dstGeo.sDirIndex << std::endl;
    }
    if (dstGeo.tDirIndex >= 0) {
      out << indent << "    linkDstTdir " << dstGeo.tDirIndex << std::endl;
    }
    if (dstGeo.pDirIndex >= 0) {
      out << indent << "    linkDstPdir " << dstGeo.pDirIndex << std::endl;
    }
    if ((dstGeo.bits & LinkAutoSscale) == 0) {
      out << indent << "    linkDstSscale " << dstGeo.sScale << std::endl;
    }
    if ((dstGeo.bits & LinkAutoTscale) == 0) {
      out << indent << "    linkDstTscale " << dstGeo.tScale << std::endl;
    }
    if ((dstGeo.bits & LinkAutoPscale) == 0) {
      out << indent << "    linkDstPscale " << dstGeo.pScale << std::endl;
    }
  }

  out << indent << "  endface" << std::endl;

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
