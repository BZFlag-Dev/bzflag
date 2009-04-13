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

#include <math.h>
#include <assert.h>

#include "global.h"
#include "Pack.h"
#include "vectors.h"

#include "TetraBuilding.h"
#include "MeshUtils.h"
#include "MeshTransform.h"


////////////////////////////////////////////////////////////////////////////////
//
// TetraBuilding
//


const char* TetraBuilding::typeName = "TetraBuilding";


TetraBuilding::TetraBuilding()
{
  return;
}


TetraBuilding::TetraBuilding(const MeshTransform& xform,
			     const fvec3 _vertices[4],
			     const fvec3 _normals[4][3],
			     const fvec2 _texcoords[4][3],
			     const bool _useNormals[4],
			     const bool _useTexcoords[4],
			     const BzMaterial* _materials[4],
			     unsigned char drive, unsigned char shoot, bool rico)
{
  // tetra specific parameters
  memcpy (vertices, _vertices, sizeof(vertices));
  memcpy (normals, _normals, sizeof(normals));
  memcpy (texcoords, _texcoords, sizeof(texcoords));
  memcpy (useNormals, _useNormals, sizeof(useNormals));
  memcpy (useTexcoords, _useTexcoords, sizeof(useTexcoords));
  memcpy (materials, _materials, sizeof(materials));
  transform = xform;

  // common obstace parameters
  driveThrough = drive;
  shootThrough = shoot;
  ricochet     = rico;

  finalize();

  return;
}


TetraBuilding::~TetraBuilding()
{
  return;
}


Obstacle* TetraBuilding::copyWithTransform(const MeshTransform& xform) const
{
  MeshTransform tmpXform = transform;
  tmpXform.append(xform);

  TetraBuilding* copy =
    new TetraBuilding(tmpXform, vertices, normals, texcoords,
		      useNormals, useTexcoords, (const BzMaterial**)materials,
		      driveThrough, shootThrough, ricochet);
  return copy;
}


void TetraBuilding::finalize()
{
  return;
}


MeshObstacle* TetraBuilding::makeMesh()
{
  MeshObstacle* mesh;
  checkVertexOrder();

  // setup the coordinates
  int i;
  std::vector<char> checkTypes;
  std::vector<fvec3> checkPoints;
  std::vector<fvec3> verts;
  std::vector<fvec3> norms;
  std::vector<fvec2> texcds;

  // setup the inside check point
  fvec3 center(0.0f, 0.0f, 0.0f);
  for (i = 0; i < 4; i++) {
    center[0] += vertices[i][0];
    center[1] += vertices[i][1];
    center[2] += vertices[i][2];
  }
  center[0] *= 0.25f;
  center[1] *= 0.25f;
  center[2] *= 0.25f;
  checkPoints.push_back(center);
  checkTypes.push_back(MeshObstacle::CheckInside);

  for (i = 0; i < 4; i++) {
    verts.push_back(vertices[i]);
  }

  mesh = new MeshObstacle(transform, checkTypes, checkPoints,
                          verts, norms, texcds, 4, false, false,
			  driveThrough, shootThrough, ricochet);

  // add the faces to the mesh
  std::vector<int> vlist;
  std::vector<int> nlist;
  std::vector<int> tlist;

  push3Ints(vlist, 0, 2, 1);
  addFace(mesh, vlist, nlist, tlist, materials[0], -1);
  push3Ints(vlist, 0, 1, 3);
  addFace(mesh, vlist, nlist, tlist, materials[1], -1);
  push3Ints(vlist, 1, 2, 3);
  addFace(mesh, vlist, nlist, tlist, materials[2], -1);
  push3Ints(vlist, 2, 0, 3);
  addFace(mesh, vlist, nlist, tlist, materials[3], -1);

  // wrap it up
  mesh->finalize();

  if (!mesh->isValid()) {
    delete mesh;
    mesh = NULL;
  }
  return mesh;
}


