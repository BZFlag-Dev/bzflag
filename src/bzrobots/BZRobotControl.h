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

#ifndef __BZROBOTCONTROL_H__
#define __BZROBOTCONTROL_H__

#include "common.h"

#include "BZRobotCallbacks.h"
#include "BZRobotPlayer.h"

class BZRobotControl {
public:
  static BZRobotCallbacks *CallbackSet(BZRobotPlayer *rrp);
  static void Execute(void *_rrp);
  static double GetDistanceRemaining(void *_rrp);
  static double GetTurnRemaining(void *_rrp);
  static void SetAhead(void *_rrp, double);
  static void SetFire(void *_rrp);
  static void SetTurnRate(void *_rrp, double);
  static void SetMaxVelocity(void *_rrp, double);
  static void SetResume(void *_rrp);
  static void SetStop(void *_rrp, bool);
  static void SetTurnLeft(void *_rrp, double);
  static void SetTickDuration(void *_rrp, double);
  static double GetBattleFieldSize(void *_rrp);
  static double GetGunHeat(void *_rrp);
  static double GetVelocity(void *_rrp);
  static double GetHeading(void *_rrp);
  static double GetHeight(void *_rrp);
  static double GetWidth(void *_rrp);
  static double GetLength(void *_rrp);
  static long GetTime(void *_rrp);
  static double GetX(void *_rrp);
  static double GetY(void *_rrp);
  static double GetZ(void *_rrp);
};

#else

struct BZRobotControl;

#endif /* __BZROBOTCONTROL_H__ */

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
