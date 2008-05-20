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

/* system headers */
#include <string>
#include <string.h>

/* interface header */
#include "CustomWaterLevel.h"

/* bzfs implementation headers */
#include "WorldInfo.h"
#include "ParseMaterial.h"

/* common implementation headers */
#include "BzMaterial.h"
#include "TextureMatrix.h"


CustomWaterLevel::CustomWaterLevel()
{
  modedMaterial = false;
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
    if (!(input >> height)) {
      return false;
    }
  } else if (parseMaterials(cmd, input, &material, 1, materror)) {
    if (materror) {
      return false;
    }
    modedMaterial = true;
  } else {
    // NOTE: we don't use a WorldFileObstacle
    return WorldFileObject::read(cmd, input);
  }

  return true;
}


void CustomWaterLevel::writeToWorld(WorldInfo* world) const
{
  if (modedMaterial) {
    const BzMaterial* matref = MATERIALMGR.addMaterial(&material);
    world->addWaterLevel(height, matref);
  } else {
    world->addWaterLevel(height, NULL); // build the material later
  }

  return;
}


// Local variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
