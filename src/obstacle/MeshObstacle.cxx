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

#include "common.h"

// implementatino header
#include "MeshObstacle.h"

// system headers
#include <math.h>
#include <stdlib.h>

// common headers
#include "bzfio.h"
#include "global.h"
#include "Pack.h"
#include "MeshFace.h"
#include "CollisionManager.h"
#include "Intersect.h"
#include "MeshDrawInfo.h"
#include "MeshTransform.h"
#include "StateDatabase.h"
#include "TextUtils.h"

// local headers
#include "Triangulate.h"


const char*	MeshObstacle::typeName = "MeshObstacle";


MeshObstacle::MeshObstacle()
{
  faceCount = faceSize = 0;
  faces = NULL;
  edges = NULL;
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
  invertedTransform = false;
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
  invertedTransform = xformtool.isInverted();

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

  edges = NULL;

  noclusters = _noclusters;
  smoothBounce = bounce;
  driveThrough = drive;
  shootThrough = shoot;
  ricochet = rico;

  hasSpecialFaces = false;

  drawInfo = NULL;

  return;
}


bool MeshObstacle::addWeapon(const std::vector<std::string>& lines)
{
  weapons.push_back(lines);
  return true;
}


static bool hasZoneFixedFlag(const MeshFace::SpecialData* sd)
{
  if ((sd == NULL) || sd->zoneParams.empty()) {
    return false;
  }
  for (size_t i = 0; i < sd->zoneParams.size(); i++) {
    const std::string& line = sd->zoneParams[i];
    if ((strncasecmp(line.c_str(), "zonezoneflag",  12) == 0) ||
        (strncasecmp(line.c_str(), "zonefixedflag", 13) == 0)) {
      return true;
    }
  }
  return false;
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
    }
    else {
      // 
      logDebugMessage(1,
        "WARNING: face triangulated into %u new faces\n", triSize);
      // warn if a zone with fixedFlag was split
      if (hasZoneFixedFlag(sd)) {
        logDebugMessage(0, "WARNING: face zone with fixedFlags was split\n");
      }
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
    const int index = (invertedTransform ? ((count - 1) - i) : i);
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

  for (size_t w = 0; w < weapons.size(); w++) {
    copy->addWeapon(weapons[w]);
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
    faces[f]->setFaceID(f);
  }

  // setup the face edges
  makeEdges();

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


//============================================================================//

struct VertPair {
  VertPair() : v0(NULL), v1(NULL) {}
  VertPair(const fvec3* _v0, const fvec3* _v1) : v0(_v0), v1(_v1) {}
  const fvec3* v0;
  const fvec3* v1;
  bool operator<(const VertPair& vp) const {
    if (v0 < vp.v0) { return true; }
    if (vp.v0 < v0) { return false; }
    if (v1 < vp.v1) { return true; }
    if (vp.v1 < v1) { return false; }
    return false;
  }
};

struct FacePair {
  FacePair() : f0(NULL), f1(NULL) {}
  FacePair(MeshFace* _f0, MeshFace* _f1) : f0(_f0), f1(_f1) {}
  MeshFace* f0;
  MeshFace* f1;
};

typedef std::map<VertPair, FacePair> EdgeMap;

typedef std::map<MeshFace*, std::vector<MeshFace::EdgeRef> > FaceEdges;


void MeshObstacle::makeEdges() // FIXME -- incomplete
{
  if (isPassable()) {
    return;
  }

  EdgeMap edgeMap;
  for (int f = 0; f < faceCount; f++) {
    MeshFace* face = faces[f];
    const int vertCount = face->getVertexCount();
    const fvec3** verts = face->vertices;
    for (int v = 0; v < vertCount; v++) {
      const fvec3* v0 = verts[v];
      const fvec3* v1 = verts[(v + 1) % vertCount];
      bool useFace0 = true;
      if (v0 > v1) {
        const fvec3* vt = v1;
        v1 = v0;
        v0 = vt;
        useFace0 = false;
      }
      FacePair& facePair = edgeMap[VertPair(v0, v1)];
      if (useFace0) {
        if (facePair.f0 == NULL) {
          facePair.f0 = face;
        } else {
          logDebugMessage(4, "non-manifold mesh -- f0 overlap\n");
          return;
        }
      } else {
        if (facePair.f1 == NULL) {
          facePair.f1 = face;
        } else {
          logDebugMessage(4, "non-manifold mesh -- f1 overlap\n");
          return;
        }
      }
    }
  }

  EdgeMap::const_iterator edgeIt;
  for (edgeIt = edgeMap.begin(); edgeIt != edgeMap.end(); ++edgeIt) {
    const FacePair& facePair = edgeIt->second;
    if (facePair.f0 == NULL) {
      logDebugMessage(4, "non-manifold mesh -- missing f0\n");
      return;
    }
    if (facePair.f1 == NULL) {
      logDebugMessage(4, "non-manifold mesh -- missing f1\n");
      return;
    }
  }

  edgeCount = (int)edgeMap.size();
  edges = new MeshFace::Edge[edgeCount];

  FaceEdges faceEdges;

  int e = 0;
  for (edgeIt = edgeMap.begin(); edgeIt != edgeMap.end(); ++edgeIt, ++e) {
    const VertPair& vertPair = edgeIt->first;
    const FacePair& facePair = edgeIt->second;
    MeshFace::Edge& edge = edges[e];
    edge.v0 = vertPair.v0;
    edge.v1 = vertPair.v1;
    edge.f0 = facePair.f0;
    edge.f1 = facePair.f1;
    edge.polarity = 0; // clear it

    faceEdges[facePair.f0].push_back(MeshFace::EdgeRef(&edge, +1));
    faceEdges[facePair.f1].push_back(MeshFace::EdgeRef(&edge, -1));
  }

/* FIXME -- add the edges to the faces
  FaceEdges::const_iterator feIt;
  for (feIt = faceEdges.begin(); feIt != faceEdges.end(); ++feIt) {
    MeshFace* face = feIt->first;
    face->edges = new MeshFace::EdgeRef[face->getVertexCount()];
    const std::vector<MeshFace::EdgeRef>& refs = feIt->second;
    std::vector<MeshFace::EdgeRef>::const_iterator refIt;
    for (size_t r = 0; r < refs.size(); r++) {
      face->edges[r] = refs[r];
    }
  }
*/
}


//============================================================================//

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
    logDebugMessage(0,
                    "ERROR: MeshObstacle::setMeshDrawInfo() already exists\n");
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
  if (checkCount <= 0) {
    return false;
  }

  const CheckType ct0 = (CheckType)checkTypes[0];
  switch (ct0) {
    case Convex:             { return containsPointConvex(point, false); }
    case ConvexTrace:        { return containsPointConvex(point, true);  }
    case InsideParity:       { return containsPointParity(point, true,  false); }
    case OutsideParity:      { return containsPointParity(point, false, false); }
    case InsideParityTrace:  { return containsPointParity(point, true,  true);  }
    case OutsideParityTrace: { return containsPointParity(point, false, true);  }
    default: {
      break; // pass through
    }
  }

  int c, f;
  fvec3 dir;
  bool hasOutsides = false;

  for (c = 0; c < checkCount; c++) {
    if (checkTypes[c] == InsideCheck) {
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
    else if (checkTypes[c] == OutsideCheck) {
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
      logDebugMessage(0, "checkType (%i) is not supported yet\n",
                         checkTypes[c]);
      exit (1);
    }
  }

  return hasOutsides;
}


bool MeshObstacle::containsPointConvex(const fvec3& point, bool trace) const
{
  
  return trace && point.x != 0.0f; // FIXME
}


bool MeshObstacle::containsPointParity(const fvec3& point,
                                       bool inside, bool trace) const
{
  
  return inside && trace && point.x != 0.0f; // FIXME
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

  fullSize += sizeof(uint32_t); // weaponCount
  for (size_t w = 0; w < weapons.size(); w++) {
    fullSize += sizeof(uint32_t); // lineCount
    for (size_t l = 0; l < weapons[w].size(); l++) {
      fullSize += nboStdStringPackSize(weapons[w][l]);
    }
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
  stateByte |= canRicochet()    ? (1 << 2) : 0;
  stateByte |= smoothBounce     ? (1 << 3) : 0;
  stateByte |= noclusters       ? (1 << 4) : 0;
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

  buf = nboPackUInt32(buf, weapons.size());
  for (size_t w = 0; w < weapons.size(); w++) {
    buf = nboPackUInt32(buf, weapons[w].size());
    for (size_t l = 0; l < weapons[w].size(); l++) {
      buf = nboPackStdString(buf, weapons[w][l]);
    }
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
  ricochet      = (stateByte & (1 << 2)) != 0;
  smoothBounce  = (stateByte & (1 << 3)) != 0;
  noclusters    = (stateByte & (1 << 4)) != 0;
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
    // setup the drawInfo arrays
    drawInfo->clientSetup(this);
    if (!drawInfo->isValid()) {
      delete drawInfo;
      drawInfo = NULL;
    }
  }

  uint32_t weaponCount, lineCount;
  buf = nboUnpackUInt32(buf, weaponCount);
  for (size_t w = 0; w < weaponCount; w++) {
    std::vector<std::string> lines;
    buf = nboUnpackUInt32(buf, lineCount);
    for (size_t l = 0; l < lineCount; l++) {
      std::string line;
      buf = nboUnpackStdString(buf, line);
      lines.push_back(line);
    }
    weapons.push_back(lines);
  }

  finalize();

  return buf;
}


static std::string debugIndex(int index)
{
  if (debugLevel >= 1) {
    return " # " + TextUtils::itoa(index);
  }
  return "";
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

  int i;
  for (i = 0; i < checkCount; i++) {
    switch (checkTypes[i]) {
      case InsideCheck: {
        out << indent << "  inside"  << checkPoints[i] << std::endl;
        break;
      }
      case OutsideCheck: {
        out << indent << "  outside"  << checkPoints[i] << std::endl;
        break;
      }
    }
  }
  for (i = 0; i < vertexCount; i++) {
    out << indent << "  vertex"   << vertices[i]  << debugIndex(i) << std::endl;
  }
  for (i = 0; i < normalCount; i++) {
    out << indent << "  normal"   << normals[i]   << debugIndex(i) << std::endl;
  }
  for (i = 0; i < texcoordCount; i++) {
    out << indent << "  texcoord" << texcoords[i] << debugIndex(i) << std::endl;
  }

  // weapons
  for (size_t w = 0; w < weapons.size(); w++) {
    out << indent << "  weapon" << std::endl;
    for (size_t l = 0; l < weapons[w].size(); l++) {
      out << indent << "    " << weapons[w][l] << std::endl;
    }
    out << indent << "  endweapon" << std::endl;
  }

  // faces
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


bool MeshObstacle::makeTexcoords(const fvec2& autoScale,
                                 const fvec4& plane,
                                 const std::vector<fvec3>& vertices,
                                 std::vector<fvec2>& texcoords)
{
  const float defScale = 1.0f / 8.0f;
  const float sScale = (autoScale.s == 0.0f) ? defScale : 1.0f / autoScale.s;
  const float tScale = (autoScale.t == 0.0f) ? defScale : 1.0f / autoScale.t;

  fvec3 x = fvec3(vertices[1]) - fvec3(vertices[0]);
  fvec3 y = fvec3::cross(plane.xyz(), x);

  if (!fvec3::normalize(x) ||
      !fvec3::normalize(y)) {
    return false;
  }

  texcoords.resize(vertices.size());

  const bool horizontal = fabsf(plane[2]) > 0.999f;

  const int count = (int)vertices.size();
  for (int i = 0; i < count; i++) {
    const fvec3& v = vertices[i];
    const fvec3 delta = fvec3(v) - vertices[0];
    const fvec2 nh = fvec2(plane.x, plane.y).normalize();
    const float vs = 1.0f / sqrtf(1.0f - (plane.z * plane.z));

    if (sScale < 0.0f) {
      texcoords[i].s = -sScale * fvec3::dot(delta, x);
    }
    else {
      if (horizontal) {
        texcoords[i].s = sScale * v.x;
      } else {
        texcoords[i].s = sScale * ((nh.x * v.y) - (nh.y * v.x));
      }
    }

    if (tScale < 0.0f) {
      texcoords[i].t = -tScale * fvec3::dot(delta, y);
    }
    else {
      if (horizontal) {
        texcoords[i].t = tScale * v.y;
      } else {
        texcoords[i].t = tScale * (v.z * vs);
      }
    }
  }

  return true;
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

  int f;
  const MeshFace* face;

  std::vector<fvec3> extraNormals;
  std::vector<fvec2> extraTexcoords;

  // generate missing texcoords and normals
  for (f = 0; f < faceCount; f++) {
    face = faces[f];
    if (!face->useNormals()) {
      extraNormals.push_back(face->getPlane().xyz());
    }
    if (!face->useTexcoords()) {
      const BzMaterial* bzmat = face->getMaterial();
      const int vertCount = face->getVertexCount();
      std::vector<fvec3> vertArray;
      std::vector<fvec2> txcdArray;
      for (int v = 0; v < vertCount; v++) {
        vertArray.push_back(face->getVertex(v));
      }
      const fvec2& autoScale = bzmat->getTextureAutoScale(0);
      makeTexcoords(autoScale, face->getPlane(), vertArray, txcdArray);
      if ((int)txcdArray.size() != vertCount) {
        logDebugMessage(0, "WARNING: messed up generated texcoords\n");
      }
      for (size_t t = 0; t < txcdArray.size(); t++) {
        extraTexcoords.push_back(txcdArray[t]);
      }
    }
  }

  const int fullNormCount = normalCount   + (int)extraNormals.size();
  const int fullTxcdCount = texcoordCount + (int)extraTexcoords.size();

  // informative comment block
  out << "# faces = "     << faceCount     << std::endl;
  out << "# vertices = "  << vertexCount   << std::endl;
  out << "# normals = "   << normalCount   << std::endl;
  out << "# texcoords = " << texcoordCount << std::endl;
  if (!extraNormals.empty()) {
    out << "# generated normals = " << extraNormals.size() << std::endl;
  }
  if (!extraTexcoords.empty()) {
    out << "# generated texcoords = " << extraTexcoords.size() << std::endl;
  }
  out << "# mins = " << extents.mins << std::endl;
  out << "# maxs = " << extents.maxs << std::endl;


  for (i = 0; i < vertexCount; i++) {
    out << "v" << vertices[i] << debugIndex(i) << std::endl;
  }
  for (i = 0; i < normalCount; i++) {
    out << "vn" << normals[i] << debugIndex(i) << std::endl;
  }
  const int nc = normalCount;
  for (size_t j = 0; j < extraNormals.size(); j++) {
    out << "vn" << extraNormals[j] << debugIndex(nc + j) << std::endl;
  }
  for (i = 0; i < texcoordCount; i++) {
    out << "vt" << texcoords[i] << debugIndex(i) << std::endl;
  }
  const int tc = texcoordCount;
  for (size_t j = 0; j < extraTexcoords.size(); j++) {
    out << "vt" << extraTexcoords[j] << debugIndex(tc + j) << std::endl;
  }

  int usedExtraNorms = 0;
  int usedExtraTxcds = 0;

  const BzMaterial* bzmat = NULL;

  for (f = 0; f < faceCount; f++) {
    face = faces[f];

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
      // vertices
      int vIndex = &face->getVertex(i) - vertices;
      vIndex = vIndex - vertexCount;
      out << " " << vIndex;

      // texcoords
      if (useTexcoords) {
	int tIndex = &face->getTexcoord(i) - texcoords;
	tIndex = tIndex - fullTxcdCount;
	out << "/" << tIndex;
      } else {
        out << "/" << usedExtraTxcds - (int)extraTexcoords.size();
        usedExtraTxcds++;
      }

      // normals
      if (useNormals) {
	int nIndex = &face->getNormal(i) - normals;
	nIndex = nIndex - fullNormCount;
	out << "/" << nIndex;
      }
      else {
        out << "/" << usedExtraNorms - (int)extraNormals.size();
      }
    }

    if (!useNormals) {
      usedExtraNorms++;
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
