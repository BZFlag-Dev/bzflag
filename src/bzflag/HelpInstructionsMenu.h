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

#ifndef __HELPINSTRUCTIONSMENU_H__
#define __HELPINSTRUCTIONSMENU_H__

#include "HelpMenu.h"

#include <vector>
#include <string>

class HelpInstructionsMenu : public HelpMenu {
public:
  HelpInstructionsMenu(const char* title, std::vector<std::string> text);
  ~HelpInstructionsMenu() { }

  void resize(int width, int height);

private:
  // no default constructor
  HelpInstructionsMenu();
};

#endif /* __HELPINSTRUCTIONSMENU_H__ */

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
