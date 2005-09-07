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

#ifdef _MSC_VER
#pragma warning( 4: 4786)
#endif

// interface header
#include "motd.h"

// system headers
#include <vector>

// common implementation headers
#include "TextUtils.h"
#include "AnsiCodes.h"
#include "version.h"

// local implementation headers
#include "ControlPanel.h"

extern ControlPanel* controlPanel;

typedef struct {
  std::string title;
  std::string date;
  std::string text;
  std::string version;
} MOTD_message;

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
	messages.push_back(msg);
      }
    }
    if (messages.size() == 0) {
      MOTD_message msg;
      msg.text = data;
      messages.push_back(msg);
    }
  } else {
    MOTD_message msg;
    msg.text    = "<not available>";
    msg.version = "0.0";
    messages.push_back(msg);
  }

  std::vector<std::string> versions;
  versions.push_back("0.0");
  versions.push_back(getMajorMinorVersion());
  versions.push_back(getMajorMinorRevVersion());

  std::vector<std::string> msgs;
  for (i = 0; i < messages.size(); ++i)
    for (j = 0; j < versions.size(); ++j)
      if (messages[i].version == versions[j])
	msgs.push_back(messages[i].text);

  controlPanel->addMessage(ColorStrings[UnderlineColor]
			   + ColorStrings[WhiteColor]
			   + "Message of the day: ");
  for (j = 0; j < msgs.size(); ++j)
    controlPanel->addMessage(ColorStrings[WhiteColor] + "* " + msgs[j]);
}

void MessageOfTheDay::getURL(const std::string URL)
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
