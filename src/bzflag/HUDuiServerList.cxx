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

//
// HUDuiServerList
//

ServerList* HUDuiServerList::dataList = NULL;

HUDuiServerList::HUDuiServerList() : HUDuiScrollList(), filterOptions(0), sortMode(NoSort)
{
  // do nothing
}

HUDuiServerList::HUDuiServerList(bool paged) : HUDuiScrollList(paged), filterOptions(0), sortMode(NoSort)
{
  // do nothing
}

HUDuiServerList::~HUDuiServerList()
{
  // do nothing
}

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

struct HUDuiServerList::filter: public std::binary_function<HUDuiControl*, uint16_t, bool>
{
public:
  result_type operator()(first_argument_type control, second_argument_type filter) const
    {
      HUDuiServerListItem* item = (HUDuiServerListItem*) control;
      ServerItem* server = dataList->lookupServer(item->getServerKey());

      bool returnValue = false;

      for (int i = 0; i < EndOfFilterConstants; ++i)
      {
	if (filter & i)
	{
	  switch (i) {
	    case EmptyServer:
	      returnValue = (server->getPlayerCount() == 0);
	      break;

	    case FullServer:
	      returnValue = (server->getPlayerCount() == server->ping.maxPlayers);
	      break;

	    case Jumping:
	      returnValue = (server->ping.gameOptions & JumpingGameStyle);
	      break;

	    case AntidoteFlag:
	      returnValue = (server->ping.gameOptions & AntidoteGameStyle);
	      break;
	  }
	}
	if (returnValue == true)
	  return true;
      }
      return false;
    }
};

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
  return; // Do nothing
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
  setSelected(0);
  getNav().set((size_t) 0);
}

void HUDuiServerList::applyFilters()
{
  items = originalItems;

  items.remove_if(std::bind2nd(filter(), filterOptions));

  refreshNavQueue();
  setSelected(0);
  getNav().set((size_t) 0);
}

void HUDuiServerList::toggleFilter(FilterConstants filter)
{
  filterOptions ^= filter;
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