/* bzflag
 * Copyright (c) 1993 - 2005 Tim Riker
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


void CustomMaterial::writeToManager() const
{
  material.setName(name);

  if ((name.size() > 0) && (MATERIALMGR.findMaterial(name) != NULL)) {
    std::cout << "warning: duplicate material name"
	      << " (" << name << ")" << std::endl;
    std::cout << "	 the first material will be used" << std::endl;
  }

  const BzMaterial* matref = MATERIALMGR.addMaterial(&material);
  const std::string& refname = matref->getName();

  if ((name.size() > 0) && (refname.size() > 0)) {
    if (name != refname) {
      std::cout << "warning: matching named materials"
		<< " (" << refname << " vs. " << name << ")" << std::endl;
      std::cout << "	 the first name will be used" << std::endl;
    }
  }

  int index = MATERIALMGR.getIndex(matref);
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
