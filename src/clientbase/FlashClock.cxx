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

/* interface header */
#include "FlashClock.h"

/* system implementation headers */
#include <math.h>


FlashClock::FlashClock()
  : startTime(BzTime::getCurrent())
  , duration(0.0f)
  , onDuration(0.0f)
  , flashDuration(0.0f)
{
  // do nothing
}

FlashClock::~FlashClock()
{
  // do nothing
}

void FlashClock::setClock(float _duration)
{
  setClock(_duration, 0.0f, 0.0f);
}

void FlashClock::setClock(float _duration, float onTime, float offTime)
{
  if (BzTime::getTick().getSeconds() > 0.0)
    startTime = BzTime::getTick();
  duration = _duration;
  if (onTime <= 0.0f || offTime <= 0.0f) {
    onDuration = 0.0f;
    flashDuration = 0.0f;
  } else {
    onDuration = onTime;
    flashDuration = onTime + offTime;
  }
}

bool FlashClock::isOn()
{
  if (duration == 0.0f) return false;
  const double dt = BzTime::getTick() - startTime;
  if (duration > 0.0f && dt >= (double)duration) {
    duration = 0.0f;
    return false;
  }
  if (flashDuration == 0.0f) return true;
  return (fmod(dt, (double)flashDuration) < onDuration);
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
