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

/*
 * HUDuiServerList:
 *	FILL THIS IN
 */

#ifndef	__HUDUISERVERLIST_H__
#define	__HUDUISERVERLIST_H__

// ancestor class
#include "HUDuiScrollList.h"

#include "HUDuiServerListItem.h"
#include "ServerItem.h"
#include "ServerList.h"

class HUDuiServerList : public HUDuiScrollList {
  public:
      HUDuiServerList();
      HUDuiServerList(bool paged);
      ~HUDuiServerList();

    typedef enum {
      EmptyServer  = 0x0002,
      FullServer   = 0x0004,
      Jumping      = 0x0008,
      AntidoteFlag = 0x0010,
      EndOfFilterConstants
    } FilterConstants;

    typedef enum {
      NoSort = 0,
      DomainName,
      ServerName,
      PlayerCount,
      Ping
    } SortConstants;

    void addItem(ServerItem item);
    void addItem(HUDuiControl* item);
	
    void setServerList(ServerList* list);

    ServerItem* getSelectedServer();

    void sortBy(SortConstants sortType);
    void searchServers(std::string pattern);

    void applyFilters();
    void toggleFilter(FilterConstants filter);

  protected:
    struct filter;
    struct search;
    template<int sortType> struct compare;

    static ServerList* dataList;

  private:
    std::list<HUDuiControl*> originalItems;

    SortConstants sortMode;
    uint16_t filterOptions;
};

#endif // __HUDuiServerList_H__

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8