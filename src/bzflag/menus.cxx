/* bzflag
 * Copyright (c) 1993 - 2003 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifdef macintosh
#include "mac_funcs.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include "bzsignal.h"
#if defined(_WIN32)
#include <process.h>
#else
#include <sys/types.h>
#ifndef GUSI_20
  #include <sys/wait.h>
#endif
#include <unistd.h>
#endif
#include <math.h>
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
#include "ErrorHandler.h"
#include "KeyMap.h"
#include "TimeKeeper.h"
#include "World.h"
#include "Bundle.h"

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
      HUDDialogStack::get()->pop();
      return true;

    case 13:	// return
      HUDDialogStack::get()->top()->execute();
      return true;
  }

  if (getBzfKeyMap().isMappedTo(BzfKeyMap::Quit, key)) {
    getMainWindow()->setQuit();
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
    getMainWindow()->setQuit();
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
    void		execute() { getMainWindow()->setQuit(); }
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
			~KeyboardMapMenu() { }

    HUDuiDefaultKey*	getDefaultKey() { return &defaultKey; }
    void		execute();
    void		dismiss();
    void		resize(int width, int height);

    bool		isEditing() const;
    void		setKey(const BzfKeyEvent&);

  private:
    void		update();

    HUDuiLabel*		createLabel(const char*, const char* = NULL);

  private:
    KeyboardMapMenuDefaultKey defaultKey;
    HUDuiControl*	reset;
    BzfKeyMap::Key		editing;
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

KeyboardMapMenu::KeyboardMapMenu() : defaultKey(this), editing(BzfKeyMap::LastKey)
{
  // add controls
  std::vector<HUDuiControl*>& controls = getControls();

  controls.push_back(createLabel("Key Mapping"));

  controls.push_back(reset = createLabel(NULL, "Reset Defaults"));
  controls.push_back(createLabel(NULL, "Fire shot:"));
  controls.push_back(createLabel(NULL, "Drop flag:"));
  controls.push_back(createLabel(NULL, "Identify/Lock On:"));
  controls.push_back(createLabel(NULL, "Radar Short:"));
  controls.push_back(createLabel(NULL, "Radar Medium:"));
  controls.push_back(createLabel(NULL, "Radar Long:"));
  controls.push_back(createLabel(NULL, "Send to All:"));
  controls.push_back(createLabel(NULL, "Send to Teammates:"));
  controls.push_back(createLabel(NULL, "Send to Nemesis:"));
  controls.push_back(createLabel(NULL, "Send to Recipient:"));
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

  initNavigation(controls, 1, controls.size()-1);
}

bool			KeyboardMapMenu::isEditing() const
{
  return editing != BzfKeyMap::LastKey;
}

void			KeyboardMapMenu::setKey(const BzfKeyEvent& event)
{
  if (editing != BzfKeyMap::LastKey) {
    // check for previous mapping
    BzfKeyMap& map = getBzfKeyMap();
    const BzfKeyMap::Key previous = map.isMapped(event);

    // ignore setting to same value
    if (previous != editing) {
      // if there was a previous setting remove it
      if (previous != BzfKeyMap::LastKey)
	map.unset(previous, event);

      // map new key
      map.set(editing, event);
    }

    // not editing anymore
    editing = BzfKeyMap::LastKey;

    // make sure we update strings
    update();
  }
}

void			KeyboardMapMenu::execute()
{
  const HUDuiControl* const focus = HUDui::getFocus();
  if (focus == reset) {
    getBzfKeyMap().resetAll();
    update();
  }
  else {
    // start editing
    std::vector<HUDuiControl*>& list = getControls();
    editing = BzfKeyMap::LastKey;
    for (int i = 0; i < (int)BzfKeyMap::LastKey; i++)
      if (list[i + 2] == focus) {
	editing = (BzfKeyMap::Key)i;
	break;
      }

    BzfKeyMap& map = getBzfKeyMap();
    if (editing != BzfKeyMap::LastKey) {
      BzfKeyEvent event;
      event.ascii = 0;
      event.button = 0;
      map.set(editing, event);
      update();
    }
  }
}

void			KeyboardMapMenu::dismiss()
{
  editing = BzfKeyMap::LastKey;
  notifyBzfKeyMapChanged();
}

void			KeyboardMapMenu::resize(int width, int height)
{
  int i;
  // use a big font for title, smaller font for the rest
  const float titleFontWidth = (float)height / 10.0f;
  const float titleFontHeight = (float)height / 10.0f;
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

  // reposition options in two columns
  x = 0.30f * (float)width;
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
  for (i = mid+1; i < count; i++) {
    list[i]->setFontSize(fontWidth, fontHeight);
    list[i]->setPosition(x, y);
    y -= 1.0f * h;
  }

  update();
}

void			KeyboardMapMenu::update()
{
  // load current settings
  BzfKeyMap& map = getBzfKeyMap();
  std::vector<HUDuiControl*>& list = getControls();
  for (int j = 0; j < (int)BzfKeyMap::LastKey; j++) {
    const BzfKeyEvent& key1 = map.get((BzfKeyMap::Key)j);
    const BzfKeyEvent& key2 = map.getAlternate((BzfKeyMap::Key)j);
    std::string value;
    if (key1.ascii == 0 && key1.button == 0) {
      if (j == (int)editing)
	value = "???";
      else
	value = "<not mapped>";
    }
    else {
      value += BzfKeyMap::getKeyEventString(key1);
      if (key2.ascii != 0 || key2.button != 0) {
	value += " or ";
	value += BzfKeyMap::getKeyEventString(key2);
      }
      else if (j == (int)editing) {
	value += " or ???";
      }
    }
    ((HUDuiLabel*)list[j + 2])->setString(value);
  }
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
  options = &option->getList();
  options->push_back(std::string("[O----------]"));
  options->push_back(std::string("[-O---------]"));
  options->push_back(std::string("[--O--------]"));
  options->push_back(std::string("[---O-------]"));
  options->push_back(std::string("[----O------]"));
  options->push_back(std::string("[-----O-----]"));
  options->push_back(std::string("[------O----]"));
  options->push_back(std::string("[-------O---]"));
  options->push_back(std::string("[--------O--]"));
  options->push_back(std::string("[---------O-]"));
  options->push_back(std::string("[----------O]"));
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
  options = &option->getList();
  options->push_back(std::string("[O---]"));
  options->push_back(std::string("[-O--]"));
  options->push_back(std::string("[--O-]"));
  options->push_back(std::string("[---O]"));
  option->update();
  list.push_back(option);

  // set radar size
  option = new HUDuiList;
  option->setFont(MainMenu::getFont());
  option->setLabel("Radar & Panel Size:");
  option->setCallback(callback, (void*)"R");
  options = &option->getList();
  options->push_back(std::string("[O----------]"));
  options->push_back(std::string("[-O---------]"));
  options->push_back(std::string("[--O--------]"));
  options->push_back(std::string("[---O-------]"));
  options->push_back(std::string("[----O------]"));
  options->push_back(std::string("[-----O-----]"));
  options->push_back(std::string("[------O----]"));
  options->push_back(std::string("[-------O---]"));
  options->push_back(std::string("[--------O--]"));
  options->push_back(std::string("[---------O-]"));
  options->push_back(std::string("[----------O]"));
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
    ((HUDuiList*)list[i++])->setIndex(renderer->useEnhancedRadar() ? 1 : 0);
    ((HUDuiList*)list[i++])->setIndex(renderer->useBigFont() ? 1 : 0);
    ((HUDuiList*)list[i++])->setIndex((int)(10.0f * renderer->getPanelOpacity()));
    ((HUDuiList*)list[i++])->setIndex(renderer->useColoredShots() ? 1 : 0);
    ((HUDuiList*)list[i++])->setIndex(renderer->getRadarShotLength());
    ((HUDuiList*)list[i++])->setIndex(renderer->getRadarSize());
  }
}

void			GUIOptionsMenu::callback(HUDuiControl* w, void* data)
{
  HUDuiList* list = (HUDuiList*)w;

  SceneRenderer* sceneRenderer = getSceneRenderer();
  switch (((const char*)data)[0]) {
    case 'e':
      sceneRenderer->setEnhancedRadar(list->getIndex() != 0);
      break;

    case 'w':
      sceneRenderer->setBigFont(list->getIndex() != 0);
      break;

    case 'y':
    {
      sceneRenderer->setPanelOpacity(((float)list->getIndex()) / 10.0f);
      break;
    }

    case 'z':
      sceneRenderer->setColoredShots(list->getIndex() != 0);
      break;

    case 'l':
      sceneRenderer->setRadarShotLength(list->getIndex());
      break;

    case 'R':
    {
      sceneRenderer->setRadarSize(list->getIndex());
      break;
    }
  }
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
    FormatMenu*		formatMenu;
    KeyboardMapMenu*	keyboardMapMenu;
    GUIOptionsMenu*	guiOptionsMenu;
};

OptionsMenu::OptionsMenu() : formatMenu(NULL), keyboardMapMenu(NULL),
                             guiOptionsMenu(NULL)
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
  options = &option->getList();
  if (window->hasGammaControl()) {
    options->push_back(std::string("[O--------------]"));
    options->push_back(std::string("[-O-------------]"));
    options->push_back(std::string("[--O------------]"));
    options->push_back(std::string("[---O-----------]"));
    options->push_back(std::string("[----O----------]"));
    options->push_back(std::string("[-----O---------]"));
    options->push_back(std::string("[------O--------]"));
    options->push_back(std::string("[-------O-------]"));
    options->push_back(std::string("[--------O------]"));
    options->push_back(std::string("[---------O-----]"));
    options->push_back(std::string("[----------O----]"));
    options->push_back(std::string("[-----------O---]"));
    options->push_back(std::string("[------------O--]"));
    options->push_back(std::string("[-------------O-]"));
    options->push_back(std::string("[--------------O]"));
  }
  else {
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

  keyMapping = label = new HUDuiLabel;
  label->setFont(MainMenu::getFont());
  label->setLabel("Change Key Mapping");
  list.push_back(label);

  guiOptions = label = new HUDuiLabel;
  label->setFont(MainMenu::getFont());
  label->setLabel("GUI Options");
  list.push_back(label);

  initNavigation(list, 1,list.size()-1);
}

OptionsMenu::~OptionsMenu()
{
  delete formatMenu;
  delete keyboardMapMenu;
  delete guiOptionsMenu;
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
}

void			OptionsMenu::resize(int width, int height)
{
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
    HUDuiList* tex;
    int i = 1;
    ((HUDuiList*)list[i++])->setIndex(renderer->useDithering() ? 1 : 0);
    ((HUDuiList*)list[i++])->setIndex(renderer->useBlending() ? 1 : 0);
    ((HUDuiList*)list[i++])->setIndex(renderer->useSmoothing() ? 1 : 0);
    ((HUDuiList*)list[i++])->setIndex(renderer->useLighting() ? 1 : 0);
    tex = (HUDuiList*)list[i++];
    ((HUDuiList*)list[i++])->setIndex(renderer->useQuality());
    ((HUDuiList*)list[i++])->setIndex(renderer->useShadows() ? 1 : 0);
    ((HUDuiList*)list[i++])->setIndex(renderer->useZBuffer() ? 1 : 0);
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
    ((HUDuiList*)list[+i])->setIndex(info->useUDPconnection ? 1 : 0);

    if (!renderer->useTexture())
      tex->setIndex(0);
    else
      tex->setIndex(OpenGLTexture::getFilter());
  }
}

void			OptionsMenu::callback(HUDuiControl* w, void* data)
{
  HUDuiList* list = (HUDuiList*)w;

  SceneRenderer* sceneRenderer = getSceneRenderer();
  switch (((const char*)data)[0]) {
    case '1':
      sceneRenderer->setDithering(list->getIndex() != 0);
      break;

    case '2':
      sceneRenderer->setBlending(list->getIndex() != 0);
      break;

    case '3':
      sceneRenderer->setSmoothing(list->getIndex() != 0);
      break;

    case '4':
      sceneRenderer->setLighting(list->getIndex() != 0);

      sceneRenderer->setTextureReplace(!sceneRenderer->useLighting() &&
					sceneRenderer->useQuality() < 2);
      break;

    case '5':
      sceneRenderer->setTexture(list->getIndex() != 0);
      OpenGLTexture::setFilter((OpenGLTexture::Filter)list->getIndex());
      break;

    case '6':
      sceneRenderer->setQuality(list->getIndex());

      sceneRenderer->setTextureReplace(!sceneRenderer->useLighting() &&
					sceneRenderer->useQuality() < 2);
      break;

    case '7':
      sceneRenderer->setShadows(list->getIndex() != 0);
      break;

    case '8':
      sceneRenderer->setZBuffer(list->getIndex() != 0);
      if (sceneRenderer->useZBuffer() != (list->getIndex() != 0)) {
	list->setIndex(sceneRenderer->useZBuffer() ? 1 : 0);
      }
      else {
	setSceneDatabase();
      }
      break;

    case 's':
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
    HelpMenuDefaultKey	defaultKey;
    static HelpMenu**	helpMenus;
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
  control->setString(string);
  if (label) control->setLabel(label);
  return control;
}

float			HelpMenu::getLeftSide(int, int height)
{
  return (float)height / 6.0f;
}

void			HelpMenu::resize(int width, int height)
{
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

  protected:
    float		getLeftSide(int width, int height);
};

Help1Menu::Help1Menu() : HelpMenu("Controls")
{
  // add controls
  std::vector<HUDuiControl*>& list = getControls();
  list.push_back(createLabel("controls tank motion", "Mouse Position:"));
  list.push_back(createLabel("fires shot"));
  list.push_back(createLabel("drops flag (if not bad)"));
  list.push_back(createLabel("identifies player (locks on GM)"));
  list.push_back(createLabel("jump (if allowed)"));
  list.push_back(createLabel("short radar range"));
  list.push_back(createLabel("medium radar range"));
  list.push_back(createLabel("long radar range"));
  list.push_back(createLabel("toggle binoculars"));
  list.push_back(createLabel("toggle heads-up flag help"));
  list.push_back(createLabel("send message to teammates"));
  list.push_back(createLabel("send message to everybody"));
  list.push_back(createLabel("send message to nemesis"));
  list.push_back(createLabel("send message to recipient"));
  list.push_back(createLabel("toggle score sheet"));
  list.push_back(createLabel("toggle tank labels"));
  list.push_back(createLabel("set time of day backward"));
  list.push_back(createLabel("set time of day forward"));
  list.push_back(createLabel("pause/resume"));
  list.push_back(createLabel("self destruct/cancel"));
  list.push_back(createLabel("quit"));
  list.push_back(createLabel("scroll message log backward"));
  list.push_back(createLabel("scroll message log forward"));
  list.push_back(createLabel("show/dismiss menu", "Esc:"));
}

float			Help1Menu::getLeftSide(int width, int height)
{
  return 0.5f * width - height / 20.0f;
}

void			Help1Menu::resize(int width, int height)
{
  static const BzfKeyMap::Key key[] = {
				BzfKeyMap::LastKey,
				BzfKeyMap::LastKey,
				BzfKeyMap::LastKey,
				BzfKeyMap::FireShot,
				BzfKeyMap::DropFlag,
				BzfKeyMap::Identify,
				BzfKeyMap::Jump,
				BzfKeyMap::ShortRange,
				BzfKeyMap::MediumRange,
				BzfKeyMap::LongRange,
				BzfKeyMap::Binoculars,
				BzfKeyMap::FlagHelp,
				BzfKeyMap::SendTeam,
				BzfKeyMap::SendAll,
				BzfKeyMap::SendNemesis,
				BzfKeyMap::SendRecipient,
                                BzfKeyMap::Score,
				BzfKeyMap::Labels,
				BzfKeyMap::TimeForward,
				BzfKeyMap::TimeBackward,
				BzfKeyMap::Pause,
				BzfKeyMap::Destruct,
				BzfKeyMap::Quit,
				BzfKeyMap::ScrollBackward,
				BzfKeyMap::ScrollForward,
				BzfKeyMap::SlowKeyboardMotion
			};

  // get current key mapping and set strings appropriately
  BzfKeyMap& map = getBzfKeyMap();
  std::vector<HUDuiControl*>& list = getControls();
  for (int j = 0; j < (int)(sizeof(key) / sizeof(key[0])); j++) {
    if (key[j] == BzfKeyMap::LastKey) continue;

    std::string value;
    const BzfKeyEvent& key1 = map.get(key[j]);
    if (key1.ascii == 0 && key1.button == 0) {
      value = "<not mapped>";
    }
    else {
      value = BzfKeyMap::getKeyEventString(key1);
      const BzfKeyEvent& key2 = map.getAlternate(key[j]);
      if (key2.ascii != 0 || key2.button != 0) {
	value += " or ";
	value += BzfKeyMap::getKeyEventString(key2);
      }
    }
    value += ":";
    list[j]->setLabel(value);
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
	"There are two styles of play, determined by the server configuration:  capture-"));
  list.push_back(createLabel(
	"the-flag and free-for-all.  In free-for-all the object is simply to get the"));
  list.push_back(createLabel(
	"highest score by shooting opponents.  The object in capture-the-flag is to"));
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
  list.push_back(createLabel("Tim Riker", "Copyright (c) 1993 - 2003"));
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

class ServerMenu;

class ServerMenuDefaultKey : public MenuDefaultKey {
  public:
			ServerMenuDefaultKey(ServerMenu* _menu) :
				menu(_menu) { }
			~ServerMenuDefaultKey() { }

    bool		keyPress(const BzfKeyEvent&);
    bool		keyRelease(const BzfKeyEvent&);

  private:
    ServerMenu*		menu;
};

class ServerItem {
  public:
    std::string		name;
    std::string		description;
    PingPacket		ping;
};

static const int	MaxListServers = 5;
class ListServer {
  public:
    Address		address;
    int			port;
    int			socket;
    int			phase;
    int			bufferSize;
    char		buffer[1024];
};

class ServerMenu : public HUDDialog {
  public:
			ServerMenu();
			~ServerMenu() { }

    HUDuiDefaultKey*	getDefaultKey() { return &defaultKey; }
    int			getSelected() const;
    void		setSelected(int);
    void		show();
    void		execute();
    void		dismiss();
    void		resize(int width, int height);

  public:
    static const int	NumItems;

  private:
    void		addLabel(const char* string, const char* label);
    void		checkEchos();
    void		readServerList(int index);
    void		addToList(ServerItem&);
    void		addToListWithLookup(ServerItem&);
    void		setStatus(const char*, const std::vector<std::string> *parms = NULL);
    void		pick();
    int			getPlayerCount(int index) const;
    static void		playingCB(void*);

  private:
    ServerMenuDefaultKey defaultKey;
    std::vector<ServerItem>		servers;
    int			pingInSocket;
    struct sockaddr_in	pingInAddr;
    int			pingBcastSocket;
    struct sockaddr_in	pingBcastAddr;
    int			width, height;
    HUDuiLabel*		status;

    HUDuiLabel*		pageLabel;
    int			selectedIndex;

    int			phase;
    ListServer		listServers[MaxListServers];
    int			numListServers;

    static const int	NumReadouts;
};

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

const int		ServerMenu::NumReadouts = 21;
const int		ServerMenu::NumItems = 10;

ServerMenu::ServerMenu() : defaultKey(this),
				pingInSocket(-1),
				pingBcastSocket(-1),
				numListServers(0)
{
  // add controls
  addLabel("Servers", "");
  addLabel("Players", "");
  addLabel("Red", "");
  addLabel("Green", "");
  addLabel("Blue", "");
  addLabel("Purple", "");
  addLabel("Rogue", "");
  addLabel("", "");			// max shots
  addLabel("", "");			// capture-the-flag/free-style
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
  else if (index != 0 && index >= servers.size())
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
      if (base + i < servers.size())
	label->setString(servers[base + i].description);
      else
	label->setString("");
    }

    // change page label
    if (servers.size() > NumItems) {
      char msg[50];
      std::vector<std::string> args;
      sprintf(msg, "%d", newPage + 1);
      args.push_back(msg);
      sprintf(msg, "%d", (servers.size() + NumItems - 1) / NumItems);
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
  sprintf(buf, "%d/%d", ping.rogueCount + ping.redCount +
			ping.greenCount + ping.blueCount +
			ping.purpleCount, ping.maxPlayers);
  ((HUDuiLabel*)list[1])->setLabel(buf);

  if (ping.redMax >= ping.maxPlayers)
    sprintf(buf, "%d", ping.redCount);
  else
    sprintf(buf, "%d/%d", ping.redCount, ping.redMax);
  ((HUDuiLabel*)list[2])->setLabel(buf);

  if (ping.greenMax >= ping.maxPlayers)
    sprintf(buf, "%d", ping.greenCount);
  else
    sprintf(buf, "%d/%d", ping.greenCount, ping.greenMax);
  ((HUDuiLabel*)list[3])->setLabel(buf);

  if (ping.blueMax >= ping.maxPlayers)
    sprintf(buf, "%d", ping.blueCount);
  else
    sprintf(buf, "%d/%d", ping.blueCount, ping.blueMax);
  ((HUDuiLabel*)list[4])->setLabel(buf);

  if (ping.purpleMax >= ping.maxPlayers)
    sprintf(buf, "%d", ping.purpleCount);
  else
    sprintf(buf, "%d/%d", ping.purpleCount, ping.purpleMax);
  ((HUDuiLabel*)list[5])->setLabel(buf);

  if (ping.gameStyle & RoguesGameStyle) {
    if (ping.rogueMax >= ping.maxPlayers)
      sprintf(buf, "%d", ping.rogueCount);
    else
      sprintf(buf, "%d/%d", ping.rogueCount, ping.rogueMax);
    ((HUDuiLabel*)list[6])->setLabel(buf);
    ((HUDuiLabel*)list[6])->setString("Rogue");
  }
  else {
    ((HUDuiLabel*)list[6])->setLabel("");
    ((HUDuiLabel*)list[6])->setString("");
  }


  std::vector<std::string> args;
  sprintf(buf, "%d", ping.maxShots);
  args.push_back(buf);

  if (ping.maxShots == 1)
    ((HUDuiLabel*)list[7])->setString("{1} Shot", &args );
  else
    ((HUDuiLabel*)list[7])->setString("{1} Shots", &args );

  if (ping.gameStyle & TeamFlagGameStyle)
    ((HUDuiLabel*)list[8])->setString("Capture-the-Flag");
  else
    ((HUDuiLabel*)list[8])->setString("Free-style");

  if (ping.gameStyle & SuperFlagGameStyle)
    ((HUDuiLabel*)list[9])->setString("Super Flags");
  else
    ((HUDuiLabel*)list[9])->setString("");

  if (ping.gameStyle & AntidoteGameStyle)
    ((HUDuiLabel*)list[10])->setString("Antidote Flags");
  else
    ((HUDuiLabel*)list[10])->setString("");

  if ((ping.gameStyle & ShakableGameStyle) && ping.shakeTimeout != 0) {
    std::vector<std::string> args;
    sprintf(buf, "%.1f", 0.1f * float(ping.shakeTimeout));
    args.push_back(buf);
    if (ping.shakeWins == 1)
      ((HUDuiLabel*)list[11])->setString("{1} sec To Drop Bad Flag", &args);
    else
      ((HUDuiLabel*)list[11])->setString("{1} secs To Drop Bad Flag", &args);
  }
  else
    ((HUDuiLabel*)list[11])->setString("");

  if ((ping.gameStyle & ShakableGameStyle) && ping.shakeWins != 0) {
    std::vector<std::string> args;
    sprintf(buf, "%d", ping.shakeWins);
    args.push_back(buf);
    args.push_back(ping.shakeWins == 1 ? "" : "s");
    if (ping.shakeWins == 1)
      ((HUDuiLabel*)list[11])->setString("{1} Win Drops Bad Flag", &args);
    else
      ((HUDuiLabel*)list[11])->setString("{1} Wins Drops Bad Flag", &args);
  }
  else
    ((HUDuiLabel*)list[12])->setString("");

  if (ping.gameStyle & JumpingGameStyle)
    ((HUDuiLabel*)list[13])->setString("Jumping");
  else
    ((HUDuiLabel*)list[13])->setString("");

  if (ping.gameStyle & RicochetGameStyle)
    ((HUDuiLabel*)list[14])->setString("Ricochet");
  else
    ((HUDuiLabel*)list[14])->setString("");

  if (ping.gameStyle & InertiaGameStyle)
    ((HUDuiLabel*)list[15])->setString("Inertia");
  else
    ((HUDuiLabel*)list[15])->setString("");

  if (ping.maxTime != 0) {
    std::vector<std::string> args;
    if (ping.maxTime >= 3600)
      sprintf(buf, "%d:%02d:%02d", ping.maxTime / 3600, (ping.maxTime / 60) % 60, ping.maxTime % 60);
    else if (ping.maxTime >= 60)
      sprintf(buf, "%d:%02d", ping.maxTime / 60, ping.maxTime % 60);
    else
      sprintf(buf, "0:%02d", ping.maxTime);
    args.push_back(buf);
    ((HUDuiLabel*)list[16])->setString("Time limit: {1}", &args);
  }
  else
    ((HUDuiLabel*)list[16])->setString("");

  if (ping.maxTeamScore != 0) {
    std::vector<std::string> args;
    sprintf(buf, "%d", ping.maxTeamScore);
    args.push_back(buf);
    ((HUDuiLabel*)list[17])->setString("Max team score: {1}", &args);
  }
  else 
    ((HUDuiLabel*)list[17])->setString("");


  if (ping.maxPlayerScore != 0) {
    std::vector<std::string> args;
    sprintf(buf, "%d", ping.maxPlayerScore);
    args.push_back(buf);
    ((HUDuiLabel*)list[18])->setString("Max player score: {1}", &args);
  }
  else
    ((HUDuiLabel*)list[18])->setString("");
}

void			ServerMenu::show()
{
  // clear server list
  servers.clear();

  // clear server readouts
  std::vector<HUDuiControl*>& list = getControls();
  ((HUDuiLabel*)list[1])->setLabel("");
  ((HUDuiLabel*)list[2])->setLabel("");
  ((HUDuiLabel*)list[3])->setLabel("");
  ((HUDuiLabel*)list[4])->setLabel("");
  ((HUDuiLabel*)list[5])->setLabel("");
  ((HUDuiLabel*)list[6])->setLabel("");
  ((HUDuiLabel*)list[6])->setString("");
  ((HUDuiLabel*)list[7])->setString("");
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
  
  std::vector<std::string> args;
  args.push_back("0");
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

  // open output multicast socket
  Address multicastAddress(BroadcastAddress);
  struct sockaddr_in pingOutAddr;
  const int pingOutSocket = openMulticast(multicastAddress,
				ServerPort, NULL,
				info->ttl, info->multicastInterface,
				"w", &pingOutAddr);
  pingInSocket = openMulticast(multicastAddress, ServerPort, NULL,
				info->ttl, info->multicastInterface,
				"r", &pingInAddr);

  // send ping and close output socket
  if (pingInSocket != -1 && pingOutSocket != -1) {
    PingPacket::sendRequest(pingOutSocket, &pingOutAddr, info->ttl);
    closeMulticast(pingOutSocket);
  }
  else {
    // clean up
    closeMulticast(pingInSocket);
    closeMulticast(pingOutSocket);
    pingInSocket = -1;
  }

  // also try broadcast
  pingBcastSocket = openBroadcast(BroadcastPort, NULL, &pingBcastAddr);
  if (pingBcastSocket != -1)
    PingPacket::sendRequest(pingBcastSocket, &pingBcastAddr, 1);

  // listen for echos
  addPlayingCallback(&playingCB, this);
}

void			ServerMenu::execute()
{
  if (selectedIndex < 0 || selectedIndex >= servers.size())
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
  closeMulticast(pingInSocket);
  closeMulticast(pingBcastSocket);
  pingInSocket = -1;
  pingBcastSocket = -1;
}

void			ServerMenu::resize(int _width, int _height)
{
  // remember size
  width = _width;
  height = _height;

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
    if (i % 6 == 1) {
      x = (0.125f + 0.25f * (float)((i - 1) / 6)) * (float)width;
      y = y0;
    }

    HUDuiLabel* label = (HUDuiLabel*)list[i];
    label->setFontSize(fontWidth, fontHeight);
    const OpenGLTexFont& font = label->getFont();
    y -= 1.0f * font.getHeight();
    label->setPosition(x, y);
  }

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
  // lookup server list in phase 0
  if (phase == 0) {
    // dereference URL
    std::vector<std::string> urls, failedURLs;
    urls.push_back(getStartupInfo()->listServerURL);
    BzfNetwork::dereferenceURLs(urls, MaxListServers, failedURLs);

    // print urls we failed to open
    int i;
    for (i = 0; i < failedURLs.size(); ++i) {
	std::vector<std::string> args;
	args.push_back(failedURLs[i]);
	printError("Can't open list server: {1}", &args);
    }

    // check urls for validity
    numListServers = 0;
    for (i = 0; i < urls.size(); ++i) {
	// parse url
	std::string protocol, hostname, path;
	int port = ServerPort + 1;
	Address address;
	if (!BzfNetwork::parseURL(urls[i], protocol, hostname, port, path) ||
	    protocol != "bzflist" || port < 1 || port > 65535 ||
	    (address = Address::getHostAddress(hostname.c_str())).isAny()) {
	    std::vector<std::string> args;
	    args.push_back(urls[i]);
	    printError("Can't open list server: {1}", &args);
	    continue;
	}

	// add to list
	listServers[numListServers].address = address;
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
	continue;
      }

      // set to non-blocking.  we don't want to wait for the connection.
      if (BzfNetwork::setNonBlocking(listServer.socket) < 0) {
	printError("Error with list server socket");
	close(listServer.socket);
	listServer.socket = -1;
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

    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 0;

    fd_set read_set, write_set;
    FD_ZERO(&read_set);
    FD_ZERO(&write_set);
    if (pingInSocket != -1) FD_SET(pingInSocket, &read_set);
    if (pingBcastSocket != -1) FD_SET(pingBcastSocket, &read_set);
    int fdMax = (pingInSocket > pingBcastSocket) ?
				pingInSocket : pingBcastSocket;

    // check for list server connection or data
    for (i = 0; i < numListServers; i++) {
      ListServer& listServer = listServers[i];
      if (listServer.socket != -1) {
	if (listServer.phase == 2)
	  FD_SET(listServer.socket, &write_set);
	else if (listServer.phase == 3)
	  FD_SET(listServer.socket, &read_set);
	if (listServer.socket > fdMax)
	  fdMax = listServer.socket;
      }
    }

    const int nfound = select(fdMax+1, (fd_set*)&read_set,
					(fd_set*)&write_set, 0, &timeout);
    if (nfound <= 0) break;

    // check broadcast and multicast sockets
    ServerItem serverInfo;
	sockaddr_in addr;

    if (pingInSocket != -1 && FD_ISSET(pingInSocket, &read_set)) {
		if (serverInfo.ping.read(pingInSocket, &addr)) {
			serverInfo.ping.serverId.serverHost = addr.sin_addr;
			addToListWithLookup(serverInfo);
		}
	}
	if (pingBcastSocket != -1 && FD_ISSET(pingBcastSocket, &read_set)) {
		if (serverInfo.ping.read(pingBcastSocket, &addr)) {
			serverInfo.ping.serverId.serverHost = addr.sin_addr;
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
	  static const char* msg = "LIST\n\n";
#if !defined(_WIN32)
	  // ignore SIGPIPE for this send
	  SIG_PF oldPipeHandler = bzSignal(SIGPIPE, SIG_IGN);
#endif
	  if (send(listServer.socket, msg, strlen(msg), 0) != (int)strlen(msg)) {
	    // probably unable to connect to server
	    close(listServer.socket);
	    listServer.socket = -1;
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
      if (strncmp(version, ServerVersion, 7) == 0 &&
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

	// add to list
	addToList(serverInfo);
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

  addToList(info);
}

int			ServerMenu::getPlayerCount(int index) const
{
  const PingPacket& item = servers[index].ping;
  return item.rogueCount + item.redCount + item.greenCount +
				item.blueCount + item.purpleCount;
}

void			ServerMenu::addToList(ServerItem& info)
{
  // update if we already have it
  const int count = servers.size();
  int i;
  for (i = 0; i < count; i++) {
    ServerItem& server = servers[i];
    if (server.ping.serverId.serverHost.s_addr ==
				info.ping.serverId.serverHost.s_addr &&
	server.ping.serverId.port == info.ping.serverId.port) {
      if (server.description.size() < info.description.size())
	server.description = info.description;
      break;
    }
  }

  // add if we don't already have it
  if (i == count) {
    std::vector<std::string> args;
    char msg[50];
    sprintf(msg, "%d", count + 1);
    args.push_back(msg);
    setStatus("Servers found: {1}", &args);

    // add to server list
    servers.push_back(info);
  }

  // sort by number of players
  const int n = servers.size();
  for (i = 0; i < n - 1; ++i) {
    int indexWithMin = i;
    for (int j = i + 1; j < n; ++j)
      if (getPlayerCount(j) > getPlayerCount(indexWithMin))
	indexWithMin = j;
    ServerItem temp = servers[i];
    servers[i] = servers[indexWithMin];
    servers[indexWithMin] = temp;
  }

  // force update
  const int oldSelectedIndex = selectedIndex;
  selectedIndex = -1;
  setSelected(oldSelectedIndex);
}

void			ServerMenu::playingCB(void* _self)
{
  ((ServerMenu*)_self)->checkEchos();
}

//
// ServerStartMenu
//

char			ServerStartMenu::settings[] = "bfaaaaabaaacaa";

ServerStartMenu::ServerStartMenu()
{
  // add controls
  std::vector<HUDuiControl*>& controls = getControls();
  HUDuiList* list;
  HUDuiLabel* label;
  std::vector<std::string>* items;

  controls.push_back(createLabel("Start Server"));

  list = createList("Style:");
  items = &list->getList();
  items->push_back("Capture the Flag");
  items->push_back("Free for All");
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

  list = createList("Server Visibility:");
  items = &list->getList();
  items->push_back("local host only (ttl=0)");
  items->push_back("subnet only (ttl=1)");
  items->push_back("local area (ttl=8)");
  items->push_back("site (ttl=32)");
  items->push_back("organization (ttl=64)");
  items->push_back("continent (ttl=128)");
  items->push_back("world (ttl=255)");
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

  label = createLabel("Start");
  controls.push_back(label);

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
  if (strlen(_settings) == 13) {
    strcpy(settings, _settings);
    settings[14] = settings[13];
    settings[13] = settings[12];
    settings[12] = 'a';
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
  if (focus == list[15]) {
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
    char serverCmd[512];
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
    else {
      args[arg++] = "-r";
      args[arg++] = "-h";
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

    // server visibility
    static const char* serverTTL[] = { "0", "1", "8", "32", "64", "128", "255" };
    args[arg++] = "-ttl";
    args[arg++] = serverTTL[((HUDuiList*)list[12])->getIndex()];

    // game over
    static const char* gameOverArg[] = { "",
				"-time", "-time", "-time", "-time",
				"-mps", "-mps", "-mps",
				"-mts", "-mts", "-mts", "-mts" };
    static const char* gameOverValue[] = { "",
				"300", "900", "3600", "10800",
				"3", "10", "25",
				"3", "10", "25", "100" };
    if (((HUDuiList*)list[13])->getIndex() != 0) {
      args[arg++] = gameOverArg[((HUDuiList*)list[13])->getIndex()];
      args[arg++] = gameOverValue[((HUDuiList*)list[13])->getIndex()];
    }

    // server reset
    if (((HUDuiList*)list[14])->getIndex() == 0)
      args[arg++] = "-g";

    // no more arguments
    args[arg++] = NULL;

    // start the server
#if defined(_WIN32)

    // Windows
    int result = _spawnvp(_P_DETACH, serverCmd, args);
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

      // exec server and exit if it returns.
      execvp(serverCmd, (char* const*)args);
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
    if (i == 15) {
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
    void		setStatus(const char*, const std::vector<std::string> *parms = NULL);
    void		loadInfo();

  private:
    float		center;
    HUDuiTypeIn*	callsign;
    HUDuiList*		team;
    HUDuiTypeIn*	server;
    HUDuiTypeIn*	port;
    HUDuiLabel*		status;
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

  HUDuiLabel* findServer = new HUDuiLabel;
  findServer->setFont(MainMenu::getFont());
  findServer->setString("Find Server");
  list.push_back(findServer);

  HUDuiLabel* connectLabel = new HUDuiLabel;
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
  teams.push_back(std::string(Team::getName(RogueTeam)));
  teams.push_back(std::string(Team::getName(RedTeam)));
  teams.push_back(std::string(Team::getName(GreenTeam)));
  teams.push_back(std::string(Team::getName(BlueTeam)));
  teams.push_back(std::string(Team::getName(PurpleTeam)));
  team->update();
  team->setIndex((int)info->team);
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

  HUDuiLabel* startServer = new HUDuiLabel;
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
  team->setIndex((int)info->team);
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
  info->team = (TeamColor)team->getIndex();
  strcpy(info->serverName, server->getString().c_str());
  info->serverPort = atoi(port->getString().c_str());
}

void			JoinMenu::execute()
{
  std::vector<HUDuiControl*>& list = getControls();
  HUDuiControl* focus = HUDui::getFocus();
  if (focus == list[7]) {
    if (!serverStartMenu) serverStartMenu = new ServerStartMenu;
    HUDDialogStack::get()->push(serverStartMenu);
  }

  else if (focus == list[1]) {
    if (!serverMenu) serverMenu = new ServerMenu;
    HUDDialogStack::get()->push(serverMenu);
  }

  else if (focus == list[2]) {
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

void			JoinMenu::setStatus(const char* msg, const std::vector<std::string> *parms)
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
    if (i <= 2 || i == 6) y -= 0.5f * h;
  }
}

//
// MainMenu
//

static const char*	titleFile = "title";
OpenGLTexFont*		MainMenu::mainFont = NULL;

MainMenu::MainMenu() : HUDDialog(), joinMenu(NULL),
				optionsMenu(NULL), quitMenu(NULL)
{
  // create font
  font = TextureFont::getTextureFont(TextureFont::HelveticaBold, true);
  mainFont = &font;

  // load title
  title = getTexture(titleFile, OpenGLTexture::Linear, false, true);

  // add controls
  std::vector<HUDuiControl*>& list = getControls();
  HUDuiLabel* label;
  HUDuiTextureLabel* textureLabel;

  textureLabel = new HUDuiTextureLabel;
  textureLabel->setFont(font);
  textureLabel->setTexture(title);
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
// ex: shiftwidth=2 tabstop=8
