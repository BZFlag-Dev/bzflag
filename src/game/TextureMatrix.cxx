/* bzflag
 * Copyright (c) 1993 - 2007 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* interface header */
#include "TextureMatrix.h"

/* system implementation headers */
#include <math.h>
#include <string.h>

/* common implemnetation headers */
#include "GameTime.h"
#include "Pack.h"


//
// Texture Matrix Manager
//

TextureMatrixManager TEXMATRIXMGR;


TextureMatrixManager::TextureMatrixManager()
{
  return;
}


TextureMatrixManager::~TextureMatrixManager()
{
  clear();
  return;
}


void TextureMatrixManager::clear()
{
  std::vector<TextureMatrix*>::iterator it;
  for (it = matrices.begin(); it != matrices.end(); it++) {
    delete *it;
  }
  matrices.clear();
  return;
}


void TextureMatrixManager::update()
{
  const double gameTime = GameTime::getStepTime();
  std::vector<TextureMatrix*>::iterator it;
  for (it = matrices.begin(); it != matrices.end(); it++) {
    TextureMatrix* texmat = *it;
    texmat->update(gameTime);
  }
  return;
}


int TextureMatrixManager::addMatrix(TextureMatrix* texmat)
{
  matrices.push_back (texmat);
  return ((int)matrices.size() - 1);
}


int TextureMatrixManager::findMatrix(const std::string& texmat) const
{
  if (texmat.size() <= 0) {
    return -1;
  }
  else if ((texmat[0] >= '0') && (texmat[0] <= '9')) {
    int index = atoi (texmat.c_str());
    if ((index < 0) || (index >= (int)matrices.size())) {
      return -1;
    } else {
      return index;
    }
  }
  else {
    for (int i = 0; i < (int)matrices.size(); i++) {
      if (matrices[i]->getName() == texmat) {
	return i;
      }
    }
    return -1;
  }
}


const TextureMatrix* TextureMatrixManager::getMatrix(int id) const
{
  if ((id >= 0) && (id < (int)matrices.size())) {
    return matrices[id];
  } else {
    return NULL;
  }
}


void * TextureMatrixManager::pack(void *buf) const
{
  std::vector<TextureMatrix*>::const_iterator it;
  buf = nboPackUInt(buf, (unsigned int)matrices.size());
  for (it = matrices.begin(); it != matrices.end(); it++) {
    TextureMatrix* texmat = *it;
    buf = texmat->pack(buf);
  }
  return buf;
}


void * TextureMatrixManager::unpack(void *buf)
{
  unsigned int i;
  uint32_t count;
  buf = nboUnpackUInt (buf, count);
  for (i = 0; i < count; i++) {
    TextureMatrix* texmat = new TextureMatrix;
    buf = texmat->unpack(buf);
    addMatrix(texmat);
  }
  return buf;
}


int TextureMatrixManager::packSize() const
{
  int fullSize = sizeof (uint32_t);
  std::vector<TextureMatrix*>::const_iterator it;
  for (it = matrices.begin(); it != matrices.end(); it++) {
    TextureMatrix* texmat = *it;
    fullSize = fullSize + texmat->packSize();
  }
  return fullSize;
}


void TextureMatrixManager::print(std::ostream& out,
				 const std::string& indent) const
{
  std::vector<TextureMatrix*>::const_iterator it;
  for (it = matrices.begin(); it != matrices.end(); it++) {
    TextureMatrix* texmat = *it;
    texmat->print(out, indent);
  }
  return;
}


//
// Texture Matrix
//

static const float fullIdentity[4][4] = {
  { 1.0f, 0.0f, 0.0f, 0.0f },
  { 0.0f, 1.0f, 0.0f, 0.0f },
  { 0.0f, 0.0f, 1.0f, 0.0f },
  { 0.0f, 0.0f, 0.0f, 1.0f }
};

static const float partialIdentity[3][2] = {
  { 1.0f, 0.0f },
  { 0.0f, 1.0f },
  { 0.0f, 0.0f }  // translation elements
};


