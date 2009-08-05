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

#ifndef __TANK_H__
#define __TANK_H__

#include "common.h"
#include "vectors.h"

/* system interface headers */
#include <string>

class Tank {
public:
  Tank();
  Tank(
    std::string ncallsign,
    std::string nteam,
    std::string nflag,
    bool npaused,
    bool nalive,
    bool nfrozen,
    bool nsuper,
    fvec3 nposition,
    double nangle,
    fvec3 nvelocity,
    double nangularVelocity
  );

  std::string callsign, team, flag;
  bool paused, alive, frozen, super;

  double position[3], angle;
  double velocity[3], angularVelocity;
};

#endif /* __TANK_H__ */

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
