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
#include "Pack.h"
#include "vectors.h"

#include "MeshFace.h"
#include "MeshObstacle.h"

#include "Intersect.h"


const char* MeshFace::typeName = "MeshFace";


MeshFace::MeshFace(MeshObstacle* _mesh)
{
  mesh = _mesh;
  vertexCount = 0;
  vertices = NULL;
  normals = NULL;
  texcoords = NULL;
  smoothBounce = false;
  driveThrough = false;
  shootThrough = false;

  return;
}


MeshFace::MeshFace(MeshObstacle* _mesh, int _vertexCount,
                   float** _vertices, float** _normals,
                   float** _texcoords, const MeshMaterial& _material,
                   bool bounce, bool drive, bool shoot)
{
  mesh = _mesh;
  vertexCount = _vertexCount;
  vertices = _vertices;
  normals = _normals;
  texcoords = _texcoords;
  material = _material;
  smoothBounce = bounce;
  driveThrough = drive;
  shootThrough = shoot;

  finalize();

  return;
}


void MeshFace::finalize()
{
  float maxCrossSqr = 0.0f;
  float bestCross[3] = { 0.0f, 0.0f, 0.0f };
  int bestSet[3] = { -1, -1, -1 };
  
  // find the best vertices for making the plane
  int i, j, k;
  for (i = 0; i < (vertexCount - 2); i++) {
    for (j = i; j < (vertexCount - 1); j++) {
      for (k = j; k < (vertexCount - 0); k++) {
        float edge1[3], edge2[3], cross[3];
        vec3sub(edge1, vertices[k], vertices[j]);
        vec3sub(edge2, vertices[i], vertices[j]);
        vec3cross(cross, edge1, edge2);
        const float lenSqr = vec3dot(cross, cross);
        if (lenSqr > maxCrossSqr) {
          maxCrossSqr = lenSqr;
          bestSet[0] = i;
          bestSet[1] = j;
          bestSet[2] = k;
          memcpy (bestCross, cross, sizeof(float[3]));
        }
      }
    }
  }

  if (maxCrossSqr < +1.0e-20f) {
    vertexCount = 0;
    DEBUG1("invalid mesh face\n");
    if (debugLevel >= 3) {
      print(std::cerr, 3);
    }
    return;
  }

  // make the plane
  float scale = 1.0f / sqrtf (maxCrossSqr);
  const float* vert = vertices[bestSet[1]];
  plane[0] = bestCross[0] * scale;
  plane[1] = bestCross[1] * scale;
  plane[2] = bestCross[2] * scale;
  plane[3] = -vec3dot(plane, vert);
  
  // see if the whole face is convex
  int v, a;
  for (v = 0; v < vertexCount; v++) {
    float a[3], b[3], c[3];
    vec3sub(a, vertices[(v + 1) % vertexCount], vertices[(v + 0) % vertexCount]);
    vec3sub(b, vertices[(v + 2) % vertexCount], vertices[(v + 1) % vertexCount]);
    vec3cross(c, a, b);
    const float d = vec3dot(c, plane);
    if (d <= 0.0f) {
      vertexCount = 0;
      DEBUG1("non-convex mesh face\n");
      if (debugLevel >= 3) {
        print(std::cerr, 3);
      }
      return;
    }
  }

  // make the edge planes
  edgePlanes = new fvec4[vertexCount];
  for (v = 0; v < vertexCount; v++) {
    float edge[3];
    const int next = (v + 1) % vertexCount;
    vec3sub(edge, vertices[next], vertices[v]);
    vec3cross(edgePlanes[v], edge, plane);
    const float scale = 1.0f / sqrtf(vec3dot(edgePlanes[v], edgePlanes[v]));
    edgePlanes[v][0] = edgePlanes[v][0] * scale;
    edgePlanes[v][1] = edgePlanes[v][1] * scale;
    edgePlanes[v][2] = edgePlanes[v][2] * scale;
    edgePlanes[v][3] = -vec3dot(vertices[v], edgePlanes[v]);
  }

  // setup extents
  mins[0] = mins[1] = mins[2] = +MAXFLOAT;
  maxs[0] = maxs[1] = maxs[2] = -MAXFLOAT;
  for (v = 0; v < vertexCount; v++) {
    for (a = 0; a < 3; a++) {
      if (vertices[v][a] < mins[a]) {
        mins[a] = vertices[v][a];
      }
      if (vertices[v][a] > maxs[a]) {
        maxs[a] = vertices[v][a];
      }
    }
  }

  // setup fake obstacle parameters
  pos[0] = (maxs[0] + mins[0]) / 2.0f;
  pos[1] = (maxs[1] + mins[1]) / 2.0f;
  pos[2] = mins[2];
  size[0] = (maxs[0] - mins[0]) / 2.0f;
  size[1] = (maxs[1] - mins[1]) / 2.0f;
  size[2] = (maxs[2] - mins[2]);
  angle = 0.0f;
  ZFlip = false;

  return;
}


