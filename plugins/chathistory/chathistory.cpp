/* bzflag
 * Copyright (c) 1993-2018 Tim Riker
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
#include <map>

class LastChatCommand : public bz_CustomSlashCommandHandler
{
public:
  virtual ~LastChatCommand() {};
  virtual bool SlashCommand (int playerID, bz_ApiString command, bz_ApiString message, bz_APIStringList *param);
};

LastChatCommand lastChatCommand;

// event handler callback
class ChatEvents : public bz_Plugin
{
public:
  virtual ~ChatEvents() {};
  virtual const char* Name () {return "Chat History";}
  virtual void Init (const char* c);
  virtual void Cleanup (void);
  virtual void Event (bz_EventData *eventData);
};

BZ_PLUGIN(ChatEvents)

typedef std::vector<std::string>	tvChatHistory;

std::map<std::string, tvChatHistory>	chatHistories;

unsigned int		maxChatLines;

void ChatEvents::Init (const char* commandLine)
{
  // Default to 50 lines per player
  maxChatLines = 50;

  // Allow configuring how many lines to store
  if (commandLine && strlen(commandLine) > 0) {
    int realLines = atoi(commandLine);
    maxChatLines  = realLines;
  }

  // Register our custon slash commands
  bz_registerCustomSlashCommand("last", &lastChatCommand);
  bz_registerCustomSlashCommand("flushchat", &lastChatCommand);

  // Register the raw chat event
  Register(bz_eRawChatMessageEvent);

}

void ChatEvents::Cleanup(void)
{
  // Remove our custom slash commands
  bz_removeCustomSlashCommand("last");
  bz_removeCustomSlashCommand("flushchat");

  // Remove our events
  Flush();
}


bool LastChatCommand::SlashCommand (int playerID, bz_ApiString command, bz_ApiString message, bz_APIStringList * /*_param*/)
{
  // Only admins can run this command
  if (!bz_getAdmin(playerID)) {
    bz_sendTextMessage(BZ_SERVER, playerID, "You must be admin to use the ChatHistory plugin");
    return true;
  }

  // The 'last' command show the last X lines of text for a callsign
  if (command == "last") {
    // Create a string list and tokenize the message
    bz_APIStringList *params = bz_newStringList();
    params->tokenize(message.c_str(), " ", 0, true);

    // Must have two parameters
    if (params->size() != 2) {
      bz_sendTextMessage(BZ_SERVER, playerID, "Usage: /last <NUMBER OF LINES> <CALLSIGN>");
      return true;
    }

    // Parse the number of lines to return
    unsigned int numLines = (unsigned int)atoi(params->get(0).c_str());
    if (numLines == 0)
      numLines = 5;

    // Look up the player's chat history container
    std::map<std::string, tvChatHistory>::iterator itr = chatHistories.find(bz_tolower(params->get(1).c_str()));

    // If the container doesn't exist or has a 0 size, bail out
    if (itr == chatHistories.end() || !itr->second.size()) {
      bz_sendTextMessage(BZ_SERVER, playerID, "That player has no chat history.");
      return true;
    }

    // Store a reference to the chat history container
    tvChatHistory &history = itr->second;

    // If the number of lines stored is less than the number requested, reduce the requested amount
    if (history.size() < numLines)
      numLines = (unsigned int)history.size();

    // Send the messages to the requestor
    bz_sendTextMessage(BZ_SERVER, playerID, bz_format("Last %d message(s) for %s", numLines, params->get(1).c_str()));
    for (unsigned int i = numLines; i > 0; i--) {
      std::string chatItem = history[history.size()-i];
      bz_sendTextMessage(BZ_SERVER, playerID, bz_format("  <%s> %s", params->get(1).c_str(), chatItem.c_str()));
    }

    return true;
  }

  // Clear all the chat histories
  if (command == "flushchat") {
    chatHistories.clear();
    bz_sendTextMessage(BZ_SERVER, playerID, "Chat History has been flushed");
    return true;
  }

  return false;
}

void ChatEvents::Event (bz_EventData *eventData)
{
  // We only handle raw chat messages
  if (eventData->eventType != bz_eRawChatMessageEvent)
    return;

  // Cast the event data
  bz_ChatEventData_V1	*chatEventData = (bz_ChatEventData_V1*)eventData;

  // Retrieve the sender information
  bz_BasePlayerRecord *fromPlayer = bz_getPlayerByIndex(chatEventData->from);

  // Bail if we can't find the player
  if (!fromPlayer)
    return;

  // Store the message in a std::string
  std::string message = chatEventData->message.c_str();

  // Store the callsign
  std::string callsign = bz_tolower(fromPlayer->callsign.c_str());

  // Create a new chat history log for this callsign if necessary
  if (chatHistories.find(callsign) == chatHistories.end()) {
    tvChatHistory h;
    chatHistories[callsign] = h;
  }

  // Store a reference to the chat history for this callsign
  tvChatHistory &history = chatHistories[callsign];

  // Add the new message
  history.push_back(message);

  // Check if the number of chat history items exceeds the per-callsign limit.
  // If it does, remove the oldest entry
  if (history.size() > maxChatLines)
    history.erase(history.begin());

  // Free the memory from the player record
  bz_freePlayerRecord(fromPlayer);

}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
