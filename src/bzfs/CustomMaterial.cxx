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
#include "CustomMaterial.h"

/* system implementation headers */
#include <sstream>

/* bzfs implementation headers */
#include "ParseMaterial.h"

/* common implementation headers */
#include "BzMaterial.h"


CustomMaterial::CustomMaterial()
{
  return;
}


CustomMaterial::~CustomMaterial()
{
  return;
}


bool CustomMaterial::read(const char *cmd, std::istream& input)
{
  bool materror;

  if (parseMaterials(cmd, input, &material, 1, materror)) {
    if (materror) {
      return false;
    }
  }
  else {
    return WorldFileObject::read(cmd, input);
  }

  return true;
}


void CustomMaterial::write(WorldInfo * /*world*/) const
{
  material.setName(name);
  const BzMaterial* refmat = MATERIALMGR.addMaterial(&material);
  int index = MATERIALMGR.getIndex(refmat);
  if (index < 0) {
    std::cout << "CustomMaterial::write: material didn't register" << std::endl;
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