MeshFace::~MeshFace()
{
  delete[] vertices;
  delete[] normals;
  delete[] texcoords;
  delete[] edgePlanes;
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


void MeshFace::getExtents(float* _mins, float* _maxs) const
{
  memcpy (_mins, mins, sizeof(fvec3));
  memcpy (_maxs, maxs, sizeof(fvec3));
  return;
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
  //  L - line unit vector          Lo - line origin
  //  N - plane normal unit vector  d  - plane offset
  //  P - point in question         t - time
  //
  //  (N dot P) + d = 0                      { plane equation }
  //  P = (t * L) + Lo                       { line equation }
  //  t (N dot L) + (N dot Lo) + d = 0
  //
  //  t = - (d + (N dot Lo)) / (N dot L)     { time of impact }
  //
  const float* dir = ray.getDirection();
  const float* origin = ray.getOrigin();
  float hitTime;

  // get the time until the shot would hit each plane
  const float linedot = (plane[0] * dir[0]) +
                        (plane[1] * dir[1]) +
                        (plane[2] * dir[2]);
  if (linedot >= -0.001f) {
    // shot is either parallel, or going through backwards
    return -1.0f;
  }
  const float origindot = (plane[0] * origin[0]) +
                          (plane[1] * origin[1]) +
                          (plane[2] * origin[2]);
  // linedot should be safe to divide with now
  hitTime = - (plane[3] + origindot) / linedot;
  if (hitTime < 0.0f) {
    return -1.0f;
  }

  // get the contact location
  float point[3];
  point[0] = (dir[0] * hitTime) + origin[0];
  point[1] = (dir[1] * hitTime) + origin[1];
  point[2] = (dir[2] * hitTime) + origin[2];

  // now test against the edge planes
  for (int q = 0; q < vertexCount; q++) {
    float d = (edgePlanes[q][0] * point[0]) +
              (edgePlanes[q][1] * point[1]) +
              (edgePlanes[q][2] * point[2]) + edgePlanes[q][3];
    if (d > 0.001f) {
      return -1.0f;
    }
  }

  return hitTime;
}


void MeshFace::get3DNormal(const float* p, float* n) const
{
  if (!smoothBounce || !useNormals()) {
    // just use the plain normal
    memcpy (n, plane, sizeof(float[3]));
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
      float ea[3], eb[3], cross[3];
      vec3sub(ea, p, vertices[i]);
      vec3sub(eb, vertices[next], vertices[i]);
      vec3cross(cross, ea, eb);
      areas[i] = sqrtf(vec3dot(cross, cross));
      totalArea = totalArea + areas[i];
    }
    float smallestArea = MAXFLOAT;
    float* twinAreas = new float[vertexCount];
    for (i = 0; i < vertexCount; i++) {
      int next = (i + 1) % vertexCount;
      twinAreas[i] = areas[i] + areas[next];
      if (twinAreas[i] < 1.0e-10f) {
        memcpy (n, normals[next], sizeof(float[3]));
        delete[] areas;
        delete[] twinAreas;
        return;
      }
      if (twinAreas[i] < smallestArea) {
        smallestArea = twinAreas[i];
      }
    }
    float normal[3] = {0.0f, 0.0f, 0.0f};
    for (i = 0; i < vertexCount; i++) {
      int next = (i + 1) % vertexCount;
      float factor = smallestArea / twinAreas[i];
      normal[0] = normal[0] + (normals[next][0] * factor);
      normal[1] = normal[1] + (normals[next][1] * factor);
      normal[2] = normal[2] + (normals[next][2] * factor);
    }
    float len = sqrtf(vec3dot(normal, normal));
    if (len < 1.0e-10) {
      memcpy (n, plane, sizeof(float[3]));
      delete[] areas;
      delete[] twinAreas;
      return;
    }
    len = 1.0f / len;
    n[0] = normal[0] * len;
    n[1] = normal[1] * len;
    n[2] = normal[2] * len;

    delete[] areas;
    delete[] twinAreas;
  }
  
  return;
}


