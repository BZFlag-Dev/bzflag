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

#ifndef	__SERVERMENU_H__
#define	__SERVERMENU_H__

#include "common.h"

/* local interface headers */
#include "HUDDialog.h"
#include "MenuDefaultKey.h"
#include "HUDuiDefaultKey.h"
#include "HUDuiControl.h"

#include "HUDuiServerList.h"
#include "ServerList.h"
#include "ServerItem.h"
#include "HUDuiTabbedControl.h"
#include "HUDuiServerInfo.h"
#include "HUDuiServerListCustomTab.h"

class ServerMenu;

class ServerMenuDefaultKey : public MenuDefaultKey {
public:
  ServerMenuDefaultKey(ServerMenu* _menu) :
    menu(_menu) { }
  ~ServerMenuDefaultKey() { }

  bool keyPress(const BzfKeyEvent&);
  bool keyRelease(const BzfKeyEvent&);

private:
  ServerMenu* menu;
};

class ServerMenu : public HUDDialog {
public:
  ServerMenu();
  ~ServerMenu();

  HUDuiDefaultKey* getDefaultKey() { return &defaultKey; }

  void		execute();
  void		resize(int width, int height);

  ServerList serverList;

  static void	callback(HUDuiControl* w, void* data);

  void updateStatus();
  void markAsFavorite(ServerItem* item);

  static void playingCB(void*);

  HUDuiServerList* normalList;
  HUDuiServerList* recentList;
  HUDuiServerList* favoritesList;

  ServerMenuDefaultKey	defaultKey;

  HUDuiLabel* title;
  HUDuiTabbedControl* tabbedControl;
  HUDuiServerInfo* serverInfo;
  HUDuiServerListCustomTab* customTabControl;
};


#endif /* __SERVERMENU_H__ */

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