void TetraBuilding::checkVertexOrder()
{
  int v, a;

  // make sure the the planes are facing outwards
  fvec3 edge[3]; // edges from vertex 0
  for (v = 0; v < 3; v++) {
    for (a = 0; a < 3; a++) {
      edge[v][a] = vertices[v+1][a] - vertices[0][a];
    }
  }
  const fvec3 cross = fvec3::cross(edge[0], edge[1]);
  const float dot   = fvec3::dot(cross, edge[2]);

  // swap vertices 1 & 2 if we are out of order
  if (dot < 0.0f) {
    float tmpVertex[3];
    memcpy (tmpVertex, vertices[1], sizeof(tmpVertex));
    memcpy (vertices[1], vertices[2], sizeof(vertices[1]));
    memcpy (vertices[2], tmpVertex, sizeof(vertices[2]));

    float tmpNormals[4][3];
    memcpy (tmpNormals, normals[1], sizeof(tmpNormals));
    memcpy (normals[1], normals[2], sizeof(normals[1]));
    memcpy (normals[2], tmpNormals, sizeof(normals[2]));

    float tmpTexcoords[3][2];
    memcpy (tmpTexcoords, texcoords[1], sizeof(tmpTexcoords));
    memcpy (texcoords[1], texcoords[2], sizeof(texcoords[1]));
    memcpy (texcoords[2], tmpTexcoords, sizeof(texcoords[2]));

    bool tmpBool = useNormals[1];
    useNormals[1] = useNormals[2];
    useNormals[2] = tmpBool;

    tmpBool = useTexcoords[1];
    useTexcoords[1] = useTexcoords[2];
    useTexcoords[2] = tmpBool;

    const BzMaterial* tmpMat = materials[1];
    materials[1] = materials[2];
    materials[2] = tmpMat;
  }

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
  return true;
}


float TetraBuilding::intersect(const Ray&) const
{
  assert(false);
  return -1.0f;
}


void TetraBuilding::get3DNormal(const fvec3&, fvec3&) const
{
  assert(false);
  return;
}


void TetraBuilding::getNormal(const fvec3&, fvec3&) const
{
  assert(false);
  return;
}


bool TetraBuilding::getHitNormal(const fvec3&, float, const fvec3&, float,
			       float, float, float, fvec3&) const
{
  assert(false);
  return false;
}


bool TetraBuilding::inCylinder(const fvec3&,float, float) const
{
  assert(false);
  return false;
}


bool TetraBuilding::inBox(const fvec3&, float, float, float, float) const
{
  assert(false);
  return false;
}


bool TetraBuilding::inMovingBox(const fvec3&, float, const fvec3&, float,
			      float, float, float) const
{
  assert(false);
  return false;
}


bool TetraBuilding::isCrossing(const fvec3& /*p*/, float /*angle*/,
			  float /*dx*/, float /*dy*/, float /*height*/,
			  fvec4* /*_plane*/) const
{
  assert(false);
  return false;
}


////////////////////////////////////////////////////////////////////////////////
//
// Packing stuff
//

static void pack4Bools (unsigned char* byte, const bool bools[4])
{
  *byte = 0;
  for (int i = 0; i < 4; i++) {
    if (bools[i]) {
      *byte = *byte | (1 << i);
    }
  }
  return;
}

static void unpack4Bools (unsigned char byte, bool bools[4])
{
  for (int i = 0; i < 4; i++) {
    if (byte & (1 << i)) {
      bools[i] = true;
    } else {
      bools[i] = false;
    }
  }
  return;
}


void *TetraBuilding::pack(void* buf) const
{
  int v;

  // pack the state byte
  unsigned char stateByte = 0;
  stateByte |= isDriveThrough() ? (1 << 0) : 0;
  stateByte |= isShootThrough() ? (1 << 1) : 0;
  stateByte |= canRicochet()    ? (1 << 2) : 0;
  buf = nboPackUInt8(buf, stateByte);

  // pack the transform
  buf = transform.pack(buf);

  // pack the vertices
  for (v = 0; v < 4; v++) {
    buf = nboPackFVec3(buf, vertices[v]);
  }

  // pack the normals
  unsigned char useNormalsByte;
  pack4Bools (&useNormalsByte, useNormals);
  buf = nboPackUInt8(buf, useNormalsByte);
  for (v = 0; v < 4; v++) {
    if (useNormals[v]) {
      for (int i = 0; i < 3; i++) {
	buf = nboPackFVec3(buf, normals[v][i]);
      }
    }
  }

  // pack the texcoords
  unsigned char useTexcoordsByte;
  pack4Bools (&useTexcoordsByte, useTexcoords);
  buf = nboPackUInt8(buf, useTexcoordsByte);
  for (v = 0; v < 4; v++) {
    if (useTexcoords[v]) {
      for (int i = 0; i < 3; i++) {
	buf = nboPackFloat(buf, texcoords[v][i][0]);
	buf = nboPackFloat(buf, texcoords[v][i][1]);
      }
    }
  }

  // pack the materials
  for (v = 0; v < 4; v++) {
    int matindex = MATERIALMGR.getIndex(materials[v]);
    buf = nboPackInt32(buf, matindex);
  }

  return buf;
}


