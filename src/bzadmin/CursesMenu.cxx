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

#include "CursesMenu.h"
#include "StateDatabase.h"


CursesMenuItem::CursesMenuItem(const std::string& str) : text(str) {
  
}


CursesMenuItem::~CursesMenuItem() {

}


void CursesMenuItem::showItem(WINDOW* menuWin, int line, int col, int width,
			      bool selected) {
  /* just print the text of the item centered between col and col+width
     if this item is selected, use reverse video */
  wmove(menuWin, line, col);
  if (selected)
    wattron(menuWin, A_REVERSE);
  for (unsigned int i = 0; i < (width - text.size()) / 2; ++i)
    waddstr(menuWin, " ");
  waddstr(menuWin, text.c_str());
  for (int i = (width - text.size()) /2 + text.size(); i < width; ++i)
    waddstr(menuWin, " ");
  if (selected)
    wattroff(menuWin, A_REVERSE);
}


bool CursesMenuItem::handleKey(int, std::string&, CursesMenu&) {
  return false;
}


void CursesMenuItem::deselect() {
  
}


SubmenuCMItem::SubmenuCMItem(const std::string& str, MenuCallback callback)
  : CursesMenuItem(str), cb(callback) {
  
}
  

bool SubmenuCMItem::handleKey(int c, std::string&, CursesMenu& menu) {
  // different key codes for the enter key
  if (c == '\n' || c == 13) {
    menu.setUpdateCallback(cb);
    menu.forceUpdate();
  }
  return false;
}


CallbackCMItem::CallbackCMItem(const std::string& str, MenuCallback callback)
  : CursesMenuItem(str), cb(callback) {
  
}
  

bool CallbackCMItem::handleKey(int c, std::string&, CursesMenu& menu) {
  // different key codes for the enter key
  if (c == '\n' || c == 13) {
    cb(menu);
  }
  return false;
}


CommandCMItem::CommandCMItem(const std::string& str, const std::string& cmd,
			     bool update) 
  : CursesMenuItem(str), command(cmd), forceUpdate(update) {
  
}


bool CommandCMItem::handleKey(int c, std::string& str, CursesMenu& menu) {
  // different key codes for the enter key
  if (c == '\n' || c == 13) {
    str = command;
    if (forceUpdate)
      menu.forceUpdate();
    return true;
  }
  return false;
}


BoolCMItem::BoolCMItem(std::string name, bool& variable,  std::string trueText,
			  std::string falseText)
  : CursesMenuItem(name), varRef(variable), trueTxt(trueText), 
    falseTxt(falseText) {
    
}


void BoolCMItem::showItem(WINDOW* menuWin, int line, int col, int width, 
			  bool selected) {
  /* print the name of the variable to the left of the center and the
     value to the right, use reverse video if it is selected */
  wmove(menuWin, line, col);
  if (selected)
    wattron(menuWin, A_REVERSE);
  for (unsigned int i = 0; i < width / 2 - text.size() - 1; ++i)
    waddstr(menuWin, " ");
  waddstr(menuWin, text.c_str());
  wmove(menuWin, line, col + width / 2 + 1);
  std::string& value = (varRef ? trueTxt : falseTxt);
  waddstr(menuWin, value.c_str());
  for (int i = width / 2 + 1 + value.size(); i < width; ++i)
    waddstr(menuWin, " ");
  if (selected)
    wattroff(menuWin, A_REVERSE);
}


bool BoolCMItem::handleKey(int c, std::string&, CursesMenu&) {
  if (c == ' ')
    varRef = !varRef;
  return false;
}


BZDBCMItem::BZDBCMItem(const std::string& variable) 
  : CursesMenuItem(variable), editing(false) {
  
}


void BZDBCMItem::showItem(WINDOW* menuWin, int line, int col, int width, 
			  bool selected) {
  /* print the name of the variable to the left of the center and the
     value to the right, use reverse video if it is selected, but don't use
     reverse video for the value if we're editing it */
  wmove(menuWin, line, col);
  if (selected)
    wattron(menuWin, A_REVERSE);
  for (unsigned int i = 0; i < width / 2 - text.size() - 1; ++i)
    waddstr(menuWin, " ");
  waddstr(menuWin, text.c_str());
  wmove(menuWin, line, col + width / 2 + 1);
  std::string value = BZDB.get(text);
  if (editing) {
    wattroff(menuWin, A_REVERSE);
    waddstr(menuWin, editString.c_str());
    waddstr(menuWin, "_");
  }
  else
    waddstr(menuWin, value.c_str());
  for (int i = width / 2 + 1 + value.size(); i < width; ++i)
    waddstr(menuWin, " ");
  if (selected && !editing)
    wattroff(menuWin, A_REVERSE);
}


