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
#include "CustomWaterLevel.h"

/* bzfs implementation headers */
#include "WorldInfo.h"
#include "ParseMaterial.h"

/* common implementation headers */
#include "MeshMaterial.h"
#include "TextureMatrix.h"


CustomWaterLevel::CustomWaterLevel()
{
  height = -1.0f; // disabled
  material.texture = "water";
  material.textureMatrix = -2; // generate a default later
  material.diffuse[0] = 0.65f;
  material.diffuse[1] = 1.0f;
  material.diffuse[2] = 0.5f;
  material.diffuse[3] = 0.9f;
  // only use the color as a backup
  material.useColorOnTexture = false;
  return;
}


CustomWaterLevel::~CustomWaterLevel()
{
  return;
}


bool CustomWaterLevel::read(const char *cmd, std::istream& input)
{
  bool materror;

  if (strcasecmp ("height", cmd) == 0) {
    input >> height;
  }
  else if (parseMaterial(cmd, input, material, materror)) {
    if (materror) {
      return false;
    }
  }
  else {
    // NOTE: we don't use a WorldFileObstacle
    return WorldFileObject::read(cmd, input);
  }

  return true;
}


void CustomWaterLevel::write(WorldInfo* world) const
{
  world->addWaterLevel(height, material);
  return;
}


// Local variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
