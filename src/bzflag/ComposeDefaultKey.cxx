/* bzflag
 * Copyright (c) 1993 - 2005 Tim Riker
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

/* common implementation headers */
#include "BzfEvent.h"
#include "KeyManager.h"
#include "AutoCompleter.h"

/* local implementation headers */
#include "LocalPlayer.h"
#include "HUDRenderer.h"
#include "LocalCommand.h"


/* FIXME -- pulled from player.h */
void addMessage(const Player* player, const std::string& msg, int mode = 3,
		bool highlight=false, const char* oldColor=NULL);
extern char messageMessage[PlayerIdPLen + MessageLen];
#define MAX_MESSAGE_HISTORY (20)
extern HUDRenderer *hud;
extern ServerLink*	serverLink;
extern DefaultCompleter completer;
void selectNextRecipient (bool forward, bool robotIn);

MessageQueue	messageHistory;
unsigned int	messageHistoryIndex = 0;



bool			ComposeDefaultKey::keyPress(const BzfKeyEvent& key)
{
  bool sendIt;
  LocalPlayer *myTank = LocalPlayer::getMyTank();
  if (myTank && KEYMGR.get(key, true) == "jump" && BZDB.isTrue("jumpTyping")) {
    // jump while typing
    myTank->setJump();
    return false;
  }

  if (!myTank || myTank->getInputMethod() != LocalPlayer::Keyboard) {
    if ((key.button == BzfKeyEvent::Up) ||
	(key.button == BzfKeyEvent::Down))
      return true;
  }

  switch (key.ascii) {
    case 3:	// ^C
    case 27: {	// escape
      sendIt = false;			// finished composing -- don't send
      break;
    }
    case 4:	// ^D
    case 13: {	// return
      sendIt = true;
      break;
    }
    case 6:     // ^F
    case 9: {	// <tab>
      // auto completion
      std::string line1 = hud->getComposeString();
      int lastSpace = line1.find_last_of(" \t");
      std::string line2 = line1.substr(0, lastSpace+1);
      line2 += completer.complete(line1.substr(lastSpace+1));
      hud->setComposeString(line2);
      return true;
    }
    default: {
      return false;
    }
  }

  if (sendIt) {
    std::string message = hud->getComposeString();
    if (message.length() > 0) {
      const char* cmd = message.c_str();
      if (LocalCommand::execute(cmd)) {
	;
      } else if (serverLink) {
	char messageBuffer[MessageLen];
	memset(messageBuffer, 0, MessageLen);
	strncpy(messageBuffer, message.c_str(), MessageLen);
	nboPackString(messageMessage + PlayerIdPLen, messageBuffer,
		      MessageLen);
	serverLink->send(MsgMessage, sizeof(messageMessage), messageMessage);
      }

      // record message in history
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
  if (!myTank || myTank->getInputMethod() != LocalPlayer::Keyboard) {
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
    else if (myTank && ((key.shift == BzfKeyEvent::ShiftKey
			 || (hud->getComposeString().length() == 0)) &&
			(key.button == BzfKeyEvent::Left
			 || key.button == BzfKeyEvent::Right))) {
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
    else if ((key.shift == 0) && (key.button == BzfKeyEvent::F2)) {
      // auto completion  (F2)
      std::string line1 = hud->getComposeString();
      int lastSpace = line1.find_last_of(" \t");
      std::string line2 = line1.substr(0, lastSpace+1);
      line2 += completer.complete(line1.substr(lastSpace+1));
      hud->setComposeString(line2);
    }
  }
  
  if ((key.ascii == 4) || // ^D
      (key.ascii == 13)) { // return
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
