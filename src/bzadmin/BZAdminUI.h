/* bzflag
 * Copyright (c) 1993 - 2003 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef BZADMINUI_H
#define BZADMINUI_H

#include <string>

#include "Address.h"
#include "global.h"

using namespace std;


/** This class is an abstract base class for all bzadmin interfaces. */
class BZAdminUI {
public:

  /** Need a virtual destructor so subclasses get to do their cleanups. */
  virtual ~BZAdminUI() { }

  /** This function prints a message in the main window. */
  virtual void outputMessage(const string&) { }
  /** See if the user has entered a command, if it has, store it in str and
      return true. */
  virtual bool checkCommand(string&) { return false; }
  /** Tell the UI that a player has been added. */
  virtual void addedPlayer(PlayerId) { }
  /** Warn the UI that a player will be removed. */
  virtual void removingPlayer(PlayerId) { }
  /** Get the current target (the player that messages should be sent to,
      or 0 for public messages). */
  virtual PlayerId getTarget() const { return AllPlayers; }

};

#endif
