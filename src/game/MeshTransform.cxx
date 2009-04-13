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

// implementation header
#include "MeshTransform.h"

// system headers
#include <math.h>
#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>

// common headers
#include "Pack.h"


//============================================================================//
//
// Mesh Transform Manager
//

MeshTransformManager TRANSFORMMGR;


MeshTransformManager::MeshTransformManager()
{
  return;
}


MeshTransformManager::~MeshTransformManager()
{
  clear();
  return;
}


void MeshTransformManager::clear()
{
  std::vector<MeshTransform*>::iterator it;
  for (it = transforms.begin(); it != transforms.end(); it++) {
    delete *it;
  }
  transforms.clear();
  return;
}


void MeshTransformManager::update()
{
  return;
}


int MeshTransformManager::addTransform(MeshTransform* transform)
{
  transforms.push_back(transform);
  return ((int)transforms.size() - 1);
}


int MeshTransformManager::findTransform(const std::string& transform) const
{
  if (transform.size() <= 0) {
    return -1;
  } else if ((transform[0] >= '0') && (transform[0] <= '9')) {
    int index = atoi(transform.c_str());
    if ((index < 0) || (index >= (int)transforms.size())) {
      return -1;
    } else {
      return index;
    }
  } else {
    for (int i = 0; i < (int)transforms.size(); i++) {
      if (transforms[i]->getName() == transform) {
	return i;
      }
    }
    return -1;
  }
}


void * MeshTransformManager::pack(void *buf) const
{
  std::vector<MeshTransform*>::const_iterator it;
  buf = nboPackUInt32(buf, (int)transforms.size());
  for (it = transforms.begin(); it != transforms.end(); it++) {
    MeshTransform* transform = *it;
    buf = transform->pack(buf);
  }
  return buf;
}


void * MeshTransformManager::unpack(void *buf)
{
  unsigned int i;
  uint32_t count;
  buf = nboUnpackUInt32(buf, count);
  for (i = 0; i < count; i++) {
    MeshTransform* transform = new MeshTransform;
    buf = transform->unpack(buf);
    addTransform(transform);
  }
  return buf;
}


int MeshTransformManager::packSize() const
{
  int fullSize = sizeof(uint32_t);
  std::vector<MeshTransform*>::const_iterator it;
  for (it = transforms.begin(); it != transforms.end(); it++) {
    MeshTransform* transform = *it;
    fullSize = fullSize + transform->packSize();
  }
  return fullSize;
}


void MeshTransformManager::print(std::ostream& out,
                                 const std::string& indent) const
{
  std::vector<MeshTransform*>::const_iterator it;
  for (it = transforms.begin(); it != transforms.end(); it++) {
    MeshTransform* transform = *it;
    transform->print(out, indent);
  }
  return;
}


//============================================================================//
//
// Mesh Transform Tool
//

static void multiply(fvec4 m[4], const fvec4 n[4])
{
  fvec4 t[4];
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 4; j++) {
      t[i][j] = (m[0][j] * n[i][0]) +
                (m[1][j] * n[i][1]) +
                (m[2][j] * n[i][2]) +
                (m[3][j] * n[i][3]);
    }
  }
  memcpy(m, t, sizeof(fvec4[4]));
  return;
}


static void shift(fvec4 m[4], const fvec3& p)
{
  const fvec4 t[4] = {
    fvec4(1.0f, 0.0f, 0.0f, p.x),
    fvec4(0.0f, 1.0f, 0.0f, p.y),
    fvec4(0.0f, 0.0f, 1.0f, p.z),
    fvec4(0.0f, 0.0f, 0.0f, 1.0f)
  };
  multiply(m, t);
  return;
}


static void scale(fvec4 m[4], const fvec3& p)
{
  const fvec4 t[4] = {
    fvec4(p.x, 0.0f, 0.0f, 0.0f),
    fvec4(0.0f, p.y, 0.0f, 0.0f),
    fvec4(0.0f, 0.0f, p.z, 0.0f),
    fvec4(0.0f, 0.0f, 0.0f, 1.0f)
  };
  multiply(m, t);
  return;
}


static void shear(fvec4 m[4], const fvec3& p)
{
  const fvec4 t[4] = {
    fvec4(1.0f, 0.0f, p.x, 0.0f),
    fvec4(0.0f, 1.0f, p.y, 0.0f),
    fvec4(p.z, 0.0f, 1.0f, 0.0f),
    fvec4(0.0f, 0.0f, 0.0f, 1.0f)
  };
  multiply(m, t);
  return;
}


