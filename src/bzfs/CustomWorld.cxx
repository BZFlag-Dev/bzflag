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

// class-interface header
#include "CustomWorld.h"

// system headers
#include <string.h>

// common-iterface headers
#include "StateDatabase.h"
#include "TextUtils.h"

CustomWorld::CustomWorld()
{
  size = 800;
  fHeight = 0;
}


bool CustomWorld::read(const char *cmd, istream& input)
{
  if (strcmp(cmd, "size") == 0) {
    input >> size;
	size *=2;
    BZDB->set(StateDatabase::BZDB_WORLDSIZE, string_util::format("%d", size));
  }
  else if (strcmp(cmd, "flagHeight") == 0)
    input >> fHeight;
  else
    return false;
  return true;
}


void CustomWorld::write(WorldInfo*) const
{
  BZDB->set(StateDatabase::BZDB_FLAGHEIGHT, string_util::format("%f", fHeight));
}

// Local variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
