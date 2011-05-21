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

// chathistory.cpp : Defines the entry point for the DLL application.
//

#include "bzfsAPI.h"
#include <string>
#include <algorithm>
#include <sstream>
#include <stdarg.h>
#include <vector>
#include <stdio.h>
#include <assert.h>
#include <map>
#include <vector>
#include "plugin_utils.h"


BZ_GET_PLUGIN_VERSION

class LastChatCommand: public bz_CustomSlashCommandHandler {
  public:
    virtual ~LastChatCommand() {};
    virtual bool handle(int playerID, bz_ApiString command, bz_ApiString message, bz_APIStringList* param);
};

LastChatCommand lastChatCommand;

// event handler callback
class ChatEvents: public bz_EventHandler {
  public:
    virtual ~ChatEvents() {};
    virtual void process(bz_EventData* eventData);
};

ChatEvents chatEvents;

typedef std::vector < std::string > tvChatHistory;

std::map < std::string, tvChatHistory > chatHistories;

unsigned int maxChatLines;

BZF_PLUGIN_CALL int bz_Load(const char* commandLine) {
  bz_debugMessage(4, "ChatEvents plugin loaded");

  maxChatLines = 1000;
  if (commandLine) {
    int realLines = atoi(commandLine);
    maxChatLines = realLines;
  }

  bz_registerCustomSlashCommand("last", &lastChatCommand);
  bz_registerCustomSlashCommand("flushchat", &lastChatCommand);

  bz_registerEvent(bz_eRawChatMessageEvent, &chatEvents);

  return 0;
}

BZF_PLUGIN_CALL int bz_Unload(void) {
  bz_removeCustomSlashCommand("last");
  bz_removeCustomSlashCommand("flushchat");

  bz_removeEvent(bz_eRawChatMessageEvent, &chatEvents);

  bz_debugMessage(4, "ChatEvents plugin unloaded");
  return 0;
}

bool LastChatCommand::handle(int playerID, bz_ApiString _command, bz_ApiString _message, bz_APIStringList* /*_param*/) {
  std::string command = _command.c_str();
  std::string message = _message.c_str();

  bz_BasePlayerRecord* fromPlayer = bz_getPlayerByIndex(playerID);

  if (!fromPlayer) {
    return false;
  }

  if (!fromPlayer->admin) {
    bz_sendTextMessage(BZ_SERVER, playerID, "You must be admin to use the ChatHistory plugin");
    bz_freePlayerRecord(fromPlayer);
    return true;
  }
  bz_freePlayerRecord(fromPlayer);

  if (command == "last") {
    std::vector < std::string > params = tokenize(message, std::string(" "), 0, true);
    if (params.size() < 2) {
      bz_sendTextMessage(BZ_SERVER, playerID, "Usage: /last <NUMBER OF LINES> <CALLSIGN>");
      return true;
    }

    unsigned int numLines = (unsigned int) atoi(params[0].c_str());
    if (numLines == 0) {
      numLines = 5;
    }

    std::string callsign;
    tolower(params[1], callsign);
    std::map < std::string, tvChatHistory >::iterator itr = chatHistories.find(callsign);

    if (itr == chatHistories.end() || !itr->second.size()) {
      bz_sendTextMessage(BZ_SERVER, playerID, "That player has no chat history.");
      return true;
    }

    tvChatHistory& history = itr->second;

    if (history.size() < numLines) {
      numLines = (unsigned int) history.size();
    }

    bz_sendTextMessage(BZ_SERVER, playerID, format("Last %d message for %s", numLines, callsign.c_str()).c_str());

    for (unsigned int i = 0; i < numLines - 1; i++) {
      std::string chatItem = history[history.size() - i];
      bz_sendTextMessage(BZ_SERVER, playerID, format("%d<%s> %s", i, callsign.c_str(), chatItem.c_str()).c_str());
    }

    return true;
  }

  if (command == "flushchat") {
    chatHistories.clear();
    bz_sendTextMessage(BZ_SERVER, playerID, "Chat History has been flushed");
    return true;
  }

  return false;
}

void ChatEvents::process(bz_EventData* eventData) {
  bz_ChatEventData_V1* chatEventData = (bz_ChatEventData_V1*) eventData;

  bz_BasePlayerRecord* fromPlayer = bz_getPlayerByIndex(chatEventData->from);

  std::string message = chatEventData->message.c_str();

  std::string callsign = "";
  if (fromPlayer) {
    callsign = fromPlayer->callsign.c_str();
  }

  makelower(callsign);

  switch (eventData->eventType) {
    default:
      break;

    case bz_eRawChatMessageEvent:
      std::map < std::string, tvChatHistory >::iterator itr = chatHistories.find(callsign);
      if (itr == chatHistories.end()) {
        tvChatHistory h;
        chatHistories[callsign] = h;
      }

      tvChatHistory& history = chatHistories[callsign];

      history.push_back(message);
      if (history.size() > maxChatLines) {
        history.erase(history.begin());
      }
      break;

  }

  bz_freePlayerRecord(fromPlayer);
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=2 tabstop=8
