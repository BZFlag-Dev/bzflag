/* bzflag
 * Copyright (c) 1993 - 2005 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* interface header */
#include "ComposeDefaultKey.h"

/* system headers */
#include <iostream>
#include <vector>
#include <string>

/* common implementation headers */
#include "BzfEvent.h"
#include "RemotePlayer.h"
#include "KeyManager.h"
#include "AutoCompleter.h"
#include "BZDBCache.h"
#include "AnsiCodes.h"
#include "TextUtils.h"
#include "CommandsStandard.h"

/* local implementation headers */
#include "LocalPlayer.h"
#include "World.h"
#include "HUDRenderer.h"
#include "Roster.h"


/* FIXME -- pulled from player.h */
void addMessage(const Player* player, const std::string& msg, int mode = 3,
		bool highlight=false, const char* oldColor=NULL);
extern char messageMessage[PlayerIdPLen + MessageLen];
#define MAX_MESSAGE_HISTORY (20)
extern HUDRenderer *hud;
extern ServerLink*	serverLink;
extern DefaultCompleter completer;
void selectNextRecipient (bool forward, bool robotIn);

static bool foundVarDiff = false;


MessageQueue	messageHistory;
unsigned int	messageHistoryIndex = 0;


void printout(const std::string& name, void*)
{
  std::cout << name << " = " << BZDB.get(name) << std::endl;
}


static bool varIsEqual(const std::string& name)
{
  // avoid "poll"
  if (name[0] != '_') {
    return true;
  }

  // get the parameters
  const std::string exp = BZDB.get(name);
  const std::string defexp = BZDB.getDefault(name);
  const float val = BZDB.eval(name);
  BZDB.set("tmp", defexp); // BZDB.eval() can't take expressions directly
  BZDB.setPersistent("tmp", false);
  const float defval = BZDB.eval("tmp");
  const bool valNaN = !(val == val);
  const bool defNaN = !(defval == defval);

  if (valNaN != defNaN) {
    return false;
  }

  if (valNaN) {
    return (exp == defexp);
  } else {
    return (val == defval);
  }
}


static void listSetVars(const std::string& name, void* boolPtr)
{
  bool& diff = *((bool*)boolPtr);

  if (diff) {
    if (varIsEqual(name)) {
      return;
    } else {
      foundVarDiff = true;
    }
  }

  char message[MessageLen];
  if (BZDB.getPermission(name) == StateDatabase::Locked) {
    if (BZDBCache::colorful) {
      sprintf(message, "/set %s%s %s%f %s%s",
	      ColorStrings[RedColor].c_str(), name.c_str(),
	      ColorStrings[GreenColor].c_str(), BZDB.eval(name),
	      ColorStrings[BlueColor].c_str(), BZDB.get(name).c_str());
    } else {
      sprintf(message, "/set %s <%f> %s", name.c_str(),
	      BZDB.eval(name), BZDB.get(name).c_str());
    }
    addMessage(LocalPlayer::getMyTank(), message, 2);
  }
}


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
      if (strncmp(cmd, "SILENCE", 7) == 0) {
	Player *loudmouth = getPlayerByName(cmd + 8);
	if (loudmouth) {
	  silencePlayers.push_back(cmd + 8);
	  std::string silenceMessage = "Silenced ";
	  silenceMessage += (cmd + 8);
	  addMessage(NULL, silenceMessage);
	}
      } else if (strncmp(cmd, "DUMP", 4) == 0) {
	BZDB.iterate(printout, NULL);
      } else if (strncmp(cmd, "UNSILENCE", 9) == 0) {
	Player *loudmouth = getPlayerByName(cmd + 10);
	if (loudmouth) {
	  std::vector<std::string>::iterator it = silencePlayers.begin();
	  for (; it != silencePlayers.end(); it++) {
	    if (*it == cmd + 10) {
	      silencePlayers.erase(it);
	      std::string unsilenceMessage = "Unsilenced ";
	      unsilenceMessage += (cmd + 10);
	      addMessage(NULL, unsilenceMessage);
	      break;
	    }
	  }
	}
      } else if (message == "CLIENTQUERY") {
	message = "/clientquery";

	char messageBuffer[MessageLen];
	memset(messageBuffer, 0, MessageLen);
	strncpy(messageBuffer, message.c_str(), MessageLen);
	nboPackString(messageMessage + PlayerIdPLen, messageBuffer, MessageLen);
	serverLink->send(MsgMessage, sizeof(messageMessage), messageMessage);

      } else if (strncmp(cmd, "SAVEWORLD", 9) == 0) {
	std::string path = cmd + 10;
	if (World::getWorld()->writeWorld(path)) {
	  addMessage(NULL, "World Saved");
	} else {
	  addMessage(NULL, "Invalid file name specified");
	}
      } else if (strncasecmp(cmd, "/highlight", 10) == 0) {
        const char* c = cmd + 10;
        while ((*c != '\0') && isspace(*c)) c++; // skip leading white
        BZDB.set("highlightPattern", std::string(c));
      } else if (message == "/set") {
	bool diff = false;
	BZDB.iterate(listSetVars, &diff);
      } else if (message == "/diff") {
	bool diff = true;
	foundVarDiff = false;
	BZDB.iterate(listSetVars, &diff);
	if (!foundVarDiff) {
	  addMessage(LocalPlayer::getMyTank(), "all variables are at defaults", 2);
	}
#ifdef DEBUG
      } else if (strncmp(cmd, "/localset", 9) == 0) {
	std::string params = cmd + 9;
	std::vector<std::string> tokens =
	  TextUtils::tokenize(params, " ", 2);
	if (tokens.size() == 2) {
	  if (!(BZDB.getPermission(tokens[0]) == StateDatabase::Server)) {
	    BZDB.setPersistent(tokens[0], BZDB.isPersistent(tokens[0]));
	    BZDB.set(tokens[0], tokens[1]);
	    std::string msg = "/localset " + tokens[0] + " " + tokens[1];
	    addMessage(NULL, msg);
	  } else {
	    addMessage(NULL, "This is a server-defined variable.  Use /set instead of /localset.");
	  }
	} else {
	  addMessage(NULL, "usage: /localset <variable> <value>");
	}
#endif
      } else if (strncmp(message.c_str(), "/quit", 5) == 0 ) {

	char messageBuffer[MessageLen]; // send message
	memset(messageBuffer, 0, MessageLen);
	strncpy(messageBuffer, message.c_str(), MessageLen);
	nboPackString(messageMessage + PlayerIdPLen, messageBuffer, MessageLen);
	serverLink->send(MsgMessage, sizeof(messageMessage), messageMessage);

	CommandsStandard::quit(); // kill client

      } else if (serverLink) {
	char messageBuffer[MessageLen];
	memset(messageBuffer, 0, MessageLen);
	strncpy(messageBuffer, message.c_str(), MessageLen);
	nboPackString(messageMessage + PlayerIdPLen, messageBuffer, MessageLen);
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
  return keyPress(key);
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
