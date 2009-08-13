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

#ifndef __BZADVANCEDROBOT_H__
#define __BZADVANCEDROBOT_H__

/* local interface headers */
#include "BZRobot.h"

class BZAdvancedRobot :public BZRobot
{
public:
  BZAdvancedRobot();
  virtual ~BZAdvancedRobot();

  void execute();
  double getDistanceRemaining() const;
  double getTurnRemaining() const;
  void setAhead(double distance);
  void setFire();
  void setTurnRate(double turnRate);
  void setMaxVelocity(double maxVelocity);
  void setResume();
  void setStop();
  void setStop(bool overwrite);
  void setTurnLeft(double degrees);
  
  /* These are helper functions. */
  double getBearing(double x, double y) const;
  double getDistance(double x, double y) const;
};

#else
class BZAdvancedRobot;
#endif  /* __BZADVANCEDROBOT_H__ */

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
