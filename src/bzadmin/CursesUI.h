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

#ifndef CURSESUI_H
#define CURSESUI_H

#include <iostream>
#include <map>
#include <string>

#include "config.h"

// which curses?
#ifdef HAVE_CURSES_H
#include <curses.h>
#else
#ifdef HAVE_NCURSES_H
#include <ncurses.h>
#else
#include "pdcurses_adapter.h"
#endif
#endif

#include "Address.h"
#include "AutoCompleter.h"
#include "BZAdminUI.h"
#include "CursesMenu.h"
#include "global.h"
#include "UIMap.h"

#define CMDLENGTH (MessageLen - 3)


/** This class is an interface for bzadmin that uses ncurses. */
class CursesUI : public BZAdminUI {
protected:

  /** The parameters to this constructor are a map of all players and the
      local player's PlayerId. */
  CursesUI(const PlayerIdMap& p, PlayerId m);

public:

  ~CursesUI();

  /** This function prints a message in the main window. */
  virtual void outputMessage(const std::string& msg);

  /** See if the user has entered a command, if it has, store it in str and
      return true. */
  virtual bool checkCommand(std::string& str);

  /** Tell the UI that a player has been added. */
  virtual void addedPlayer(PlayerId p);

  /** Warn the UI that a player will be removed. */
  virtual void removingPlayer(PlayerId p);

  /** Get the current target (the player that messages should be sent to,
      or 0 for public messages). */
  virtual PlayerId getTarget() const;

  /** This function returns a pointer to a dynamically allocated
      CursesUI object. */
  static BZAdminUI* creator(const PlayerIdMap& players, PlayerId me);

protected:

  /** This function moves and resizes the windows when the terminal has been
      resized. */
  void handleResize(int lines, int cols);

  /** All messages that are written in the main window are saved in a buffer.
      This function updates the main window with the contents of that buffer.
      It is useful to do this when the window has been resized (because
      the terminal has been resized, or because the menu has been toggled).
      @param numberOfMessages The last @c numberOfMessages messages from the
                              buffer will be written to the window.
  */
  void updateMainWinFromBuffer(unsigned int numberOfMessages);
  
  /** This function redraws the target window (the line that says who you are
      talking to). */
  void updateTargetWin();

  /** This function redraws the command window (the line where you type your
      messages). */
  void updateCmdWin();

  /** This function toggles the visibility of the menu window. */
  void toggleMenu();
  
  /** This function sets the menu to the main menu. */
  static void initMainMenu(CursesMenu& menu);
  
  /** This function sets the menu to the player menu. */
  static void initPlayerMenu(CursesMenu& menu);
  
  /** This function sets the menu to the ban menu. */
  static void initBanMenu(CursesMenu& menu);
  
  /** This function sets the menu to the "Set server variables" submenu. */
  static void initServerVarMenu(CursesMenu& menu);
  
  /** Add a single BZDBCMItem to the menu. */
  static void addBZDBCMItem(const std::string& name, void* menu);
  
  /** This function sets the menu to the filter menu. */
  static void initFilterMenu(CursesMenu& menu);

  WINDOW* mainWin;
  WINDOW* targetWin;
  WINDOW* cmdWin;
  WINDOW* menuWin;
  
  // 0 = no menu, 1 = menu is visible and handles keystrokes, 2 = menu is
  // visible but command prompt handles keystrokes
  int menuState;
  CursesMenu menu;
  
  std::string cmd;
  const PlayerIdMap& players;
  PlayerIdMap::const_iterator targetIter;
  PlayerId me;
  AutoCompleter comp;
  std::vector<std::string> history;
  unsigned int maxHistory;
  unsigned int currentHistory;
  std::vector<std::string> msgBuffer;
  unsigned int maxBufferSize;
  unsigned int scrollOffset;
  
  static UIAdder uiAdder;
};

#endif

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
