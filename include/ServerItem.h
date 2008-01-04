/* bzflag
 * Copyright (c) 1993 - 2008 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef  __SERVERITEM_H__
#define  __SERVERITEM_H__

#include "common.h"

/* system interface headers */
#include <iostream>
#include <string>
#include <time.h>

/* common interface headers */
#include "Ping.h"


class ServerItem {

public:
  ServerItem();
  void		writeToFile(std::ostream& out) const; // serialize out
  bool		readFromFile(std::istream& in); // serialize in
  void		resetAge(); // set last updated to now
  void		setAge(time_t minutes, time_t seconds);
  int		getPlayerCount() const;
  std::string	getAddrName() const;
  time_t	getAgeMinutes() const;
  time_t	getAgeSeconds() const;
  std::string	getAgeString() const; // nifty formated age string
  time_t	getNow() const; // current time
  unsigned int	getSortFactor() const;

public:
  std::string	name;
  unsigned int	port;
  std::string	description;
  PingPacket	ping;
  time_t	updateTime; // last time I was updated
  bool		cached;     // was I cached ?
  bool		favorite;   // favorite server, user selection
};

bool	operator<(const ServerItem &left, const ServerItem &right);

#endif /* __SERVERITEM_H__ */

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
