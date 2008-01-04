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

#ifdef _MSC_VER
#pragma warning( 4: 4786)
#endif

#include "CursesUI.h"


// add this UI to the map
UIAdder CursesUI::uiAdder("curses", &CursesUI::creator);


CursesUI::CursesUI(BZAdminClient& c) :
  BZAdminUI(c),
  menuState(NoMenu), menu(c), client(c), players(c.getPlayers()),
  me(c.getMyId()), maxHistory(20), currentHistory(0),
  maxBufferSize(300), scrollOffset(0) {

  // initialize ncurses
  initscr();
  start_color();
  use_default_colors();
  init_pair(Default, COLOR_FGDEFAULT, COLOR_BGDEFAULT);
  init_pair(White, COLOR_WHITE, COLOR_BGDEFAULT);
  init_pair(Red, COLOR_RED, COLOR_BGDEFAULT);
  init_pair(Green, COLOR_GREEN, COLOR_BGDEFAULT);
  init_pair(Blue, COLOR_BLUE, COLOR_BGDEFAULT);
  init_pair(Purple, COLOR_MAGENTA, COLOR_BGDEFAULT);
  init_pair(Yellow, COLOR_YELLOW, COLOR_BGDEFAULT);
  init_pair(Cyan, COLOR_CYAN, COLOR_BGDEFAULT);
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

  // initialize the menu
  menu.setUpdateCallback(initMainMenu);

  // add additional chat targets
  additionalTargets[PlayerId(250 - ObserverTeam)] = PlayerInfo("teammates");
  additionalTargets[AdminPlayers] = PlayerInfo("admins");
}


CursesUI::~CursesUI() {
  endwin();
}


void CursesUI::outputMessage(const std::string& msg, ColorCode color) {
  // add message to the message buffer, remove the oldest message if it's full
  if (msgBuffer.size() == maxBufferSize)
    msgBuffer.erase(msgBuffer.begin());
  std::pair<std::string, ColorCode> p(msg, color);
  msgBuffer.push_back(p);

  // if we have scrolled away from the bottom, don't show the new message
  if (scrollOffset == 0) {
    wattron(mainWin, COLOR_PAIR(color));
    waddstr(mainWin, (msg + "\n").c_str());
    wattroff(mainWin, COLOR_PAIR(color));
    wrefresh(mainWin);
  }
}


void CursesUI::handleNewPacket(uint16_t code) {
  BZAdminUI::handleNewPacket(code);
  menu.handleNewPacket(code);
}


