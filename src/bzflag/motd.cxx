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

#ifdef _MSC_VER
#pragma warning( 4: 4786)
#endif

// interface header
#include "motd.h"

#include <vector>
#include <string>

#include "URLManager.h"
#include "TextUtils.h"

MessageOfTheDay::MessageOfTheDay()
{
}

MessageOfTheDay::~MessageOfTheDay()
{
}

void MessageOfTheDay::getURL(const std::string URL)
{
  messages.clear();

  // get all up on the internet and go get the thing
  if (!URLManager::instance().getURL(URL,data)) {
    data = "Default MOTD";
  } else {
    // trim trailing whitespace
    int l = data.size() - 1;
    while (TextUtils::isWhitespace(data[l])) {
      data.erase(l, 1);
      l--;
    }

    // parse into messages
    std::vector<std::string> lines = TextUtils::tokenize(data, "\n");
    if (((float)lines.size() / 4) != (floorf((float)lines.size() / 4))) {
      data = "MOTD contains unexpected data";
    } else {
      for (unsigned int i = 0; i < lines.size(); ++i) {
	MOTD_message msg;
	msg.title = lines[i++];
	msg.date = lines[i++];
	msg.text = lines[i++];
	msg.version = lines[i].substr(lines[i].find(':') + 2);
	messages.push_back(msg);
      }
    }
  }

  if (messages.size() == 0) {
    MOTD_message msg;
    msg.text = data;
    messages.push_back(msg);
  }
}

std::vector<std::string> MessageOfTheDay::getPrintable(const std::vector<std::string>& matchVersions)
{
  std::vector<std::string> retval;
  unsigned int i, j;
  for (i = 0; i < messages.size(); ++i) {
    if (matchVersions.size() == 0) {
      retval.push_back(messages[i].text);
    } else {
      for (j = 0; j < matchVersions.size(); ++j) {
	if (messages[i].version == matchVersions[j]) {
	  retval.push_back(messages[i].text);
	}
      }
    }
  }
  return retval;
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
