/* bzflag
 * Copyright (c) 1993 - 2006 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef __STARTUPINFO_H__
	#define __STARTUPINFO_H__

	#include "common.h"

/* system interface headers */
	#include <string>

/* common interface headers */
	#include "Team.h"


class StartupInfo
{
public:
	StartupInfo();
	~StartupInfo();

	bool hasConfiguration;
	bool autoConnect;
	char serverName[80];
	int serverPort;
	bool useUDPconnection;
	TeamColor team;
	char callsign[CallSignLen];
	char password[PasswordLen];
	char token[TokenLen];
	char email[EmailLen];
	std::string listServerURL;
	int listServerPort;
	bool joystick;
	std::string joystickName;
};


#endif /* __STARTUPINFO_H__ */

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
