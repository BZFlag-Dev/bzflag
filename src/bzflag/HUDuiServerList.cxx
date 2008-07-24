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

ServerList* HUDuiServerList::dataList = NULL;

const int HUDuiServerList::EmptyServer = 0;
const int HUDuiServerList::FullServer = 1;
const int HUDuiServerList::Jumping = 2;
const int HUDuiServerList::AntidoteFlag = 3;

const int HUDuiServerList::DomainName = 0;
const int HUDuiServerList::ServerName = 1;
const int HUDuiServerList::PlayerCount = 2;
const int HUDuiServerList::Ping = 3;


HUDuiServerList::HUDuiServerList() : HUDuiScrollList(), emptyServerFilter(false), fullServerFilter(false), jumpingFilter(false), antidoteFlagFilter(false)
{
  // do nothing
}

HUDuiServerList::HUDuiServerList(bool paged) : HUDuiScrollList(paged), emptyServerFilter(false), fullServerFilter(false), jumpingFilter(false), antidoteFlagFilter(false)
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
  originalItems.push_back(newItem);
  items.push_back(newItem);
  newItem->setFontFace(getFontFace());
  newItem->setFontSize(getFontSize());
  addControl(newItem);
  resizeItems(); // May not be very efficient way of doing it
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
  if ((items.size() <= 0)||(dataList == NULL))
    return NULL;

  std::list<HUDuiControl*>::iterator it;
  it = items.begin();
  std::advance(it, getSelected());

  HUDuiServerListItem* selected = (HUDuiServerListItem*) *it;
  return dataList->lookupServer(selected->getServerKey());
}

void HUDuiServerList::applyFilters()
{
  items = originalItems;

  if (emptyServerFilter)
    items.remove_if(is_empty);

  if (fullServerFilter)
    items.remove_if(is_full);

  if (jumpingFilter)
    items.remove_if(has_jumping);

  if (antidoteFlagFilter)
    items.remove_if(has_antidote_flags);

  refreshNavQueue();
  getNav().set((size_t) 0);
}

void HUDuiServerList::toggleFilter(int filter)
{
  switch (filter) {
    case HUDuiServerList::EmptyServer:
      emptyServerFilter = !emptyServerFilter;
      break;

    case HUDuiServerList::FullServer:
      fullServerFilter = !fullServerFilter;
      break;

    case HUDuiServerList::Jumping:
      jumpingFilter = !jumpingFilter;
      break;

    case HUDuiServerList::AntidoteFlag:
      antidoteFlagFilter = !antidoteFlagFilter;
      break;
  }
  applyFilters();
}

void HUDuiServerList::sortBy(int sortType)
{
  switch (sortType) {
    case HUDuiServerList::DomainName:
      items.sort(compare_by_domain);
      break;

    case HUDuiServerList::ServerName:
      items.sort(compare_by_name);
      break;

    case HUDuiServerList::PlayerCount:
      items.sort(compare_by_players);
      break;

    case HUDuiServerList::Ping:
      items.sort(compare_by_ping);
      break;
  }
  refreshNavQueue();
  setSelected((int) getNav().getIndex());
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

bool HUDuiServerList::is_empty(const HUDuiControl* value)
{
  HUDuiServerListItem* item = (HUDuiServerListItem*) value;
  ServerItem* server = dataList->lookupServer(item->getServerKey());

  if (server->getPlayerCount() == 0)
    return true;
  else
    return false;
}

bool HUDuiServerList::is_full(const HUDuiControl* value)
{
  HUDuiServerListItem* item = (HUDuiServerListItem*) value;
  ServerItem* server = dataList->lookupServer(item->getServerKey());

  if (server->getPlayerCount() == server->ping.maxPlayers)
    return true;
  else
    return false;
}

bool HUDuiServerList::has_jumping(const HUDuiControl* value)
{
  HUDuiServerListItem* item = (HUDuiServerListItem*) value;
  ServerItem* server = dataList->lookupServer(item->getServerKey());

  if (server->ping.gameOptions & JumpingGameStyle)
    return true;
  else
    return false;
}

bool HUDuiServerList::has_antidote_flags(const HUDuiControl* value)
{
  HUDuiServerListItem* item = (HUDuiServerListItem*) value;
  ServerItem* server = dataList->lookupServer(item->getServerKey());

  if (server->ping.gameOptions & AntidoteGameStyle)
    return true;
  else
    return false;
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8