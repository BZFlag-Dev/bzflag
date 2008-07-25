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
#include "TextUtils.h"
#include "bzglob.h"
#include "BundleMgr.h"
#include "Bundle.h"
#include "FontManager.h"
#include "LocalFontFace.h"

//
// HUDuiServerList
//

ServerList* HUDuiServerList::dataList = NULL;

struct HUDuiServerList::search: public std::binary_function<HUDuiControl*, std::string, bool>
{
public:
  result_type operator()(first_argument_type control, second_argument_type pattern) const
    {
      HUDuiServerListItem* item = (HUDuiServerListItem*) control;

      return !(glob_match(TextUtils::tolower(pattern), TextUtils::tolower(item->getServerName())));
    }
};

template<int sortType> struct HUDuiServerList::compare: public std::binary_function<HUDuiControl*, HUDuiControl*, bool>
{
public:
  bool operator()(HUDuiControl* first, HUDuiControl* second) const
    {
      HUDuiServerListItem* _first = (HUDuiServerListItem*) first;
      HUDuiServerListItem* _second = (HUDuiServerListItem*) second;

      switch (sortType) {
	case DomainName:
	  return (_first->getDomainName().compare(_second->getDomainName()) < 0);
	  break;

	case ServerName:
	  return (_first->getServerName().compare(_second->getServerName()) < 0);
	  break;

	case PlayerCount:
	  return (_first->getPlayerCount().compare(_second->getPlayerCount()) < 0);
	  break;

	case Ping:
	  return (_first->getServerPing().compare(_second->getServerPing()) < 0);
	  break;
      }
      return false;
    }
};

struct HUDuiServerList::filter: public std::binary_function<HUDuiControl*, FilterConstants, bool>
{
public:
  result_type operator()(first_argument_type control, second_argument_type filter) const
    {
      HUDuiServerListItem* item = (HUDuiServerListItem*) control;
      ServerItem* server = dataList->lookupServer(item->getServerKey());

      switch (filter) {
	case EmptyServer:
	  return (server->getPlayerCount() == 0);
	  break;

	case FullServer:
	  return (server->getPlayerCount() == server->ping.maxPlayers);
	  break;

	case Jumping:
	  return (server->ping.gameOptions & JumpingGameStyle);
	  break;

	case AntidoteFlag:
	  return (server->ping.gameOptions & AntidoteGameStyle);
	  break;
      }
      return false;
    }
};

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

void HUDuiServerList::searchServers(std::string pattern)
{
  applyFilters();
  items.remove_if(std::bind2nd(search(), pattern));
  refreshNavQueue();
  getNav().set((size_t) 0);
}

void HUDuiServerList::applyFilters()
{
  items = originalItems;

  if (emptyServerFilter)
    items.remove_if(std::bind2nd(filter(), HUDuiServerList::EmptyServer));

  if (fullServerFilter)
    items.remove_if(std::bind2nd(filter(), HUDuiServerList::FullServer));

  if (jumpingFilter)
    items.remove_if(std::bind2nd(filter(), HUDuiServerList::Jumping));

  if (antidoteFlagFilter)
    items.remove_if(std::bind2nd(filter(), HUDuiServerList::AntidoteFlag));

  refreshNavQueue();
  getNav().set((size_t) 0);
}

void HUDuiServerList::toggleFilter(FilterConstants filter)
{
  switch (filter) {
    case EmptyServer:
      emptyServerFilter = !emptyServerFilter;
      break;

    case FullServer:
      fullServerFilter = !fullServerFilter;
      break;

    case Jumping:
      jumpingFilter = !jumpingFilter;
      break;

    case AntidoteFlag:
      antidoteFlagFilter = !antidoteFlagFilter;
      break;
  }
  applyFilters();
}

void HUDuiServerList::sortBy(SortConstants sortType)
{
  sortMode = sortType;

  switch (sortType) {
    case DomainName:
      items.sort(compare<DomainName>());
      break;

    case ServerName:
      items.sort(compare<ServerName>());
      break;

    case PlayerCount:
      items.sort(compare<PlayerCount>());
      break;

    case Ping:
      items.sort(compare<Ping>());
      break;
  }

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