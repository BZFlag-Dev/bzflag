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

#ifndef __BULLET_H__
#define __BULLET_H__

#include <string>

class Bullet {
public:
  Bullet() :
      heading(0.0f),
      headingRadians(0.0f),
	  ownerName(""),
      velocity(0.0f),
      victimName(""),
      x(0.0f),
      y(0.0f),
	  z(0.0f),
	  active(false) {}

  virtual ~Bullet();

  inline double const getHeading() { return heading; }
  inline double const getHeadingRadians() { return headingRadians; }
  inline std::string const getName() { return ownerName; }
  inline double const getVelocity() { return velocity; }
  inline std::string const getVictim() { return victimName; }
  inline double const getX() { return x; }
  inline double const getY() { return y; }
  inline double const getZ() { return z; }
  inline bool const isActive() { return active; }

protected:
  double heading;
  double headingRadians;
  std::string ownerName;
  double velocity;
  std::string victimName;
  double x, y, z;
  bool active;
};

#else

class Bullet;

#endif /* __BULLET_H__ */

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