static void makeFullMatrix(float f[4][4], const float p[3][2])
{
  // we assume that the other elements have been initialized
  f[0][0] = p[0][0];
  f[0][1] = p[0][1];
  f[1][0] = p[1][0];
  f[1][1] = p[1][1];
  f[3][0] = p[2][0];
  f[3][1] = p[2][1];
  return;
}


static void multiply(float m[3][2], const float n[3][2])
{
  float t[3][2];
  t[0][0] = (m[0][0] * n[0][0]) + (m[0][1] * n[1][0]);
  t[0][1] = (m[0][0] * n[0][1]) + (m[0][1] * n[1][1]);
  t[1][0] = (m[1][0] * n[0][0]) + (m[1][1] * n[1][0]);
  t[1][1] = (m[1][0] * n[0][1]) + (m[1][1] * n[1][1]);
  t[2][0] = (m[2][0] * n[0][0]) + (m[2][1] * n[1][0]) + n[2][0];
  t[2][1] = (m[2][0] * n[0][1]) + (m[2][1] * n[1][1]) + n[2][1];
  memcpy(m, t, sizeof(float[3][2]));
  return;
}


static void shift(float m[3][2], float ushf, float vshf)
{
  const float t[3][2] = {{1.0f, 0.0f},
			 {0.0f, 1.0f},
			 {ushf, vshf}};
  multiply(m, t);
  return;
}


static void scale(float m[3][2], float uscl, float vscl)
{
  const float t[3][2] = {{uscl, 0.0f},
			 {0.0f, vscl},
			 {0.0f, 0.0f}};
  multiply(m, t);
  return;
}


static void spin(float m[3][2], float radians)
{
  const float crd = cosf(radians);
  const float srd = sinf(radians);
  const float t[3][2] = {{+crd, +srd},
			 {-srd, +crd},
			 {0.0f, 0.0f}};
  multiply(m, t);
  return;
}


TextureMatrix::TextureMatrix()
{
  name = "";

  // load the identity matrices
  memcpy(matrix, fullIdentity, sizeof(float[4][4]));
  memcpy(staticMatrix, partialIdentity, sizeof(float[3][2]));

  // the static parameters
  useStatic = false;
  rotation = 0.0f;
  uFixedShift = vFixedShift = 0.0f;
  uFixedScale = vFixedScale = 1.0f;
  uFixedCenter = vFixedCenter = 0.5f;

  // the dynamic parameters
  useDynamic = false;
  spinFreq = 0.0f;
  uShiftFreq = vShiftFreq = 0.0f;
  uScaleFreq = vScaleFreq = 0.0f;
  uScale = vScale = 1.0f;
  uCenter = vCenter = 0.5f;

  return;
}


TextureMatrix::~TextureMatrix()
{
  return;
}


void TextureMatrix::finalize()
{
  useStatic = false;
  useDynamic = false;

  if ((rotation != 0.0f) ||
      (uFixedShift != 0.0f) || (vFixedShift != 0.0f) ||
      (uFixedScale != 1.0f) || (vFixedScale != 1.0f)) {
    useStatic = true;
  }

  if ((spinFreq != 0.0f) ||
      (uShiftFreq != 0.0f) || (vShiftFreq != 0.0f) ||
      (uScaleFreq != 0.0f) || (vScaleFreq != 0.0f)) {
    useDynamic = true;
  }

  if (useStatic) {
    // setup the staticMatrix
    const float radians = rotation * (float)(M_PI / 180.0);

    shift(staticMatrix, -(uFixedShift + uFixedCenter),
			-(vFixedShift + vFixedCenter));
    spin(staticMatrix, -radians);
    if ((uFixedScale != 0.0f) && (vFixedScale != 0.0f)) {
      scale(staticMatrix, (1.0f / uFixedScale), (1.0f / vFixedScale));
    }
    shift(staticMatrix, +uFixedCenter, +vFixedCenter);

    if (!useDynamic) {
      // setup the matrix and don't touch it during updates
      makeFullMatrix(matrix, staticMatrix); // convert to 4x4
    }
  }

  return;
}


