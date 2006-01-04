/* bzflag
 * Copyright (c) 1993 - 2006 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef __FILTER_H__
#define __FILTER_H__

// bzflag global header
#include "common.h"

// System headers
#include <vector>
#include <string>

// implementation-specific bzfs-specific headers
#include "GameKeeper.h"

class Filter {
 public:
  enum Action {ACCEPT, DROP, STOP};

  Filter();

  /** This function loads a ban filter from a file, if it has been set. */
  void load();
  /** This function clear the filter list */
  void clear();
  /** This function check if a player has to be accepted or dropped.
      player is the Player to match,
      index is the rule index, starting from 0
      Return the action to be performed on match, or end */
  Action check(GameKeeper::Player &player, int &index);
private:
  struct FilterItem {
    int	 hostId;
    int	 netMask;
    std::string principal;
    Action      action;
  };
  std::vector<FilterItem> filterList;
};

#endif /* __FILTER_H__ */

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

