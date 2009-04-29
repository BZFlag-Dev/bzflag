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

// implementatino header
#include "MeshObstacle.h"

// system headers
#include <math.h>
#include <stdlib.h>

// common headers
#include "global.h"
#include "Pack.h"
#include "MeshFace.h"
#include "CollisionManager.h"
#include "Intersect.h"
#include "MeshDrawInfo.h"
#include "MeshTransform.h"
#include "StateDatabase.h"

// local headers
#include "Triangulate.h"


const char*	MeshObstacle::typeName = "MeshObstacle";


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
  driveThrough = 0;
  shootThrough = 0;
  ricochet = false;
  inverted = false;
  hasSpecialFaces = false;
  drawInfo = NULL;
  return;
}


static void fvec3ListToArray(const std::vector<fvec3>& list,
			      int& count, fvec3* &array)
{
  count = list.size();
  array = new fvec3[count];
  for (int i = 0; i < count; i++) {
    array[i] = list[i];
  }
  return;
}

static void arrayToCfvec3List(const fvec3* array, int count,
			      std::vector<fvec3>& list)
{
  list.clear();
  for (int i = 0; i < count; i++) {
    list.push_back(array[i]);
  }
  return;
}


MeshObstacle::MeshObstacle(const MeshTransform& transform,
			   const std::vector<char>& checkTypesL,
			   const std::vector<fvec3>& checkList,
			   const std::vector<fvec3>& verticeList,
			   const std::vector<fvec3>& normalList,
			   const std::vector<fvec2>& texcoordList,
			   int _faceCount, bool _noclusters, bool bounce,
			   unsigned char drive, unsigned char shoot, bool rico)
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
  fvec3ListToArray(checkList,   checkCount,  checkPoints);
  fvec3ListToArray(verticeList, vertexCount, vertices);
  fvec3ListToArray(normalList,  normalCount, normals);

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
    texcoords[i] = texcoordList[i];
  }

  faceSize = _faceCount;
  faceCount = 0;
  faces = new MeshFace*[faceSize];

  noclusters = _noclusters;
  smoothBounce = bounce;
  driveThrough = drive;
  shootThrough = shoot;
  ricochet = rico;

  hasSpecialFaces = false;

  drawInfo = NULL;

  return;
}


bool MeshObstacle::addFace(const std::vector<int>& _vertices,
			   const std::vector<int>& _normals,
			   const std::vector<int>& _texcoords,
			   const BzMaterial* _material, int phydrv,
			   bool _noclusters, bool bounce,
			   unsigned char drive, unsigned char shoot, bool rico,
			   bool triangulate, const MeshFace::SpecialData* sd)
{
  // protect the face list from overrun
  if (faceCount >= faceSize) {
    return false;
  }

  // make sure the list lengths are the same
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

  // use the indices to makes lists of pointers
  const fvec3** v;
  const fvec3** n;
  const fvec2** t;
  makeFacePointers(_vertices, _normals, _texcoords, v, n, t);

  // override the flags if they are set for the whole mesh
  _noclusters = _noclusters || noclusters;
  bounce = bounce || smoothBounce;
  drive = drive | driveThrough;
  shoot = shoot | shootThrough;
  rico  = rico || ricochet;

  // override the triangulation setting depending on count
  triangulate = triangulate && (count > 3);

  // make the face
  MeshFace* face;
  if (triangulate) {
    // avoid warnings that may not apply
    int tmpDebugLevel = debugLevel;
    debugLevel = 0;
    face = new MeshFace(this, count, v, n, t, _material, phydrv,
			_noclusters, bounce, drive, shoot, rico, sd);
    debugLevel = tmpDebugLevel;
  } else {
    face = new MeshFace(this, count, v, n, t, _material, phydrv,
			_noclusters, bounce, drive, shoot, rico, sd);
  }

  // check its validity
  if (face->isValid()) {
    faces[faceCount] = face;
    faceCount++;
    if (face->isSpecial()) {
      hasSpecialFaces = true;
    }
  }
  else if (triangulate) {
    // triangulate
    std::vector<TriIndices> triIndices;
    triangulateFace(count, (const fvec3**)v, triIndices);
    delete face; // delete the old face
    const unsigned int triSize = triIndices.size();
    if (triSize <= 0) {
      return false;
    } else {
      // prepare array for extra faces
      const int extra = (int)(triIndices.size() - 1);
      MeshFace** tmp = new MeshFace*[faceSize + extra];
      memcpy(tmp, faces, faceCount * sizeof(MeshFace*));
      delete[] faces;
      faces = tmp;
      faceSize += extra;
      // add the triangles
      for (i = 0; i < triSize; i++) {
	std::vector<int> triV, triN, triT;
	for (int j = 0; j < 3; j++) {
	  const int index = triIndices[i].indices[j];
	  triV.push_back(_vertices[index]);
	  if (_normals.size() > 0) {
	    triN.push_back(_normals[index]);
	  }
	  if (_texcoords.size() > 0) {
	    triT.push_back(_texcoords[index]);
	  }
	}
	makeFacePointers(triV, triN, triT, v, n, t);
	face = new MeshFace(this, 3, v, n, t, _material, phydrv,
			    _noclusters, bounce, drive, shoot, rico, sd);
	if (face->isValid()) {
	  faces[faceCount] = face;
	  faceCount++;
          if (face->isSpecial()) {
            hasSpecialFaces = true;
          }
	} else {
	  delete face;
	}
      }
    }
  }
  else {
    // just nuke it
    delete face;
    return false;
  }

  return true;
}