static void spin(fvec4 m[4], const float radians, const fvec3& normal)
{
  // normalize
  fvec3 n = normal;
  if (!fvec3::normalize(n)) {
    return;
  }

  // setup
  const float cos_val = cosf(radians);
  const float sin_val = sinf(radians);
  const float icos_val = (1.0f - cos_val);
  fvec4 t[4];
  t[3][3] = 1.0f;
  t[0][3] = t[1][3] = t[2][3] = 0.0f;
  t[3][0] = t[3][1] = t[3][2] = 0.0f;
  t[0][0] = (n[0] * n[0] * icos_val) + cos_val;
  t[0][1] = (n[0] * n[1] * icos_val) - (n[2] * sin_val);
  t[0][2] = (n[0] * n[2] * icos_val) + (n[1] * sin_val);
  t[1][0] = (n[1] * n[0] * icos_val) + (n[2] * sin_val);
  t[1][1] = (n[1] * n[1] * icos_val) + cos_val;
  t[1][2] = (n[1] * n[2] * icos_val) - (n[0] * sin_val);
  t[2][0] = (n[2] * n[0] * icos_val) - (n[1] * sin_val);
  t[2][1] = (n[2] * n[1] * icos_val) + (n[0] * sin_val);
  t[2][2] = (n[2] * n[2] * icos_val) + cos_val;

  // execute
  multiply(m, t);

  return;
}


MeshTransform::Tool::Tool(const MeshTransform& xform)
{
  // load the identity matrices
  int i, j;
  for (i = 0; i < 4; i++) {
    for (j = 0; j < 4; j++) {
      if (i == j) {
	vertexMatrix[i][j] = 1.0f;
      } else {
	vertexMatrix[i][j] = 0.0f;
      }
    }
  }
  for (i = 0; i < 3; i++) {
    for (j = 0; j < 3; j++) {
      if (i == j) {
	normalMatrix[i][j] = 1.0f;
      } else {
	normalMatrix[i][j] = 0.0f;
      }
    }
  }

  skewed = false;
  if (xform.transforms.size() > 0) {
    empty = false;
  } else {
    empty = true;
    inverted = false;
    return;
  }

  // setup the matrices
  processTransforms(xform.transforms);

  // generate the normal matrix
  const fvec4* vm = vertexMatrix;
  
  normalMatrix[0] = fvec3::cross(vm[1].xyz(), vm[2].xyz());
  normalMatrix[1] = fvec3::cross(vm[2].xyz(), vm[0].xyz());
  normalMatrix[2] = fvec3::cross(vm[0].xyz(), vm[1].xyz());

  // setup the polarity
  const float determinant =
    (vm[0][0] * ((vm[1][1] * vm[2][2]) - (vm[1][2] * vm[2][1]))) +
    (vm[0][1] * ((vm[1][2] * vm[2][0]) - (vm[1][0] * vm[2][2]))) +
    (vm[0][2] * ((vm[1][0] * vm[2][1]) - (vm[1][1] * vm[2][0])));

  inverted = (determinant < 0.0f);

  return;
}


MeshTransform::Tool::~Tool()
{
  return;
}


void MeshTransform::Tool::processTransforms(
                            const std::vector<TransformData>& transforms)
{
  for (unsigned int i = 0; i < transforms.size(); i++) {
    const TransformData& transform = transforms[i];
    switch (transform.type) {
      case ShiftTransform: {
	shift(vertexMatrix, transform.data.xyz());
	break;
      }
      case ScaleTransform: {
	skewed = true;
	scale(vertexMatrix, transform.data.xyz());
	break;
      }
      case ShearTransform: {
	skewed = true;
	shear(vertexMatrix, transform.data.xyz());
	break;
      }
      case SpinTransform: {
	spin(vertexMatrix, transform.data.w, transform.data.xyz());
	break;
      }
      case IndexTransform: {
	const MeshTransform* xform =
	  TRANSFORMMGR.getTransform(transform.index);
	if (xform != NULL) {
	  processTransforms(xform->transforms);
	}
	break;
      }
      default: {
	printf("MeshTransform::Tool(): unknown type: %i\n",
	       transform.type);
	break;
      }
    }
  }

  return;
}


void MeshTransform::Tool::modifyVertex(fvec3& v) const
{
  if (empty) {
    return;
  }

  fvec3 t;
  const fvec4* vm = vertexMatrix;
  t.x = vm[0].planeDist(v);
  t.y = vm[1].planeDist(v);
  t.z = vm[2].planeDist(v);
  v = t;
}


