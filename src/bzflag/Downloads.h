/* bzflag
 * Copyright (c) 1993 - 2007 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef DOWNLOADS_H
#define DOWNLOADS_H

#include "common.h"

/* system interface headers */
#include <string>


namespace Downloads {
  void startDownloads(bool doDownloads,
		      bool updateDownloads,
		      bool referencing);
  void finalizeDownloads();
  void removeTextures(); // free the downloaded GL textures
  bool requestFinalized();
}

bool authorizedServer(const std::string& hostname);
bool parseHostname(const std::string& url, std::string& hostname);


#endif

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