bool TextureMatrix::setName(const std::string& texmat)
{
  if (texmat.size() <= 0) {
    name = "";
    return false;
  }
  else if ((texmat[0] >= '0') && (texmat[0] <= '9')) {
    name = "";
    return false;
  }
  else {
    name = texmat;
  }
  return true;
}


const std::string& TextureMatrix::getName() const
{
  return name;
}


void TextureMatrix::setStaticShift (float u, float v)
{
  uFixedShift = u;
  vFixedShift = v;
  return;
}


void TextureMatrix::setStaticSpin (float angle)
{
  rotation = angle;
  return;
}


void TextureMatrix::setStaticScale (float u, float v)
{
  if (u != 0.0f) {
    uFixedScale = u;
  }
  if (v != 0.0f) {
    vFixedScale = v;
  }
  return;
}


void TextureMatrix::setStaticCenter (float u, float v)
{
  uFixedCenter = u;
  vFixedCenter = v;
  return;
}


void TextureMatrix::setDynamicShift (float uFreq, float vFreq)
{
  uShiftFreq = uFreq;
  vShiftFreq = vFreq;
  return;
}


void TextureMatrix::setDynamicSpin (float freq)
{
  spinFreq = freq;
  return;
}


void TextureMatrix::setDynamicScale (float uFreq, float vFreq,
				    float _uScale, float _vScale)
{
  uScaleFreq = uFreq;
  vScaleFreq = vFreq;
  if (_uScale >= 1.0f) {
    uScale = _uScale;
  }
  if (_vScale >= 1.0f) {
    vScale = _vScale;
  }
  return;
}


void TextureMatrix::setDynamicCenter (float u, float v)
{
  uCenter = u;
  vCenter = v;
  return;
}


void TextureMatrix::update (double t)
{
  if (!useDynamic) {
    // the matrix has already been setup with the
    // static tranformations, or an identity matrix.
    return;
  }

  // the matrix reloaded
//  memcpy(matrix, identityMatrix, sizeof(float[4][4]));

  float partial[3][2];
  memcpy(partial, partialIdentity, sizeof(float[3][2]));

  // the spin params
  const float radians = (float)(fmod(t * (double)spinFreq, 1.0) * (M_PI * 2.0));
  // the scale params
  const float urad = (float)(fmod(t * (double)uScaleFreq, 1.0) * (M_PI * 2.0));
  const float vrad = (float)(fmod(t * (double)vScaleFreq, 1.0) * (M_PI * 2.0));
  const float uratio = 0.5f + (0.5f * cosf(urad));
  const float vratio = 0.5f + (0.5f * sinf(vrad));
  const float uscl = 1.0f + (uratio * (uScale - 1.0f));
  const float vscl = 1.0f + (vratio * (vScale - 1.0f));
  // the shift params
  const float ushf = (float)fmod(t * (double)uShiftFreq, 1.0);
  const float vshf = (float)fmod(t * (double)vShiftFreq, 1.0);

  shift(partial, -(ushf + uCenter), -(vshf + vCenter));
  spin(partial, -radians);
  scale(partial, (1.0f / uscl), (1.0f / vscl));
  shift(partial, +uCenter, +vCenter);

  if (useStatic) {
    multiply(partial, staticMatrix);
  }

  makeFullMatrix(matrix, partial);

  return;
}


void * TextureMatrix::pack(void *buf) const
{
  buf = nboPackStdString (buf, name);

  uint8_t state = 0;
  if (useStatic)  state |= (1 << 0);
  if (useDynamic) state |= (1 << 1);
  buf = nboPackUByte (buf, state);

  if (useStatic) {
    buf = nboPackFloat (buf, rotation);
    buf = nboPackFloat (buf, uFixedShift);
    buf = nboPackFloat (buf, vFixedShift);
    buf = nboPackFloat (buf, uFixedScale);
    buf = nboPackFloat (buf, vFixedScale);
    buf = nboPackFloat (buf, uFixedCenter);
    buf = nboPackFloat (buf, vFixedCenter);
  }

  if (useDynamic) {
    buf = nboPackFloat (buf, spinFreq);
    buf = nboPackFloat (buf, uShiftFreq);
    buf = nboPackFloat (buf, vShiftFreq);
    buf = nboPackFloat (buf, uScaleFreq);
    buf = nboPackFloat (buf, vScaleFreq);
    buf = nboPackFloat (buf, uScale);
    buf = nboPackFloat (buf, vScale);
    buf = nboPackFloat (buf, uCenter);
    buf = nboPackFloat (buf, vCenter);
  }

  return buf;
}


