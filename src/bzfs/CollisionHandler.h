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

#ifndef _COLLISIONHANDLER_H_
#define _COLLISIONHANDLER_H_

/* system interface headers */
#include <string>
#include <cstring>
#include <vector>
#include <cstdlib>

/* common interface headers */
#include "bzfsAPI.h"
#include "TankCollisions.h"
#include "GameKeeper.h"

class CollisionHandler : public bz_EventHandler
{

  float lastUpdate;
  TankCollisions* colDet;

public:
  CollisionHandler();
  virtual void process(bz_EventData* data);
};

#endif // _COLLISIONHANDLER_H_

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
