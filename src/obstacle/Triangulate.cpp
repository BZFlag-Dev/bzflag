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


// top dog
#include "common.h"

// implementation header
#include "Triangulate.h"

// system headers
#include <string.h>
#include <vector>

// common headers
#include "vectors.h"


// triangulation parameters
static fvec3 Normal; // FIXME, uNormal, vNormal;
//static fvec2* MVertsSpace = NULL;
static const fvec3** Verts = NULL;
static int Count = 0;
static int* WorkSet = NULL;


static inline bool makeNormal()
{
  // Newell method
  Normal[0] = Normal[1] = Normal[2] = 0.0f;
  for (int i = 0; i < Count; i++) {
    const fvec3& v0 = *Verts[i];
    const fvec3& v1 = *Verts[(i + 1) % Count];
    Normal[0] += ((v0.y - v1.y) * (v0.z + v1.z));
    Normal[1] += ((v0.z - v1.z) * (v0.x + v1.x));
    Normal[2] += ((v0.x - v1.x) * (v0.y + v1.y));
  }

  // normalize
  return fvec3::normalize(Normal);
}


static inline bool isConvex(int w0, int w1, int w2)
{
  // caution: faces can fold around the normal
  const int v0 = WorkSet[w0];
  const int v1 = WorkSet[w1];
  const int v2 = WorkSet[w2];
  const fvec3 e0 = *Verts[v1] - *Verts[v0];
  const fvec3 e1 = *Verts[v2] - *Verts[v1];
  const fvec3 cross = fvec3::cross(e0, e1);
  if (fvec3::dot(cross, Normal) <= 0.0f) {
    return false;
  }
  return true;
}


static inline bool isFaceClear(int w0, int w1, int w2)
{
  int i;
  const int v0 = WorkSet[w0];
  const int v1 = WorkSet[w1];
  const int v2 = WorkSet[w2];

  // setup the edges
  fvec3 edges[3];
  edges[0] = *Verts[v1] - *Verts[v0];
  edges[1] = *Verts[v2] - *Verts[v1];
  edges[2] = *Verts[v0] - *Verts[v2];

  // get the triangle normal
  fvec3 normal = fvec3::cross(edges[0], edges[1]);

  // setup the planes
  fvec4 planes[3];
  planes[0].xyz() =  fvec3::cross(edges[0], normal);
  planes[0].w     = -fvec3::dot(planes[0].xyz(), *Verts[v0]);
  planes[1].xyz() =  fvec3::cross(edges[1], normal);
  planes[1].w     = -fvec3::dot(planes[1].xyz(), *Verts[v1]);
  planes[2].xyz() =  fvec3::cross(edges[2], normal);
  planes[2].w     = -fvec3::dot(planes[2].xyz(), *Verts[v2]);

  for (int w = 0; w < Count; w++) {
    if ((w == w0) || (w == w1) || (w == w2)) {
      continue; // FIXME: lazy
    }
    const int v = WorkSet[w];
    for (i = 0; i < 3; i++) {
      const float dist = planes[i].planeDist(*Verts[v]);
      if (dist > 0.0f) {
	break; // this point is clear
      }
    }
    if (i == 3) {
      return false;
    }
  }
  return true;
}


static inline float getDot(int w0, int w1, int w2)
{
  const int v0 = WorkSet[w0];
  const int v1 = WorkSet[w1];
  const int v2 = WorkSet[w2];
  const fvec3 e0 = *Verts[v1] - *Verts[v0];
  const fvec3 e1 = *Verts[v2] - *Verts[v1];
  return fvec3::dot(e0.normalize(), e1.normalize());
}


void triangulateFace(int count, const fvec3** verts,
		     std::vector<TriIndices>& tris)
{
  tris.clear();

  Verts = verts;
  Count = count;
  WorkSet = new int[Count];
  for (int i = 0; i < Count; i++) {
    WorkSet[i] = i;
  }
  makeNormal();

  int best = 0;
  bool left = false;
  bool first = true;
  float score = 0.0f;

  while (Count >= 3) {
    bool convex = false;
    bool faceClear = false;

    int offset;
    if (best == Count) {
      offset = Count - 1;
    } else {
      offset = (best % Count);
    }

    // stripping pattern
    if (left) {
      offset = (offset + (Count - 1)) % Count;
    }
    left = !left;

    // find the best triangle
    for (int w = offset; w < offset + (Count - 2); w++) {
      const int w0 = (w + 0) % Count;
      const int w1 = (w + 1) % Count;
      const int w2 = (w + 2) % Count;

      const bool convex2 = isConvex(w0, w1, w2);
      if (convex && !convex2) {
	continue;
      }

      const bool faceClear2 = isFaceClear(w0, w1, w2);
      if ((faceClear && !faceClear2) && (convex || !convex2)) {
	continue;
      }

      if (first) {
	const float score2 = 2.0f - getDot(w0, w1, w2);
	if ((score2 < score) &&
	    (convex || !convex2) && (faceClear || !faceClear2)) {
	  continue;
	} else {
	  score = score2;
	}
      }

      best = w0;
      if (convex && faceClear) {
	break;
      }
      convex = convex2;
      faceClear = faceClear2;
    }

    first = false;

    // add the triangle
    TriIndices ti;
    ti.indices[0] = WorkSet[(best + 0) % Count];
    ti.indices[1] = WorkSet[(best + 1) % Count];
    ti.indices[2] = WorkSet[(best + 2) % Count];
    tris.push_back(ti);

    // remove the middle vertex
    const int m = (best + 1) % Count;
    memmove(WorkSet + m, WorkSet + m + 1, (Count - m - 1) * sizeof(int));
    Count--;
  }

  delete[] WorkSet;

  return;
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