void MeshTransform::Tool::modifyNormal(fvec3& n) const
{
  if (empty) {
    return;
  }

  fvec3 t;
  const fvec3* nm = normalMatrix;
  t.x = fvec3::dot(n, nm[0]);
  t.y = fvec3::dot(n, nm[1]);
  t.z = fvec3::dot(n, nm[2]);
  n = t;

  // normalize
  if (!fvec3::normalize(n)) {
    n = fvec3(0.0f, 0.0f, 1.0f); // dunno, going with Z...
  }

  if (inverted) {
    n = -n;
  }

  return;
}


void MeshTransform::Tool::modifyOldStyle(fvec3& pos, fvec3& size,
					 float& angle, bool& flipz) const
{
  if (empty) {
    flipz = false;
    return;
  }

  // straight transform
  modifyVertex(pos);

  // transform the object's axis unit vectors
  const float cos_val = cosf(angle);
  const float sin_val = sinf(angle);

  fvec3 x, y, z;
  const fvec4* vm = vertexMatrix;
  const fvec2& xy0 = vm[0].xyz().xy();
  const fvec2& xy1 = vm[1].xyz().xy();
  const fvec2& xy2 = vm[2].xyz().xy();

  // NOTE - the translation (shift) elements are not used
  const fvec2 xUnit(+cos_val, +sin_val);
  x.x = fvec2::dot(xUnit, xy0);
  x.y = fvec2::dot(xUnit, xy1);
  x.z = fvec2::dot(xUnit, xy2);

  const fvec2 yUnit(-sin_val, +cos_val);
  y.x = fvec2::dot(yUnit, xy0);
  y.y = fvec2::dot(yUnit, xy1);
  y.z = fvec2::dot(yUnit, xy2);

  z.x = vm[0].z;
  z.y = vm[1].z;
  z.z = vm[2].z;

  size.x *= x.length();
  size.y *= y.length();
  size.z *= z.length();

  // setup the angle
  angle = atan2f(x.y, x.x);

  // see if the Z axis has flipped
  if (z.z < 0.0f) {
    flipz = true;
    pos.z = pos.z - size.z;
  } else {
    flipz = false;
  }

  return;
}


//============================================================================//
//
// Mesh Transform
//

MeshTransform::MeshTransform()
{
  name = "";
  transforms.clear();

  return;
}


MeshTransform::~MeshTransform()
{
  return;
}


MeshTransform& MeshTransform::operator=(const MeshTransform& old)
{
  name = ""; // not copied
  transforms.clear();
  for (unsigned int i = 0; i < old.transforms.size(); i++) {
    transforms.push_back(old.transforms[i]);
  }
  return *this;
}


void MeshTransform::append(const MeshTransform& xform)
{
  for (unsigned int i = 0; i < xform.transforms.size(); i++) {
    transforms.push_back(xform.transforms[i]);
  }
  return;
}


void MeshTransform::prepend(const MeshTransform& xform)
{
  if (xform.transforms.size() <= 0) {
    return;
  }
  MeshTransform oldCopy = *this;
  transforms.clear();
  unsigned int i;
  for (i = 0; i < xform.transforms.size(); i++) {
    transforms.push_back(xform.transforms[i]);
  }
  for (i = 0; i < oldCopy.transforms.size(); i++) {
    transforms.push_back(oldCopy.transforms[i]);
  }
  return;
}


void MeshTransform::finalize()
{
  return;
}


bool MeshTransform::setName(const std::string& xformname)
{
  if (xformname.size() <= 0) {
    name = "";
    return false;
  } else if ((xformname[0] >= '0') && (xformname[0] <= '9')) {
    name = "";
    return false;
  } else {
    name = xformname;
  }
  return true;
}


const std::string& MeshTransform::getName() const
{
  return name;
}


void MeshTransform::addShift(const fvec3& shift)
{
  TransformData transform;
  transform.data.xyz() = shift;
  transform.data.w = 0.0f;
  transform.type = ShiftTransform;
  transform.index = -1;
  transforms.push_back(transform);
  return;
}


void MeshTransform::addScale(const fvec3& scale)
{
  TransformData transform;
  transform.data.xyz() = scale;
  transform.data.w = 0.0f;
  transform.type = ScaleTransform;
  transform.index = -1;
  transforms.push_back(transform);
  return;
}


void MeshTransform::addShear(const fvec3& shear)
{
  TransformData transform;
  transform.data.xyz() = shear;
  transform.data.w = 0.0f;
  transform.type = ShearTransform;
  transform.index = -1;
  transforms.push_back(transform);
  return;
}


