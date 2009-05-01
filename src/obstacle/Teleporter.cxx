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

// system headers
#include <math.h>
#include <assert.h>

// common headers
#include "global.h"
#include "Pack.h"
#include "Teleporter.h"
#include "Intersect.h"
#include "MeshTransform.h"
#include "BzMaterial.h"
#include "DynamicColor.h"
#include "TextureMatrix.h"

// local headers
#include "MeshUtils.h"


const char* Teleporter::typeName = "Teleporter";


//============================================================================//

Teleporter::Teleporter()
{
  return;
}


Teleporter::Teleporter(const MeshTransform& xform,
                       const fvec3& p, float a,
                       float w, float b, float h,
                       float _border, float _texSize,
		       unsigned char drive, unsigned char shoot, bool rico)
: Obstacle(p, a, w, b, h, drive, shoot, rico)
, border(_border)
, texSize(_texSize)
, transform(xform)
{
  border = fabsf(border);
  size.x = fabsf(size.x);
  size.y = fabsf(size.y);
  size.z = fabsf(size.z);
  size.y = (size.y > 0.001f) ? size.y : 0.001f;
  size.z = (size.z > 0.001f) ? size.z : 0.001f;

  finalize();

  return;
}


Teleporter::~Teleporter()
{
  return;
}


Obstacle* Teleporter::copyWithTransform(const MeshTransform& xform) const
{
  MeshTransform tmpXform = transform;
  tmpXform.append(xform);

  Teleporter* copy =
    new Teleporter(tmpXform, pos, angle, size.x, size.y, size.z,
                   border, texSize, driveThrough, shootThrough, ricochet);

  copy->setName(name);

  return copy;
}


void Teleporter::finalize()
{
  return;
}


const char* Teleporter::getType() const
{
  return typeName;
}


const char* Teleporter::getClassName() // const
{
  return typeName;
}


bool Teleporter::isValid() const
{
  return true;
}


//============================================================================//

