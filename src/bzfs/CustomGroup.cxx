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

/* interface header */
#include "CustomGroup.h"

/* system headers */
#include <sstream>
#include <string>

/* common headers */
#include "ObstacleMgr.h"


CustomGroup::CustomGroup(const std::string& _groupdef)
{
  groupdef = _groupdef;
  return;
}


bool CustomGroup::read(const char *cmd, std::istream& input) {
  if (!WorldFileLocation::read(cmd, input)) {
    return false;
  }
  return true;
}


void CustomGroup::writeToGroupDef(GroupDefinition *grpdef) const
{
  // include the old style parameters
  MeshTransform xform;
  if ((size[0] != 1.0f) || (size[1] != 1.0f) || (size[2] != 1.0f)) {
    xform.addScale(size);
  }
  if (rotation != 0.0f) {
    const float zAxis[3] = {0.0f, 0.0f, 1.0f};
    xform.addSpin(rotation * (180.0f / M_PI), zAxis);
  }  
  if ((pos[0] != 0.0f) || (pos[1] != 0.0f) || (pos[2] != 0.0f)) {
    xform.addShift(pos);
  }
  xform.append(transform);
  
  // make the group instance
  if (groupdef.size() > 0) {
    GroupInstance* instance = new GroupInstance(groupdef, xform);
    grpdef->addGroupInstance(instance);
  } else {
    std::cout << "warning: group instance has no group definition" << std::endl;
  }
  
  return;
}


// Local variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
