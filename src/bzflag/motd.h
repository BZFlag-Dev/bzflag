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


#ifndef __MOTD_H__
#define __MOTD_H__

// bzflag global header
#include "global.h"
#include <string>
#include <vector>

typedef struct {
  std::string title;
  std::string date;
  std::string text;
  std::string version;
} MOTD_message;

class MessageOfTheDay {
public:
	MessageOfTheDay();
	~MessageOfTheDay();

	void			  getURL(const std::string URL);

	std::vector<MOTD_message> getMessages() { return messages; };
	std::vector<std::string>  getPrintable(const std::vector<std::string>& matchVersions);

private:
	std::string		  data;
	std::vector<MOTD_message> messages;
};
#endif //__MOTD_H__


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
