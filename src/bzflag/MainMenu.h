/* bzflag
 * Copyright (c) 1993-2012 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef	__MAINMENU_H__
#define	__MAINMENU_H__

#include "common.h"

#include "HUDDialog.h"
#include "HUDuiControl.h"
#include "HUDuiDefaultKey.h"

class JoinMenu;
class OptionsMenu;
class QuitMenu;

/** MainMenu is the main menu
 */
class MainMenu : public HUDDialog {
public:
  MainMenu();
  ~MainMenu();

  HUDuiDefaultKey* getDefaultKey();
  void execute();
  void resize(int width, int height);
  void createControls();

  static int getFontFace();

private:

  HUDuiControl* createLabel(const char* string);

  HUDuiControl*	join;
  HUDuiControl*	login;
  HUDuiControl*	options;
  HUDuiControl*	help;
  HUDuiControl*	leave;
  HUDuiControl*	save;
  HUDuiControl*	quit;

  JoinMenu*	joinMenu;
  OptionsMenu*	optionsMenu;
  QuitMenu*	quitMenu;
};


#endif /* __MAINMENU_H__ */

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