MeshObstacle* Teleporter::makeMesh()
{
  MeshObstacle* mesh;

  const bool wantBorder = (border >= 0.001f);

  // setup the texture scale
  float texScale;
  if (texSize == 0.0f) {
    texScale = 1.0f / border;
  }
  else if (texSize > 0.0f) {
    texScale = 1.0f / texSize;
  }
  else {
    texScale = -texSize;
  }


  // setup the transform
  MeshTransform tmpXform;
  tmpXform.addSpin((angle * RAD2DEGf), fvec3(0.0f, 0.0f, 1.0f));
  tmpXform.addShift(pos);
  tmpXform.append(transform);

  std::vector<char>  checkTypes;
  std::vector<fvec3> checkPoints;
  std::vector<fvec3> vertices;
  std::vector<fvec3> normals;
  std::vector<fvec2> texcoords;

  const float br = border;
  const float hb = br * 0.5f;
  const float yo = size.y + br;
  const float ym = size.y + hb;
  const float yi = size.y;
  const float xl = size.x;
  const float xb = (size.x > hb) ? size.x : hb;
  const float zt = size.z + br;
  const float zm = size.z + hb;
  const float zb = size.z;

  // link texcoords
  const float ztxc = zb / (2.0f * yi);
  texcoords.push_back(fvec2(0.0f, 0.0f)); // t0
  texcoords.push_back(fvec2(1.0f, 0.0f)); // t1
  texcoords.push_back(fvec2(1.0f, ztxc)); // t2
  texcoords.push_back(fvec2(0.0f, ztxc)); // t3

  // back face vertices  (-x normal)
  vertices.push_back(fvec3(-xl, +yi, 0.0f)); // v0
  vertices.push_back(fvec3(-xl, -yi, 0.0f)); // v1
  vertices.push_back(fvec3(-xl, -yi,   zb)); // v2
  vertices.push_back(fvec3(-xl, +yi,   zb)); // v3
  // front face vertices (+x normal)
  vertices.push_back(fvec3(+xl, -yi, 0.0f)); // v4
  vertices.push_back(fvec3(+xl, +yi, 0.0f)); // v5
  vertices.push_back(fvec3(+xl, +yi,   zb)); // v6
  vertices.push_back(fvec3(+xl, -yi,   zb)); // v7

  if (wantBorder) {
    //  border vertex-index layout
    //  (outside for -X, inside for +X)
    //  (ip = inside point)
    //
    //                +Z
    //
    //  15+------------------------+14
    //    |22                    23|
    //    |                        |
    //    |   ip              ip   |
    //    |                        |
    //    |      19+------+18      |
    // +Y |        |10  11|        | -Y
    //    |        |      |        |
    //    |        |      |        |
    //    |21    20|      |17    16|
    //   8+--------+9   12+--------+13
    //
    //                -Z

    checkTypes.push_back(MeshObstacle::CheckInside);
    checkTypes.push_back(MeshObstacle::CheckInside);
    checkPoints.push_back(fvec3(0.0f, +ym, +zm));
    checkPoints.push_back(fvec3(0.0f, -ym, +zm));

    vertices.push_back(fvec3(-xb, +yo, 0.0f)); // v8
    vertices.push_back(fvec3(-xb, +yi, 0.0f)); // v9
    vertices.push_back(fvec3(-xb, +yi,   zb)); // v10
    vertices.push_back(fvec3(-xb, -yi,   zb)); // v11
    vertices.push_back(fvec3(-xb, -yi, 0.0f)); // v12
    vertices.push_back(fvec3(-xb, -yo, 0.0f)); // v13
    vertices.push_back(fvec3(-xb, -yo,   zt)); // v14
    vertices.push_back(fvec3(-xb, +yo,   zt)); // v15

    vertices.push_back(fvec3(+xb, -yo, 0.0f)); // v16
    vertices.push_back(fvec3(+xb, -yi, 0.0f)); // v17
    vertices.push_back(fvec3(+xb, -yi,   zb)); // v18
    vertices.push_back(fvec3(+xb, +yi,   zb)); // v19
    vertices.push_back(fvec3(+xb, +yi, 0.0f)); // v20
    vertices.push_back(fvec3(+xb, +yo, 0.0f)); // v21
    vertices.push_back(fvec3(+xb, +yo,   zt)); // v22
    vertices.push_back(fvec3(+xb, -yo,   zt)); // v23

    // t4 - t11
    const fvec2 xTexOffset(+yo, -zt);
    for (size_t i = 16; i < 24; i++) {
      texcoords.push_back(texScale * (xTexOffset + vertices[i].yz()));
    }

    const float xb2 = xb * 2.0f;
    const float yo2 = yo * 2.0f;
    const float b2yi = br + (yi * 2.0f);
    texcoords.push_back(texScale * fvec2(+xb2,  0.0f)); // t12
    texcoords.push_back(texScale * fvec2(0.0f,   -zt)); // t13
    texcoords.push_back(texScale * fvec2(+xb2,   -zt)); // t14
    texcoords.push_back(texScale * fvec2(0.0f,   -br)); // t15
    texcoords.push_back(texScale * fvec2(+xb2,   -br)); // t16
    texcoords.push_back(texScale * fvec2(+yo2,  0.0f)); // t17
    texcoords.push_back(texScale * fvec2(0.0f,  -xb2)); // t18
    texcoords.push_back(texScale * fvec2(+yo2,  -xb2)); // t19
    texcoords.push_back(texScale * fvec2(+br,   0.0f)); // t20
    texcoords.push_back(texScale * fvec2(+br,   -xb2)); // t21
    texcoords.push_back(texScale * fvec2(+b2yi, 0.0f)); // t22
    texcoords.push_back(texScale * fvec2(+b2yi, -xb2)); // t23
  }

  const int faceCount = 2 + (wantBorder ? 14 : 0);

  mesh = new MeshObstacle(tmpXform, checkTypes, checkPoints,
                          vertices, normals, texcoords, faceCount,
                          false, false,
                          driveThrough, shootThrough, ricochet);
  mesh->setName(name);

  std::vector<int> vlist;
  std::vector<int> nlist;
  std::vector<int> tlist;

  const BzMaterial* linkMat   = getLinkMaterial();
  const BzMaterial* borderMat = getTeleMaterial();

  MeshFace::SpecialData sd;

  // back face
  sd.linkName = "b";
  push4Ints(vlist, 0, 1, 2, 3); push4Ints(tlist, 0, 1, 2, 3);
  mesh->addFace(vlist, nlist, tlist,
                linkMat, -1, false, false,
                0xFF, 0xFF, false, false, &sd);
  vlist.clear(); tlist.clear();

  // front face
  sd.linkName = "f";
  push4Ints(vlist, 4, 5, 6, 7); push4Ints(tlist, 0, 1, 2, 3);
  mesh->addFace(vlist, nlist, tlist,
                linkMat, -1, false, false,
                0xFF, 0xFF, false, false, &sd);
  vlist.clear(); tlist.clear();

  // -x faces
  push4Ints(vlist,  8,  9, 10, 15); push4Ints(tlist, 4, 5,  6, 11);
  addFace(mesh, vlist, nlist, tlist, borderMat, -1);
  push4Ints(vlist, 10, 11, 14, 15); push4Ints(tlist, 6, 7, 10, 11);
  addFace(mesh, vlist, nlist, tlist, borderMat, -1);
  push4Ints(vlist, 12, 13, 14, 11); push4Ints(tlist, 8, 9, 10, 7);
  addFace(mesh, vlist, nlist, tlist, borderMat, -1);
  // +x faces
  push4Ints(vlist, 16, 17, 18, 23); push4Ints(tlist, 4, 5,  6, 11);
  addFace(mesh, vlist, nlist, tlist, borderMat, -1);
  push4Ints(vlist, 18, 19, 22, 23); push4Ints(tlist, 6, 7, 10, 11);
  addFace(mesh, vlist, nlist, tlist, borderMat, -1);
  push4Ints(vlist, 20, 21, 22, 19); push4Ints(tlist, 8, 9, 10, 7);
  addFace(mesh, vlist, nlist, tlist, borderMat, -1);

  // +y outside
  push4Ints(vlist, 21,  8, 15, 22); push4Ints(tlist, 13, 14, 12,  0);
  addFace(mesh, vlist, nlist, tlist, borderMat, -1);
  // -y outside
  push4Ints(vlist, 13, 16, 23, 14); push4Ints(tlist, 13, 14, 12,  0);
  addFace(mesh, vlist, nlist, tlist, borderMat, -1);
  // +y inside
  push4Ints(vlist,  9, 20, 19, 10); push4Ints(tlist, 13, 14, 16, 15);
  addFace(mesh, vlist, nlist, tlist, borderMat, -1);
  // -y inside
  push4Ints(vlist, 17, 12, 11, 18); push4Ints(tlist, 13, 14, 16, 15);
  addFace(mesh, vlist, nlist, tlist, borderMat, -1);
  // crossbar top
  push4Ints(vlist, 14, 23, 22, 15); push4Ints(tlist,  0, 18, 19, 17);
  addFace(mesh, vlist, nlist, tlist, borderMat, -1);
  // crossbar bottom
  push4Ints(vlist, 10, 19, 18, 11); push4Ints(tlist, 20, 21, 23, 22);
  addFace(mesh, vlist, nlist, tlist, borderMat, -1);
  // +y pillar cap
  push4Ints(vlist,  8, 21, 20,  9); push4Ints(tlist,  0, 18, 21, 20);
  addFace(mesh, vlist, nlist, tlist, borderMat, -1);
  // -y pillar cap
  push4Ints(vlist, 12, 17, 16, 13); push4Ints(tlist,  0, 18, 21, 20);
  addFace(mesh, vlist, nlist, tlist, borderMat, -1);

  // wrap it up
  mesh->finalize();

  if (mesh->isValid()) {
    return mesh;
  } else {
    delete mesh;
    return NULL;
  }
}


