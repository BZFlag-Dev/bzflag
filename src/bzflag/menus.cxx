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

#ifdef macintosh
#include "mac_funcs.h"
#endif

#ifdef _MSC_VER
#pragma warning( 4 : 4786 )
#endif

/* system headers */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#ifndef _WIN32
#  include <sys/types.h>
#  include <dirent.h>
#  include <pwd.h>
#endif
#include <map>
#include "common.h"
#include "bzsignal.h"
#if defined(_WIN32)
#define _WINSOCKAPI_
#include <shlobj.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <direct.h>
#  define _POSIX_
#  include <limits.h>
#  undef _POSIX_
#  include <process.h>
#else
#  include <sys/types.h>
#  ifndef GUSI_20
#    include <sys/wait.h>
#  endif
#  include <unistd.h>
#endif
#include <math.h>
#include <time.h>
#include <fstream>

/* implementation-specific headers */
#include "bzfio.h"
#include "menus.h"
#include "sound.h"
#include "global.h"
#include "texture.h"
#include "playing.h"
#include "network.h"
#include "Team.h"
#include "Ping.h"
#include "Protocol.h"
#include "BzfDisplay.h"
#include "BzfWindow.h"
#include "MainWindow.h"
#include "SceneRenderer.h"
#include "OpenGLTexture.h"
#include "OpenGLTexFont.h"
#include "ErrorHandler.h"
#include "TimeKeeper.h"
#include "World.h"
#include "Bundle.h"
#include "ControlPanel.h"
#include "StateDatabase.h"
#include "CommandManager.h"
#include "CommandsStandard.h"
#include "KeyManager.h"
#include "ServerListCache.h"
#include "BZDBCache.h"
#include "TextUtils.h"
#include "TextureManager.h"
#include "ActionBinding.h"
#include "TextUtils.h"

// cause persistent rebuilding for build versioning
#include "version.h"


#ifdef _MSC_VER
#define PATH_MAX MAX_PATH
#endif

extern ControlPanel* controlPanel; // playing.cxx

//
// MenuDefaultKey
//

MenuDefaultKey		MenuDefaultKey::instance;

MenuDefaultKey::MenuDefaultKey() { }
MenuDefaultKey::~MenuDefaultKey() { }

bool			MenuDefaultKey::keyPress(const BzfKeyEvent& key)
{
  switch (key.ascii) {
    case 27:	// escape
      playLocalSound(SFX_DROP_FLAG);
      HUDDialogStack::get()->pop();
      return true;

    case 13:	// return
      playLocalSound(SFX_GRAB_FLAG);
      HUDDialogStack::get()->top()->execute();
      return true;
  }

  if (KEYMGR.get(key, true) == "quit") {
    CommandsStandard::quit();
    return true;
  }

  return false;
}

bool			MenuDefaultKey::keyRelease(const BzfKeyEvent& key)
{
  switch (key.ascii) {
    case 27:	// escape
    case 13:	// return
      return true;
  }
  return false;
}

//
// QuitMenu
//

class QuitMenuDefaultKey : public MenuDefaultKey {
  public:
			QuitMenuDefaultKey() { }
			~QuitMenuDefaultKey() { }

    bool		keyPress(const BzfKeyEvent&);
    bool		keyRelease(const BzfKeyEvent&);
};

bool			QuitMenuDefaultKey::keyPress(const BzfKeyEvent& key)
{
  if (key.ascii == 'Y' || key.ascii == 'y') {
    CommandsStandard::quit();
    return true;
  }
  return MenuDefaultKey::keyPress(key);
}

bool			QuitMenuDefaultKey::keyRelease(const BzfKeyEvent& key)
{
  if (key.ascii == 'Y' || key.ascii == 'y')
    return true;
  return MenuDefaultKey::keyRelease(key);
}

class QuitMenu : public HUDDialog {
  public:
			QuitMenu();
			~QuitMenu() { }

    HUDuiDefaultKey*	getDefaultKey() { return &defaultKey; }
    void		execute() { CommandsStandard::quit(); }
    void		resize(int width, int height);

    HUDuiControl*	createLabel(const char*, const char* = NULL);

  private:
    QuitMenuDefaultKey	defaultKey;
};

QuitMenu::QuitMenu()
{
  // add controls
  std::vector<HUDuiControl*>& list = getControls();
  HUDuiLabel* label;

  label = new HUDuiLabel;
  label->setFont(MainMenu::getFont());
  label->setString("Enter to quit, Esc to resume");
  list.push_back(label);

  label = new HUDuiLabel;
  label->setFont(MainMenu::getFont());
  label->setString("Really quit?");
  list.push_back(label);

  initNavigation(list, 1, 1);
}

HUDuiControl*		QuitMenu::createLabel(const char* string,
				const char* label)
{
  HUDuiLabel* control = new HUDuiLabel;
  control->setFont(MainMenu::getFont());
  control->setString(string);
  if (label) control->setLabel(label);
  return control;
}

void			QuitMenu::resize(int width, int height)
{
  HUDDialog::resize(width, height);

  // use a big font
  float fontWidth = (float)height / 10.0f;
  float fontHeight = (float)height / 10.0f;
  float smallFontWidth = (float)height / 36.0f;
  float smallFontHeight = (float)height / 36.0f;
  float x, y;

  // get stuff
  std::vector<HUDuiControl*>& list = getControls();
  HUDuiLabel* label = (HUDuiLabel*)list[0];
  label->setFontSize(smallFontWidth, smallFontHeight);
  const OpenGLTexFont& font = label->getFont();
  smallFontWidth = label->getFont().getWidth();
  smallFontHeight = label->getFont().getHeight();

  // help message
  label = (HUDuiLabel*)list[0];
  const float stringWidth = font.getWidth(label->getString());
  x = 0.5f * ((float)width - stringWidth);
  y = (float)height - fontHeight - 1.5f * font.getHeight();
  label->setPosition(x, y);

  // quit message
  label = (HUDuiLabel*)list[1];
  label->setFontSize(fontWidth, fontHeight);
  fontWidth = label->getFont().getWidth();
  fontHeight = label->getFont().getHeight();
  const float labelWidth = label->getFont().getWidth(label->getString());
  x = 0.5f * ((float)width - labelWidth);
  y = (float)height - 3.5f * fontHeight;
  label->setPosition(x, y);
}

//
// FormatMenu
//

class FormatMenu;

class FormatMenuDefaultKey : public MenuDefaultKey {
  public:
			FormatMenuDefaultKey(FormatMenu* _menu) :
				menu(_menu) { }
			~FormatMenuDefaultKey() { }

    bool		keyPress(const BzfKeyEvent&);
    bool		keyRelease(const BzfKeyEvent&);

  private:
    FormatMenu*		menu;
};

class FormatMenu : public HUDDialog {
  public:
			FormatMenu();
			~FormatMenu();

    HUDuiDefaultKey*	getDefaultKey() { return &defaultKey; }
    int			getSelected() const;
    void		setSelected(int);
    void		show();
    void		execute();
    void		resize(int width, int height);

    void		setFormat(bool test);

  public:
    static const int	NumItems;

  private:
    void		addLabel(const char* msg, const char* _label);

  private:
    FormatMenuDefaultKey defaultKey;
    int			numFormats;
    float		center;

    HUDuiLabel*		currentLabel;
    HUDuiLabel*		pageLabel;
    int			selectedIndex;
    bool*		badFormats;

    static const int	NumColumns;
    static const int	NumReadouts;
};

bool			FormatMenuDefaultKey::keyPress(const BzfKeyEvent& key)
{
  if (key.ascii == 0) switch (key.button) {
    case BzfKeyEvent::Up:
      if (HUDui::getFocus()) {
	menu->setSelected(menu->getSelected() - 1);
      }
      return true;

    case BzfKeyEvent::Down:
      if (HUDui::getFocus()) {
	menu->setSelected(menu->getSelected() + 1);
      }
      return true;

    case BzfKeyEvent::PageUp:
      if (HUDui::getFocus()) {
	menu->setSelected(menu->getSelected() - FormatMenu::NumItems);
      }
      return true;

    case BzfKeyEvent::PageDown:
      if (HUDui::getFocus()) {
	menu->setSelected(menu->getSelected() + FormatMenu::NumItems);
      }
      return true;
  }

  else if (key.ascii == '\t') {
    if (HUDui::getFocus()) {
	menu->setSelected(menu->getSelected() + 1);
    }
    return true;
  }

  else if (key.ascii == 'T' || key.ascii == 't') {
    menu->setFormat(true);
    return true;
  }
  return MenuDefaultKey::keyPress(key);
}

bool			FormatMenuDefaultKey::keyRelease(const BzfKeyEvent& key)
{
  switch (key.button) {
    case BzfKeyEvent::Up:
    case BzfKeyEvent::Down:
    case BzfKeyEvent::PageUp:
    case BzfKeyEvent::PageDown:
      return true;
  }
  switch (key.ascii) {
    case 27:	// escape
    case 13:	// return
    case 'T':
    case 't':
      return true;
  }
  return MenuDefaultKey::keyRelease(key);
}

const int		FormatMenu::NumReadouts = 4;
const int		FormatMenu::NumItems = 30;
const int		FormatMenu::NumColumns = 3;

FormatMenu::FormatMenu() : defaultKey(this), badFormats(NULL)
{
  int i;

  BzfDisplay* display = getDisplay();
  numFormats = display->getNumResolutions();
  badFormats = new bool[numFormats];
  for (i = 0; i < numFormats; i++)
    badFormats[i] = false;

  // add controls
  addLabel("Video Format", "");
  addLabel("", "");			// instructions
  addLabel("", "Current Format:");	// current format readout
  addLabel("", "");			// page readout
  currentLabel = (HUDuiLabel*)(getControls()[NumReadouts - 2]);
  pageLabel = (HUDuiLabel*)(getControls()[NumReadouts - 1]);

  // add resolution list items
  for (i = 0; i < NumItems; ++i)
    addLabel("", "");

  // fill in static labels
  if (numFormats < 2) {
    currentLabel->setString("<switching not available>");
    setFocus(NULL);
  }
  else {
    HUDuiLabel* label = (HUDuiLabel*)(getControls()[NumReadouts - 3]);
    label->setString("Press Enter to select and T to test a format. Esc to exit.");
    setFocus(pageLabel);
  }
}

FormatMenu::~FormatMenu()
{
  delete[] badFormats;
}

void			FormatMenu::addLabel(
				const char* msg, const char* _label)
{
  HUDuiLabel* label = new HUDuiLabel;
  label->setFont(MainMenu::getFont());
  label->setString(msg);
  label->setLabel(_label);
  getControls().push_back(label);
}

int			FormatMenu::getSelected() const
{
  return selectedIndex;
}

void			FormatMenu::setSelected(int index)
{
  BzfDisplay* display = getDisplay();
  std::vector<HUDuiControl*>& list = getControls();

  // clamp index
  if (index < 0)
    index = numFormats - 1;
  else if (index != 0 && index >= numFormats)
    index = 0;

  // ignore if no change
  if (selectedIndex == index)
    return;

  // update current format
  currentLabel->setString(display->getResolution(
				display->getResolution())->name);

  // update selected index and get old and new page numbers
  const int oldPage = (selectedIndex < 0) ? -1 : (selectedIndex / NumItems);
  selectedIndex = index;
  const int newPage = (selectedIndex / NumItems);

  // if page changed then load items for this page
  if (oldPage != newPage) {
    // fill items
    const int base = newPage * NumItems;
    for (int i = 0; i < NumItems; ++i) {
      HUDuiLabel* label = (HUDuiLabel*)list[i + NumReadouts];
      if (base + i < numFormats)
	if (badFormats[base + i])
	  label->setString("<unloadable>");
	else
	  label->setString(display->getResolution(base + i)->name);
      else
	label->setString("");
    }

    // change page label
    if (numFormats > NumItems) {
      char msg[50];
      std::vector<std::string> args;
      sprintf(msg, "%d", newPage + 1);
      args.push_back(msg);
      sprintf(msg, "%d", (numFormats + NumItems - 1) / NumItems);
      args.push_back(msg);
      pageLabel->setString("Page {1} of {2}", &args);
    }
  }

  // set focus to selected item
  if (numFormats > 0) {
    const int indexOnPage = selectedIndex % NumItems;
    getControls()[NumReadouts + indexOnPage]->setFocus();
  }
  else {
    setFocus(NULL);
  }
}

void			FormatMenu::show()
{
  pageLabel->setString("");
  selectedIndex = -1;
  setSelected(getDisplay()->getResolution());
}

void			FormatMenu::execute()
{
  setFormat(false);
}

void			FormatMenu::setFormat(bool test)
{
  if (selectedIndex >= numFormats || badFormats[selectedIndex])
    return;

  if (!setVideoFormat(selectedIndex, test)) {
    // can't load format
    badFormats[selectedIndex] = true;
  }
  else if (!test) {
    // print OpenGL renderer, which might have changed
    printError((const char*)glGetString(GL_RENDERER));
  }

  // update readouts
  const int oldSelectedIndex = selectedIndex;
  selectedIndex = -1;
  setSelected(oldSelectedIndex);
}

void			FormatMenu::resize(int width, int height)
{
  HUDDialog::resize(width, height);

  // use a big font for title, smaller font for the rest
  const float titleFontWidth = (float)height / 10.0f;
  const float titleFontHeight = (float)height / 10.0f;

  // reposition title
  float x, y;
  std::vector<HUDuiControl*>& list = getControls();
  {
    HUDuiLabel* title = (HUDuiLabel*)list[0];
    title->setFontSize(titleFontWidth, titleFontHeight);
    const OpenGLTexFont& titleFont = title->getFont();
    const float titleWidth = titleFont.getWidth(title->getString());
    x = 0.5f * ((float)width - titleWidth);
    y = (float)height - titleFont.getHeight();
    title->setPosition(x, y);
  }

  // reposition test and current format messages
  float fontWidth = (float)height / 36.0f;
  float fontHeight = (float)height / 36.0f;
  {
    HUDuiLabel* label = (HUDuiLabel*)list[1];
    label->setFontSize(fontWidth, fontHeight);
    const OpenGLTexFont& font = label->getFont();
    const float stringWidth = font.getWidth(label->getString());
    x = 0.5f * ((float)width - stringWidth);
    y -= 1.5f * font.getHeight();
    label->setPosition(x, y);
  }
  {
    HUDuiLabel* label = currentLabel;
    label->setFontSize(fontWidth, fontHeight);
    const OpenGLTexFont& font = label->getFont();
    y -= 1.0f * font.getHeight();
    label->setPosition(0.5f * (float)width, y);
  }

  // position page readout
  fontWidth = (float)height / 36.0f;
  fontHeight = (float)height / 36.0f;
  {
    HUDuiLabel* label = pageLabel;
    label->setFontSize(fontWidth, fontHeight);
    const OpenGLTexFont& font = label->getFont();
    const float stringWidth = font.getWidth(label->getString());
    x = 0.5f * ((float)width - stringWidth);
    y -= 2.0f * font.getHeight();
    label->setPosition(x, y);
  }

  // position format item list
  const float yBase = y;
  int lastColumn = -1;
  for (int i = 0; i < NumItems; ++i) {
    const int column = i * NumColumns / NumItems;
    if (column != lastColumn) {
      lastColumn = column;
      x = (float)width * ((0.5f + (float)column) / (float)(NumColumns + 1));
      y = yBase;
    }

    HUDuiLabel* label = (HUDuiLabel*)list[i + NumReadouts];
    label->setFontSize(fontWidth, fontHeight);
    const OpenGLTexFont& font = label->getFont();
    y -= 1.0f * font.getHeight();
    label->setPosition(x, y);
  }
}


//
// QuickKeysMenu
//

class QuickKeysMenu : public HUDDialog {
  public:
			QuickKeysMenu();
			~QuickKeysMenu() { }

    HUDuiDefaultKey*	getDefaultKey()
				{ return MenuDefaultKey::getInstance(); }

    void		show();
    void		dismiss();
    void		execute() {}
    void		resize(int width, int height);

  private:
    HUDuiLabel*		createLabel(const std::string &);
    HUDuiTypeIn*	createInput(const std::string &);
  private:
    int 		firstKeyControl;
};

QuickKeysMenu::QuickKeysMenu()
{
  // add controls
  std::vector<HUDuiControl*>& controls = getControls();

  controls.push_back(createLabel("Define Quick Keys"));
  controls.push_back(createLabel("Notice: depending on platform not all keys might work"));

  controls.push_back(createLabel("Send to All"));
  controls.push_back(createLabel("Send to Team"));

  firstKeyControl = controls.size();

  int i;
  for (i=1; i < 11; i++) {
    std::string keyLabel = string_util::format("Alt-F%d", i);
    controls.push_back(createInput(keyLabel));
  }

  for (i=1; i < 11; i++) {
    std::string keyLabel = string_util::format("Ctrl-F%d", i);
    controls.push_back(createInput(keyLabel));
  }

  initNavigation(controls, firstKeyControl, controls.size()-1);
}

void			QuickKeysMenu::show()
{
  std::vector<HUDuiControl*>& controls = getControls();

  int i;
  for (i=1; i < 11; i++) {
    HUDuiTypeIn *entry = static_cast<HUDuiTypeIn*>(controls[firstKeyControl + i - 1]);
    std::string keyName = string_util::format("quickMessage%d", i);
    std::string keyValue = BZDB.get(keyName);
    entry->setString(keyValue);
  }

  for (i=1; i < 11; i++) {
    HUDuiTypeIn *entry = static_cast<HUDuiTypeIn*>(controls[firstKeyControl + i + 9]);
    std::string keyName = string_util::format("quickTeamMessage%d", i);
    std::string keyValue = BZDB.get(keyName);
    entry->setString(keyValue);
  }
}

void			QuickKeysMenu::dismiss()
{
  std::vector<HUDuiControl*>& controls = getControls();

  int i;
  for (i=1; i < 11; i++) {
    HUDuiTypeIn *entry = static_cast<HUDuiTypeIn*>(controls[firstKeyControl + i - 1]);
    std::string keyValue = entry->getString();
    std::string keyName = string_util::format("quickMessage%d", i);
    if (keyValue.empty() && BZDB.isSet(keyName))
      BZDB.unset(keyName);
    else if (!keyValue.empty())
      BZDB.set(keyName, keyValue);
  }

  for (i=1; i < 11; i++) {
    HUDuiTypeIn *entry = static_cast<HUDuiTypeIn*>(controls[firstKeyControl + i + 9]);
    std::string keyValue = entry->getString();
    std::string keyName = string_util::format("quickTeamMessage%d", i);
    if (keyValue.empty() && BZDB.isSet(keyName))
      BZDB.unset(keyName);
    else if (!keyValue.empty())
      BZDB.set(keyName, keyValue);
  }
}

