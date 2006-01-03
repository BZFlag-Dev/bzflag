/* bzflag
 * Copyright (c) 1993 - 2006 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/**
 * GameTime:
 *	Manages the network time.
 *      Time is stored as microseconds since the epoch.
 */

#ifndef	BZF_GAME_TIME_H
#define	BZF_GAME_TIME_H

#include "common.h"


namespace GameTime {
  void reset();
  void update();

  void setStepTime();
  double getStepTime();

  int packSize();
  void* pack(void *, float lag);
  void* unpack(void *);
  
  const float startRate = 1.0f;
  const float finalRate = 10.0f;
}


#endif // BZF_GAME_TIME_H

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
