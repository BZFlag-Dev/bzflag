/* bzflag
 * Copyright (c) 1993 - 2004 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#include "common.h"
#include "ComposeDefaultKey.h"
#include "LocalPlayer.h"
#include "RemotePlayer.h"
#include "Roster.h"
#include "KeyManager.h"
#include "HUDRenderer.h"

#include "playing.h" // THIS IS TEMPORARY... WILL BE REMOVED... BABY STEPS

MessageQueue	messageHistory;
unsigned int	messageHistoryIndex = 0;

void printout(const std::string& name, void*)
{
  std::cout << name << " = " << BZDB.get(name) << std::endl;
}

void listSetVars(const std::string& name, void*)
{
  char message[MessageLen];

  if (BZDB.getPermission(name) == StateDatabase::Locked) {
    sprintf(message, "/set %s %f", name.c_str(), BZDB.eval(name));
    addMessage(LocalPlayer::getMyTank(), message, false, NULL);
  }
}

bool			ComposeDefaultKey::keyPress(const BzfKeyEvent& key)
{
  bool sendIt;
  LocalPlayer *myTank = LocalPlayer::getMyTank();
  if (KEYMGR.get(key, true) == "jump") {
    // jump while typing
    myTank->jump();
  }

  if (myTank->getInputMethod() != LocalPlayer::Keyboard) {
    if ((key.button == BzfKeyEvent::Up) ||
	(key.button == BzfKeyEvent::Down))
      return true;
  }

  switch (key.ascii) {
  case 3:	// ^C
  case 27:	// escape
    //    case 127:	// delete
    sendIt = false;			// finished composing -- don't send
    break;

  case 4:	// ^D
  case 13:	// return
    sendIt = true;
    break;

  default:
    return false;
  }

  if (sendIt) {
    std::string message = hud->getComposeString();

    if (message.length() > 0) {
      const char* silence = message.c_str();
      if (strncmp(silence, "SILENCE", 7) == 0) {
	Player *loudmouth = getPlayerByName(silence + 8);
	if (loudmouth) {
	  silencePlayers.push_back(silence + 8);
	  std::string message = "Silenced ";
	  message += (silence + 8);
	  addMessage(NULL, message);
	}
      } else if (strncmp(silence, "DUMP", 4) == 0) {
	BZDB.iterate(printout, NULL);
      } else if (strncmp(silence, "UNSILENCE", 9) == 0) {
	Player *loudmouth = getPlayerByName(silence + 10);
	if (loudmouth) {
	  std::vector<std::string>::iterator it = silencePlayers.begin();
	  for (; it != silencePlayers.end(); it++) {
	    if (*it == silence + 10) {
	      silencePlayers.erase(it);
	      std::string message = "Unsilenced ";
	      message += (silence + 10);
	      addMessage(NULL, message);
	      break;
	    }
	  }
	}
      } else if (strncmp(silence, "SAVEWORLD", 9) == 0) {
	std::string path = silence + 10;
	if (World::getWorld()->writeWorld(path)) {
	  addMessage(NULL, "World Saved");
	} else {
	  addMessage(NULL, "Invalid file name specified");
	}
      } else if (message == "/set") {
	BZDB.iterate(listSetVars, NULL);
      } else {
	int i, mhLen = messageHistory.size();
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

	char messageBuffer[MessageLen];
	memset(messageBuffer, 0, MessageLen);
	strncpy(messageBuffer, message.c_str(), MessageLen);
	nboPackString(messageMessage + PlayerIdPLen, messageBuffer, MessageLen);
	serverLink->send(MsgMessage, sizeof(messageMessage), messageMessage);
      }
    }
  }

  messageHistoryIndex = 0;
  hud->setComposing(std::string());
  HUDui::setDefaultKey(NULL);
  return true;
}

bool			ComposeDefaultKey::keyRelease(const BzfKeyEvent& key)
{
  LocalPlayer *myTank = LocalPlayer::getMyTank();
  if (myTank->getInputMethod() != LocalPlayer::Keyboard) {
    if (key.button == BzfKeyEvent::Up) {
      if (messageHistoryIndex < messageHistory.size()) {
	hud->setComposeString(messageHistory[messageHistoryIndex]);
	messageHistoryIndex++;
      }
      else
	hud->setComposeString(std::string());
      return true;
    }
    else if (key.button == BzfKeyEvent::Down) {
      if (messageHistoryIndex > 0){
	messageHistoryIndex--;
	hud->setComposeString(messageHistory[messageHistoryIndex]);
      }
      else
	hud->setComposeString(std::string());
      return true;
    }
    else if ((key.shift == BzfKeyEvent::ShiftKey || (hud->getComposeString().length() == 0)) &&
	     (key.button == BzfKeyEvent::Left || key.button == BzfKeyEvent::Right)) {
      // exclude robot from private message recipient.
      // No point sending messages to robot (now)
      selectNextRecipient(key.button != BzfKeyEvent::Left, false);
      const Player *recipient = myTank->getRecipient();
      if (recipient) {
	void* buf = messageMessage;
	buf = nboPackUByte(buf, recipient->getId());
	std::string composePrompt = "Send to ";
	composePrompt += recipient->getCallSign();
	composePrompt += ": ";
	hud->setComposing(composePrompt);
      }
      return false;
    }
  }
  return keyPress(key);
}
