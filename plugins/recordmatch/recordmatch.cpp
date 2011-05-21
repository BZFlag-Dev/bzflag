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

// recordmatch.cpp : Defines the entry point for the DLL application.
//

#include "bzfsAPI.h"
#include <stdio.h>

class GameStartEndHandler : public bz_EventHandler {
  public:
    virtual void process(bz_EventData* eventData);
};

GameStartEndHandler gameStartEndHandler;

std::string path;
bool started = false;
std::string filename;

BZ_GET_PLUGIN_VERSION

BZF_PLUGIN_CALL int bz_Load(const char* commandLine) {
  bz_registerEvent(bz_eGameStartEvent, &gameStartEndHandler);
  bz_registerEvent(bz_eGameEndEvent, &gameStartEndHandler);
  bz_debugMessage(4, "recordmatch plugin loaded");

  filename = commandLine;
  return 0;
}

BZF_PLUGIN_CALL int bz_Unload(void) {
  bz_debugMessage(4, "recordmatch plugin unloaded");
  bz_removeEvent(bz_eGameStartEvent, &gameStartEndHandler);
  bz_removeEvent(bz_eGameEndEvent, &gameStartEndHandler);

  return 0;
}

void GameStartEndHandler::process(bz_EventData* eventData) {
  switch (eventData->eventType) {
    case bz_eGameStartEvent: {
      started = bz_startRecBuf();

      bz_Time time;

      bz_getLocaltime(&time);

      char temp[512];
      sprintf(temp, "match-%d%02d%02d-%02d%02d%02d.rec",
              time.year, time.month, time.day,
              time.hour, time.minute, time.second);

      filename = temp;
    }
    break;

    case bz_eGameEndEvent: {
      if (!started) {
        break;
      }
      std::string recFile = path + filename;

      bz_saveRecBuf(recFile.c_str(), 0);
      bz_stopRecBuf();

      started = false;
      bz_sendTextMessagef(BZ_SERVER, BZ_ALLUSERS, "Match saved in file %s",
                          filename.c_str());
    }
    break;

    default:
      break;
  }
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=2 tabstop=8 expandtab