const BzMaterial* Teleporter::getLinkMaterial()
{
  const std::string matName = "LinkMaterial";

  const BzMaterial* matPtr = MATERIALMGR.findMaterial(matName);
  if (matPtr != NULL) {
    return matPtr;
  }

  int dyncolID = DYNCOLORMGR.findColor(matName);
  if (dyncolID < 0) {
    DynamicColor* dyncol = new DynamicColor;
    dyncol->addState(0.6f, 0.5f, 0.0f, 0.0f, 0.75f);
    dyncol->addState(0.6f, 0.0f, 0.3f, 0.0f, 0.75f);
    dyncol->addState(0.6f, 0.0f, 0.0f, 0.7f, 0.75f);
    dyncol->setName(matName);
    dyncol->finalize();
    dyncolID = DYNCOLORMGR.addColor(dyncol);
  }

  int texmatID = TEXMATRIXMGR.findMatrix(matName);
  if (texmatID < 0) {
    TextureMatrix* texmat = new TextureMatrix;
    texmat->setDynamicShift(0.0f, -0.05f);
    texmat->setName(matName);
    texmat->finalize();
    texmatID = TEXMATRIXMGR.addMatrix(texmat);
  }

  BzMaterial mat;
  const fvec4 color(0.0f, 0.0f, 0.0f, 0.5f);
  mat.setDiffuse(color);
  mat.setDynamicColor(dyncolID);
  mat.setTexture("telelink");
  mat.setTextureMatrix(texmatID);
  mat.setNoLighting(true);
  mat.setName(matName);

  return MATERIALMGR.addMaterial(&mat);
}


const BzMaterial* Teleporter::getTeleMaterial()
{
  const std::string matName = "TeleMaterial";

  const BzMaterial* matPtr = MATERIALMGR.findMaterial(matName);
  if (matPtr != NULL) {
    return matPtr;
  }

  BzMaterial mat;
  const fvec4 color(1.0f, 1.0f, 1.0f, 1.0f);
  mat.setDiffuse(color);
  mat.setTexture("caution");
  mat.setName(matName);

  return MATERIALMGR.addMaterial(&mat);
}


