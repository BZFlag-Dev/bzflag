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
#include "CollisionManager.h"
#include "Intersect.h"

#include "MeshTransform.h"


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
  noclusters = false;
  smoothBounce = false;
  driveThrough = false;
  shootThrough = false;
  inverted = false;
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

static void arrayToCfvec3List(const fvec3* array, int count,
			      std::vector<cfvec3>& list)
{
  list.clear();
  for (int i = 0; i < count; i++) {
    list.push_back(array[i]);
  }
  return;
}


MeshObstacle::MeshObstacle(const MeshTransform& transform,
			   const std::vector<char>& checkTypesL,
			   const std::vector<cfvec3>& checkList,
			   const std::vector<cfvec3>& verticeList,
			   const std::vector<cfvec3>& normalList,
			   const std::vector<cfvec2>& texcoordList,
			   int _faceCount, bool _noclusters,
			   bool bounce, bool drive, bool shoot)
{
  unsigned int i;

  // get the transform tool
  MeshTransform::Tool xformtool(transform);
  inverted = xformtool.isInverted();

  // copy the info
  checkTypes = new char[checkTypesL.size()];
  for (i = 0; i < checkTypesL.size(); i++) {
    checkTypes[i] = checkTypesL[i];
  }
  cfvec3ListToArray (checkList, checkCount, checkPoints);
  cfvec3ListToArray (verticeList, vertexCount, vertices);
  cfvec3ListToArray (normalList, normalCount, normals);

  // modify according to the transform
  int j;
  for (j = 0; j < checkCount; j++) {
    xformtool.modifyVertex(checkPoints[j]);
  }
  for (j = 0; j < vertexCount; j++) {
    xformtool.modifyVertex(vertices[j]);
  }
  for (j = 0; j < normalCount; j++) {
    xformtool.modifyNormal(normals[j]);
  }

  texcoordCount = texcoordList.size();
  texcoords = new fvec2[texcoordCount];
  for (i = 0; i < (unsigned int)texcoordCount; i++) {
    memcpy (texcoords[i], texcoordList[i].data, sizeof(fvec2));
  }

  faceSize = _faceCount;
  faceCount = 0;
  faces = new MeshFace*[faceSize];

  noclusters = _noclusters;
  smoothBounce = bounce;
  driveThrough = drive;
  shootThrough = shoot;

  return;
}


bool MeshObstacle::addFace(const std::vector<int>& _vertices,
			   const std::vector<int>& _normals,
			   const std::vector<int>& _texcoords,
			   const BzMaterial* _material, int phydrv,
			   bool _noclusters,
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
    int index = (inverted ? ((count - 1) - i) : i);
    v[index] = (float*)vertices[_vertices[i]];
    if (n != NULL) {
      n[index] = (float*)normals[_normals[i]];
    }
    if (t != NULL) {
      t[index] = (float*)texcoords[_texcoords[i]];
    }
  }

  // override the flags if they are set for the whole mesh
  _noclusters = _noclusters|| noclusters;
  bounce = bounce || smoothBounce;
  drive = drive || driveThrough;
  shoot = shoot || shootThrough;

  // make the face
  MeshFace* face = new MeshFace(this, count, v, n, t, _material, phydrv,
				_noclusters, bounce, drive, shoot);

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


Obstacle* MeshObstacle::copyWithTransform(const MeshTransform& xform) const
{
  int i;
  std::vector<char> ctlist;
  std::vector<cfvec3> clist;
  std::vector<cfvec3> vlist;
  std::vector<cfvec3> nlist;
  std::vector<cfvec2> tlist;

  for (i = 0; i < checkCount; i++) {
    ctlist.push_back(checkTypes[i]);
  }
  arrayToCfvec3List(checkPoints, checkCount, vlist);
  arrayToCfvec3List(vertices, vertexCount, vlist);
  arrayToCfvec3List(normals, normalCount, nlist);
  for (i = 0; i < texcoordCount; i++) {
    tlist.push_back(texcoords[i]);
  }

  MeshObstacle* copy =
    new MeshObstacle(xform, ctlist, clist, vlist, nlist, tlist, faceCount,
		     noclusters, smoothBounce, driveThrough, shootThrough);

  for (i = 0; i < faceCount; i++) {
    copyFace(i, copy);
  }

  copy->finalize();

  return copy;
}