bool CursesUI::checkCommand(std::string& str) {
  wrefresh(cmdWin);
  str = "";

  // get a character and do checks that are always needed
  int c = wgetch(cmdWin);
  switch (c) {
  case KEY_RESIZE:
    handleResize(LINES, COLS);
    return false;
  case KEY_F(2):
    toggleMenu();
    return false;
  case ERR:
    return false;
  }

  // if the menu is active, use the keystrokes for that
  if (menuState == 1)
    return menu.handleKey(c, str);

  // if not, go ahead and parse commands
  switch (c) {

    // clear command (21 is Ctrl-U)
  case 21:
    cmd = "";
    updateCmdWin();
    currentHistory = history.size();
    return false;

    // delete last character
  case KEY_BACKSPACE:
  case KEY_DC:
  case 8:
  case 127:
    cmd = cmd.substr(0, cmd.size() - 1);
    updateCmdWin();
    return false;

    // redraw command (12 is ctrl-l)
  case 12:
    wclear(cmdWin);
    wclear(targetWin);
    updateCmdWin();
    updateTargetWin();
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

    // scroll main window
  case KEY_NPAGE:
    scrollOffset = (scrollOffset < unsigned(LINES - 2) / 2 ?
		    0 : scrollOffset - (LINES - 2) / 2);
    updateMainWinFromBuffer(LINES - 2);
    return false;
  case KEY_PPAGE:
    if (msgBuffer.size() < unsigned(LINES - 2))
      scrollOffset = 0;
    else if (scrollOffset > msgBuffer.size() - (LINES - 2) - (LINES - 2) / 2)
      scrollOffset = msgBuffer.size() - (LINES - 2);
    else
      scrollOffset += (LINES - 2) / 2;
    updateMainWinFromBuffer(LINES - 2);
    return false;

    // change target - we have two maps to iterate over, so if we get to
    // the end/beginning of the first one we go to the beginning/end of the
    // second one and vice versa, also the maps should never be empty
  case KEY_LEFT:
    if (targetIter == additionalTargets.begin()) {
      targetIter = players.begin();
      for (unsigned int j = 0; j < players.size() - 1; j++)
	++targetIter;
    }
    else if (targetIter == players.begin()) {
      targetIter = additionalTargets.begin();
      for (unsigned int j = 0; j < additionalTargets.size() - 1; j++)
	++targetIter;
    }
    else
      targetIter--;
    updateTargetWin();
    return false;
  case KEY_RIGHT:
    targetIter++;
    if (targetIter == players.end())
      targetIter = additionalTargets.begin();
    else if (targetIter == additionalTargets.end())
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
    if (targetIter != players.end() && targetIter->first != me &&
	targetIter->first <= LastRealPlayer) {
      if (targetIter->second.isAdmin) {
	outputMessage("Warning: Kicking Administrator ("
	  + targetIter->second.name + ")!", Red);
      }
      cmd = "/kick \"";
      cmd += targetIter->second.name;
      cmd += "\"";
      targetIter = players.find(me);
      updateCmdWin();
      updateTargetWin();
    }
    return false;

    // ban target
  case KEY_F(6):
    if (targetIter != players.end() && targetIter->first != me &&
	targetIter->first <= LastRealPlayer) {
      if (targetIter->second.ip != "") {
	if (targetIter->second.isAdmin) {
	  outputMessage("Warning: Banning Administrator ("
	    + targetIter->second.name + ")!", Red);
	}
	cmd = "/ban ";
	cmd += targetIter->second.ip;
	targetIter = players.find(me);
	updateCmdWin();
	updateTargetWin();
      }
      else {
	std::string msg = "--- Can't ban ";
	msg += targetIter->second.name + ", you don't have the IP address";
      }
    }
    return false;

    // tab - autocomplete
  case '\t': {
    std::string matches;
    cmd = comp.complete(cmd, &matches);
    updateCmdWin();
    if (matches.size() > 0) {
      outputMessage(matches, White);
      updateTargetWin();
    }
    return false;
  }
  default:
    if (c < 32 || c > 127 || cmd.size() >= CMDLENGTH)
      return false;
    cmd += char(c);
    updateCmdWin();
    return false;
  }
}


void CursesUI::addedPlayer(PlayerId p) {
  PlayerIdMap::const_iterator iter = players.find(p);
  comp.registerWord(iter->second.name, true /* quote spaces */);
  if (p == me)
    targetIter = iter;
}


void CursesUI::removingPlayer(PlayerId p) {
  if (targetIter->first == p) {
    targetIter = players.find(me);
    updateTargetWin();
  }
  comp.unregisterWord(players.find(p)->second.name);
}


PlayerId CursesUI::getTarget() const {
  if (targetIter->first == me)
    return AllPlayers;
  return targetIter->first;
}


void CursesUI::handleResize(int lines, int cols) {
  resizeterm(lines, cols);
  wresize(mainWin, lines - 2, cols);
  updateMainWinFromBuffer(lines - 2);
  mvwin(targetWin, lines - 2, 0);
  wresize(targetWin, 1, cols);
  mvwin(cmdWin, lines - 1, 0);
  wresize(cmdWin, 1, cols);
  updateTargetWin();
  updateCmdWin();
  wrefresh(mainWin);
}


void CursesUI::updateMainWinFromBuffer(unsigned int numberOfMessages) {
  wclear(mainWin);
  int start = msgBuffer.size() - numberOfMessages - scrollOffset;
  start = (start < 0 ? 0 : start);
  unsigned int end = start + numberOfMessages;
  end = (end >= msgBuffer.size() ? msgBuffer.size() : end);
  for (unsigned int i = start ; i < end; ++i) {
    wattron(mainWin, COLOR_PAIR(msgBuffer[i].second));
    waddstr(mainWin, (msgBuffer[i].first + "\n").c_str());
    wattroff(mainWin, COLOR_PAIR(msgBuffer[i].second));
  }
  wrefresh(mainWin);
}


void CursesUI::updateTargetWin() {
  werase(targetWin);
  wmove(targetWin, 0, 0);
  wmove(targetWin, 1, 1);
  std::string tmp = "Send to ";
  tmp = tmp + (targetIter == players.end() || targetIter->first == me ?
	       "all" : targetIter->second.name) + ":";
  waddstr(targetWin, tmp.c_str());
  wrefresh(targetWin);
}