void MeshFace::getNormal(const float* /*p*/, float* n) const
{
  memcpy (n, plane, sizeof(float[3]));
  n[0] = 0.0f;
  n[1] = 0.0f;
  n[2] = 1.0f;
  return;
}


/////////////////////////////////////////////////////////////
//  FIXME - everything after this point is currently JUNK! //
/////////////////////////////////////////////////////////////


bool MeshFace::getHitNormal(const float* pos1, float,
                            const float* pos2, float,
			    float, float, float height,
			    float* normal) const
{
  pos1 = pos2;
  height = height;
  memcpy (normal, plane, sizeof(float[3]));
  normal[0] = 0.0f;
  normal[1] = 0.0f;
  normal[2] = 1.0f;
  return false;
}


bool MeshFace::inCylinder(const float* p,float radius, float height) const
{
  return inBox(p, 0.0f, radius, radius, height);
}


bool MeshFace::inBox(const float* p, float angle,
                          float dx, float dy, float height) const
{
  if ((mins[0] > (p[0] + dx)) || (maxs[0] < (p[0] - dx)) ||
      (mins[1] > (p[1] + dy)) || (maxs[1] < (p[1] - dy)) ||
      (mins[2] > (p[2] + height)) || (maxs[2] < p[2])) {
    return false;
  }
  angle = angle; //FIXME
  return true;
}


bool MeshFace::inMovingBox(const float*, float,
                                const float* p, float angle,
                                float dx, float dy, float height) const
{
  return inBox(p, angle, dx, dy, height);
}


// This is only used when the player has an Oscillation Overthruster
// flag, and only after we already know that the tank is interfering
// with this tetrahedron, so it doesn't have to be particularly fast.
// As a note, some of the info from the original collision test might
// be handy here.
bool MeshFace::isCrossing(const float* p, float angle,
                               float dx, float dy, float height,
                               float* plane) const
{
  p = p;
  angle = angle;
  dx = dy = height;
  plane = plane;
  return false;
}


void *MeshFace::pack(void *buf)
{
  // state byte
  unsigned char stateByte = 0;
  if (useNormals()) {
    stateByte = stateByte | (1 << 0);
  }
  if (useTexcoords()) {
    stateByte = stateByte | (1 << 1);
  }
  if (isDriveThrough()) {
    stateByte = stateByte | (1 << 2);
  }
  if (isShootThrough()) {
    stateByte = stateByte | (1 << 3);
  }
  if (smoothBounce) {
    stateByte = stateByte | (1 << 4);
  }
  buf = nboPackUByte(buf, stateByte);
  // vertices
  buf = nboPackInt(buf, vertexCount);
  for (int i = 0; i < vertexCount; i++) {
    int index = (fvec3*)vertices[i] - mesh->getVertices();
    buf = nboPackInt(buf, index);
  }
  // normals
  if (useNormals()) {
    for (int i = 0; i < vertexCount; i++) {
      int index = (fvec3*)normals[i] - mesh->getNormals();
      buf = nboPackInt(buf, index);
    }
  }
  // texcoords
  if (useTexcoords()) {
    for (int i = 0; i < vertexCount; i++) {
      int index = (fvec2*)texcoords[i] - mesh->getTexcoords();
      buf = nboPackInt(buf, index);
    }
  }
  // material
  buf = material.pack(buf);

  return buf;
}


