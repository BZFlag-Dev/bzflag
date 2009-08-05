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

/* interface header */
#include "Tank.h"

Tank::Tank() {}

Tank::Tank(
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
	) :
	callsign(ncallsign),
	team(nteam),
	flag(nflag),
	paused(npaused),
	alive(nalive),
	frozen(nfrozen),
	super(nsuper),
	angle(nangle),
	angularVelocity(nangularVelocity)
{
  position[0] = nposition.x;
  position[1] = nposition.y;
  position[2] = nposition.z;

  velocity[0] = nvelocity.x;
  velocity[1] = nvelocity.y;
  velocity[2] = nvelocity.z;
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
