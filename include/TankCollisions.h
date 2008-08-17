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

/*
 * TankCollisions:
 *        Interface for testing collisions between shots and tanks.
 */

#ifndef __TANKCOLLISIONS_H__
#define __TANKCOLLISIONS_H__

#include "common.h"

/* system interface headers */
#include <vector>

/* common interface headers */
#include "Flag.h"
#include "Intersect.h"
#include "SegmentedShotStrategy.h"
#include "ShotStrategy.h"
#include "ShotUpdate.h"

class TankCollisions {
 public:
  TankCollisions();
  ~TankCollisions();

  bool test (const ShotCollider& tank, const ShotPath* shotPath) const;
};

#endif // TANKCOLLISIONS_H

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
