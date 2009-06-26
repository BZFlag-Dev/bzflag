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

#ifndef __BACKENDSHOT_H__
#define __BACKENDSHOT_H__

#include "common.h"

/* system interface headers */
#include <ostream>

/* local interface headers */
#include "BZAdvancedRobot.h"
#include "Shot.h"

class BackendShot : public Shot
{
  public:
		BackendShot() : Shot() {}
    BackendShot(uint64_t _id) : Shot(_id) {}
    BackendShot(const Shot &s) : Shot(s) {}

		void getPosition(double &sx, double &sy, double &sz, double dt = 0) const; //Return the shots position in dt seconds
		void getVelocity(double &sx, double &sy, double &sz, double dt = 0) const;
};

#else

class BackendShot;

#endif /* __BACKENDSHOT_H__ */

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