void MeshObstacle::makeFacePointers(const std::vector<int>& _vertices,
				    const std::vector<int>& _normals,
				    const std::vector<int>& _texcoords,
				    const fvec3**& v,
				    const fvec3**& n,
				    const fvec2**& t)
{
  const int count = _vertices.size();

  // use the indices to makes lists of pointers
  v = new const fvec3*[count];
  n = NULL;
  t = NULL;

  if (_normals.size() > 0) {
    n = new const fvec3*[count];
  }
  if (_texcoords.size() > 0) {
    t = new const fvec2*[count];
  }

  for (int i = 0; i < count; i++) {
    // invert the vertices if required
    const int index = (inverted ? ((count - 1) - i) : i);
    v[index] = &vertices[_vertices[i]];
    if (n != NULL) {
      n[index] = &normals[_normals[i]];
    }
    if (t != NULL) {
      t[index] = &texcoords[_texcoords[i]];
    }
  }
  return;
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
  delete drawInfo;
  return;
}


Obstacle* MeshObstacle::copyWithTransform(const MeshTransform& xform) const
{
  int i;
  MeshObstacle* copy;
  std::vector<char> ctlist;
  std::vector<fvec3> clist;
  std::vector<fvec3> vlist;
  std::vector<fvec3> nlist;
  std::vector<fvec2> tlist;

  // empty arrays for copies of pure visual meshes
  if ((drawInfo != NULL) &&
      ((faceCount <= 0) || (driveThrough && shootThrough))) {
    // load blanks for pure visual meshes
    copy = new MeshObstacle(xform, ctlist, clist,
			    vlist, nlist, tlist, 0, noclusters,
			    smoothBounce, driveThrough, shootThrough, ricochet);
    copy->setName(name);
  }
  else {
    for (i = 0; i < checkCount; i++) {
      ctlist.push_back(checkTypes[i]);
    }
    arrayToCfvec3List(checkPoints, checkCount, clist);
    arrayToCfvec3List(vertices, vertexCount, vlist);
    arrayToCfvec3List(normals, normalCount, nlist);
    for (i = 0; i < texcoordCount; i++) {
      tlist.push_back(texcoords[i]);
    }

    copy = new MeshObstacle(xform, ctlist, clist,
			    vlist, nlist, tlist, faceCount, noclusters,
			    smoothBounce, driveThrough, shootThrough, ricochet);
    copy->setName(name);

    for (i = 0; i < faceCount; i++) {
      copyFace(i, copy);
    }
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
    index = &face->getVertex(i) - vertices;
    vlist.push_back(index);

    if (face->useNormals()) {
      index = &face->getNormal(i) - normals;
      nlist.push_back(index);
    }
    if (face->useTexcoords()) {
      index = &face->getTexcoord(i) - texcoords;
      tlist.push_back(index);
    }
  }

  mesh->addFace(vlist, nlist, tlist, face->getMaterial(),
		face->getPhysicsDriver(), face->noClusters(),
		face->isSmoothBounce(),
		face->isDriveThrough(), face->isShootThrough(),
		face->canRicochet(), false, face->getSpecialData());
  return;
}


