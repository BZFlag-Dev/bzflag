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

#include "TextureMatrix.h"

#include <math.h>
#include <string.h>
#include <vector>

#include "TimeKeeper.h"
#include "Pack.h"


// NOTE: Here are some extra features that might be worth implementing:
//
// - CLAMPS, CLAMPT: allow texture clamps
// - GL_REPLACE: already implemented in the scene node files
// - glTexGen() modes (sphere, object_linear, eye_linear, reflection)
// - glTexSubImage2D() - would be handy to have this in TextureManager
//


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
  float t = TimeKeeper::getCurrent() - TimeKeeper::getStartTime();
  std::vector<TextureMatrix*>::iterator it;
  for (it = matrices.begin(); it != matrices.end(); it++) {
    TextureMatrix* texmat = *it;
    texmat->update(t);
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


void * TextureMatrixManager::pack(void *buf)
{
  std::vector<TextureMatrix*>::iterator it;
  buf = nboPackUInt(buf, (unsigned int)matrices.size());
  for (it = matrices.begin(); it != matrices.end(); it++) {
    TextureMatrix* texmat = *it;
    buf = texmat->pack(buf);
  }
  return buf;
}


void * TextureMatrixManager::unpack(void *buf)
{
  unsigned int i, count;
  buf = nboUnpackUInt (buf, count);
  for (i = 0; i < count; i++) {
    TextureMatrix* texmat = new TextureMatrix;
    buf = texmat->unpack(buf);
    addMatrix(texmat);
  }
  return buf;
}


int TextureMatrixManager::packSize()
{
  int fullSize = sizeof (uint32_t);
  std::vector<TextureMatrix*>::iterator it;
  for (it = matrices.begin(); it != matrices.end(); it++) {
    TextureMatrix* texmat = *it;
    fullSize = fullSize + texmat->packSize();
  }
  return fullSize;
}


void TextureMatrixManager::print(std::ostream& out, int level)
{
  std::vector<TextureMatrix*>::iterator it;
  for (it = matrices.begin(); it != matrices.end(); it++) {
    TextureMatrix* texmat = *it;
    texmat->print(out, level);
  }
  return;
}


//
// Texture Matrix
//

TextureMatrix::TextureMatrix()
{
  // load an identity matrix
  for (int row = 0; row < 4; row++) {
    for (int col = 0; col < 4; col++) {
      if (row == col) {
	matrix[(col*4) + row] = 1.0f;
	  } else {
	    matrix[(col*4) + row] = 0.0f;
	  }
	}
  }

  // clear the parameters
  uShiftFreq = vShiftFreq = 0.0f;
  rotateFreq = 0.0f;
  uRotateCenter = vRotateCenter = 0.0f;
  uScaleFreq = vScaleFreq = 0.0f;
  uScale = vScale = 1.0f;
  uScaleCenter = vScaleCenter = 0.0f;

  name = "";
}


TextureMatrix::~TextureMatrix()
{
}


const float* TextureMatrix::getMatrix() const
{
  return matrix;
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


void TextureMatrix::setShiftParams (float uFreq, float vFreq)
{
  uShiftFreq = uFreq;
  vShiftFreq = vFreq;
  return;
}


void TextureMatrix::setRotateParams (float freq, float uCenter, float vCenter)
{
  rotateFreq = freq;
  uRotateCenter = uCenter;
  vRotateCenter = vCenter;
  return;
}


void TextureMatrix::setScaleParams (float uFreq, float vFreq,
				    float uCenter, float vCenter,
				    float _uScale, float _vScale)
{
  uScaleFreq = uFreq;
  vScaleFreq = vFreq;
  uScale = _uScale;
  vScale = _vScale;
  uScaleCenter = uCenter;
  vScaleCenter = vCenter;
  return;
}


void TextureMatrix::update (float t)
{
  // FIXME - implement rotation and scaling
  float angle = fmodf(t * rotateFreq, 1.0f) * (M_PI * 2.0f);
  float c = cosf (-angle);
  float s = sinf (-angle);
  float tu = fmodf(t * uShiftFreq, 1.0f);
  float tv = fmodf(t * vShiftFreq, 1.0f);
  //float tu = t * uShiftFreq;
  //float tv = t * vShiftFreq;
  float ru = -uRotateCenter;
  float rv = -vRotateCenter;
  matrix[(0*4) + 0] = c;
  matrix[(0*4) + 1] = s;
  matrix[(1*4) + 0] = -s;
  matrix[(1*4) + 1] = c;

//  matrix[(3*4) + 0] = (c * ru) - (s * rv) - ru - tu;
//  matrix[(3*4) + 1] = (s * ru) + (c * rv) - rv - tv;
  matrix[(3*4) + 0] = (c * (ru - tu)) - (s * (rv - tv)) - ru;
  matrix[(3*4) + 1] = (s * (ru - tu)) + (c * (rv - tv)) - rv;

/*
  matrix[(3*4) + 0] -= fmodf(t * uShiftFreq, 1.0f);
  matrix[(3*4) + 1] -= fmodf(t * vShiftFreq, 1.0f);


  float uScale = 1.0f;
  if (uScaleFreq != 0.0f) {
    uScale = fmodf(t * uScaleFreq, 1.0f);
    uScale = (1.5f - (0.5f * cosf ((M_PI * 2.0f) * uScale))) / uScale;
  }

  float vScale = 1.0f;
  if (vScaleFreq != 0.0f) {
    vScale = fmodf(t * vScaleFreq, 1.0f);
    vScale = (1.5f - (0.5f * cosf ((M_PI * 2.0f) * vScale))) / vScale;
  }

  float su = uScaleCenter;
  float sv = vScaleCenter;

  matrix[(0*4) + 0] = uScale;
  matrix[(1*4) + 1] = vScale;
  matrix[(3*4) + 0] = (uScale * (uScaleCenter - uShift)) - uScaleCenter;
  matrix[(3*4) + 1] = (vScale * (vScaleCenter - vShift)) - vScaleCenter;
*/
  return;
}


void * TextureMatrix::pack(void *buf)
{
  buf = nboPackStdString (buf, name);

  buf = nboPackFloat (buf, uShiftFreq);
  buf = nboPackFloat (buf, vShiftFreq);

  buf = nboPackFloat (buf, rotateFreq);
  buf = nboPackFloat (buf, uRotateCenter);
  buf = nboPackFloat (buf, vRotateCenter);

  buf = nboPackFloat (buf, uScaleFreq);
  buf = nboPackFloat (buf, vScaleFreq);
  buf = nboPackFloat (buf, uScale);
  buf = nboPackFloat (buf, vScale);
  buf = nboPackFloat (buf, uScaleCenter);
  buf = nboPackFloat (buf, vScaleCenter);

  return buf;
}


void * TextureMatrix::unpack(void *buf)
{
  buf = nboUnpackStdString (buf, name);

  buf = nboUnpackFloat (buf, uShiftFreq);
  buf = nboUnpackFloat (buf, vShiftFreq);

  buf = nboUnpackFloat (buf, rotateFreq);
  buf = nboUnpackFloat (buf, uRotateCenter);
  buf = nboUnpackFloat (buf, vRotateCenter);

  buf = nboUnpackFloat (buf, uScaleFreq);
  buf = nboUnpackFloat (buf, vScaleFreq);
  buf = nboUnpackFloat (buf, uScale);
  buf = nboUnpackFloat (buf, vScale);
  buf = nboUnpackFloat (buf, uScaleCenter);
  buf = nboUnpackFloat (buf, vScaleCenter);

  return buf;
}


int TextureMatrix::packSize()
{
  return (nboStdStringPackSize(name) + sizeof(float[11]));
}


void TextureMatrix::print(std::ostream& out, int /*level*/)
{
  out << "textureMatrix" << std::endl;

  if (name.size() > 0) {
    out << "  name " << name << std::endl;
  }

  if ((uShiftFreq != 0.0f) || (vShiftFreq != 0.0f)) {
    out << "  shift " << uShiftFreq << " " << vShiftFreq << std::endl;
  }
  if ((rotateFreq != 0.0f) ||
      (uRotateCenter != 0.0f) || (vRotateCenter != 0.0f)) {
    out << "  rotate " << rotateFreq << " "
		      << uRotateCenter << " " << vRotateCenter << std::endl;
  }
  if ((uScaleFreq != 0.0f) || (vScaleFreq != 0.0f) ||
      (uScale != 1.0f) || (vScale != 1.0f) ||
      (uScaleCenter != 0.0f) || (vScaleCenter != 0.0f)) {
    out << "  scale " << uScaleFreq << " " << vScaleFreq << " "
		      << uScale << " " << vScale << " "
		      << uScaleCenter << " " << vScaleCenter << std::endl;
  }

  out << "end" << std::endl << std::endl;

  return;
}


// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

