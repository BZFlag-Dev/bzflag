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
      JumpingOn    = 0x0008,
      JumpingOff   = 0x0010,
      RicochetOn   = 0x0020,
      RicochetOff  = 0x0040,
      AntidoteFlagOn = 0x0080,
      AntidoteFlagOff = 0x0100,
      SuperFlagsOn = 0x0200,
      SuperFlagsOff = 0x0400,
      HandicapOn = 0x0800,
      HandicapOff = 0x1000,
      ClassicCTFGameMode = 0x2000,
      RabbitChaseGameMode = 0x4000,
      OpenFFAGameMode = 0x8000,
      EndOfFilterConstants
    } FilterConstants;

    typedef enum {
      DomainName = 0,
      ServerName,
      PlayerCount,
      Ping,
      NoSort
    } SortConstants;

    static float DOMAIN_PERCENTAGE;
    static float SERVER_PERCENTAGE;
    static float PLAYER_PERCENTAGE;
    static float PING_PERCENTAGE;

    std::map<int, std::pair<std::string, float*>> columns; 

    void addItem(ServerItem* item);
    void addItem(HUDuiControl* item);

    void removeItem(ServerItem* item);
    void clearList();

    void update();

    HUDuiServerListItem* get(int index);

    ServerItem* getSelectedServer();

    void sortBy(SortConstants sortType);
    void searchServers(std::string pattern);

    float getHeight() const;

    void applyFilters();
    void toggleFilter(FilterConstants filter);

    void setFontSize(float size);
    void setFontFace(const LocalFontFace* face);
    void setSize(float width, float height);

    ServerList &dataList;

  protected:
    struct filter;
    struct search;
    template<int sortType> struct compare;

    static bool comp(HUDuiControl* first, HUDuiControl* second);
    static bool equal(HUDuiControl* first, HUDuiControl* second);

    size_t callbackHandler(size_t oldFocus, size_t proposedFocus, HUDNavChangeMethod changeMethod);

    bool doKeyPress(const BzfKeyEvent& key);

    void setActiveColumn(int column);
    int getActiveColumn();

    void refreshNavQueue();

    void doRender();

    bool reverseSort;

  private:
    std::list<HUDuiControl*> originalItems;

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