void			QuickKeysMenu::resize(int width, int height)
{
  HUDDialog::resize(width, height);

  int i;
  // use a big font for title, smaller font for the rest
  const float titleFontWidth = (float)height / 10.0f;
  const float titleFontHeight = (float)height / 10.0f;
  const float bigFontWidth = (float)height / 28.0f;
  const float bigFontHeight = (float)height / 28.0f;

  const float fontWidth = (float)height / 32.0f;
  const float fontHeight = (float)height / 32.0f;

  // reposition title
  std::vector<HUDuiControl*>& list = getControls();
  HUDuiLabel* title = (HUDuiLabel*)list[0];
  title->setFontSize(titleFontWidth, titleFontHeight);
  const OpenGLTexFont& titleFont = title->getFont();
  const float titleWidth = titleFont.getWidth(title->getString());
  float x = 0.5f * ((float)width - titleWidth);
  float y = (float)height - titleFont.getHeight();
  title->setPosition(x, y);

  // reposition help
  HUDuiLabel*help = (HUDuiLabel*)list[1];
  help->setFontSize( bigFontWidth, bigFontHeight);
  const OpenGLTexFont& helpFont = help->getFont();
  const float helpWidth = helpFont.getWidth(help->getString());
  x = 0.5f * ((float)width - helpWidth);
  y -= 1.1f * helpFont.getHeight();
  help->setPosition(x, y);

  // reposition column titles
  HUDuiLabel *all = (HUDuiLabel*)list[2];
  all->setFontSize(bigFontWidth, bigFontHeight);
  x = 0.1f * width;
  y -= 1.5f * helpFont.getHeight();
  all->setPosition(x, y);
  HUDuiLabel *team = (HUDuiLabel*)list[3];
  team->setFontSize(bigFontWidth, bigFontHeight);
  x = 0.6f * width;
  team->setPosition(x, y);


  // reposition options in two columns
  x = 0.10f * (float)width;
  const float topY = y - (0.6f * titleFont.getHeight());
  y = topY;
  list[4]->setFontSize(fontWidth, fontHeight);
  const float h = list[4]->getFont().getHeight();
  const int count = list.size() - firstKeyControl;
  const int mid = (count / 2) + firstKeyControl;

  for (i = firstKeyControl; i < mid; i++) {
    list[i]->setFontSize(fontWidth, fontHeight);
    list[i]->setPosition(x, y);
    y -= 1.0f * h;
  }

  x = 0.60f * (float)width;
  y = topY;
  for (;i < count + firstKeyControl; i++) {
    list[i]->setFontSize(fontWidth, fontHeight);
    list[i]->setPosition(x, y);
    y -= 1.0f * h;
  }
}

HUDuiLabel*		QuickKeysMenu::createLabel(const std::string &str)
{
  HUDuiLabel* label = new HUDuiLabel;
  label->setFont(MainMenu::getFont());
  label->setString(str);
  return label;
}


HUDuiTypeIn*		QuickKeysMenu::createInput(const std::string &label)
{
  HUDuiTypeIn* entry = new HUDuiTypeIn;
  entry->setFont(MainMenu::getFont());
  entry->setLabel(label);
  entry->setMaxLength(40); // some strings >20 won't already fit into column
  return entry;
}


//
// KeyboardMapMenu
//

class KeyboardMapMenu;

class KeyboardMapMenuDefaultKey : public MenuDefaultKey {
  public:
			KeyboardMapMenuDefaultKey(KeyboardMapMenu*);
			~KeyboardMapMenuDefaultKey() { }

    bool		keyPress(const BzfKeyEvent&);
    bool		keyRelease(const BzfKeyEvent&);

  public:
    KeyboardMapMenu*	menu;
};

class KeyboardMapMenu : public HUDDialog {
  public:
			KeyboardMapMenu();
			~KeyboardMapMenu() { delete quickKeysMenu; }

    HUDuiDefaultKey*	getDefaultKey() { return &defaultKey; }
    void		execute();
    void		dismiss();
    void		resize(int width, int height);

    bool		isEditing() const;
    void		setKey(const BzfKeyEvent&);
    void		onScan(const std::string& name, bool press, const std::string& cmd);
    static void		onScanCB(const std::string& name, bool press,
				 const std::string& cmd, void* userData);

  private:
    void		update();

    HUDuiLabel*		createLabel(const char*, const char* = NULL);

    void		initkeymap(const std::string& name, int index);
  private:
    struct keymap {
      int index;	// ui label index
      std::string key1;
      std::string key2;
    };
    typedef std::map<std::string, keymap> KeyKeyMap;
    KeyKeyMap				mappable;
    KeyboardMapMenuDefaultKey		defaultKey;
    HUDuiControl*			reset;
    HUDuiControl*                       quickKeys;
    int 				editing;
    QuickKeysMenu*			quickKeysMenu;
};

KeyboardMapMenuDefaultKey::KeyboardMapMenuDefaultKey(KeyboardMapMenu* _menu) :
				menu(_menu)
{
  // do nothing
}

bool			KeyboardMapMenuDefaultKey::keyPress(
				const BzfKeyEvent& key)
{
  // escape key has usual effect
  if (key.ascii == 27)
    return MenuDefaultKey::keyPress(key);

  // keys have normal effect if not editing
  if (!menu->isEditing())
    return MenuDefaultKey::keyPress(key);

  // ignore keys we don't know
  if (key.ascii != 0 && isspace(key.ascii)) {
    if (key.ascii != ' ' && key.ascii != '\t' && key.ascii != '\r')
      return true;
  }

  // all other keys modify mapping
  menu->setKey(key);
  return true;
}

bool			KeyboardMapMenuDefaultKey::keyRelease(
				const BzfKeyEvent&)
{
  // ignore key releases
  return true;
}

KeyboardMapMenu::KeyboardMapMenu() : defaultKey(this), editing(-1), quickKeysMenu(NULL)
{
  // add controls
  std::vector<HUDuiControl*>& controls = getControls();

  controls.push_back(createLabel("Key Mapping"));
  controls.push_back(createLabel("Use up/down arrows to navigate, enter key to enter edit mode"));
  controls.push_back(reset = createLabel(NULL, "Reset Defaults"));
  controls.push_back(createLabel("fire", "Fire shot:"));
  controls.push_back(createLabel(NULL, "Drop flag:"));
  controls.push_back(createLabel(NULL, "Identify/Lock On:"));
  controls.push_back(createLabel(NULL, "Radar Short:"));
  controls.push_back(createLabel(NULL, "Radar Medium:"));
  controls.push_back(createLabel(NULL, "Radar Long:"));
  controls.push_back(createLabel(NULL, "Send to All:"));
  controls.push_back(createLabel(NULL, "Send to Teammates:"));
  controls.push_back(createLabel(NULL, "Send to Nemesis:"));
  controls.push_back(createLabel(NULL, "Send to Recipient:"));
  controls.push_back(createLabel(NULL, "Send to Admin:"));
  controls.push_back(createLabel(NULL, "Jump:"));
  controls.push_back(createLabel(NULL, "Binoculars:"));
  controls.push_back(createLabel(NULL, "Toggle Score:"));
  controls.push_back(createLabel(NULL, "Tank Labels:"));
  controls.push_back(createLabel(NULL, "Flag Help:"));
  controls.push_back(createLabel(NULL, "Time Forward:"));
  controls.push_back(createLabel(NULL, "Time Backward:"));
  controls.push_back(createLabel(NULL, "Pause/Resume:"));
  controls.push_back(createLabel(NULL, "Self Destruct/Cancel:"));
  controls.push_back(createLabel(NULL, "Fast Quit:"));
  controls.push_back(createLabel(NULL, "Scroll Backward:"));
  controls.push_back(createLabel(NULL, "Scroll Forward:"));
  controls.push_back(createLabel(NULL, "Slow Keyboard Motion:"));
  controls.push_back(createLabel(NULL, "Toggle Flags on Radar:"));
  controls.push_back(createLabel(NULL, "Toggle Flags on Field:"));
  controls.push_back(createLabel(NULL, "Silence/UnSilence Key:"));
  controls.push_back(createLabel(NULL, "Server Command Key:"));
  controls.push_back(createLabel(NULL, "Hunt Key:"));
  controls.push_back(createLabel(NULL, "AutoPilot Key: "));
  controls.push_back(quickKeys = createLabel(NULL, "Define Quick Keys"));

  initNavigation(controls, 2, controls.size()-1);

  int i = 3;

  initkeymap("fire", i++);
  initkeymap("drop", i++);
  initkeymap("identify", i++);
  initkeymap("set displayRadarRange 0.25", i++);
  initkeymap("set displayRadarRange 0.5", i++);
  initkeymap("set displayRadarRange 1.0", i++);
  initkeymap("send all", i++);
  initkeymap("send team", i++);
  initkeymap("send nemesis", i++);
  initkeymap("send recipient", i++);
  initkeymap("send admin", i++);
  initkeymap("jump", i++);
  initkeymap("toggle displayBinoculars", i++);
  initkeymap("toggle displayScore", i++);
  initkeymap("toggle displayLabels", i++);
  initkeymap("toggle displayFlagHelp", i++);
  initkeymap("time forward", i++);
  initkeymap("time backward", i++);
  initkeymap("pause", i++);
  initkeymap("destruct", i++);
  initkeymap("quit", i++);
  initkeymap("scrollpanel up", i++);
  initkeymap("scrollpanel down", i++);
  initkeymap("toggle slowKeyboard", i++);
  initkeymap("toggleFlags radar", i++);
  initkeymap("toggleFlags main", i++);
  initkeymap("silence", i++);
  initkeymap("servercommand", i++);
  initkeymap("hunt", i++);
  initkeymap("autopilot", i++);
  
}

void			KeyboardMapMenu::initkeymap(const std::string& name, int index)
{
  mappable[name].key1 = "";
  mappable[name].key2 = "";
  mappable[name].index = index;
}

bool			KeyboardMapMenu::isEditing() const
{
  return editing != -1;
}

void			KeyboardMapMenu::setKey(const BzfKeyEvent& event)
{
  if (editing == -1)
    return;
  KeyKeyMap::iterator it;
  for (it = mappable.begin(); it != mappable.end(); it++)
    if (it->second.index == editing)
      break;
  if ((KEYMGR.keyEventToString(event) == it->second.key1 && it->second.key2.empty()) || (KEYMGR.keyEventToString(event) == it->second.key2))
    return;
  ActionBinding::instance().associate(KEYMGR.keyEventToString(event),
				      it->first);
  editing = -1;
  update();
}

void			KeyboardMapMenu::execute()
{
  const HUDuiControl* const focus = HUDui::getFocus();
  if (focus == reset) {
    ActionBinding::instance().resetBindings();
    update();
  }
  else if (focus == quickKeys) {
    if (!quickKeysMenu) quickKeysMenu = new QuickKeysMenu;
    HUDDialogStack::get()->push(quickKeysMenu);
  }
  else {
    // start editing
    std::vector<HUDuiControl*>& list = getControls();
    KeyKeyMap::iterator it;
    for (it = mappable.begin(); it != mappable.end(); it++) {
      if (list[it->second.index] == focus) {
	editing = it->second.index;
	if (!it->second.key1.empty() && !it->second.key2.empty()) {
	  ActionBinding::instance().deassociate(it->first);
	}
      }
    }
  }
  update();
}

void			KeyboardMapMenu::dismiss()
{
  editing = -1;
  notifyBzfKeyMapChanged();
}

void			KeyboardMapMenu::resize(int width, int height)
{
  HUDDialog::resize(width, height);

  int i;
  // use a big font for title, smaller font for the rest
  const float titleFontWidth = (float)height / 10.0f;
  const float titleFontHeight = (float)height / 10.0f;
  const float bigFontWidth = (float)height / 28.0f;
  const float bigFontHeight = (float)height / 28.0f;

  const float fontWidth = (float)height / 38.0f;
  const float fontHeight = (float)height / 38.0f;

  // reposition title
  std::vector<HUDuiControl*>& list = getControls();
  HUDuiLabel* title = (HUDuiLabel*)list[0];
  title->setFontSize(titleFontWidth, titleFontHeight);
  const OpenGLTexFont& titleFont = title->getFont();
  const float titleWidth = titleFont.getWidth(title->getString());
  float x = 0.5f * ((float)width - titleWidth);
  float y = (float)height - titleFont.getHeight();
  title->setPosition(x, y);
  // reposition help
  HUDuiLabel*help = (HUDuiLabel*)list[1];
  help->setFontSize( bigFontWidth, bigFontHeight);
  const OpenGLTexFont& helpFont = help->getFont();
  const float helpWidth = helpFont.getWidth(help->getString());
  x = 0.5f * ((float)width - helpWidth);
  y -= 1.1f * helpFont.getHeight();
  help->setPosition(x, y);


  // reposition options in two columns
  x = 0.30f * (float)width;
  const float topY = y - (0.6f * titleFont.getHeight());
  y = topY;
  list[2]->setFontSize(fontWidth, fontHeight);
  const float h = list[2]->getFont().getHeight();
  const int count = list.size() - 2;
  const int mid = (count / 2);

  for (i = 2; i <= mid+1; i++) {
    list[i]->setFontSize(fontWidth, fontHeight);
    list[i]->setPosition(x, y);
    y -= 1.0f * h;
  }

  x = 0.80f * (float)width;
  y = topY;
  for (i = mid+2; i < count+2; i++) {
    list[i]->setFontSize(fontWidth, fontHeight);
    list[i]->setPosition(x, y);
    y -= 1.0f * h;
  }

  update();
}

void			KeyboardMapMenu::update()
{
  KeyKeyMap::iterator it;
  // clear
  for (it = mappable.begin(); it != mappable.end(); it++) {
    it->second.key1 = "";
    it->second.key2 = "";
  }
  // load current settings
  KEYMGR.iterate(&onScanCB, this);
  std::vector<HUDuiControl*>& list = getControls();
  for (it = mappable.begin(); it != mappable.end(); it++) {
    std::string value = "";
    if (it->second.key1.empty()) {
      if (isEditing() && (it->second.index == editing))
	value = "???";
      else
	value = "<not mapped>";
    } else {
      value += it->second.key1;
      if (!it->second.key2.empty()) {
	value += " or " + it->second.key2;
      } else if (isEditing() && (it->second.index == editing)) {
	value += " or ???";
      }
    }
    ((HUDuiLabel*)list[it->second.index])->setString(value);
  }
}

void			KeyboardMapMenu::onScan(const std::string& name, bool press,
						const std::string& cmd)
{
  if (!press)
    return;
  KeyKeyMap::iterator it = mappable.find(cmd);
  if (it == mappable.end())
    return;
  if (it->second.key1.empty())
    it->second.key1 = name;
  else if (it->second.key2.empty())
    it->second.key2 = name;
}

void			KeyboardMapMenu::onScanCB(const std::string& name, bool press,
						  const std::string& cmd, void* userData)
{
  reinterpret_cast<KeyboardMapMenu*>(userData)->onScan(name, press, cmd);
}

HUDuiLabel*		KeyboardMapMenu::createLabel(
				const char* str, const char* _label)
{
  HUDuiLabel* label = new HUDuiLabel;
  label->setFont(MainMenu::getFont());
  if (str) label->setString(str);
  if (_label) label->setLabel(_label);
  return label;
}




//
// GUIOptionsMenu
//

class GUIOptionsMenu : public HUDDialog {
  public:
			GUIOptionsMenu();
			~GUIOptionsMenu();

    HUDuiDefaultKey*	getDefaultKey()
				{ return MenuDefaultKey::getInstance(); }
    void		execute();
    void		resize(int width, int height);
    static void		callback(HUDuiControl* w, void* data);

  private:
};

GUIOptionsMenu::GUIOptionsMenu()
{
  // add controls
  std::vector<HUDuiControl*>& list = getControls();

  HUDuiLabel* label = new HUDuiLabel;
  label->setFont(MainMenu::getFont());
  label->setString("GUI Options");
  list.push_back(label);

  HUDuiList* option = new HUDuiList;
  option->setFont(MainMenu::getFont());
  option->setLabel("Enhanced radar:");
  option->setCallback(callback, (void*)"e");
  std::vector<std::string>* options = &option->getList();
  options->push_back(std::string("Off"));
  options->push_back(std::string("On"));
  option->update();
  list.push_back(option);

  option = new HUDuiList;
  option->setFont(MainMenu::getFont());
  option->setLabel("Controlpanel & Score FontSize:");
  option->setCallback(callback, (void*)"w");
  options = &option->getList();
  options->push_back(std::string("normal"));
  options->push_back(std::string("bigger"));
  option->update();
  list.push_back(option);

  // set Radar Translucency
  option = new HUDuiList;
  option->setFont(MainMenu::getFont());
  option->setLabel("Radar & Panel Opacity:");
  option->setCallback(callback, (void*)"y");
  option->createSlider(11);
  option->update();
  list.push_back(option);

  // toggle coloring of shots on radar
  option = new HUDuiList;
  option->setFont(MainMenu::getFont());
  option->setLabel("Colored shots on radar:");
  option->setCallback(callback, (void*)"z");
  options = &option->getList();
  options->push_back(std::string("Off"));
  options->push_back(std::string("On"));
  option->update();
  list.push_back(option);

  // set radar shot length
  option = new HUDuiList;
  option->setFont(MainMenu::getFont());
  option->setLabel("Radar Shot Length:");
  option->setCallback(callback, (void*)"l");
  option->createSlider(11);
  option->update();
  list.push_back(option);

  // set radar shot size
  option = new HUDuiList;
  option->setFont(MainMenu::getFont());
  option->setLabel("Radar Shot Size:");
  option->setCallback(callback, (void*)"s");
  option->createSlider(11);
  option->update();
  list.push_back(option);

  // set radar size
  option = new HUDuiList;
  option->setFont(MainMenu::getFont());
  option->setLabel("Radar & Panel Size:");
  option->setCallback(callback, (void*)"R");
  option->createSlider(11);
  option->update();
  list.push_back(option);

  // set maxmotion size
  option = new HUDuiList;
  option->setFont(MainMenu::getFont());
  option->setLabel("Mouse Box Size:");
  option->setCallback(callback, (void*)"M");
  option->createSlider(11);
  option->update();
  list.push_back(option);

  // set locale
  option = new HUDuiList;
  option->setFont(MainMenu::getFont());
  option->setLabel("Locale:");
  option->setCallback(callback, (void*)"L");
  options = &option->getList();
  std::vector<std::string> locales;
  if (BundleMgr::getLocaleList(&locales) == true) {
	options->push_back(std::string("default"));
	for (int i = 0; i < (int)locales.size(); i++) {
	  options->push_back(locales[i]);
	}
	locales.erase(locales.begin(), locales.end());
  }
  else {
	// Something failed when trying to compile a list
	// of all the locales.
	options->push_back(std::string("default"));
  }

  for (int i = 0; i < (int)options->size(); i++) {
    if ((*options)[i].compare(World::getLocale()) == 0) {
      option->setIndex(i);
      break;
    }
  }
  option->update();
  list.push_back(option);

  // GUI coloring
  option = new HUDuiList;
  option->setFont(MainMenu::getFont());
  option->setLabel("Control panel coloring:");
  option->setCallback(callback, (void*)"c");
  options = &option->getList();
  options->push_back(std::string("off"));
  options->push_back(std::string("on"));
  option->update();
  list.push_back(option);
  // Underline color
  option = new HUDuiList;
  option->setFont(MainMenu::getFont());
  option->setLabel("Underline color:");
  option->setCallback(callback, (void*)"u");
  options = &option->getList();
  options->push_back(std::string("cyan"));
  options->push_back(std::string("grey"));
  options->push_back(std::string("text"));
  option->update();
  list.push_back(option);
  // Killer Highlight
  option = new HUDuiList;
  option->setFont(MainMenu::getFont());
  option->setLabel("Killer Highlight:");
  option->setCallback(callback, (void*)"k");
  options = &option->getList();
  options->push_back(std::string("Blinking"));
  options->push_back(std::string("Underline"));
  options->push_back(std::string("None"));
  option->update();
  list.push_back(option);

  initNavigation(list, 1,list.size()-1);
}

GUIOptionsMenu::~GUIOptionsMenu()
{
}

void			GUIOptionsMenu::execute()
{
}