void CursesUI::updateCmdWin() {
  werase(cmdWin);
  wmove(cmdWin, 1, 1);
  waddstr(cmdWin, cmd.c_str());
  wmove(cmdWin, 1, 1 + cmd.size());
  wrefresh(cmdWin);
}


void CursesUI::toggleMenu() {
  if (menuState == NoMenu) {
    menuState = VisibleActive;
    curs_set(0);
    const int menuWinSize = (LINES - 2) / 2;
    wresize(mainWin, LINES - 2 - menuWinSize, COLS);
    mvwin(mainWin, menuWinSize, 0);
    updateMainWinFromBuffer(LINES - 2 - menuWinSize);
    menuWin = newwin(menuWinSize, 0, 0, 0);
    menu.setWindow(menuWin);
    menu.showMenu();
  }
  else if (menuState == VisibleActive) {
    menuState = VisibleInactive;
    curs_set(1);
  }
  else {
    menuState = NoMenu;
    menu.setWindow(NULL);
    delwin(menuWin);
    wresize(mainWin, LINES - 2, COLS);
    mvwin(mainWin, 0, 0);
    updateMainWinFromBuffer(LINES - 2);
  }
  wrefresh(targetWin);
  wrefresh(cmdWin);
}


void CursesUI::initMainMenu(CursesMenu& menu) {
  menu.setHeader("MAIN MENU");
  menu.clear();
  menu.addItem(new SubmenuCMItem("Show players",
				  &CursesUI::initPlayerMenu));
  //menu.addItem(new SubmenuCMItem("Edit banlist",
  //				  &CursesUI::initBanMenu));
  menu.addItem(new SubmenuCMItem("Edit server variables",
				  &CursesUI::initServerVarMenu));
  menu.addItem(new SubmenuCMItem("Edit message filter",
				 &CursesUI::initFilterMenu));
}


void CursesUI::initPlayerMenu(CursesMenu& menu) {
  menu.setHeader("PLAYERLIST");
  menu.clear();
  PlayerIdMap::const_iterator it;
  for (it = menu.players.begin(); it != menu.players.end(); ++it)
    menu.addItem(new PlayerCMItem(menu.players, it->first));
  menu.addItem(new SubmenuCMItem("Back to main menu",
				 &CursesUI::initMainMenu));
  std::map<uint16_t, bool>& updateTypes(menu.getUpdateTypes());
  updateTypes.clear();
  updateTypes[MsgAddPlayer] = true;
  updateTypes[MsgRemovePlayer] = true;
  updateTypes[MsgScore] = false;
  updateTypes[MsgKilled] = false;
  updateTypes[MsgAdminInfo] = false;
}


void CursesUI::initBanMenu(CursesMenu& menu) {
  menu.setHeader("BANLIST");
  menu.clear();
  menu.addItem(new SubmenuCMItem("Not implemented - go back",
				  &CursesUI::initMainMenu));
  menu.getUpdateTypes().clear();
}


void CursesUI::initServerVarMenu(CursesMenu& menu) {
  menu.setHeader("SERVER VARIABLE EDITOR");
  menu.clear();
  BZDB.iterate(&CursesUI::addBZDBCMItem, &menu);
  menu.addItem(new SubmenuCMItem("Back to main menu",
				 &CursesUI::initMainMenu));
  std::map<uint16_t, bool>& updateTypes(menu.getUpdateTypes());
  updateTypes.clear();
  updateTypes[MsgSetVar] = false;
}


void CursesUI::addBZDBCMItem(const std::string& name, void* menu) {
  ((CursesMenu*)menu)->addItem(new BZDBCMItem(name));
}


void CursesUI::initFilterMenu(CursesMenu& menu) {
  menu.setHeader("MESSAGE FILTER EDITOR");
  menu.clear();
  std::map<std::string, uint16_t>::const_iterator iter;
  for (iter = menu.client.getMessageTypeMap().begin();
       iter != menu.client.getMessageTypeMap().end(); ++iter)
    menu.addItem(new FilterCMItem(iter->first, menu.client));
  menu.addItem(new SubmenuCMItem("Back to main menu",
				 &CursesUI::initMainMenu));
  menu.getUpdateTypes().clear();
}


BZAdminUI* CursesUI::creator(BZAdminClient& client) {
  return new CursesUI(client);
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