void MeshTransform::addSpin(const float degrees, const fvec3& normal)
{
  const float radians = (float)(degrees * (M_PI / 180.0));
  TransformData transform;
  transform.data.xyz() = normal;
  transform.data.w = radians;
  transform.type = SpinTransform;
  transform.index = -1;
  transforms.push_back(transform);
  return;
}


void MeshTransform::addReference(int index)
{
  TransformData transform;
  transform.type = IndexTransform;
  transform.index = index;
  transforms.push_back(transform);
  return;
}


void * MeshTransform::pack(void *buf) const
{
  buf = nboPackStdString(buf, name);

  buf = nboPackUInt32(buf, (uint32_t)transforms.size());

  for (unsigned int i = 0; i < transforms.size(); i++) {
    const TransformData& transform = transforms[i];
    buf = nboPackUInt8(buf, (uint8_t) transform.type);
    if (transform.type == IndexTransform) {
      buf = nboPackInt32(buf, transform.index);
    } else {
      if (transform.type == SpinTransform) {
        buf = nboPackFVec4(buf, transform.data);
      } else {
        buf = nboPackFVec3(buf, transform.data.xyz());
      }
    }
  }

  return buf;
}


void * MeshTransform::unpack(void *buf)
{
  buf = nboUnpackStdString(buf, name);

  uint32_t count;
  int32_t inTmp;
  buf = nboUnpackUInt32(buf, count);

  for (unsigned int i = 0; i < count; i++) {
    TransformData transform;
    uint8_t type;
    buf = nboUnpackUInt8(buf, type);
    transform.type = (TransformType) type;
    if (transform.type == IndexTransform) {
      buf = nboUnpackInt32(buf, inTmp);
      transform.index = int(inTmp);
      transform.data = fvec4(0.0f, 0.0f, 0.0f, 0.0f);
    }
    else {
      transform.index = -1;
      if (transform.type == SpinTransform) {
	buf = nboUnpackFVec4(buf, transform.data);
      } else {
	buf = nboUnpackFVec3(buf, transform.data.xyz());
	transform.data[3] = 0.0f;
      }
    }
    transforms.push_back(transform);
  }

  finalize();

  return buf;
}


int MeshTransform::packSize() const
{
  int fullSize = nboStdStringPackSize(name);
  fullSize += sizeof(uint32_t);

  for (unsigned int i = 0; i < transforms.size(); i++) {
    const TransformData& transform = transforms[i];
    fullSize += sizeof(uint8_t);
    if (transform.type == IndexTransform) {
      fullSize += sizeof(int32_t);
    } else {
      if (transform.type == SpinTransform) {
        fullSize += sizeof(fvec4);
      } else {
	fullSize += sizeof(fvec3);
      }
    }
  }

  return fullSize;
}


void MeshTransform::print(std::ostream& out, const std::string& indent) const
{
  out << indent << "transform" << std::endl;

  if (name.size() > 0) {
    out << indent << "  name " << name << std::endl;
  }

  printTransforms(out, indent);

  out << indent << "end" << std::endl << std::endl;

  return;
}


void MeshTransform::printTransforms(std::ostream& out,
				    const std::string& indent) const
{
  for (unsigned int i = 0; i < transforms.size(); i++) {
    const TransformData& transform = transforms[i];
    const fvec4& d = transform.data;
    switch (transform.type) {
      case ShiftTransform: {
	out << indent << "  shift "
	    << d[0] << " " << d[1] << " " << d[2] << std::endl;
	break;
      }
      case ScaleTransform: {
	out << indent << "  scale "
	    << d[0] << " " << d[1] << " " << d[2] << std::endl;
	break;
      }
      case ShearTransform: {
	out << indent << "  shear "
	    << d[0] << " " << d[1] << " " << d[2] << std::endl;
	break;
      }
      case SpinTransform: {
	const float degrees = (float)(d[3] * (180.0 / M_PI));
	out << indent << "  spin " << degrees << " "
	    << d[0] << " " << d[1] << " " << d[2] << std::endl;
	break;
      }
      case IndexTransform: {
	const MeshTransform* xform = TRANSFORMMGR.getTransform(transform.index);
	if (xform != NULL) {
	  out << indent << "  xform ";
	  if (xform->getName().size() > 0) {
	    out << xform->getName();
	  } else {
	    out << transform.index;
	  }
	  out << std::endl;
	}
	break;
      }
      default: {
	break;
      }
    }
  }
  return;
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
