/* bzflag
 * Copyright (c) 1993 - 2004 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* interface header */
#include "ForceFeedback.h"

/* common interface headers */
#include "BzfJoystick.h"

/* local implementation headers */
#include "MainMenu.h"
#include "LocalPlayer.h"
#include "playing.h"

static BzfJoystick*     getJoystick();
static bool             useForceFeedback();


static BzfJoystick*     getJoystick()
{
  MainWindow *win = getMainWindow();
  if (win)
    return win->getJoystick();
  else
    return NULL;
}

static bool             useForceFeedback()
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

  /* FIXME: Let the user disable force feedback */

  return true;
}

namespace ForceFeedback {

  void death()
  {
    if (useForceFeedback())
      getJoystick()->ffRumble(1, 0.0f, 1.5f, 1.0f, 0.0f);
  }

}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
