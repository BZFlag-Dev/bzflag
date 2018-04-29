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

// interface header
#include "motd.h"

// system headers
#include <vector>

// common implementation headers
#include "TextUtils.h"
#include "AnsiCodes.h"
#include "version.h"

// local implementation headers
#include "playing.h"
#include "NewVersionMenu.h"
#include "HUDDialogStack.h"

typedef struct {
  std::string title;
  std::string date;
  std::string text;
  std::string version;
} MOTD_message;

static NewVersionMenu* nvMenu;

void MessageOfTheDay::finalization(char *_data, unsigned int length, bool good)
{
  unsigned int i;
  unsigned int j;

  std::vector<MOTD_message> messages;

  if (good) {
    std::string data(_data, length);

    // parse into messages
    std::vector<std::string> lines = TextUtils::tokenize(data, "\n");
    if (lines.size() % 4) {
      data = "MOTD contains unexpected data";
    } else {
      for (i = 0; i < lines.size(); ++i) {
	MOTD_message msg;
	msg.title   = lines[i++];
	msg.date    = lines[i++];
	msg.text    = lines[i++];
	msg.version = lines[i].substr(lines[i].find(':') + 2);
	// backwards compatibility - if it's not a regex already, go by our old rules
	if (isdigit(msg.version[0])) {
	  if (msg.version == "0.0") {
	    msg.version = "[a-z0-9\\.-]*";
	  } else {
	    msg.version.insert(0, "^");
	    msg.version += "[a-z0-9\\.-]*";
	  }
	}
	messages.push_back(msg);
      }
    }
    if (messages.empty()) {
      MOTD_message msg;
      msg.text = data;
      messages.push_back(msg);
    }
  } else {
    MOTD_message msg;
    msg.text    = "<not available>";
    msg.version = "[a-z0-9\\.-]*";
    messages.push_back(msg);
  }

  regex_t re;

  std::vector<std::string> msgs;
  for (i = 0; i < messages.size(); ++i) {
    if (regcomp(&re, messages[i].version.c_str(), REG_EXTENDED | REG_ICASE) == 0) {
      if (regexec(&re, getAppVersion(), 0, NULL, 0) == 0) {
	if (messages[i].title.substr(0, 9) == "UPGRADE: ") {
	  // new version released.  handle appropriately.
	  std::string announce = messages[i].title.substr(9, messages[i].title.length() - 9);
	  nvMenu = new NewVersionMenu(announce, messages[i].text, messages[i].date);
	  HUDDialogStack::get()->push(nvMenu);
	} else {
	  // standard MOTD
	  msgs.push_back(messages[i].text);
	}
      }
      regfree(&re);
    }
  }
  controlPanel->addMessage(ColorStrings[UnderlineColor]
			   + ColorStrings[WhiteColor]
			   + "Message of the day: ");
  for (j = 0; j < msgs.size(); ++j)
    controlPanel->addMessage(ColorStrings[WhiteColor] + "* " + msgs[j]);
}


void MessageOfTheDay::getURL(const std::string &URL)
{
  // get all up on the internet and go get the thing
  setURL(URL);
  addHandle();
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
