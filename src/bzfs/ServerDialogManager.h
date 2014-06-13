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

#ifndef BZF_SERVERDIALOG_H
#define BZF_SERVERDIALOG_H

#include "common.h"

#include <string>
#include <map>

#include "DialogData.h"

class ServerDialogManager {
public:
  std::map<uint32_t, DialogData*> dialogData;
  uint32_t lastDialogID;


  ServerDialogManager();

  uint32_t getNextDialogID();

  DialogData* addDialog(DialogType type, int playerID, std::string title);

  bool send(uint32_t dialogID);
  bool close(uint32_t dialogID);
};


#endif

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
