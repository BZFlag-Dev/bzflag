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

#include "ServerDialogManager.h"
#include "bzfs.h"

ServerDialogManager::ServerDialogManager() : lastDialogID(0)
{
}

// TODO: Handle ID reuse in the event that we loop around.
uint32_t ServerDialogManager::getNextDialogID()
{
  lastDialogID++;

  return lastDialogID;
}

DialogData* ServerDialogManager::addDialog(DialogType type, int playerID, std::string title)
{
  // Get a new dialog ID
  uint32_t dialogID = getNextDialogID();

  // Create a new modal dialog
  DialogData *dialog = new DialogData(dialogID, type, playerID, title);

  // Store the dialog in the manager
  dialogData[dialogID] = dialog;

  // Send back the dialog so that it can be populated with items and buttons
  return dialog;
}

bool ServerDialogManager::send(uint32_t dialogID) {
  if (dialogData[dialogID] == NULL)
    return false;

  void *buf, *bufStart = getDirectMessageBuffer();
  buf = dialogData[dialogID]->packDialog(bufStart);
  int len = (char*)buf - (char*)bufStart;
  directMessage(dialogData[dialogID]->dialogOwner, MsgDialogCreate, len, bufStart);

  return true;
}

bool ServerDialogManager::close(uint32_t dialogID) {
  return true;
}



// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
