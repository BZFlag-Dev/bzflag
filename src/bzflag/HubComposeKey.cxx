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

// interface header
#include "HubComposeKey.h"

// system headers
#include <string>
#include <set>

// common headers
#include "BzfEvent.h"
#include "KeyManager.h"

// local headers
#include "LocalPlayer.h"
#include "HUDRenderer.h"
#include "HubLink.h"
#include "ControlPanel.h"
#include "LocalCommand.h"
#include "guiplaying.h"
#include "playing.h"
#include "HUDui.h"


#define MAX_MESSAGE_HISTORY (20)


static const std::string hubPrefix = "/hub ";


void HubComposeKey::init(bool keep)
{
  keepAlive = keep;
  messageHistoryIndex = 0;
  hud->setComposing("HUB:");
  HUDui::setDefaultKey(this);
  if (hubLink) {
    hubLink->startComposing();
  }
}


static bool isWordCompletion(const BzfKeyEvent& key)
{
  if ((key.chr == 6) || // ^F
      (key.chr == 9) || // <TAB>
      ((key.shift == 0) && (key.button == BzfKeyEvent::F2))) {
    return true;
  } else {
    return false;
  }
}


bool HubComposeKey::keyPress(const BzfKeyEvent& key)
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
       myTank->isObserver())) {
    if ((key.button == BzfKeyEvent::Up) ||
	(key.button == BzfKeyEvent::Down)) {
      return true;
    }
  }

  if (isWordCompletion(key)) {
    const std::string line = hud->getComposeString();
    std::set<std::string> partials;

    AutoCompleter ac;
    ac.registerWord("/status");
    ac.registerWord("/connect");
    ac.registerWord("/disconnect");

    // use a custom AutoCompleter for local variables
    ac.complete(line, partials);
    if (hubLink) {
      const std::string prefixLine = hubPrefix + line;
      hubLink->wordComplete(prefixLine, partials);
    }

    if (!partials.empty()) {
      // find the longest common string
      const std::string first = *(partials.begin());
      const std::string last = *(partials.rbegin());
      const size_t minLen = std::min(first.size(), last.size());
      size_t i;
      for (i = 0; i < minLen; i++) {
        if (first[i] != last[i]) {
          break;
        }
      }
      const std::string longest = first.substr(0, i);
      hud->setComposeString(line + longest);

      if (partials.size() >= 2) {
        const size_t lastSpace = line.find_last_of(" \t");
        const std::string lastWord = (lastSpace == std::string::npos) ?
                                     line : line.substr(lastSpace + 1);
        std::string matches;
        std::set<std::string>::const_iterator it;
        for (it = partials.begin(); it != partials.end(); ++it) {
          matches += "  ";
          matches += lastWord + longest + it->substr(longest.size());
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
      if (message == "/connect") {
        delete hubLink;
        hubLink = new HubLink(BZDB.get("hubServer"));
      }
      else if (message == "/disconnect") {
        delete hubLink;
        hubLink = NULL;
      }
      else if (message == "/status") {
        addMessage(NULL, hubLink ? "hublink active" : "hublink inactive");
      }
      else if (message == "/closetabs") {
        if (hubLink) {
          addMessage(NULL, "hublink is active, can not close its tabs");
        }
        else if (controlPanel) {
          for (int i = controlPanel->getTabCount() - 1; i >= 0; i--) {
            controlPanel->removeTab(controlPanel->getTabLabel(i));
          }
        }
      }
      else if (hubLink == NULL) {
        controlPanel->addMessage("not connected to the HUB", ControlPanel::MessageCurrent);
      }
      else {
        hubLink->recvCommand(hubPrefix + message);
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


  if (keepAlive && sendIt) {
    init(true);
  }
  else {
    messageHistoryIndex = 0;
    hud->setComposing("");
    HUDui::setDefaultKey(NULL);
  }

  return true;
}


bool HubComposeKey::keyRelease(const BzfKeyEvent& key)
{
  LocalPlayer* myTank = LocalPlayer::getMyTank();

  if (myTank && KEYMGR.get(key, true) == "jump" && BZDB.isTrue("jumpTyping")) {
    // jump while typing
    myTank->setJump();
    return false;
  }

  if (!myTank || myTank->isObserver() ||
      (myTank->getInputMethod() != LocalPlayer::Keyboard)) {
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
