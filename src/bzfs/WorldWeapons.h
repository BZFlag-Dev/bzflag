/* bzflag
 * Copyright (c) 1993 - 2003 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef __WORLDWEAPON_H__
#define __WORLDWEAPON_H__

#ifdef _WIN32
#pragma warning(4:4786)
#endif

#include <vector>
#include "Flag.h"
#include "TimeKeeper.h"

class WorldWeapons
{
public:
  WorldWeapons();
  ~WorldWeapons();
  void fire();
  void add( const FlagDesc *type, const float *origin, float direction, float initdelay, const std::vector<float> &delay);
private:
  struct Weapon
  {
    const FlagDesc	*type;
    float		origin[3];
    float		direction;
    std::vector<float>  delay;
    TimeKeeper		nextTime;
    int			nextDelay;
  };

  std::vector<Weapon*> weapons;
  int worldShotId;

  WorldWeapons( const WorldWeapons &w);
  WorldWeapons& operator=(const WorldWeapons &w) const;
};

#endif
