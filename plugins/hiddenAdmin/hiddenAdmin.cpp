/* bzflag
 * Copyright (c) 1993-2010 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

// hiddenAdmin.cpp : Defines the entry point for the DLL application.
//


#include "bzfsAPI.h"
#include <string>
#include <map>

BZ_GET_PLUGIN_VERSION

// event handler callback

class HiddenAdmin : public bz_EventHandler {
  public:
    HiddenAdmin();
    virtual ~HiddenAdmin();

    virtual void process(bz_EventData* eventData);

  protected:
};

HiddenAdmin hiddenAdmin;

BZF_PLUGIN_CALL int bz_Load(const char* /*commandLine*/) {
  bz_debugMessage(4, "HiddenAdmin plugin loaded");

  bz_registerEvent(bz_eGetPlayerInfoEvent, &hiddenAdmin);

  return 0;
}

BZF_PLUGIN_CALL int bz_Unload(void) {
  bz_removeEvent(bz_eGetPlayerInfoEvent, &hiddenAdmin);

  bz_debugMessage(4, "HiddenAdmin plugin unloaded");
  return 0;
}


HiddenAdmin::HiddenAdmin() {
}

HiddenAdmin::~HiddenAdmin() {
}

void HiddenAdmin::process(bz_EventData* eventData) {
  if (eventData->eventType != bz_eGetPlayerInfoEvent) {
    return;
  }

  bz_GetPlayerInfoEventData_V1* infoData = (bz_GetPlayerInfoEventData_V1*)eventData;

  infoData->admin = false;
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=2 tabstop=8
