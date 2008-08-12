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
 *	User interface class for displaying the servers in a ServerList.
 */

#ifndef	__HUDUISERVERLIST_H__
#define	__HUDUISERVERLIST_H__

// ancestor class
#include "HUDuiScrollList.h"

#include "HUDuiServerListItem.h"
#include "ServerItem.h"
#include "ServerList.h"

#include "HUDuiFrame.h"

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
      DomainName = 0,
      ServerName,
      PlayerCount,
      Ping,
      NoSort
    } SortConstants;

    static double DOMAIN_PERCENTAGE;
    static double SERVER_PERCENTAGE;
    static double PLAYER_PERCENTAGE;
    static double PING_PERCENTAGE;

    void addItem(ServerItem item);
    void addItem(HUDuiControl* item);

    void update();
	
    void setServerList(ServerList* list);

    ServerItem* getSelectedServer();

    void sortBy(SortConstants sortType);
    void searchServers(std::string pattern);

    void applyFilters();
    void toggleFilter(FilterConstants filter);

    void setFontSize(float size);
    void setFontFace(const LocalFontFace* face);
    void setSize(float width, float height);

  protected:
    struct filter;
    struct search;
    template<int sortType> struct compare;

    static bool comp(HUDuiControl* first, HUDuiControl* second);

    static ServerList* dataList;

    size_t callbackHandler(size_t oldFocus, size_t proposedFocus, HUDNavChangeMethod changeMethod);

    bool doKeyPress(const BzfKeyEvent& key);

    void setActiveColumn(int column);
    int getActiveColumn();

    void refreshNavQueue();

    void doRender();
    void calculateLines();

    bool reverseSort;

  private:
    std::list<HUDuiControl*> originalItems;

    std::vector<std::pair<std::pair<float, float>, std::pair<float, float>>> linesToRender;

    SortConstants sortMode;
    uint16_t filterOptions;

    int activeColumn;

    bool devInfo;
};

#endif // __HUDuiServerList_H__

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8