void			GUIOptionsMenu::resize(int width, int height)
{
  HUDDialog::resize(width, height);

  // use a big font for title, smaller font for the rest
  const float titleFontWidth = (float)height / 10.0f;
  const float titleFontHeight = (float)height / 10.0f;
  const float fontWidth = (float)height / 30.0f;
  const float fontHeight = (float)height / 30.0f;

  // reposition title
  std::vector<HUDuiControl*>& list = getControls();
  HUDuiLabel* title = (HUDuiLabel*)list[0];
  title->setFontSize(titleFontWidth, titleFontHeight);
  const OpenGLTexFont& titleFont = title->getFont();
  const float titleWidth = titleFont.getWidth(title->getString());
  float x = 0.5f * ((float)width - titleWidth);
  float y = (float)height - titleFont.getHeight();
  title->setPosition(x, y);

  // reposition options
  x = 0.5f * ((float)width + 0.5f * titleWidth);
  y -= 0.6f * titleFont.getHeight();
  const int count = list.size();
  for (int i = 1; i < count; i++) {
    list[i]->setFontSize(fontWidth, fontHeight);
    list[i]->setPosition(x, y);
    y -= 1.0f * list[i]->getFont().getHeight();
  }

  // load current settings
  SceneRenderer* renderer = getSceneRenderer();
  if (renderer) {
    int i = 1;
    ((HUDuiList*)list[i++])->setIndex(BZDBCache::enhancedRadar ? 1 : 0);
    ((HUDuiList*)list[i++])->setIndex(BZDB.isTrue("bigfont") ? 1 : 0);
    ((HUDuiList*)list[i++])->setIndex((int)(10.0f * renderer->getPanelOpacity() + 0.5));
    ((HUDuiList*)list[i++])->setIndex(BZDB.isTrue("coloredradarshots") ? 1 : 0);
    ((HUDuiList*)list[i++])->setIndex(static_cast<int>(BZDB.eval("linedradarshots")));
    ((HUDuiList*)list[i++])->setIndex(static_cast<int>(BZDB.eval("sizedradarshots")));
    ((HUDuiList*)list[i++])->setIndex(renderer->getRadarSize());
    ((HUDuiList*)list[i++])->setIndex(renderer->getMaxMotionFactor());
    i++; // locale
    ((HUDuiList*)list[i++])->setIndex(BZDB.isTrue("colorful") ? 1 : 0);
    ((HUDuiList*)list[i++])->setIndex(atoi(OpenGLTexFont::getUnderlineColor().c_str()));
    ((HUDuiList*)list[i++])->setIndex(static_cast<int>(BZDB.eval("killerhighlight")));
  }
}

void			GUIOptionsMenu::callback(HUDuiControl* w, void* data)
{
  HUDuiList* list = (HUDuiList*)w;

  SceneRenderer* sceneRenderer = getSceneRenderer();
  switch (((const char*)data)[0]) {
    case 'e':
      BZDB.set("enhancedradar", list->getIndex() ? "1" : "0");
      break;

    case 'w':
      BZDB.set("bigfont", list->getIndex() ? "1" : "0");
      break;

    case 'y':
    {
      sceneRenderer->setPanelOpacity(((float)list->getIndex()) / 10.0f);
      break;
    }

    case 'z':
      BZDB.set("coloredradarshots", list->getIndex() ? "1" : "0");
      break;

    case 'l':
      BZDB.set("linedradarshots", string_util::format("%d", list->getIndex()));
      break;

    case 's':
      BZDB.set("sizedradarshots", string_util::format("%d", list->getIndex()));
      break;

    case 'R':
    {
      sceneRenderer->setRadarSize(list->getIndex());
      break;
    }

    case 'M':
    {
      sceneRenderer->setMaxMotionFactor(list->getIndex());
      break;
    }

    case 'c':
    {
      BZDB.set("colorful", list->getIndex() ? "1" : "0");
      break;
    }

    case 'u':
    {
      OpenGLTexFont::setUnderlineColor(list->getIndex());
      break;
    }

    case 'k':
    {
      BZDB.set("killerhighlight", string_util::format("%d", list->getIndex()));
      break;
    }

    case 'L':
    {
      std::vector<std::string>* options = &list->getList();
      std::string locale = (*options)[list->getIndex()];

      World::setLocale(locale);
      BZDB.set("locale", locale);
      World::getBundleMgr()->getBundle(locale, true);

      GUIOptionsMenu *menu = (GUIOptionsMenu *) HUDDialogStack::get()->top();
      if (menu)
	menu->resize(menu->getWidth(), menu->getHeight());
      break;
    }
  }
}

//
// SaveWorldMenu
//

class SaveWorldMenu : public HUDDialog {
  public:
			SaveWorldMenu();
			~SaveWorldMenu();

    HUDuiDefaultKey*	getDefaultKey()
				{ return MenuDefaultKey::getInstance(); }
    void		execute();
    void		resize(int width, int height);

  private:
    HUDuiTypeIn*	filename;
    HUDuiLabel*		status;
};

SaveWorldMenu::SaveWorldMenu()
{
  // add controls
  std::vector<HUDuiControl*>& list = getControls();

  HUDuiLabel* label = new HUDuiLabel;
  label->setFont(MainMenu::getFont());
  label->setString("Save World");
  list.push_back(label);

  filename = new HUDuiTypeIn;
  filename->setFont(MainMenu::getFont());
  filename->setLabel("File Name:");
  filename->setMaxLength(255);
  list.push_back(filename);

  status = new HUDuiLabel;
  status->setFont(MainMenu::getFont());
  status->setString("");
  status->setPosition(0.5f * (float)width, status->getY());
  list.push_back(status);

  // only navigate to the file name
  initNavigation(list, 1,1);
}

SaveWorldMenu::~SaveWorldMenu()
{
}

void			SaveWorldMenu::execute()
{
  World *pWorld = World::getWorld();
  if (pWorld == NULL) {
    status->setString( "No world loaded to save" );
  } else {
    bool success = World::getWorld()->writeWorld(filename->getString());
    if (success) {
      status->setString( "File Saved" );
    } else {
      status->setString( "Error saving file" );
    }
  }
  const OpenGLTexFont& font = status->getFont();
  const float statusWidth = font.getWidth(status->getString());
  status->setPosition(0.5f * ((float)width - statusWidth), status->getY());
}

void			SaveWorldMenu::resize(int width, int height)
{
  HUDDialog::resize(width, height);

  // use a big font for title, smaller font for the rest
  const float titleFontWidth = (float)height / 12.0f;
  const float titleFontHeight = (float)height / 12.0f;

  // use a big font
  float fontWidth = (float)height / 24.0f;
  float fontHeight = (float)height / 24.0f;

  // reposition title
  std::vector<HUDuiControl*>& list = getControls();
  HUDuiLabel* title = (HUDuiLabel*)list[0];
  title->setFontSize(titleFontWidth, titleFontHeight);
  const OpenGLTexFont& titleFont = title->getFont();
  const float titleWidth = titleFont.getWidth(title->getString());
  float x = 0.5f * ((float)width - titleWidth);
  float y = (float)height - titleFont.getHeight();
  title->setPosition(x, y);

  // reposition options
  x = 0.5f * ((float)width - 0.75f * titleWidth);
  y -= 0.6f * 3 * titleFont.getHeight();
  list[1]->setFontSize(fontWidth, fontHeight);
  const float h = list[1]->getFont().getHeight();
  const int count = list.size();
  int i;
  for (i = 1; i < count-1; i++) {
    list[i]->setFontSize(fontWidth, fontHeight);
    list[i]->setPosition(x, y);
    y -= 1.0f * h;
  }

  x = 100.0f;
  y -= 100.0f;
  list[i]->setFontSize(fontWidth, fontHeight);
  list[i]->setPosition(x, y);
}

//
// OptionsMenu
//

class OptionsMenu : public HUDDialog {
  public:
			OptionsMenu();
			~OptionsMenu();

    HUDuiDefaultKey*	getDefaultKey()
				{ return MenuDefaultKey::getInstance(); }
    void		execute();
    void		resize(int width, int height);

    static void		callback(HUDuiControl* w, void* data);
    static int		gammaToIndex(float);
    static float	indexToGamma(int);

  private:
    HUDuiControl*	videoFormat;
    HUDuiControl*	keyMapping;
    HUDuiControl*	guiOptions;
    HUDuiControl*	clearCache;
    HUDuiControl*	saveWorld;
    FormatMenu*		formatMenu;
    KeyboardMapMenu*	keyboardMapMenu;
    GUIOptionsMenu*	guiOptionsMenu;
    SaveWorldMenu*	saveWorldMenu;
};

OptionsMenu::OptionsMenu() : formatMenu(NULL), keyboardMapMenu(NULL),
			     guiOptionsMenu(NULL), saveWorldMenu(NULL)
{
  // add controls
  std::vector<HUDuiControl*>& list = getControls();

  HUDuiLabel* label = new HUDuiLabel;
  label->setFont(MainMenu::getFont());
  label->setString("Options");
  list.push_back(label);

  HUDuiList* option = new HUDuiList;
  option->setFont(MainMenu::getFont());
  option->setLabel("Dithering:");
  option->setCallback(callback, (void*)"1");
  std::vector<std::string>* options = &option->getList();
  options->push_back(std::string("Off"));
  options->push_back(std::string("On"));
  option->update();
  list.push_back(option);

  option = new HUDuiList;
  option->setFont(MainMenu::getFont());
  option->setLabel("Blending:");
  option->setCallback(callback, (void*)"2");
  options = &option->getList();
  options->push_back(std::string("Off"));
  options->push_back(std::string("On"));
  option->update();
  list.push_back(option);

  option = new HUDuiList;
  option->setFont(MainMenu::getFont());
  option->setLabel("Smoothing:");
  option->setCallback(callback, (void*)"3");
  options = &option->getList();
  options->push_back(std::string("Off"));
  options->push_back(std::string("On"));
  option->update();
  list.push_back(option);

  option = new HUDuiList;
  option->setFont(MainMenu::getFont());
  option->setLabel("Lighting:");
  option->setCallback(callback, (void*)"4");
  options = &option->getList();
  options->push_back(std::string("Off"));
  options->push_back(std::string("On"));
  option->update();
  list.push_back(option);

  option = new HUDuiList;
  option->setFont(MainMenu::getFont());
  option->setLabel("Texturing:");
  option->setCallback(callback, (void*)"5");
  options = &option->getList();
  options->push_back(std::string("Off"));
  options->push_back(std::string("Nearest"));
  options->push_back(std::string("Linear"));
  options->push_back(std::string("Nearest Mipmap Nearest"));
  options->push_back(std::string("Linear Mipmap Nearest"));
  options->push_back(std::string("Nearest Mipmap Linear"));
  options->push_back(std::string("Linear Mipmap Linear"));
  option->update();
  list.push_back(option);

  option = new HUDuiList;
  option->setFont(MainMenu::getFont());
  option->setLabel("Quality:");
  option->setCallback(callback, (void*)"6");
  options = &option->getList();
  options->push_back(std::string("Low"));
  options->push_back(std::string("Medium"));
  options->push_back(std::string("High"));
  options->push_back(std::string("Experimental"));
  option->update();
  list.push_back(option);

  option = new HUDuiList;
  option->setFont(MainMenu::getFont());
  option->setLabel("Shadows:");
  option->setCallback(callback, (void*)"7");
  options = &option->getList();
  options->push_back(std::string("Off"));
  options->push_back(std::string("On"));
  option->update();
  list.push_back(option);

  option = new HUDuiList;
  option->setFont(MainMenu::getFont());
  option->setLabel("Depth Buffer:");
  option->setCallback(callback, (void*)"8");
  options = &option->getList();
  GLint value;
  glGetIntegerv(GL_DEPTH_BITS, &value);
  if (value == 0) {
    options->push_back(std::string("Not available"));
  }
  else {
    options->push_back(std::string("Off"));
    options->push_back(std::string("On"));
  }
  option->update();
  list.push_back(option);

#if defined(DEBUG_RENDERING)
  option = new HUDuiList;
  option->setFont(MainMenu::getFont());
  option->setLabel("Hidden Line:");
  option->setCallback(callback, (void*)"a");
  options = &option->getList();
  options->push_back(std::string("Off"));
  options->push_back(std::string("On"));
  option->update();
  list.push_back(option);

  option = new HUDuiList;
  option->setFont(MainMenu::getFont());
  option->setLabel("Wireframe:");
  option->setCallback(callback, (void*)"b");
  options = &option->getList();
  options->push_back(std::string("Off"));
  options->push_back(std::string("On"));
  option->update();
  list.push_back(option);

  option = new HUDuiList;
  option->setFont(MainMenu::getFont());
  option->setLabel("Depth Complexity:");
  option->setCallback(callback, (void*)"c");
  options = &option->getList();
  options->push_back(std::string("Off"));
  options->push_back(std::string("On"));
  option->update();
  list.push_back(option);
#endif

  BzfDisplay* display = getDisplay();
  int numFormats = display->getNumResolutions();
  if (numFormats < 2) {
    videoFormat = NULL;
  }
  else {
    videoFormat = label = new HUDuiLabel;
    label->setFont(MainMenu::getFont());
    label->setLabel("Change Video Format");
    list.push_back(label);
  }

  BzfWindow* window = getMainWindow()->getWindow();
  option = new HUDuiList;
  option->setFont(MainMenu::getFont());
  option->setLabel("Brightness:");
  option->setCallback(callback, (void*)"g");
  if (window->hasGammaControl()) {
    option->createSlider(15);
  }
  else {
    options = &option->getList();
    options->push_back(std::string("Unavailable"));
  }
  option->update();
  list.push_back(option);

  option = new HUDuiList;
  option->setFont(MainMenu::getFont());
  option->setLabel("Sound Volume:");
  option->setCallback(callback, (void*)"s");
  options = &option->getList();
  if (isSoundOpen()) {
    options->push_back(std::string("Off"));
    options->push_back(std::string("1"));
    options->push_back(std::string("2"));
    options->push_back(std::string("3"));
    options->push_back(std::string("4"));
    options->push_back(std::string("5"));
    options->push_back(std::string("6"));
    options->push_back(std::string("7"));
    options->push_back(std::string("8"));
    options->push_back(std::string("9"));
    options->push_back(std::string("10"));
  }
  else {
    options->push_back(std::string("Unavailable"));
  }
  option->update();
  list.push_back(option);

  option = new HUDuiList;
  option->setFont(MainMenu::getFont());
  option->setLabel("UDP network connection:");
  option->setCallback(callback, (void*)"U");
  options = &option->getList();
  options->push_back(std::string("Off"));
  options->push_back(std::string("On"));
  option->update();
  list.push_back(option);

  option = new HUDuiList;
  option->setFont(MainMenu::getFont());
  option->setLabel("Server List Cache:");
  option->setCallback(callback, (void*)"S");
  options = &option->getList();
  options->push_back(std::string("Off / Backup Mode"));
  options->push_back(std::string("5 Minutes"));
  options->push_back(std::string("15 Minutes"));
  options->push_back(std::string("30 Minutes"));
  options->push_back(std::string("1 Hour"));
  options->push_back(std::string("5 Hours"));
  options->push_back(std::string("15 Hours"));
  options->push_back(std::string("1 day"));
  options->push_back(std::string("15 days"));
  options->push_back(std::string("30 days"));
  option->update();
  list.push_back(option);

  clearCache = label = new HUDuiLabel;
  label->setFont(MainMenu::getFont());
  label->setLabel("Clear Server List Cache");
  list.push_back(label);

  keyMapping = label = new HUDuiLabel;
  label->setFont(MainMenu::getFont());
  label->setLabel("Change Key Mapping");
  list.push_back(label);

  guiOptions = label = new HUDuiLabel;
  label->setFont(MainMenu::getFont());
  label->setLabel("GUI Options");
  list.push_back(label);

  saveWorld = label = new HUDuiLabel;
  label->setFont(MainMenu::getFont());
  label->setLabel("Save World");
  list.push_back(label);

  initNavigation(list, 1,list.size()-1);
}

OptionsMenu::~OptionsMenu()
{
  delete formatMenu;
  delete keyboardMapMenu;
  delete guiOptionsMenu;
  delete saveWorldMenu;
}

void			OptionsMenu::execute()
{
  HUDuiControl* focus = HUDui::getFocus();
  if (focus == videoFormat) {
    if (!formatMenu) formatMenu = new FormatMenu;
    HUDDialogStack::get()->push(formatMenu);
  }
  else if (focus == keyMapping) {
    if (!keyboardMapMenu) keyboardMapMenu = new KeyboardMapMenu;
    HUDDialogStack::get()->push(keyboardMapMenu);
  }
  else if (focus == guiOptions) {
    if (!guiOptionsMenu) guiOptionsMenu = new GUIOptionsMenu;
    HUDDialogStack::get()->push(guiOptionsMenu);
  }
  else if (focus == clearCache) {
    if ((ServerListCache::get())->clearCache()){
       controlPanel->addMessage("Cache Cleared");
    } else {
      // already cleared -- do nothing
    }
  }
  else if (focus == saveWorld) {
    if (!saveWorldMenu) saveWorldMenu = new SaveWorldMenu;
    HUDDialogStack::get()->push(saveWorldMenu);
  }
}

void			OptionsMenu::resize(int width, int height)
{
  int i;
  HUDDialog::resize(width, height);

  // use a big font for title, smaller font for the rest
  const float titleFontWidth = (float)height / 10.0f;
  const float titleFontHeight = (float)height / 10.0f;
  const float fontWidth = (float)height / 36.0f;
  const float fontHeight = (float)height / 36.0f;

  // reposition title
  std::vector<HUDuiControl*>& list = getControls();
  HUDuiLabel* title = (HUDuiLabel*)list[0];
  title->setFontSize(titleFontWidth, titleFontHeight);
  const OpenGLTexFont& titleFont = title->getFont();
  const float titleWidth = titleFont.getWidth(title->getString());
  float x = 0.5f * ((float)width - titleWidth);
  float y = (float)height - titleFont.getHeight();
  title->setPosition(x, y);

  // reposition options in two columns
  x = 0.3f * (float)width;
  const float topY = y - 0.6f * titleFont.getHeight();
  y = topY;
  list[1]->setFontSize(fontWidth, fontHeight);
  const float h = list[1]->getFont().getHeight();
  const int count = list.size();
  const int mid = count / 2;

  for (i = 1; i <= mid; i++) {
    list[i]->setFontSize(fontWidth, fontHeight);
    list[i]->setPosition(x, y);
    y -= 1.0f * h;
  }

  x = 0.80f * (float)width;
  y = topY;
  for (i = mid + 1; i < count; i++) {
    list[i]->setFontSize(fontWidth, fontHeight);
    list[i]->setPosition(x, y);
    y -= 1.0f * h;
  }

  // load current settings
  SceneRenderer* renderer = getSceneRenderer();
  if (renderer) {
    HUDuiList* tex;
    int i = 1;
    ((HUDuiList*)list[i++])->setIndex(BZDB.isTrue("dither"));
    ((HUDuiList*)list[i++])->setIndex(BZDBCache::blend);
    ((HUDuiList*)list[i++])->setIndex(BZDB.isTrue("smooth"));
    ((HUDuiList*)list[i++])->setIndex(BZDB.isTrue("lighting"));
    tex = (HUDuiList*)list[i++];
    ((HUDuiList*)list[i++])->setIndex(renderer->useQuality());
    ((HUDuiList*)list[i++])->setIndex(BZDB.isTrue("shadows"));
    ((HUDuiList*)list[i++])->setIndex(BZDB.isTrue("zbuffer"));
#if defined(DEBUG_RENDERING)
    ((HUDuiList*)list[i++])->setIndex(renderer->useHiddenLine() ? 1 : 0);
    ((HUDuiList*)list[i++])->setIndex(renderer->useWireframe() ? 1 : 0);
    ((HUDuiList*)list[i++])->setIndex(renderer->useDepthComplexity() ? 1 : 0);
#endif

    if (videoFormat)
      i++;

    // brightness
    BzfWindow* window = getMainWindow()->getWindow();
    if (window->hasGammaControl())
      ((HUDuiList*)list[i])->setIndex(gammaToIndex(window->getGamma()));
    i++;

    // sound
    ((HUDuiList*)list[i++])->setIndex(getSoundVolume());

    const StartupInfo* info = getStartupInfo();

    // mind the ++i !
    ((HUDuiList*)list[i++])->setIndex(info->useUDPconnection ? 1 : 0);

    if (!BZDB.isTrue("texture"))
      tex->setIndex(0);
    else
      tex->setIndex(OpenGLTexture::getFilter());

    // server cache age
    int index = 0;
    switch ((ServerListCache::get())->getMaxCacheAge()){
      case 0: index = 0; break;
      case 5: index = 1; break;
      case 15: index = 2; break;
      case 30: index = 3; break;
      case 60: index = 4; break;
      case 60*5: index = 5; break;
      case 60*15: index = 6; break;
      case 60*24: index = 7; break;
      case 60*24*15: index = 8; break;
      case 60*24*30: index = 9; break;
      default: index = 4;
    }
    ((HUDuiList*)list[i++])->setIndex(index);
    i++; // clear cache label
  }
}

