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

#ifndef __BZROBOTCALLBACKS_H__
#define __BZROBOTCALLBACKS_H__

#include "common.h"

typedef struct BZRobotCallbacks {
  void *data; // Data to be sent back to callbacks
  void (*Ahead)(void *data, double);
  void (*Back)(void *data, double);
  void (*Execute)(void *data);
  void (*Fire)(void *data);
  double (*GetBattleFieldSize)(void *data);
  double (*GetDistanceRemaining)(void *data);
  double (*GetGunCoolingRate)(void *data);
  double (*GetGunHeat)(void *data);
  double (*GetHeading)(void *data);
  double (*GetHeight)(void *data);
  const char * (*GetName)(void *data);
  double (*GetLength)(void *data);
  double (*GetTime)(void *data);
  double (*GetTurnRemaining)(void *data);
  double (*GetVelocity)(void *data);
  double (*GetWidth)(void *data);
  double (*GetX)(void *data);
  double (*GetY)(void *data);
  double (*GetZ)(void *data);
  void (*SetAhead)(void *data, double);
  void (*SetFire)(void *data);
  void (*SetMaxVelocity)(void *data, double);
  void (*SetResume)(void *data);
  void (*SetStop)(void *data, bool);
  void (*SetTurnLeft)(void *data, double);
  void (*SetTurnRate)(void *data, double);
  void (*Resume)(void *data);
  void (*Scan)(void *data);
  void (*Stop)(void *data, bool);
  void (*TurnLeft)(void *data, double);
  void (*TurnRight)(void *data, double);
} BZRobotCallbacks;

#else

struct BZRobotCallbacks;

#endif /* __BZROBOTCALLBACKS_H__ */

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
