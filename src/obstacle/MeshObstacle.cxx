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
#include "MeshObstacle.h"
#include "MeshFace.h"
#include "Intersect.h"


const char* MeshObstacle::typeName = "MeshObstacle";


MeshObstacle::MeshObstacle()
{
  faceCount = faceSize = 0;
  faces = NULL;
  checkCount = 0;
  checkTypes = NULL;
  checkPoints = NULL;
  vertexCount = normalCount = 0;
  vertices = normals = NULL;
  texcoordCount = 0;
  texcoords = NULL;
  fragments = true;
  smoothBounce = false;
  driveThrough = false;
  shootThrough = false;
  isLocal = false;
  return;
}


static void cfvec3ListToArray(const std::vector<cfvec3>& list,
                              int& count, fvec3* &array)
{
  count = list.size();
  array = new fvec3[count];
  for (int i = 0; i < count; i++) {
    memcpy (array[i], list[i].data, sizeof(fvec3));
  }
  return;
}

MeshObstacle::MeshObstacle(const std::vector<char>& checkTypesL,
                           const std::vector<cfvec3>& checkList,
                           const std::vector<cfvec3>& verticeList,
                           const std::vector<cfvec3>& normalList,
                           const std::vector<cfvec2>& texcoordList,
                           int _faceCount,
                           bool bounce, bool drive, bool shoot)
{
  unsigned int i;
  // copy the info
  checkTypes = new char[checkTypesL.size()];
  for (i = 0; i < checkTypesL.size(); i++) {
    checkTypes[i] = checkTypesL[i];
  }
  cfvec3ListToArray (checkList, checkCount, checkPoints);
  cfvec3ListToArray (verticeList, vertexCount, vertices);
  cfvec3ListToArray (normalList, normalCount, normals);
  texcoordCount = texcoordList.size();
  texcoords = new fvec2[texcoordCount];
  for (i = 0; i < (unsigned int)texcoordCount; i++) {
    memcpy (texcoords[i], texcoordList[i].data, sizeof(fvec2));
  }
  faceSize = _faceCount;
  faceCount = 0;
  faces = new MeshFace*[faceSize];
  fragments = true;
  smoothBounce = bounce;
  driveThrough = drive;
  shootThrough = shoot;
  isLocal = false;

  return;
}


bool MeshObstacle::addFace(const std::vector<int>& _vertices,
                           const std::vector<int>& _normals,
                           const std::vector<int>& _texcoords,
                           const BzMaterial* _material,
                           bool bounce, bool drive, bool shoot)
{
  // protect the face list from overrun
  if (faceCount >= faceSize) {
    return false;
  }

  // make sure the list lengths are sane
  unsigned int i;
  unsigned int count = _vertices.size();
  if ((count < 3) ||
      ((_normals.size() > 0) && (_normals.size() != count)) ||
      ((_texcoords.size() > 0) && (_texcoords.size() != count))) {
    return false;
  }

  // validate the indices
  for (i = 0; i < _vertices.size(); i++) {
    if (_vertices[i] >= vertexCount) {
      return false;
    }
  }
  for (i = 0; i < _normals.size(); i++) {
    if (_normals[i] >= normalCount) {
      return false;
    }
  }
  for (i = 0; i < _texcoords.size(); i++) {
    if (_texcoords[i] >= texcoordCount) {
      return false;
    }
  }

  // use the indicies to makes lists of pointers
  float **v = new float*[_vertices.size()];
  float **n = NULL;
  float **t = NULL;
  if (_normals.size() > 0) {
    n = new float*[count];
  }
  if (_texcoords.size() > 0) {
    t = new float*[count];
  }
  for (i = 0; i < count; i++) {
    v[i] = (float*)vertices[_vertices[i]];
    if (n != NULL) {
      n[i] = (float*)normals[_normals[i]];
    }
    if (t != NULL) {
      t[i] = (float*)texcoords[_texcoords[i]];
    }
  }
  
  // override the flags if they're set for the whole mesh
  if (smoothBounce) {
    bounce = true;
  }
  if (driveThrough) {
    drive = true;
  }
  if (shootThrough) {
    shoot = true;
  }
  
  // make the face
  MeshFace* face = new MeshFace(this, count, v, n, t, _material, 
                                bounce, drive, shoot);

  // check its validity
  if (face->isValid()) {
    faces[faceCount] = face;
    faceCount++;
  } else {
    delete face;
    return false;
  }

  return true;
}


MeshObstacle::~MeshObstacle()
{
  delete[] checkTypes;
  delete[] checkPoints;
  delete[] vertices;
  delete[] normals;
  delete[] texcoords;
  for (int i = 0; i < faceCount; i++) {
    delete faces[i];
  }
  delete[] faces;
  return;
}


