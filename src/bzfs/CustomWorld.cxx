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

#ifdef _MSC_VER
#pragma warning( 4: 4786 )
#endif

// class-interface header
#include "CustomWorld.h"

// system headers
#include <string.h>

// common-iterface headers
#include "StateDatabase.h"
#include "TextUtils.h"

CustomWorld::CustomWorld()
{
  // initialize with database defaults
  _size = BZDB.eval(StateDatabase::BZDB_WORLDSIZE);
  _fHeight = BZDB.eval(StateDatabase::BZDB_FLAGHEIGHT);
}


bool CustomWorld::read(const char *cmd, std::istream& input)
{
  if (strcmp(cmd, "size") == 0) {
    input >> _size;
    _size *= 2.0;
    BZDB.set(StateDatabase::BZDB_WORLDSIZE, string_util::format("%f", _size));
  } else if (strcmp(cmd, "flagHeight") == 0) {
    input >> _fHeight;
    BZDB.set(StateDatabase::BZDB_FLAGHEIGHT, string_util::format("%f", _fHeight));
  } else {
    return WorldFileObject::read(cmd, input);
  }
  return true;
}


void CustomWorld::write(WorldInfo*) const
{
}

// Local variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
