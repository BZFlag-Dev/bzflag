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

/* class interface header */
#include "Filter.h"

// bzflag common header
#include "TextUtils.h"
#include "network.h"

Filter::Filter()
{
  load();
}

void Filter::load()
{
  FilterItem	       filterItem;
  std::vector<std::string> tokenList;
  std::string	      ip;
  char		     row[256];
  int		      sep;
  struct in_addr	   addr;

  // try to open the filter file
  std::ifstream is("filter.txt");

  if (!is.good())
    // file does not exist, but that's OK, we'll create it later if needed
    return;

  // clear all current bans
  clear();

  // try to read filter entries
  while (!is.eof()) {
    is.getline(row, sizeof(row));
    tokenList = TextUtils::tokenize(row, " ", 3, false);
    ip		   = tokenList[0];
    sep		  = ip.find("/");

    inet_aton(ip.substr(sep + 1, ip.size()).c_str(), &addr);
    filterItem.netMask   = addr.s_addr;

    inet_aton(ip.substr(0, sep - 1).c_str(), &addr);
    filterItem.hostId    = addr.s_addr & filterItem.netMask;

    filterItem.principal = tokenList[1];
    if (tokenList[2] == "ACCEPT")
      filterItem.action = ACCEPT;
    else if (tokenList[2] == "DROP")
      filterItem.action = DROP;
    filterList.push_back(filterItem);
  }
}

void Filter::clear()
{
  filterList.clear();
}

Filter::Action Filter::check(GameKeeper::Player &player, int &index)
{
  Action	 action = STOP;
  FilterItem     filterItem;
  int	    addr = player.netHandler->getIPAddress().s_addr;

  for (unsigned int i = index; i < filterList.size(); i++) {
    filterItem   = filterList[i];
    bool anyName = filterItem.principal == "_any_";
      if ((filterItem.principal != "_unregistered_") && !anyName)
	continue;
    if ((addr & filterItem.netMask) != filterItem.hostId)
      continue;
    action = filterItem.action;
    break;
  }
  return action;
}

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

