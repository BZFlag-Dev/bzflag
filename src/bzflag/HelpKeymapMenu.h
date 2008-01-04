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

#ifndef __HELPKEYMAPMENU_H__
#define __HELPKEYMAPMENU_H__

/* system headers */
#include <string>
#include <map>

/* parent interface header */
#include "HelpMenu.h"

class HelpKeymapMenu : public HelpMenu {
public:
  HelpKeymapMenu();
  ~HelpKeymapMenu() { }

  void		resize(int width, int height);

  void		onScan(const std::string& name, bool, const std::string&);
  static void	onScanCB(const std::string& name, bool press,
			 const std::string& cmd, void* userData);

protected:
  float		getLeftSide(int width, int height);

private:
  void		initKeymap(const std::string& name, int index);
  struct keymap {
    int index;	// ui label index
    std::string key1;
    std::string key2;
  };
  typedef std::map<std::string, keymap> KeyKeyMap;
  KeyKeyMap	mappable;
};

#endif /* __HELPKEYMAPMENU_H__ */

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
