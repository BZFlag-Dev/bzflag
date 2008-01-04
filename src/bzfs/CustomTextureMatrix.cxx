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

/* interface header */
#include "CustomTextureMatrix.h"

/* system implementation headers */
#include <iostream>
#include <math.h>

/* common implementation headers */
#include "TextureMatrix.h"


CustomTextureMatrix::CustomTextureMatrix()
{
  texmat = new TextureMatrix;
  return;
}


CustomTextureMatrix::~CustomTextureMatrix()
{
  delete texmat;
  return;
}


bool CustomTextureMatrix::read(const char *cmd, std::istream& input)
{
  if (strcasecmp ("fixedshift", cmd) == 0) {
    float u, v;
    if (!(input >> u >> v)) {
      return false;
    }
    texmat->setStaticShift (u, v);
  }
  else if (strcasecmp ("fixedscale", cmd) == 0) {
    float u, v;
    if (!(input >> u >> v)) {
      return false;
    }
    texmat->setStaticScale (u, v);
  }
  else if (strcasecmp ("fixedspin", cmd) == 0) {
    float angle;
    if (!(input >> angle)) {
      return false;
    }
    texmat->setStaticSpin (angle);
  }
  else if (strcasecmp ("fixedcenter", cmd) == 0) {
    float u, v;
    if (!(input >> u >> v)) {
      return false;
    }
    texmat->setStaticCenter (u, v);
  }
  else if (strcasecmp ("shift", cmd) == 0) {
    float uFreq, vFreq;
    if (!(input >> uFreq >> vFreq)) {
      return false;
    }
    texmat->setDynamicShift (uFreq, vFreq);
  }
  else if (strcasecmp ("spin", cmd) == 0) {
    float freq;
    if (!(input >> freq)) {
      return false;
    }
    texmat->setDynamicSpin (freq);
  }
  else if (strcasecmp ("scale", cmd) == 0) {
    float uFreq, vFreq, uScale, vScale;
    if (!(input >> uFreq >> vFreq >> uScale >> vScale)) {
      return false;
    }
    texmat->setDynamicScale (uFreq, vFreq, uScale, vScale);
  }
  else if (strcasecmp ("center", cmd) == 0) {
    float u, v;
    if (!(input >> u >> v)) {
      return false;
    }
    texmat->setDynamicCenter (u, v);
  }
  else {
    // NOTE: we don't use a WorldFileObstacle
    return WorldFileObject::read(cmd, input);
  }

  return true;
}


void CustomTextureMatrix::writeToManager() const
{
  texmat->setName(name);
  if ((name.size() > 0) && (TEXMATRIXMGR.findMatrix(name) >= 0)) {
    std::cout << "warning: duplicate texture matrix"
	      << " (" << name << ")" << std::endl;
  }
  texmat->finalize();
  TEXMATRIXMGR.addMatrix (texmat);
  texmat = NULL;
  return;
}


// Local variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
