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

#include "common.h"

/* interface header */
#include "CustomTextureMatrix.h"

/* system implementation headers */
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
  if (strcasecmp ("shift", cmd) == 0) {
    float uFreq, vFreq;
    input >> uFreq >> vFreq;
    texmat->setShiftParams (uFreq, vFreq);
  }
  else if (strcasecmp ("rotate", cmd) == 0) {
    float freq, uCenter, vCenter;
    input >> freq >> uCenter >> vCenter;
    texmat->setRotateParams (freq, uCenter, vCenter);
  }
  else if (strcasecmp ("scale", cmd) == 0) {
    float uFreq, vFreq, uCenter, vCenter, uScale, vScale;
    input >> uFreq >> vFreq >> uCenter >> vCenter >> uScale >> vScale;
    texmat->setScaleParams (uFreq, vFreq, uCenter, vCenter, uScale, vScale);
  }
  else {
    // NOTE: we don't use a WorldFileObstacle
    return WorldFileObject::read(cmd, input);
  }

  return true;
}


void CustomTextureMatrix::write(WorldInfo */*world*/) const
{
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
