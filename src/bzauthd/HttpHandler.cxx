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

#include <common.h>
#include "HttpHandler.h"

INSTANTIATE_SINGLETON(HttpHandler)

HttpHandler::HttpHandler()
{
  ctx = NULL;
}

HttpHandler::~HttpHandler()
{
  if(ctx) mg_stop(ctx);
}

bool HttpHandler::initialize()
{
  if(!(ctx = mg_start())) return false;
  if(1 != mg_set_option(ctx, "ports", "88")) return false;
  return true;
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
