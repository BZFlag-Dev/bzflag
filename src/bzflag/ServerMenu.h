/* bzflag
 * Copyright (c) 1993 - 2004 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef __SERVERMENU_H__
#define __SERVERMENU_H__

/* system interface headers */
#include <vector>
#include <string>

/* common interface headers */
#include "ListServer.h"
#include "BzfEvent.h"
#include "HUDDialog.h"
#include "ServerItem.h"
#include "ServerListCache.h"

/* local interface headers */
#include "MenuDefaultKey.h"

class ServerMenu;
class HUDuiLabel;

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
  void setSelected(int);
  void show();
  void execute();
  void dismiss();
  void resize(int width, int height);
  size_t curlReader(void *ptr, size_t size, size_t nmemb);

  static const int NumItems;

private:
  void addLabel(const char* str, const char* label);
  void checkEchos();
  void readServerList(int index);
  void addToList(ServerItem&, bool doCache=false);
  void addToListWithLookup(ServerItem&);
  void setStatus(const char*, const std::vector<std::string> *parms = NULL);
  void pick();
  static void playingCB(void*);
  void addCacheToList();
  ServerItem& serversAt(int index);

private:
  ServerMenuDefaultKey	defaultKey;
  std::vector<ServerItem> servers;
  int pingBcastSocket;
  struct sockaddr_in pingBcastAddr;
  HUDuiLabel* status;

  HUDuiLabel* pageLabel;
  int selectedIndex;

  ListServer listServers[MaxListServers];
  ServerListCache* serverCache;
  bool addedCacheToList;

  static const int NumReadouts;

  CURLM *multiHandle;
  CURL  *easyHandle;

  std::string completeUrl;
};


#endif /* __SERVERMENU_H__ */

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
