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
#include "TetraBuilding.h"
#include "Intersect.h"


const char*		TetraBuilding::typeName = "TetraBuilding";

static bool makePlane (const float* p1, const float* p2, const float* pc, float* r);


TetraBuilding::TetraBuilding(const float (*_vertices)[3], const bool *_visible,
                             const bool *_colored, const float (*_colors)[4],
			     bool drive, bool shoot)
{
  int v, a;

  memcpy (vertices, _vertices, 4 * sizeof (float[3]));
  memcpy (visible, _visible, 4 * sizeof (bool));
  memcpy (colored, _colored, 4 * sizeof (bool));
  memcpy (colors, _colors, 4 * sizeof (float[4]));

  // make sure the the planes are facing outwards
  float edge[3][3]; // edges from vertex 0
  for (v = 0; v < 3; v++) {
    for (a = 0; a < 3; a++) {
      edge[v][a] = vertices[v+1][a] - vertices[0][a];
    }
  }
  float cross[3];
  cross[0] = (edge[0][1] * edge[1][2]) - (edge[0][2] * edge[1][1]);
  cross[1] = (edge[0][2] * edge[1][0]) - (edge[0][0] * edge[1][2]);
  cross[2] = (edge[0][0] * edge[1][1]) - (edge[0][1] * edge[1][0]);
  float dot =
    (cross[0] * edge[2][0]) + (cross[1] * edge[2][1]) + (cross[2] * edge[2][2]);
  // swap vertices 1 & 2 if we are out of order
  if (dot < 0.0f) {
    memcpy (vertices[1], _vertices[2], sizeof(float[3]));
    memcpy (vertices[2], _vertices[1], sizeof(float[3]));
    visible[1] = _visible[2];
    visible[2] = _visible[1];
  }

  // make outward facing normals to the planes
  if (!makePlane (vertices[1], vertices[2], vertices[3], planes[0]) ||
      !makePlane (vertices[0], vertices[3], vertices[2], planes[1]) ||
      !makePlane (vertices[0], vertices[1], vertices[3], planes[2]) ||
      !makePlane (vertices[0], vertices[2], vertices[1], planes[3])) {
    // trigger isValid() to return false;
    for (v = 0; v < 4; v++) {
      for (a = 0; a < 3; a++) {
        vertices[v][a] = 0.0f;
      }
    }
  }

  // setup the extents
  mins[0] = mins[1] = mins[2] = +Infinity;
  maxs[0] = maxs[1] = maxs[2] = -Infinity;
  for (v = 0; v < 4; v++) {
    const float* vertex = vertices[v];
    for (a = 0; a < 3; a++) {
      if (vertex[a] < mins[a]) {
        mins[a] = vertex[a];
      }
      if (vertex[a] > maxs[a]) {
        maxs[a] = vertex[a];
      }
    }
  }

  // setup the basic obstacle parameters
  pos[0] = 0.5f * (maxs[0] + mins[0]);
  pos[1] = 0.5f * (maxs[1] + mins[1]);
  pos[2] = mins[2];
  size[0] = 0.5f * (maxs[0] - mins[0]);
  size[1] = 0.5f * (maxs[1] - mins[1]);
  size[2] = maxs[2] - mins[2];
  angle = 0.0f;
  driveThrough = drive;
  shootThrough = shoot;
  ZFlip = false;

  return;
}


TetraBuilding::~TetraBuilding()
{
  // do nothing
  return;
}


const char* TetraBuilding::getType() const
{
  return typeName;
}


const char* TetraBuilding::getClassName() // const
{
  return typeName;
}


bool TetraBuilding::isValid() const
{
  int v, a;

  float edge[3][3]; // edges from vertex 0
  for (v = 0; v < 3; v++) {
    for (a = 0; a < 3; a++) {
      edge[v][a] = vertices[v+1][a] - vertices[0][a];
    }
  }
  float cross[3];
  cross[0] = (edge[0][1] * edge[1][2]) - (edge[0][2] * edge[1][1]);
  cross[1] = (edge[0][2] * edge[1][0]) - (edge[0][0] * edge[1][2]);
  cross[2] = (edge[0][0] * edge[1][1]) - (edge[0][1] * edge[1][0]);
  float dot =
    (cross[0] * edge[2][0]) + (cross[1] * edge[2][1]) + (cross[2] * edge[2][2]);

  if (fabsf(dot) < 0.0001) {
    return false; // tetrahedrons require a volume
  }

  // now check the vertices
  for (v = 0; v < 4; v++) {
    for (a = 0; a < 3; a++) {
      if (fabsf(vertices[v][a]) > maxExtent) {
        return false;
      }
    }
  }

  return true;
}