//============================================================================//

float Teleporter::intersect(const Ray&) const
{
  assert(false);
  return -1.0f;
}


void Teleporter::getNormal(const fvec3&, fvec3&) const
{
  assert(false);
  return;
}


void Teleporter::get3DNormal(const fvec3&, fvec3&) const
{
  assert(false);
  return;
}


bool Teleporter::getHitNormal(const fvec3&, float, const fvec3&, float,
                              float, float, float, fvec3&) const
{
  assert(false);
  return false;
}


bool Teleporter::inCylinder(const fvec3& p, float radius, float height) const
{
  // used by bzfs/WorldGenerators.cxx via bzfs/WorldInfo::inCylinderNoOctree()
  // (note that the bzfs code does not use the MeshTransform capabilities)

  if (p.z > (pos.z + size.z + border)) {
    return false;
  }
  if ((p.z + height) < pos.z) {
    return false;
  }

  const float xsize = (size.x > border) ? size.x : border;
  const float ysize = (size.y + border);

  return Intersect::testRectCircle(pos, angle, xsize, ysize, p, radius);
}


bool Teleporter::inBox(const fvec3&, float, float, float, float) const
{
  assert(false);
  return false;
}


bool Teleporter::inMovingBox(const fvec3&, float, const fvec3&, float,
                             float, float, float) const
{
  assert(false);
  return false;
}


bool Teleporter::isCrossing(const fvec3& /*p*/, float /*angle*/,
                            float /*dx*/, float /*dy*/, float /*height*/,
                            fvec4* /*_plane*/) const
{
  assert(false);
  return false;
}


//============================================================================//

void* Teleporter::pack(void* buf) const
{
  buf = nboPackStdString(buf, name);

  buf = nboPackFVec3(buf, pos);
  buf = nboPackFloat(buf, angle);
  buf = nboPackFVec3(buf, size);
  buf = nboPackFloat(buf, border);
  buf = nboPackFloat(buf, texSize);
  buf = transform.pack(buf);

  unsigned char stateByte = 0;
  stateByte |= isDriveThrough() ? _DRIVE_THRU : 0;
  stateByte |= isShootThrough() ? _SHOOT_THRU : 0;
  stateByte |= canRicochet()    ? _RICOCHET   : 0;
  buf = nboPackUInt8(buf, stateByte);

  return buf;
}


void* Teleporter::unpack(void* buf)
{
  buf = nboUnpackStdString(buf, name);

  buf = nboUnpackFVec3(buf, pos);
  buf = nboUnpackFloat(buf, angle);
  buf = nboUnpackFVec3(buf, size);
  buf = nboUnpackFloat(buf, border);
  buf = nboUnpackFloat(buf, texSize);
  buf = transform.unpack(buf);
  unsigned char stateByte;
  buf = nboUnpackUInt8(buf, stateByte);
  driveThrough = ((stateByte & _DRIVE_THRU) != 0) ? 0xFF : 0;
  shootThrough = ((stateByte & _SHOOT_THRU) != 0) ? 0xFF : 0;
  ricochet     = ((stateByte & _RICOCHET) != 0);

  finalize();

  return buf;
}


int Teleporter::packSize() const
{
  int fullSize = 0;
  fullSize += nboStdStringPackSize(name);
  fullSize += sizeof(fvec3);   // pos
  fullSize += sizeof(float);   // rotation
  fullSize += sizeof(fvec3);   // size
  fullSize += sizeof(float);   // border
  fullSize += sizeof(float);   // texSize
  fullSize += transform.packSize();
  fullSize += sizeof(uint8_t); // state bits
  return fullSize;
}


void Teleporter::print(std::ostream& out, const std::string& indent) const
{
  out << indent << "teleporter";
  if (!name.empty() && (name[0] != '$')) {
    out << " " << name;
  }
  out << std::endl;
  out << indent << "  position " << pos.x << " " << pos.y << " "
				 << pos.z << std::endl;
  out << indent << "  size " << size.x << " " << size.y << " "
			     << size.z << std::endl;
  out << indent << "  rotation " << (angle * RAD2DEGf) << std::endl;
  out << indent << "  border " << border << std::endl;

  if (texSize != 0.0f) {
    out << indent << "  texsize " << texSize << std::endl;
  }

  transform.printTransforms(out, indent);

  if (ricochet) {
    out << indent << "  ricochet" << std::endl;
  }

  out << indent << "end" << std::endl << std::endl;
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