void MeshObstacle::finalize()
{
  // set the face IDs
  for (int f = 0; f < faceCount; f++) {
    faces[f]->setID(f);
  }

  // cross-link the face edges - FIXME
  for (int f = 0; f < faceCount; f++) {
    faces[f]->edges = NULL;
  }

  // set the extents
  for (int f = 0; f < faceCount; f++) {
    const Extents& exts = faces[f]->getExtents();
    extents.expandToBox(exts);
  }

  // setup fake obstacle parameters
  pos.x = (extents.maxs.x + extents.mins.x) / 2.0f;
  pos.y = (extents.maxs.y + extents.mins.y) / 2.0f;
  pos.z = extents.mins.z;
  size.x = (extents.maxs.x - extents.mins.x) / 2.0f;
  size.y = (extents.maxs.y - extents.mins.y) / 2.0f;
  size.z = (extents.maxs.z - extents.mins.z);
  angle = 0.0f;
  zFlip = false;

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


void MeshObstacle::setDrawInfo(MeshDrawInfo* di)
{
  if (drawInfo != NULL) {
    printf("MeshObstacle::setMeshDrawInfo() already exists\n");
    exit(1);
  } else {
    drawInfo = di;
  }
  return;
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


bool MeshObstacle::containsPoint(const fvec3& point) const
{
  // this should use the CollisionManager's rayTest function
//  const ObsList* olist = COLLISIONMGR.rayTest (&ray, t);
  return containsPointNoOctree(point);
}


bool MeshObstacle::containsPointNoOctree(const fvec3& point) const
{
  if (checkCount <= 0) {
    return false;
  }

  int c, f;
  fvec3 dir;
  bool hasOutsides = false;

  for (c = 0; c < checkCount; c++) {
    if (checkTypes[c] == CheckInside) {
      dir = checkPoints[c] - point;
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
      dir = point - checkPoints[c];
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


void MeshObstacle::get3DNormal(const fvec3& /*p*/, fvec3& /*n*/) const
{
  return; // this should never be called if intersect() is always < 0.0f
}


void MeshObstacle::getNormal(const fvec3& p, fvec3& n) const
{
  const fvec3 center(pos.x, pos.y, pos.z + (0.5f * size.z));
  fvec3 out = p - center;
  if (out.z < 0.0f) {
    out.z = 0.0f;
  }

  float lengthSq = out.lengthSq();
  if (lengthSq > 0.0f) {
    n = out * (1.0f / sqrtf(lengthSq));
  } else {
    n = fvec3(0.0f, 0.0f, 1.0f);
  }

  return;
}


bool MeshObstacle::getHitNormal(const fvec3& /*oldPos*/, float /*oldAngle*/,
                                const fvec3& p, float /*angle*/,
                                float, float, float /*height*/,
                                fvec3& n) const
{
  getNormal(p, n);
  return true;
}


bool MeshObstacle::inCylinder(const fvec3& p,
			       float /*radius*/, float height) const
{
  const fvec3 mid(p.x, p.y, p.z + (0.5f * height));
  return containsPoint(mid);
}


bool MeshObstacle::inBox(const fvec3& p, float /*angle*/,
			 float /*dx*/, float /*dy*/, float height) const
{
  const fvec3 mid(p.x, p.y, p.z + (0.5f * height));
  return containsPoint(mid);
}


bool MeshObstacle::inMovingBox(const fvec3&, float,
			       const fvec3& p, float /*angle*/,
			       float /*dx*/, float /*dy*/, float height) const
{
  const fvec3 mid(p.x, p.y, p.z + (0.5f * height));
  return containsPoint(mid);
}


bool MeshObstacle::isCrossing(const fvec3& /*p*/, float /*angle*/,
			       float /*dx*/, float /*dy*/, float /*height*/,
			       fvec4* /*plane*/) const
{
  return false; // the MeshFaces should handle this case
}


int MeshObstacle::packSize() const
{
  int fullSize = 0;

  fullSize += sizeof(uint8_t); // state byte

  fullSize += nboStdStringPackSize(name);

  fullSize += 4 * sizeof(int32_t); // the counts
  fullSize += checkCount    * (sizeof(fvec3) + sizeof(uint8_t));
  fullSize += vertexCount   * sizeof(fvec3);
  fullSize += normalCount   * sizeof(fvec3);
  fullSize += texcoordCount * sizeof(fvec2);


  fullSize += sizeof(int32_t); // faceCount
  for (int f = 0; f < faceCount; f++) {
    fullSize += faces[f]->packSize();
  }

  if ((drawInfo != NULL) && !drawInfo->isCopy()) {
    fullSize += drawInfo->packSize();
  }

  return fullSize;
}


void *MeshObstacle::pack(void *buf) const
{
  int i;

  const bool drawInfoOwner = (drawInfo != NULL) && !drawInfo->isCopy();

  // setup the state byte
  uint8_t stateByte = 0;
  stateByte |= isDriveThrough() ? (1 << 0) : 0;
  stateByte |= isShootThrough() ? (1 << 1) : 0;
  stateByte |= smoothBounce     ? (1 << 2) : 0;
  stateByte |= noclusters       ? (1 << 3) : 0;
  stateByte |= canRicochet()    ? (1 << 4) : 0;
  stateByte |= drawInfoOwner    ? (1 << 5) : 0;

  buf = nboPackUInt8(buf, stateByte);

  buf = nboPackStdString(buf, name);

  buf = nboPackInt32(buf, checkCount);
  for (i = 0; i < checkCount; i++) {
    buf = nboPackUInt8(buf, checkTypes[i]);
    buf = nboPackFVec3(buf, checkPoints[i]);
  }

  buf = nboPackInt32(buf, vertexCount);
  for (i = 0; i < vertexCount; i++) {
    buf = nboPackFVec3(buf, vertices[i]);
  }

  buf = nboPackInt32(buf, normalCount);
  for (i = 0; i < normalCount; i++) {
    buf = nboPackFVec3(buf, normals[i]);
  }

  buf = nboPackInt32(buf, texcoordCount);
  for (i = 0; i < texcoordCount; i++) {
    buf = nboPackFVec2(buf, texcoords[i]);
  }

  buf = nboPackInt32(buf, faceCount);
  for (i = 0; i < faceCount; i++) {
    buf = faces[i]->pack(buf);
  }

  if (drawInfoOwner) {
    buf = drawInfo->pack(buf);
  }

  return buf;
}


void *MeshObstacle::unpack(void *buf)
{
  int i;
  int32_t inTmp;

  uint8_t stateByte;
  buf = nboUnpackUInt8(buf, stateByte);

  // unravel the state byte
  bool drawInfoOwner;
  driveThrough  = (stateByte & (1 << 0)) != 0 ? 0xFF : 0;
  shootThrough  = (stateByte & (1 << 1)) != 0 ? 0xFF : 0;
  smoothBounce  = (stateByte & (1 << 2)) != 0;
  noclusters    = (stateByte & (1 << 3)) != 0;
  ricochet      = (stateByte & (1 << 4)) != 0;
  drawInfoOwner = (stateByte & (1 << 5)) != 0;

  buf = nboUnpackStdString(buf, name);

  buf = nboUnpackInt32(buf, inTmp);
  checkCount = int(inTmp);
  checkTypes = new char[checkCount];
  checkPoints = new fvec3[checkCount];
  for (i = 0; i < checkCount; i++) {
    uint8_t tmp;
    buf = nboUnpackUInt8(buf, tmp);
    checkTypes[i] = tmp;
    buf = nboUnpackFVec3(buf, checkPoints[i]);
  }

  buf = nboUnpackInt32(buf, inTmp);
  vertexCount = int(inTmp);
  vertices = new fvec3[vertexCount];
  for (i = 0; i < vertexCount; i++) {
    buf = nboUnpackFVec3(buf, vertices[i]);
  }

  buf = nboUnpackInt32(buf, inTmp);
  normalCount = int(inTmp);
  normals = new fvec3[normalCount];
  for (i = 0; i < normalCount; i++) {
    buf = nboUnpackFVec3(buf, normals[i]);
  }

  buf = nboUnpackInt32(buf, inTmp);
  texcoordCount = int(inTmp);
  texcoords = new fvec2[texcoordCount];
  for (i = 0; i < texcoordCount; i++) {
    buf = nboUnpackFVec2(buf, texcoords[i]);
  }

  buf = nboUnpackInt32(buf, inTmp);
  faceSize = int(inTmp);
  faces = new MeshFace*[faceSize];
  faceCount = 0;
  for (i = 0; i < faceSize; i++) {
    MeshFace* face = new MeshFace(this);
    buf = face->unpack(buf);
    if (!face->isValid()) {
      delete face;
    }
    else {
      faces[faceCount] = face;
      faceCount++;
      if (face->isSpecial()) {
        hasSpecialFaces = true;
      }
    }
  }

  if (drawInfoOwner) {
    drawInfo = new MeshDrawInfo();
    buf = drawInfo->unpack(buf);
    if (!BZDB.isTrue("useDrawInfo")) {
      delete drawInfo;
      drawInfo = NULL;
    }
    else {
      // setup the drawInfo arrays
      drawInfo->clientSetup(this);
      if (!drawInfo->isValid()) { 
        delete drawInfo;
        drawInfo = NULL;
      }
    }
  }

  finalize();

  return buf;
}


static void outputFloat(std::ostream& out, float value)
{
  char buffer[32];
  snprintf(buffer, 30, " %.8g", value);
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
  out << indent << "# mins = " << extents.mins.x << " "
			       << extents.mins.y << " "
			       << extents.mins.z << std::endl;
  out << indent << "# maxs = " << extents.maxs.x << " "
			       << extents.maxs.y << " "
			       << extents.maxs.z << std::endl;

  if (!name.empty() && (name[0] != '$')) {
    out << indent << "  name " << name << std::endl;
  }

  if (noclusters) {
    out << indent << "  noclusters" << std::endl;
  }
  if (smoothBounce) {
    out << indent << "  smoothBounce" << std::endl;
  }
  if (driveThrough && shootThrough) {
      out << indent << "  passable" << std::endl;
  } else {
    if (driveThrough) {
      out << indent << "  driveThrough" << std::endl;
    }
    if (shootThrough) {
      out << indent << "  shootThrough" << std::endl;
    }
  }
  if (ricochet) {
    out << indent << "  ricochet" << std::endl;
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
    for (j = 0; j < 2; j++) {
      outputFloat(out, texcoords[i][j]);
    }
    out << std::endl;
  }

  for (int f = 0; f < faceCount; f++) {
    faces[f]->print(out, indent);
  }

  // MeshDrawInfo
  if ((drawInfo != NULL) && !drawInfo->isCopy()) {
    std::string indent2 = indent + "  ";
    drawInfo->print(out, indent2);
  }

  out << indent << "end" << std::endl << std::endl;

  return;
}


void MeshObstacle::printOBJ(std::ostream& out, const std::string& /*indent*/) const
{
  // save as OBJ
  int i;

  out << "# OBJ - start" << std::endl;
  if (!name.empty() && (name[0] != '$')) {
    out << "o " << name << "_" << getObjCounter() << std::endl;
  } else {
    out << "o unnamed_" << getObjCounter() << std::endl;
  }

  out << "# faces = " << faceCount << std::endl;
  out << "# vertices = " << vertexCount << std::endl;
  out << "# normals = " << normalCount << std::endl;
  out << "# texcoords = " << texcoordCount << std::endl;

  const fvec3* tmp;
  tmp = &extents.mins;
  out << "# mins = " << tmp->x << " " << tmp->y << " " << tmp->z << std::endl;
  tmp = &extents.maxs;
  out << "# maxs = " << tmp->x << " " << tmp->y << " " << tmp->z << std::endl;


  for (i = 0; i < vertexCount; i++) {
    out << "v";
    outputFloat(out, vertices[i].x);
    outputFloat(out, vertices[i].y);
    outputFloat(out, vertices[i].z);
    out << std::endl;
  }
  for (i = 0; i < normalCount; i++) {
    out << "vn";
    outputFloat(out, normals[i].x);
    outputFloat(out, normals[i].y);
    outputFloat(out, normals[i].z);
    out << std::endl;
  }
  for (i = 0; i < texcoordCount; i++) {
    out << "vt";
    outputFloat(out, texcoords[i].x);
    outputFloat(out, texcoords[i].y);
    out << std::endl;
  }
  const BzMaterial* bzmat = NULL;
  for (int f = 0; f < faceCount; f++) {
    const MeshFace* face = faces[f];
    const BzMaterial* nextMat = face->getMaterial();
    if (bzmat != nextMat) {
      bzmat = nextMat;
      out << "usemtl ";
      MATERIALMGR.printReference(out, bzmat);
      out << std::endl;
    }
    const int vCount = face->getVertexCount();
    const bool useNormals = face->useNormals();
    const bool useTexcoords = face->useTexcoords();
    out << "f";
    for (i = 0; i < vCount; i++) {
      int vIndex = &face->getVertex(i) - vertices;
      vIndex = vIndex - vertexCount;
      out << " " << vIndex;
      if (useTexcoords) {
	int tIndex = &face->getTexcoord(i) - texcoords;
	tIndex = tIndex - texcoordCount;
	out << "/" << tIndex;
      }
      if (useNormals) {
	if (!useTexcoords) {
	  out << "/";
	}
	int nIndex = &face->getNormal(i) - normals;
	nIndex = nIndex - normalCount;
	out << "/" << nIndex;
      }
    }
    out << std::endl;
  }

  out << "# OBJ - end" << std::endl << std::endl;

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
