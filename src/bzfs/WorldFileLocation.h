/* bzflag
 * Copyright (c) 1993 - 2009 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */
#ifndef __WORLDFILELOCATION_H__
#define __WORLDFILELOCATION_H__

/* interface header */
#include "WorldFileObject.h"

// system headers
#include <iostream>

// bzfs-specific headers
#include "MeshTransform.h"


class WorldFileLocation : public WorldFileObject {
public:
  WorldFileLocation();
  virtual bool read(const char *cmd, std::istream&);
  virtual void writeToWorld(WorldInfo*) const;
  void *pack(void *buf) const;
protected:
  fvec3 pos;
  fvec3 size;
  float rotation;
  MeshTransform transform;
};

inline void WorldFileLocation::writeToWorld(WorldInfo*) const {}

#endif /* __WORLDFILELOCATION_H__ */

// Local variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
