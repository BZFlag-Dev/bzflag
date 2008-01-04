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

/* interface header */
#include "FormatMenu.h"

/* common implementation headers */
#include "BzfDisplay.h"
#include "ErrorHandler.h"
#include "FontManager.h"

/* local implementation headers */
#include "FontSizer.h"
#include "MainMenu.h"
#include "playing.h"
#include "HUDui.h"
#include "HUDNavigationQueue.h"

const int		FormatMenu::NumReadouts = 4;
const int		FormatMenu::NumItems = 30;
const int		FormatMenu::NumColumns = 3;

bool FormatMenuDefaultKey::keyPress(const BzfKeyEvent& key)
{
  if (key.ascii == 0) switch (key.button) {
    case BzfKeyEvent::PageUp:
      if (HUDui::getFocus()) {
	menu->setPage(menu->page - 1);
      }
      return true;

    case BzfKeyEvent::PageDown:
      if (HUDui::getFocus()) {
	menu->setPage(menu->page + 1);
      }
      return true;

  } else if (key.ascii == 'T' || key.ascii == 't') {
    menu->setFormat(true);
    return true;
  }
  return MenuDefaultKey::keyPress(key);
}

bool FormatMenuDefaultKey::keyRelease(const BzfKeyEvent& key)
{
  switch (key.button) {
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

size_t FormatMenu::navCallback(size_t oldFocus, size_t proposedFocus, HUDNavChangeMethod changeMethod, void* obj)
{
  FormatMenu* fm = (FormatMenu*)obj;
  // sync the menu's selected page with the focus item.
  if (changeMethod == hnNext) {
    if (oldFocus + 1 != proposedFocus) {
      // we have wrapped
      fm->setPage(fm->page + 1);
    } else if (((HUDuiLabel*)(fm->getNav()[proposedFocus]))->getString() == "") {
      // there was an odd number of items on the last page, and 
      // we have run off the end of the list...wrap early
      fm->setPage(0);
      proposedFocus = 0;
    }
  } else if (changeMethod == hnPrev) {
    if (oldFocus - 1 != proposedFocus) {
      // we have wrapped backwards
      fm->setPage(fm->page - 1);
      // find the last non-empty entry (i.e. if we wrapped from first to last page)
      while (((HUDuiLabel*)(fm->getNav()[proposedFocus]))->getString() == "")
	--proposedFocus;
    }
  }
  return proposedFocus;
}

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
  currentLabel = (HUDuiLabel*)(getElements()[NumReadouts - 2]);
  pageLabel = (HUDuiLabel*)(getElements()[NumReadouts - 1]);

  // add resolution list items
  for (i = 0; i < NumItems; ++i)
    addLabel("", "", true);

  // fill in static labels
  if (numFormats < 2) {
    currentLabel->setString("<switching not available>");
    HUDui::setFocus(NULL);
  } else {
    HUDuiLabel* label = (HUDuiLabel*)(getElements()[NumReadouts - 3]);
    label->setString("Press Enter to select and T to test a format. Esc to exit.");
    initNavigation();
    getNav().setCallback(&navCallback, this);
  }
}

FormatMenu::~FormatMenu()
{
  delete[] badFormats;
}

void FormatMenu::addLabel(const char* msg, const char* _label, bool navigable)
{
  HUDuiLabel* label = new HUDuiLabel;
  label->setFontFace(MainMenu::getFontFace());
  label->setString(msg);
  label->setLabel(_label);
  addControl(label, navigable);
}

void FormatMenu::setPage(int _page)
{
  if (_page == page) // no change
    return;

  // switch pages
  page = _page;

  // wrap from last page to first
  if (page > (int)(numFormats / NumItems))
    page = 0;
  // wrap from first page back to last
  if (page < 0)
    page = (int)(numFormats / NumItems);

  // fill items
  const int base = page * NumItems;
  std::vector<HUDuiElement*>& listHUD = getElements();
  BzfDisplay* display = getDisplay();
  for (int i = 0; i < NumItems; ++i) {
    HUDuiLabel* label = (HUDuiLabel*)listHUD[i + NumReadouts];
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
    sprintf(msg, "%d", page + 1);
    args.push_back(msg);
    sprintf(msg, "%d", (numFormats + NumItems - 1) / NumItems);
    args.push_back(msg);
    pageLabel->setString("Page {1} of {2}", &args);
  }
}

void FormatMenu::show()
{
  pageLabel->setString("");
  int selectedIndex = getDisplay()->getResolution();
  // sync focus and selection
  int _page = (selectedIndex / NumItems);
  setPage(_page);
  getNav().set(selectedIndex - (_page * NumItems));
  // set current format
  BzfDisplay* display = getDisplay();
  currentLabel->setString(display->getResolution(display->getResolution())->name);
}

void FormatMenu::execute()
{
  setFormat(false);
}

void FormatMenu::setFormat(bool test)
{
  int selectedIndex = (page * NumItems) + (int)getNav().getIndex();
  if (selectedIndex >= numFormats || badFormats[selectedIndex])
    return;

  if (!setVideoFormat(selectedIndex, test)) {
    // can't load format
    badFormats[selectedIndex] = true;
  } else if (!test) {
    // print OpenGL renderer, which might have changed
    printError((const char*)glGetString(GL_RENDERER));
  }

  BzfDisplay* display = getDisplay();
  currentLabel->setString(display->getResolution(display->getResolution())->name);
}

void FormatMenu::resize(int _width, int _height)
{
  HUDDialog::resize(_width, _height);
  FontSizer fs = FontSizer(_width, _height);

  FontManager &fm = FontManager::instance();
  int fontFace = MainMenu::getFontFace();

  // use a big font for title, smaller font for the rest
  fs.setMin(0, (int)(1.0 / BZDB.eval("headerFontSize") / 2.0));
  const float titleFontSize = fs.getFontSize(fontFace, "headerFontSize");

  // reposition title
  float x, y;
  std::vector<HUDuiElement*>& listHUD = getElements();
  {
    HUDuiLabel* title = (HUDuiLabel*)listHUD[0];
    title->setFontSize(titleFontSize);
    const float titleWidth = fm.getStringWidth(fontFace, titleFontSize, title->getString().c_str());
    const float titleHeight = fm.getStringHeight(fontFace, titleFontSize);
    x = 0.5f * ((float)_width - titleWidth);
    y = (float)_height - titleHeight;
    title->setPosition(x, y);
  }

  // reposition test and current format messages
  fs.setMin(0, 20);
  float fontSize = fs.getFontSize(fontFace, "menuFontSize");
  {
    HUDuiLabel* label = (HUDuiLabel*)listHUD[1];
    label->setFontSize(fontSize);
    const float stringWidth = fm.getStringWidth(fontFace, fontSize, label->getString().c_str());
    x = 0.5f * ((float)_width - stringWidth);
    y -= 1.5f * fm.getStringHeight(fontFace, fontSize);
    label->setPosition(x, y);
  }
  {
    HUDuiLabel* label = currentLabel;
    label->setFontSize(fontSize);
    y -= 1.0f * fm.getStringHeight(fontFace, fontSize);
    label->setPosition(0.5f * (float)_width, y);
  }

  // position page readout
  fontSize = fs.getFontSize(fontFace, "menuFontSize");
  {
    HUDuiLabel* label = pageLabel;
    label->setFontSize(fontSize);
    const float stringWidth = fm.getStringWidth(fontFace, fontSize, label->getString().c_str());
    x = 0.5f * ((float)_width - stringWidth);
    y -= 2.0f * fm.getStringHeight(fontFace, fontSize);
    label->setPosition(x, y);
  }

  // position format item list
  const float yBase = y;
  int lastColumn = -1;
  for (int i = 0; i < NumItems; ++i) {
    const int column = i * NumColumns / NumItems;
    if (column != lastColumn) {
      lastColumn = column;
      x = (float)_width * ((0.5f + (float)column) / (float)(NumColumns + 1));
      y = yBase;
    }

    HUDuiLabel* label = (HUDuiLabel*)listHUD[i + NumReadouts];
    label->setFontSize(fontSize);
    y -= 1.0f * fm.getStringHeight(fontFace, fontSize);
    label->setPosition(x, y);
  }
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
