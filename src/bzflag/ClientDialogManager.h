/* bzflag
* Copyright (c) 1993-2014 Tim Riker
*
* This package is free software;  you can redistribute it and/or
* modify it under the terms of the license found in the file
* named COPYING that should have accompanied this file.
*
* THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
* IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
*/

#ifndef BZF_CLIENTDIALOGMANAGER_H
#define BZF_CLIENTDIALOGMANAGER_H

#include "common.h"

#include <string>
#include <map>

#include "DialogData.h"

class ClientDialogManager {
public:
  std::map<uint32_t, DialogData*> dialogData;

  ClientDialogManager();

  uint32_t unpackDialogCreate(const void * msg);
  uint32_t unpackDialogDestroy(const void * msg);
};


#endif

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
