/* bzflag
 * Copyright (c) 1993 - 2008 Tim Riker
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
#include "ForceFeedback.h"

/* common interface headers */
#include "BzfJoystick.h"

/* local implementation headers */
#include "LocalPlayer.h"
#include "playing.h"

static BzfJoystick*     getJoystick();
static bool	     useForceFeedback(const char *type = "Rumble");


static BzfJoystick*     getJoystick()
{
  MainWindow *win = getMainWindow();
  if (win)
    return win->getJoystick();
  else
    return NULL;
}

static bool	     useForceFeedback(const char *type)
{
  BzfJoystick* js = getJoystick();

  /* There must be a joystick class, and we need an opened joystick device */
  if (!js)
    return false;
  if (!js->joystick())
    return false;

  /* Joystick must be the current input method */
  if (LocalPlayer::getMyTank()->getInputMethod() != LocalPlayer::Joystick)
    return false;

  /* Did the user enable force feedback of this type? */
  if (BZDB.get("forceFeedback") != type)
    return false;

  return true;
}

namespace ForceFeedback {

  void death()
  {
    /* Nice long hard rumble for death */
    if (useForceFeedback("Rumble"))
      getJoystick()->ffRumble(1, 0.0f, 1.5f, 1.0f, 0.0f);
    else if (useForceFeedback("Directional"))
      getJoystick()->ffDirectionalPeriodic(1, 0.0f, 1.5f, 1.0f, 0.0f, 1.0f, 0.15f, BzfJoystick::FF_SawtoothDown);
  }

  void shotFired()
  {
    /* Tiny little kick for a normal shot being fired */
    if (useForceFeedback("Rumble"))
      getJoystick()->ffRumble(1, 0.0f, 0.1f, 0.0f, 1.0f);
    else if (useForceFeedback("Directional"))
      getJoystick()->ffDirectionalConstant(1, 0.0f, 0.1f, 0.0f, -1.0f, 0.5f);
  }

  void laserFired()
  {
    /* Funky pulsating rumble for the laser.
     * (Only tested so far with the Logitech Wingman Cordless Rumblepad,
     *  some quirks in its driver may mean it's feeling a little different
     *  than it should)
     */
    if (useForceFeedback("Rumble"))
      getJoystick()->ffRumble(4, 0.01f, 0.02f, 1.0f, 1.0f);
    else if (useForceFeedback("Directional"))
      getJoystick()->ffDirectionalPeriodic(4, 0.1f, 0.1f, 0.0f, -1.0f, 0.5f, 0.05f, BzfJoystick::FF_Sine);
  }

  void shockwaveFired()
  {
    /* try to 'match' the shockwave sound */
    if (useForceFeedback("Rumble"))
      getJoystick()->ffRumble(1, 0.0f, 0.5f, 0.0f, 1.0f);
    else if (useForceFeedback("Directional"))
      getJoystick()->ffDirectionalPeriodic(1, 0.0f, 1.0f, 0.0f, -1.0f, 0.5f, 0.1f, BzfJoystick::FF_Sine);
  }

  /* Burrowed, oscillating, etc, tanks get a special resistance force
   * when moving through solid matter.  We use half-second increments
   * of force-on time.
   */
  static TimeKeeper friction_timer = TimeKeeper::getSunGenesisTime();
  void solidMatterFriction()
  {
    /* There is no way to simulate this with a rumble effect */
    if (useForceFeedback("Directional")) {
      if ((TimeKeeper::getCurrent() - friction_timer) >= 0.5f) {
	getJoystick()->ffDirectionalResistance(0.5f, 1.0f, 0.5f, BzfJoystick::FF_Position);
	friction_timer = TimeKeeper::getCurrent();
      }
    }
  }
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
