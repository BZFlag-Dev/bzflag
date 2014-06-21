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

#include "ClientDialogManager.h"

#include "Address.h"

ClientDialogManager::ClientDialogManager()
{
}

uint32_t ClientDialogManager::unpackDialogCreate(const void * msg) {
  DialogData* dialog = new DialogData();

  // Try to unpack the dialog message
  if (dialog->unpackDialog(msg)) {
    // Success!  Store the dialog.
    dialogData[dialog->dialogID] = dialog;

    return dialog->dialogID;
  }

  return 0;
}

uint32_t ClientDialogManager::unpackDialogDestroy(const void * msg) {
  uint32_t dialogID;

  msg = nboUnpackUInt(msg, dialogID);

  if (dialogData[dialogID] != NULL) {
    delete dialogData[dialogID];
    dialogData.erase(dialogID);
    return dialogID;
  }

  return 0;
}

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