void *MeshFace::unpack(void *buf)
{
  // state byte
  unsigned char stateByte = 0;
  buf = nboUnpackUByte(buf, stateByte);
  if (stateByte & (1 << 2)) {
    driveThrough = true;
  }
  if (stateByte & (1 << 3)) {
    shootThrough = true;
  }
  if (stateByte & (1 << 4)) {
    smoothBounce = true;
  }
  // vertices
  buf = nboUnpackInt(buf, vertexCount);
  vertices = new float*[vertexCount];
  for (int i = 0; i < vertexCount; i++) {
    int index;
    buf = nboUnpackInt(buf, index);
    vertices[i] = (float*)mesh->getVertices()[index];
  }
  // normals
  if (stateByte & (1 << 0)) {
    normals = new float*[vertexCount];
    for (int i = 0; i < vertexCount; i++) {
      int index;
      buf = nboUnpackInt(buf, index);
      normals[i] = (float*)mesh->getNormals()[index];
    }
  }
  // texcoords
  if (stateByte & (1 << 1)) {
    texcoords = new float*[vertexCount];
    for (int i = 0; i < vertexCount; i++) {
      int index;
      buf = nboUnpackInt(buf, index);
      texcoords[i] = (float*)mesh->getTexcoords()[index];
    }
  }
  // material
  buf = material.unpack(buf);

  finalize();

  return buf;
}


int MeshFace::packSize()
{
  int fullSize = sizeof(unsigned char);
  fullSize += sizeof(int);
  fullSize += sizeof(int) * vertexCount;
  if (useNormals()) {
    fullSize += sizeof(int) * vertexCount;
  }
  if (useTexcoords()) {
    fullSize += sizeof(int) * vertexCount;
  }
  fullSize += material.packSize();

  return fullSize;
}


void MeshFace::print(std::ostream& out, int level)
{
  int i;
  out << "  face" << std::endl;

  if (level > 1) {
    out << "  # plane normal = " << plane[0] << " " << plane[1] << " "
                                 << plane[2] << " " << plane[3] << std::endl;
  }

  out << "    vertices";
  for (i = 0; i < vertexCount; i++) {
    int index = (fvec3*)vertices[i] - mesh->getVertices();
    out << " " << index;
  }
  if (level > 1) {
    out << " #";
    for (i = 0; i < vertexCount; i++) {
      out << " " << vertices[i][0] << " " << vertices[i][1] << " " << vertices[i][2];
    }
  }
  out << std::endl;

  if (normals != NULL) {
    out << "    normals";
    for (i = 0; i < vertexCount; i++) {
      int index = (fvec3*)normals[i] - mesh->getNormals();
      out << " " << index;
    }
    if (level > 1) {
      out << " #";
      for (i = 0; i < vertexCount; i++) {
        out << " " << normals[i][0] <<  " " << normals[i][1] << " " << normals[i][2];
      }
    }
    out << std::endl;
  }

  if (texcoords != NULL) {
    out << "    texcoords";
    for (i = 0; i < vertexCount; i++) {
      int index = (fvec2*)texcoords[i] - mesh->getTexcoords();
      out << " " << index;
    }
    if (level > 1) {
      out << " #";
      for (i = 0; i < vertexCount; i++) {
        out << " " << texcoords[i][0] <<  " " << texcoords[i][1];
      }
    }
    out << std::endl;
  }

  material.print(out, level);
  
  if (smoothBounce && !mesh->hasSmoothBounce()) {
    out << "    smoothBounce" << std::endl;
  }
  if (driveThrough && !mesh->isDriveThrough()) {
    out << "    driveThrough" << std::endl;
  }
  if (shootThrough && !mesh->isShootThrough()) {
    out << "    shootThrough" << std::endl;
  }

  out << "  endface" << std::endl;

  return;
}


// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

