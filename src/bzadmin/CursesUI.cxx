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

#ifdef _WIN32
#pragma warning( 4: 4786)
#endif

#include "CursesUI.h"


// add this UI to the map
UIAdder CursesUI::uiAdder("curses", &CursesUI::creator);


CursesUI::CursesUI(const std::map<PlayerId, std::string>& p, PlayerId m) :
  players(p), me(m), maxHistory(20), currentHistory(0) {
  
  // initialize ncurses
  initscr();
  nonl();
  cbreak();
  noecho();

  // create main output window
  mainWin = newwin(LINES - 5, 0, 0, 0);
  wsetscrreg(mainWin, 0, LINES - 2);
  scrollok(mainWin, TRUE);
  wrefresh(mainWin);

  // create target window
  targetWin = newwin(1, 0, LINES - 2, 0);
  wattron(targetWin, A_UNDERLINE);
  targetIter = players.begin();
  updateTargetWin();

  // create command window
  cmdWin = newwin(0, 0, LINES - 1, 0);
  keypad(cmdWin, TRUE);
  nodelay(cmdWin, TRUE);
  updateCmdWin();

  // register commands for tab completion
  comp.registerWord("/ban ");
  comp.registerWord("/banlist");
  comp.registerWord("/countdown");
  comp.registerWord("/deregister");
  comp.registerWord("/flag ");
  comp.registerWord("reset");
  comp.registerWord("up");
  comp.registerWord("show");
  comp.registerWord("/flaghistory");
  comp.registerWord("/gameover");
  comp.registerWord("/ghost ");
  comp.registerWord("/groupperms");
  comp.registerWord("/help");
  comp.registerWord("/identify ");
  comp.registerWord("/idlestats");
  comp.registerWord("/kick ");
  comp.registerWord("/lagstats");
  comp.registerWord("/lagwarn ");
  comp.registerWord("/password ");
  comp.registerWord("/playerlist");
  comp.registerWord("/poll ");
  comp.registerWord("ban");
  comp.registerWord("kick");
  comp.registerWord("/quit");
  comp.registerWord("/register ");
  comp.registerWord("/reload");
  comp.registerWord("/removegroup ");
  comp.registerWord("/report ");
  comp.registerWord("/reset");
  comp.registerWord("/set");
  comp.registerWord("/setgroup ");
  comp.registerWord("/setpass ");
  comp.registerWord("/shutdownserver");
  comp.registerWord("/superkill");
  comp.registerWord("/unban ");
  comp.registerWord("/veto");
  comp.registerWord("/vote");
}


CursesUI::~CursesUI() {
  endwin();
}


void CursesUI::outputMessage(const std::string& msg) {
  waddstr(mainWin, (msg + "\n").c_str());
  wrefresh(mainWin);
}


bool CursesUI::checkCommand(std::string& str) {
  wrefresh(cmdWin);
  str = "";
  int i;
  int c = wgetch(cmdWin);
  switch (c) {
  
  case KEY_RESIZE:
    handleResize(LINES, COLS);
    return false;

  case ERR:
    return false;

    // clear command (21 is Ctrl-U)
  case 21:
    cmd = "";
    updateCmdWin();
    currentHistory = history.size();

    // delete last character
  case KEY_BACKSPACE:
  case KEY_DC:
  case 127:
    cmd = cmd.substr(0, cmd.size() - 1);
    updateCmdWin();
    return false;

    // send command
  case '\n': // works with PDCurses
  case 13:   // works with ncurses
    if (history.size() == maxHistory)
      history.erase(history.begin());
    history.push_back(cmd);
    str = cmd;
    cmd = "";
    currentHistory = history.size();
    updateCmdWin();
    return true;

    // scroll main window - doesn't work
  case KEY_NPAGE:
    wscrl(mainWin, 1);
    return false;
  case KEY_PPAGE:
    wscrl(mainWin, -1);
    return false;

    // change target
  case KEY_LEFT:
    if (targetIter == players.begin())
      for (unsigned int i = 0; i < players.size() - 1; i++)
	++targetIter;
    else
      targetIter--;
    updateTargetWin();
    return false;
  case KEY_RIGHT:
    targetIter++;
    if (targetIter == players.end())
      targetIter = players.begin();
    updateTargetWin();
    return false;

    // command history
  case KEY_UP:
    if (currentHistory == 0 || history.size() == 0)
      return false;
    --currentHistory;
    cmd = history[currentHistory];
    updateCmdWin();
    return false;
  case KEY_DOWN:
    if (currentHistory < history.size())
      ++currentHistory;
    if (currentHistory == history.size())
      cmd = "";
    else
      cmd = history[currentHistory];
    updateCmdWin();
    return false;

    // kick target
  case KEY_F(5):
    if (targetIter != players.end() && targetIter->first != me) {
      cmd = "/kick ";
      cmd += targetIter->second;
      targetIter = players.find(me);
      updateCmdWin();
      updateTargetWin();
    }
    return false;

    // tab - autocomplete
  case '\t':
    i = cmd.find_last_of(" \t");
    cmd = cmd.substr(0, i+1) + comp.complete(cmd.substr(i+1));
    updateCmdWin();
    return false;

  default:
    if (c < 32 || c > 127 || cmd.size() >= CMDLENGTH)
      return false;
    cmd += char(c);
    updateCmdWin();
    return false;
  }
}


void CursesUI::addedPlayer(PlayerId p) {
  std::map<PlayerId, std::string>::const_iterator iter = players.find(p);
  comp.registerWord(iter->second);
  if (p == me)
    targetIter == iter;
}


void CursesUI::removingPlayer(PlayerId p) {
  if (targetIter->first == p) {
    targetIter = players.find(me);
    updateTargetWin();
  }
  comp.unregisterWord(players.find(p)->second);
}


PlayerId CursesUI::getTarget() const {
  if (targetIter->first == me)
    return AllPlayers;
  return targetIter->first;
}


void CursesUI::handleResize(int lines, int cols) {
  resize_term(lines, cols);
  mvwin(targetWin, lines - 2, 0);
  wresize(targetWin, 1, cols);
  mvwin(cmdWin, lines - 1, 0);
  wresize(cmdWin, 1, cols);
  wresize(mainWin, lines - 2, cols);
  updateTargetWin();
  updateCmdWin();
  wrefresh(mainWin);
}


void CursesUI::updateTargetWin() {
  wclear(targetWin);
  wmove(targetWin, 0, 0);
  wmove(targetWin, 1, 1);
  std::string tmp = "Send to ";
  tmp = tmp + (targetIter == players.end() || targetIter->first == me ?
	       "all" : targetIter->second) + ":";
  waddstr(targetWin, tmp.c_str());
  wrefresh(targetWin);
}


void CursesUI::updateCmdWin() {
  wclear(cmdWin);
  wmove(cmdWin, 1, 1);
  waddstr(cmdWin, cmd.c_str());
  wmove(cmdWin, 1, 1 + cmd.size());
  wrefresh(cmdWin);
}


BZAdminUI* CursesUI::creator(const std::map<PlayerId, std::string>& players,
			     PlayerId me) {
  return new CursesUI(players, me);
}

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
