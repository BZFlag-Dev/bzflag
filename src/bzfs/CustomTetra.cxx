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
#include "CustomTetra.h"

/* system implementation headers */
#include <math.h>

/* common implementation headers */
#include "StateDatabase.h"


CustomTetra::CustomTetra()
{
  vertexCount = 0; // no vertices have yet been defined
  
  // make all of the planes visible
  for (int i = 0; i < 4; i++) {
    visible[i] = true;
  }
  
  // FIXME - can't use WorldFileObstable as the base
  //         class because of the position, size, rotation, protperties
  driveThrough = false; //FIXME
  shootThrough = false; //FIXME
}

bool CustomTetra::read(const char *cmd, std::istream& input)
{
  if (strcasecmp(cmd, "vertex") == 0) {
    if (vertexCount >= 4) {
      std::cout << "Extra tetrahedron vertex" << std::endl;
      return true; // keep on chugging
    }
    float* vertex = vertices[vertexCount];
    input >> vertex[0] >> vertex[1] >> vertex[2];
    
    vertexCount++;
  }
  else if (strcasecmp(cmd, "visible") == 0) {
    input >> visible[0] >> visible[1] >> visible[2] >> visible[3];
  }
  else if (strcasecmp(cmd, "drivethrough") == 0) {
    driveThrough = true;
  }
  else if (strcasecmp(cmd, "shootthrough") == 0) {
    shootThrough = true;
  }
  else if (strcasecmp(cmd, "passable") == 0) {
    driveThrough = shootThrough = true;
  }
  else {
    // NOTE: we don't use a WorldFileObstacle
    return WorldFileObject::read(cmd, input);
  }

  return true;
}


void CustomTetra::write(WorldInfo *world) const
{
  if (vertexCount < 4) {
    std::cout << "Not creating tetrahedron, not enough vertices ("
              << vertexCount << ")" << std::endl;
    return;
  }
  
  world->addTetra(vertices, visible, driveThrough, shootThrough);
}

// Local variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
