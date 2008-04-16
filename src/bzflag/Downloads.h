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

#ifndef DOWNLOADS_H
#define DOWNLOADS_H

#include "common.h"
#include "Singleton.h"
#include "BzMaterial.h"
#include "AccessList.h"

/* system interface headers */
#include <string>

class Downloads :   public Singleton<Downloads>
{
public:
  void startDownloads(bool doDownloads, bool updateDownloads, bool referencing);
  void finalizeDownloads();
  void removeTextures(); // free the downloaded GL textures
  bool requestFinalized();
  bool authorizedServer(const std::string& hostname);

protected:
  friend class Singleton<Downloads>;

private:
  Downloads();
  ~Downloads();

  void printAuthNotice();
  bool checkAuthorizations(BzMaterialManager::TextureSet& set);

  AccessList * downloadAccessList;
  bool textureDownloading;
};


bool parseHostname(const std::string& url, std::string& hostname);


#endif

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
