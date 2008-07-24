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

#include "BzfEvent.h"

class HUDuiServerList : public HUDuiScrollList {
  public:
      HUDuiServerList();
      HUDuiServerList(bool paged);
      ~HUDuiServerList();

    void addItem(ServerItem item);
    void addItem(HUDuiControl* item);
	
    void setServerList(ServerList* list);

    ServerItem* getSelectedServer();

    void sortBy(int sortType);

    const static int EmptyServer;
    const static int FullServer;
    const static int Jumping;
    const static int AntidoteFlag;

    const static int DomainName;
    const static int ServerName;
    const static int PlayerCount;
    const static int Ping;

    void applyFilters();
    void toggleFilter(int filter);

  protected:
    static bool compare_by_domain(HUDuiControl* first, HUDuiControl* second);
    static bool compare_by_name(HUDuiControl* first, HUDuiControl* second);
    static bool compare_by_players(HUDuiControl* first, HUDuiControl* second);
    static bool compare_by_ping(HUDuiControl* first, HUDuiControl* second);

    static bool is_empty(const HUDuiControl* value);
    static bool is_full(const HUDuiControl* value);
    static bool has_jumping(const HUDuiControl* value);
    static bool has_antidote_flags(const HUDuiControl* value);

  private:
    static ServerList* dataList;
    std::list<HUDuiControl*> originalItems;

    bool emptyServerFilter;
    bool fullServerFilter;
    bool jumpingFilter;
    bool antidoteFlagFilter;
};

#endif // __HUDuiServerList_H__

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8