void TetraBuilding::getExtents(float* _mins, float* _maxs) const
{
  memcpy (_mins, mins, sizeof(float[3]));
  memcpy (_maxs, maxs, sizeof(float[3]));
  return;
}


void TetraBuilding::getCorner(int index, float* pos) const
{
  memcpy (pos, vertices[index], 3 * sizeof(float));
  return;
}


static bool makePlane (const float* p1, const float* p2, const float* pc,
                       float* r)
{
  // make vectors from points
  float x[3] = {p1[0] - pc[0], p1[1] - pc[1], p1[2] - pc[2]};
  float y[3] = {p2[0] - pc[0], p2[1] - pc[1], p2[2] - pc[2]};
  float n[3];

  // cross product to get the normal
  n[0] = (x[1] * y[2]) - (x[2] * y[1]);
  n[1] = (x[2] * y[0]) - (x[0] * y[2]);
  n[2] = (x[0] * y[1]) - (x[1] * y[0]);

  // normalize
  float len = (n[0] * n[0]) + (n[1] * n[1]) + (n[2] * n[2]);
  if (len < +0.001f) {
    return false;
  } else {
    len = 1.0f / sqrtf (len);
  }
  r[0] = n[0] * len;
  r[1] = n[1] * len;
  r[2] = n[2] * len;

  // finish the plane equation: {rx*px + ry*py + rz+pz + rd = 0}
  r[3] = -((pc[0] * r[0]) + (pc[1] * r[1]) + (pc[2] * r[2]));

  return true;
}


float TetraBuilding::intersect(const Ray& ray) const
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
  int p;
  const float* dir = ray.getDirection();
  const float* origin = ray.getOrigin();
  float times[4];

  // get the time until the shot would hit each plane
  for (p = 0; p < 4; p++) {
    const float linedot = (planes[p][0] * dir[0]) +
                          (planes[p][1] * dir[1]) +
                          (planes[p][2] * dir[2]);
    if (linedot >= -0.001f) {
      // shot is either parallel, or going through backwards
      times[p] = Infinity;
      continue;
    }
    const float origindot = (planes[p][0] * origin[0]) +
                            (planes[p][1] * origin[1]) +
                            (planes[p][2] * origin[2]);
    // linedot should be safe to divide with now
    times[p] = - (planes[p][3] + origindot) / linedot;
    if (times[p] < 0.0f) {
      times[p] = Infinity;
    }
  }

  // sort, smallest time first - FIXME (ick, bubble sort)
  int order[4] = { 0, 1, 2, 3 };
  for (int i = 3; i > 0; i--) {
    for (int j = 3; j > (3 - i); j--)
      if (times[order[j]] < times[order[j - 1]]) {
        int tmp = order[j];
        order[j] = order[j - 1];
        order[j - 1] = tmp;
    }
  }

  // see if the point is within the face
  for (p = 0; p < 4; p++) {
    int target = order[p];
    if (times[target] == Infinity) {
      continue;
    }
    float targetTime = times[target];
    // get the contact location
    float point[3];
    point[0] = (dir[0] * targetTime) + origin[0];
    point[1] = (dir[1] * targetTime) + origin[1];
    point[2] = (dir[2] * targetTime) + origin[2];
    bool gotFirstHit = true;
    // now test against the planes
    for (int q = 0; q < 4; q++) {
      if (q == target) {
        continue;
      }
      float d = (planes[q][0] * point[0]) +
                (planes[q][1] * point[1]) +
                (planes[q][2] * point[2]) + planes[q][3];
      if (d > 0.001f) {
        gotFirstHit = false;
        break;
      }
    }
    if (gotFirstHit) {
      lastPlaneShot = p;
      return targetTime;
    }
  }

  return -1.0f;
}


