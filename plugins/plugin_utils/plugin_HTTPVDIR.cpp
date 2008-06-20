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

#include "plugin_HTTPVDIR.h"
BZFSHTTPVDir::BZFSHTTPVDir()
{
  bz_loadPlugin("HTTPServer",NULL);

}

void BZFSHTTPVDir::registerVDir ( void )
{
  bz_callCallback("RegisterHTTPDVDir",this);
}

BZFSHTTPVDir::~BZFSHTTPVDir()
{
  bz_callCallback("RemoveHTTPDVDir",this);
}

bool BZFSHTTPVDir::put ( HTTPReply &/*reply*/, const char* /*vdir*/, const char* /*resource*/, int /*userID*/, int /*requestID*/, void* /*data*/, size_t /*size*/ )
{
  return true;
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