void MeshObstacle::copyFace(int f, MeshObstacle* mesh) const
{
  MeshFace* face = faces[f];

  std::vector<int> vlist;
  std::vector<int> nlist;
  std::vector<int> tlist;
  const int vcount = face->getVertexCount();
  for (int i = 0; i < vcount; i++) {
    int index;
    index = ((fvec3*) face->getVertex(i)) - vertices;
    vlist.push_back(index);

    if (face->useNormals()) {
      index = ((fvec3*) face->getNormal(i)) - normals;
      nlist.push_back(index);
    }
    if (face->useTexcoords()) {
      index = ((fvec2*) face->getTexcoord(i)) - texcoords;
      tlist.push_back(index);
    }
  }

  mesh->addFace(vlist, nlist, tlist, face->getMaterial(),
		face->getPhysicsDriver(), face->noClusters(),
		face->isSmoothBounce(),
		face->isDriveThrough(), face->isShootThrough());
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


bool MeshObstacle::containsPoint(const float point[3]) const
{
  // this should use the CollisionManager's rayTest function
//  const ObsList* olist = COLLISIONMGR.rayTest (&ray, t);
  return containsPointNoOctree(point);
}


bool MeshObstacle::containsPointNoOctree(const float point[3]) const
{
  if (checkCount <= 0) {
    return false;
  }

  int c, f;
  float dir[3];
  bool hasOutsides = false;

  for (c = 0; c < checkCount; c++) {
    if (checkTypes[c] == CheckInside) {
      vec3sub (dir, checkPoints[c], point);
      Ray ray(point, dir);
      bool hitFace = false;
      for (f = 0; f < faceCount; f++) {
	const MeshFace* face = faces[f];
	const float hittime = face->intersect(ray);
	if ((hittime > 0.0f) && (hittime <= 1.0f)) {
	  hitFace = true;
	  break;
	}
      }
      if (!hitFace) {
	return true;
      }
    }
    else if (checkTypes[c] == CheckOutside) {
      hasOutsides = true;
      vec3sub (dir, point, checkPoints[c]);
      Ray ray(checkPoints[c], dir);
      bool hitFace = false;
      for (f = 0; f < faceCount; f++) {
	const MeshFace* face = faces[f];
	const float hittime = face->intersect(ray);
	if ((hittime > 0.0f) && (hittime <= 1.0f)) {
	  hitFace = true;
	  break;
	}
      }
      if (!hitFace) {
	return false;
      }
    }
    else {
      printf ("checkType (%i) is not supported yet\n", checkTypes[c]);
      exit (1);
    }
  }

  return hasOutsides;
}


float MeshObstacle::intersect(const Ray& /*ray*/) const
{
  return -1.0f; // rays only intersect with mesh faces
}


void MeshObstacle::get3DNormal(const float* /*p*/, float* /*n*/) const
{
  return; // this should never be called if intersect() is always < 0.0f
}


void MeshObstacle::getNormal(const float* p, float* n) const
{
  const fvec3 center = { pos[0], pos[1], pos[2] + (0.5f * size[2]) };
  fvec3 out;
  vec3sub (out, p, center);
  if (out[2] < 0.0f) {
    out[2] = 0.0f;
  }
  float len = vec3dot(out, out);
  if (len > 0.0f) {
    len = 1 / sqrtf(len);
    n[0] = out[0] * len;
    n[1] = out[1] * len;
    n[2] = out[2] * len;
  } else {
    n[0] = 0.0f;
    n[1] = 0.0f;
    n[2] = 1.0f;
  }

  return;
}


bool MeshObstacle::getHitNormal(const float* /*oldPos*/, float /*oldAngle*/,
				 const float* p, float /*angle*/,
				 float, float, float /*height*/,
				 float* n) const
{
  if (n != NULL) {
    getNormal(p, n);
  }
  return true;
}


bool MeshObstacle::inCylinder(const float* p,
			       float /*radius*/, float height) const
{
  const float mid[3] = { p[0], p[1], p[2] + (0.5f * height) };
  return containsPoint(mid);
}


bool MeshObstacle::inBox(const float* p, float /*angle*/,
			 float /*dx*/, float /*dy*/, float height) const
{
  const float mid[3] = { p[0], p[1], p[2] + (0.5f * height) };
  return containsPoint(mid);
}


bool MeshObstacle::inMovingBox(const float*, float,
			       const float* p, float /*angle*/,
			       float /*dx*/, float /*dy*/, float height) const
{
  const float mid[3] = { p[0], p[1], p[2] + (0.5f * height) };
  return containsPoint(mid);
}


bool MeshObstacle::isCrossing(const float* /*p*/, float /*angle*/,
			       float /*dx*/, float /*dy*/, float /*height*/,
			       float* /*plane*/) const
{
  return false; // the MeshFaces should handle this case
}


void *MeshObstacle::pack(void *buf) const
{
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
  stateByte |= isDriveThrough() ? (1 << 0) : 0;
  stateByte |= isShootThrough() ? (1 << 1) : 0;
  stateByte |= smoothBounce     ? (1 << 2) : 0;
  stateByte |= noclusters       ? (1 << 3) : 0;
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
  unsigned char stateByte;
  buf = nboUnpackUByte(buf, stateByte);
  driveThrough = (stateByte & (1 << 0)) != 0;
  shootThrough = (stateByte & (1 << 1)) != 0;
  smoothBounce = (stateByte & (1 << 2)) != 0;
  noclusters   = (stateByte & (1 << 3)) != 0;

  finalize();

  return buf;
}


int MeshObstacle::packSize() const
{
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


void MeshObstacle::print(std::ostream& out, const std::string& indent) const
{
  out << indent << "mesh" << std::endl;

  out << indent << "# faces = " << faceCount << std::endl;
  out << indent << "# checks = " << checkCount << std::endl;
  out << indent << "# vertices = " << vertexCount << std::endl;
  out << indent << "# normals = " << normalCount << std::endl;
  out << indent << "# texcoords = " << texcoordCount << std::endl;
  out << indent << "# mins = " << mins[0] << " " << mins[1] << " " << mins[2] << std::endl;
  out << indent << "# maxs = " << maxs[0] << " " << maxs[1] << " " << maxs[2] << std::endl;

  if (noclusters) {
    out << indent << "  noclusters" << std::endl;
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

  int i, j;
  for (i = 0; i < checkCount; i++) {
    if (checkTypes[i] == CheckInside) {
      out << indent << "  inside";
    } else {
      out << indent << "  outside";
    }
    for (j = 0; j < 3; j++) {
      outputFloat(out, checkPoints[i][j]);
    }
    out << std::endl;
  }
  for (i = 0; i < vertexCount; i++) {
    out << indent << "  vertex";
    for (j = 0; j < 3; j++) {
      outputFloat(out, vertices[i][j]);
    }
    out << std::endl;
  }
  for (i = 0; i < normalCount; i++) {
    out << indent << "  normal";
    for (j = 0; j < 3; j++) {
      outputFloat(out, normals[i][j]);
    }
    out << std::endl;
  }
  for (i = 0; i < texcoordCount; i++) {
    out << indent << "  texcoord";
    for (j = 0; j < 3; j++) {
      outputFloat(out, texcoords[i][j]);
    }
    out << std::endl;
  }

  for (int f = 0; f < faceCount; f++) {
    faces[f]->print(out, indent);
  }

  out << indent << "end" << std::endl << std::endl;

  return;
}


// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

