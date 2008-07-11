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
#include "HUDuiServerList.h"

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
// HUDuiServerList
//

HUDuiServerList::HUDuiServerList() : HUDuiScrollList()
{
  // do nothing
}

HUDuiServerList::HUDuiServerList(bool paged) : HUDuiScrollList(paged)
{
  // do nothing
}

HUDuiServerList::~HUDuiServerList()
{
  // do nothing
}

// Add a new item to our scrollable list
void HUDuiServerList::addItem(ServerItem item)
{
  HUDuiServerListItem* newItem = new HUDuiServerListItem(item);
  newItem->setFontFace(getFontFace());
  newItem->setFontSize(getFontSize());
  HUDuiScrollList::addItem(newItem);
  update();
}

//
// Add a new item to our scrollable list
void HUDuiServerList::addItem(HUDuiServerListItem* item)
{
  //HUDuiServerListItem* newItem = new HUDuiServerListItem(item);
  item->setFontFace(getFontFace());
  item->setFontSize(getFontSize());
  HUDuiScrollList::addItem(item);
  update();
}
//
//

// Over-ride the generic HUDuiControl version of addItem
void HUDuiServerList::addItem(HUDuiControl* item)
{
  // Do nothing
  return;
}

// Internal alphabetical compare function
//bool HUDuiServerList::compare_alphabetically(HUDuiServerListItem* first, HUDuiServerListItem* second)
//{
//  if (first->getValue().compare(second->getValue()) < 0)
//    return true;
//  else
//    return false;
//}

// Sort our scrollable list alphabetically
//void HUDuiServerList::sortAlphabetically()
//{
//  items.sort(compare_alphabetically);
//}

// Change our scrollable list items' sizes to match any changes to our scrollable list
/*
void HUDuiServerList::resizeItems()
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

  std::list<HUDuiServerListItem*>::iterator it;

  if (items.size() > 0) {
    for (it = items.begin(); it != items.end(); ++it)
    {
      HUDuiServerListItem* test = *it;
      if (test != NULL)
      {
	test->setFontFace(getFontFace());
	test->setFontSize(getFontSize());
	test->shorten(getWidth());
      }
    }
  }
}
*/

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8