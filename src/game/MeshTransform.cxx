/* bzflag
 * Copyright (c) 1993 - 2008 Tim Riker
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
  transforms.push_back (transform);
  return ((int)transforms.size() - 1);
}


int MeshTransformManager::findTransform(const std::string& transform) const
{
  if (transform.size() <= 0) {
    return -1;
  }
  else if ((transform[0] >= '0') && (transform[0] <= '9')) {
    int index = atoi (transform.c_str());
    if ((index < 0) || (index >= (int)transforms.size())) {
      return -1;
    } else {
      return index;
    }
  }
  else {
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
  buf = nboPackUInt(buf, (int)transforms.size());
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
  buf = nboUnpackUInt (buf, count);
  for (i = 0; i < count; i++) {
    MeshTransform* transform = new MeshTransform;
    buf = transform->unpack(buf);
    addTransform(transform);
  }
  return buf;
}


int MeshTransformManager::packSize() const
{
  int fullSize = sizeof (uint32_t);
  std::vector<MeshTransform*>::const_iterator it;
  for (it = transforms.begin(); it != transforms.end(); it++) {
    MeshTransform* transform = *it;
    fullSize = fullSize + transform->packSize();
  }
  return fullSize;
}


void MeshTransformManager::print(std::ostream& out, const std::string& indent) const
{
  std::vector<MeshTransform*>::const_iterator it;
  for (it = transforms.begin(); it != transforms.end(); it++) {
    MeshTransform* transform = *it;
    transform->print(out, indent);
  }
  return;
}


//
// Mesh Transform Tool
//

static void multiply(float m[4][4], const float n[4][4])
{
  float t[4][4];
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 4; j++) {
      t[i][j] = (m[0][j] * n[i][0]) + (m[1][j] * n[i][1]) +
		(m[2][j] * n[i][2]) + (m[3][j] * n[i][3]);
    }
  }
  memcpy (m, t, sizeof(float[4][4]));
  return;
}


static void shift(float m[4][4], const float p[3])
{
  const float t[4][4] = {{1.0f, 0.0f, 0.0f, p[0]},
			 {0.0f, 1.0f, 0.0f, p[1]},
			 {0.0f, 0.0f, 1.0f, p[2]},
			 {0.0f, 0.0f, 0.0f, 1.0f}};
  multiply(m, t);
  return;
}


static void scale(float m[4][4], const float p[3])
{
  const float t[4][4] = {{p[0], 0.0f, 0.0f, 0.0f},
			 {0.0f, p[1], 0.0f, 0.0f},
			 {0.0f, 0.0f, p[2], 0.0f},
			 {0.0f, 0.0f, 0.0f, 1.0f}};
  multiply(m, t);
  return;
}


static void shear(float m[4][4], const float p[3])
{
  const float t[4][4] = {{1.0f, 0.0f, p[0], 0.0f},
			 {0.0f, 1.0f, p[1], 0.0f},
			 {p[2], 0.0f, 1.0f, 0.0f},
			 {0.0f, 0.0f, 0.0f, 1.0f}};
  multiply(m, t);
  return;
}


static void spin(float m[4][4], const float radians, const float normal[3])
{
  // normalize
  const float len = (normal[0] * normal[0]) +
		    (normal[1] * normal[1]) +
		    (normal[2] * normal[2]);
  if (len <= 0.0f) {
    return;
  }
  const float scale = 1.0f / sqrtf(len);
  const float n[3] = {normal[0] * scale,
		      normal[1] * scale,
		      normal[2] * scale};

  // setup
  const float cos_val = cosf(radians);
  const float sin_val = sinf(radians);
  const float icos_val = (1.0f - cos_val);
  float t[4][4];
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
  const float (*vm)[4] = vertexMatrix;
  normalMatrix[0][0] = (vm[1][1] * vm[2][2]) - (vm[1][2] * vm[2][1]);
  normalMatrix[0][1] = (vm[1][2] * vm[2][0]) - (vm[1][0] * vm[2][2]);
  normalMatrix[0][2] = (vm[1][0] * vm[2][1]) - (vm[1][1] * vm[2][0]);
  normalMatrix[1][0] = (vm[2][1] * vm[0][2]) - (vm[2][2] * vm[0][1]);
  normalMatrix[1][1] = (vm[2][2] * vm[0][0]) - (vm[2][0] * vm[0][2]);
  normalMatrix[1][2] = (vm[2][0] * vm[0][1]) - (vm[2][1] * vm[0][0]);
  normalMatrix[2][0] = (vm[0][1] * vm[1][2]) - (vm[0][2] * vm[1][1]);
  normalMatrix[2][1] = (vm[0][2] * vm[1][0]) - (vm[0][0] * vm[1][2]);
  normalMatrix[2][2] = (vm[0][0] * vm[1][1]) - (vm[0][1] * vm[1][0]);

  // setup the polarity
  const float determinant =
    (vm[0][0] * ((vm[1][1] * vm[2][2]) - (vm[1][2] * vm[2][1]))) +
    (vm[0][1] * ((vm[1][2] * vm[2][0]) - (vm[1][0] * vm[2][2]))) +
    (vm[0][2] * ((vm[1][0] * vm[2][1]) - (vm[1][1] * vm[2][0])));
  if (determinant < 0.0f) {
    inverted = true;
  } else {
    inverted = false;
  }

  // FIXME - remove this check when protocol changes from "0026"
  const float badcheck_2_0_0 = vm[0][0] * vm[1][1] * vm[2][2];
  if ((determinant * badcheck_2_0_0) < 0.0f) {
    printf ("WARNING:  MeshTransform::Tool::Tool()  2.0.0 inversion bug\n");
    printf ("	  The most likely cause is a 'spin' transformation\n");
  }

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
	shift(vertexMatrix, transform.data);
	break;
      }
      case ScaleTransform: {
	skewed = true;
	scale(vertexMatrix, transform.data);
	break;
      }
      case ShearTransform: {
	skewed = true;
	shear(vertexMatrix, transform.data);
	break;
      }
      case SpinTransform: {
	spin(vertexMatrix, transform.data[3], transform.data);
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


void MeshTransform::Tool::modifyVertex(float v[3]) const
{
  if (empty) {
    return;
  }

  float t[3];
  const float (*vm)[4] = vertexMatrix;
  t[0] = (v[0] * vm[0][0]) + (v[1] * vm[0][1]) + (v[2] * vm[0][2]) + vm[0][3];
  t[1] = (v[0] * vm[1][0]) + (v[1] * vm[1][1]) + (v[2] * vm[1][2]) + vm[1][3];
  t[2] = (v[0] * vm[2][0]) + (v[1] * vm[2][1]) + (v[2] * vm[2][2]) + vm[2][3];
  memcpy(v, t, sizeof(float[3]));
}


void MeshTransform::Tool::modifyNormal(float n[3]) const
{
  if (empty) {
    return;
  }

  float t[3];
  const float (*nm)[3] = normalMatrix;
  t[0] = (n[0] * nm[0][0]) + (n[1] * nm[0][1]) + (n[2] * nm[0][2]);
  t[1] = (n[0] * nm[1][0]) + (n[1] * nm[1][1]) + (n[2] * nm[1][2]);
  t[2] = (n[0] * nm[2][0]) + (n[1] * nm[2][1]) + (n[2] * nm[2][2]);
  // normalize
  const float len = (t[0] * t[0]) + (t[1] * t[1]) + (t[2] * t[2]);
  if (len > 0.0f) {
    const float scale = 1.0f / sqrtf(len);
    n[0] = t[0] * scale;
    n[1] = t[1] * scale;
    n[2] = t[2] * scale;
  } else {
    n[0] = n[1] = 0.0f; // dunno, going with Z...
    n[2] = 1.0f;
  }

  if (inverted) {
    n[0] = -n[0];
    n[1] = -n[1];
    n[2] = -n[2];
  }

  return;
}


void MeshTransform::Tool::modifyOldStyle(float pos[3], float size[3],
					 float& angle, bool& flipz) const
{
  if (empty) {
    flipz = false;
    return;
  }

  // straight transform
  modifyVertex(pos);

  // transform the object's axis unit vectors
  const float cos_val = cosf (angle);
  const float sin_val = sinf (angle);
  float x[3], y[3], z[3];
  const float (*vm)[4] = vertexMatrix;
  // NOTE - the translation (shift) elements are not used
  x[0] = (+cos_val * vm[0][0]) + (+sin_val * vm[0][1]);
  x[1] = (+cos_val * vm[1][0]) + (+sin_val * vm[1][1]);
  x[2] = (+cos_val * vm[2][0]) + (+sin_val * vm[2][1]);
  y[0] = (-sin_val * vm[0][0]) + (+cos_val * vm[0][1]);
  y[1] = (-sin_val * vm[1][0]) + (+cos_val * vm[1][1]);
  y[2] = (-sin_val * vm[2][0]) + (+cos_val * vm[2][1]);
  z[0] = vm[0][2];
  z[1] = vm[1][2];
  z[2] = vm[2][2];
  const float xlen = sqrtf ((x[0] * x[0]) + (x[1] * x[1]) + (x[2] * x[2]));
  const float ylen = sqrtf ((y[0] * y[0]) + (y[1] * y[1]) + (y[2] * y[2]));
  const float zlen = sqrtf ((z[0] * z[0]) + (z[1] * z[1]) + (z[2] * z[2]));
  size[0] *= xlen;
  size[1] *= ylen;
  size[2] *= zlen;

  // setup the angle
  angle = atan2f (x[1], x[0]);

  // see if the Z axis has flipped
  if (z[2] < 0.0f) {
    flipz = true;
    pos[2] = pos[2] - size[2];
  } else {
    flipz = false;
  }

  return;
}


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
  }
  else if ((xformname[0] >= '0') && (xformname[0] <= '9')) {
    name = "";
    return false;
  }
  else {
    name = xformname;
  }
  return true;
}


const std::string& MeshTransform::getName() const
{
  return name;
}


void MeshTransform::addShift(const float shift[3])
{
  TransformData transform;
  memcpy(transform.data, shift, sizeof(float[3]));
  transform.data[3] = 0.0f;
  transform.type = ShiftTransform;
  transform.index = -1;
  transforms.push_back(transform);
  return;
}


void MeshTransform::addScale(const float scale[3])
{
  TransformData transform;
  memcpy(transform.data, scale, sizeof(float[3]));
  transform.data[3] = 0.0f;
  transform.type = ScaleTransform;
  transform.index = -1;
  transforms.push_back(transform);
  return;
}


void MeshTransform::addShear(const float shear[3])
{
  TransformData transform;
  memcpy(transform.data, shear, sizeof(float[3]));
  transform.data[3] = 0.0f;
  transform.type = ShearTransform;
  transform.index = -1;
  transforms.push_back(transform);
  return;
}


void MeshTransform::addSpin(const float degrees, const float normal[3])
{
  const float radians = (float)(degrees * (M_PI / 180.0));
  TransformData transform;
  memcpy(transform.data, normal, sizeof(float[3]));
  transform.data[3] = radians;
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

  buf = nboPackUInt(buf, (uint32_t)transforms.size());

  for (unsigned int i = 0; i < transforms.size(); i++) {
    const TransformData& transform = transforms[i];
    buf = nboPackUByte (buf, (uint8_t) transform.type);
    if (transform.type == IndexTransform) {
      buf = nboPackInt (buf, transform.index);
    } else {
      buf = nboPackVector (buf, transform.data);
      if (transform.type == SpinTransform) {
	buf = nboPackFloat (buf, transform.data[3]);
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
  buf = nboUnpackUInt(buf, count);

  for (unsigned int i = 0; i < count; i++) {
    TransformData transform;
    uint8_t type;
    buf = nboUnpackUByte (buf, type);
    transform.type = (TransformType) type;
    if (transform.type == IndexTransform) {
      buf = nboUnpackInt (buf, inTmp);
      transform.index = int(inTmp);
      float* d = transform.data;
      d[0] = d[1] = d[2] = d[3] = 0.0f;
    } else {
      transform.index = -1;
      buf = nboUnpackVector (buf, transform.data);
      if (transform.type == SpinTransform) {
	buf = nboUnpackFloat (buf, transform.data[3]);
      } else {
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
      fullSize += sizeof(float[3]);
      if (transform.type == SpinTransform) {
	fullSize += sizeof(float);
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
    const float* d = transform.data;
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
