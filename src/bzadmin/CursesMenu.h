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

#ifndef CURSESMENU_H
#define CURSESMENU_H

/* bzflag special common - 1st one */
#include "common.h"

#include <map>
#include <string>
#include <vector>

#include "BZAdminClient.h"
#include "curses_wrapper.h"
#include "PlayerInfo.h"


class CursesMenu;
class BZAdminClient;


/** Callbacks of this type are used by CursesMenu when it needs to be
    updated. */
typedef void (*MenuCallback)(CursesMenu&);


/** This class is a very primitive widget base class for the menu in
    the curses interface for bzadmin. It has a virtual function showItem()
    that prints this widget at a given position in a curses window and
    another virtual function handleKey() that handles a keypress when this
    item is selected. This base class just prints the text content when
    showItem() is called and does nothing when handleKey() is called -
    it works like a label or menu header.
    @see CursesMenu
*/
class CursesMenuItem {
public:
  /** @c str is the text content of this item. It will be displayed in
      the menu. */
  CursesMenuItem(const std::string& str = "");
  /** This destructor does nothing, but we need a virtual destructor since
      we have virtual member functions. */
  virtual ~CursesMenuItem();
  /** This is the function that displays the menu item in the terminal.
      @param menuWin  the curses window to display the item in
      @param line     the line in the window that the item should be
		      displayed on
      @param col      the column in the window where the left edge of the
		      item should be
      @param width    the width of the item (in characters)
      @param selected true if this item is selected, false if it is not
  */
  virtual void showItem(WINDOW* menuWin, int line, int col, int width,
			bool selected);
  /** This function handles a key press. It is called when the menu is active,
      this item is the selected item, and the user presses a key on the
      keyboard. If the menu item wants to send a command to the server
      it should put the command in @c str and return @c true, otherwise
      it should return @c false.
      @param c    the key code
      @param str  if the menu item wants to send a command to the server it
		  should be stored here
      @param menu this is the menu that the item lives in, the menu item is
		  allowed to modify it (e.g. to clear it and add new menu
		  items if we want to go to a submenu)
      @return     true if a command should be sent to the server, false if not
  */
  virtual bool handleKey(int c, std::string& str, CursesMenu& menu);

  /** This function is called when the menu item is deselected. */
  virtual void deselect();

protected:
  std::string text;
};


/** This menu item type looks like a normal CursesMenuItem, but when it
    receives an "enter" key press it goes to a submenu.
*/
class SubmenuCMItem : public CursesMenuItem {
 public:
  /** @c str is the text of this menu item, @c callback is the callback that
      will be used as the new rebuilder function by the menu when this item
      receives an "enter" key press. */
  SubmenuCMItem(const std::string& str, MenuCallback callback);
  /** If @c c is a key code for the enter key, this function will change
      the rebuilder function of @c menu to @c cb. Other keys are ignored. */
  virtual bool handleKey(int c, std::string& str, CursesMenu& menu);
 protected:
  MenuCallback cb;
};


/** This menu item type looks like a normal CursesMenuItem, but when it
    receives an "enter" key press it runs a callback function. The callback
    function is given the parent CursesMenu as an argument. The callback can
    for example be used to change the contents of the menu.
    @see MenuCallback
*/
class CallbackCMItem : public CursesMenuItem {
 public:
  /** @c str is the text of this menu item, @c callback is the callback that
      will be called when the user presses enter and this item is selected. */
  CallbackCMItem(const std::string& str, MenuCallback callback);
  /** If @c c is a key code for the enter key, this function will call the
      callback. If it's something else it will be ignored. */
  virtual bool handleKey(int c, std::string& str, CursesMenu& menu);
 protected:
  MenuCallback cb;
};


/** This menu item type just sends a command to the server when it received
    an enter key press. */
class CommandCMItem : public CursesMenuItem {
public:
  /** @c str is the text that will be shown on the menu item, @c cmd is
      the command that will be sent to the server. If @c update is true
      the menu will be updated immediately after the user presses enter.
  */
  CommandCMItem(const std::string& str, const std::string& cmd,
		bool update = false);
  /** If @c c is a key code for the enter key, this function will put
      the @c command in @c str and return @c false. If it's something
      else it will be ignored. */
  virtual bool handleKey(int c, std::string& str, CursesMenu& menu);
protected:
  std::string command;
  bool forceUpdate;
};


/** This menu item type displays the value of a boolean variable and
    lets the user toggle it. */
