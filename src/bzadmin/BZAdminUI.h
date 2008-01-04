/* bzflag
 * Copyright (c) 1993 - 2007 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef BZADMINUI_H
#define BZADMINUI_H

#include "common.h"

/* system interface headers */
#include <string>

/* common interface headers */
#include "Address.h"
#include "colors.h"
#include "global.h"

//#include "BZAdminClient.h"
class BZAdminClient;

/** This class is an abstract base class for all bzadmin interfaces.
    All subclasses should register themselves in the UIMap. This should
    be done in the same files as the subclass is defined in, and it should
    be done before the main() function is called (i.e. when global variables
    and static member variables are initialized). This can be done using
    UIAdder. */
class BZAdminUI {
public:

  /** This constructor just sets the BZAdminClient reference. */
  BZAdminUI(BZAdminClient& c) : client(c) { }

  /** Need a virtual destructor so subclasses get to do their cleanups. */
  virtual ~BZAdminUI() { }

  /** This function prints the message. */
  virtual void outputMessage(const std::string&, ColorCode) { }
  /** This function is called by the client when a new packet has arrived. */
  virtual void handleNewPacket(uint16_t);
  /** See if the user has entered a command, if it has, store it in str and
      return true. */
  virtual bool checkCommand(std::string&) { return false; }
  /** Tell the UI that a player has been added. */
  virtual void addedPlayer(PlayerId) { }
  /** Warn the UI that a player will be removed. */
  virtual void removingPlayer(PlayerId) { }
  /** Get the current target (the player that messages should be sent to,
      or 0 for public messages). */
  virtual PlayerId getTarget() const { return AllPlayers; }

protected:

  BZAdminClient& client;
};

#endif

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
