/* bzflag
 * Copyright (c) 1993 - 2007 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* common headers */
#include "common.h"

#ifdef HAVE_SDL

/* interface headers */
#include "SDLJoystick.h"

/* system headers */
#include <vector>
#include <string>
#include <string.h>
#include <ctype.h>

/* implementation headers */
#include "ErrorHandler.h"
#include "TextUtils.h"
#include "bzfSDL.h"

SDLJoystick::SDLJoystick() : joystickID(NULL)
{
  if (SDL_InitSubSystem(SDL_INIT_JOYSTICK) == -1) {
    std::vector<std::string> args;
    args.push_back(SDL_GetError());
    printError("Could not initialize SDL Joystick subsystem: %s.\n", &args);
  };
  xAxis = 0;
  yAxis = 1;
}

SDLJoystick::~SDLJoystick()
{
  SDL_QuitSubSystem(SDL_INIT_JOYSTICK);
}

void			SDLJoystick::initJoystick(const char* joystickName)
{
  if (!strcasecmp(joystickName, "off") || !strcmp(joystickName, "")) {
    if (joystickID != NULL) {
      SDL_JoystickClose(joystickID);
      joystickID = NULL;
    }
    return;
  }
  char num = joystickName[0];
  int  i   = (int)num - '0';
  if (!isdigit(num) || i >= SDL_NumJoysticks()) {
    printError("No supported SDL joysticks were found.");
    joystickID = NULL;
    return;
  }
  joystickID = SDL_JoystickOpen(i);
  if (joystickID == NULL)
    return;
  if (SDL_JoystickNumAxes(joystickID) < 2) {
    SDL_JoystickClose(joystickID);
    printError("Joystick has less then 2 axes:\n");
    joystickID = NULL;
    return;
  }
  joystickButtons = SDL_JoystickNumButtons(joystickID);
}

bool			SDLJoystick::joystick() const
{
  return joystickID != NULL;
}

void			SDLJoystick::getJoy(int& x, int& y)
{
  x = y = 0;

  if (!joystickID)
    return;

  SDL_JoystickUpdate();
  x = SDL_JoystickGetAxis(joystickID, xAxis);
  y = SDL_JoystickGetAxis(joystickID, yAxis);

  x = x * 1000 / 32768;
  y = y * 1000 / 32768;

  // ballistic
  x = (x * abs(x)) / 1000;
  y = (y * abs(y)) / 1000;

}

unsigned long		SDLJoystick::getJoyButtons()
{
  unsigned long buttons = 0;

  if (!joystickID)
    return 0;

  SDL_JoystickUpdate();
  for (int i = 0; i < joystickButtons; i++)
    buttons |= SDL_JoystickGetButton(joystickID, i) << i;

  return buttons;
}

void		    SDLJoystick::getJoyDevices(std::vector<std::string>
						 &list) const
{
  int numJoystick = SDL_NumJoysticks();
  if (numJoystick > 9) //user would have to be insane to have this many anyway
    numJoystick = 9;
  int i;
  for (i = 0; i < numJoystick; i++) {
    char joystickName[50]; //only room for so much on the menu
    snprintf(joystickName, 50, "%d - %s", i, SDL_JoystickName(i));
    list.push_back(joystickName);
  }
}

void		    SDLJoystick::getJoyDeviceAxes(std::vector<std::string> &list) const
{
  if (!joystickID) return;
  list.clear();
  // number all the axes and send them off
  for (int i = 0; i < SDL_JoystickNumAxes(joystickID); ++i) {
    list.push_back(TextUtils::format("%d", i));
  }
}

void		    SDLJoystick::setXAxis(const std::string axis)
{
  // unset
  if (axis == "") return;
  xAxis = atoi(axis.c_str());
}

void		    SDLJoystick::setYAxis(const std::string axis)
{
  // unset
  if (axis == "") return;
  yAxis = atoi(axis.c_str());
}

unsigned int	SDLJoystick::getHatswitch(int switchno) const
{
  if (!joystickID) return 0;
  return SDL_JoystickGetHat(joystickID, switchno);
}

unsigned int	SDLJoystick::getJoyDeviceNumHats() const
{
  if (!joystickID) return 0;
  return SDL_JoystickNumHats(joystickID);
}

#endif

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
