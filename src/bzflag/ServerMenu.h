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

#ifndef __SERVERMENU_H__
#define __SERVERMENU_H__

// ancestor classes
#include "MenuDefaultKey.h"
#include "HUDDialog.h"

/* system interface headers */
#include <string>
#include <vector>
#include <map>

/* common interface headers */
#include "BzfEvent.h"
#include "ServerItem.h"
#include "ServerList.h"
#include "ServerPing.h"

/* local interface headers */
#include "HUDuiDefaultKey.h"
#include "HUDuiLabel.h"
#include "HUDuiTypeIn.h"

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
  int getSelected() const;
  void setSelected(int, bool forcerefresh=false);
  void show();
  void execute();
  void dismiss();
  void resize(int width, int height);
  static void playingCB(void*);
  void updateStatus();

  bool getFind() const;
  void setFind(bool mode);

  void toggleFavView();
  void setFav(bool);
  
  void pingServer(int server);

  static const int NumItems;

private:
  HUDuiLabel* addLabel(const char* str, const char* label, bool navigable = false);
  void pick();

  ServerItem& serversAt(int index);

private:
  ServerList realServerList;
  ServerList serverList;
  ServerMenuDefaultKey	defaultKey;
  HUDuiLabel* status;
  HUDuiLabel* help;

  HUDuiLabel* pageLabel;
  std::vector<HUDuiLabel*> readouts;
  std::vector<HUDuiLabel*> items;
  int selectedIndex;
  unsigned int serversFound;
  unsigned int realServersFound;

  HUDuiTypeIn* search;
  bool findMode;
  std::string filter;
  bool favView;
  bool newfilter;

  static const int NumReadouts;
  
  std::map<int, ServerPing*> activePings;
};


#endif /* __SERVERMENU_H__ */

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