void *TetraBuilding::unpack(void* buf)
{
  int v;

  // unpack the state byte
  unsigned char stateByte;
  buf = nboUnpackUInt8(buf, stateByte);
  driveThrough = (stateByte & (1 << 0)) != 0 ? 0xFF : 0;
  shootThrough = (stateByte & (1 << 1)) != 0 ? 0xFF : 0;
  ricochet     = (stateByte & (1 << 2)) != 0;

  // unpack the transform
  buf = transform.unpack(buf);

  // unpack the vertices
  for (v = 0; v < 4; v++) {
    buf = nboUnpackFVec3(buf, vertices[v]);
  }

  // unpack the normals
  unsigned char useNormalsByte;
  buf = nboUnpackUInt8(buf, useNormalsByte);
  unpack4Bools (useNormalsByte, useNormals);
  for (v = 0; v < 4; v++) {
    if (useNormals[v]) {
      for (int i = 0; i < 3; i++) {
	buf = nboUnpackFVec3(buf, normals[v][i]);
      }
    }
  }

  // unpack the texcoords
  unsigned char useTexcoordsByte;
  buf = nboUnpackUInt8(buf, useTexcoordsByte);
  unpack4Bools (useTexcoordsByte, useTexcoords);
  for (v = 0; v < 4; v++) {
    if (useTexcoords[v]) {
      for (int i = 0; i < 3; i++) {
	buf = nboUnpackFloat(buf, texcoords[v][i][0]);
	buf = nboUnpackFloat(buf, texcoords[v][i][1]);
      }
    }
  }

  // unpack the materials
  for (v = 0; v < 4; v++) {
    int32_t matindex;
    buf = nboUnpackInt32(buf, matindex);
    materials[v] = MATERIALMGR.getMaterial(matindex);
  }

  finalize();

  return buf;
}


int TetraBuilding::packSize() const
{
  int v;
  int fullSize = transform.packSize();
  // state byte
  fullSize = fullSize + sizeof(unsigned char);
  // vectors
  fullSize = fullSize + (4 * sizeof(float[3]));
  // normals
  fullSize = fullSize + sizeof(unsigned char);
  for (v = 0; v < 4; v++) {
    if (useNormals[v]) {
      fullSize = fullSize + sizeof(float[3][3]);
    }
  }
  // texcoords
  fullSize = fullSize + sizeof(unsigned char);
  for (v = 0; v < 4; v++) {
    if (useTexcoords[v]) {
      fullSize = fullSize + sizeof(float[3][2]);
    }
  }
  // materials
  fullSize = fullSize + sizeof(int32_t[4]);

  return fullSize;
}


void TetraBuilding::print(std::ostream& out, const std::string& indent) const
{
  int i;

  out << indent << "tetra" << std::endl;

  transform.printTransforms(out, "");

  // write the vertex information
  for (i = 0; i < 4; i++) {
    const fvec3& vertex = vertices[i];
    out << indent << "  vertex " << vertex[0] << " " << vertex[1] << " "
				 << vertex[2] << std::endl;
    if (useNormals[i]) {
      for (int j = 0; j < 3; j++) {
	const fvec3& normal = normals[i][j];
	out << indent << "  normal " << normal[0] << " " << normal[1] << " "
				     << normal[2] << std::endl;
      }
    }
    if (useTexcoords[i]) {
      for (int j = 0; j < 3; j++) {
	const fvec2& texcoord = texcoords[i][j];
	out << indent << "  normal " << texcoord[0] << " "
				     << texcoord[1] << " "
				     << texcoord[2] << std::endl;
      }
    }
    out << "  matref ";
    MATERIALMGR.printReference(out, materials[i]);
    out << std::endl;
  }

  // write the regular stuff
  if (isPassable()) {
    out << indent << "  passable" << std::endl;
  } else {
    if (isDriveThrough()) {
      out << indent << "  drivethrough" << std::endl;
    }
    if (isShootThrough()) {
      out << indent << "  shootthrough" << std::endl;
    }
  }
  if (canRicochet()) {
    out << indent << "  ricochet" << std::endl;
  }
  out << indent << "end" << std::endl << std::endl;

  return;
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