bool BZDBCMItem::handleKey(int c, std::string& str, CursesMenu&) {
  // if we're not editing, start if we get a return key
  if (!editing && (c == '\n' || c == 13)) {
    editing = true;
    editString = BZDB.get(text);
    return false;
  }
  
  // OK, we're editing
  switch (c) {
    
    // different codes for the return key - stop and send a /set command
  case '\n':
  case 13:
    editing = false;
    str = "/set ";
    str += text + " ";
    str += editString;
    return true;
    
    // ESC - stop editing, don't touch the BZDB value
  case 27:
    editing = false;
    break;
    
    // backspace/delete - delete the last character
  case KEY_BACKSPACE:
  case KEY_DC:
  case 127:
    editString = (editString.size() > 0 ? 
		  editString.substr(0, editString.size() - 1) : editString);
    break;
    
    // valid characters - edit the string
  default:
    if (c < 32 || c > 127 || editString.size() > 30)
      return false;
    editString += char(c);
    break;
  }
  
  return false;
}


void BZDBCMItem::deselect() {
  editing = false;
}


PlayerCMItem::PlayerCMItem(const PlayerIdMap& players, PlayerId playerId) 
  : CursesMenuItem(), playerMap(players), id(playerId) {
    
}


void PlayerCMItem::showItem(WINDOW* menuWin, int line, int col, int width,
			    bool selected) {
  // show the player name to the left of the center, reverse video if selected
  wmove(menuWin, line, col);
  if (selected)
    wattron(menuWin, A_REVERSE);
  std::string name, ip;
  PlayerIdMap::const_iterator iter = playerMap.find(id);
  if (iter != playerMap.end()) {
    name = iter->second.name;
    ip = iter->second.ip;
  }
  for (unsigned int i = 0; i < width / 2 - name.size() - 1; ++i)
    waddstr(menuWin, " ");
  waddstr(menuWin, name.c_str());
  wmove(menuWin, line, col + width / 2 + 1);
  waddstr(menuWin, ip.c_str());
  for (int i = width / 2 + 1 + ip.size(); i < width; ++i)
    waddstr(menuWin, " ");
  wattroff(menuWin, A_REVERSE);
}


CursesMenu::CursesMenu(const PlayerIdMap& p)
  : players(p), dirty(true) {
  
}


CursesMenu::~CursesMenu() {
  clear();
}


void CursesMenu::addItem(CursesMenuItem* item) {
  items.push_back(item);
}

  
void CursesMenu::clear() {
  for (unsigned int i = 0; i < items.size(); ++i)
    delete items[i];
  items.clear();
  selection = 0;
}
  

void CursesMenu::showMenu() {
  if (window == NULL)
    return;
  wclear(window);
  
  // update the menu if needed
  rebuild();
  
  // this magic should scroll the menu so that the selected menu item
  // always is visible
  int start = selection - LINES / 8;
  start = (start + LINES / 4  - 2> (signed)items.size() ? 
	   items.size() - LINES / 4 + 2  : start);
  start = (start < 0 ? 0 : start);
  int end = start + LINES / 4 - 2;
  end = ((unsigned)end > items.size() ? items.size() : end);
  
  // show the menu items
  for (int i = start; i < end; ++i)
    items[i]->showItem(window, 2 + 2 * (i - start), 10, COLS - 20, 
		       i == selection);
  
  // draw a line at the bottom of the menu window
  wmove(window, (LINES - 2) / 2 - 1, 0);
  wattron(window, A_UNDERLINE);
  for (int i = 0; i < COLS; ++i)
    waddstr(window, " ");
  wattroff(window, A_UNDERLINE);
  
  wrefresh(window);
}


bool CursesMenu::handleKey(int c, std::string& str) {
  bool result = false;
  str = "";
  switch (c) {
  case KEY_UP:
    items[selection]->deselect();
    selection = (selection == 0 ? items.size() - 1 : selection - 1);
    break;
  case KEY_DOWN:
    items[selection]->deselect();
    selection = ((unsigned)selection == items.size() - 1 ? 0 : selection + 1);
    break;
  default:
    result = items[selection]->handleKey(c, str, *this);
  }
  showMenu();
  return result;
}


void CursesMenu::setWindow(WINDOW* win) {
  window = win;
}


void CursesMenu::setUpdateCallback(MenuCallback cb) {
  rebuilder = cb;
}


void CursesMenu::forceUpdate() {
  dirty = true;
}


void CursesMenu::rebuild() {
  if (dirty) {
    dirty = false;
    rebuilder(*this);
  }
}



// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
