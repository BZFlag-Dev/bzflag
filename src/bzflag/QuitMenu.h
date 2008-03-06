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

#ifndef	__QUITMENU_H__
#define __QUITMENU_H__

/* system interface headers */

/* common interface headers */
#include "BzfEvent.h"
#include "CommandsStandard.h"

/* local interface headers */
#include "MenuDefaultKey.h"
#include "HUDDialog.h"
#include "HUDuiDefaultKey.h"


class QuitMenuDefaultKey : public MenuDefaultKey {
public:
  QuitMenuDefaultKey() { }
  ~QuitMenuDefaultKey() { }

  bool keyPress(const BzfKeyEvent&);
  bool keyRelease(const BzfKeyEvent&);

};


class QuitMenu : public HUDDialog {
public:
  QuitMenu();
  ~QuitMenu();

  HUDuiDefaultKey* getDefaultKey() { return &defaultKey; }
  void execute() { CommandsStandard::quit(); }
  void resize(int width, int height);

private:
  QuitMenuDefaultKey defaultKey;

};


#endif /* __QUITMENU_H__ */

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
