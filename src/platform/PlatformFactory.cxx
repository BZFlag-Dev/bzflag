/* bzflag
 * Copyright (c) 1993 - 2003 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#include "PlatformFactory.h"

PlatformFactory*	PlatformFactory::instance = 0;
BzfMedia*		PlatformFactory::media = 0;

PlatformFactory::PlatformFactory()
{
  // do nothing
}

PlatformFactory::~PlatformFactory()
{
  // do nothing
}

BzfMedia*		PlatformFactory::getMedia()
{
  if (!media) media = getInstance()->createMedia();
  return media;
}
// ex: shiftwidth=2 tabstop=8
