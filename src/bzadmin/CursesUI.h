#ifndef CURSESUI_H
#define CURSESUI_H

#include <iostream>
#include <map>
#include <string>

#include <curses.h>

#include "Address.h"
#include "AutoCompleter.h"
#include "BZAdminUI.h"
#include "global.h"

#define CMDLENGTH (MessageLen - 3)

using namespace std;


/** This class is an interface for bzadmin that uses ncurses. */
class CursesUI : public BZAdminUI {
public:
  
  /** The parameters to this constructor are a map of all players and the
      local player's PlayerId. */
  CursesUI(map<PlayerId, string>& p, PlayerId m);
  
  ~CursesUI();
  
  /** This function prints a message in the main window. */
  virtual void outputMessage(const string& msg);
  
  /** See if the user has entered a command, if it has, store it in str and
      return true. */
  virtual bool checkCommand(string& str);
  
  /** Tell the UI that a player has been added. */
  virtual void addedPlayer(PlayerId p);
  
  /** Warn the UI that a player will be removed. */
  virtual void removingPlayer(PlayerId p);
  
  /** Get the current target (the player that messages should be sent to,
      or 0 for public messages). */
  virtual PlayerId getTarget() const;
  
protected:

  void updateTargetWin();
  void updateCmdWin();
  
  WINDOW* mainWin;
  WINDOW* targetWin;
  WINDOW* cmdWin;
  string cmd;
  map<PlayerId, string>& players;
  map<PlayerId, string>::const_iterator targetIter;  
  PlayerId me;
  AutoCompleter comp;
  vector<string> history;
  unsigned int maxHistory;
  unsigned int currentHistory;
};

#endif
