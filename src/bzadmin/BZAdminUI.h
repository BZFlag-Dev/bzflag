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
  virtual PlayerId getTarget() const { return PlayerId(); }

};

#endif
