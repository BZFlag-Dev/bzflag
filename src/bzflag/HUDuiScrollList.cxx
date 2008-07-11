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

// interface headers
#include "HUDuiScrollList.h"

// system headers
#include <math.h>

// common implementation headers
#include "BundleMgr.h"
#include "Bundle.h"
#include "FontManager.h"
#include "LocalFontFace.h"
#include "HUDui.h"

#include <iostream>

//
// HUDuiScrollList
//

HUDuiScrollList::HUDuiScrollList() : HUDuiNestedContainer(), index(0), visiblePosition(0), numVisibleItems(1), pagedList(false), pageLabel(NULL)
{
  showFocus(false);
  getNav().setCallback(callback, this);
}

HUDuiScrollList::HUDuiScrollList(bool paged) : HUDuiNestedContainer(), index(0), visiblePosition(0), numVisibleItems(1), pagedList(paged), pageLabel(new HUDuiLabel)
{
  // do nothing
}

HUDuiScrollList::~HUDuiScrollList()
{
  // do nothing
}

int HUDuiScrollList::getSelected() const
{
  return index;
}

void HUDuiScrollList::clear()
{
  items.clear();
  setSelected(0);
}

void HUDuiScrollList::setSelected(int _index)
{
  // Ensure the index is not negative, or past the end of our list
  if (_index <= 0)
    _index = 0;
  else if (_index >= (int)items.size())
    _index = (int)items.size() - 1;

  if (pagedList) {
    // Figure out what page the new index is on
    int newPage = (_index/numVisibleItems) + 1;
    visiblePosition = _index - ((newPage - 1)*numVisibleItems);
  } else {
    // The new index falls within the portion of the list already on screen
    if ((_index >= index - visiblePosition)&&(_index < (index + (numVisibleItems - visiblePosition)))) {
      visiblePosition = visiblePosition + (_index - index);
    // Moving one down outside of list range
    } else if (_index == (index + (numVisibleItems - visiblePosition))) {
      visiblePosition = numVisibleItems - 1;
    // Moving one up outside of list range
    } else if (_index == ((index - visiblePosition - 1))) {
      visiblePosition = 0;
    // Jumping to a different part of the list
    } else {
      // Jump to that part of the list and set the new index as first
      visiblePosition = 0;
    }
  }

  index = _index;
}

// Add a new item to our scrollable list
void HUDuiScrollList::addItem(HUDuiControl* item)
{
  items.push_back(item);
  addControl(item);
  update();
}

void HUDuiScrollList::update()
{
  setSelected(index);
}

size_t HUDuiScrollList::callback(size_t oldFocus, size_t proposedFocus, HUDNavChangeMethod changeMethod, void* data)
{
  // Don't scroll up any further once you've hit the top of the list
  if ((oldFocus == 0)&&(changeMethod == hnPrev)) proposedFocus = oldFocus;
  
  // Don't scroll past the bottom of the list
  if ((oldFocus == ((HUDuiScrollList*)data)->getNav().size() - 1)&&(changeMethod == hnNext)) proposedFocus = oldFocus;

  ((HUDuiScrollList*)data)->setSelected((int) proposedFocus);
  return (size_t) ((HUDuiScrollList*)data)->getSelected();
}

// Set the scrollable list to be paged/non-paged
void HUDuiScrollList::setPaged(bool paged)
{
  pagedList = paged;
  if (pagedList) {
    pageLabel = new HUDuiLabel;
  } else {
    pageLabel = NULL;
  }
  resizeItems();
  update();
}

bool HUDuiScrollList::doKeyPress(const BzfKeyEvent& key)
{
  // Figure out what page the user is on
  int currentPage = (index/numVisibleItems) + 1;

  if (key.chr == 0)
    switch (key.button) {

      case BzfKeyEvent::PageUp:
        if ((pagedList)&&(index != -1)) {
          // Jump back to the previous page
          getNav().set((size_t) (currentPage - 2)*numVisibleItems);
        }
        break;

      case BzfKeyEvent::PageDown:
        if ((pagedList)&&(index != -1)) {
          // Skip to the next page
          getNav().set((size_t) (currentPage)*numVisibleItems);
        }
        break;

      // Testing purposes only
      case BzfKeyEvent::Home:
        setPaged(!pagedList);
        break;

      default:
        return false;
  }

  switch (key.chr) {
    case 13:
    case 27:
      return false;
  }

  return true;
}

bool HUDuiScrollList::doKeyRelease(const BzfKeyEvent&)
{
  // ignore key releases
  return false;
}

// Update our scrollable list when the size is changed
void HUDuiScrollList::setSize(float width, float height)
{
  HUDuiControl::setSize(width, height);
  resizeItems();
}

// Update our scrollable list whe the font size is changed
void HUDuiScrollList::setFontSize(float size)
{
  HUDuiControl::setFontSize(size);
  resizeItems();
}

// Change our scrollable list items' sizes to match any changes to our scrollable list
void HUDuiScrollList::resizeItems()
{
  // Determine how many items are visible
  FontManager &fm = FontManager::instance();
  float itemHeight = fm.getStringHeight(getFontFace()->getFMFace(), getFontSize());
  float listHeight = getHeight();
  numVisibleItems = (int)floorf(listHeight/itemHeight);

  // If it's a paged list make it one item shorter so we can fit the page label
  if (pagedList) {
    numVisibleItems = numVisibleItems - 1;
  }

  std::list<HUDuiControl*>::iterator it;

  if (items.size() > 0) {
    for (it = items.begin(); it != items.end(); ++it)
    {
      HUDuiControl* item = *it;
      if (item != NULL)
      {
        item->setFontFace(getFontFace());
        item->setFontSize(getFontSize());
        item->setSize(getWidth(), itemHeight);
      }
    }
  }
}

void HUDuiScrollList::doRender()
{
  FontManager &fm = FontManager::instance();
  float itemHeight = fm.getStringHeight(getFontFace()->getFMFace(), getFontSize());

  std::list<HUDuiControl*>::iterator it;
  it = items.begin();
  std::advance(it, (getSelected() - visiblePosition));

  // Draw the list items
  for (int i = (getSelected() - visiblePosition); i<((numVisibleItems - visiblePosition) + getSelected()); i++) {
    if (i < (int)items.size()) {
      HUDuiControl* item = *it;
      item->setFontSize(getFontSize());
      item->setPosition(getX(), (getY() - itemHeight*(i-(getSelected() - visiblePosition))));
      item->render();
      std::advance(it, 1);
    }
  }

  // Draw the page label
  if (pagedList) {
    int numPages = (((int)items.size() - 1)/numVisibleItems) + 1;
    int currentPage = (getSelected()/numVisibleItems) + 1;

    std::vector<std::string> args;
    char msg[50];

    sprintf(msg, "%ld", currentPage);
    args.push_back(msg);
    sprintf(msg, "%ld", numPages);
    args.push_back(msg);

    pageLabel->setString("Page: {1}/{2}", &args);

    float labelWidth = fm.getStringWidth(getFontFace()->getFMFace(), getFontSize(), pageLabel->getString().c_str());

    pageLabel->setFontFace(getFontFace());
    pageLabel->setFontSize(getFontSize());
    pageLabel->setPosition((getX() +(getWidth() - labelWidth)), (getY() - itemHeight*(numVisibleItems + 1)));
    pageLabel->render();
  }
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