void * TextureMatrix::unpack(void *buf)
{
  buf = nboUnpackStdString (buf, name);

  uint8_t state;
  buf = nboUnpackUByte (buf, state);
  useStatic =  (state & (1 << 0)) != 0;
  useDynamic = (state & (1 << 1)) != 0;

  if (useStatic) {
    buf = nboUnpackFloat (buf, rotation);
    buf = nboUnpackFloat (buf, uFixedShift);
    buf = nboUnpackFloat (buf, vFixedShift);
    buf = nboUnpackFloat (buf, uFixedScale);
    buf = nboUnpackFloat (buf, vFixedScale);
    buf = nboUnpackFloat (buf, uFixedCenter);
    buf = nboUnpackFloat (buf, vFixedCenter);
  }

  if (useDynamic) {
    buf = nboUnpackFloat (buf, spinFreq);
    buf = nboUnpackFloat (buf, uShiftFreq);
    buf = nboUnpackFloat (buf, vShiftFreq);
    buf = nboUnpackFloat (buf, uScaleFreq);
    buf = nboUnpackFloat (buf, vScaleFreq);
    buf = nboUnpackFloat (buf, uScale);
    buf = nboUnpackFloat (buf, vScale);
    buf = nboUnpackFloat (buf, uCenter);
    buf = nboUnpackFloat (buf, vCenter);
  }

  finalize();

  return buf;
}


int TextureMatrix::packSize() const
{
  int fullSize = 0;
  fullSize += nboStdStringPackSize(name);
  fullSize += sizeof(uint8_t);
  if (useStatic) {
    fullSize += sizeof(float[7]);
  }
  if (useDynamic) {
    fullSize += sizeof(float[9]);
  }
  return fullSize;
}


void TextureMatrix::print(std::ostream& out, const std::string& indent) const
{
  out << indent << "textureMatrix" << std::endl;

  if (name.size() > 0) {
    out << indent << "  name " << name << std::endl;
  }

  if (useStatic) {
    if (rotation != 0.0f) {
      out << indent << "  fixedspin " << rotation << std::endl;
    }
    if ((uFixedShift != 0.0f) || (vFixedShift != 0.0f)) {
      out << indent << "  fixedshift " << uFixedShift << " " << vFixedShift << std::endl;
    }
    if ((uFixedScale != 1.0f) || (vFixedScale != 1.0f)) {
      out << indent << "  fixedscale " << uFixedScale << " " << vFixedScale << std::endl;
    }
    if ((uFixedCenter != 0.5f) || (vFixedCenter != 0.5f)) {
      out << indent << "  fixedcenter " << uFixedCenter << " " << vFixedCenter << std::endl;
    }
  }

  if (useDynamic) {
    if (spinFreq != 0.0f) {
      out << indent << "  spin " << spinFreq << std::endl;
    }
    if ((uShiftFreq != 0.0f) || (vShiftFreq != 0.0f)) {
      out << indent << "  shift " << uShiftFreq << " " << vShiftFreq << std::endl;
    }
    if ((uScaleFreq != 0.0f) || (vScaleFreq != 0.0f) ||
	(uScale != 1.0f) || (vScale != 1.0f)) {
      out << indent << "  scale " << uScaleFreq << " " << vScaleFreq << " "
			<< uScale << " " << vScale << std::endl;
    }
    if ((uCenter != 0.5f) || (uCenter != 0.5f)) {
      out << indent << "  center " << uCenter << " " << vCenter << std::endl;
    }
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