void MeshObstacle::finalize()
{
  int f;

  // cross-link the face edges - FIXME
  for (f = 0; f < faceCount; f++) {
    faces[f]->edges = NULL;
  }

  // set the extents
  mins[0] = mins[1] = mins[2] = +MAXFLOAT;
  maxs[0] = maxs[1] = maxs[2] = -MAXFLOAT;
  for (f = 0; f < faceCount; f++) {
    for (int a = 0; a < 3; a++) {
      if (faces[f]->mins[a] < mins[a]) {
        mins[a] = faces[f]->mins[a];
      }
      if (faces[f]->maxs[a] > maxs[a]) {
        maxs[a] = faces[f]->maxs[a];
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


const char* MeshObstacle::getType() const
{
  return typeName;
}


const char* MeshObstacle::getClassName() // const
{
  return typeName;
}


bool MeshObstacle::isValid() const
{
  // check the planes
/* FIXME - kill the whole thing for one bad face?
  for (int f = 0; f < faceCount; f++) {
    if (!faces[f]->isValid()) {
      return false;
    }
  }
*/

  // now check the vertices
  for (int v = 0; v < vertexCount; v++) {
    for (int a = 0; a < 3; a++) {
      if (fabsf(vertices[v][a]) > maxExtent) {
        return false;
      }
    }
  }

  return true;
}


void MeshObstacle::getExtents(float* _mins, float* _maxs) const
{
  memcpy (_mins, mins, sizeof(float[3]));
  memcpy (_maxs, maxs, sizeof(float[3]));
  return;
}


float MeshObstacle::intersect(const Ray& ray) const
{
  ray.getDirection();
  return -1.0f;
}


void MeshObstacle::get3DNormal(const float* /*p*/, float* n) const
{
  n = n;
  return;
}




/////////////////////////////////////////////////////////////
//  FIXME - everything after this point is currently JUNK! //
/////////////////////////////////////////////////////////////



void MeshObstacle::getNormal(const float* p, float* n) const
{
  p = p;
  n = n;
  return;
}


bool MeshObstacle::getHitNormal(const float* pos1, float,
				 const float* pos2, float,
				 float, float, float height,
				 float* normal) const
{
  pos1 = pos2;
  normal = normal;
  height = height;
  return false;
}


bool MeshObstacle::inCylinder(const float* p,
                               float radius, float height) const
{
  p = p;
  radius = height;
  return false;
}


bool MeshObstacle::inBox(const float* p, float angle,
                          float dx, float dy, float height) const
{
  p = p;
  angle = angle;
  dx = dy = height;
  return false;
}


bool MeshObstacle::inMovingBox(const float*, float,
                                const float* p, float angle,
                                float dx, float dy, float height) const
{
  p = p;
  angle = angle;
  dx = dy = height;
  return false;
}


// This is only used when the player has an Oscillation Overthruster
// flag, and only after we already know that the tank is interfering
// with this tetrahedron, so it doesn't have to be particularly fast.
// As a note, some of the info from the original collision test might
// be handy here.
bool MeshObstacle::isCrossing(const float* p, float angle,
                               float dx, float dy, float height,
                               float* plane) const
{
  p = p;
  angle = angle;
  dx = dy = height;
  plane = plane;
  return false;
}


void *MeshObstacle::pack(void *buf)
{
  if (isLocal) {
    return buf;
  }

  int i;

  buf = nboPackInt(buf, checkCount);
  for (i = 0; i < checkCount; i++) {
    buf = nboPackUByte(buf, checkTypes[i]);
    buf = nboPackVector(buf, checkPoints[i]);
  }

  buf = nboPackInt(buf, vertexCount);
  for (i = 0; i < vertexCount; i++) {
    buf = nboPackVector(buf, vertices[i]);
  }

  buf = nboPackInt(buf, normalCount);
  for (i = 0; i < normalCount; i++) {
    buf = nboPackVector(buf, normals[i]);
  }

  buf = nboPackInt(buf, texcoordCount);
  for (i = 0; i < texcoordCount; i++) {
    buf = nboPackFloat(buf, texcoords[i][0]);
    buf = nboPackFloat(buf, texcoords[i][1]);
  }

  buf = nboPackInt(buf, faceCount);
  for (i = 0; i < faceCount; i++) {
    buf = faces[i]->pack(buf);
  }

  // pack the state byte
  unsigned char stateByte = 0;
  if (isDriveThrough()) {
    stateByte |= (1 << 0);
  }
  if (isShootThrough()) {
    stateByte |= (1 << 1);
  }
  if (useSmoothBounce()) {
    stateByte |= (1 << 2);
  }
  if (useFragments()) {
    stateByte |= (1 << 3);
  }
  buf = nboPackUByte(buf, stateByte);

  return buf;
}


void *MeshObstacle::unpack(void *buf)
{
  int i;

  buf = nboUnpackInt(buf, checkCount);
  checkTypes = new char[checkCount];
  checkPoints = new fvec3[checkCount];
  for (i = 0; i < checkCount; i++) {
    unsigned char tmp;
    buf = nboUnpackUByte(buf, tmp);
    checkTypes[i] = tmp;
    buf = nboUnpackVector(buf, checkPoints[i]);
  }

  buf = nboUnpackInt(buf, vertexCount);
  vertices = new fvec3[vertexCount];
  for (i = 0; i < vertexCount; i++) {
    buf = nboUnpackVector(buf, vertices[i]);
  }

  buf = nboUnpackInt(buf, normalCount);
  normals = new fvec3[normalCount];
  for (i = 0; i < normalCount; i++) {
    buf = nboUnpackVector(buf, normals[i]);
  }

  buf = nboUnpackInt(buf, texcoordCount);
  texcoords = new fvec2[texcoordCount];
  for (i = 0; i < texcoordCount; i++) {
    buf = nboUnpackFloat(buf, texcoords[i][0]);
    buf = nboUnpackFloat(buf, texcoords[i][1]);
  }

  buf = nboUnpackInt(buf, faceCount);
  faceSize = faceCount;
  faces = new MeshFace*[faceCount];
  for (i = 0; i < faceCount; i++) {
    faces[i] = new MeshFace(this);
    buf = faces[i]->unpack(buf);
  }

  // unpack the state byte
  driveThrough = shootThrough = false;
  smoothBounce = fragments = false;
  unsigned char stateByte;
  buf = nboUnpackUByte(buf, stateByte);
  if (stateByte & (1 << 0)) {
    driveThrough = true;
  }
  if (stateByte & (1 << 1)) {
    shootThrough = true;
  }
  if (stateByte & (1 << 2)) {
    smoothBounce = true;
  }
  if (stateByte & (1 << 3)) {
    fragments = true;
  }

  finalize();

  return buf;
}


int MeshObstacle::packSize()
{
  if (isLocal) {
    return 0;
  }
  
  int fullSize = 5 * sizeof(int);
  fullSize += sizeof(char) * checkCount;
  fullSize += sizeof(fvec3) * checkCount;
  fullSize += sizeof(fvec3) * vertexCount;
  fullSize += sizeof(fvec3) * normalCount;
  fullSize += sizeof(fvec2) * texcoordCount;
  for (int f = 0; f < faceCount; f++) {
    fullSize += faces[f]->packSize();
  }
  fullSize += sizeof(unsigned char); // state byte
  return fullSize;
}


static void outputFloat(std::ostream& out, float value)
{
  char buffer[32];
  sprintf (buffer, " %.8f", value);
  out << buffer;
  return;
}


void MeshObstacle::print(std::ostream& out, int level)
{
  if (isLocal) {
    return;
  }

  out << "mesh" << std::endl;
  if (level > 0) {
    out << "# faces = " << faceCount << std::endl;
    out << "# checks = " << checkCount << std::endl;
    out << "# vertices = " << vertexCount << std::endl;
    out << "# normals = " << normalCount << std::endl;
    out << "# texcoords = " << texcoordCount << std::endl;
    out << "# mins = " << mins[0] << " " << mins[1] << " " << mins[2] << std::endl;
    out << "# maxs = " << maxs[0] << " " << maxs[1] << " " << maxs[2] << std::endl;
  }
  
  if (smoothBounce) {
    out << "  smoothBounce" << std::endl;
  }
  if (driveThrough) {
    out << "  driveThrough" << std::endl;
  }
  if (shootThrough) {
    out << "  shootThrough" << std::endl;
  }

  int i, j;
  for (i = 0; i < checkCount; i++) {
    if (checkTypes[i] == CheckInside) {
      out << "  inside";
    } else {
      out << "  outside";
    }
    for (j = 0; j < 3; j++) {
      outputFloat(out, checkPoints[i][j]);
    }
    out << std::endl;
  }
  for (i = 0; i < vertexCount; i++) {
    out << "  vertex";
    for (j = 0; j < 3; j++) {
      outputFloat(out, vertices[i][j]);
    }
    out << std::endl;
  }
  for (i = 0; i < normalCount; i++) {
    out << "  normal";
    for (j = 0; j < 3; j++) {
      outputFloat(out, normals[i][j]);
    }
    out << std::endl;
  }
  for (i = 0; i < texcoordCount; i++) {
    out << "  texcoord";
    for (j = 0; j < 3; j++) {
      outputFloat(out, texcoords[i][j]);
    }
    out << std::endl;
  }

  for (int f = 0; f < faceCount; f++) {
    faces[f]->print(out, level);
  }

  out << "end" << std::endl;

  return;
}


// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

