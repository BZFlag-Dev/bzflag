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

#ifndef BZADMIN_H
#define BZADMIN_H

#include <string>
#include <map>

#include "BZAdminUI.h"
#include "ServerLink.h"
#include "Team.h"
#include "Address.h"

using namespace std;


// global variables
extern map<PlayerId, string> players;
extern TeamColor myTeam;

/** Checks for new packets from the server, ignores them or stores a
    text message in str. Tells ui about new or removed players. Returns
    false if no interesting packets have arrived. */
bool getServerString(ServerLink& sLink, string& str, BZAdminUI& ui);

/** Sends the message msg to the server. */
void sendMessage(ServerLink& sLink, const string& msg, PlayerId target);


#endif

