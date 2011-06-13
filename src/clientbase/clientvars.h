/* bzflag
 * Copyright (c) 1993-2010 Tim Riker
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
 * vars scope limited to the playing client parts
 */

#ifndef BZF_CLIENTVARS_H
#define BZF_CLIENTVARS_H

#include "common.h"

// common client headers
#include "FlashClock.h"
#include "LocalPlayer.h"
#include "Region.h"
#include "World.h"
#include "game/Team.h"


extern LocalPlayer* myTank;
extern World* world;
extern Team* teams;

#ifdef ROBOT
extern std::vector<BzfRegion*> obstacleList;  // for robots
#endif


#endif // BZF_CLIENTVARS_H

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=2 tabstop=8 expandtab