void			OptionsMenu::callback(HUDuiControl* w, void* data)
{
  HUDuiList* list = (HUDuiList*)w;

  SceneRenderer* sceneRenderer = getSceneRenderer();
  switch (((const char*)data)[0]) {
    case '1':
      BZDB.set("dither", list->getIndex() ? "1" : "0");
      sceneRenderer->notifyStyleChange();
      break;

    case '2':
      BZDB.set("blend", list->getIndex() ? "1" : "0");
      sceneRenderer->notifyStyleChange();
      break;

    case '3':
      BZDB.set("smooth", list->getIndex() ? "1" : "0");
      sceneRenderer->notifyStyleChange();
      break;

    case '4':
      BZDB.set("lighting", list->getIndex() ? "1" : "0");

      BZDB.set("_texturereplace", (!BZDB.isTrue("lighting") &&
		sceneRenderer->useQuality() < 2) ? "1" : "0");
      BZDB.setPersistent("_texturereplace", false);
      sceneRenderer->notifyStyleChange();
      break;

    case '5':
      OpenGLTexture::setFilter((OpenGLTexture::Filter)list->getIndex());
      BZDB.set("texture", OpenGLTexture::getFilterName());
      sceneRenderer->notifyStyleChange();
      break;

    case '6':
      sceneRenderer->setQuality(list->getIndex());

      BZDB.set("_texturereplace", (!BZDB.isTrue("lighting") &&
        sceneRenderer->useQuality() < 2) ? "1" : "0");
      BZDB.setPersistent("_texturereplace", false);
      sceneRenderer->notifyStyleChange();
      break;

    case '7':
      BZDB.set("shadows", list->getIndex() ? "1" : "0");
      sceneRenderer->notifyStyleChange();
      break;

    case '8':
      BZDB.set("zbuffer", list->getIndex() ? "1" : "0");
      // FIXME - test for whether the z buffer will work
      setSceneDatabase();
      sceneRenderer->notifyStyleChange();
      break;

    case 's':
      BZDB.set("volume", string_util::format("%d", list->getIndex()));
      setSoundVolume(list->getIndex());
      break;

    case 'U': {
		StartupInfo* info = getStartupInfo();
		info->useUDPconnection = (list->getIndex() != 0);
	}
      break;

    case 'g': {
      BzfWindow* window = getMainWindow()->getWindow();
      if (window->hasGammaControl())
	window->setGamma(indexToGamma(list->getIndex()));
      break;
    }

    case 'S': { // server cache
      time_t minutes = 0;
      int index = list->getIndex();
      switch (index){
	case 0: minutes = 0; break;
	case 1: minutes = 5; break;
	case 2: minutes = 15; break;
	case 3: minutes = 30; break;
	case 4: minutes = 60; break;
	case 5: minutes = 60*5; break;
	case 6: minutes = 60*15; break;
	case 7: minutes = 60*24; break;
	case 8: minutes = 60*24*15; break;
	case 9: minutes = 60*24*30; break;
      }
      (ServerListCache::get())->setMaxCacheAge(minutes);
      break;
    }

#if defined(DEBUG_RENDERING)
    case 'a':
      sceneRenderer->setHiddenLine(list->getIndex() != 0);
      break;

    case 'b':
      sceneRenderer->setWireframe(list->getIndex() != 0);
      break;

    case 'c':
      sceneRenderer->setDepthComplexity(list->getIndex() != 0);
      break;
#endif

    case 'r':
      // do nothing -- wait for enter or t key
      break;
  }
}

int			OptionsMenu::gammaToIndex(float gamma)
{
    return (int)(0.5f + 5.0f * (1.0f + logf(gamma) / logf(2.0)));
}

float			OptionsMenu::indexToGamma(int index)
{
    // map index 5 to gamma 1.0 and index 0 to gamma 0.5
    return powf(2.0f, (float)index / 5.0f - 1.0f);
}

//
// HelpMenu
//

class HelpMenu;

class HelpMenuDefaultKey : public MenuDefaultKey {
  public:
			HelpMenuDefaultKey() { }
			~HelpMenuDefaultKey() { }

    bool		keyPress(const BzfKeyEvent&);
    bool		keyRelease(const BzfKeyEvent&);
};

class HelpMenu : public HUDDialog {
  public:
			HelpMenu(const char* title = "Help");
			~HelpMenu() { }

    HUDuiDefaultKey*	getDefaultKey() { return &defaultKey; }
    void		execute() { }
    void		resize(int width, int height);

   static HelpMenu*	getHelpMenu(HUDDialog* = NULL, bool next = true);
   static void		done();

  protected:
    HUDuiControl*	createLabel(const char* string,
				const char* label = NULL);
    virtual float	getLeftSide(int width, int height);

  private:
    HelpMenuDefaultKey			defaultKey;
    static HelpMenu**			helpMenus;
};

bool			HelpMenuDefaultKey::keyPress(const BzfKeyEvent& key)
{
  if (key.button == BzfKeyEvent::PageUp) {
    HUDDialog* oldMenu = HUDDialogStack::get()->top();
    HUDDialogStack::get()->pop();
    HUDDialogStack::get()->push(HelpMenu::getHelpMenu(oldMenu, false));
    return true;
  }
  if (key.button == BzfKeyEvent::PageDown || key.ascii == 13) {
    HUDDialog* oldMenu = HUDDialogStack::get()->top();
    HUDDialogStack::get()->pop();
    HUDDialogStack::get()->push(HelpMenu::getHelpMenu(oldMenu, true));
    return true;
  }
  return MenuDefaultKey::keyPress(key);
}

bool			HelpMenuDefaultKey::keyRelease(const BzfKeyEvent& key)
{
  if (key.button == BzfKeyEvent::PageUp ||
      key.button == BzfKeyEvent::PageDown || key.ascii == 13)
    return true;
  return MenuDefaultKey::keyRelease(key);
}

HelpMenu::HelpMenu(const char* title) : HUDDialog()
{
  // add controls
  std::vector<HUDuiControl*>& list = getControls();
  list.push_back(createLabel(title));
  list.push_back(createLabel("Page Down for next page",
			  "Page Up for previous page"));


  initNavigation(list, 1, 1);
}

HUDuiControl*		HelpMenu::createLabel(const char* string,
				const char* label)
{
  HUDuiLabel* control = new HUDuiLabel;
  control->setFont(MainMenu::getFont());
  if (string) control->setString(string);
  if (label) control->setLabel(label);
  return control;
}

float			HelpMenu::getLeftSide(int, int height)
{
  return (float)height / 6.0f;
}

void			HelpMenu::resize(int width, int height)
{
  HUDDialog::resize(width, height);

  // use a big font for title, smaller font for the rest
  const float titleFontWidth = (float)height / 15.0f;
  const float titleFontHeight = (float)height / 15.0f;
  const float fontWidth = (float)height / 48.0f;
  const float fontHeight = (float)height / 48.0f;

  // reposition title
  std::vector<HUDuiControl*>& list = getControls();
  HUDuiLabel* title = (HUDuiLabel*)list[0];
  title->setFontSize(titleFontWidth, titleFontHeight);
  const OpenGLTexFont& titleFont = title->getFont();
  const float titleWidth = titleFont.getWidth(title->getString());
  float x = 0.5f * ((float)width - titleWidth);
  float y = (float)height - titleFont.getHeight();
  title->setPosition(x, y);

  // position focus holder off screen
  list[1]->setFontSize(fontWidth, fontHeight);
  const float h = list[1]->getFont().getHeight();
  y -= 1.25f * h;
  list[1]->setPosition(0.5f * ((float)width + h), y);

  // reposition options
  x = getLeftSide(width, height);
  y -= 1.5f * h;
  const int count = list.size();
  for (int i = 2; i < count; i++) {
    list[i]->setFontSize(fontWidth, fontHeight);
    list[i]->setPosition(x, y);
    y -= 1.0f * h;
  }
}

//
// Help1Menu
//

class Help1Menu : public HelpMenu {
  public:
			Help1Menu();
			~Help1Menu() { }

    void		resize(int width, int height);

    void		onScan(const std::string& name, bool, const std::string&);
    static void		onScanCB(const std::string& name, bool press,
				 const std::string& cmd, void* userData);

  protected:
    float		getLeftSide(int width, int height);

  private:
    void		initKeymap(const std::string& name, int index);
    struct keymap {
      int index;	// ui label index
      std::string key1;
      std::string key2;
    };
    typedef std::map<std::string, keymap> KeyKeyMap;
    KeyKeyMap	mappable;
};

Help1Menu::Help1Menu() : HelpMenu("Controls")
{
  // add controls
  std::vector<HUDuiControl*>& list = getControls();
  list.push_back(createLabel("Mouse Position", "Controls Tank Position:"));
  list.push_back(createLabel(NULL, "Fires Shot:"));
  list.push_back(createLabel(NULL, "Drops Flag (if not bad):"));
  list.push_back(createLabel(NULL, "Identifies Player (locks on GM):"));
  list.push_back(createLabel(NULL, "Short Radar Range:"));
  list.push_back(createLabel(NULL, "Medium Radar Range:"));
  list.push_back(createLabel(NULL, "Long Radar Range:"));
  list.push_back(createLabel(NULL, "Send Message to Everybody:"));
  list.push_back(createLabel(NULL, "Send Message to Teammates:"));
  list.push_back(createLabel(NULL, "Send Message to Nemesis:"));
  list.push_back(createLabel(NULL, "Send Message to Recipient:"));
  list.push_back(createLabel(NULL, "Jump (if allowed):"));
  list.push_back(createLabel(NULL, "Toggle Binoculars:"));
  list.push_back(createLabel(NULL, "Toggle Score Sheet:"));
  list.push_back(createLabel(NULL, "Toggle Tank Labels:"));
  list.push_back(createLabel(NULL, "Toggle Heads-up Flag Help:"));
  list.push_back(createLabel(NULL, "Set Time of Day Backward:"));
  list.push_back(createLabel(NULL, "Set Time of Day Forward:"));
  list.push_back(createLabel(NULL, "Pause/Resume:"));
  list.push_back(createLabel(NULL, "Self destruct/Cancel:"));
  list.push_back(createLabel(NULL, "Quit:"));
  list.push_back(createLabel(NULL, "Scroll Message Log Backward:"));
  list.push_back(createLabel(NULL, "Scroll Message Log Forward:"));
  list.push_back(createLabel(NULL, "Slow Keyboard Motion:"));
  list.push_back(createLabel(NULL, "Toggle Radar Flags:"));
  list.push_back(createLabel(NULL, "Toggle Main Flags:"));
  list.push_back(createLabel(NULL, "Silence/UnSilence:"));
  list.push_back(createLabel(NULL, "Server Admin:"));
  list.push_back(createLabel(NULL, "Hunt:"));
  list.push_back(createLabel(NULL, "Auto Pilot:"));
  list.push_back(createLabel("Esc", "Show/Dismiss menu:"));

  initKeymap("fire", 3);
  initKeymap("drop", 4);
  initKeymap("identify", 5);
  initKeymap("set displayRadarRange 0.25", 6);
  initKeymap("set displayRadarRange 0.5", 7);
  initKeymap("set displayRadarRange 1.0", 8);
  initKeymap("send all", 9);
  initKeymap("send team", 10);
  initKeymap("send nemesis", 11);
  initKeymap("send recipient", 12);
  initKeymap("jump", 13);
  initKeymap("toggle displayBinoculars", 14);
  initKeymap("toggle displayScore", 15);
  initKeymap("toggle displayLabels", 16);
  initKeymap("toggle displayFlagHelp", 17);
  initKeymap("time backward", 18);
  initKeymap("time forward", 19);
  initKeymap("pause", 20);
  initKeymap("destruct", 21);
  initKeymap("quit", 22);
  initKeymap("scrollpanel up", 23);
  initKeymap("scrollpanel down", 24);
  initKeymap("toggle slowKeyboard", 25);
  initKeymap("toggle displayRadarFlags", 26);
  initKeymap("toggle displayMainFlags", 27);
  initKeymap("silence", 28);
  initKeymap("servercommand", 29);
  initKeymap("hunt", 30);
  initKeymap("autopilot", 31);
}

void			Help1Menu::onScan(const std::string& name, bool press,
					  const std::string& cmd)
{
  if (!press)
    return;
  KeyKeyMap::iterator it = mappable.find(cmd);
  if (it == mappable.end())
    return;
  if (it->second.key1.empty())
    it->second.key1 = name;
  else if (it->second.key2.empty())
    it->second.key2 = name;
}

void		Help1Menu::onScanCB(const std::string& name, bool press,
			 const std::string& cmd, void* userData)
{
  reinterpret_cast<Help1Menu*>(userData)->onScan(name, press, cmd);
}

void			Help1Menu::initKeymap(const std::string& name, int index)
{
  mappable[name].key1 = "";
  mappable[name].key2 = "";
  mappable[name].index = index;
}

float			Help1Menu::getLeftSide(int width, int height)
{
  return 0.5f * width - height / 20.0f;
}

void			Help1Menu::resize(int width, int height)
{
  // get current key mapping and set strings appropriately
  KeyKeyMap::iterator it;
  // clear
  for (it = mappable.begin(); it != mappable.end(); it++) {
    it->second.key1 = "";
    it->second.key2 = "";
  }
  // load current settings
  KEYMGR.iterate(&onScanCB, this);
  std::vector<HUDuiControl*>& list = getControls();
  for (it = mappable.begin(); it != mappable.end(); it++) {
    std::string value = "";
    if (it->second.key1.empty()) {
      value = "<not mapped>";
    } else {
      value += it->second.key1;
      if (!it->second.key2.empty())
	value += " or " + it->second.key2;
    }
    ((HUDuiLabel*)list[it->second.index])->setString(value);
  }

  // now do regular resizing
  HelpMenu::resize(width, height);
}

//
// Help2Menu
//

class Help2Menu : public HelpMenu {
  public:
			Help2Menu();
			~Help2Menu() { }
};

Help2Menu::Help2Menu() : HelpMenu("General")
{
  // add controls
  std::vector<HUDuiControl*>& list = getControls();
  list.push_back(createLabel(
	"BZFlag is a multi-player networked tank battle game.  There are five teams:"));
  list.push_back(createLabel(
	"red, green, blue, purple, and rogues (rogue tanks are black).  Destroying a"));
  list.push_back(createLabel(
	"player on another team scores a win, while being destroyed or destroying a"));
  list.push_back(createLabel(
	"teammate scores a loss.  Individual and aggregate team scores are tallied."));
  list.push_back(createLabel(
	"Rogues have no teammates (not even other rogues),so they cannot shoot"));
  list.push_back(createLabel(
	"teammates and they don't have a team score."));
  list.push_back(createLabel(""));
  list.push_back(createLabel(
	"There are three styles of play, determined by the server configuration:  capture-"));
  list.push_back(createLabel(
	"the-flag, rabbit-chase and free-for-all.  In free-for-all the object is simply to get the"));
  list.push_back(createLabel(
	"highest score by shooting opponents.  The object in rabbit chase is to be the highest score"));
  list.push_back(createLabel(
	"so that you have the white tank, then everyone is against you. The object in capture-the-flag is to"));
  list.push_back(createLabel(
	"capture enemy flags while preventing opponents from capturing yours.  In this"));
  list.push_back(createLabel(
	"style, each team (but not rogues) has a team base and each team with at least"));
  list.push_back(createLabel(
	"one player has a team flag which has the color of the team.  To capture a flag,"));
  list.push_back(createLabel(
	"you must grab it and bring it back to your team base (you must be on the ground"));
  list.push_back(createLabel(
	"in your base to register the capture).  Capturing a flag destroys all the players"));
  list.push_back(createLabel(
	"on that team and gives your team score a bonus;  the players will restart on"));
  list.push_back(createLabel(
	"their team base.  Taking your flag onto an enemy base counts as a capture against"));
  list.push_back(createLabel(
	"your team but not for the enemy team."));
}

//
// Help3Menu
//

class Help3Menu : public HelpMenu {
  public:
			Help3Menu();
			~Help3Menu() { }
};

Help3Menu::Help3Menu() : HelpMenu("Environment")
{
  // add controls
  std::vector<HUDuiControl*>& list = getControls();
  list.push_back(createLabel(
	"The world environment contains an outer wall and several buildings."));
  list.push_back(createLabel(
	"You cannot go outside the outer wall (you can't even jump over it)."));
  list.push_back(createLabel(
	"You cannot normally drive or shoot through buildings."));
  list.push_back(createLabel(
	""));
  list.push_back(createLabel(
	"The server may be configured to include teleporters:  large transparent"));
  list.push_back(createLabel(
	"black slabs.  Objects entering one side of a teleporter are instantly"));
  list.push_back(createLabel(
	"moved to one side of another (or possibly the same) teleporter.  The"));
  list.push_back(createLabel(
	"teleport is reversible;  reentering the same side of the destination"));
  list.push_back(createLabel(
	"teleporter brings you back to where you started.  Teleport connections"));
  list.push_back(createLabel(
	"are fixed at the start of the game and don't change during the game."));
  list.push_back(createLabel(
	"The connections are always the same in the capture-the-flag style."));
  list.push_back(createLabel(
	"Each side of a teleporter teleports independently of the other side."));
  list.push_back(createLabel(
	"It's possible for a teleporter to teleport to the opposite side of"));
  list.push_back(createLabel(
	"itself.  Such a thru-teleporter acts almost as if it wasn't there."));
  list.push_back(createLabel(
	"A teleporter can also teleport to the same side of itself.  This is a"));
  list.push_back(createLabel(
	"reverse teleporter.  Shooting at a reverse teleporter is likely to be"));
  list.push_back(createLabel(
	"self destructive;  shooting a laser at one is invariably fatal."));
}

//
// Help4Menu
//

class Help4Menu : public HelpMenu {
  public:
			Help4Menu();
			~Help4Menu() { }
};

Help4Menu::Help4Menu() : HelpMenu("Flags I")
{
  // add controls
  std::vector<HUDuiControl*>& list = getControls();
  list.push_back(createLabel(
	"Flags come in two varieties:  team flags and super flags.  Team flags"));
  list.push_back(createLabel(
	"are used only in the capture-the-flag style.  The server may also be"));
  list.push_back(createLabel(
	"configured to supply super flags, which give your tank some advantage"));
  list.push_back(createLabel(
	"or disadvantage.  You normally can't tell which until you pick one up,"));
  list.push_back(createLabel(
	"but good flags generally outnumber bad flags two to one."));
  list.push_back(createLabel(
	""));
  list.push_back(createLabel(
	"Team flags are not allowed to be in Bad Places.  Bad Places are:  on"));
  list.push_back(createLabel(
	"a building or on an enemy base.  Team flags dropped in a Bad Place are"));
  list.push_back(createLabel(
	"moved to a safety position.  Captured flags are placed back on their"));
  list.push_back(createLabel(
	"team base.  Super flags dropped above a building always disappear."));
  list.push_back(createLabel(
	""));
  list.push_back(createLabel(
	"A random good super flag will remain for up to 4 possessions.  After"));
  list.push_back(createLabel(
	"that it'll disappear and will eventually be replaced by a new random"));
  list.push_back(createLabel(
	"flag.  Bad random super flags disappear after the first possession."));
  list.push_back(createLabel(
	"Bad super flags can't normally be dropped.  The server can be set to"));
  list.push_back(createLabel(
	"automatically drop the flag for you after some time, after you destroy"));
  list.push_back(createLabel(
	"a certain number of enemies, and/or when you grab an antidote flag."));
  list.push_back(createLabel(
	"Antidote flags are yellow and only appear when you have a bad flag."));
}

