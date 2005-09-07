/* bzflag
 * Copyright (c) 1993 - 2005 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef	__KEYBOARDMAPMENU_H__
#define	__KEYBOARDMAPMENU_H__

#include "common.h"

/* system interface headers */
#include <map>
#include <string>

/* common interface headers */
#include "BzfEvent.h"

/* local interface headers */
#include "HUDuiDefaultKey.h"
#include "HUDuiLabel.h"
#include "HUDuiControl.h"
#include "HUDDialog.h"
#include "QuickKeysMenu.h"
#include "MenuDefaultKey.h"


class KeyboardMapMenu;

class KeyboardMapMenuDefaultKey : public MenuDefaultKey {
  public:
			KeyboardMapMenuDefaultKey(KeyboardMapMenu*);
			~KeyboardMapMenuDefaultKey() { }

    bool		keyPress(const BzfKeyEvent&);
    bool		keyRelease(const BzfKeyEvent&);

  public:
    KeyboardMapMenu*	menu;
};

class KeyboardMapMenu : public HUDDialog {
  public:
			KeyboardMapMenu();
			~KeyboardMapMenu() { delete quickKeysMenu; }

    HUDuiDefaultKey*	getDefaultKey() { return &defaultKey; }
    void		execute();
    void		dismiss();
    void		resize(int width, int height);

    bool		isEditing() const;
    void		setKey(const BzfKeyEvent&);
    void		onScan(const std::string& name, bool press, const std::string& cmd);
    static void		onScanCB(const std::string& name, bool press,
				 const std::string& cmd, void* userData);

  private:
    void		update();

    HUDuiLabel*		createLabel(const char*, const char* = NULL);

    void		initkeymap(const std::string& name, int index);
  private:
    struct keymap {
      int index;	// ui label index
      std::string key1;
      std::string key2;
    };
    typedef std::map<std::string, keymap> KeyKeyMap;
    KeyKeyMap				mappable;
    KeyboardMapMenuDefaultKey		defaultKey;
    HUDuiControl*			reset;
    HUDuiControl*		       quickKeys;
    int				editing;
    QuickKeysMenu*			quickKeysMenu;
};


#endif /* __KEYBOARDMAPMENU_H__ */

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
