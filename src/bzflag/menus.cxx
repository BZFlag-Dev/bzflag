/* bzflag
 * Copyright 1993-1999, Chris Schoeneman
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#if defined(_WIN32)
#include <process.h>
#else
#include <sys/types.h>
#include <sys/wait.h>
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
#include "MainWindow.h"
#include "SceneRenderer.h"
#include "OpenGLTexture.h"
#include "ErrorHandler.h"
#include "KeyMap.h"

//
// MenuDefaultKey
//

MenuDefaultKey		MenuDefaultKey::instance;

MenuDefaultKey::MenuDefaultKey() { }
MenuDefaultKey::~MenuDefaultKey() { }

boolean			MenuDefaultKey::keyPress(const BzfKeyEvent& key)
{
  switch (key.ascii) {
    case 27:	// escape
      HUDDialogStack::get()->pop();
      return True;

    case 13:	// return
      HUDDialogStack::get()->top()->execute();
      return True;
  }

  if (getKeyMap().isMappedTo(KeyMap::Quit, key)) {
    getMainWindow()->setQuit();
    return True;
  }

  return False;
}

boolean			MenuDefaultKey::keyRelease(const BzfKeyEvent& key)
{
  switch (key.ascii) {
    case 27:	// escape
    case 13:	// return
      return True;
  }
  return False;
}

//
// QuitMenu
//

class QuitMenuDefaultKey : public MenuDefaultKey {
  public:
			QuitMenuDefaultKey() { }
			~QuitMenuDefaultKey() { }

    boolean		keyPress(const BzfKeyEvent&);
    boolean		keyRelease(const BzfKeyEvent&);
};

boolean			QuitMenuDefaultKey::keyPress(const BzfKeyEvent& key)
{
  if (key.ascii == 'Y' || key.ascii == 'y') {
    getMainWindow()->setQuit();
    return True;
  }
  return MenuDefaultKey::keyPress(key);
}

boolean			QuitMenuDefaultKey::keyRelease(const BzfKeyEvent& key)
{
  if (key.ascii == 'Y' || key.ascii == 'y')
    return True;
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
  HUDuiControlList& list = getControls();
  HUDuiLabel* label;

  label = new HUDuiLabel;
  label->setFont(MainMenu::getFont());
  label->setString("Enter to quit, Esc to resume");
  list.append(label);

  label = new HUDuiLabel;
  label->setFont(MainMenu::getFont());
  label->setString("Really quit?");
  list.append(label);

  // set control order
  const int count = list.getLength();
  for (int i = 0; i < count; i++) {
    list[i]->setNext(list[i]);
    list[i]->setPrev(list[i]);
  }

  // set initial focus
  setFocus(list[1]);
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
  HUDuiControlList& list = getControls();
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

    boolean		keyPress(const BzfKeyEvent&);
    boolean		keyRelease(const BzfKeyEvent&);

  private:
    FormatMenu*		menu;
};

class FormatMenu : public HUDDialog {
  public:
			FormatMenu();
			~FormatMenu() { }

    HUDuiDefaultKey*	getDefaultKey() { return &defaultKey; }
    void		execute();
    void		resize(int width, int height);

    void		setFormat(boolean test);

  private:
    FormatMenuDefaultKey defaultKey;
    int			numFormats;
    float		center;
};

boolean			FormatMenuDefaultKey::keyPress(const BzfKeyEvent& key)
{
  if (key.ascii == 'T' || key.ascii == 't') {
    menu->setFormat(True);
    return True;
  }
  return MenuDefaultKey::keyPress(key);
}

boolean			FormatMenuDefaultKey::keyRelease(const BzfKeyEvent& key)
{
  if (key.ascii == 'T' || key.ascii == 't')
    return True;
  return MenuDefaultKey::keyRelease(key);
}

FormatMenu::FormatMenu() : defaultKey(this)
{
  // add controls
  HUDuiControlList& list = getControls();

  HUDuiLabel* label = new HUDuiLabel;
  label->setFont(MainMenu::getFont());
  label->setString("Video Format");
  list.append(label);

  BzfDisplay* display = getDisplay();
  numFormats = display->getNumResolutions();
  label = new HUDuiLabel;
  label->setFont(MainMenu::getFont());
  label->setString((numFormats < 2) ? "" :
	"Press Enter to select and T to test a format.  Esc to exit.");
  list.append(label);

  // add formats at end
  if (numFormats < 2) {
    label = new HUDuiLabel;
    label->setFont(MainMenu::getFont());
    label->setString("<switching not available>");
    list.append(label);

    // set initial focus
    setFocus(NULL);
  }

  else {
    char buffer[128];
    sprintf(buffer, "Current format: %s",
		display->getResolution(display->getResolution())->name);
    label = new HUDuiLabel;
    label->setFont(MainMenu::getFont());
    label->setString(buffer);
    list.append(label);

    int i;
    for (i = 0; i < numFormats; i++) {
      label = new HUDuiLabel;
      label->setFont(MainMenu::getFont());
      label->setString(display->getResolution(i)->name);
      label->setCallback(NULL, (void*)i);
      list.append(label);
    }

    // set order
    const int first = 3;
    const int last = first + numFormats - 1;
    for (i = 1; i < numFormats - 1; i++) {
      list[first + i]->setPrev(list[first + i - 1]);
      list[first + i]->setNext(list[first + i + 1]);
    }
    list[first]->setPrev(list[last]);
    list[first]->setNext(list[first + 1]);
    list[last]->setPrev(list[last - 1]);
    list[last]->setNext(list[first]);

    // set initial focus
    setFocus(list[first + display->getResolution()]);
  }
}

void			FormatMenu::execute()
{
  setFormat(False);
}

void			FormatMenu::setFormat(boolean test)
{
  HUDuiLabel* control = (HUDuiLabel*)HUDui::getFocus();
  if (!control) return;
  const int format = (int)control->getUserData();
  if (!setVideoFormat(format, test)) {
    control->setString("<unloadable>");
    control->getPrev()->setNext(control->getNext());
    control->getNext()->setPrev(control->getPrev());
    HUDui::setFocus(control->getNext());
  }
  else if (!test) {
    BzfDisplay* display = getDisplay();
    char buffer[128];
    sprintf(buffer, "Current format: %s",
		display->getResolution(display->getResolution())->name);
    HUDuiControlList& list = getControls();
    HUDuiLabel* label = (HUDuiLabel*)list[2];
    label->setString(buffer);

    const OpenGLTexFont& font = label->getFont();
    const float stringWidth = font.getWidth(label->getString());
    label->setPosition(center - 0.5f * stringWidth, label->getY());
  }
}

void			FormatMenu::resize(int width, int height)
{
  // use a big font for title, smaller font for the rest
  const float titleFontWidth = (float)height / 10.0f;
  const float titleFontHeight = (float)height / 10.0f;

  // reposition title
  float x, y;
  HUDuiControlList& list = getControls();
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
    HUDuiLabel* label = (HUDuiLabel*)list[2];
    label->setFontSize(fontWidth, fontHeight);
    const OpenGLTexFont& font = label->getFont();
    const float stringWidth = font.getWidth(label->getString());
    center = 0.5f * (float)width;
    x = center - 0.5f * stringWidth;
    y -= 1.0f * font.getHeight();
    label->setPosition(x, y);
  }

  // compute font size that'll fit all the allowed formats, but
  // no larger than default.  then put formats in columns and rows.
  // use the minimum number of rows, but arrange formats in columns.
  int columns, rows;
  if (numFormats >= 2) {
    // get pixels left vertically for formats
    float height2 = y - 0.30f * height;

    // compute number of columns and rows to fit all formats
    columns = 3;
    rows = 4 * columns;
    while (columns * rows < numFormats) {
      columns++;
      rows = 4 * columns;
    }

    // find largest font that fits all rows and columns
    HUDuiLabel* label = (HUDuiLabel*)list[3];
    label->setFontSize(1.0f, 1.0f);
    const OpenGLTexFont& font = label->getFont();
    const float defaultSize = 1.0f + font.getWidth("0000x0000x00@000");
    const float unitColumns = ((float)width - 1.0f) / defaultSize;
    const float unitRows = height2 - 1.0f;
    fontWidth = unitColumns / (float)columns;
    fontHeight = unitRows / (float)rows;
    if (fontWidth > fontHeight) {
      if (fontHeight > (float)height / 36.0f)
	fontHeight = (float)height / 36.0f;
      fontWidth = fontHeight;
    }
    else {
      if (fontWidth > (float)height / 36.0f)
	fontWidth = (float)height / 36.0f;
      fontHeight = fontWidth;
    }

    // reposition
    const float dx = (float)(width - fontHeight) / (float)columns;
    x = 2.0f * fontHeight;
    y -= 1.5f * fontHeight;

    ((HUDuiLabel*)list[3])->setFontSize(fontWidth, fontHeight);
    const float h = ((HUDuiLabel*)list[3])->getFont().getHeight();
    rows = (numFormats + columns - 1) / columns;
    for (int i = 0; i < numFormats; i++) {
      const int column = i / rows;
      const int row = i % rows;

      HUDuiLabel* label = (HUDuiLabel*)list[3 + i];
      label->setFontSize(fontWidth, fontHeight);
      label->setPosition(x + dx * (float)column, y - h * (float)row);
    }
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

    boolean		keyPress(const BzfKeyEvent&);
    boolean		keyRelease(const BzfKeyEvent&);

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

    boolean		isEditing() const;
    void		setKey(const BzfKeyEvent&);

  private:
    void		update();

    HUDuiLabel*		createLabel(const char*, const char* = NULL);

  private:
    KeyboardMapMenuDefaultKey defaultKey;
    HUDuiControl*	reset;
    KeyMap::Key		editing;
};

KeyboardMapMenuDefaultKey::KeyboardMapMenuDefaultKey(KeyboardMapMenu* _menu) :
				menu(_menu)
{
  // do nothing
}

boolean			KeyboardMapMenuDefaultKey::keyPress(
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
      return True;
  }

  // all other keys modify mapping
  menu->setKey(key);
  return True;
}

boolean			KeyboardMapMenuDefaultKey::keyRelease(
				const BzfKeyEvent&)
{
  // ignore key releases
  return True;
}

KeyboardMapMenu::KeyboardMapMenu() : defaultKey(this), editing(KeyMap::LastKey)
{
  // add controls
  HUDuiControlList& controls = getControls();

  controls.append(createLabel("Key Mapping"));

  controls.append(reset = createLabel(NULL, "Reset Defaults"));
  controls.append(createLabel(NULL, "Fire shot:"));
  controls.append(createLabel(NULL, "Drop flag:"));
  controls.append(createLabel(NULL, "Identify/Lock On:"));
  controls.append(createLabel(NULL, "Radar Short:"));
  controls.append(createLabel(NULL, "Radar Medium:"));
  controls.append(createLabel(NULL, "Radar Long:"));
  controls.append(createLabel(NULL, "Send to All:"));
  controls.append(createLabel(NULL, "Send to Teammates:"));
  controls.append(createLabel(NULL, "Jump:"));
  controls.append(createLabel(NULL, "Binoculars:"));
  controls.append(createLabel(NULL, "Toggle Score:"));
  controls.append(createLabel(NULL, "Flag Help:"));
  controls.append(createLabel(NULL, "Time Forward:"));
  controls.append(createLabel(NULL, "Time Backward:"));
  controls.append(createLabel(NULL, "Pause/Resume:"));
  controls.append(createLabel(NULL, "Fast Quit:"));

  // set control order
  controls[0]->setNext(controls[0]);
  controls[0]->setPrev(controls[0]);
  controls[1]->setNext(controls[2]);
  controls[2]->setNext(controls[3]);
  controls[3]->setNext(controls[4]);
  controls[4]->setNext(controls[5]);
  controls[5]->setNext(controls[6]);
  controls[6]->setNext(controls[7]);
  controls[7]->setNext(controls[8]);
  controls[8]->setNext(controls[9]);
  controls[9]->setNext(controls[10]);
  controls[10]->setNext(controls[11]);
  controls[11]->setNext(controls[12]);
  controls[12]->setNext(controls[13]);
  controls[13]->setNext(controls[14]);
  controls[14]->setNext(controls[15]);
  controls[15]->setNext(controls[16]);
  controls[16]->setNext(controls[17]);
  controls[17]->setNext(controls[1]);
  controls[1]->setPrev(controls[17]);
  controls[2]->setPrev(controls[1]);
  controls[3]->setPrev(controls[2]);
  controls[4]->setPrev(controls[3]);
  controls[5]->setPrev(controls[4]);
  controls[6]->setPrev(controls[5]);
  controls[7]->setPrev(controls[6]);
  controls[8]->setPrev(controls[7]);
  controls[9]->setPrev(controls[8]);
  controls[10]->setPrev(controls[9]);
  controls[11]->setPrev(controls[10]);
  controls[12]->setPrev(controls[11]);
  controls[13]->setPrev(controls[12]);
  controls[14]->setPrev(controls[13]);
  controls[15]->setPrev(controls[14]);
  controls[16]->setPrev(controls[15]);
  controls[17]->setPrev(controls[16]);

  // set initial focus
  setFocus(controls[1]);
}

boolean			KeyboardMapMenu::isEditing() const
{
  return editing != KeyMap::LastKey;
}

void			KeyboardMapMenu::setKey(const BzfKeyEvent& event)
{
  if (editing != KeyMap::LastKey) {
    // check for previous mapping
    KeyMap& map = getKeyMap();
    const KeyMap::Key previous = map.isMapped(event);

    // ignore setting to same value
    if (previous != editing) {
      // if there was a previous setting remove it
      if (previous != KeyMap::LastKey)
	map.unset(previous, event);

      // map new key
      map.set(editing, event);
    }

    // not editing anymore
    editing = KeyMap::LastKey;

    // make sure we update strings
    update();
  }
}

void			KeyboardMapMenu::execute()
{
  const HUDuiControl* const focus = HUDui::getFocus();
  if (focus == reset) {
    getKeyMap().resetAll();
    update();
  }
  else {
    // start editing
    HUDuiControlList& list = getControls();
    const int count = list.getLength();
    editing = KeyMap::LastKey;
    for (int i = 0; i < (int)KeyMap::LastKey; i++)
      if (list[i + 2] == focus) {
	editing = (KeyMap::Key)i;
	break;
      }

    KeyMap& map = getKeyMap();
    if (editing != KeyMap::LastKey) {
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
  editing = KeyMap::LastKey;
  notifyKeyMapChanged();
}

void			KeyboardMapMenu::resize(int width, int height)
{
  // use a big font for title, smaller font for the rest
  const float titleFontWidth = (float)height / 10.0f;
  const float titleFontHeight = (float)height / 10.0f;
  const float fontWidth = (float)height / 36.0f;
  const float fontHeight = (float)height / 36.0f;

  // reposition title
  HUDuiControlList& list = getControls();
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
  const int count = list.getLength();
  for (int i = 1; i < count; i++) {
    list[i]->setFontSize(fontWidth, fontHeight);
    list[i]->setPosition(x, y);
    y -= 1.0f * h;
    if (i == 1) y -= 0.5f * h;
  }

  update();
}

void			KeyboardMapMenu::update()
{
  // load current settings
  KeyMap& map = getKeyMap();
  HUDuiControlList& list = getControls();
  for (int j = 0; j < (int)KeyMap::LastKey; j++) {
    const BzfKeyEvent& key1 = map.get((KeyMap::Key)j);
    const BzfKeyEvent& key2 = map.getAlternate((KeyMap::Key)j);
    BzfString value;
    if (key1.ascii == 0 && key1.button == 0) {
      if (j == (int)editing)
	value = "???";
      else
	value = "<not mapped>";
    }
    else {
      value += KeyMap::getKeyEventString(key1);
      if (key2.ascii != 0 || key2.button != 0) {
	value += " or ";
	value += KeyMap::getKeyEventString(key2);
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
// OptionsMenu
//

class OptionsMenu : public HUDDialog {
  public:
			OptionsMenu();
			~OptionsMenu() { }

    HUDuiDefaultKey*	getDefaultKey()
				{ return MenuDefaultKey::getInstance(); }
    void		execute();
    void		resize(int width, int height);

    static void		callback(HUDuiControl* w, void* data);

  private:
    HUDuiControl*	videoFormat;
    HUDuiControl*	keyMapping;
};

OptionsMenu::OptionsMenu()
{
  // add controls
  HUDuiControlList& list = getControls();

  HUDuiLabel* label = new HUDuiLabel;
  label->setFont(MainMenu::getFont());
  label->setString("Options");
  list.append(label);

  HUDuiList* option = new HUDuiList;
  option->setFont(MainMenu::getFont());
  option->setLabel("Dithering:");
  option->setCallback(callback, (void*)"1");
  BzfStringAList* options = &option->getList();
  options->append(BzfString("Off"));
  options->append(BzfString("On"));
  option->update();
  list.append(option);

  option = new HUDuiList;
  option->setFont(MainMenu::getFont());
  option->setLabel("Blending:");
  option->setCallback(callback, (void*)"2");
  options = &option->getList();
  options->append(BzfString("Off"));
  options->append(BzfString("On"));
  option->update();
  list.append(option);

  option = new HUDuiList;
  option->setFont(MainMenu::getFont());
  option->setLabel("Smoothing:");
  option->setCallback(callback, (void*)"3");
  options = &option->getList();
  options->append(BzfString("Off"));
  options->append(BzfString("On"));
  option->update();
  list.append(option);

  option = new HUDuiList;
  option->setFont(MainMenu::getFont());
  option->setLabel("Lighting:");
  option->setCallback(callback, (void*)"4");
  options = &option->getList();
  options->append(BzfString("Off"));
  options->append(BzfString("On"));
  option->update();
  list.append(option);

  option = new HUDuiList;
  option->setFont(MainMenu::getFont());
  option->setLabel("Texturing:");
  option->setCallback(callback, (void*)"5");
  options = &option->getList();
  options->append(BzfString("Off"));
  options->append(BzfString("Nearest"));
  options->append(BzfString("Linear"));
  options->append(BzfString("Nearest Mipmap Nearest"));
  options->append(BzfString("Linear Mipmap Nearest"));
  options->append(BzfString("Nearest Mipmap Linear"));
  options->append(BzfString("Linear Mipmap Linear"));
  option->update();
  list.append(option);

  option = new HUDuiList;
  option->setFont(MainMenu::getFont());
  option->setLabel("Quality:");
  option->setCallback(callback, (void*)"6");
  options = &option->getList();
  options->append(BzfString("Low"));
  options->append(BzfString("Medium"));
  options->append(BzfString("High"));
  option->update();
  list.append(option);

  option = new HUDuiList;
  option->setFont(MainMenu::getFont());
  option->setLabel("Shadows:");
  option->setCallback(callback, (void*)"7");
  options = &option->getList();
  options->append(BzfString("Off"));
  options->append(BzfString("On"));
  option->update();
  list.append(option);

  option = new HUDuiList;
  option->setFont(MainMenu::getFont());
  option->setLabel("Depth Buffer:");
  option->setCallback(callback, (void*)"8");
  options = &option->getList();
  GLint value;
  glGetIntegerv(GL_DEPTH_BITS, &value);
  if (value == 0) {
    options->append(BzfString("Not available"));
  }
  else {
    options->append(BzfString("Off"));
    options->append(BzfString("On"));
  }
  option->update();
  list.append(option);

#if defined(DEBUG_RENDERING)
  option = new HUDuiList;
  option->setFont(MainMenu::getFont());
  option->setLabel("Hidden Line:");
  option->setCallback(callback, (void*)"a");
  options = &option->getList();
  options->append(BzfString("Off"));
  options->append(BzfString("On"));
  option->update();
  list.append(option);

  option = new HUDuiList;
  option->setFont(MainMenu::getFont());
  option->setLabel("Wireframe:");
  option->setCallback(callback, (void*)"b");
  options = &option->getList();
  options->append(BzfString("Off"));
  options->append(BzfString("On"));
  option->update();
  list.append(option);

  option = new HUDuiList;
  option->setFont(MainMenu::getFont());
  option->setLabel("Depth Complexity:");
  option->setCallback(callback, (void*)"c");
  options = &option->getList();
  options->append(BzfString("Off"));
  options->append(BzfString("On"));
  option->update();
  list.append(option);
#endif

#if !defined(_WIN32)			// windows only changes at startup
  BzfDisplay* display = getDisplay();
  int numFormats = display->getNumResolutions();
  if (numFormats < 2) {
    videoFormat = NULL;
  }
  else {
    videoFormat = label = new HUDuiLabel;
    label->setFont(MainMenu::getFont());
    label->setLabel("Change Video Format");
    list.append(label);
  }
#else
  videoFormat = NULL;
#endif

  option = new HUDuiList;
  option->setFont(MainMenu::getFont());
  option->setLabel("Sound Volume:");
  option->setCallback(callback, (void*)"s");
  options = &option->getList();
  if (isSoundOpen()) {
    options->append(BzfString("Off"));
    options->append(BzfString("1"));
    options->append(BzfString("2"));
    options->append(BzfString("3"));
    options->append(BzfString("4"));
    options->append(BzfString("5"));
    options->append(BzfString("6"));
    options->append(BzfString("7"));
    options->append(BzfString("8"));
    options->append(BzfString("9"));
    options->append(BzfString("10"));
  }
  else {
    options->append(BzfString("Disabled"));
  }
  option->update();
  list.append(option);

  keyMapping = label = new HUDuiLabel;
  label->setFont(MainMenu::getFont());
  label->setLabel("Change Key Mapping");
  list.append(label);

  // set control order
  const int count = list.getLength();
  list[0]->setNext(list[0]);
  list[0]->setPrev(list[0]);
  list[1]->setPrev(list[count - 1]);
  list[1]->setNext(list[2]);
  for (int j = 2; j < count - 1; j++) {
    list[j]->setPrev(list[j - 1]);
    list[j]->setNext(list[j + 1]);
  }
  list[count - 1]->setPrev(list[count - 2]);
  list[count - 1]->setNext(list[1]);

  // set initial focus
  setFocus(list[1]);
}

void			OptionsMenu::execute()
{
  HUDuiControl* focus = HUDui::getFocus();
  if (focus == videoFormat) {
    static FormatMenu formatMenu;
    HUDDialogStack::get()->push(&formatMenu);
  }
  else if (focus == keyMapping) {
    static KeyboardMapMenu keyboardMapMenu;
    HUDDialogStack::get()->push(&keyboardMapMenu);
  }
}

void			OptionsMenu::resize(int width, int height)
{
  // use a big font for title, smaller font for the rest
  const float titleFontWidth = (float)height / 10.0f;
  const float titleFontHeight = (float)height / 10.0f;
  const float fontWidth = (float)height / 24.0f;
  const float fontHeight = (float)height / 24.0f;

  // reposition title
  HUDuiControlList& list = getControls();
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
  const int count = list.getLength();
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
    ((HUDuiList*)list[i++])->setIndex(getSoundVolume());

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

//
// HelpMenu
//

class HelpMenu;

static HelpMenu*	getHelpMenu(HUDDialog* = NULL, boolean next = True);

class HelpMenuDefaultKey : public MenuDefaultKey {
  public:
			HelpMenuDefaultKey() { }
			~HelpMenuDefaultKey() { }

    boolean		keyPress(const BzfKeyEvent&);
    boolean		keyRelease(const BzfKeyEvent&);
};

class HelpMenu : public HUDDialog {
  public:
			HelpMenu(const char* title = "Help");
			~HelpMenu() { }

    HUDuiDefaultKey*	getDefaultKey() { return &defaultKey; }
    void		execute() { }
    void		resize(int width, int height);

  protected:
    HUDuiControl*	createLabel(const char* string,
				const char* label = NULL);
    virtual float	getLeftSide(int width, int height);

  private:
    HelpMenuDefaultKey	defaultKey;
};

boolean			HelpMenuDefaultKey::keyPress(const BzfKeyEvent& key)
{
  if (key.button == BzfKeyEvent::PageUp) {
    HUDDialog* oldMenu = HUDDialogStack::get()->top();
    HUDDialogStack::get()->pop();
    HUDDialogStack::get()->push(getHelpMenu(oldMenu, False));
    return True;
  }
  if (key.button == BzfKeyEvent::PageDown || key.ascii == 13) {
    HUDDialog* oldMenu = HUDDialogStack::get()->top();
    HUDDialogStack::get()->pop();
    HUDDialogStack::get()->push(getHelpMenu(oldMenu, True));
    return True;
  }
  return MenuDefaultKey::keyPress(key);
}

boolean			HelpMenuDefaultKey::keyRelease(const BzfKeyEvent& key)
{
  if (key.button == BzfKeyEvent::PageUp ||
      key.button == BzfKeyEvent::PageDown || key.ascii == 13)
    return True;
  return MenuDefaultKey::keyRelease(key);
}

HelpMenu::HelpMenu(const char* title) : HUDDialog()
{
  // add controls
  HUDuiControlList& list = getControls();
  list.append(createLabel(title));
  list.append(createLabel("Page Down for next page",
			  "Page Up for previous page"));

  // set control order
  list[1]->setNext(list[1]);
  list[1]->setPrev(list[1]);

  // set initial focus
  setFocus(list[1]);
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
  HUDuiControlList& list = getControls();
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
  const int count = list.getLength();
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
  HUDuiControlList& list = getControls();
  list.append(createLabel("controls tank motion", "Mouse Position:"));
  list.append(createLabel("fires shot"));
  list.append(createLabel("drops flag (if not bad)"));
  list.append(createLabel("identifies player (locks on GM)"));
  list.append(createLabel("jump (if allowed)"));
  list.append(createLabel("short radar range"));
  list.append(createLabel("medium radar range"));
  list.append(createLabel("long radar range"));
  list.append(createLabel("toggle binoculars"));
  list.append(createLabel("toggle heads-up flag help"));
  list.append(createLabel("send message to teammates"));
  list.append(createLabel("send message to everybody"));
  list.append(createLabel("toggle score sheet"));
  list.append(createLabel("set time of day backward"));
  list.append(createLabel("set time of day forward"));
  list.append(createLabel("pause/resume"));
  list.append(createLabel("quit"));
  list.append(createLabel("show/dismiss menu", "Esc:"));
}

float			Help1Menu::getLeftSide(int width, int height)
{
  return 0.5f * width - height / 20.0f;
}

void			Help1Menu::resize(int width, int height)
{
  static const KeyMap::Key key[] = {
				KeyMap::LastKey,
				KeyMap::LastKey,
				KeyMap::LastKey,
				KeyMap::FireShot,
				KeyMap::DropFlag,
				KeyMap::Identify,
				KeyMap::Jump,
				KeyMap::ShortRange,
				KeyMap::MediumRange,
				KeyMap::LongRange,
				KeyMap::Binoculars,
				KeyMap::FlagHelp,
				KeyMap::SendTeam,
				KeyMap::SendAll,
				KeyMap::Score,
				KeyMap::TimeForward,
				KeyMap::TimeBackward,
				KeyMap::Pause,
				KeyMap::Quit
			};

  // get current key mapping and set strings appropriately
  KeyMap& map = getKeyMap();
  HUDuiControlList& list = getControls();
  for (int j = 0; j < sizeof(key) / sizeof(key[0]); j++) {
    if (key[j] == KeyMap::LastKey) continue;

    BzfString value;
    const BzfKeyEvent& key1 = map.get(key[j]);
    if (key1.ascii == 0 && key1.button == 0) {
      value = "<not mapped>";
    }
    else {
      value = KeyMap::getKeyEventString(key1);
      const BzfKeyEvent& key2 = map.getAlternate(key[j]);
      if (key2.ascii != 0 || key2.button != 0) {
	value += " or ";
	value += KeyMap::getKeyEventString(key2);
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
  HUDuiControlList& list = getControls();
  list.append(createLabel(
	"BZFLAG is a multi-player networked tank battle game.  There are five teams:"));
  list.append(createLabel(
	"red, green, blue, purple, and rogues (rogue tanks are black).  Destroying a"));
  list.append(createLabel(
	"player on another team scores a win, while being destroyed or destroying a"));
  list.append(createLabel(
	"teammate scores a loss.  Individual and aggregate team scores are tallied."));
  list.append(createLabel(
	"Rogues have no teammates (not even other rogues),so they cannot shoot"));
  list.append(createLabel(
	"teammates and they don't have a team score."));
  list.append(createLabel(""));
  list.append(createLabel(
	"There are two styles of play, determined by the server configuration:  capture-"));
  list.append(createLabel(
	"the-flag and free-for-all.  In free-for-all the object is simply to get the"));
  list.append(createLabel(
	"highest score by shooting opponents.  The object in capture-the-flag is to"));
  list.append(createLabel(
	"capture enemy flags while preventing opponents from capturing yours.  In this"));
  list.append(createLabel(
	"style, each team (but not rogues) has a team base and each team with at least"));
  list.append(createLabel(
	"one player has a team flag which has the color of the team.  To capture a flag,"));
  list.append(createLabel(
	"you must grab it and bring it back to your team base (you must be on the ground"));
  list.append(createLabel(
	"in your base to register the capture).  Capturing a flag destroys all the players"));
  list.append(createLabel(
	"on that team and gives your team score a bonus;  the players will restart on"));
  list.append(createLabel(
	"their team base.  Taking your flag onto an enemy base counts as a capture against"));
  list.append(createLabel(
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
  HUDuiControlList& list = getControls();
  list.append(createLabel(
	"The world environment contains an outer wall and several buildings."));
  list.append(createLabel(
	"You cannot go outside the outer wall (you can't even jump over it)."));
  list.append(createLabel(
	"You cannot normally drive or shoot through buildings."));
  list.append(createLabel(
	""));
  list.append(createLabel(
	"The server may be configured to include teleporters:  large transparent"));
  list.append(createLabel(
	"black slabs.  Objects entering one side of a teleporter are instantly"));
  list.append(createLabel(
	"moved to one side of another (or possibly the same) teleporter.  The"));
  list.append(createLabel(
	"teleport is reversible;  reentering the same side of the destination"));
  list.append(createLabel(
	"teleporter brings you back to where you started.  Teleport connections"));
  list.append(createLabel(
	"are fixed at the start of the game and don't change during the game."));
  list.append(createLabel(
	"The connections are always the same in the capture-the-flag style."));
  list.append(createLabel(
	"Each side of a teleporter teleports independently of the other side."));
  list.append(createLabel(
	"It's possible for a teleporter to teleport to the opposite side of"));
  list.append(createLabel(
	"itself.  Such a thru-teleporter acts almost as if it wasn't there."));
  list.append(createLabel(
	"A teleporter can also teleport to the same side of itself.  This is a"));
  list.append(createLabel(
	"reverse teleporter.  Shooting at a reverse teleporter is likely to be"));
  list.append(createLabel(
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
  HUDuiControlList& list = getControls();
  list.append(createLabel(
	"Flags come in two varieties:  team flags and super flags.  Team flags"));
  list.append(createLabel(
	"are used only in the capture-the-flag style.  The server may also be"));
  list.append(createLabel(
	"configured to supply super flags, which give your tank some advantage"));
  list.append(createLabel(
	"or disadvantage.  You normally can't tell which until you pick one up,"));
  list.append(createLabel(
	"but good flags generally outnumber bad flags two to one."));
  list.append(createLabel(
	""));
  list.append(createLabel(
	"Team flags are not allowed to be in Bad Places.  Bad Places are:  on"));
  list.append(createLabel(
	"a building or on an enemy base.  Team flags dropped in a Bad Place are"));
  list.append(createLabel(
	"moved to a safety position.  Captured flags are placed back on their"));
  list.append(createLabel(
	"team base.  Super flags dropped above a building always disappear."));
  list.append(createLabel(
	""));
  list.append(createLabel(
	"A random good super flag will remain for up to 4 possessions.  After"));
  list.append(createLabel(
	"that it'll disappear and will eventually be replaced by a new random"));
  list.append(createLabel(
	"flag.  Bad random super flags disappear after the first possession."));
  list.append(createLabel(
	"Bad super flags can't normally be dropped.  The server can be set to"));
  list.append(createLabel(
	"automatically drop the flag for you after some time, after you destroy"));
  list.append(createLabel(
	"a certain number of enemies, and/or when you grab an antidote flag."));
  list.append(createLabel(
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
  HUDuiControlList& list = getControls();
  list.append(createLabel(
	"", "Good Flags:"));
  list.append(createLabel(
	"boosts top speed", "High Speed (V)"));
  list.append(createLabel(
	"boosts turn rate", "Quick Turn (A)"));
  list.append(createLabel(
	"can drive through buildings", "Oscillation Overthruster (OO)"));
  list.append(createLabel(
	"faster shots more often", "Rapid Fire (F)"));
  list.append(createLabel(
	"very fast reload, very short range", "Machine Gun (MG)"));
  list.append(createLabel(
	"shots guide themselves (right mouse locks on)", "Guided Missile (GM)"));
  list.append(createLabel(
	"infinite shot speed and range, long reload time", "Laser (L)"));
  list.append(createLabel(
	"shots ricochet", "Ricochet (R)"));
  list.append(createLabel(
	"shoots through buildings", "Super Bullet (SB)"));
  list.append(createLabel(
	"tank invisible on enemy radar", "Stealth (ST)"));
  list.append(createLabel(
	"tank invisible out the window", "Cloaking (CL)"));
  list.append(createLabel(
	"shots invisible on radar", "Invisible Bullet (IB)"));
  list.append(createLabel(
	"tank becomes smaller", "Tiny (T)"));
  list.append(createLabel(
	"tank becomes paper thin", "Narrow (N)"));
  list.append(createLabel(
	"getting hit just drops the flag", "Shield(SH)"));
  list.append(createLabel(
	"destroy tanks by touching them", "Steamroller (SR)"));
  list.append(createLabel(
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
  HUDuiControlList& list = getControls();
  list.append(createLabel(
	"teleport to enter/leave zone", "Phantom Zone (PZ)"));
  list.append(createLabel(
	"destroys player and all player's teammates", "Genocide (G)"));
  list.append(createLabel(
	"allows tank to jump", "Jumping (JP)"));
  list.append(createLabel(
	"shows type of nearest superflag", "Identify (ID)"));
  list.append(createLabel(
	"", ""));
  list.append(createLabel(
	"", "Bad Flags:"));
  list.append(createLabel(
	"can't identify tanks", "Colorblindness (CB)"));
  list.append(createLabel(
	"makes tank very large", "Obesity (O)"));
  list.append(createLabel(
	"tank can't turn right", "Left Turn Only (<-)"));
  list.append(createLabel(
	"tank can't turn left", "Right Turn Only (->)"));
  list.append(createLabel(
	"tank has lots of momentum", "Momentum (M)"));
  list.append(createLabel(
	"can't see out the window", "Blindness (B)"));
  list.append(createLabel(
	"can't see anything on radar", "Jamming (JM)"));
  list.append(createLabel(
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
  HUDuiControlList& list = getControls();
  list.append(createLabel(
	"The radar is on the left side of the control panel.  It shows an overhead"));
  list.append(createLabel(
	"x-ray view of the game.  Buildings and the outer wall are shown in light"));
  list.append(createLabel(
	"blue.  Team bases are outlined in the team color.  Teleporters are short"));
  list.append(createLabel(
	"yellow lines.  Tanks are dots in the tank's team color, except rogues are"));
  list.append(createLabel(
	"yellow.  The size of the tank's dot is a rough indication of the tank's"));
  list.append(createLabel(
	"altitude:  higher tanks have larger dots.  Flags are small crosses.  Team"));
  list.append(createLabel(
	"flags are in the team color, superflags are white, and the antidote flag"));
  list.append(createLabel(
	"is yellow.  Shots are small dots (or lines or circles, for lasers and"));
  list.append(createLabel(
	"shock waves, respectively).  Your tank is always dead center and forward"));
  list.append(createLabel(
	"is always up on the radar.  The yellow V is your field of view.  North"));
  list.append(createLabel(
	"is indicated by the letter N."));
  list.append(createLabel(
	""));
  list.append(createLabel(
	"The heads-up-display (HUD) has several displays.  The two boxes in the"));
  list.append(createLabel(
	"center of the view are the motion control boxes;  within the small box"));
  list.append(createLabel(
	"your tank won't move, outside the large box you don't move any faster than"));
  list.append(createLabel(
	"at the edge of the large box.  Moving the mouse above or below the small"));
  list.append(createLabel(
	"box moves forward or backward, respectively.  Similarly for left and right."));
  list.append(createLabel(
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
  HUDuiControlList& list = getControls();
  list.append(createLabel(
	"Above the larger box is a tape showing your current heading.  North is"));
  list.append(createLabel(
	"0, east is 90, etc.  If jumping is allowed or you have the jumping flag,"));
  list.append(createLabel(
	"an altitude tape appears to the right of the larger box."));
  list.append(createLabel(
	""));
  list.append(createLabel(
	"Small colored diamonds or arrows may appear on the heading tape.  An"));
  list.append(createLabel(
	"arrow pointing left means that a particular flag is to your left, an"));
  list.append(createLabel(
	"arrow pointing right means that the flag is to your right, and a diamond"));
  list.append(createLabel(
	"indicates the heading to the flag by its position on the heading tape."));
  list.append(createLabel(
	"In capture-the-flag mode a marker always shows where your team flag is."));
  list.append(createLabel(
	"A yellow marker shows the way to the antidote flag."));
  list.append(createLabel(
	""));
  list.append(createLabel(
	"At the top of the display are, from left to right, your callsign and"));
  list.append(createLabel(
	"score, your status, and the flag you have.  Your callsign is in the"));
  list.append(createLabel(
	"color of your team.  Your status is one of:  ready, dead, sealed, zoned"));
  list.append(createLabel(
	"or reloading (showing the time until reloaded).  It can also show the"));
  list.append(createLabel(
	"time until a bad flag is dropped (if there's a time limit)."));
  list.append(createLabel(
	""));
  list.append(createLabel(
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
  HUDuiControlList& list = getControls();
  list.append(createLabel("Chris Schoeneman", "Author:"));
  list.append(createLabel("", ""));
  list.append(createLabel("David Hoeferlin, Tom Hubina", "Code Contributors:"));
  list.append(createLabel("Dan Kartch, Jed Lengyel", ""));
  list.append(createLabel("Jeff Myers, Tim Olson", ""));
  list.append(createLabel("Brian Smits, Greg Spencer", ""));
  list.append(createLabel("Daryll Strauss", ""));
  list.append(createLabel("", ""));
  list.append(createLabel("Tamar Cohen", "Tank Models:"));
  list.append(createLabel("", ""));
  list.append(createLabel("Kevin Novins, Rick Pasetto", "Special Thanks:"));
  list.append(createLabel("Adam Rosen, Erin Shaw", ""));
  list.append(createLabel("Ben Trumbore, Don Greenberg", ""));
  list.append(createLabel("", ""));
  list.append(createLabel("bzflag@bigfoot.com", "BZFLAG Email:"));
  list.append(createLabel("http://www.bigfoot.com/~bzflag",
						"BZFLAG Home Page:"));
  list.append(createLabel("", ""));
  list.append(createLabel("Chris Schoeneman", "Copyright 1993-1999"));
}

float			Help9Menu::getLeftSide(int width, int height)
{
  return 0.5f * width - height / 20.0f;
}

//
// help menu getter
//

static HelpMenu*	getHelpMenu(HUDDialog* dialog, boolean next)
{
  static Help1Menu help1;
  static Help2Menu help2;
  static Help3Menu help3;
  static Help4Menu help4;
  static Help5Menu help5;
  static Help6Menu help6;
  static Help7Menu help7;
  static Help8Menu help8;
  static Help9Menu help9;
  if (dialog == &help1) return next ? (HelpMenu*)&help2 : (HelpMenu*)&help9;
  if (dialog == &help2) return next ? (HelpMenu*)&help3 : (HelpMenu*)&help1;
  if (dialog == &help3) return next ? (HelpMenu*)&help4 : (HelpMenu*)&help2;
  if (dialog == &help4) return next ? (HelpMenu*)&help5 : (HelpMenu*)&help3;
  if (dialog == &help5) return next ? (HelpMenu*)&help6 : (HelpMenu*)&help4;
  if (dialog == &help6) return next ? (HelpMenu*)&help7 : (HelpMenu*)&help5;
  if (dialog == &help7) return next ? (HelpMenu*)&help8 : (HelpMenu*)&help6;
  if (dialog == &help8) return next ? (HelpMenu*)&help9 : (HelpMenu*)&help7;
  if (dialog == &help9) return next ? (HelpMenu*)&help1 : (HelpMenu*)&help8;
  return next ? (HelpMenu*)&help1 : (HelpMenu*)&help9;
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

    boolean		keyPress(const BzfKeyEvent&);
    boolean		keyRelease(const BzfKeyEvent&);

  private:
    ServerMenu*		menu;
};

class ServerItem {
  public:
    BzfString		name;
    BzfString		nameAndPort;
    PingPacket		ping;
};
BZF_DEFINE_ALIST(ServerList, ServerItem);

class ServerMenu : public HUDDialog {
  public:
			ServerMenu();
			~ServerMenu() { }

    HUDuiDefaultKey*	getDefaultKey() { return &defaultKey; }
    void		pick();
    void		show();
    void		execute();
    void		dismiss();
    void		resize(int width, int height);

  private:
    void		addLabel(const char* string, const char* label);
    void		checkEchos();
    void		addToList(ServerItem&);
    static void		playingCB(void*);

  private:
    ServerMenuDefaultKey defaultKey;
    ServerList		servers;
    int			pingInSocket;
    struct sockaddr_in	pingInAddr;
    int			pingBcastSocket;
    struct sockaddr_in	pingBcastAddr;
    int			width, height;
    static const int	NumReadouts;
};

boolean			ServerMenuDefaultKey::keyPress(const BzfKeyEvent& key)
{
  if (key.ascii == 0) switch (key.button) {
    case BzfKeyEvent::Up:
      if (HUDui::getFocus()) {
	HUDui::setFocus(HUDui::getFocus()->getPrev());
	menu->pick();
      }
      return True;

    case BzfKeyEvent::Down:
      if (HUDui::getFocus()) {
	HUDui::setFocus(HUDui::getFocus()->getNext());
	menu->pick();
      }
      return True;
  }

  else if (key.ascii == '\t') {
    if (HUDui::getFocus()) {
      HUDui::setFocus(HUDui::getFocus()->getNext());
      menu->pick();
    }
    return True;
  }

  return MenuDefaultKey::keyPress(key);
}

boolean			ServerMenuDefaultKey::keyRelease(const BzfKeyEvent& key)
{
  switch (key.button) {
    case BzfKeyEvent::Up:
    case BzfKeyEvent::Down:
      return True;
  }
  switch (key.ascii) {
    case 27:	// escape
    case 13:	// return
      return True;
  }
  return False;
}

const int		ServerMenu::NumReadouts = 19;

ServerMenu::ServerMenu() : defaultKey(this),
				pingInSocket(-1),
				pingBcastSocket(-1)
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

  // set initial focus
  setFocus(NULL);
}

void			ServerMenu::addLabel(
				const char* msg, const char* _label)
{
  HUDuiLabel* label = new HUDuiLabel;
  label->setFont(MainMenu::getFont());
  label->setString(msg);
  label->setLabel(_label);
  getControls().append(label);
}

void			ServerMenu::pick()
{
  // get index of server from picked control
  HUDuiLabel* label = (HUDuiLabel*)HUDui::getFocus();
  if (!label) return;
  const int index = (int)label->getUserData();

  // get server info
  const ServerItem& item = servers[index];
  const PingPacket& ping = item.ping;

  // update server readouts
  char buf[60];
  HUDuiControlList& list = getControls();
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

  sprintf(buf, "%d Shot%s", ping.maxShots, ping.maxShots == 1 ? "" : "s");
  ((HUDuiLabel*)list[7])->setString(buf);

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
    sprintf(buf, "%.1fsec To Drop Bad Flag", 0.1f * float(ping.shakeTimeout));
    ((HUDuiLabel*)list[11])->setString(buf);
  }
  else
    ((HUDuiLabel*)list[11])->setString("");

  if ((ping.gameStyle & ShakableGameStyle) && ping.shakeWins != 0) {
    sprintf(buf, "%d Win%s Drops Bad Flag", ping.shakeWins,
					ping.shakeWins == 1 ? "" : "s");
    ((HUDuiLabel*)list[12])->setString(buf);
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
    if (ping.maxTime >= 3600)
      sprintf(buf, "Time limit: %d:%02d:%02d",
		ping.maxTime / 3600, (ping.maxTime / 60) % 60, ping.maxTime % 60);
    else if (ping.maxTime >= 60)
      sprintf(buf, "Time limit: %d:%02d", ping.maxTime / 60, ping.maxTime % 60);
    else
      sprintf(buf, "Time limit: 0:%02d", ping.maxTime);
    ((HUDuiLabel*)list[16])->setString(buf);
  }
  else
    ((HUDuiLabel*)list[16])->setString("");

  if (ping.maxTeamScore != 0)
    sprintf(buf, "Max team score: %d", ping.maxTeamScore);
  else
    sprintf(buf, "");
  ((HUDuiLabel*)list[17])->setString(buf);

  if (ping.maxPlayerScore != 0)
    sprintf(buf, "Max player score: %d", ping.maxPlayerScore);
  else
    sprintf(buf, "");
  ((HUDuiLabel*)list[18])->setString(buf);
}

void			ServerMenu::show()
{
  // clear server list
  servers.removeAll();

  // clear server readouts
  HUDuiControlList& list = getControls();
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

  // no focus
  HUDui::setFocus(NULL);

  // remove server labels
  const int count = list.getLength();
  for (int i = count - 1; i >= NumReadouts; i--) {
    delete list[i];
    list.remove(i);
  }

  // open output multicast socket
  const StartupInfo* info = getStartupInfo();
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
  // get index of server from picked control
  HUDuiLabel* label = (HUDuiLabel*)HUDui::getFocus();
  if (!label) return;
  const int index = (int)label->getUserData();

  // update startup info
  StartupInfo* info = getStartupInfo();
  strcpy(info->serverName, servers[index].name);
  info->serverPort = ntohs((unsigned short)servers[index].ping.serverId.port);

  // all done
  HUDDialogStack::get()->pop();
}

void			ServerMenu::dismiss()
{
  // no more callbacks
  removePlayingCallback(&playingCB, this);

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
  HUDuiControlList& list = getControls();
  const int numServers = list.getLength() - NumReadouts;

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
  for (i = 1; i < NumReadouts; i++) {
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

  // compute font size that'll fit all the allowed formats, but
  // no larger than default.
  int columns, rows;
  if (numServers != 0) {
    // get pixels left vertically for formats
    float height2 = y - 0.30f * height;

    // compute number of columns and rows to fit all formats
    columns = 2;
    rows = 8 * columns;
    while (columns * rows < numServers) {
      columns++;
      rows = 8 * columns;
    }

    // find largest font that fits all rows and columns
    HUDuiLabel* label = (HUDuiLabel*)list[NumReadouts];
    label->setFontSize(1.0f, 1.0f);
    const OpenGLTexFont& font = label->getFont();
    const float defaultSize = (1.0f +
		font.getWidth("NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN")) /
		font.getWidth();
    const float unitColumns = ((float)width - 1.0f) / defaultSize;
    const float unitRows = height2 - 1.0f;
    fontWidth = unitColumns / (float)columns;
    fontHeight = unitRows / (float)rows;
    if (fontWidth > fontHeight) {
      if (fontHeight > (float)height / 36.0f)
	fontHeight = (float)height / 36.0f;
      fontWidth = fontHeight;
    }
    else {
      if (fontWidth > (float)height / 36.0f)
	fontWidth = (float)height / 36.0f;
      fontHeight = fontWidth;
    }

    label->setFontSize(fontWidth, fontHeight);
    const float h = label->getFont().getHeight();

    // put servers in columns and rows.  use the minimum number of
    // rows, but arrange formats in columns.
    const float dx = (float)(width - h) / (float)columns;
    x = 2.0f * h;
    y -= 7.0f * 1.0f * h;

    rows = (numServers + columns - 1) / columns;
    for (i = 0; i < numServers; i++) {
      const int column = i / rows;
      const int row = i % rows;

      HUDuiLabel* label = (HUDuiLabel*)list[NumReadouts + i];
      label->setFontSize(fontWidth, fontHeight);
      label->setPosition(x + dx * (float)column, y - h * (float)row);
    }
  }

  else {
    // do nothing -- no servers
  }
}

void			ServerMenu::checkEchos()
{
  // get echo messages
  while (1) {
    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 0;

    fd_set read_set;
    FD_ZERO(&read_set);
    if (pingInSocket != -1) FD_SET(pingInSocket, &read_set);
    if (pingBcastSocket != -1) FD_SET(pingBcastSocket, &read_set);
    const int fdMax = (pingInSocket > pingBcastSocket) ?
				pingInSocket : pingBcastSocket;

    const int nfound = select(fdMax+1, (fd_set*)&read_set, 0, 0, &timeout);
    if (nfound <= 0) break;

    ServerItem serverInfo;
    if (pingInSocket != -1 && FD_ISSET(pingInSocket, &read_set))
      if (serverInfo.ping.read(pingInSocket, NULL))
	addToList(serverInfo);
    if (pingBcastSocket != -1 && FD_ISSET(pingBcastSocket, &read_set))
      if (serverInfo.ping.read(pingBcastSocket, NULL))
	addToList(serverInfo);
  }
}

void			ServerMenu::addToList(ServerItem& info)
{
  // get name and don't wait forever.
  info.name = Address::getHostByAddress(info.ping.serverId.serverHost);
  info.nameAndPort = info.name;

  // tack on port number if not default
  const int port = (int)ntohs((unsigned short)info.ping.serverId.port);
  if (port != ServerPort) {
    char portBuf[20];
    sprintf(portBuf, "%d", port);
    info.nameAndPort += ":";
    info.nameAndPort += portBuf;
  }

  // update if we already have it
  HUDuiControlList& list = getControls();
  const int count = servers.getLength();
  int i;
  for (i = 0; i < count; i++) {
    const ServerItem& server = servers[i];
    if (server.ping.serverId == info.ping.serverId) {
      servers[i] = info;
      ((HUDuiLabel*)list[NumReadouts + i])->setString(info.nameAndPort);
      break;
    }
  }

  // add if we don't already have it
  if (i == count) {
    // add to server list
    servers.append(info);

    // add a new label
    HUDuiLabel* label = new HUDuiLabel;
    label->setFont(MainMenu::getFont());
    label->setString(info.nameAndPort);
    label->setCallback(NULL, (void*)count);
    list.append(label);

    // set next/prev
    if (count == 0) {
      label->setPrev(label);
      label->setNext(label);
      HUDui::setFocus(label);
    }
    else {
      label->setNext(list[NumReadouts]);
      label->setPrev(list[NumReadouts + count - 1]);
      list[NumReadouts]->setPrev(label);
      list[NumReadouts + count - 1]->setNext(label);
    }

    // re-layout
    resize(width, height);
  }

  pick();
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
  HUDuiControlList& controls = getControls();
  HUDuiList* list;
  HUDuiLabel* label;
  BzfStringAList* items;

  controls.append(createLabel("Start Server"));

  list = createList("Style:");
  items = &list->getList();
  items->append("Capture the Flag");
  items->append("Free for All");
  list->update();
  controls.append(list);

  list = createList("Max Players:");
  items = &list->getList();
  items->append("2");
  items->append("3");
  items->append("4");
  items->append("8");
  items->append("20");
  items->append("40");
  list->update();
  controls.append(list);

  list = createList("Max Shots:");
  items = &list->getList();
  items->append("1");
  items->append("2");
  items->append("3");
  items->append("4");
  items->append("5");
  list->update();
  controls.append(list);

  list = createList("Teleporters:");
  items = &list->getList();
  items->append("no");
  items->append("yes");
  list->update();
  controls.append(list);

  list = createList("Ricochet:");
  items = &list->getList();
  items->append("no");
  items->append("yes");
  list->update();
  controls.append(list);

  list = createList("Jumping:");
  items = &list->getList();
  items->append("no");
  items->append("yes");
  list->update();
  controls.append(list);

  list = createList("Superflags:");
  items = &list->getList();
  items->append("no");
  items->append("good flags only");
  items->append("all flags");
  list->update();
  controls.append(list);

  list = createList("Max Superflags:");
  items = &list->getList();
  items->append("10");
  items->append("20");
  items->append("30");
  items->append("40");
  list->update();
  controls.append(list);

  list = createList("Bad Flag Antidote:");
  items = &list->getList();
  items->append("no");
  items->append("yes");
  list->update();
  controls.append(list);

  list = createList("Bad Flag Time Limit:");
  items = &list->getList();
  items->append("no limit");
  items->append("15 seconds");
  items->append("30 seconds");
  items->append("60 seconds");
  items->append("180 seconds");
  list->update();
  controls.append(list);

  list = createList("Bad Flag Win Limit:");
  items = &list->getList();
  items->append("no limit");
  items->append("drop after 1 win");
  items->append("drop after 2 wins");
  items->append("drop after 3 wins");
  list->update();
  controls.append(list);

  list = createList("Server Visibility:");
  items = &list->getList();
  items->append("local host only (ttl=0)");
  items->append("subnet only (ttl=1)");
  items->append("local area (ttl=8)");
  items->append("site (ttl=32)");
  items->append("organization (ttl=64)");
  items->append("continent (ttl=128)");
  items->append("world (ttl=255)");
  list->update();
  controls.append(list);

  list = createList("Game Over:");
  items = &list->getList();
  items->append("never");
  items->append("after 5 minutes");
  items->append("after 15 minutes");
  items->append("after 60 minutes");
  items->append("after 3 hours");
  items->append("when a player gets +3");
  items->append("when a player gets +10");
  items->append("when a player gets +25");
  items->append("when a team gets +3");
  items->append("when a team gets +10");
  items->append("when a team gets +25");
  items->append("when a team gets +100");
  list->update();
  controls.append(list);

  list = createList("Server Reset:");
  items = &list->getList();
  items->append("no, quit after game");
  items->append("yes, reset for more games");
  list->update();
  controls.append(list);

  label = createLabel("Start");
  controls.append(label);

  status = createLabel("");
  controls.append(status);

  failedMessage = createLabel("");
  controls.append(failedMessage);

  // set control order
  controls[0]->setNext(controls[0]);
  controls[0]->setPrev(controls[0]);
  controls[16]->setNext(controls[16]);
  controls[16]->setPrev(controls[16]);
  controls[17]->setNext(controls[17]);
  controls[17]->setPrev(controls[17]);
  controls[1]->setNext(controls[2]);
  controls[2]->setNext(controls[3]);
  controls[3]->setNext(controls[4]);
  controls[4]->setNext(controls[5]);
  controls[5]->setNext(controls[6]);
  controls[6]->setNext(controls[7]);
  controls[7]->setNext(controls[8]);
  controls[8]->setNext(controls[9]);
  controls[9]->setNext(controls[10]);
  controls[10]->setNext(controls[11]);
  controls[11]->setNext(controls[12]);
  controls[12]->setNext(controls[13]);
  controls[13]->setNext(controls[14]);
  controls[14]->setNext(controls[15]);
  controls[15]->setNext(controls[1]);
  controls[1]->setPrev(controls[15]);
  controls[2]->setPrev(controls[1]);
  controls[3]->setPrev(controls[2]);
  controls[4]->setPrev(controls[3]);
  controls[5]->setPrev(controls[4]);
  controls[6]->setPrev(controls[5]);
  controls[7]->setPrev(controls[6]);
  controls[8]->setPrev(controls[7]);
  controls[9]->setPrev(controls[8]);
  controls[10]->setPrev(controls[9]);
  controls[11]->setPrev(controls[10]);
  controls[12]->setPrev(controls[11]);
  controls[13]->setPrev(controls[12]);
  controls[14]->setPrev(controls[13]);
  controls[15]->setPrev(controls[14]);

  // set settings
  loadSettings();

  // set initial focus
  setFocus(controls[1]);
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
  HUDuiControlList& controls = getControls();
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
  HUDuiControlList& controls = getControls();
  char* scan = settings;
  for (int i = 1; *scan; i++, scan++)
    *scan = (char)('a' + ((HUDuiList*)controls[i])->getIndex());
}

void			ServerStartMenu::execute()
{
  static const char*	serverApp = "bzfs";

  HUDuiControlList& list = getControls();
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

void			ServerStartMenu::setStatus(const char* msg)
{
  status->setString(msg);
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
  HUDuiControlList& list = getControls();
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
  const int count = list.getLength();
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
			~JoinMenu() { }

    HUDuiDefaultKey*	getDefaultKey()
				{ return MenuDefaultKey::getInstance(); }
    void		show();
    void		execute();
    void		dismiss();
    void		resize(int width, int height);

  private:
    static void		teamCallback(HUDuiControl*, void*);
    static void		joinGameCallback(boolean, void*);
    static void		joinErrorCallback(const char* msg);
    void		setStatus(const char*);
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
    static JoinMenu*	activeMenu;
};

JoinMenu*		JoinMenu::activeMenu = NULL;

JoinMenu::JoinMenu() : oldErrorCallback(NULL)
{
  // add controls
  HUDuiControlList& list = getControls();
  StartupInfo* info = getStartupInfo();

  HUDuiLabel* label = new HUDuiLabel;
  label->setFont(MainMenu::getFont());
  label->setString("Join Game");
  list.append(label);

  HUDuiLabel* startServer = new HUDuiLabel;
  startServer->setFont(MainMenu::getFont());
  startServer->setString("Start Server");
  list.append(startServer);

  HUDuiLabel* findServer = new HUDuiLabel;
  findServer->setFont(MainMenu::getFont());
  findServer->setString("Find Server");
  list.append(findServer);

  HUDuiLabel* connectLabel = new HUDuiLabel;
  connectLabel->setFont(MainMenu::getFont());
  connectLabel->setString("Connect");
  list.append(connectLabel);

  callsign = new HUDuiTypeIn;
  callsign->setFont(MainMenu::getFont());
  callsign->setLabel("Callsign:");
  callsign->setMaxLength(CallSignLen - 1);
  callsign->setString(info->callsign);
  list.append(callsign);

  team = new HUDuiList;
  team->setFont(MainMenu::getFont());
  team->setLabel("Team:");
  team->setCallback(teamCallback, NULL);
  BzfStringAList& teams = team->getList();
  teams.append(BzfString(Team::getName(RogueTeam)));
  teams.append(BzfString(Team::getName(RedTeam)));
  teams.append(BzfString(Team::getName(GreenTeam)));
  teams.append(BzfString(Team::getName(BlueTeam)));
  teams.append(BzfString(Team::getName(PurpleTeam)));
  team->update();
  team->setIndex((int)info->team);
  list.append(team);

  server = new HUDuiTypeIn;
  server->setFont(MainMenu::getFont());
  server->setLabel("Server:");
  server->setMaxLength(64);
  server->setString(info->serverName);
  list.append(server);

  char buffer[10];
  sprintf(buffer, "%d", info->serverPort);
  port = new HUDuiTypeIn;
  port->setFont(MainMenu::getFont());
  port->setLabel("Port:");
  port->setMaxLength(5);
  port->setString(buffer);
  list.append(port);

  status = new HUDuiLabel;
  status->setFont(MainMenu::getFont());
  status->setString("");
  list.append(status);

  failedMessage = new HUDuiLabel;
  failedMessage->setFont(MainMenu::getFont());
  failedMessage->setString("");
  list.append(failedMessage);

  // set control order
  list[0]->setNext(list[0]);
  list[0]->setPrev(list[0]);
  list[8]->setNext(list[8]);
  list[8]->setPrev(list[8]);
  list[9]->setNext(list[9]);
  list[9]->setPrev(list[9]);
  list[1]->setNext(list[2]);
  list[2]->setNext(list[3]);
  list[3]->setNext(list[4]);
  list[4]->setNext(list[5]);
  list[5]->setNext(list[6]);
  list[6]->setNext(list[7]);
  list[7]->setNext(list[1]);
  list[1]->setPrev(list[7]);
  list[2]->setPrev(list[1]);
  list[3]->setPrev(list[2]);
  list[4]->setPrev(list[3]);
  list[5]->setPrev(list[4]);
  list[6]->setPrev(list[5]);
  list[7]->setPrev(list[6]);

  // set initial focus
  setFocus(list[1]);
}

void			JoinMenu::show()
{
  activeMenu = this;

  HUDuiControlList& list = getControls();
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
  strcpy(info->callsign, callsign->getString());
  info->team = (TeamColor)team->getIndex();
  strcpy(info->serverName, server->getString());
  info->serverPort = atoi(port->getString());
}

void			JoinMenu::execute()
{
  HUDuiControlList& list = getControls();
  HUDuiControl* focus = HUDui::getFocus();
  if (focus == list[1]) {
    static ServerStartMenu serverStartMenu;
    HUDDialogStack::get()->push(&serverStartMenu);
  }

  else if (focus == list[2]) {
    static ServerMenu serverMenu;
    HUDDialogStack::get()->push(&serverMenu);
  }

  else if (focus == list[3]) {
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

void			JoinMenu::joinGameCallback(boolean okay, void* _self)
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

void			JoinMenu::setStatus(const char* msg)
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
  HUDuiControlList& list = getControls();
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
  const int count = list.getLength();
  for (int i = 1; i < count; i++) {
    list[i]->setFontSize(fontWidth, fontHeight);
    list[i]->setPosition(x, y);
    y -= 1.0f * h;
    if (i <= 3) y -= 0.5f * h;
  }
}

//
// MainMenu
//

static const char*	titleFile = "title";
OpenGLTexFont		MainMenu::font;
OpenGLTexture		MainMenu::title;

MainMenu::MainMenu() : HUDDialog()
{
  // force creation of font
  getFont();

  // load title
  title = getTexture(titleFile, OpenGLTexture::Linear, False, True);

  // add controls
  HUDuiControlList& list = getControls();
  HUDuiLabel* label;
  HUDuiTextureLabel* textureLabel;

  textureLabel = new HUDuiTextureLabel;
  textureLabel->setFont(font);
  textureLabel->setTexture(title);
  textureLabel->setString("BZFLAG");
  list.append(textureLabel);

  label = new HUDuiLabel;
  label->setFont(font);
  label->setString("Up/Down arrows to move, Enter to select, Esc to dismiss");
  list.append(label);

  label = new HUDuiLabel;
  label->setFont(font);
  label->setString("Join Game");
  list.append(label);

  label = new HUDuiLabel;
  label->setFont(font);
  label->setString("Options");
  list.append(label);

  label = new HUDuiLabel;
  label->setFont(font);
  label->setString("Help");
  list.append(label);

  label = new HUDuiLabel;
  label->setFont(font);
  label->setString("Quit");
  list.append(label);

  // set control order
  list[0]->setNext(list[0]);
  list[0]->setPrev(list[0]);
  list[1]->setNext(list[1]);
  list[1]->setPrev(list[1]);
  list[2]->setNext(list[3]);
  list[3]->setNext(list[4]);
  list[4]->setNext(list[5]);
  list[5]->setNext(list[2]);
  list[2]->setPrev(list[5]);
  list[3]->setPrev(list[2]);
  list[4]->setPrev(list[3]);
  list[5]->setPrev(list[4]);

  // set initial focus
  setFocus(list[2]);
}

MainMenu::~MainMenu()
{
  // do nothing
}

const OpenGLTexFont&	MainMenu::getFont()
{
  // make font
  if (!font.isValid())
    font = TextureFont::getTextureFont(TextureFont::HelveticaBoldItalic, True);
  return font;
}

HUDuiDefaultKey*	MainMenu::getDefaultKey()
{
  return MenuDefaultKey::getInstance();
}

void			MainMenu::execute()
{
  HUDuiControlList& list = getControls();
  HUDuiControl* focus = HUDui::getFocus();
  if (focus == list[2]) {
    static JoinMenu joinMenu;
    HUDDialogStack::get()->push(&joinMenu);
  }
  else if (focus == list[3]) {
    static OptionsMenu optionsMenu;
    HUDDialogStack::get()->push(&optionsMenu);
  }
  else if (focus == list[4]) {
    HUDDialogStack::get()->push(getHelpMenu());
  }
  else if (focus == list[5]) {
    static QuitMenu quitMenu;
    HUDDialogStack::get()->push(&quitMenu);
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
  HUDuiControlList& list = getControls();
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
  const int count = list.getLength();
  for (int i = 2; i < count; i++) {
    HUDuiLabel* label = (HUDuiLabel*)list[i];
    label->setFontSize(fontWidth, fontHeight);
    label->setPosition(x, y);
    y -= 1.2f * fontHeight;
  }
}
