/* bzflag
 * Copyright (c) 1993 - 2007 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* interface header */
#include "ClientAuthentication.h"

/* system implementation headers */
#include <string>
#include "DirectoryNames.h"

char	      ClientAuthentication::principalName[128];
bool	      ClientAuthentication::authentication = false;

void ClientAuthentication::login(const char *username, const char *)
{
  if (authentication)
    logout();

  strncpy(principalName, username, 128);
  principalName[127] = 0;
}

void ClientAuthentication::logout()
{
  principalName[0] = 0;
  if (authentication)
    authentication = false;
}


void ClientAuthentication::sendCredential(ServerLink&)
{
  if (!authentication)
    return;
}

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