void TetraBuilding::get3DNormal(const float* /*p*/, float* n) const
{
  // intersect() must be called on this obstacle
  // before this function can be used.
  memcpy (n, planes[lastPlaneShot], sizeof(float[3]));
  return;
}




/////////////////////////////////////////////////////////////
//  FIXME - everything after this point is currently JUNK! //
/////////////////////////////////////////////////////////////



void TetraBuilding::getNormal(const float* p, float* n) const
{
  p = p;
  n[0] = 0.0f;
  n[1] = 0.0f;
  n[2] = +1.0f;
}


bool TetraBuilding::getHitNormal(const float* pos1, float,
				 const float* pos2, float,
				 float, float, float height,
				 float* normal) const
{
  pos1 = pos1;
  pos2 = pos2;
  height = height;
  normal[0] = 0.0f;
  normal[1] = 0.0f;
  normal[2] = +1.0f;
  return false;
}


bool TetraBuilding::inCylinder(const float* p,
                               float radius, float height) const
{
  if (((p[0] + radius) < mins[0]) || ((p[0] - radius) > maxs[0]) ||
      ((p[1] + radius) < mins[1]) || ((p[1] - radius) > maxs[1]) ||
      ((p[2] + height) < mins[2]) || (p[2] > maxs[2])) {
    return false;
  }
  return true;
}


bool TetraBuilding::inBox(const float* p, float angle,
                          float dx, float dy, float height) const
{
  angle = sqrtf ((dx * dx) + (dy * dy));
  return inCylinder (p, angle, height);
}


bool TetraBuilding::inMovingBox(const float*, float,
                                const float* p, float angle,
                                float dx, float dy, float height) const
{
  angle = sqrtf ((dx * dx) + (dy * dy));
  return inCylinder (p, angle, height);
}


// This is only used when the player has an Oscillation Overthruster
// flag, and only after we already know that the tank is interfering
// with this tetrahedron, so it doesn't have to be particularly fast.
// As a note, some of the info from the original collision test might
// be handy here.
bool TetraBuilding::isCrossing(const float* p, float angle,
                               float dx, float dy, float height,
                               float* plane) const
{
  float corner[8][3]; // the box corners
  const float cosval = cos(angle);
  const float sinval = sin(angle);
  int bv, tp; // box vertices, tetra planes

  // make the box vertices
  corner[0][0] = (cosval * dx) - (sinval * dy);
  corner[1][0] = (cosval * dx) + (sinval * dy);
  corner[0][1] = (cosval * dy) + (sinval * dx);
  corner[1][1] = (cosval * dy) - (sinval * dx);
  for (bv = 0; bv < 2; bv++) {
    corner[bv + 2][0] = -corner[bv][0];
    corner[bv + 2][1] = -corner[bv][1];
  }
  for (bv = 0; bv < 4; bv++) {
    corner[bv][0] = p[0] + corner[bv][0];
    corner[bv][1] = p[1] + corner[bv][1];
    corner[bv][2] = p[2];
    corner[bv + 4][0] = corner[bv][0];
    corner[bv + 4][1] = corner[bv][1];
    corner[bv + 4][2] = corner[bv][2] + height;
  }

  // see if any tetra planes separate the box vertices
  bool done = false;
  for (tp = 0; tp < 4; tp++) {
    int splitdir = 0;
    for (bv = 0; bv < 8; bv++) {
      const float d = (corner[bv][0] * planes[tp][0]) +
                      (corner[bv][1] * planes[tp][1]) +
                      (corner[bv][2] * planes[tp][2]) + planes[tp][3];

      int newdir = (d > 0.0f) ? +1 : -1;

      if (bv == 0) {
        splitdir = newdir;
      }
      else if (splitdir != newdir) {
        done = true;
        break;
      }
    }
    if (done) {
      break;
    }
  }

  // copy the plane information if requested
  if (tp < 4) {
    if (plane != NULL) {
      memcpy (plane, planes[tp], sizeof(float[4]));
    }
    return true;
  }

  return false;
}

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

