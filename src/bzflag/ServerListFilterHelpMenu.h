/* bzflag
 * Copyright (c) 1993-2018 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef __SERVERLISTFILTERHELPMENU_H__
#define __SERVERLISTFILTERHELPMENU_H__

/* common interface headers */
#include "BzfEvent.h"

/* local interface headers */
#include "HUDDialog.h"
#include "HUDuiDefaultKey.h"
#include "HUDuiControl.h"
#include "MenuDefaultKey.h"


class MenuDefaultKey;

class ServerListFilterHelpMenuDefaultKey : public MenuDefaultKey {
public:
  ServerListFilterHelpMenuDefaultKey() {}
  ~ServerListFilterHelpMenuDefaultKey() {}

  bool keyPress(const BzfKeyEvent&);
  bool keyRelease(const BzfKeyEvent&);
};


class ServerListFilterHelpMenu : public HUDDialog {
public:
  ServerListFilterHelpMenu(const char* title = "Server List Filter Help");
  ~ServerListFilterHelpMenu()
  {
  }

  HUDuiDefaultKey* getDefaultKey()
  {
    return &defaultKey;
  }
  void execute()
  {
  }
  void resize(int width, int height);

  static ServerListFilterHelpMenu* getServerListFilterHelpMenu(HUDDialog* = NULL, bool next = true);
  static void done();

protected:
  HUDuiControl* createLabel(const char* string, const char* label = NULL);
  HUDuiControl* createInput(const std::string &);
  virtual float	getLeftSide(int width, int height);

private:
  ServerListFilterHelpMenuDefaultKey defaultKey;
  static ServerListFilterHelpMenu** serverListFilterHelpMenus;
};


#endif /* __HELPMENU_H__ */

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
