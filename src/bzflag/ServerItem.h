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

#ifndef  __SERVERITEM_H__
#define  __SERVERITEM_H__

/* system interface headers */
#include <iostream>
#include <string>
#include <time.h>

/* common interface headers */
#include "Ping.h"


class ServerItem {
public:
  void		writeToFile(std::ostream& out) const; // serialize out
  bool		readFromFile(std::istream& in); // serialize in
  void		setUpdateTime(); // set last updated to now
  int		getPlayerCount() const;
  time_t	getAgeMinutes() const;
  time_t	getAgeSeconds() const;
  std::string	getAgeString() const; // nifty formated age string
  time_t	getNow() const; // current time
  bool		operator<(const ServerItem &right);
public:
  std::string	name;
  std::string	description;
  PingPacket	ping;
  bool		cached; // was I cached ?
  time_t	updateTime; // last time I was updated
};

#endif /* __SERVERITEM_H__ */

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