//
// Help5Menu
//

class Help5Menu : public HelpMenu {
  public:
			Help5Menu();
			~Help5Menu() { }

  protected:
    float		getLeftSide(int width, int height);
};

Help5Menu::Help5Menu() : HelpMenu("Flags II")
{
  // add controls
  std::vector<HUDuiControl*>& list = getControls();
  list.push_back(createLabel(
	"", "Good Flags:"));
  list.push_back(createLabel(
	"boosts top speed", "High Speed (V)"));
  list.push_back(createLabel(
	"boosts turn rate", "Quick Turn (A)"));
  list.push_back(createLabel(
	"can drive through buildings", "Oscillation Overthruster (OO)"));
  list.push_back(createLabel(
	"faster shots more often", "Rapid Fire (F)"));
  list.push_back(createLabel(
	"very fast reload, very short range", "Machine Gun (MG)"));
  list.push_back(createLabel(
	"shots guide themselves (right mouse locks on)", "Guided Missile (GM)"));
  list.push_back(createLabel(
	"infinite shot speed and range, long reload time", "Laser (L)"));
  list.push_back(createLabel(
	"shots ricochet", "Ricochet (R)"));
  list.push_back(createLabel(
	"shoots through buildings", "Super Bullet (SB)"));
  list.push_back(createLabel(
	"tank invisible on enemy radar", "Stealth (ST)"));
  list.push_back(createLabel(
	"tank invisible out the window", "Cloaking (CL)"));
  list.push_back(createLabel(
	"shots invisible on radar", "Invisible Bullet (IB)"));
  list.push_back(createLabel(
	"tank becomes smaller", "Tiny (T)"));
  list.push_back(createLabel(
	"tank becomes paper thin", "Narrow (N)"));
  list.push_back(createLabel(
	"getting hit just drops the flag", "Shield (SH)"));
  list.push_back(createLabel(
	"destroy tanks by touching them", "Steamroller (SR)"));
  list.push_back(createLabel(
	"expanding spherical shell of destruction", "Shock Wave (SW)"));
}

float			Help5Menu::getLeftSide(int width, int)
{
  return 0.35f * width;
}

//
// Help6Menu
//

class Help6Menu : public HelpMenu {
  public:
			Help6Menu();
			~Help6Menu() { }

  protected:
    float		getLeftSide(int width, int height);
};

Help6Menu::Help6Menu() : HelpMenu("Flags III")
{
  // add controls
  std::vector<HUDuiControl*>& list = getControls();
  list.push_back(createLabel(
	"teleport to enter/leave zone", "Phantom Zone (PZ)"));
  list.push_back(createLabel(
	"destroys player and all player's teammates", "Genocide (G)"));
  list.push_back(createLabel(
	"allows tank to jump", "Jumping (JP)"));
  list.push_back(createLabel(
	"shows type of nearest superflag", "Identify (ID)"));
  list.push_back(createLabel(
	"it's useless", "Useless (US)"));
  list.push_back(createLabel(
	"In opponent's hud, you appear as a teammate", "Masquerade (MQ)"));
  list.push_back(createLabel(
	"See stealthed, cloaked and masquerading tanks as normal.", "Seer (SE)"));
  list.push_back(createLabel(
	"Steal flags.  Small and fast but can't kill.", "Thief (TH)"));
  list.push_back(createLabel(
	"Tank burrows underground, impervious to normal shots, but can be steamrolled by anyone!", "Burrow (BU)"));
  list.push_back(createLabel(
	"", ""));
  list.push_back(createLabel(
	"", "Bad Flags:"));
  list.push_back(createLabel(
	"can't identify tanks", "Colorblindness (CB)"));
  list.push_back(createLabel(
	"makes tank very large", "Obesity (O)"));
  list.push_back(createLabel(
	"tank can't turn right", "Left Turn Only (<-)"));
  list.push_back(createLabel(
	"tank can't turn left", "Right Turn Only (->)"));
  list.push_back(createLabel(
	"tank has lots of momentum", "Momentum (M)"));
  list.push_back(createLabel(
	"can't see out the window", "Blindness (B)"));
  list.push_back(createLabel(
	"can't see anything on radar", "Jamming (JM)"));
  list.push_back(createLabel(
	"fish eye view out the window", "Wide Angle (WA)"));
}

float			Help6Menu::getLeftSide(int width, int)
{
  return 0.35f * width;
}

//
// Help7Menu
//

class Help7Menu : public HelpMenu {
  public:
			Help7Menu();
			~Help7Menu() { }
};

Help7Menu::Help7Menu() : HelpMenu("Readouts I")
{
  // add controls
  std::vector<HUDuiControl*>& list = getControls();
  list.push_back(createLabel(
	"The radar is on the left side of the control panel.  It shows an overhead"));
  list.push_back(createLabel(
	"x-ray view of the game.  Buildings and the outer wall are shown in light"));
  list.push_back(createLabel(
	"blue.  Team bases are outlined in the team color.  Teleporters are short"));
  list.push_back(createLabel(
	"yellow lines.  Tanks are dots in the tank's team color, except rogues are"));
  list.push_back(createLabel(
	"yellow.  The size of the tank's dot is a rough indication of the tank's"));
  list.push_back(createLabel(
	"altitude:  higher tanks have larger dots.  Flags are small crosses.  Team"));
  list.push_back(createLabel(
	"flags are in the team color, superflags are white, and the antidote flag"));
  list.push_back(createLabel(
	"is yellow.  Shots are small dots (or lines or circles, for lasers and"));
  list.push_back(createLabel(
	"shock waves, respectively).  Your tank is always dead center and forward"));
  list.push_back(createLabel(
	"is always up on the radar.  The yellow V is your field of view.  North"));
  list.push_back(createLabel(
	"is indicated by the letter N."));
  list.push_back(createLabel(
	""));
  list.push_back(createLabel(
	"The heads-up-display (HUD) has several displays.  The two boxes in the"));
  list.push_back(createLabel(
	"center of the view are the motion control boxes;  within the small box"));
  list.push_back(createLabel(
	"your tank won't move, outside the large box you don't move any faster than"));
  list.push_back(createLabel(
	"at the edge of the large box.  Moving the mouse above or below the small"));
  list.push_back(createLabel(
	"box moves forward or backward, respectively.  Similarly for left and right."));
  list.push_back(createLabel(
	"The distance away from the small box determines the speed."));
}

//
// Help8Menu
//

class Help8Menu : public HelpMenu {
  public:
			Help8Menu();
			~Help8Menu() { }
};

Help8Menu::Help8Menu() : HelpMenu("Readouts II")
{
  // add controls
  std::vector<HUDuiControl*>& list = getControls();
  list.push_back(createLabel(
	"Above the larger box is a tape showing your current heading.  North is"));
  list.push_back(createLabel(
	"0, east is 90, etc.  If jumping is allowed or you have the jumping flag,"));
  list.push_back(createLabel(
	"an altitude tape appears to the right of the larger box."));
  list.push_back(createLabel(
	""));
  list.push_back(createLabel(
	"Small colored diamonds or arrows may appear on the heading tape.  An"));
  list.push_back(createLabel(
	"arrow pointing left means that a particular flag is to your left, an"));
  list.push_back(createLabel(
	"arrow pointing right means that the flag is to your right, and a diamond"));
  list.push_back(createLabel(
	"indicates the heading to the flag by its position on the heading tape."));
  list.push_back(createLabel(
	"In capture-the-flag mode a marker always shows where your team flag is."));
  list.push_back(createLabel(
	"A yellow marker shows the way to the antidote flag."));
  list.push_back(createLabel(
	""));
  list.push_back(createLabel(
	"At the top of the display are, from left to right, your callsign and"));
  list.push_back(createLabel(
	"score, your status, and the flag you have.  Your callsign is in the"));
  list.push_back(createLabel(
	"color of your team.  Your status is one of:  ready, dead, sealed, zoned"));
  list.push_back(createLabel(
	"or reloading (showing the time until reloaded).  It can also show the"));
  list.push_back(createLabel(
	"time until a bad flag is dropped (if there's a time limit)."));
  list.push_back(createLabel(
	""));
  list.push_back(createLabel(
	"Other informational messages may occasionally flash on the HUD."));
}

//
// Help9Menu
//

class Help9Menu : public HelpMenu {
  public:
			Help9Menu();
			~Help9Menu() { }

  protected:
    float		getLeftSide(int width, int height);
};

Help9Menu::Help9Menu() : HelpMenu("Credits")
{
  // add controls
  std::vector<HUDuiControl*>& list = getControls();
  list.push_back(createLabel("Tim Riker", "Maintainer:"));
  list.push_back(createLabel("", ""));
  list.push_back(createLabel("Chris Schoeneman", "Original Author:"));
  list.push_back(createLabel("", ""));
  list.push_back(createLabel("David Hoeferlin, Tom Hubina", "Code Contributors:"));
  list.push_back(createLabel("Dan Kartch, Jed Lengyel", ""));
  list.push_back(createLabel("Jeff Myers, Tim Olson", ""));
  list.push_back(createLabel("Brian Smits, Greg Spencer", ""));
  list.push_back(createLabel("Daryll Strauss, Frank Thilo", ""));
  list.push_back(createLabel("Dave Brosius, David Trowbridge", ""));
  list.push_back(createLabel("Sean Morrison, Tupone Alfredo", ""));
  list.push_back(createLabel("Lars Luthman, Nils McCarthy", ""));
  list.push_back(createLabel("", ""));
  list.push_back(createLabel("Tamar Cohen", "Tank Models:"));
  list.push_back(createLabel("", ""));
  list.push_back(createLabel("Kevin Novins, Rick Pasetto", "Special Thanks:"));
  list.push_back(createLabel("Adam Rosen, Erin Shaw", ""));
  list.push_back(createLabel("Ben Trumbore, Don Greenberg", ""));
  list.push_back(createLabel("", ""));
  list.push_back(createLabel("http://BZFlag.org/",
						"BZFlag Home Page:"));
  list.push_back(createLabel("", ""));
  list.push_back(createLabel("Tim Riker", "Copyright (c) 1993 - 2004"));
}

float			Help9Menu::getLeftSide(int width, int height)
{
  return 0.5f * width - height / 20.0f;
}

//
// help menu getter
//

static const int	numHelpMenus = 9;
HelpMenu**		HelpMenu::helpMenus = NULL;

HelpMenu*		HelpMenu::getHelpMenu(HUDDialog* dialog, bool next)
{
  if (!helpMenus) {
    helpMenus = new HelpMenu*[numHelpMenus];
    helpMenus[0] = new Help1Menu;
    helpMenus[1] = new Help2Menu;
    helpMenus[2] = new Help3Menu;
    helpMenus[3] = new Help4Menu;
    helpMenus[4] = new Help5Menu;
    helpMenus[5] = new Help6Menu;
    helpMenus[6] = new Help7Menu;
    helpMenus[7] = new Help8Menu;
    helpMenus[8] = new Help9Menu;
  }
  for (int i = 0; i < numHelpMenus; i++)
    if (dialog == helpMenus[i])
      if (next)
	return helpMenus[(i + 1) % numHelpMenus];
      else
	return helpMenus[(i - 1 + numHelpMenus) % numHelpMenus];
  return next ? helpMenus[0] : helpMenus[numHelpMenus - 1];
}

void			HelpMenu::done()
{
  if (helpMenus) {
    for (int i = 0; i < numHelpMenus; i++)
      delete helpMenus[i];
    delete[] helpMenus;
    helpMenus = NULL;
  }
}

//
// ServerMenu
//

void			ServerItem::writeToFile(std::ostream& out) const
{
  char buffer[MAX_STRING+1]; // MAX_STRING is inherited from ServerListCache.h

  // write out desc.
  memset(buffer,0,sizeof(buffer));
  int copyLength = description.size() < MAX_STRING ? description.size(): MAX_STRING;
  strncpy(&buffer[0],description.c_str(),copyLength);
  out.write(buffer,sizeof(buffer));

  // write out name
  memset(buffer,0,sizeof(buffer));
  copyLength = name.size() < MAX_STRING ? name.size(): MAX_STRING;
  strncpy(&buffer[0],name.c_str(),copyLength);
  out.write(buffer,sizeof(buffer));

  // write out pingpacket
  ping.writeToFile(out);

  // write out current time
  memset(buffer,0,sizeof(buffer));
  nboPackInt(buffer,(int32_t)updateTime);
  out.write(&buffer[0], 4);
}

bool			ServerItem::readFromFile(std::istream& in)
{
  char buffer [MAX_STRING+1];

  //read description
  memset(buffer,0,sizeof(buffer));
  in.read(buffer,sizeof(buffer));
  if ((size_t)in.gcount() < sizeof(buffer)) return false; // failed to read entire string
  description = buffer;

  //read name
  memset(buffer,0,sizeof(buffer));
  in.read(buffer,sizeof(buffer));
  if ((size_t)in.gcount() < sizeof(buffer)) return false; // failed to read entire string

  name = buffer;

  bool pingWorked = ping.readFromFile(in);
  if (!pingWorked) return false; // pingpacket failed to read

  // read in time
  in.read(&buffer[0],4);
  if (in.gcount() < 4) return false;
  int32_t theTime;
  nboUnpackInt(&buffer[0],theTime);
  updateTime = (time_t) theTime;
  cached = true;
  return true;
}

// set the last updated time to now
void			ServerItem::setUpdateTime()
{
  updateTime = getNow();
}

// get current age in minutes
time_t			ServerItem::getAgeMinutes() const
{
  time_t time = (getNow() - updateTime)/(time_t)60;
  return time;
}

// get current age in seconds
time_t			ServerItem::getAgeSeconds() const
{
  time_t time = (getNow() - updateTime);
  return time;
}

// get a simple string which describes the age of item
std::string		ServerItem::getAgeString() const
{
  std::string returnMe;
  char buffer [80];
  time_t age = getAgeMinutes();
  float fAge;
  if (age < 60){ // < 1 hr
    if (age < 1){
      time_t ageSecs = getAgeSeconds();
      sprintf(buffer,"%-3ld secs",(long)ageSecs);
    } else {
      sprintf(buffer,"%-3ld mins",(long)age);
    }
  } else { // >= 60 minutes
    if (age < (24*60)){ // < 24 hours & > 1 hr
      fAge = ((float)age / 60.0f);
      sprintf(buffer, "%-2.1f hrs", fAge);
    } else  { // > 24 hrs
      if (age < (24*60*99)){  // > 1 day & < 99 days
       fAge = ((float) age / (60.0f*24.0f));
       sprintf(buffer, "%-2.1f days", fAge);
      } else { // over 99 days
       fAge = ((float) age / (60.0f*24.0f));
       sprintf(buffer, "%-3f days", fAge);  //should not happen
      }
    }
  }
  returnMe = buffer;
  return returnMe;
}

// get the current time
time_t			ServerItem::getNow() const
{
#if defined(_WIN32)
  return time(NULL);
#else
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return tv.tv_sec;
#endif
}

bool			ServerItem::operator<(const ServerItem &right)
{
  const ServerItem & left = *this;
  if (left.cached && right.cached){
    if (left.getPlayerCount() < right.getPlayerCount()){
      return true;
    }
    else if (left.getPlayerCount() == right.getPlayerCount()){
      if (left.getAgeMinutes() > right.getAgeMinutes()){
	return true;
      }
      else {
	return false;
      }
    }
    else {
      return false;
    }
  }
  else if (!left.cached && !right.cached) {
    if (left.getPlayerCount() < right.getPlayerCount()){
      return true;
    }
    else {
      return false;
    }
  }
  else if (!left.cached && right.cached) {
    return false;
  }
  else {
    // left.cached && !right.cached // always less
    return true;
  }
}

int			ServerItem::getPlayerCount() const
{
  // if null ping we return a 0 player count
  int curPlayer = 0;
  if (&ping != 0) {
    int maxPlayer = ping.maxPlayers;
    curPlayer = ping.rogueCount + ping.redCount + ping.greenCount +
      ping.blueCount + ping.purpleCount + ping.observerCount;
    if (curPlayer > maxPlayer)
      curPlayer = maxPlayer;
  }
  return curPlayer;
}


bool			ServerMenuDefaultKey::keyPress(const BzfKeyEvent& key)
{
  if (key.ascii == 0) switch (key.button) {
    case BzfKeyEvent::Up:
      if (HUDui::getFocus()) {
	menu->setSelected(menu->getSelected() - 1);
      }
      return true;

    case BzfKeyEvent::Down:
      if (HUDui::getFocus()) {
	menu->setSelected(menu->getSelected() + 1);
      }
      return true;

    case BzfKeyEvent::PageUp:
      if (HUDui::getFocus()) {
	menu->setSelected(menu->getSelected() - ServerMenu::NumItems);
      }
      return true;

    case BzfKeyEvent::PageDown:
      if (HUDui::getFocus()) {
	menu->setSelected(menu->getSelected() + ServerMenu::NumItems);
      }
      return true;
  }

  else if (key.ascii == '\t') {
    if (HUDui::getFocus()) {
	menu->setSelected(menu->getSelected() + 1);
    }
    return true;
  }

  return MenuDefaultKey::keyPress(key);
}

bool			ServerMenuDefaultKey::keyRelease(const BzfKeyEvent& key)
{
  switch (key.button) {
    case BzfKeyEvent::Up:
    case BzfKeyEvent::Down:
    case BzfKeyEvent::PageUp:
    case BzfKeyEvent::PageDown:
      return true;
  }
  switch (key.ascii) {
    case 27:	// escape
    case 13:	// return
      return true;
  }
  return false;
}

const int		ServerMenu::NumReadouts = 24;
const int		ServerMenu::NumItems = 10;

ServerMenu::ServerMenu() : defaultKey(this),
				pingBcastSocket(-1),
				selectedIndex(0),
				numListServers(0),
				serverCache(ServerListCache::get())
{
  // add controls
  addLabel("Servers", "");
  addLabel("Players", "");
  addLabel("Rogue", "");
  addLabel("Red", "");
  addLabel("Green", "");
  addLabel("Blue", "");
  addLabel("Purple", "");
  addLabel("Observers", "");
  addLabel("", "");			// max shots
  addLabel("", "");			// capture-the-flag/free-style/rabbit chase
  addLabel("", "");			// super-flags
  addLabel("", "");			// antidote-flag
  addLabel("", "");			// shaking time
  addLabel("", "");			// shaking wins
  addLabel("", "");			// jumping
  addLabel("", "");			// ricochet
  addLabel("", "");			// inertia
  addLabel("", "");			// time limit
  addLabel("", "");			// max team score
  addLabel("", "");			// max player score
  addLabel("", "");			// cached status
  addLabel("", "");			// cached age
  addLabel("", "");			// search status
  addLabel("", "");			// page readout
  status = (HUDuiLabel*)(getControls()[NumReadouts - 2]);
  pageLabel = (HUDuiLabel*)(getControls()[NumReadouts - 1]);

  // add server list items
  for (int i = 0; i < NumItems; ++i)
    addLabel("", "");

  // set initial focus
  setFocus(status);
}



void			ServerMenu::addLabel(
				const char* msg, const char* _label)
{
  HUDuiLabel* label = new HUDuiLabel;
  label->setFont(MainMenu::getFont());
  label->setString(msg);
  label->setLabel(_label);
  getControls().push_back(label);
}

int			ServerMenu::getSelected() const
{
  return selectedIndex;
}

