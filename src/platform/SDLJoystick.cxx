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

/* common headers */
#include "common.h"

#ifdef HAVE_SDL

/* interface headers */
#include "SDLJoystick.h"

/* implementation headers */
#include "ErrorHandler.h"
#include "bzfSDL.h"

SDLJoystick::SDLJoystick()
{
  if (SDL_InitSubSystem(SDL_INIT_JOYSTICK) == -1) {
    std::vector<std::string> args;
    args.push_back(SDL_GetError());
    printError("Could not initialize SDL Joystick subsystem: %s.\n", &args);
  };
} 

SDLJoystick::~SDLJoystick()
{
  SDL_QuitSubSystem(SDL_INIT_JOYSTICK);
}

void			SDLJoystick::initJoystick(const char* joystickName)
{
  if (!strcmp(joystickName, "off") || !strcmp(joystickName, "")) {
    joystickID = NULL;
    return;
  }
  int numJoystick = SDL_NumJoysticks();
  if (!numJoystick) {
    printError("No supported SDL joysticks were found.");
    joystickID = NULL;
    return;
  }
  int i;
  for (i = 0; i < numJoystick; i++)
    if (strcmp(SDL_JoystickName(i), joystickName) == 0)
      break;
  if (i >= numJoystick)
    i = 0;
  joystickID = SDL_JoystickOpen(i);
  if (joystickID == NULL)
    return;
  if (SDL_JoystickNumAxes(joystickID) < 2) {
    SDL_JoystickClose(joystickID);
    printError("Joystick has less then 2 axis:\n");
    joystickID = NULL;
    return;
  }
  joystickButtons = SDL_JoystickNumButtons(joystickID);
}

bool			SDLJoystick::joystick() const
{
  return joystickID != NULL;
}

void			SDLJoystick::getJoy(int& x, int& y) const
{
  x = y = 0;

  if (!joystickID)
    return;

  SDL_JoystickUpdate();
  x = SDL_JoystickGetAxis(joystickID, 0);
  y = SDL_JoystickGetAxis(joystickID, 1);

  x = x * 1000 / 32768;
  y = y * 1000 / 32768;

  // ballistic
  x = (x * abs(x)) / 1000;
  y = (y * abs(y)) / 1000;

}

unsigned long		SDLJoystick::getJoyButtons() const
{
  unsigned long buttons = 0;

  if (!joystickID)
    return 0;

  SDL_JoystickUpdate();
  for (int i = 0; i < joystickButtons; i++)
    buttons |= SDL_JoystickGetButton(joystickID, i) << i;

  return buttons;
}

void                    SDLJoystick::getJoyDevices(std::vector<std::string>
						 &list) const
{
  int numJoystick = SDL_NumJoysticks();
  int i;
  for (i = 0; i < numJoystick; i++)
    list.push_back(SDL_JoystickName(i));
}
#endif

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
