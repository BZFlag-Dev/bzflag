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
#ifndef __WORLDFILEOBSTACLE_H__
#define __WORLDFILEOBSTACLE_H__

// system headers
#include <iostream>

// bzfs-specific headers
#include "WorldFileObject.h"


class WorldFileObstacle : public WorldFileObject {
public:
  WorldFileObstacle();
  virtual bool read(const char *cmd, std::istream&);

protected:
  float pos[3];
  float rotation;
  float size[3];
  bool driveThrough;
  bool shootThrough;
  bool flipZ;
};

#endif /* __WORLDFILEOBSTACLE_H__ */

// Local variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
