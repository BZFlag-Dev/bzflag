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

// common implementation headers
#include "BundleMgr.h"
#include "Bundle.h"
#include "FontManager.h"
#include "LocalFontFace.h"
#include "HUDui.h"

//
// HUDuiServerList
//

HUDuiServerList::HUDuiServerList() : HUDuiScrollList(), dataList(NULL)
{
  // do nothing
}

HUDuiServerList::HUDuiServerList(bool paged) : HUDuiScrollList(paged), dataList(NULL)
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

// Over-ride the generic HUDuiControl version of addItem
void HUDuiServerList::addItem(HUDuiControl* item)
{
  // Do nothing
  return;
}

void HUDuiServerList::setServerList(ServerList* list)
{
  dataList = list;
}


ServerItem* HUDuiServerList::getSelectedServer()
{
  std::list<HUDuiControl*>::iterator it;
  it = items.begin();
  std::advance(it, getSelected());

  HUDuiServerListItem* selected = (HUDuiServerListItem*) *it;
  return dataList->lookupServer(selected->getServerKey());
}

// Internal domain name compare function
bool HUDuiServerList::compare_by_domain(HUDuiControl* first, HUDuiControl* second)
{
  HUDuiServerListItem* _first = (HUDuiServerListItem*) first;
  HUDuiServerListItem* _second = (HUDuiServerListItem*) second;
  if (_first->getDomainName().compare(_second->getDomainName()) < 0)
    return true;
  else
    return false;
}

// Internal server name compare function
bool HUDuiServerList::compare_by_name(HUDuiControl* first, HUDuiControl* second)
{
  HUDuiServerListItem* _first = (HUDuiServerListItem*) first;
  HUDuiServerListItem* _second = (HUDuiServerListItem*) second;
  if (_first->getServerName().compare(_second->getServerName()) < 0)
    return true;
  else
    return false;
}

// Internal player count compare function
bool HUDuiServerList::compare_by_players(HUDuiControl* first, HUDuiControl* second)
{
  HUDuiServerListItem* _first = (HUDuiServerListItem*) first;
  HUDuiServerListItem* _second = (HUDuiServerListItem*) second;
  if (_first->getPlayerCount().compare(_second->getPlayerCount()) < 0)
    return true;
  else
    return false;
}

// Internal ping compare function
bool HUDuiServerList::compare_by_ping(HUDuiControl* first, HUDuiControl* second)
{
  HUDuiServerListItem* _first = (HUDuiServerListItem*) first;
  HUDuiServerListItem* _second = (HUDuiServerListItem*) second;
  if (_first->getServerPing().compare(_second->getServerPing()) < 0)
    return true;
  else
    return false;
}

// Sort our server list by domain names
void HUDuiServerList::sortByDomain()
{
  items.sort(compare_by_domain);
  refreshNavQueue();
  setSelected((int) getNav().getIndex());
}

// Sort our server list by server names
void HUDuiServerList::sortByServerName()
{
  items.sort(compare_by_name);
  refreshNavQueue();
  setSelected((int) getNav().getIndex());
}

// Sort our server list by player counts
void HUDuiServerList::sortByPlayerCount()
{
  items.sort(compare_by_players);
  refreshNavQueue();
  setSelected((int) getNav().getIndex());
}

// Sort our server list by ping
void HUDuiServerList::sortByPing()
{
  items.sort(compare_by_ping);
  refreshNavQueue();
  setSelected((int) getNav().getIndex());
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8