void			ServerMenu::setSelected(int index)
{
  // clamp index
  if (index < 0)
    index = servers.size() - 1;
  else if (index != 0 && index >= (int)servers.size())
    index = 0;

  // ignore if no change
  if (selectedIndex == index)
    return;

  // update selected index and get old and new page numbers
  const int oldPage = (selectedIndex < 0) ? -1 : (selectedIndex / NumItems);
  selectedIndex = index;
  const int newPage = (selectedIndex / NumItems);

  // if page changed then load items for this page
  if (oldPage != newPage) {
    // fill items
    std::vector<HUDuiControl*>& list = getControls();
    const int base = newPage * NumItems;
    for (int i = 0; i < NumItems; ++i) {
      HUDuiLabel* label = (HUDuiLabel*)list[i + NumReadouts];
      if (base + i < (int)servers.size()) {
	label->setString(servers[base + i].description);
	if (servers[base + i].cached ){
	  label->setDarker(true);
	}
	else {
	  label->setDarker(false);
	}
      }
      else {
	label->setString("");
      }
    }

    // change page label
    if ((int)servers.size() > NumItems) {
      char msg[50];
      std::vector<std::string> args;
      sprintf(msg, "%d", newPage + 1);
      args.push_back(msg);
      sprintf(msg, "%ld", (long int)(servers.size() + NumItems - 1) / NumItems);
      args.push_back(msg);
      pageLabel->setString("Page {1} of {2}", &args);
    }
  }

  // set focus to selected item
  if (servers.size() > 0) {
    const int indexOnPage = selectedIndex % NumItems;
    getControls()[NumReadouts + indexOnPage]->setFocus();
  }

  // update readouts
  pick();
}

void			ServerMenu::pick()
{
  if (servers.size() == 0)
    return;

  // get server info
  const ServerItem& item = servers[selectedIndex];
  const PingPacket& ping = item.ping;

  // update server readouts
  char buf[60];
  std::vector<HUDuiControl*>& list = getControls();

  const uint8_t maxes [] = { ping.maxPlayers, ping.rogueMax, ping.redMax, ping.greenMax,
      ping.blueMax, ping.purpleMax, ping.observerMax };

  // if this is a cached item set the player counts to "?/max count"
  if (item.cached && item.getPlayerCount() == 0) {
    for (int i = 1; i <=7; i ++){
      sprintf(buf, "?/%d", maxes[i-1]);
      ((HUDuiLabel*)list[i])->setLabel(buf);
    }
  } else {  // not an old item, set players #s to info we have
    sprintf(buf, "%d/%d", ping.rogueCount + ping.redCount + ping.greenCount +
	ping.blueCount + ping.purpleCount + ping.observerCount, ping.maxPlayers);
    ((HUDuiLabel*)list[1])->setLabel(buf);

    if (ping.rogueMax == 0)
      buf[0]=0;
    else if (ping.rogueMax >= ping.maxPlayers)
      sprintf(buf, "%d", ping.rogueCount);
    else
      sprintf(buf, "%d/%d", ping.rogueCount, ping.rogueMax);
    ((HUDuiLabel*)list[2])->setLabel(buf);

    if (ping.redMax == 0)
      buf[0]=0;
    else if (ping.redMax >= ping.maxPlayers)
      sprintf(buf, "%d", ping.redCount);
    else
      sprintf(buf, "%d/%d", ping.redCount, ping.redMax);
    ((HUDuiLabel*)list[3])->setLabel(buf);

    if (ping.greenMax == 0)
      buf[0]=0;
    else if (ping.greenMax >= ping.maxPlayers)
      sprintf(buf, "%d", ping.greenCount);
    else
      sprintf(buf, "%d/%d", ping.greenCount, ping.greenMax);
    ((HUDuiLabel*)list[4])->setLabel(buf);

    if (ping.blueMax == 0)
      buf[0]=0;
    else if (ping.blueMax >= ping.maxPlayers)
      sprintf(buf, "%d", ping.blueCount);
    else
      sprintf(buf, "%d/%d", ping.blueCount, ping.blueMax);
    ((HUDuiLabel*)list[5])->setLabel(buf);

    if (ping.purpleMax == 0)
      buf[0]=0;
    else if (ping.purpleMax >= ping.maxPlayers)
      sprintf(buf, "%d", ping.purpleCount);
    else
      sprintf(buf, "%d/%d", ping.purpleCount, ping.purpleMax);
    ((HUDuiLabel*)list[6])->setLabel(buf);

    if (ping.observerMax == 0)
      buf[0]=0;
    else if (ping.observerMax >= ping.maxPlayers)
      sprintf(buf, "%d", ping.observerCount);
    else
      sprintf(buf, "%d/%d", ping.observerCount, ping.observerMax);
    ((HUDuiLabel*)list[7])->setLabel(buf);
  }

  std::vector<std::string> args;
  sprintf(buf, "%d", ping.maxShots);
  args.push_back(buf);

  if (ping.maxShots == 1)
    ((HUDuiLabel*)list[8])->setString("{1} Shot", &args );
  else
    ((HUDuiLabel*)list[8])->setString("{1} Shots", &args );

  if (ping.gameStyle & TeamFlagGameStyle)
    ((HUDuiLabel*)list[9])->setString("Capture-the-Flag");
  else if (ping.gameStyle & RabbitChaseGameStyle)
    ((HUDuiLabel*)list[9])->setString("Rabbit Chase");
  else
    ((HUDuiLabel*)list[9])->setString("Free-style");

  if (ping.gameStyle & SuperFlagGameStyle)
    ((HUDuiLabel*)list[10])->setString("Super Flags");
  else
    ((HUDuiLabel*)list[10])->setString("");

  if (ping.gameStyle & AntidoteGameStyle)
    ((HUDuiLabel*)list[11])->setString("Antidote Flags");
  else
    ((HUDuiLabel*)list[11])->setString("");

  if ((ping.gameStyle & ShakableGameStyle) && ping.shakeTimeout != 0) {
    std::vector<std::string> args;
    sprintf(buf, "%.1f", 0.1f * float(ping.shakeTimeout));
    args.push_back(buf);
    if (ping.shakeWins == 1)
      ((HUDuiLabel*)list[12])->setString("{1} sec To Drop Bad Flag", &args);
    else
      ((HUDuiLabel*)list[12])->setString("{1} secs To Drop Bad Flag", &args);
  }
  else
    ((HUDuiLabel*)list[13])->setString("");

  if ((ping.gameStyle & ShakableGameStyle) && ping.shakeWins != 0) {
    std::vector<std::string> args;
    sprintf(buf, "%d", ping.shakeWins);
    args.push_back(buf);
    args.push_back(ping.shakeWins == 1 ? "" : "s");
    if (ping.shakeWins == 1)
      ((HUDuiLabel*)list[12])->setString("{1} Win Drops Bad Flag", &args);
    else
      ((HUDuiLabel*)list[12])->setString("{1} Wins Drops Bad Flag", &args);
  }
  else
    ((HUDuiLabel*)list[13])->setString("");

  if (ping.gameStyle & JumpingGameStyle)
    ((HUDuiLabel*)list[14])->setString("Jumping");
  else
    ((HUDuiLabel*)list[14])->setString("");

  if (ping.gameStyle & RicochetGameStyle)
    ((HUDuiLabel*)list[15])->setString("Ricochet");
  else
    ((HUDuiLabel*)list[15])->setString("");

  if (ping.gameStyle & InertiaGameStyle)
    ((HUDuiLabel*)list[16])->setString("Inertia");
  else
    ((HUDuiLabel*)list[16])->setString("");

  if (ping.maxTime != 0) {
    std::vector<std::string> args;
    if (ping.maxTime >= 3600)
      sprintf(buf, "%d:%02d:%02d", ping.maxTime / 3600, (ping.maxTime / 60) % 60, ping.maxTime % 60);
    else if (ping.maxTime >= 60)
      sprintf(buf, "%d:%02d", ping.maxTime / 60, ping.maxTime % 60);
    else
      sprintf(buf, "0:%02d", ping.maxTime);
    args.push_back(buf);
    ((HUDuiLabel*)list[17])->setString("Time limit: {1}", &args);
  }
  else
    ((HUDuiLabel*)list[17])->setString("");

  if (ping.maxTeamScore != 0) {
    std::vector<std::string> args;
    sprintf(buf, "%d", ping.maxTeamScore);
    args.push_back(buf);
    ((HUDuiLabel*)list[18])->setString("Max team score: {1}", &args);
  }
  else
    ((HUDuiLabel*)list[18])->setString("");


  if (ping.maxPlayerScore != 0) {
    std::vector<std::string> args;
    sprintf(buf, "%d", ping.maxPlayerScore);
    args.push_back(buf);
    ((HUDuiLabel*)list[19])->setString("Max player score: {1}", &args);
  }
  else
    ((HUDuiLabel*)list[19])->setString("");

  if (item.cached){
    ((HUDuiLabel*)list[20])->setString("Cached");
    ((HUDuiLabel*)list[21])->setString(item.getAgeString());
  }
  else {
    ((HUDuiLabel*)list[20])->setString("");
    ((HUDuiLabel*)list[21])->setString("");
  }
}

void			ServerMenu::show()
{
  // clear server list
  servers.clear();
  addedCacheToList = false;

  // clear server readouts
  std::vector<HUDuiControl*>& list = getControls();
  ((HUDuiLabel*)list[1])->setLabel("");
  ((HUDuiLabel*)list[2])->setLabel("");
  ((HUDuiLabel*)list[3])->setLabel("");
  ((HUDuiLabel*)list[4])->setLabel("");
  ((HUDuiLabel*)list[5])->setLabel("");
  ((HUDuiLabel*)list[6])->setLabel("");
  ((HUDuiLabel*)list[7])->setLabel("");
  ((HUDuiLabel*)list[8])->setString("");
  ((HUDuiLabel*)list[9])->setString("");
  ((HUDuiLabel*)list[10])->setString("");
  ((HUDuiLabel*)list[11])->setString("");
  ((HUDuiLabel*)list[12])->setString("");
  ((HUDuiLabel*)list[13])->setString("");
  ((HUDuiLabel*)list[14])->setString("");
  ((HUDuiLabel*)list[15])->setString("");
  ((HUDuiLabel*)list[16])->setString("");
  ((HUDuiLabel*)list[17])->setString("");
  ((HUDuiLabel*)list[18])->setString("");
  ((HUDuiLabel*)list[19])->setString("");
  ((HUDuiLabel*)list[20])->setString("");
  ((HUDuiLabel*)list[21])->setString("");

  char buffer[80];

  // add cache items w/o re-caching them
  int numItemsAdded = 0;
  for (SRV_STR_MAP::const_iterator iter = serverCache->begin();
       iter != serverCache->end(); iter++) {
    // if maxCacheAge is 0 we add nothing
    // if the item is young enough we add it
    if (serverCache->getMaxCacheAge() != 0 && iter->second.getAgeMinutes() < serverCache->getMaxCacheAge()) {
      ServerItem aCopy = iter->second;
      addToList(aCopy);
      numItemsAdded ++;
    }
  }

  std::vector<std::string> args;
  sprintf(buffer, "%d", numItemsAdded);
  args.push_back(buffer);
  setStatus("Servers found: {1}", &args);
  pageLabel->setString("");
  selectedIndex = -1;
  setSelected(0);

  // focus to no-server
  setFocus(status);

  // schedule lookup of server list url.  dereference URL chain every
  // time instead of only first time just in case one of the pointers
  // has changed.
  const StartupInfo* info = getStartupInfo();
  if (info->listServerURL.size() == 0)
    phase = -1;
  else
    phase = 0;

  // also try broadcast
  pingBcastSocket = openBroadcast(BroadcastPort, NULL, &pingBcastAddr);
  if (pingBcastSocket != -1)
    PingPacket::sendRequest(pingBcastSocket, &pingBcastAddr);

  // listen for echos
  addPlayingCallback(&playingCB, this);
}

void			ServerMenu::execute()
{
  if (selectedIndex < 0 || selectedIndex >= (int)servers.size())
    return;

  // update startup info
  StartupInfo* info = getStartupInfo();
  strcpy(info->serverName, servers[selectedIndex].name.c_str());
  info->serverPort = ntohs((unsigned short)
				servers[selectedIndex].ping.serverId.port);

  // all done
  HUDDialogStack::get()->pop();
}

void			ServerMenu::dismiss()
{
  // no more callbacks
  removePlayingCallback(&playingCB, this);

  // close server list sockets
  for (int i = 0; i < numListServers; i++)
    if (listServers[i].socket != -1) {
      close(listServers[i].socket);
      listServers[i].socket = -1;
    }
  numListServers = 0;

  // close input multicast socket
  closeMulticast(pingBcastSocket);
  pingBcastSocket = -1;
}

void			ServerMenu::resize(int width, int height)
{
  // remember size
  HUDDialog::resize(width, height);

  // get number of servers
  std::vector<HUDuiControl*>& list = getControls();

  // use a big font for title, smaller font for the rest
  const float titleFontWidth = (float)height / 10.0f;
  const float titleFontHeight = (float)height / 10.0f;

  // reposition title
  float x, y;
  {
    HUDuiLabel* title = (HUDuiLabel*)list[0];
    title->setFontSize(titleFontWidth, titleFontHeight);
    const OpenGLTexFont& titleFont = title->getFont();
    const float titleWidth = titleFont.getWidth(title->getString());
    x = 0.5f * ((float)width - titleWidth);
    y = (float)height - titleFont.getHeight();
    title->setPosition(x, y);
  }

  // reposition server readouts
  int i;
  const float y0 = y;
  float fontWidth = (float)height / 36.0f;
  float fontHeight = (float)height / 36.0f;
  for (i = 1; i < NumReadouts - 2; i++) {
    if (i % 7 == 1) {
      x = (0.125f + 0.25f * (float)((i - 1) / 7)) * (float)width;
      y = y0;
    }

    HUDuiLabel* label = (HUDuiLabel*)list[i];
    label->setFontSize(fontWidth, fontHeight);
    const OpenGLTexFont& font = label->getFont();
    y -= 1.0f * font.getHeight();
    label->setPosition(x, y);
  }

  y = ((HUDuiLabel*)list[7])->getY(); //reset bottom to last team label

  // reposition search status readout
  {
    fontWidth = (float)height / 24.0f;
    fontHeight = (float)height / 24.0f;
    status->setFontSize(fontWidth, fontHeight);
    const OpenGLTexFont& font = status->getFont();
    const float statusWidth = font.getWidth(status->getString());
    x = 0.5f * ((float)width - statusWidth);
    y -= 0.8f * font.getHeight();
    status->setPosition(x, y);
  }

  // position page readout and server item list
  fontWidth = (float)height / 36.0f;
  fontHeight = (float)height / 36.0f;
  x = 0.125f * (float)width;
  for (i = -1; i < NumItems; ++i) {
    HUDuiLabel* label = (HUDuiLabel*)list[i + NumReadouts];
    label->setFontSize(fontWidth, fontHeight);
    const OpenGLTexFont& font = label->getFont();
    y -= 1.0f * font.getHeight();
    label->setPosition(x, y);
  }
}

void			ServerMenu::setStatus(const char* msg, const std::vector<std::string> *parms)
{
  status->setString(msg, parms);
  const OpenGLTexFont& font = status->getFont();
  const float statusWidth = font.getWidth(status->getString());
  status->setPosition(0.5f * ((float)width - statusWidth), status->getY());
}