class BoolCMItem : public CursesMenuItem {
 public:
  /** This creates a BoolCMItem which displays and edits the value of
      the bool variable @c variable. @c trueText and @c falseText are
      the values that are displayed when the variable is true or false. */
  BoolCMItem(std::string name, bool& variable, std::string trueText = "true",
	     std::string falseText = "false");
  /** This function displays the name of the variable and it's value. */
  virtual void showItem(WINDOW* menuWin, int line, int col, int width,
			bool selected);
  /** This function handles key presses from the user. Space toggles the
      variable value, any other key is ignored. */
  virtual bool handleKey(int c, std::string& str, CursesMenu& menu);
 protected:
  bool& varRef;
  std::string trueTxt, falseTxt;
};


/** This menu item type displays the filter status of a message type and
    lets the user toggle it. */
class FilterCMItem : public CursesMenuItem {
 public:
  /** This creates a FilterCMItem which displays and edits the filter status
      of the message type @c msgType in the client @c client. */
  FilterCMItem(const std::string& msgType, BZAdminClient& c);
  /** This function displays the name of the message type and it's status. */
  virtual void showItem(WINDOW* menuWin, int line, int col, int width,
			bool selected);
  /** This function handles key presses from the user. Space toggles the
      variable value, any other key is ignored. */
  virtual bool handleKey(int c, std::string& str, CursesMenu& menu);
 protected:
  std::string messageType;
  BZAdminClient& client;
  uint16_t numMsgType;
};


/** This menu item type displays the value of a BZDB variable and lets
    the user edit it. It doesn't know if the player has permission to
    edit the values, but if not the server will complain. */
class BZDBCMItem : public CursesMenuItem {
public:
  /** This creates a BZDBCMItem which displays the variable with the name
      @c variable. */
  BZDBCMItem(const std::string& variable);
  /** This function displays the name and the value of this item's
      BZDB variable. */
  virtual void showItem(WINDOW* menuWin, int line, int col, int width,
			bool selected);
  /** This function handles key presses when this item is selected. It has code
      that allows the user to edit the value of this items BZDB variable.
      When the user has edited the value and hits the enter key, a @c /set
      command will be placed in @c str and @c true will be returned. */
  virtual bool handleKey(int c, std::string& str, CursesMenu& menu);
  /** This stops editing the variable value without touching BZDB
      (same effect as when the user hits the escape key). */
  virtual void deselect();
protected:
  bool editing;
  std::string editString;
};


/** This menu item type displays information about a player. In the future it
    might also allow the user to ban and kick the player. */
class PlayerCMItem : public CursesMenuItem {
public:
  PlayerCMItem(const PlayerIdMap& players, PlayerId playerId);
  virtual void showItem(WINDOW* menuWin, int line, int col, int width,
			bool selected);
protected:
  const PlayerIdMap& playerMap;
  PlayerId id;
};


/** This class prints and handles key presses for a text-based curses menu.
    The menu is simply a list of menu items, which are printed in a single
    column. This class handles selection of menu items, scrolls the menu
    to keep the selected item visible, and it also has functions for clearing
    the menu (deleting all menu items) and adding new items.
    @see CursesMenuItem
*/
class CursesMenu {
public:

  /** CursesUI is our friend. (ugly, should be done differently) */
  friend class CursesUI;

  CursesMenu(BZAdminClient& c);

  /** This is needed to delete the dynamically allocated
      CursesMenuItem objects. */
  ~CursesMenu();

  /** This function changes the menu header. */
  void setHeader(const std::string& newHeader);

  /** This function adds an item to the menu. The item has to be
      dynamically allocated, and this CursesMenu object will delete the
      item when it's no longer used. */
  void addItem(CursesMenuItem* item);

  /** This function clears the menu (removes and deletes all items). */
  void clear();

  /** This function shows the menu. */
  void showMenu();

  /** This function handles a key. If the key generates a command that should
      be sent to the server that command will be places in @c str and @c true
      will be returned. */
  bool handleKey(int c, std::string& str);

  /** Set the curses window that this menu will be displayed in. */
  void setWindow(WINDOW* win);

  /** Sets the callback function that is used to update this menu. */
  void setUpdateCallback(MenuCallback cb);

  /** Force the menu to update itself using the current rebuilder function
      next time it gets the chance. */
  void forceUpdate();

  /** This function updates the menu if neccessary. */
  void rebuild();

  /** Return a reference to the map of message types that will cause the menu
      to redraw itself. If the associated @c bool is true, the menu will also
      be completely rebuilt (menu items can be removed or added). */
  std::map<uint16_t, bool>& getUpdateTypes();

  /** This function updates the menu if @c msgType is in the set of message
      types that will cause the menu to update. */
  void handleNewPacket(uint16_t msgType);

protected:

  std::string header;
  std::vector<CursesMenuItem*> items;
  int selection;
  WINDOW* window;
  MenuCallback rebuilder;
  BZAdminClient& client;
  const PlayerIdMap& players;
  bool dirty;
  std::map<uint16_t, bool> updateOnMsg;
};

#endif


// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
