/* bzflag
 * Copyright (c) 1993 - 2009 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* interface header */
#include "ComposeDefaultKey.h"

/* system headers */
#include <string>
#include <set>

/* common implementation headers */
#include "BzfEvent.h"
#include "KeyManager.h"
#include "EventHandler.h"

/* local implementation headers */
#include "LocalPlayer.h"
#include "HUDRenderer.h"
#include "LocalCommand.h"
#include "playing.h"
#include "HUDui.h"

#define MAX_MESSAGE_HISTORY (20)

MessageQueue messageHistory;
unsigned int messageHistoryIndex = 0;

static bool isWordCompletion(const BzfKeyEvent& key)
{
  if ((key.chr == 6) || // ^F
      (key.chr== 9) || // <TAB>
      ((key.shift == 0) && (key.button == BzfKeyEvent::F2))) {
    return true;
  } else {
    return false;
  }
}


bool ComposeDefaultKey::keyPress(const BzfKeyEvent& key)
{
  bool sendIt;
  LocalPlayer *myTank = LocalPlayer::getMyTank();
  if (myTank && KEYMGR.get(key, true) == "jump" && BZDB.isTrue("jumpTyping")) {
    // jump while typing
    myTank->setJump();
    return false;
  }

  if (myTank &&
      ((myTank->getInputMethod() != LocalPlayer::Keyboard) ||
       (myTank->getTeam() == ObserverTeam))) {
    if ((key.button == BzfKeyEvent::Up) ||
	(key.button == BzfKeyEvent::Down)) {
      return true;
    }
  }

  if (isWordCompletion(key)) {
    const std::string line = hud->getComposeString();
    std::set<std::string> partials;
    if (!BZDB.isTrue("noDefaultWordComplete")) {
      completer.complete(line, partials);
    }
    eventHandler.WordComplete(line, partials);
    if (!partials.empty()) {    
      // find the longest common string
      const std::string first = *(partials.begin());
      const std::string last = *(partials.rbegin());
      size_t i;
      size_t maxLen = std::max(first.size(), last.size());
      for (i = 0; i < maxLen; i++) {
        if (first[i] != last[i]) {
          break;
        }
      }
      hud->setComposeString(line + first.substr(0, i));

      if (partials.size() >= 2) {
        const int lastSpace = line.find_last_of(" \t");
        const std::string lastWord = line.substr(lastSpace + 1);
                  
        std::string matches;
        std::set<std::string>::const_iterator it;
        for (it = partials.begin(); it != partials.end(); ++it) {
          matches += "  ";
          matches += lastWord + it->substr(i);
        }
        controlPanel->addMessage(matches, ControlPanel::MessageCurrent);
      }
    }
    return true;
  }

  switch (key.chr) {
    case 3: // ^C
    case 27: { // escape
      sendIt = false; // finished composing -- don't send
      break;
    }
    case 4: // ^D
    case 13: { // return
      sendIt = true;
      break;
    }
    default: {
      return false;
    }
  }

  if (sendIt) {
    std::string message = hud->getComposeString();
    if (message.length() > 0) {
      const char* cmd = message.c_str();
      if (!LocalCommand::execute(cmd)) {
        if (!myTank) {
          std::string msg = std::string("unknown local command: ") + cmd;
          controlPanel->addMessage(msg, ControlPanel::MessageCurrent);
        }
        else if (serverLink) {
          char messageBuffer[MessageLen];
          memset(messageBuffer, 0, MessageLen);
          strncpy(messageBuffer, message.c_str(), MessageLen);
          serverLink->sendMessage(msgDestination, messageBuffer);
        }
      }

      // record message in history
      const size_t mhLen = messageHistory.size();
      size_t i;
      for (i = 0; i < mhLen; i++) {
	if (messageHistory[i] == message) {
	  messageHistory.erase(messageHistory.begin() + i);
	  messageHistory.push_front(message);
	  break;
	}
      }
      if (i == mhLen) {
	if (mhLen >= MAX_MESSAGE_HISTORY) {
	  messageHistory.pop_back();
	}
	messageHistory.push_front(message);
      }
    }
  }

  messageHistoryIndex = 0;
  hud->setComposing(std::string());
  HUDui::setDefaultKey(NULL);
  return true;
}


bool ComposeDefaultKey::keyRelease(const BzfKeyEvent& key)
{
  LocalPlayer* myTank = LocalPlayer::getMyTank();
  if (!myTank ||
      (myTank->getInputMethod() != LocalPlayer::Keyboard) ||
      (myTank->getTeam() == ObserverTeam)) {
    if (key.button == BzfKeyEvent::Up) {
      if (messageHistoryIndex < messageHistory.size()) {
	hud->setComposeString(messageHistory[messageHistoryIndex]);
	messageHistoryIndex++;
      } else {
	hud->setComposeString(std::string());
      }
      return true;
    } else if (key.button == BzfKeyEvent::Down) {
      if (messageHistoryIndex > 0) {
	messageHistoryIndex--;
	hud->setComposeString(messageHistory[messageHistoryIndex]);
      } else {
	hud->setComposeString(std::string());
      }
      return true;
    }
    else if (myTank && ((key.shift == BzfKeyEvent::ShiftKey
			 || (hud->getComposeString().length() == 0)) &&
			(key.button == BzfKeyEvent::Left
			 || key.button == BzfKeyEvent::Right))) {
      // exclude robot from private message recipient.
      // No point sending messages to robot (now)
      selectNextRecipient(key.button != BzfKeyEvent::Left, false);
      const Player *recipient = myTank->getRecipient();
      if (recipient) {
	msgDestination = recipient->getId();
	std::string composePrompt = "Send to ";
	composePrompt += recipient->getCallSign();
	composePrompt += ": ";
	hud->setComposing(composePrompt);
      }
      return false;
    }
  }

  if ((key.chr == 4) || // ^D
      (key.chr == 6) || // ^F
      (key.chr == 13) || // return
      isWordCompletion(key)) {
    return true;
  }

  return keyPress(key);
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