void			ServerMenu::checkEchos()
{

  // counter used to print a status spinner
  static int counter=0;
  // how frequent to update spinner
  const float STATUS_UPDATE_FREQUENCY = 0.5; 
  // timer used to track the spinner update frequency
  static TimeKeeper lastUpdate = TimeKeeper::getCurrent();

  // print a spinning status message that updates periodically until we are 
  // actually receiving data from a list server (phase 3).  the loop below
  // is not entered until later -- so update the spinning status here too
  if (phase < 2) {
    if (TimeKeeper::getCurrent() - lastUpdate > STATUS_UPDATE_FREQUENCY) {
      /* a space trailing the spinning status icon adjusts for the
       * variable width font -- would be better to actually print
       * a status icon elsewhere or print spinning icon separate
       * from the text (and as a cool graphic).
       */
      setStatus(string_util::format("%s Searching", (counter%4==0)?"-":(counter%4==1)?" \\":(counter%4==2)?" |":" /").c_str());
      counter++;
      lastUpdate = TimeKeeper::getCurrent();
    }
  }

  // lookup server list in phase 0
  if (phase == 0) {
    int i;

    std::vector<std::string> urls;
    urls.push_back(getStartupInfo()->listServerURL);

    // check urls for validity
    numListServers = 0;
    for (i = 0; i < (int)urls.size(); ++i) {
	// parse url
	std::string protocol, hostname, path;
	int port = 80;
	Address address;
	if (!BzfNetwork::parseURL(urls[i], protocol, hostname, port, path) ||
	    protocol != "http" || port < 1 || port > 65535 ||
	    (address = Address::getHostAddress(hostname.c_str())).isAny()) {
	    std::vector<std::string> args;
	    args.push_back(urls[i]);
	    printError("Can't open list server: {1}", &args);
	    if (!addedCacheToList) {
	      addedCacheToList = true;
	      addCacheToList();
	    }
	    continue;
	}

	// add to list
	listServers[numListServers].address = address;
	listServers[numListServers].hostname = hostname;
	listServers[numListServers].pathname = path;
	listServers[numListServers].port    = port;
	listServers[numListServers].socket  = -1;
	listServers[numListServers].phase   = 2;
	numListServers++;
    }

    // do phase 1 only if we found a valid list server url
    if (numListServers > 0)
      phase = 1;
    else
      phase = -1;
  }

  // connect to list servers in phase 1
  else if (phase == 1) {
    phase = -1;
    for (int i = 0; i < numListServers; i++) {
      ListServer& listServer = listServers[i];

      // create socket.  give up on failure.
      listServer.socket = socket(AF_INET, SOCK_STREAM, 0);
      if (listServer.socket < 0) {
	printError("Can't create list server socket");
	listServer.socket = -1;
	if (!addedCacheToList) {
	  addedCacheToList = true;
	  addCacheToList();
	}
	continue;
      }

      // set to non-blocking.  we don't want to wait for the connection.
      if (BzfNetwork::setNonBlocking(listServer.socket) < 0) {
	printError("Error with list server socket");
	close(listServer.socket);
	listServer.socket = -1;
	if (!addedCacheToList){
	  addedCacheToList = true;
	  addCacheToList();
	}
	continue;
      }

      // start connection
      struct sockaddr_in addr;
      addr.sin_family = AF_INET;
      addr.sin_port = htons(listServer.port);
      addr.sin_addr = listServer.address;
      if (connect(listServer.socket, (CNCTType*)&addr, sizeof(addr)) < 0) {
#if defined(_WIN32)
#undef EINPROGRESS
#define EINPROGRESS EWOULDBLOCK
#endif
	if (getErrno() != EINPROGRESS) {
	  printError("Can't connect list server socket");
	  close(listServer.socket);
	  listServer.socket = -1;
	  if (!addedCacheToList){
	    addedCacheToList = true;
	    addCacheToList();
	  }
	  continue;
	}
      }

      // at least this socket is okay so proceed to phase 2
      phase = 2;
    }
  }

  // get echo messages
  while (1) {
    int i;

    // print a spinning status message that updates periodically until we are 
    // actually receiving data from a list server (phase 3).
    if (phase < 2) {
      if (TimeKeeper::getCurrent() - lastUpdate > STATUS_UPDATE_FREQUENCY) {
	/* a space trailing the spinning status icon adjusts for the
	 * variable width font -- would be better to actually print
	 * a status icon elsewhere or print spinning icon separate
	 * from the text. (and as a cool graphic)
	 */
	setStatus(string_util::format("%s Searching", (counter%4==0)?"-":(counter%4==1)?" \\":(counter%4==2)?" |":" /").c_str());
	counter++;
	lastUpdate = TimeKeeper::getCurrent();
      }
    }

    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 250;

    fd_set read_set, write_set;
    FD_ZERO(&read_set);
    FD_ZERO(&write_set);
    if (pingBcastSocket != -1) FD_SET(pingBcastSocket, &read_set);
    int fdMax = pingBcastSocket;

    // check for list server connection or data
    for (i = 0; i < numListServers; i++) {
      ListServer& listServer = listServers[i];
      if (listServer.socket != -1) {
	if (listServer.phase == 2) {
	  FD_SET(listServer.socket, &write_set);
	} else if (listServer.phase == 3) {
	  FD_SET(listServer.socket, &read_set);
	}
	if (listServer.socket > fdMax) {
	  fdMax = listServer.socket;
	}
      }
    }

    const int nfound = select(fdMax+1, (fd_set*)&read_set,
					(fd_set*)&write_set, 0, &timeout);
    if (nfound <= 0) {
      break;
    }
    
    // check broadcast sockets
    ServerItem serverInfo;
    sockaddr_in addr;
    
    if (pingBcastSocket != -1 && FD_ISSET(pingBcastSocket, &read_set)) {
      if (serverInfo.ping.read(pingBcastSocket, &addr)) {
	serverInfo.ping.serverId.serverHost = addr.sin_addr;
	serverInfo.cached = false;
	addToListWithLookup(serverInfo);
      }
    }

    // check list servers
    for (i = 0; i < numListServers; i++) {
      ListServer& listServer = listServers[i];
      if (listServer.socket != -1) {
	// read more data from server
	if (FD_ISSET(listServer.socket, &read_set)) {
	  readServerList(i);
	}

	// send list request
	else if (FD_ISSET(listServer.socket, &write_set)) {
#if !defined(_WIN32)
	  // ignore SIGPIPE for this send
	  SIG_PF oldPipeHandler = bzSignal(SIGPIPE, SIG_IGN);
#endif
	  bool errorSending;
	  {
	    char url[1024];
#ifdef _WIN32
	    _snprintf(url, sizeof(url),
#else
	    snprintf(url, sizeof(url),
#endif
		     "GET %s?action=LIST&version=%s HTTP/1.1\r\nHost: %s\r\nCache-control: no-cache\r\n\r\n",
		     listServer.pathname.c_str(), getServerVersion(),
		     listServer.hostname.c_str());
	    errorSending = send(listServer.socket, url, strlen(url), 0)
	      != (int) strlen(url);
	  }
	  if (errorSending) {
	    // probably unable to connect to server
	    close(listServer.socket);
	    listServer.socket = -1;
	    if (!addedCacheToList){
	      addedCacheToList = true;
	      addCacheToList();
	     }
	  }
	  else {
	    listServer.phase = 3;
	    listServer.bufferSize = 0;
	  }
#if !defined(_WIN32)
	  bzSignal(SIGPIPE, oldPipeHandler);
#endif
	}
      }
    }
  }
}

void			ServerMenu::readServerList(int index)
{
  ListServer& listServer = listServers[index];

  // read more data into server list buffer
  int n = recv(listServer.socket, listServer.buffer + listServer.bufferSize,
				sizeof(listServer.buffer) -
					listServer.bufferSize - 1, 0);
  if (n > 0) {
    listServer.bufferSize += n;
    listServer.buffer[listServer.bufferSize] = 0;

    char* base = listServer.buffer;
    while (*base) {
      // find next newline
      char* scan = base;
      while (*scan && *scan != '\n') scan++;

      // if no newline then no more complete replies
      if (*scan != '\n') break;
      *scan++ = '\0';

      // parse server info
      char *scan2, *name, *version, *info, *address, *title;
      name = base;
      version = name;
      while (*version && !isspace(*version))  version++;
      while (*version &&  isspace(*version)) *version++ = 0;
      info = version;
      while (*info && !isspace(*info))  info++;
      while (*info &&  isspace(*info)) *info++ = 0;
      address = info;
      while (*address && !isspace(*address))  address++;
      while (*address &&  isspace(*address)) *address++ = 0;
      title = address;
      while (*title && !isspace(*title))  title++;
      while (*title &&  isspace(*title)) *title++ = 0;

      // extract port number from address
      int port = ServerPort;
      scan2 = strchr(name, ':');
      if (scan2) {
	port = atoi(scan2 + 1);
	*scan2 = 0;
      }

      // check info
      if (strcmp(version, getServerVersion()) == 0 &&
	  (int)strlen(info) == PingPacketHexPackedSize &&
	  port >= 1 && port <= 65535) {
	// store info
	ServerItem serverInfo;
	serverInfo.ping.unpackHex(info);
	int dot[4] = {127,0,0,1};
	if (sscanf(address, "%d.%d.%d.%d", dot+0, dot+1, dot+2, dot+3) == 4) {
	  if (dot[0] >= 0 && dot[0] <= 255 &&
	      dot[1] >= 0 && dot[1] <= 255 &&
	      dot[2] >= 0 && dot[2] <= 255 &&
	      dot[3] >= 0 && dot[3] <= 255) {
	    InAddr addr;
	    unsigned char* paddr = (unsigned char*)&addr.s_addr;
	    paddr[0] = (unsigned char)dot[0];
	    paddr[1] = (unsigned char)dot[1];
	    paddr[2] = (unsigned char)dot[2];
	    paddr[3] = (unsigned char)dot[3];
	    serverInfo.ping.serverId.serverHost = addr;
	  }
	}
	serverInfo.ping.serverId.port = htons((int16_t)port);
	serverInfo.name = name;

	// construct description
	serverInfo.description = serverInfo.name;
	if (port != ServerPort) {
	  char portBuf[20];
	  sprintf(portBuf, "%d", port);
	  serverInfo.description += ":";
	  serverInfo.description += portBuf;
	}
	if (strlen(title) > 0) {
	  serverInfo.description += "; ";
	  serverInfo.description += title;
	}

	serverInfo.cached = false;
	// add to list & add it to the server cache
	addToList(serverInfo,true);
      }

      // next reply
      base = scan;
    }

    // remove parsed replies
    listServer.bufferSize -= (base - listServer.buffer);
    memmove(listServer.buffer, base, listServer.bufferSize);
  }
  else if (n == 0) {
    // server hungup
    close(listServer.socket);
    listServer.socket = -1;
    listServer.phase = 4;
  }
  else if (n < 0) {
    close(listServer.socket);
    listServer.socket = -1;
    listServer.phase = -1;
  }
}

void			ServerMenu::addToListWithLookup(ServerItem& info)
{
  info.name = Address::getHostByAddress(info.ping.serverId.serverHost);

  // tack on port number to description if not default
  info.description = info.name;
  const int port = (int)ntohs((unsigned short)info.ping.serverId.port);
  if (port != ServerPort) {
    char portBuf[20];
    sprintf(portBuf, "%d", port);
    info.description += ":";
    info.description += portBuf;
  }

  addToList(info); // do not cache network lan - etc. servers
}


void			ServerMenu::addToList(ServerItem& info, bool doCache)
{
  // update if we already have it
  int i;

  // search and delete entry for this item if it exists
  // (updating info in place may "unsort" the list)
  for (i = 0; i < (int)servers.size(); i++) {
    ServerItem& server = servers[i];
    if (server.ping.serverId.serverHost.s_addr == info.ping.serverId.serverHost.s_addr
	&& server.ping.serverId.port == info.ping.serverId.port) {
      servers.erase(servers.begin() + i); // erase this item
    }
  }

  // find point to insert new player at
  int insertPoint = -1; // point to insert server into

  // insert new item before the first serveritem with is deemed to be less
  // in value than the item to be inserted -- cached items are less than
  // non-cached, items that have more players are more, etc..
  for (i = 0; i < (int)servers.size(); i++) {
    ServerItem& server = servers[i];
    if (server < info){
      insertPoint = i;
      break;
    }
  }

  if (insertPoint == -1){ // no spot to insert it into -- goes on back
    servers.push_back(info);
  }
  else {  // found a spot to insert it into
    servers.insert(servers.begin() + insertPoint,info);
  }

  // update display
  char buffer [80];
  std::vector<std::string> args;
  sprintf(buffer, "%d", (int)servers.size());
  args.push_back(buffer);
  setStatus("Servers found: {1}", &args);

  // force update
  const int oldSelectedIndex = selectedIndex;
  selectedIndex = -1;
  setSelected(oldSelectedIndex);

  if (doCache) {
    // update server cache if asked for
    // on save we delete at most as many items as we added
    // if the added list is normal, we weed most out, if we
    // get few items, we weed few items out
    serverCache->incAddedNum();

    // make string like "sdfsd.dmsd.com:123"
    char buffer [100];
    std::string serverAddress = info.name;
    sprintf(buffer,":%d",  ntohs((unsigned short) info.ping.serverId.port));
    serverAddress += buffer;
    info.cached = true; // values in cache are "cached"
    // update the last updated time to now
    info.setUpdateTime();

    SRV_STR_MAP::iterator iter;
    iter = serverCache->find(serverAddress);  // erase entry to allow update
    if (iter != serverCache->end()){ // if we find it, update it
      iter->second = info;
    }
    else {
      // insert into cache -- wasn't found
      serverCache->insert(serverAddress,info);
    }
  }
}

// add the entire cache to the server list
void			ServerMenu::addCacheToList()
{
  for (SRV_STR_MAP::iterator iter = serverCache->begin();
       iter != serverCache->end(); iter++){
    addToList(iter->second);
  }
}

void			ServerMenu::playingCB(void* _self)
{
  ((ServerMenu*)_self)->checkEchos();
}

//
// ServerStartMenu
//

char			ServerStartMenu::settings[] = "bfaaaaabaaaaa";

ServerStartMenu::ServerStartMenu()
{
  // add controls
  std::vector<HUDuiControl*>& controls = getControls();
  HUDuiList* list;
  std::vector<std::string>* items;

  controls.push_back(createLabel("Start Server"));

  list = createList("Style:");
  items = &list->getList();
  items->push_back("Capture the Flag");
  items->push_back("Free for All");
  items->push_back("Rabbit Hunt");
  list->update();
  controls.push_back(list);

  list = createList("Max Players:");
  items = &list->getList();
  items->push_back("2");
  items->push_back("3");
  items->push_back("4");
  items->push_back("8");
  items->push_back("20");
  items->push_back("40");
  list->update();
  controls.push_back(list);

  list = createList("Max Shots:");
  items = &list->getList();
  items->push_back("1");
  items->push_back("2");
  items->push_back("3");
  items->push_back("4");
  items->push_back("5");
  list->update();
  controls.push_back(list);

  list = createList("Teleporters:");
  items = &list->getList();
  items->push_back("no");
  items->push_back("yes");
  list->update();
  controls.push_back(list);

  list = createList("Ricochet:");
  items = &list->getList();
  items->push_back("no");
  items->push_back("yes");
  list->update();
  controls.push_back(list);

  list = createList("Jumping:");
  items = &list->getList();
  items->push_back("no");
  items->push_back("yes");
  list->update();
  controls.push_back(list);

  list = createList("Superflags:");
  items = &list->getList();
  items->push_back("no");
  items->push_back("good flags only");
  items->push_back("all flags");
  list->update();
  controls.push_back(list);

  list = createList("Max Superflags:");
  items = &list->getList();
  items->push_back("10");
  items->push_back("20");
  items->push_back("30");
  items->push_back("40");
  list->update();
  controls.push_back(list);

  list = createList("Bad Flag Antidote:");
  items = &list->getList();
  items->push_back("no");
  items->push_back("yes");
  list->update();
  controls.push_back(list);

  list = createList("Bad Flag Time Limit:");
  items = &list->getList();
  items->push_back("no limit");
  items->push_back("15 seconds");
  items->push_back("30 seconds");
  items->push_back("60 seconds");
  items->push_back("180 seconds");
  list->update();
  controls.push_back(list);

  list = createList("Bad Flag Win Limit:");
  items = &list->getList();
  items->push_back("no limit");
  items->push_back("drop after 1 win");
  items->push_back("drop after 2 wins");
  items->push_back("drop after 3 wins");
  list->update();
  controls.push_back(list);

  list = createList("Game Over:");
  items = &list->getList();
  items->push_back("never");
  items->push_back("after 5 minutes");
  items->push_back("after 15 minutes");
  items->push_back("after 60 minutes");
  items->push_back("after 3 hours");
  items->push_back("when a player gets +3");
  items->push_back("when a player gets +10");
  items->push_back("when a player gets +25");
  items->push_back("when a team gets +3");
  items->push_back("when a team gets +10");
  items->push_back("when a team gets +25");
  items->push_back("when a team gets +100");
  list->update();
  controls.push_back(list);

  list = createList("Server Reset:");
  items = &list->getList();
  items->push_back("no, quit after game");
  items->push_back("yes, reset for more games");
  list->update();
  controls.push_back(list);

  list = createList("World Map:");
  items = &list->getList();
  items->push_back("random map");

  /* add a list of world files found.  look in the current directory
   * as well as in the config file directory.
   */
#ifdef _WIN32
  /* add a list of .bzw files found in the current dir */
  std::string searchDir = BZDB.get("directory");
  if (searchDir.length() == 0) {
    long availDrives = GetLogicalDrives();
    for (int i = 2; i < 31; i++) {
      if (availDrives & (1 << i)) {
	searchDir = 'a' + i;
	searchDir += ":";
	break;
      }
    }
  }

  searchDir += "\\";
  std::string pattern = searchDir + "*.bzw";
  WIN32_FIND_DATA findData;
  HANDLE h = FindFirstFile(pattern.c_str(), &findData);
  if (h != INVALID_HANDLE_VALUE) {
    std::string file;
    std::string suffix;
    while (FindNextFile(h, &findData)) {
      file = findData.cFileName;
      worldFiles[file] = searchDir + file;
      items->push_back(file);
    }
  }

  /* add a list of .bzw files found in the config file dir */
  searchDir = "C:";
  char dir[MAX_PATH];
  ITEMIDLIST* idl;
  if (SUCCEEDED(SHGetSpecialFolderLocation(NULL, CSIDL_PERSONAL, &idl))) {
    if (SHGetPathFromIDList(idl, dir)) {
      struct stat statbuf;
      if (stat(dir, &statbuf) == 0 && (statbuf.st_mode & _S_IFDIR) != 0)
	searchDir = dir;
    }
    IMalloc* shalloc;
    if (SUCCEEDED(SHGetMalloc(&shalloc))) {
      shalloc->Free(idl);
      shalloc->Release();
    }
  }
  // yes it seems silly but the windows way is to have "my" in front of any folder you make in the my docs dir
  // todo: make this stuff go into the application data dir.
  searchDir += "\\My BZFlag Files\\";
  pattern = searchDir + "*.bzw";
  h = FindFirstFile(pattern.c_str(), &findData);
  if (h != INVALID_HANDLE_VALUE) {
    std::string file;
    std::string suffix;
    while (FindNextFile(h, &findData)) {
      file = findData.cFileName;
      worldFiles[file] = searchDir + file;
      items->push_back(file);
    }
  }
#else
  /* add a list of .bzw files found in the current dir */
  std::string pattern = BZDB.get("directory") + "/";
  DIR *directory = opendir(pattern.c_str());
  if (directory) {
    struct dirent* contents;
    std::string file;
    std::string suffix;
    while ((contents = readdir(directory))) {
      file = contents->d_name;
      if (file.length() > 4) {
	suffix = file.substr(file.length()-4, 4);
	if (compare_nocase(suffix, ".bzw") == 0) {
	  worldFiles[file] = pattern + file;
	  items->push_back(file);
	}
      }
    }
    closedir(directory);
  }

  /* add a list of .bzw files found in the config file dir */
  struct passwd* pwent = getpwuid(getuid());
  pattern = "";
  if (pwent && pwent->pw_dir) {
    pattern += std::string(pwent->pw_dir);
    pattern += "/";
  }
  pattern += ".bzf/";
  directory = opendir(pattern.c_str());
  if (directory) {
    struct dirent* contents;
    std::string file;
    std::string suffix;
    while ((contents = readdir(directory))) {
      file = contents->d_name;
      if (file.length() > 4) {
	suffix = file.substr(file.length()-4, 4);
	if (compare_nocase(suffix, ".bzw") == 0) {
	  worldFiles[file] = pattern + file;
	  items->push_back(file);
	}
      }
    }
    closedir(directory);
  }

#endif

  list->update();
  controls.push_back(list);

  start = createLabel("Start");
  controls.push_back(start);

  status = createLabel("");
  controls.push_back(status);

  failedMessage = createLabel("");
  controls.push_back(failedMessage);

  initNavigation(controls, 1, controls.size()-3);

  // set settings
  loadSettings();
}

void			ServerStartMenu::setSettings(const char* _settings)
{
  // FIXME -- temporary to automatically upgrade old configurations
  if (strlen(_settings) == 14) {
    strcpy(settings, _settings);
    settings[12] = settings[13];
    settings[13] = settings[14];
    return;
  }

  if (strlen(_settings) != strlen(settings)) return;
  strcpy(settings, _settings);
}

void			ServerStartMenu::loadSettings()
{
  std::vector<HUDuiControl*>& controls = getControls();
  const char* scan = settings;
  for (int i = 1; *scan; i++, scan++)
    ((HUDuiList*)controls[i])->setIndex(*scan - 'a');
}

void			ServerStartMenu::show()
{
  setStatus("");
}

void			ServerStartMenu::dismiss()
{
  std::vector<HUDuiControl*>& controls = getControls();
  char* scan = settings;
  for (int i = 1; *scan; i++, scan++)
    *scan = (char)('a' + ((HUDuiList*)controls[i])->getIndex());
}

void			ServerStartMenu::execute()
{
  static const char*	serverApp = "bzfs";

  std::vector<HUDuiControl*>& list = getControls();
  HUDuiControl* focus = HUDui::getFocus();
  if (focus == start) {
    // start it up:
    //   without: -p, -i, -q, -a, +f, -synctime
    //   -b if -c
    //   -h, -r if !-c
    //   +s if superflags != no
    //   -f for all bad flags if superflags == good only
    //
    // other options as they were set

    // get path to server from path to client
    extern const char* argv0;			// from bzflag.cxx
	// add 256 for flags room
    char serverCmd[PATH_MAX + 256];
    strcpy(serverCmd, argv0);
    char* base = strrchr(serverCmd, '/');
#if defined(_WIN32)
    char* base2 = strrchr(serverCmd, '\\');
    if (base2 && (!base || base2 - serverCmd > base - serverCmd))
      base = base2;
#endif
    if (!base) base = serverCmd;
    else base++;
    strcpy(base, serverApp);

    // prepare arguments for starting server
    const char* args[30];
    int arg = 0;
    args[arg++] = serverApp;

    // always try a fallback port if default port is busy
    args[arg++] = "-pf";

    // game style
    if (((HUDuiList*)list[1])->getIndex() == 0) {
      args[arg++] = "-c";
      args[arg++] = "-b";
    }
    else if (((HUDuiList*)list[1])->getIndex() == 1){
      args[arg++] = "-h";
    }
    else {
      args[arg++] = "-rabbit";
    }

    // max players
    static const char* numPlayers[] = { "2", "3", "4", "8", "20", "40" };
    args[arg++] = "-mp";
    args[arg++] = numPlayers[((HUDuiList*)list[2])->getIndex()];

    // max shots
    static const char* numShots[] = { "1", "2", "3", "4", "5" };
    args[arg++] = "-ms";
    args[arg++] = numShots[((HUDuiList*)list[3])->getIndex()];

    // teleporters
    if (((HUDuiList*)list[4])->getIndex() == 1)
      args[arg++] = "-t";

    // ricochet
    if (((HUDuiList*)list[5])->getIndex() == 1)
      args[arg++] = "+r";

    // jumping
    if (((HUDuiList*)list[6])->getIndex() == 1)
      args[arg++] = "-j";

    // superflags
    const int superflagOption = ((HUDuiList*)list[7])->getIndex();
    if (superflagOption != 0) {
      if (superflagOption == 1) {
	args[arg++] = "-f";
	args[arg++] = "bad";
      }

      args[arg++] = "+s";

      // max superflags
      static const char* numFlags[] = { "10", "20", "30", "40" };
      args[arg++] = numFlags[((HUDuiList*)list[8])->getIndex()];
    }

    // shaking
    static const char* shakingTime[] = { "", "15", "30", "60", "180" };
    static const char* shakingWins[] = { "", "1", "2", "3" };
    if (((HUDuiList*)list[9])->getIndex() == 1) args[arg++] = "-sa";
    if (((HUDuiList*)list[10])->getIndex() != 0) {
      args[arg++] = "-st";
      args[arg++] = shakingTime[((HUDuiList*)list[10])->getIndex()];
    }
    if (((HUDuiList*)list[11])->getIndex() != 0) {
      args[arg++] = "-sw";
      args[arg++] = shakingWins[((HUDuiList*)list[11])->getIndex()];
    }

    // game over
    static const char* gameOverArg[] = { "",
				"-time", "-time", "-time", "-time",
				"-mps", "-mps", "-mps",
				"-mts", "-mts", "-mts", "-mts" };
    static const char* gameOverValue[] = { "",
				"300", "900", "3600", "10800",
				"3", "10", "25",
				"3", "10", "25", "100" };
    if (((HUDuiList*)list[12])->getIndex() != 0) {
      args[arg++] = gameOverArg[((HUDuiList*)list[12])->getIndex()];
      args[arg++] = gameOverValue[((HUDuiList*)list[12])->getIndex()];
    }

    // server reset
    if (((HUDuiList*)list[13])->getIndex() == 0)
      args[arg++] = "-g";

    // world map file
    if (((HUDuiList*)list[14])->getIndex() != 0) { // not random
      args[arg++] = "-world";
      std::vector<std::string> fileList = ((HUDuiList*)list[14])->getList();
      std::string filename = fileList[((HUDuiList*)list[14])->getIndex()].c_str();
      args[arg++] = worldFiles[filename].c_str();
    }

    // no more arguments
    args[arg++] = NULL;

    // start the server
#if defined(_WIN32)

    // Windows
#ifdef __MINGW32__
    int result = _spawnvp(_P_DETACH, serverCmd, (char* const*) args);
#else
    int result = _spawnvp(_P_DETACH, serverCmd, args);
#endif
    if (result < 0) {
      if (errno == ENOENT)
	setStatus("Failed... can't find server program.");
      else if (errno == ENOMEM)
	setStatus("Failed... not enough memory.");
      else if (errno == ENOEXEC)
	setStatus("Failed... server program is not executable.");
      else
	setStatus("Failed... unknown error.");
    }
    else {
      setStatus("Server started.");
    }
#elif defined (macintosh)

  MacLaunchServer (arg, args);
#else /* defined(_WIN32) */

    // UNIX
    pid_t pid = fork();
    if (pid == -1) setStatus("Failed... cannot fork.");
    else if (pid == 0) {
      // child process.  close down stdio.
      close(0);
      close(1);
      close(2);

      // exec server
      execvp(serverCmd, (char* const*)args);
      // If execvp returns, bzfs wasnt at the anticipated location.
      // Let execvp try to find it in $PATH by feeding it the "bzfs" name by it self
      execvp(serverApp, (char* const*)args);
      // If that returns too, something bad has happened. Exit.
      exit(2);
    }
    else if (pid != 0) {
      // parent process.  wait a bit and check if child died.
      sleep(1);
      if (waitpid(pid, NULL, WNOHANG) != 0)
	setStatus("Failed.");
      else
	setStatus("Server started.");
    }

#endif /* defined(_WIN32) */
  }
}

void			ServerStartMenu::setStatus(const char* msg, const std::vector<std::string> *parms)
{
  status->setString(msg, parms);
  const OpenGLTexFont& font = status->getFont();
  const float width = font.getWidth(status->getString());
  status->setPosition(center - 0.5f * width, status->getY());
}

void			ServerStartMenu::resize(int width, int height)
{
  HUDDialog::resize(width, height);

  // use a big font for title, smaller font for the rest
  const float titleFontWidth = (float)height / 10.0f;
  const float titleFontHeight = (float)height / 10.0f;
  const float bigFontWidth = (float)height / 24.0f;
  const float bigFontHeight = (float)height / 24.0f;
  const float fontWidth = (float)height / 36.0f;
  const float fontHeight = (float)height / 36.0f;
  center = 0.5f * (float)width;

  // reposition title
  std::vector<HUDuiControl*>& list = getControls();
  HUDuiLabel* title = (HUDuiLabel*)list[0];
  title->setFontSize(titleFontWidth, titleFontHeight);
  const OpenGLTexFont& titleFont = title->getFont();
  const float titleWidth = titleFont.getWidth(title->getString());
  float x = 0.5f * ((float)width - titleWidth);
  float y = (float)height - titleFont.getHeight();
  title->setPosition(x, y);

  // reposition options
  x = 0.5f * (float)width;
  y -= 0.6f * titleFont.getHeight();
  list[1]->setFontSize(fontWidth, fontHeight);
  const float h = list[1]->getFont().getHeight();
  const int count = list.size();
  for (int i = 1; i < count; i++) {
    if (list[i] == start) {
      y -= bigFontHeight;
      list[i]->setFontSize(bigFontWidth, bigFontHeight);
      list[i]->setPosition(x, y);
    }
    else {
      list[i]->setFontSize(fontWidth, fontHeight);
      list[i]->setPosition(x, y);
    }
    y -= 1.0f * h;
  }
}

HUDuiList*		ServerStartMenu::createList(const char* str)
{
  HUDuiList* list = new HUDuiList;
  list->setFont(MainMenu::getFont());
  if (str) list->setLabel(str);
  return list;
}

HUDuiLabel*		ServerStartMenu::createLabel(const char* str)
{
  HUDuiLabel* label = new HUDuiLabel;
  label->setFont(MainMenu::getFont());
  if (str) label->setString(str);
  return label;
}

//
// JoinMenu
//

class JoinMenu : public HUDDialog {
  public:
			JoinMenu();
			~JoinMenu();

    HUDuiDefaultKey*	getDefaultKey()
				{ return MenuDefaultKey::getInstance(); }
    void		show();
    void		execute();
    void		dismiss();
    void		resize(int width, int height);

  private:
    static void		teamCallback(HUDuiControl*, void*);
    static void		joinGameCallback(bool, void*);
    static void		joinErrorCallback(const char* msg);
    TeamColor		getTeam() const;
    void		setTeam(TeamColor);
    void		setStatus(const char*, const std::vector<std::string> *parms = NULL);
    void		loadInfo();

  private:
    float		center;
    HUDuiTypeIn*	callsign;
    HUDuiTypeIn*	email;
    HUDuiList*		team;
    HUDuiTypeIn*	server;
    HUDuiTypeIn*	port;
    HUDuiLabel*		status;
    HUDuiLabel*		startServer;
    HUDuiLabel*		findServer;
    HUDuiLabel*		connectLabel;
    HUDuiLabel*		failedMessage;
    ErrorCallback	oldErrorCallback;
    ServerStartMenu*	serverStartMenu;
    ServerMenu*		serverMenu;
    static JoinMenu*	activeMenu;
};

JoinMenu*		JoinMenu::activeMenu = NULL;

JoinMenu::JoinMenu() : oldErrorCallback(NULL),
				serverStartMenu(NULL), serverMenu(NULL)
{
  // add controls
  std::vector<HUDuiControl*>& list = getControls();
  StartupInfo* info = getStartupInfo();

  HUDuiLabel* label = new HUDuiLabel;
  label->setFont(MainMenu::getFont());
  label->setString("Join Game");
  list.push_back(label);

  findServer = new HUDuiLabel;
  findServer->setFont(MainMenu::getFont());
  findServer->setString("Find Server");
  list.push_back(findServer);

  connectLabel = new HUDuiLabel;
  connectLabel->setFont(MainMenu::getFont());
  connectLabel->setString("Connect");
  list.push_back(connectLabel);

  callsign = new HUDuiTypeIn;
  callsign->setFont(MainMenu::getFont());
  callsign->setLabel("Callsign:");
  callsign->setMaxLength(CallSignLen - 1);
  callsign->setString(info->callsign);
  list.push_back(callsign);

  team = new HUDuiList;
  team->setFont(MainMenu::getFont());
  team->setLabel("Team:");
  team->setCallback(teamCallback, NULL);
  std::vector<std::string>& teams = team->getList();
  // these do not need to be in enum order, but must match getTeam() & setTeam()
  teams.push_back(std::string(Team::getName(AutomaticTeam)));
  teams.push_back(std::string(Team::getName(RogueTeam)));
  teams.push_back(std::string(Team::getName(RedTeam)));
  teams.push_back(std::string(Team::getName(GreenTeam)));
  teams.push_back(std::string(Team::getName(BlueTeam)));
  teams.push_back(std::string(Team::getName(PurpleTeam)));
  teams.push_back(std::string(Team::getName(ObserverTeam)));
  team->update();
  setTeam(info->team);
  list.push_back(team);

  server = new HUDuiTypeIn;
  server->setFont(MainMenu::getFont());
  server->setLabel("Server:");
  server->setMaxLength(64);
  server->setString(info->serverName);
  list.push_back(server);

  char buffer[10];
  sprintf(buffer, "%d", info->serverPort);
  port = new HUDuiTypeIn;
  port->setFont(MainMenu::getFont());
  port->setLabel("Port:");
  port->setMaxLength(5);
  port->setString(buffer);
  list.push_back(port);

  email = new HUDuiTypeIn;
  email->setFont(MainMenu::getFont());
  email->setLabel("Email:");
  email->setMaxLength(EmailLen - 1);
  email->setString(info->email);
  list.push_back(email);

  startServer = new HUDuiLabel;
  startServer->setFont(MainMenu::getFont());
  startServer->setString("Start Server");
  list.push_back(startServer);

  status = new HUDuiLabel;
  status->setFont(MainMenu::getFont());
  status->setString("");
  list.push_back(status);

  failedMessage = new HUDuiLabel;
  failedMessage->setFont(MainMenu::getFont());
  failedMessage->setString("");
  list.push_back(failedMessage);

  initNavigation(list, 1, list.size()-3);
}

JoinMenu::~JoinMenu()
{
  delete serverStartMenu;
  delete serverMenu;
}

void			JoinMenu::show()
{
  activeMenu = this;

  StartupInfo* info = getStartupInfo();

  // set fields
  callsign->setString(info->callsign);
  setTeam(info->team);

  server->setString(info->serverName);
  char buffer[10];
  sprintf(buffer, "%d", info->serverPort);
  port->setString(buffer);

  // clear status
  oldErrorCallback = NULL;
  setStatus("");
}

void			JoinMenu::dismiss()
{
  loadInfo();
  activeMenu = NULL;
}

void			JoinMenu::loadInfo()
{
  // load startup info with current settings
  StartupInfo* info = getStartupInfo();
  strcpy(info->callsign, callsign->getString().c_str());
  info->team = getTeam();
  strcpy(info->serverName, server->getString().c_str());
  info->serverPort = atoi(port->getString().c_str());
  strcpy(info->email, email->getString().c_str());
}

void			JoinMenu::execute()
{
  HUDuiControl* focus = HUDui::getFocus();
  if (focus == startServer) {
    if (!serverStartMenu) serverStartMenu = new ServerStartMenu;
    HUDDialogStack::get()->push(serverStartMenu);
  }

  else if (focus == findServer) {
    if (!serverMenu) serverMenu = new ServerMenu;
    HUDDialogStack::get()->push(serverMenu);
  }

  else if (focus == connectLabel) {
    // load startup info
    loadInfo();

    // make sure we've got what we need
    StartupInfo* info = getStartupInfo();
    if (info->callsign[0] == '\0') {
      setStatus("You must enter a callsign.");
      return;
    }
    if (info->serverName[0] == '\0') {
      setStatus("You must enter a server.");
      return;
    }
    if (info->serverPort <= 0 || info->serverPort > 65535) {
      char buffer[60];
      sprintf(buffer, "Port is invalid.  Try %d.", ServerPort);
      setStatus(buffer);
      return;
    }

    // let user know we're trying
    setStatus("Trying...");

    // schedule attempt to join game
    oldErrorCallback = setErrorCallback(joinErrorCallback);
    joinGame(&joinGameCallback, this);
  }
}

void			JoinMenu::joinGameCallback(bool okay, void* _self)
{
  JoinMenu* self = (JoinMenu*)_self;
  if (okay) {
    // it worked!  pop all the menus.
    HUDDialogStack* stack = HUDDialogStack::get();
    while (stack->isActive()) stack->pop();
  }

  else {
    // failed.  let user know.
    self->setStatus("Connection failed.");
  }
  setErrorCallback(self->oldErrorCallback);
  self->oldErrorCallback = NULL;
}

void			JoinMenu::joinErrorCallback(const char* msg)
{
  JoinMenu* self = activeMenu;
  self->failedMessage->setString(msg);
  const OpenGLTexFont& font = self->failedMessage->getFont();
  const float width = font.getWidth(self->failedMessage->getString());
  self->failedMessage->setPosition(self->center - 0.5f * width,
				self->failedMessage->getY());

  // also forward to old callback
  if (self->oldErrorCallback) (*self->oldErrorCallback)(msg);
}

TeamColor		JoinMenu::getTeam() const
{
  return team->getIndex() == 0 ? AutomaticTeam : TeamColor(team->getIndex() - 1);
}

void			JoinMenu::setTeam(TeamColor teamcol)
{
  team->setIndex(teamcol == AutomaticTeam ? 0 : int(teamcol) + 1);
}

void			JoinMenu::setStatus(const char* msg, const std::vector<std::string> *)
{
  status->setString(msg);
  const OpenGLTexFont& font = status->getFont();
  const float width = font.getWidth(status->getString());
  status->setPosition(center - 0.5f * width, status->getY());
  if (!oldErrorCallback) joinErrorCallback("");
}

void			JoinMenu::teamCallback(HUDuiControl*, void*)
{
  // do nothing (for now)
}

void			JoinMenu::resize(int width, int height)
{
  HUDDialog::resize(width, height);

  // use a big font for title, smaller font for the rest
  const float titleFontWidth = (float)height / 10.0f;
  const float titleFontHeight = (float)height / 10.0f;
  const float fontWidth = (float)height / 24.0f;
  const float fontHeight = (float)height / 24.0f;
  center = 0.5f * (float)width;

  // reposition title
  std::vector<HUDuiControl*>& list = getControls();
  HUDuiLabel* title = (HUDuiLabel*)list[0];
  title->setFontSize(titleFontWidth, titleFontHeight);
  const OpenGLTexFont& titleFont = title->getFont();
  const float titleWidth = titleFont.getWidth(title->getString());
  float x = 0.5f * ((float)width - titleWidth);
  float y = (float)height - titleFont.getHeight();
  title->setPosition(x, y);

  // reposition options
  x = 0.5f * ((float)width - 0.5f * titleWidth);
  y -= 0.6f * titleFont.getHeight();
  list[1]->setFontSize(fontWidth, fontHeight);
  const float h = list[1]->getFont().getHeight();
  const int count = list.size();
  for (int i = 1; i < count; i++) {
    list[i]->setFontSize(fontWidth, fontHeight);
    list[i]->setPosition(x, y);
    y -= 1.0f * h;
    if (i <= 2 || i == 7) y -= 0.5f * h;
  }
}

//
// MainMenu
//

OpenGLTexFont*		MainMenu::mainFont = NULL;

MainMenu::MainMenu() : HUDDialog(), joinMenu(NULL),
				optionsMenu(NULL), quitMenu(NULL)
{
  TextureManager &tm = TextureManager::instance();

  // create font
  font = TextureFont::getTextureFont(TextureFont::HelveticaBold, true);
  mainFont = &font;

  // load title

  OpenGLTexture *title = tm.getTexture( "title" );

  // add controls
  std::vector<HUDuiControl*>& list = getControls();
  HUDuiLabel* label;
  HUDuiTextureLabel* textureLabel;

  textureLabel = new HUDuiTextureLabel;
  textureLabel->setFont(font);
  textureLabel->setTexture(*title);
  textureLabel->setString("BZFlag");
  list.push_back(textureLabel);

  label = new HUDuiLabel;
  label->setFont(font);
  label->setString("Up/Down arrows to move, Enter to select, Esc to dismiss");
  list.push_back(label);

  label = new HUDuiLabel;
  label->setFont(font);
  label->setString("Join Game");
  list.push_back(label);

  label = new HUDuiLabel;
  label->setFont(font);
  label->setString("Options");
  list.push_back(label);

  label = new HUDuiLabel;
  label->setFont(font);
  label->setString("Help");
  list.push_back(label);

  label = new HUDuiLabel;
  label->setFont(font);
  label->setString("Quit");
  list.push_back(label);

  initNavigation(list, 2, list.size()-1);
}

MainMenu::~MainMenu()
{
  mainFont = NULL;
  delete joinMenu;
  delete optionsMenu;
  delete quitMenu;
  HelpMenu::done();
}

const OpenGLTexFont&	MainMenu::getFont()
{
  return *mainFont;
}

HUDuiDefaultKey*	MainMenu::getDefaultKey()
{
  return MenuDefaultKey::getInstance();
}

void			MainMenu::execute()
{
  std::vector<HUDuiControl*>& list = getControls();
  HUDuiControl* focus = HUDui::getFocus();
  if (focus == list[2]) {
    if (!joinMenu) joinMenu = new JoinMenu;
    HUDDialogStack::get()->push(joinMenu);
  }
  else if (focus == list[3]) {
    if (!optionsMenu) optionsMenu = new OptionsMenu;
    HUDDialogStack::get()->push(optionsMenu);
  }
  else if (focus == list[4]) {
    HUDDialogStack::get()->push(HelpMenu::getHelpMenu());
  }
  else if (focus == list[5]) {
    if (!quitMenu) quitMenu = new QuitMenu;
    HUDDialogStack::get()->push(quitMenu);
  }
}

void			MainMenu::resize(int width, int height)
{
  HUDDialog::resize(width, height);

  // use a big font
  const float titleFontWidth = (float)height / 10.0f;
  const float titleFontHeight = (float)height / 10.0f;
  const float fontWidth = (float)height / 12.0f;
  const float fontHeight = (float)height / 12.0f;
  const float tinyFontWidth = (float)height / 36.0f;
  const float tinyFontHeight = (float)height / 36.0f;

  // reposition title
  std::vector<HUDuiControl*>& list = getControls();
  HUDuiLabel* title = (HUDuiLabel*)list[0];
  title->setFontSize(titleFontWidth, titleFontHeight);
  const OpenGLTexFont& titleFont = title->getFont();
  const float titleWidth = titleFont.getWidth(title->getString());
  float x = 0.5f * ((float)width - titleWidth);
  float y = (float)height - titleFont.getHeight();
  title->setPosition(x, y);

  // reposition instructions
  HUDuiLabel* help = (HUDuiLabel*)list[1];
  help->setFontSize(tinyFontWidth, tinyFontHeight);
  const OpenGLTexFont& helpFont = help->getFont();
  const float helpWidth = helpFont.getWidth(help->getString());
  y -= 1.25f * tinyFontHeight;
  help->setPosition(0.5f * ((float)width - helpWidth), y);
  y -= 1.5f * fontHeight;

  // reposition menu items
  x += 0.5f * fontHeight;
  const int count = list.size();
  for (int i = 2; i < count; i++) {
    HUDuiLabel* label = (HUDuiLabel*)list[i];
    label->setFontSize(fontWidth, fontHeight);
    label->setPosition(x, y);
    y -= 1.2f * fontHeight;
  }
}

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

