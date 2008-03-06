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
#include "StartupInfo.h"

/* system implementation headers */
#include <string.h>

/* common implementation headers */
#include "Protocol.h"


StartupInfo::StartupInfo() : hasConfiguration(false),
			     autoConnect(false),
			     serverPort(ServerPort),
			     team(AutomaticTeam),
			     listServerURL(DefaultListServerURL),
			     listServerPort(ServerPort + 1)
{
  strcpy(serverName, "");
  strcpy(callsign, "");
  strcpy(password, "");
  strcpy(email, "default");
  joystickName = "joystick";
  joystick = false;
}

StartupInfo::~StartupInfo()
{
  hasConfiguration = false;
  autoConnect = false;
  memset(serverName, 0, 80);
  serverPort = -1;
  useUDPconnection = false;
  team = NoTeam;
  memset(callsign, 0, CallSignLen);
  memset(password, 0, PasswordLen);
  memset(token, 0, TokenLen);
  memset(email, 0, EmailLen);
  listServerURL = "";
  listServerPort = 0;
  joystick = false;
  joystickName = "";
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
